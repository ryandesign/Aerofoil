#include "QDState.h"

#include "PLQDraw.h"
#include "QDStandardPalette.h"

namespace PortabilityLayer
{
	QDState::QDState()
		: m_fontFamily(nullptr)
		, m_fontSize(12)
		, m_fontVariationFlags(0)
		, m_foreResolvedColor16(0)
		, m_backResolvedColor16(0)
		, m_foreResolvedColor8(0)
		, m_backResolvedColor8(0)
		, m_isForeResolved16(false)
		, m_isBackResolved16(false)
		, m_isForeResolved8(false)
		, m_isBackResolved8(false)
		, m_clipRect(Rect::Create(INT16_MIN, INT16_MIN, INT16_MAX, INT16_MAX))
		, m_penInvert(false)
		, m_penMask(false)
		, m_havePattern8x8(false)
	{
		m_backUnresolvedColor.r = m_backUnresolvedColor.g = m_backUnresolvedColor.b = m_backUnresolvedColor.a = 255;
		m_foreUnresolvedColor.r = m_foreUnresolvedColor.g = m_foreUnresolvedColor.b = 0;
		m_foreUnresolvedColor.a = 255;
		m_penPos.h = m_penPos.v = 0;

		memset(m_pattern8x8, 0, sizeof(m_pattern8x8));
	}

	void QDState::SetForeColor(const RGBAColor &color)
	{
		m_foreUnresolvedColor = color;
		m_isForeResolved16 = false;
		m_isForeResolved8 = false;
	}

	void QDState::SetBackColor(const RGBAColor &color)
	{
		m_backUnresolvedColor = color;
		m_isBackResolved16 = false;
		m_isBackResolved8 = false;
	}

	const RGBAColor &QDState::GetForeColor() const
	{
		return m_foreUnresolvedColor;
	}

	const RGBAColor &QDState::GetBackColor() const
	{
		return m_backUnresolvedColor;
	}

	uint8_t QDState::ResolveForeColor8(const RGBAColor *palette, unsigned int numColors)
	{
		return ResolveColor8(m_foreUnresolvedColor, m_foreResolvedColor8, m_isForeResolved8, palette, numColors);
	}

	uint8_t QDState::ResolveBackColor8(const RGBAColor *palette, unsigned int numColors)
	{
		return ResolveColor8(m_backUnresolvedColor, m_backResolvedColor8, m_isBackResolved8, palette, numColors);
	}

	uint8_t QDState::ResolveColor8(const RGBAColor &color, uint8_t &cached, bool &isCached, const RGBAColor *palette, unsigned int numColors)
	{
		if (isCached)
			return cached;

		if (palette)
		{
			PL_NotYetImplemented();
			return 0;
		}
		else
		{
			const uint8_t resolvedColor = StandardPalette::GetInstance()->MapColorLUT(color);

			isCached = true;
			cached = resolvedColor;

			return resolvedColor;
		}
	}

	void QDState::SetPenPattern8x8(const uint8_t *pattern)
	{
		if (!pattern)
		{
			m_havePattern8x8 = false;
			return;
		}

		bool isSolid = true;
		for (int i = 0; i < 8; i++)
		{
			if (pattern[i] != 0xff)
			{
				isSolid = false;
				break;
			}
		}

		if (isSolid)
			m_havePattern8x8 = false;
		else
		{
			m_havePattern8x8 = true;
			memcpy(m_pattern8x8, pattern, sizeof(m_pattern8x8));
		}
	}

	const uint8_t *QDState::GetPattern8x8() const
	{
		if (m_havePattern8x8)
			return m_pattern8x8;

		return nullptr;
	}
}