
//============================================================================
//----------------------------------------------------------------------------
//								InterfaceInit.c
//----------------------------------------------------------------------------
//============================================================================


#include "Externs.h"
#include "Environ.h"
#include "Map.h"
#include "MenuManager.h"
#include "PLKeyEncoding.h"
#include "RectUtils.h"
#include "Tools.h"


#define kHandCursorID		128
#define kVertCursorID		129
#define kHoriCursorID		130
#define kDiagCursorID		131


extern	THandle<Rect>	mirrorRects;
extern	WindowPtr		mapWindow, toolsWindow, linkWindow;
extern	WindowPtr		menuWindow;
extern	Rect			shieldRect, boardSrcRect, localRoomsDest[];
extern	CursHandle		handCursorH, vertCursorH, horiCursorH;
extern	CursHandle		diagCursorH;
extern	Cursor			handCursor, vertCursor, horiCursor;
extern	Cursor			diagCursor;
extern	MenuHandle		appleMenu, gameMenu, optionsMenu, houseMenu;
extern	Point			shieldPt;
extern	long			incrementModeTime;
extern	UInt32			doubleTime;
extern	short			fadeInSequence[], idleMode;
extern	short			toolSelected, lastBackground, wasFlower, numExtraHouses;
extern	short			lastHighScore, maxFiles, willMaxFiles;
extern	Boolean			quitting, playing, fadeGraysOut;
extern	Boolean			houseOpen, newRoomNow, evenFrame, menusUp, demoGoing;
extern	Boolean			twoPlayerGame, paused, hasMirror, splashDrawn;


//==============================================================  Functions
//--------------------------------------------------------------  InitializeMenus

// The menus are loaded from disk and the menu bar set up and drawn.

void InitializeMenus (void)
{	
	appleMenu = GetMenu(kAppleMenuID);
	if (appleMenu == nil)
		RedAlert(kErrFailedResourceLoad);
	//AppendResMenu(appleMenu, 'DRVR');	// GP: We don't support this
	InsertMenu(appleMenu, 0);
	
	gameMenu = GetMenu(kGameMenuID);
	if (gameMenu == nil)
		RedAlert(kErrFailedResourceLoad);
	InsertMenu(gameMenu, 0);
	
	optionsMenu = GetMenu(kOptionsMenuID);
	if (optionsMenu == nil)
		RedAlert(kErrFailedResourceLoad);
	InsertMenu(optionsMenu, 0);
	
	menusUp = true;
	PortabilityLayer::MenuManager::GetInstance()->SetMenuVisible(true);
	
	houseMenu = GetMenu(kHouseMenuID);
	if (houseMenu == nil)
		RedAlert(kErrFailedResourceLoad);
	
	UpdateMenus(false);
}

//--------------------------------------------------------------  GetExtraCursors

// Extra cursors (custom cursors) like the "hand" and various room�
// editing cursors are loaded up.

void GetExtraCursors (void)
{
	handCursorH = GetCursor(kHandCursorID);
	if (handCursorH == nil)
		RedAlert(kErrFailedResourceLoad);
	handCursor = **handCursorH;
	
	vertCursorH = GetCursor(kVertCursorID);
	if (vertCursorH == nil)
		RedAlert(kErrFailedResourceLoad);
	vertCursor = **vertCursorH;
	
	horiCursorH = GetCursor(kHoriCursorID);
	if (horiCursorH == nil)
		RedAlert(kErrFailedResourceLoad);
	horiCursor = **horiCursorH;
	
	diagCursorH = GetCursor(kDiagCursorID);
	if (diagCursorH == nil)
		RedAlert(kErrFailedResourceLoad);
	diagCursor = **diagCursorH;
}

//--------------------------------------------------------------  VariableInit

// All the simple interface variables are intialized here - Booleans,�
// shorts, a few Rects, etc.

void VariableInit (void)
{
	short		i;
	
	shieldPt.h = 0;
	shieldPt.v = 0;
	shieldRect = thisMac.screen;
	
	menusUp = false;
	quitting = false;
	houseOpen = false;
	newRoomNow = false;
	playing = false;
	evenFrame = false;
	if (thisMac.isDepth == 8)
		fadeGraysOut = true;
	else
		fadeGraysOut = false;
	twoPlayerGame = false;
	paused = false;
	hasMirror = false;
	demoGoing = false;
//	scrapIsARoom = true;
	splashDrawn = false;
	
#ifndef COMPILEDEMO
//	SeeIfValidScrapAvailable(false);
#endif
	
	theGlider.which = kPlayer1;
	theGlider2.leftKey = PL_KEY_ASCII('A');
	theGlider2.rightKey = PL_KEY_ASCII('D');
	theGlider2.battKey = PL_KEY_ASCII('S');
	theGlider2.bandKey = PL_KEY_ASCII('W');
	theGlider2.gamepadLeftKey = PL_KEY_GAMEPAD_BUTTON(kDPadLeft, 1);
	theGlider2.gamepadRightKey = PL_KEY_GAMEPAD_BUTTON(kDPadRight, 1);
	theGlider2.gamepadBandKey = PL_KEY_GAMEPAD_BUTTON(kFaceDown, 1);
	theGlider2.gamepadBattKey = PL_KEY_GAMEPAD_BUTTON(kFaceLeft, 1);
	theGlider2.which = kPlayer2;
	
	theMode = kSplashMode;
	thisRoomNumber = 0;
	previousRoom = -1;
	toolSelected = kSelectTool;
	lastBackground = kBaseBackgroundID;
	wasFlower = RandomInt(kNumFlowers);
	lastHighScore = -1;
	idleMode = kIdleSplashMode;
	incrementModeTime = TickCount() + kIdleSplashTicks;
	willMaxFiles = maxFiles;
	numExtraHouses = 0;
	
	fadeInSequence[0] = 4;	// 4
	fadeInSequence[1] = 5;
	fadeInSequence[2] = 6;
	fadeInSequence[3] = 7;
	fadeInSequence[4] = 5;	// 5
	fadeInSequence[5] = 6;
	fadeInSequence[6] = 7;
	fadeInSequence[7] = 8;
	fadeInSequence[8] = 6;	// 6
	fadeInSequence[9] = 7;
	fadeInSequence[10] = 8;
	fadeInSequence[11] = 9;
	fadeInSequence[12] = 7;	// 7
	fadeInSequence[13] = 8;
	fadeInSequence[14] = 9;
	fadeInSequence[15] = 10;
	
	doubleTime = GetDblTime();
	
	mirrorRects = nil;
	mainWindow = nil;
	boardWindow = nil;
	mapWindow = nil;
	toolsWindow = nil;
	linkWindow = nil;
	coordWindow = nil;
	toolSrcMap = nil;
	nailSrcMap = nil;
	menuWindow = nil;
	
	houseRect = thisMac.screen;
	houseRect.bottom -= kScoreboardTall;
	if (houseRect.right > kMaxViewWidth)
		houseRect.right = kMaxViewWidth;
	if (houseRect.bottom > kMaxViewHeight)
		houseRect.bottom = kMaxViewHeight;
	
	playOriginH = (RectWide(&thisMac.screen) - kRoomWide) / 2;
	playOriginV = (RectTall(&thisMac.screen) - kTileHigh) / 2;
	
	for (i = 0; i < 9; i++)
	{
		QSetRect(&localRoomsDest[i], 0, 0, kRoomWide, kTileHigh);
		QOffsetRect(&localRoomsDest[i], playOriginH, playOriginV);
	}
	QOffsetRect(&localRoomsDest[kNorthRoom], 0, -kVertLocalOffset);
	QOffsetRect(&localRoomsDest[kNorthEastRoom], kRoomWide, -kVertLocalOffset);
	QOffsetRect(&localRoomsDest[kEastRoom], kRoomWide, 0);
	QOffsetRect(&localRoomsDest[kSouthEastRoom], kRoomWide, kVertLocalOffset);
	QOffsetRect(&localRoomsDest[kSouthRoom], 0, kVertLocalOffset);
	QOffsetRect(&localRoomsDest[kSouthWestRoom], -kRoomWide, kVertLocalOffset);
	QOffsetRect(&localRoomsDest[kWestRoom], -kRoomWide, 0);
	QOffsetRect(&localRoomsDest[kNorthWestRoom], -kRoomWide, -kVertLocalOffset);
}
