#include "PLCore.h"
#include "PLEventQueue.h"
#include "PLKeyEncoding.h"
#include "PLMovies.h"
#include "PLSysCalls.h"
#include "PLTimeTaggedVOSEvent.h"
#include "DisplayDeviceManager.h"
#include "GpVOSEvent.h"
#include "InputManager.h"
#include "HostSuspendCallArgument.h"
#include "HostSuspendHook.h"
#include "HostVOSEventQueue.h"
#include "MacRomanConversion.h"

static void TranslateMouseInputEvent(const GpVOSEvent &vosEventBase, uint32_t timestamp, PortabilityLayer::EventQueue *queue)
{
	const GpMouseInputEvent &vosEvent = vosEventBase.m_event.m_mouseInputEvent;

	bool requeue = false;
	if (vosEvent.m_button == GpMouseButtons::kLeft)
	{
		if (vosEvent.m_eventType == GpMouseEventTypes::kDown)
			requeue = true;
		else if (vosEvent.m_eventType == GpMouseEventTypes::kUp)
			requeue = true;
	}
	else if (vosEvent.m_eventType == GpMouseEventTypes::kMove)
		requeue = true;

	if (requeue)
	{
		if (TimeTaggedVOSEvent *evt = queue->Enqueue())
			*evt = TimeTaggedVOSEvent::Create(vosEventBase, timestamp);
	}
}

static void TranslateGamepadInputEvent(const GpGamepadInputEvent &vosEvent, PortabilityLayer::EventQueue *queue)
{
	PortabilityLayer::InputManager *inputManager = PortabilityLayer::InputManager::GetInstance();

	inputManager->ApplyGamepadEvent(vosEvent);

	PL_DEAD(queue);
}

static void TranslateKeyboardInputEvent(const GpVOSEvent &vosEventBase, uint32_t timestamp, PortabilityLayer::EventQueue *queue)
{
	const GpKeyboardInputEvent &vosEvent = vosEventBase.m_event.m_keyboardInputEvent;

	GP_STATIC_ASSERT((1 << PL_INPUT_PLAYER_INDEX_BITS) >= PL_INPUT_MAX_PLAYERS);
	GP_STATIC_ASSERT((1 << PL_INPUT_TYPE_CODE_BITS) >= KeyEventType_Count);

	PortabilityLayer::InputManager *inputManager = PortabilityLayer::InputManager::GetInstance();

	if (vosEvent.m_eventType == GpKeyboardInputEventTypes::kUp || vosEvent.m_eventType == GpKeyboardInputEventTypes::kDown)
		inputManager->ApplyKeyboardEvent(vosEvent);

	if (TimeTaggedVOSEvent *evt = queue->Enqueue())
		*evt = TimeTaggedVOSEvent::Create(vosEventBase, timestamp);
}

intptr_t PackVOSKeyCode(const GpKeyboardInputEvent &vosEvent)
{
	switch (vosEvent.m_keyIDSubset)
	{
	case GpKeyIDSubsets::kASCII:
		return PL_KEY_ASCII(vosEvent.m_key.m_asciiChar);
	case GpKeyIDSubsets::kFKey:
		return PL_KEY_FKEY(vosEvent.m_key.m_fKey);
	case GpKeyIDSubsets::kNumPadNumber:
		return PL_KEY_NUMPAD_NUMBER(vosEvent.m_key.m_numPadNumber);
	case GpKeyIDSubsets::kSpecial:
		return PL_KEY_SPECIAL_ENCODE(vosEvent.m_key.m_specialKey);
		break;
	case GpKeyIDSubsets::kNumPadSpecial:
		return PL_KEY_NUMPAD_SPECIAL_ENCODE(vosEvent.m_key.m_numPadSpecialKey);
		break;
	case GpKeyIDSubsets::kUnicode:
		for (int i = 128; i < 256; i++)
		{
			if (MacRoman::ToUnicode(i) == vosEvent.m_key.m_unicodeChar)
				return PL_KEY_MACROMAN(i);
		}
		break;
	case GpKeyIDSubsets::kGamepadButton:
		return PL_KEY_GAMEPAD_BUTTON_ENCODE(vosEvent.m_key.m_gamepadKey.m_button, vosEvent.m_key.m_gamepadKey.m_player);
	default:
		return 0;
	}

	return 0;
}

static void TranslateVOSEvent(const GpVOSEvent *vosEvent, uint32_t timestamp, PortabilityLayer::EventQueue *queue)
{
	switch (vosEvent->m_eventType)
	{
	case GpVOSEventTypes::kMouseInput:
		TranslateMouseInputEvent(*vosEvent, timestamp, queue);
		break;
	case GpVOSEventTypes::kKeyboardInput:
		TranslateKeyboardInputEvent(*vosEvent, timestamp, queue);
		break;
	case GpVOSEventTypes::kGamepadInput:
		TranslateGamepadInputEvent(vosEvent->m_event.m_gamepadInputEvent, queue);
		break;
	}
}

static void ImportVOSEvents(uint32_t timestamp)
{
	PortabilityLayer::EventQueue *plQueue = PortabilityLayer::EventQueue::GetInstance();

	PortabilityLayer::HostVOSEventQueue *evtQueue = PortabilityLayer::HostVOSEventQueue::GetInstance();
	while (const GpVOSEvent *evt = evtQueue->GetNext())
	{
		TranslateVOSEvent(evt, timestamp, plQueue);
		evtQueue->DischargeOne();
	}
}

namespace PLSysCalls
{
	void Sleep(uint32_t ticks)
	{
		if (ticks > 0)
		{
			PortabilityLayer::HostSuspendCallArgument args[1];
			args[0].m_uint = static_cast<uint32_t>(ticks);

			PortabilityLayer::SuspendApplication(PortabilityLayer::HostSuspendCallID_Delay, args, nullptr);

			ImportVOSEvents(PortabilityLayer::DisplayDeviceManager::GetInstance()->GetTickCount());

			AnimationManager::GetInstance()->TickPlayers(ticks);
		}
	}
}