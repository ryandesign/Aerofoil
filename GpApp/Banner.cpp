//============================================================================
//----------------------------------------------------------------------------
//									Banner.c
//----------------------------------------------------------------------------
//============================================================================


#include "PLNumberFormatting.h"
#include "PLPasStr.h"
#include "PLStandardColors.h"
#include "Externs.h"
#include "Environ.h"
#include "FontFamily.h"
#include "FontManager.h"
#include "MainWindow.h"
#include "RectUtils.h"
#include "Room.h"
#include "Utilities.h"


#define kBannerPageTopPICT		1993
#define kBannerPageBottomPICT	1992
#define kBannerPageBottomMask	1991
#define kStarsRemainingPICT		1017
#define kStarRemainingPICT		1018


void DrawBanner (Point *);
void DrawBannerMessage (Point);


short		numStarsRemaining;
Boolean		bannerStarCountOn;

extern	Rect		justRoomsRect;
extern	Boolean		quickerTransitions, demoGoing, isUseSecondScreen;


//==============================================================  Functions
//--------------------------------------------------------------  DrawBanner
// Displays opening banner (when a new game is begun).  The banner looks�
// like a sheet of notebook paper.  The text printed on it is specified�
// by the author of the house.

void DrawBanner (Point *topLeft)
{
	Rect		wholePage, partPage, mapBounds;
	DrawSurface *tempMap;
	DrawSurface *tempMask;
	PLError_t	theErr;
	
	QSetRect(&wholePage, 0, 0, 330, 220);
	mapBounds = thisMac.screen;
	ZeroRectCorner(&mapBounds);
	CenterRectInRect(&wholePage, &mapBounds);
	topLeft->h = wholePage.left;
	topLeft->v = wholePage.top;
	partPage = wholePage;
	partPage.bottom = partPage.top + 190;	
	LoadScaledGraphic(workSrcMap, kBannerPageTopPICT, &partPage);
	
	partPage = wholePage;
	partPage.top = partPage.bottom - 30;
	mapBounds = partPage;
	ZeroRectCorner(&mapBounds);
	theErr = CreateOffScreenGWorld(&tempMap, &mapBounds, kPreferredPixelFormat);
	LoadGraphic(tempMap, kBannerPageBottomPICT);
	
	theErr = CreateOffScreenGWorld(&tempMask, &mapBounds, GpPixelFormats::kBW1);	
	LoadGraphic(tempMask, kBannerPageBottomMask);

	CopyMask((BitMap *)*GetGWorldPixMap(tempMap), 
			(BitMap *)*GetGWorldPixMap(tempMask), 
			(BitMap *)*GetGWorldPixMap(workSrcMap), 
			&mapBounds, &mapBounds, &partPage);

	DisposeGWorld(tempMap);
	DisposeGWorld(tempMask);
}

//--------------------------------------------------------------  CountStarsInHouse
// Goes through the current house and counts the total number of stars within.

short CountStarsInHouse (void)
{
	short		i, h, numRooms, numStars;
	housePtr	housePtr = *thisHouse;
	
	numStars = 0;
	
	numRooms = housePtr->nRooms;
	for (i = 0; i < numRooms; i++)
	{
		if (housePtr->rooms[i].suite != kRoomIsEmpty)
			for (h = 0; h < kMaxRoomObs; h++)
			{
				if (housePtr->rooms[i].objects[h].what == kStar)
					numStars++;
			}
	}
	
	return (numStars);
}

//--------------------------------------------------------------  DrawBannerMessage

// This function prints the author's message acorss the notebook paper banner.

void DrawBannerMessage (Point topLeft)
{
	Str255		bannerStr, subStr;
	short		count;

	DrawSurface *wasGWorld = GetGraphicsPort();

	SetGraphicsPort(workSrcMap);

	PasStringCopy((*thisHouse)->banner, bannerStr);

	workSrcMap->SetApplicationFont(12, PortabilityLayer::FontFamilyFlag_Bold);
	workSrcMap->SetForeColor(StdColors::Black());
	count = 0;
	do
	{
		GetLineOfText(bannerStr, count, subStr);
		workSrcMap->DrawString(Point::Create(topLeft.h + 16, topLeft.v + 32 + (count * 20)), subStr, true);
		count++;
	}
	while (subStr[0] > 0);
	
	if (bannerStarCountOn)
	{
		if (numStarsRemaining != 1)
			GetLocalizedString(1, bannerStr);
		else
			GetLocalizedString(2, bannerStr);
		
		NumToString((long)numStarsRemaining, subStr);
		PasStringConcat(bannerStr, subStr);
		
		if (numStarsRemaining != 1)
			GetLocalizedString(3, subStr);
		else
			GetLocalizedString(4, subStr);
		PasStringConcat(bannerStr, subStr);

		workSrcMap->SetForeColor(StdColors::Red());
		workSrcMap->DrawString(Point::Create(topLeft.h + 16, topLeft.v + 164), bannerStr, true);

		GetLocalizedString(5, subStr);
		workSrcMap->DrawString(Point::Create(topLeft.h + 16, topLeft.v + 180), subStr, true);
	}
	workSrcMap->SetForeColor(StdColors::Black());

	SetGraphicsPort(wasGWorld);
}

//--------------------------------------------------------------  BringUpBanner
// Handles bringing up displaying and disposing of the banner.

void BringUpBanner (void)
{
	Rect		wholePage;
	Point		topLeft;
	
	DrawBanner(&topLeft);
	DrawBannerMessage(topLeft);

	DumpScreenOn(&justRoomsRect);

//	if (quickerTransitions)
//		DissBitsChunky(&justRoomsRect);		// was workSrcRect
//	else
//		DissBits(&justRoomsRect);
	QSetRect(&wholePage, 0, 0, 330, 220);
	QOffsetRect(&wholePage, topLeft.h, topLeft.v);
	
	CopyBits((BitMap *)*GetGWorldPixMap(backSrcMap), 
			(BitMap *)*GetGWorldPixMap(workSrcMap), 
			&wholePage, &wholePage, srcCopy);

	
	if (demoGoing)
		WaitForInputEvent(4);
	else
		WaitForInputEvent(15);
	
//	if (quickerTransitions)
//		DissBitsChunky(&justRoomsRect);
//	else
//		DissBits(&justRoomsRect);
}

//--------------------------------------------------------------  DisplayStarsRemaining
// This brings up a small message indicating the number of stars remaining�
// in a house.  It comes up when the player gets a star (the game is paused�
// and the user informed as to how many remain).

void DisplayStarsRemaining (void)
{
	Rect		src, bounds;
	Str255		theStr;
	DrawSurface *surface = mainWindow->GetDrawSurface();

	QSetRect(&bounds, 0, 0, 256, 64);
	CenterRectInRect(&bounds, &thisMac.screen);
	QOffsetRect(&bounds, -thisMac.screen.left, -thisMac.screen.top);
	src = bounds;
	InsetRect(&src, 64, 32);

	surface->SetApplicationFont(12, PortabilityLayer::FontFamilyFlag_Bold);

	NumToString((long)numStarsRemaining, theStr);
	
	QOffsetRect(&bounds, 0, -20);
	if (numStarsRemaining < 2)
		LoadScaledGraphic(surface, kStarRemainingPICT, &bounds);
	else
	{
		LoadScaledGraphic(surface, kStarsRemainingPICT, &bounds);
		const Point textPoint = Point::Create(bounds.left + 102 - (surface->MeasureString(theStr) / 2), bounds.top + 23);
		ColorText(surface, textPoint, theStr, 4L);
	}
	
	DelayTicks(60);
	if (WaitForInputEvent(30))
		RestoreEntireGameScreen();
	CopyRectWorkToMain(&bounds);
}
