
//============================================================================
//----------------------------------------------------------------------------
//									Settings.c
//----------------------------------------------------------------------------
//============================================================================

#include "PLArrayView.h"
#include "PLKeyEncoding.h"
#include "PLSound.h"
#include "PLStandardColors.h"
#include "PLTextUtils.h"
#include "PLTimeTaggedVOSEvent.h"
#include "PLWidgets.h"
#include "DialogManager.h"
#include "DialogUtils.h"
#include "Externs.h"
#include "Environ.h"
#include "House.h"


#define kMainPrefsDialID		1012
#define kDisplayPrefsDialID		1017
#define kSoundPrefsDialID		1018
#define kControlPrefsDialID		1023
#define kBrainsPrefsDialID		1024
#define kDisplayButton			3
#define kSoundButton			4
#define kControlsButton			5
#define kBrainsButton			6
#define kDisplay1Item			3
#define kDisplay3Item			4
#define kDisplay9Item			5
#define kDoColorFadeItem		9
#define kCurrentDepth			10
#define k256Depth				11
#define k16Depth				12
#define kDispDefault			15
#define kUseQDItem				16
#define kUseScreen2Item			17
#define kSofterItem				4
#define kLouderItem				5
#define kVolNumberItem			7
#define kIdleMusicItem			8
#define kPlayMusicItem			9
#define kSoundDefault			13
#define kRightControl			5
#define kLeftControl			6
#define kBattControl			7
#define kBandControl			8
#define kControlDefaults		13
#define kESCPausesRadio			14
#define kTABPausesRadio			15
#define kMaxFilesItem			5
#define kQuickTransitCheck		7
#define kDoZoomsCheck			8
#define kBrainsDefault			9
#define kDoDemoCheck			10
#define kDoBackgroundCheck		11
#define kDoErrorCheck			12
#define kDoPrettyMapCheck		13
#define kDoBitchDlgsCheck		14


void SetBrainsToDefaults (Dialog *);
void UpdateSettingsBrains (Dialog *);
int16_t BrainsFilter (Dialog *, const TimeTaggedVOSEvent *);
void DoBrainsPrefs (void);
void SetControlsToDefaults (Dialog *);
void UpdateControlKeyName (Dialog *);
void UpdateSettingsControl (Dialog *);
int16_t ControlFilter (Dialog *, const TimeTaggedVOSEvent *);
void DoControlPrefs (void);
void SoundDefaults (Dialog *);
void UpdateSettingsSound (Dialog *);
void HandleSoundMusicChange (short, Boolean);
int16_t SoundFilter(Dialog *, const TimeTaggedVOSEvent *);
void DoSoundPrefs (void);
void DisplayDefaults (void);
void FrameDisplayIcon (Dialog *, const PortabilityLayer::RGBAColor &color);
void DisplayUpdate (Dialog *);
int16_t DisplayFilter(Dialog *dialog, const TimeTaggedVOSEvent *);
void DoDisplayPrefs (void);
void SetAllDefaults (void);
void FlashSettingsButton (DrawSurface *, short);
void UpdateSettingsMain (Dialog *);
int16_t PrefsFilter(Dialog *dialog, const TimeTaggedVOSEvent *evt);
void BitchAboutChanges (void);


Rect		prefButton[4], controlRects[4];
Str15		leftName, rightName, batteryName, bandName;
Str15		tempLeftStr, tempRightStr, tempBattStr, tempBandStr;
long		tempLeftMap, tempRightMap, tempBattMap, tempBandMap;
short		whichCtrl, wasDepthPref;
Boolean		wasFade, wasIdle, wasPlay, wasTransit, wasZooms, wasBackground;
Boolean		wasEscPauseKey, wasDemos, wasScreen2, nextRestartChange, wasErrorCheck;
Boolean		wasPrettyMap, wasBitchDialogs;

extern	short		numNeighbors, isDepthPref, maxFiles, willMaxFiles;
extern	Boolean		isDoColorFade, isPlayMusicIdle, isUseSecondScreen;
extern	Boolean		isHouseChecks, doBitchDialogs;
extern	Boolean		isEscPauseKey, failedMusic, isSoundOn, doBackground;
extern	Boolean		isMusicOn, quickerTransitions, doAutoDemo;
extern	Boolean		changeLockStateOfHouse, saveHouseLocked, doPrettyMap;


//==============================================================  Functions
//--------------------------------------------------------------  SetBrainsToDefaults

void SetBrainsToDefaults (Dialog *theDialog)
{
	SetDialogNumToStr(theDialog, kMaxFilesItem, 24L);
#ifdef powerc
	wasTransit = false;
#else
	wasTransit = true;
#endif
	wasZooms = true;
	wasDemos = true;
	wasBackground = false;
	wasErrorCheck = true;
	wasPrettyMap = true;
	wasBitchDialogs = true;
	SetDialogItemValue(theDialog, kQuickTransitCheck, (short)wasTransit);
	SetDialogItemValue(theDialog, kDoZoomsCheck, (short)wasZooms);
	SetDialogItemValue(theDialog, kDoDemoCheck, (short)wasDemos);
	SetDialogItemValue(theDialog, kDoBackgroundCheck, (short)wasBackground);
	SetDialogItemValue(theDialog, kDoErrorCheck, (short)wasErrorCheck);
	SetDialogItemValue(theDialog, kDoPrettyMapCheck, (short)wasPrettyMap);
	SetDialogItemValue(theDialog, kDoBitchDlgsCheck, (short)wasBitchDialogs);
}

//--------------------------------------------------------------  UpdateSettingsBrains

void UpdateSettingsBrains (Dialog *theDialog)
{
	DrawDefaultButton(theDialog);
	
	SetDialogNumToStr(theDialog, kMaxFilesItem, (long)willMaxFiles);
	SelectDialogItemText(theDialog, kMaxFilesItem, 0, 1024);
	
	FrameDialogItemC(theDialog, 3, kRedOrangeColor8);
}

//--------------------------------------------------------------  BrainsFilter

int16_t BrainsFilter (Dialog *dial, const TimeTaggedVOSEvent *evt)
{
	if (!evt)
		return -1;

	if (evt->IsKeyDownEvent())
	{
		intptr_t keyCode = PackVOSKeyCode(evt->m_vosEvent.m_event.m_keyboardInputEvent);

		switch (keyCode)
		{
		case PL_KEY_SPECIAL(kEnter):
		case PL_KEY_NUMPAD_SPECIAL(kEnter):
			FlashDialogButton(dial, kOkayButton);
			return kOkayButton;

		case PL_KEY_SPECIAL(kEscape):
			FlashDialogButton(dial, kCancelButton);
			return kCancelButton;

		case PL_KEY_ASCII('A'):
			return kDoDemoCheck;

		case PL_KEY_ASCII('B'):
			return kDoBackgroundCheck;

		case PL_KEY_ASCII('D'):
			FlashDialogButton(dial, kBrainsDefault);
			return kBrainsDefault;

		case PL_KEY_ASCII('E'):
			return kDoErrorCheck;

		case PL_KEY_ASCII('Q'):
			return kQuickTransitCheck;

		case PL_KEY_ASCII('Z'):
			return kDoZoomsCheck;

		default:
			return -1;
		}
	}

	if (evt->m_vosEvent.m_eventType == GpVOSEventTypes::kKeyboardInput)
	{
		const GpKeyboardInputEvent &keyEvent = evt->m_vosEvent.m_event.m_keyboardInputEvent;
		if (keyEvent.m_keyIDSubset == GpKeyIDSubsets::kUnicode)
			return 0;
		if (keyEvent.m_keyIDSubset == GpKeyIDSubsets::kASCII)
		{
			char asciiChar = (keyEvent.m_key.m_asciiChar);
			if (asciiChar < '0' || asciiChar > '9')
				return 0;
		}
	}

	return -1;
}

//--------------------------------------------------------------  DoBrainsPrefs

void DoBrainsPrefs (void)
{
	Dialog			*prefDlg;
	long			tempLong;
	short			itemHit, wasMaxFiles;
	Boolean			leaving;
	
	BringUpDialog(&prefDlg, kBrainsPrefsDialID, nullptr);
	leaving = false;
	wasMaxFiles = willMaxFiles;
	
	wasTransit = quickerTransitions;
	wasZooms = doZooms;
	wasDemos = doAutoDemo;
	wasBackground = doBackground;
	wasErrorCheck = isHouseChecks;
	wasPrettyMap = doPrettyMap;
	wasBitchDialogs = doBitchDialogs;
	
	SetDialogItemValue(prefDlg, kQuickTransitCheck, (short)wasTransit);
	SetDialogItemValue(prefDlg, kDoZoomsCheck, (short)wasZooms);
	SetDialogItemValue(prefDlg, kDoDemoCheck, (short)wasDemos);
	SetDialogItemValue(prefDlg, kDoBackgroundCheck, (short)wasBackground);
	SetDialogItemValue(prefDlg, kDoErrorCheck, (short)wasErrorCheck);
	SetDialogItemValue(prefDlg, kDoPrettyMapCheck, (short)wasPrettyMap);
	SetDialogItemValue(prefDlg, kDoBitchDlgsCheck, (short)wasBitchDialogs);

	UpdateSettingsBrains(prefDlg);

	prefDlg->GetWindow()->FocusWidget(prefDlg->GetItems()[kMaxFilesItem - 1].GetWidget());

	while (!leaving)
	{
		itemHit = prefDlg->ExecuteModal(BrainsFilter);
		switch (itemHit)
		{
			case kOkayButton:
			GetDialogNumFromStr(prefDlg, kMaxFilesItem, &tempLong);
			if (tempLong > 500)
				tempLong = 500;
			else if (tempLong < 12)
				tempLong = 12;
			willMaxFiles = static_cast<short>(tempLong);
			if (willMaxFiles != wasMaxFiles)
				nextRestartChange = true;
			quickerTransitions = wasTransit;
			doZooms = wasZooms;
			doAutoDemo = wasDemos;
			doBackground = wasBackground;
			isHouseChecks = wasErrorCheck;
			doPrettyMap = wasPrettyMap;
			doBitchDialogs = wasBitchDialogs;
			leaving = true;
			break;
			
			case kCancelButton:
			willMaxFiles = wasMaxFiles;
			leaving = true;
			break;
			
			case kQuickTransitCheck:
			wasTransit = !wasTransit;
			SetDialogItemValue(prefDlg, kQuickTransitCheck, (short)wasTransit);
			break;
			
			case kDoZoomsCheck:
			wasZooms = !wasZooms;
			SetDialogItemValue(prefDlg, kDoZoomsCheck, (short)wasZooms);
			break;
			
			case kDoDemoCheck:
			wasDemos = !wasDemos;
			SetDialogItemValue(prefDlg, kDoDemoCheck, (short)wasDemos);
			break;
			
			case kDoBackgroundCheck:
			wasBackground = !wasBackground;
			SetDialogItemValue(prefDlg, kDoBackgroundCheck, (short)wasBackground);
			break;
			
			case kBrainsDefault:
			SetBrainsToDefaults(prefDlg);
			break;
			
			case kDoErrorCheck:
			wasErrorCheck = !wasErrorCheck;
			SetDialogItemValue(prefDlg, kDoErrorCheck, (short)wasErrorCheck);
			break;
			
			case kDoPrettyMapCheck:
			wasPrettyMap = !wasPrettyMap;
			SetDialogItemValue(prefDlg, kDoPrettyMapCheck, (short)wasPrettyMap);
			break;
			
			case kDoBitchDlgsCheck:
			wasBitchDialogs = !wasBitchDialogs;
			SetDialogItemValue(prefDlg, kDoBitchDlgsCheck, (short)wasBitchDialogs);
			break;
		}
	}
	
	prefDlg->Destroy();
}

//--------------------------------------------------------------  SetControlsToDefaults

void SetControlsToDefaults (Dialog *theDialog)
{
	PasStringCopy(PSTR("lf arrow"), tempLeftStr);
	PasStringCopy(PSTR("rt arrow"), tempRightStr);
	PasStringCopy(PSTR("dn arrow"), tempBattStr);
	PasStringCopy(PSTR("up arrow"), tempBandStr);
	tempLeftMap = PL_KEY_SPECIAL(kLeftArrow);
	tempRightMap = PL_KEY_SPECIAL(kRightArrow);
	tempBattMap = PL_KEY_SPECIAL(kDownArrow);
	tempBandMap = PL_KEY_SPECIAL(kUpArrow);
	wasEscPauseKey = false;
	SelectFromRadioGroup(theDialog, kTABPausesRadio, 
				kESCPausesRadio, kTABPausesRadio);
}

//--------------------------------------------------------------  UpdateControlKeyName

void UpdateControlKeyName (Dialog *theDialog)
{
	DrawDialogUserText(theDialog, kRightControl + 4, tempRightStr, whichCtrl == 0);
	DrawDialogUserText(theDialog, kLeftControl + 4, tempLeftStr, whichCtrl == 1);
	DrawDialogUserText(theDialog, kBattControl + 4, tempBattStr, whichCtrl == 2);
	DrawDialogUserText(theDialog, kBandControl + 4, tempBandStr, whichCtrl == 3);
}

//--------------------------------------------------------------  UpdateSettingsControl

void UpdateSettingsControl (Dialog *theDialog)
{
	short		i;
	DrawSurface	*surface = theDialog->GetWindow()->GetDrawSurface();
	
	surface->SetForeColor(PortabilityLayer::RGBAColor::Create(255, 255, 255, 255));
	for (i = 0; i < 4; i++)
	{
		Rect rect = controlRects[i];
		surface->FrameRect(rect);
		InsetRect(&rect, 1, 1);
		surface->FrameRect(rect);
	}

	surface->SetForeColor(PortabilityLayer::RGBAColor::Create(255, 0, 0, 255));

	{
		Rect rect = controlRects[whichCtrl];
		surface->FrameRect(rect);
		InsetRect(&rect, 1, 1);
		surface->FrameRect(rect);
	}

	surface->SetForeColor(PortabilityLayer::RGBAColor::Create(0, 0, 0, 255));

	UpdateControlKeyName(theDialog);
	FrameDialogItemC(theDialog, 3, kRedOrangeColor8);
}

//--------------------------------------------------------------  ControlFilter

int16_t ControlFilter (Dialog *dial, const TimeTaggedVOSEvent *evt)
{
	intptr_t		wasKeyMap;

	if (!evt)
		return -1;

	if (evt->IsKeyDownEvent())
	{
		GpKeyIDSubset_t subset = evt->m_vosEvent.m_event.m_keyboardInputEvent.m_keyIDSubset;

		// Ignore Unicode (for now) and gamepad buttons
		if (subset == GpKeyIDSubsets::kASCII || subset == GpKeyIDSubsets::kSpecial || subset == GpKeyIDSubsets::kNumPadNumber || subset == GpKeyIDSubsets::kNumPadSpecial || subset == GpKeyIDSubsets::kFKey)
		{
			wasKeyMap = PackVOSKeyCode(evt->m_vosEvent.m_event.m_keyboardInputEvent);

			switch (whichCtrl)
			{
			case 0:
				if ((wasKeyMap == tempLeftMap) || (wasKeyMap == tempBattMap) ||
					(wasKeyMap == tempBandMap) || (wasKeyMap == PL_KEY_SPECIAL(kTab)) ||
					(wasKeyMap == PL_KEY_SPECIAL(kEscape)) || (wasKeyMap == PL_KEY_SPECIAL(kDelete)))
				{
					if (wasKeyMap == PL_KEY_SPECIAL(kEscape))
					{
						FlashDialogButton(dial, kCancelButton);
						return kCancelButton;
					}
					else
						SysBeep(1);
				}
				else
				{
					GetKeyName(wasKeyMap, tempRightStr);
					tempRightMap = wasKeyMap;
				}
				break;

			case 1:
				if ((wasKeyMap == tempRightMap) || (wasKeyMap == tempBattMap) ||
					(wasKeyMap == tempBandMap) || (wasKeyMap == PL_KEY_SPECIAL(kTab)) ||
					(wasKeyMap == PL_KEY_SPECIAL(kEscape)) || (wasKeyMap == PL_KEY_SPECIAL(kDelete)))
				{
					if (wasKeyMap == PL_KEY_SPECIAL(kEscape))
					{
						FlashDialogButton(dial, kCancelButton);
						return kCancelButton;
					}
					else
						SysBeep(1);
				}
				else
				{
					GetKeyName(wasKeyMap, tempLeftStr);
					tempLeftMap = wasKeyMap;
				}
				break;

			case 2:
				if ((wasKeyMap == tempRightMap) || (wasKeyMap == tempLeftMap) ||
					(wasKeyMap == tempBandMap) || (wasKeyMap == PL_KEY_SPECIAL(kTab)) ||
					(wasKeyMap == PL_KEY_SPECIAL(kEscape)) || (wasKeyMap == PL_KEY_SPECIAL(kDelete)))
				{
					if (wasKeyMap == PL_KEY_SPECIAL(kEscape))
					{
						FlashDialogButton(dial, kCancelButton);
						return kCancelButton;
						return(true);
					}
					else
						SysBeep(1);
				}
				else
				{
					GetKeyName(wasKeyMap, tempBattStr);
					tempBattMap = wasKeyMap;
				}
				break;

			case 3:
				if ((wasKeyMap == tempRightMap) || (wasKeyMap == tempLeftMap) ||
					(wasKeyMap == tempBattMap) || (wasKeyMap == PL_KEY_SPECIAL(kTab)) ||
					(wasKeyMap == PL_KEY_SPECIAL(kEscape)) || (wasKeyMap == PL_KEY_SPECIAL(kDelete)))
				{
					if (wasKeyMap == PL_KEY_SPECIAL(kEscape))
					{
						FlashDialogButton(dial, kCancelButton);
						return kCancelButton;
					}
					else
						SysBeep(1);
				}
				else
				{
					GetKeyName(wasKeyMap, tempBandStr);
					tempBandMap = wasKeyMap;
				}
				break;
			}
			UpdateControlKeyName(dial);
			return -1;
		}
	}

	return -1;
}

//--------------------------------------------------------------  DoControlPrefs

void DoControlPrefs (void)
{
	Dialog			*prefDlg;
	short			i, itemHit;
	Boolean			leaving;
	
//	CenterDialog(kControlPrefsDialID);
	prefDlg = PortabilityLayer::DialogManager::GetInstance()->LoadDialog(kControlPrefsDialID, kPutInFront, nullptr);
	if (prefDlg == nil)
		RedAlert(kErrDialogDidntLoad);
	SetGraphicsPort(&prefDlg->GetWindow()->m_surface);
	for (i = 0; i < 4; i++)
	{
		GetDialogItemRect(prefDlg, i + kRightControl, &controlRects[i]);
		InsetRect(&controlRects[i], -3, -3);
	}
	whichCtrl = 1;
	
	PasStringCopy(leftName, tempLeftStr);
	PasStringCopy(rightName, tempRightStr);
	PasStringCopy(batteryName, tempBattStr);
	PasStringCopy(bandName, tempBandStr);
	tempLeftMap = theGlider.leftKey;
	tempRightMap = theGlider.rightKey;
	tempBattMap = theGlider.battKey;
	tempBandMap = theGlider.bandKey;
	wasEscPauseKey = isEscPauseKey;
	
	leaving = false;
	
	ShowWindow(prefDlg->GetWindow());
	if (isEscPauseKey)
		SelectFromRadioGroup(prefDlg, kESCPausesRadio, 
				kESCPausesRadio, kTABPausesRadio);
	else
		SelectFromRadioGroup(prefDlg, kTABPausesRadio, 
				kESCPausesRadio, kTABPausesRadio);

	DrawSurface *surface = prefDlg->GetWindow()->GetDrawSurface();

	UpdateSettingsControl(prefDlg);
	
	while (!leaving)
	{
		itemHit = prefDlg->ExecuteModal(ControlFilter);
		switch (itemHit)
		{
			case kOkayButton:
			PasStringCopy(tempLeftStr, leftName);
			PasStringCopy(tempRightStr, rightName);
			PasStringCopy(tempBattStr, batteryName);
			PasStringCopy(tempBandStr, bandName);
			theGlider.leftKey = tempLeftMap;
			theGlider.rightKey = tempRightMap;
			theGlider.battKey = tempBattMap;
			theGlider.bandKey = tempBandMap;
			isEscPauseKey = wasEscPauseKey;
			leaving = true;
			break;
			
			case kCancelButton:
			leaving = true;
			break;
			
			case kRightControl:
			case kLeftControl:
			case kBattControl:
			case kBandControl:
			{
				Rect ctrlRect = controlRects[whichCtrl];

				surface->SetForeColor(StdColors::White());
				surface->FrameRect(ctrlRect);
				InsetRect(&ctrlRect, 1, 1);
				surface->FrameRect(ctrlRect);

				whichCtrl = itemHit - kRightControl;

				ctrlRect = controlRects[whichCtrl];
				surface->SetForeColor(StdColors::Red());
				surface->FrameRect(ctrlRect);
				InsetRect(&ctrlRect, 1, 1);
				surface->FrameRect(ctrlRect);
			}

			UpdateControlKeyName(prefDlg);
			break;
			
			case kESCPausesRadio:
			case kTABPausesRadio:
			SelectFromRadioGroup(prefDlg, itemHit, kESCPausesRadio, kTABPausesRadio);
			wasEscPauseKey = !wasEscPauseKey;
			break;
			
			case kControlDefaults:
			SetControlsToDefaults(prefDlg);
			UpdateControlKeyName(prefDlg);
			break;
		}
	}

	prefDlg->Destroy();
}

//--------------------------------------------------------------  SoundDefaults

void SoundDefaults (Dialog *theDialog)
{	
	wasIdle = true;
	wasPlay = true;
	SetDialogItemValue(theDialog, kIdleMusicItem, (short)wasIdle);
	SetDialogItemValue(theDialog, kPlayMusicItem, (short)wasPlay);
	UnivSetSoundVolume(3, thisMac.hasSM3);
	SetDialogNumToStr(theDialog, kVolNumberItem, 3L);
	HandleSoundMusicChange(3, true);
}

//--------------------------------------------------------------  UpdateSettingsSound

void UpdateSettingsSound (Dialog *theDialog)
{
	short		howLoudNow;

	DrawDefaultButton(theDialog);
	
	UnivGetSoundVolume(&howLoudNow, thisMac.hasSM3);
	
	if (howLoudNow >= 7)
		SetDialogNumToStr(theDialog, kVolNumberItem, 11L);
	else
		SetDialogNumToStr(theDialog, kVolNumberItem, (long)howLoudNow);
	
	FrameDialogItemC(theDialog, 11, kRedOrangeColor8);
}

//--------------------------------------------------------------  HandleSoundMusicChange

void HandleSoundMusicChange (short newVolume, Boolean sayIt)
{
	PLError_t		theErr;
	
	isSoundOn = (newVolume != 0);
	
	if (wasIdle)
	{
		if (newVolume == 0)
			StopTheMusic();
		else
		{
			if (!isMusicOn)
			{
				theErr = StartMusic();
				if (theErr != PLErrors::kNone)
				{
					YellowAlert(kYellowNoMusic, theErr);
					failedMusic = true;
				}
			}
		}
	}
	
	if ((newVolume != 0) && (sayIt))
		PlayPrioritySound(kChord2Sound, kChord2Priority);
}

//--------------------------------------------------------------  SoundFilter

int16_t SoundFilter (Dialog *dial, const TimeTaggedVOSEvent *evt)
{
	short		newVolume;

	if (!evt)
		return -1;

	if (evt->IsKeyDownEvent())
	{
		intptr_t keyCode = PackVOSKeyCode(evt->m_vosEvent.m_event.m_keyboardInputEvent);

		switch (keyCode)
		{
		case PL_KEY_SPECIAL(kEnter):
		case PL_KEY_NUMPAD_SPECIAL(kEnter):
			FlashDialogButton(dial, kOkayButton);
			return kOkayButton;

		case PL_KEY_SPECIAL(kEscape):
			FlashDialogButton(dial, kCancelButton);
			return kCancelButton;

		case PL_KEY_SPECIAL(kUpArrow):
			return kLouderItem;

		case PL_KEY_SPECIAL(kDownArrow):
			return kSofterItem;

		case PL_KEY_ASCII('0'):
		case PL_KEY_ASCII('1'):
		case PL_KEY_ASCII('2'):
		case PL_KEY_ASCII('3'):
		case PL_KEY_ASCII('4'):
		case PL_KEY_ASCII('5'):
		case PL_KEY_ASCII('6'):
		case PL_KEY_ASCII('7'):
			newVolume = PL_KEY_GET_VALUE(keyCode) - '0';
			if (newVolume == 7L)
				SetDialogNumToStr(dial, kVolNumberItem, 11L);
			else
				SetDialogNumToStr(dial, kVolNumberItem, (long)newVolume);

			UnivSetSoundVolume(newVolume, thisMac.hasSM3);

			HandleSoundMusicChange(newVolume, true);
			return -1;

		case PL_KEY_ASCII('D'):
			FlashDialogButton(dial, kSoundDefault);
			return kSoundDefault;

		case PL_KEY_ASCII('G'):
			return kPlayMusicItem;

		case PL_KEY_ASCII('I'):
			return kIdleMusicItem;

		default:
			return -1;
		}
	}

	return -1;
}

//--------------------------------------------------------------  DoSettingsMain

void DoSoundPrefs (void)
{
	Rect			tempRect;
	Dialog			*prefDlg;
	short			wasLoudness, tempVolume;
	PLError_t		theErr;
	short			itemHit;
	Boolean			leaving;
	
	BringUpDialog(&prefDlg, kSoundPrefsDialID, nullptr);

	DrawSurface *surface = prefDlg->GetWindow()->GetDrawSurface();

	UpdateSettingsSound(prefDlg);
	
	UnivGetSoundVolume(&wasLoudness, thisMac.hasSM3);
	
	wasIdle = isPlayMusicIdle;
	wasPlay = isPlayMusicGame;
	SetDialogItemValue(prefDlg, kIdleMusicItem, (short)wasIdle);
	SetDialogItemValue(prefDlg, kPlayMusicItem, (short)wasPlay);
	leaving = false;

	while (!leaving)
	{
		itemHit = prefDlg->ExecuteModal(SoundFilter);

		switch (itemHit)
		{
			case kOkayButton:
			isPlayMusicIdle = wasIdle;
			isPlayMusicGame = wasPlay;
			leaving = true;
			UnivGetSoundVolume(&tempVolume, thisMac.hasSM3);
			isSoundOn = (tempVolume != 0);
			break;
			
			case kCancelButton:
			UnivSetSoundVolume(wasLoudness, thisMac.hasSM3);
			HandleSoundMusicChange(wasLoudness, false);
			if (isPlayMusicIdle != wasIdle)
			{
				if (isPlayMusicIdle)
				{
					if (wasLoudness != 0)
					{
						theErr = StartMusic();
						if (theErr != PLErrors::kNone)
						{
							YellowAlert(kYellowNoMusic, theErr);
							failedMusic = true;
						}
					}
				}
				else
					StopTheMusic();
			}
			leaving = true;
			break;
			
			case kSofterItem:
			UnivGetSoundVolume(&tempVolume, thisMac.hasSM3);	
			if (tempVolume > 0)
			{
				GetDialogItemRect(prefDlg, kSofterItem, &tempRect);
				DrawCIcon(surface, 1034, tempRect.left, tempRect.top);
				tempVolume--;
				SetDialogNumToStr(prefDlg, kVolNumberItem, (long)tempVolume);
				UnivSetSoundVolume(tempVolume, thisMac.hasSM3);
				HandleSoundMusicChange(tempVolume, true);
				DelayTicks(8);
				prefDlg->GetItems()[kSofterItem - 1].GetWidget()->DrawControl(surface);
			}
			break;
			
			case kLouderItem:
			UnivGetSoundVolume(&tempVolume, thisMac.hasSM3);	
			if (tempVolume < 7)
			{
				GetDialogItemRect(prefDlg, kLouderItem, &tempRect);
				DrawCIcon(surface, 1033, tempRect.left, tempRect.top);
				tempVolume++;
				if (tempVolume == 7)
					SetDialogNumToStr(prefDlg, kVolNumberItem, 11L);
				else
					SetDialogNumToStr(prefDlg, kVolNumberItem, tempVolume);
				UnivSetSoundVolume(tempVolume, thisMac.hasSM3);
				HandleSoundMusicChange(tempVolume, true);

				DelayTicks(8);

				prefDlg->GetItems()[kLouderItem - 1].GetWidget()->DrawControl(surface);
			}
			break;
			
			case kIdleMusicItem:
			wasIdle = !wasIdle;
			SetDialogItemValue(prefDlg, kIdleMusicItem, (short)wasIdle);
			if (wasIdle)
			{
				UnivGetSoundVolume(&tempVolume, thisMac.hasSM3);
				if (tempVolume != 0)
				{
					theErr = StartMusic();
					if (theErr != PLErrors::kNone)
					{
						YellowAlert(kYellowNoMusic, theErr);
						failedMusic = true;
					}
				}
			}
			else
				StopTheMusic();
			break;
			
			case kPlayMusicItem:
			wasPlay = !wasPlay;
			SetDialogItemValue(prefDlg, kPlayMusicItem, (short)wasPlay);
			break;
			
			case kSoundDefault:
			SoundDefaults(prefDlg);
			break;
		}
	}

	prefDlg->Destroy();
}

//--------------------------------------------------------------  DisplayDefaults

void DisplayDefaults (void)
{
	numNeighbors = 9;
	wasDepthPref = kSwitchIfNeeded;
	wasFade = true;
	wasScreen2 = false;
}

//--------------------------------------------------------------  FrameDisplayIcon

void FrameDisplayIcon (Dialog *theDialog, const PortabilityLayer::RGBAColor &color)
{
	Rect		theRect;
	
	switch (numNeighbors)
	{
		case 1:
		GetDialogItemRect(theDialog, kDisplay1Item, &theRect);
		break;
		
		case 3:
		GetDialogItemRect(theDialog, kDisplay3Item, &theRect);
		break;
		
		default:
		GetDialogItemRect(theDialog, kDisplay9Item, &theRect);
		break;
	}

	DrawSurface *surface = theDialog->GetWindow()->GetDrawSurface();

	surface->SetForeColor(color);
	
	theRect.left -= 3;
	theRect.top += 0;
	theRect.right += 3;
	theRect.bottom -= 1;
	surface->FrameRect(theRect);
	InsetRect(&theRect, 1, 1);
	surface->FrameRect(theRect);
}

//--------------------------------------------------------------  DisplayUpdate

void DisplayUpdate (Dialog *theDialog)
{
	DrawDefaultButton(theDialog);
	
	SetDialogItemValue(theDialog, kDoColorFadeItem, (short)wasFade);
	SelectFromRadioGroup(theDialog, kCurrentDepth + wasDepthPref, 
			kCurrentDepth, k16Depth);
//	SetDialogItemValue(theDialog, kUseQDItem, (short)wasQD);
	SetDialogItemValue(theDialog, kUseScreen2Item, (short)wasScreen2);
	
	ForeColor(redColor);
	FrameDisplayIcon(theDialog, StdColors::Red());
	ForeColor(blackColor);
	FrameDialogItemC(theDialog, 8, kRedOrangeColor8);
	FrameDialogItemC(theDialog, 13, kRedOrangeColor8);
	FrameDialogItemC(theDialog, 14, kRedOrangeColor8);
}

//--------------------------------------------------------------  DisplayFilter

int16_t DisplayFilter(Dialog *dial, const TimeTaggedVOSEvent *evt)
{
	if (!evt)
		return -1;

	if (evt->IsKeyDownEvent())
	{
		switch (PackVOSKeyCode(evt->m_vosEvent.m_event.m_keyboardInputEvent))
		{
		case PL_KEY_SPECIAL(kEnter):
		case PL_KEY_NUMPAD_SPECIAL(kEnter):
			FlashDialogButton(dial, kOkayButton);
			return kOkayButton;

		case PL_KEY_SPECIAL(kEscape):
			FlashDialogButton(dial, kCancelButton);
			return kCancelButton;

		case PL_KEY_SPECIAL(kLeftArrow):
			switch (numNeighbors)
			{
			case 1:
				return kDisplay9Item;

			case 3:
				return kDisplay1Item;

			case 9:
				return kDisplay3Item;

			default:
				return -1;
			}
			break;

		case PL_KEY_SPECIAL(kRightArrow):
			switch (numNeighbors)
			{
			case 1:
				return kDisplay3Item;

			case 3:
				return kDisplay9Item;

			case 9:
				return kDisplay1Item;

			default:
				return -1;
			}
			break;

		case PL_KEY_SPECIAL(kUpArrow):
			switch (wasDepthPref)
			{
			case kSwitchIfNeeded:
				return k16Depth;

			case kSwitchTo256Colors:
				return kCurrentDepth;

			case kSwitchTo16Grays:
				return k256Depth;

			default:
				return -1;
			}
			break;

		case PL_KEY_SPECIAL(kDownArrow):
			switch (wasDepthPref)
			{
			case kSwitchIfNeeded:
				return k256Depth;

			case kSwitchTo256Colors:
				return k16Depth;

			case kSwitchTo16Grays:
				return kCurrentDepth;
			}
			break;

		case PL_KEY_ASCII('1'):
			return kDisplay1Item;

		case PL_KEY_ASCII('3'):
			return kDisplay3Item;

		case PL_KEY_ASCII('9'):
			return kDisplay9Item;

		case PL_KEY_ASCII('B'):
			return kDoColorFadeItem;

		case PL_KEY_ASCII('D'):
			FlashDialogButton(dial, kDispDefault);
			return kDispDefault;

		case PL_KEY_ASCII('R'):
			PL_NotYetImplemented_TODO("FixMe");	// GP: This looks like a bug

			FlashDialogButton(dial, kUseQDItem);
			return kUseScreen2Item;

		case PL_KEY_ASCII('U'):
			return kUseQDItem;

		default:
			return -1;
		}
	}

	return -1;
}

//--------------------------------------------------------------  DoDisplayPrefs

void DoDisplayPrefs (void)
{
	Dialog			*prefDlg;
	short			wasNeighbors;
	Boolean			leaving;
	
	BringUpDialog(&prefDlg, kDisplayPrefsDialID, nullptr);

	if (!thisMac.can8Bit)
	{
		MyDisableControl(prefDlg, kDoColorFadeItem);
		MyDisableControl(prefDlg, k256Depth);
	}
	if (!thisMac.can4Bit)
		MyDisableControl(prefDlg, k16Depth);
	if (thisMac.numScreens < 2)
		MyDisableControl(prefDlg, kUseScreen2Item);
	wasNeighbors = numNeighbors;
	wasFade = isDoColorFade;
	wasDepthPref = isDepthPref;
	wasScreen2 = isUseSecondScreen;
	leaving = false;

	DisplayUpdate(prefDlg);
	
	while (!leaving)
	{
		int16_t itemHit = prefDlg->ExecuteModal(DisplayFilter);
		switch (itemHit)
		{
			case kOkayButton:
			isDoColorFade = wasFade;
			isDepthPref = wasDepthPref;
			if (isUseSecondScreen != wasScreen2)
				nextRestartChange = true;
			isUseSecondScreen = wasScreen2;
			leaving = true;
			break;
			
			case kCancelButton:
			numNeighbors = wasNeighbors;
			leaving = true;
			break;
			
			case kDisplay1Item:
			FrameDisplayIcon(prefDlg, StdColors::White());
			numNeighbors = 1;
			FrameDisplayIcon(prefDlg, StdColors::Red());
			break;
			
			case kDisplay3Item:
			if (thisMac.screen.right > 512)
			{
				FrameDisplayIcon(prefDlg, StdColors::White());
				numNeighbors = 3;
				FrameDisplayIcon(prefDlg, StdColors::Red());
			}
			break;
			
			case kDisplay9Item:
			if (thisMac.screen.right > 512)
			{
				FrameDisplayIcon(prefDlg, StdColors::White());
				numNeighbors = 9;
				FrameDisplayIcon(prefDlg, StdColors::Red());
			}
			break;
			
			case kDoColorFadeItem:
			wasFade = !wasFade;
			SetDialogItemValue(prefDlg, kDoColorFadeItem, (short)wasFade);
			break;
			
			case kCurrentDepth:
			case k256Depth:
			case k16Depth:
			wasDepthPref = itemHit - kCurrentDepth;
			SelectFromRadioGroup(prefDlg, itemHit, kCurrentDepth, k16Depth);
			break;
			
			case kDispDefault:
			FrameDisplayIcon(prefDlg, StdColors::White());
			ForeColor(blackColor);
			DisplayDefaults();
			DisplayUpdate(prefDlg);
			break;
			
			case kUseQDItem:
//			wasQD = !wasQD;
//			SetDialogItemValue(prefDlg, kUseQDItem, (short)wasQD);
			break;
			
			case kUseScreen2Item:
			wasScreen2 = !wasScreen2;
			SetDialogItemValue(prefDlg, kUseScreen2Item, (short)wasScreen2);
			break;
		}
	}
	
	prefDlg->Destroy();
}

//--------------------------------------------------------------  SetAllDefaults

void SetAllDefaults (void)
{
	PLError_t		theErr;
								// Default brain settings
	willMaxFiles = 48;
	doZooms = true;
	doAutoDemo = true;
	doBackground = false;
	isHouseChecks = true;
	doPrettyMap = true;
	doBitchDialogs = true;
								// Default control settings
	PasStringCopy(PSTR("lf arrow"), leftName);
	PasStringCopy(PSTR("rt arrow"), rightName);
	PasStringCopy(PSTR("dn arrow"), batteryName);
	PasStringCopy(PSTR("up arrow"), bandName);
	theGlider.leftKey = PL_KEY_SPECIAL(kLeftArrow);
	theGlider.rightKey = PL_KEY_SPECIAL(kRightArrow);
	theGlider.battKey = PL_KEY_SPECIAL(kDownArrow);
	theGlider.bandKey = PL_KEY_SPECIAL(kUpArrow);
	theGlider.gamepadLeftKey = PL_KEY_GAMEPAD_BUTTON(kDPadLeft, 0);
	theGlider.gamepadRightKey = PL_KEY_GAMEPAD_BUTTON(kDPadRight, 0);
	theGlider.gamepadBandKey = PL_KEY_GAMEPAD_BUTTON(kFaceDown, 0);
	theGlider.gamepadBattKey = PL_KEY_GAMEPAD_BUTTON(kFaceLeft, 0);
	theGlider.gamepadFlipKey = PL_KEY_GAMEPAD_BUTTON(kFaceUp, 0);
	theGlider.gamepadFaceRightKey = PL_KEY_GAMEPAD_BUTTON(kRightBumper, 0);
	theGlider.gamepadFaceLeftKey = PL_KEY_GAMEPAD_BUTTON(kLeftBumper, 0);
	isEscPauseKey = false;
								// Default sound settings
	isPlayMusicIdle = true;
	isPlayMusicGame = true;
	UnivSetSoundVolume(3, thisMac.hasSM3);
	isSoundOn = true;
	if (!isMusicOn)
	{
		theErr = StartMusic();
		if (theErr != PLErrors::kNone)
		{
			YellowAlert(kYellowNoMusic, theErr);
			failedMusic = true;
		}
	}
								// Default display settings
	numNeighbors = 9;
	quickerTransitions = false;
	isDepthPref = kSwitchIfNeeded;
	isDoColorFade = true;
}

//--------------------------------------------------------------  FlashSettingsButton

void FlashSettingsButton (DrawSurface *surface, short who)
{
	#define		kNormalSettingsIcon		1010
	#define		kInvertedSettingsIcon	1014
	short		theID;
	
	theID = kInvertedSettingsIcon + who;
	DrawCIcon (surface, theID, prefButton[who].left + 4, prefButton[who].top + 4);
	DelayTicks(8);
	theID = kNormalSettingsIcon + who;
	DrawCIcon (surface, theID, prefButton[who].left + 4, prefButton[who].top + 4);
}

//--------------------------------------------------------------  UpdateSettingsMain

void UpdateSettingsMain (Dialog *theDialog)
{
	Str255		theStr;
	DrawSurface	*surface = theDialog->GetWindow()->GetDrawSurface();
	
	DrawDefaultButton(theDialog);
	
	GetIndString(theStr, 129, 1);
	DrawDialogUserText(theDialog, 7, theStr, false);
	GetIndString(theStr, 129, 2);
	DrawDialogUserText(theDialog, 8, theStr, false);
	GetIndString(theStr, 129, 3);
	DrawDialogUserText(theDialog, 9, theStr, false);
	GetIndString(theStr, 129, 4);
	DrawDialogUserText(theDialog, 10, theStr, false);
	
	ColorFrameRect(surface, prefButton[0], kRedOrangeColor8);
	ColorFrameRect(surface, prefButton[1], kRedOrangeColor8);
	ColorFrameRect(surface, prefButton[2], kRedOrangeColor8);
	ColorFrameRect(surface, prefButton[3], kRedOrangeColor8);
}

//--------------------------------------------------------------  PrefsFilter

int16_t PrefsFilter (Dialog *dial, const TimeTaggedVOSEvent *evt)
{
	short		i;
	Boolean		foundHit;

	if (!evt)
		return -1;

	if (evt->IsKeyDownEvent())
	{
		intptr_t packedKey = PackVOSKeyCode(evt->m_vosEvent.m_event.m_keyboardInputEvent);

		switch (packedKey)
		{
		case PL_KEY_SPECIAL(kEnter):
		case PL_KEY_NUMPAD_SPECIAL(kEnter):
			FlashDialogButton(dial, kOkayButton);
			return kOkayButton;

		case PL_KEY_ASCII('B'):
			return kBrainsButton;

		case PL_KEY_ASCII('C'):
			return kControlsButton;

		case PL_KEY_ASCII('d'):
			return kDisplayButton;

		case PL_KEY_ASCII('S'):
			return kSoundButton;

		default:
			return -1;
		}
	}
	else if (evt->IsLMouseDownEvent())
	{
		const Window *window = dial->GetWindow();
		const GpMouseInputEvent &mouseEvent = evt->m_vosEvent.m_event.m_mouseInputEvent;

		const Point testPt = Point::Create(mouseEvent.m_x - window->m_wmX, mouseEvent.m_y - window->m_wmY);

		int16_t hitCode = -1;

		for (i = 0; i < 4; i++)
		{
			if (prefButton[i].Contains(testPt))
				hitCode = kDisplayButton + i;
		}
		return hitCode;
	}

	return -1;
}

//--------------------------------------------------------------  DoSettingsMain

void DoSettingsMain (void)
{
	#define			kAllDefaultsButton		11
	Dialog			*prefDlg;
	int16_t			itemHit;
	Boolean			leaving;
	
	BringUpDialog(&prefDlg, kMainPrefsDialID, nullptr);

	DrawSurface *surface = prefDlg->GetWindow()->GetDrawSurface();
	
	GetDialogItemRect(prefDlg, kDisplayButton, &prefButton[0]);
	InsetRect(&prefButton[0], -4, -4);
	GetDialogItemRect(prefDlg, 4, &prefButton[1]);
	InsetRect(&prefButton[1], -4, -4);
	GetDialogItemRect(prefDlg, 5, &prefButton[2]);
	InsetRect(&prefButton[2], -4, -4);
	GetDialogItemRect(prefDlg, 6, &prefButton[3]);
	InsetRect(&prefButton[3], -4, -4);

	UpdateSettingsMain(prefDlg);
	
	leaving = false;
	nextRestartChange = false;
	
	while (!leaving)
	{
		int16_t selectedItem = prefDlg->ExecuteModal(PrefsFilter);

		switch (selectedItem)
		{
			case kOkayButton:
			leaving = true;
			break;
			
			case kDisplayButton:
			FlashSettingsButton(surface, 0);
			DoDisplayPrefs();
			SetGraphicsPort(&prefDlg->GetWindow()->m_surface);
			break;
			
			case kSoundButton:
			FlashSettingsButton(surface, 1);
			DoSoundPrefs();
			SetGraphicsPort(&prefDlg->GetWindow()->m_surface);
			FlushEvents(everyEvent, 0);
			break;
			
			case kControlsButton:
			FlashSettingsButton(surface, 2);
			DoControlPrefs();
			SetGraphicsPort(&prefDlg->GetWindow()->m_surface);
			break;
			
			case kBrainsButton:
			if ((OptionKeyDown()) && (!houseUnlocked))
			{
				houseUnlocked = true;
				changeLockStateOfHouse = true;
				saveHouseLocked = false;
			}
			FlashSettingsButton(surface, 3);
			DoBrainsPrefs();
			SetGraphicsPort(&prefDlg->GetWindow()->m_surface);
			break;
			
			case kAllDefaultsButton:
			SetAllDefaults();
			break;
		}
	}
	
	prefDlg->Destroy();
	
	if (nextRestartChange)
		BitchAboutChanges();
}

//--------------------------------------------------------------  BitchAboutChanges

void BitchAboutChanges (void)
{
	#define		kChangesEffectAlert	1040
	short		hitWhat;
	
//	CenterAlert(kChangesEffectAlert);
	hitWhat = PortabilityLayer::DialogManager::GetInstance()->DisplayAlert(kChangesEffectAlert, nullptr);
}
