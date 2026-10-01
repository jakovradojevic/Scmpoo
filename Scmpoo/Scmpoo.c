/* Allow unsafe CRT functions on Visual C++ 2005 and higher. */
#if _MSC_VER >= 1400
#define _CRT_SECURE_NO_DEPRECATE
#endif

#define STRICT

/* UpdateLayeredWindow requires Windows 2000 or later. */
#if defined(_WIN32) && !defined(_WIN32_WINNT)
#define _WIN32_WINNT 0x0500
#endif

#include <stdlib.h>
#include <time.h>
#include <Windows.h>
#include <CommCtrl.h>
#include <MMSystem.h>
#include <ShellAPI.h>

/* GetActiveWindow is made local to current thread in 32-bit Windows. The function of global scope is now GetForegroundWindow. */
#ifdef _WIN32
#define GetActiveWindow GetForegroundWindow
#endif

typedef struct spriteinfo {
    HBITMAP bitmaps[2];
    int x;
    int y;
    int width;
    int height;
} spriteinfo;

typedef struct resourceinfo {
    int resource;
    WORD flags;
    spriteinfo info;
} resourceinfo;

typedef struct windowinfo {
    HWND window;
    RECT rect;
    BYTE padding[66]; /* Unused. */
} windowinfo;

int paletteSearchMaxIndexUnused = 245; /* Palette search maximum index (unused). */
resourceinfo resourceList[32] = { /* Resource list. Normal 101-111, alien 112-122. */
    {101, 1, {{NULL, NULL}, 0, 0, 0, 0}},
    {102, 1, {{NULL, NULL}, 0, 0, 0, 0}},
    {103, 1, {{NULL, NULL}, 0, 0, 0, 0}},
    {104, 1, {{NULL, NULL}, 0, 0, 0, 0}},
    {105, 1, {{NULL, NULL}, 0, 0, 0, 0}},
    {106, 1, {{NULL, NULL}, 0, 0, 0, 0}},
    {107, 1, {{NULL, NULL}, 0, 0, 0, 0}},
    {108, 1, {{NULL, NULL}, 0, 0, 0, 0}},
    {109, 1, {{NULL, NULL}, 0, 0, 0, 0}},
    {110, 1, {{NULL, NULL}, 0, 0, 0, 0}},
    {111, 1, {{NULL, NULL}, 0, 0, 0, 0}},
    {112, 1, {{NULL, NULL}, 0, 0, 0, 0}},
    {113, 1, {{NULL, NULL}, 0, 0, 0, 0}},
    {114, 1, {{NULL, NULL}, 0, 0, 0, 0}},
    {115, 1, {{NULL, NULL}, 0, 0, 0, 0}},
    {116, 1, {{NULL, NULL}, 0, 0, 0, 0}},
    {117, 1, {{NULL, NULL}, 0, 0, 0, 0}},
    {118, 1, {{NULL, NULL}, 0, 0, 0, 0}},
    {119, 1, {{NULL, NULL}, 0, 0, 0, 0}},
    {120, 1, {{NULL, NULL}, 0, 0, 0, 0}},
    {121, 1, {{NULL, NULL}, 0, 0, 0, 0}},
    {122, 1, {{NULL, NULL}, 0, 0, 0, 0}},
    {123, 1, {{NULL, NULL}, 0, 0, 0, 0}}, /* VR cursor graze, bite stages 0-1. */
    {124, 1, {{NULL, NULL}, 0, 0, 0, 0}}, /* VR cursor graze, bite stages 2-3. */
    {125, 1, {{NULL, NULL}, 0, 0, 0, 0}} /* 108 upside down, pupils split (acid trip). */
};
resourceinfo flippedResourceList[32] = {0}; /* Resource list storing flipped images. */
WORD normalActionTableGravityAlwaysOff[80] = { /* Normal action table (option "Gravity always on" disabled). */
    11, 11, 7, 7,
    11, 11, 7, 7,
    11, 11, 7, 7,
    11, 11, 7, 7,
    11, 11, 7, 7,
    11, 11, 7, 7,
    11, 11, 7, 7,
    17, 20, 17, 20,
    17, 20, 17, 20,
    17, 20, 53, 53,
    53, 164, 164, 164,
    58, 58, 58, 45,
    45, 45, 43, 43,
    43, 62, 62, 62,
    65, 65, 65, 69,
    69, 69, 13, 13,
    13, 51, 51, 51,
    15, 15, 35, 35,
    47, 47, 49, 49,
    75, 75, 9, 9
};
WORD normalActionTableGravityAlwaysOn[80] = { /* Normal action table (option "Gravity always on" enabled). */
    11, 11, 7, 7,
    11, 11, 7, 7,
    11, 11, 7, 7,
    11, 11, 7, 7,
    11, 11, 7, 7,
    11, 11, 7, 7,
    11, 11, 7, 7,
    17, 20, 17, 20,
    17, 20, 17, 20,
    17, 20, 53, 53,
    53, 164, 164, 164,
    58, 58, 58, 45,
    45, 45, 43, 43,
    43, 62, 62, 62,
    65, 65, 65, 69,
    69, 69, 13, 13,
    13, 51, 51, 51,
    15, 15, 35, 35,
    47, 47, 49, 49,
    75, 75, 9, 9
};
WORD specialActionTable[8] = { /* Special action table. */
    116, 121, 126, 147,
    128, 135, 142, 155
};
int facingDirection = 1; /* Facing direction. 1 = left, -1 = right */
int facingDirectionSub = 1; /* Facing direction (sub). 1 = left, -1 = right */

/* Backward-compatible aliases for decompiler-generated names. */
#define facingDirection facingDirection
#define facingDirectionSub facingDirectionSub
WORD blinkAnimationFrames[6][8] = { /* Blink animations. */
    {7, 8, 7, 6, 7, 8, 7, 6},
    {32, 33, 32, 31, 32, 33, 32, 31},
    {74, 75, 74, 73, 74, 75, 74, 73},
    {79, 80, 79, 78, 79, 80, 79, 78},
    {82, 83, 82, 81, 82, 83, 82, 81},
    {35, 36, 35, 34, 35, 36, 35, 34}
};
WORD hangOnWindowTopEdgeAnimationFrames[2][4] = { /* Hang on window top edge animations. */
    {42, 43, 42, 44},
    {46, 47, 46, 47}
};
WORD collisionAnimationFramesWithHeightOffset[20] = { /* Collision animation with obsolete height offset. */
    62, 63, 63, 64, 64, 65, 65, 66, 66, 66,
    0, 10, 17, 21, 22, 21, 17, 10, 0, 0
};
WORD yawnAnimationFrames[11] = { /* Yawn animation. */
    37, 38, 39, 39, 39, 38, 37, 3, 37, 3, 0
};
WORD baaAnimationFrames[8] = { /* Baa animation. */
    71, 72, 71, 72, 71, 72, 3, 0
};
WORD sneezeAnimationFrames[13] = { /* Sneeze animation. */
    107, 108, 109, 109, 3, 3, 3, 110, 111, 110, 111, 3, 0
};
WORD amazedAnimationFrames[6] = { /* Amazed animation. */
    50, 51, 50, 51, 3, 0
};
WORD eatAnimationFrames[35] = { /* Eat animation (flowers). */
    58, 150, 60, 61, 60, 61, 60, 61, 58, 151, 60, 61, 60, 61, 60, 61, 2, 58, 152, 60, 61, 60, 61, 60, 61, 58, 153, 60, 61, 60, 61, 60, 61, 3, 0
};
/* Markers in cursorGrazeAnimationFrames: bite the prop down to stage 1-3, or swallow the rest. */
#define CURSOR_GRAZE_BITE_1 1001
#define CURSOR_GRAZE_BITE_GONE 1004
WORD cursorGrazeAnimationFrames[35] = { /* Graze spinning VR cursor prop (123.bmp/124.bmp, sprite 352 + stage * 8 + spin). */
    58, 1001, 60, 61, 60, 61, 60, 61, 58, 1002, 60, 61, 60, 61, 60, 61, 2, 58, 1003, 60, 61, 60, 61, 60, 61, 58, 1004, 60, 61, 60, 61, 60, 61, 3, 0
};
WORD burnAnimationFrames[34] = { /* Burn animation. */
    134, 134, 134, 134, 134, 134, 134, 134, 135, 136, 137, 138, 137, 138, 137, 138, 137, 138, 137, 138, 139, 140, 141, 142, 143, 144, 145, 144, 145, 144, 145, 144, 145, 0
};
WORD rollOverAnimationFrames[13] = { /* Roll over animation (not used). */
    3, 93, 99, 100, 99, 100, 99, 100, 99, 100, 95, 3, 0
};
WORD getUpAnimationFramesLeft[8] = { /* Get up animation (left). */
    48, 48, 48, 49, 13, 12, 3, 0
};
WORD getUpAnimationFramesRight[8] = { /* Get up animation (right). */
    48, 48, 48, 49, 13, 14, 3, 0
};
WORD merry2AnimationFrames[28] = { /* Merry 2 animation. */
    130, 130, 130, 130, 130, 129, 129, 128, 128, 127, 127, 127, 6, 6, 6, 6, 7, 8, 7, 6, 7, 8, 7, 6, 6, 6, 6, 0
};
WORD burnBathtubSplashAnimationFrames[5] = { /* Burn bathtub splash animation. */
    147, 148, 147, 146, 0
};
WORD burnGetOutOfBathtubAnimationFrames[55] = { /* Burn get out of bathtub animation. */
    169, 169, 169, 169, 169, 169, 169, 169, 170, 171, 170, 169, 170, 171, 170, 169, 169, 169, 169, 81, 81, 81, 81, 81, 81, 81, 81, 85, 85, 85, 85, 85, 85, 85, 85, 34, 34, 34, 34, 35, 36, 35, 34, 35, 36, 35, 34, 34, 34, 10, 10, 9, 9, 3, 0
};
WORD blushAnimationFrames[12] = { /* Blush animation. */
    3, 127, 128, 129, 130, 130, 130, 129, 128, 127, 127, 0
};
WORD rollAnimationFrames[9] = { /* Roll animation. */
    119, 120, 121, 122, 123, 124, 125, 126, 0
};
WORD spinAnimationFrames[8] = { /* Spin animation. 0-3: face, 4-7: back */
    3, 9, 10, 11, 2, 14, 13, 12
};
/* Backward-compatible aliases for decompiler-generated names. */
#define paletteSearchMaxIndexUnused paletteSearchMaxIndexUnused
#define resourceList resourceList
#define flippedResourceList flippedResourceList
#define normalActionTableGravityAlwaysOff normalActionTableGravityAlwaysOff
#define normalActionTableGravityAlwaysOn normalActionTableGravityAlwaysOn
#define specialActionTable specialActionTable
#define blinkAnimationFrames blinkAnimationFrames
#define hangOnWindowTopEdgeAnimationFrames hangOnWindowTopEdgeAnimationFrames
#define collisionAnimationFramesWithHeightOffset collisionAnimationFramesWithHeightOffset
#define yawnAnimationFrames yawnAnimationFrames
#define baaAnimationFrames baaAnimationFrames
#define sneezeAnimationFrames sneezeAnimationFrames
#define amazedAnimationFrames amazedAnimationFrames
#define eatAnimationFrames eatAnimationFrames
#define cursorGrazeAnimationFrames cursorGrazeAnimationFrames
#define burnAnimationFrames burnAnimationFrames
#define rollOverAnimationFrames rollOverAnimationFrames
#define getUpAnimationFramesLeft getUpAnimationFramesLeft
#define getUpAnimationFramesRight getUpAnimationFramesRight
#define merry2AnimationFrames merry2AnimationFrames
#define burnBathtubSplashAnimationFrames burnBathtubSplashAnimationFrames
#define burnGetOutOfBathtubAnimationFrames burnGetOutOfBathtubAnimationFrames
#define blushAnimationFrames blushAnimationFrames
#define rollAnimationFrames rollAnimationFrames
#define spinAnimationFrames spinAnimationFrames

WORD cursorPositionChanged = 0; /* Has cursor position changed in current timer period? */
int cursorScreenX = 0; /* Cursor position with respect to screen, X-coordinate */
int cursorScreenY = 0; /* Cursor position with respect to screen, Y-coordinate */

#define cursorPositionChanged cursorPositionChanged
#define cursorScreenX cursorScreenX
#define cursorScreenY cursorScreenY
WORD draggingScreenMateWindow = 0; /* Dragging Screen Mate window? */
WORD destroyScreenMateOnRightDoubleClick = 0; /* Destroy Screen Mate window by right double-click? */
WORD unused_A7A2 = 0; /* Unused. */
RECT screenMateWindowRect = {0, 0, 0, 0}; /* Screen Mate window rectangle. */
WORD doNotClearWindowOnPaint = 0; /* Not to clear window on WM_PAINT? */
POINT cursorPosition = {0, 0}; /* Current cursor position. */
WORD doNotClearSubwindowOnPaint = 0; /* Not to clear window on WM_PAINT? (sub) */
HBITMAP doubleBufferMain[2] = {NULL, NULL}; /* Double buffer. */
HBITMAP spriteRenderTargetMain = NULL; /* Sprite render target. */
HBITMAP spriteColourBitmapMain = NULL; /* Sprite colour image for current frame. */
HBITMAP spriteMaskBitmapMain = NULL; /* Sprite mask image for current frame. */

#define draggingScreenMateWindow draggingScreenMateWindow
#define destroyScreenMateOnRightDoubleClick destroyScreenMateOnRightDoubleClick
#define unused_A7A2 unused_A7A2
#define screenMateWindowRect screenMateWindowRect
#define doNotClearWindowOnPaint doNotClearWindowOnPaint
#define cursorPosition cursorPosition
#define doNotClearSubwindowOnPaint doNotClearSubwindowOnPaint
#define doubleBufferMain doubleBufferMain
#define spriteRenderTargetMain spriteRenderTargetMain
#define spriteColourBitmapMain spriteColourBitmapMain
#define spriteMaskBitmapMain spriteMaskBitmapMain
int spriteXInResourceImageCurrentFrame = 0; /* Sprite X-coordinate on resource image for current frame. */
int spriteYInResourceImageCurrentFrame = 0; /* Sprite Y-coordinate on resource image for current frame. */
int spriteXInResourceImagePreviousFrame = 0; /* Sprite X-coordinate on resource image for previous frame. */
int spriteYInResourceImagePreviousFrameUnused = 0; /* Sprite Y-coordinate on resource image for previous frame (unused). */
WORD currentSpriteFramebufferIndex = 0; /* Current framebuffer index. */
WORD renderOrUpdateWindowFlag = 0; /* 0 to render sprite; 1 to update window. */
WORD unused_A7D4 = 0; /* Unused. */
HBITMAP spriteColourBitmapPreviousFrame = NULL; /* Sprite colour image for previous frame. */
int screenXCurrentFrame = 0; /* Screen X-coordinate for current frame. */
int screenYCurrentFrame = 0; /* Screen Y-coordinate for current frame. */
int spriteWidthCurrentFrame = 0; /* Sprite width for current frame. */
int spriteHeightCurrentFrame = 0; /* Sprite height for current frame. */
int updateAreaRectXCurrentFrame = 0; /* Update area rectangle X-coordinate for current frame. */
int updateAreaRectYCurrentFrame = 0; /* Update area rectangle Y-coordinate for current frame. */
int updateAreaRectWidthCurrentFrame = 0; /* Update area rectangle width for current frame. */
int updateAreaRectHeightCurrentFrame = 0; /* Update area rectangle height for current frame. */
int updateAreaRectXPreviousFrame = 0; /* Update area rectangle X-coordinate for previous frame. */
int updateAreaRectYPreviousFrame = 0; /* Update area rectangle Y-coordinate for previous frame. */
int updateAreaRectWidthPreviousFrame = 0; /* Update area rectangle width for previous frame. */
int updateAreaRectHeightPreviousFrame = 0; /* Update area rectangle height for previous frame. */
int screenXPreviousFrame = 0; /* Screen X-coordinate for previous frame. */
int screenYPreviousFrame = 0; /* Screen Y-coordinate for previous frame. */
int spriteWidthPreviousFrame = 0; /* Sprite width for previous frame. */
int spriteHeightPreviousFrame = 0; /* Sprite height for previous frame. */
WORD unused_A7FA = 0; /* Current frame rectangle and previous frame rectangle have no intersecion? (unused) */

#define spriteXInResourceImageCurrentFrame spriteXInResourceImageCurrentFrame
#define spriteYInResourceImageCurrentFrame spriteYInResourceImageCurrentFrame
#define spriteXInResourceImagePreviousFrame spriteXInResourceImagePreviousFrame
#define spriteYInResourceImagePreviousFrameUnused spriteYInResourceImagePreviousFrameUnused
#define currentSpriteFramebufferIndex currentSpriteFramebufferIndex
#define renderOrUpdateWindowFlag renderOrUpdateWindowFlag
#define unused_A7D4 unused_A7D4
#define spriteColourBitmapPreviousFrame spriteColourBitmapPreviousFrame
#define screenXCurrentFrame screenXCurrentFrame
#define screenYCurrentFrame screenYCurrentFrame
#define spriteWidthCurrentFrame spriteWidthCurrentFrame
#define spriteHeightCurrentFrame spriteHeightCurrentFrame
#define updateAreaRectXCurrentFrame updateAreaRectXCurrentFrame
#define updateAreaRectYCurrentFrame updateAreaRectYCurrentFrame
#define updateAreaRectWidthCurrentFrame updateAreaRectWidthCurrentFrame
#define updateAreaRectHeightCurrentFrame updateAreaRectHeightCurrentFrame
#define updateAreaRectXPreviousFrame updateAreaRectXPreviousFrame
#define updateAreaRectYPreviousFrame updateAreaRectYPreviousFrame
#define updateAreaRectWidthPreviousFrame updateAreaRectWidthPreviousFrame
#define updateAreaRectHeightPreviousFrame updateAreaRectHeightPreviousFrame
#define screenXPreviousFrame screenXPreviousFrame
#define screenYPreviousFrame screenYPreviousFrame
#define spriteWidthPreviousFrame spriteWidthPreviousFrame
#define spriteHeightPreviousFrame spriteHeightPreviousFrame
#define unused_A7FA unused_A7FA
WORD gravityEnabled = 0; /* Is gravity enabled? */
WORD collisionEnabled = 0; /* Is collision with visible window enabled? */
int spriteX = 0; /* Current X-coordinate. */
int spriteY = 0; /* Current Y-coordinate. */

#define spriteX spriteX
#define spriteY spriteY
int spriteIndex = 0; /* Sprite index. */
int verticalSpeed = 0; /* Vertical speed. */
int horizontalSpeed = 0; /* Horizontal speed. */
int yCoordinateMemory = 0; /* Y-coordinate memory. */
int spriteXSub = 0; /* Current X-coordinate (sub). */
int spriteYSub = 0; /* Current Y-coordinate (sub). */
int spriteIndexSub = 0; /* Sprite index (sub). */
HWND landingTargetWindow = NULL; /* Active window or window to land on. */
RECT landingTargetWindowRect = {0L, 0L, 0L, 0L}; /* Rectangle of active window or window to land on. */

#define spriteIndex spriteIndex
#define verticalSpeed verticalSpeed
#define horizontalSpeed horizontalSpeed
#define yCoordinateMemory yCoordinateMemory
#define spriteXSub spriteXSub
#define spriteYSub spriteYSub
#define spriteIndexSub spriteIndexSub
#define landingTargetWindow landingTargetWindow
#define landingTargetWindowRect landingTargetWindowRect
int animationFrameCounter = 0; /* Animation frame counter. */
int randomDurationCounter = 0; /* Random duration period counter. */
int randomCaseNumberForAction = 0; /* Random case number for action. */
WORD unusedA82C = 0; /* Unused. */
HGLOBAL waveResourceHandle = NULL; /* Global handle for holding WAVE resource in memory. */
#define waveResourceHandle waveResourceHandle
#define animationFrameCounter animationFrameCounter
#define randomDurationCounter randomDurationCounter

#define randomCaseNumberForAction randomCaseNumberForAction
#define unusedA82C unusedA82C

int currentTimeHour = 0; /* Current time hour. */
int remainingChimeTimes = 0; /* Remaining times for chime. */
DWORD tickCount = 0; /* Tick count. */

#define currentTimeHour currentTimeHour
#define remainingChimeTimes remainingChimeTimes
#define dword_A834 tickCount
int timeCheckPeriodCounter = 0; /* Time check period counter. */
int framePeriodCounter = 0; /* Frame period counter. */
int targetXWindowEdgeAttachment = 0; /* Target X-coordinate for window edge attachment. */
int targetYWindowEdgeAttachment = 0; /* Target Y-coordinate for window edge attachment. */
WORD bounceWhenFalling = 0; /* Bounce when falling? */
int fallActionCaseNumber = 0; /* Case number for fall action. */
int collisionVerticalSpeedUnused = 0; /* Collision vertical speed (unused). */
int collisionSpinFrameCounterUnused = 0; /* Collision spin frame counter (unused). */
WORD collisionAnimationFrameIndex = 0; /* Collision animation frame index. */
int knownInstanceListUpdatePeriodCounter = 0; /* Known instance list update period counter. */

HBITMAP doubleBufferSub[2] = {NULL, NULL}; /* Double buffer (sub). */
HBITMAP spriteRenderTargetSub = NULL; /* Sprite render target (sub). */
HBITMAP spriteColourBitmapSubCurrentFrame = NULL; /* Sprite colour image for current frame (sub). */
HBITMAP spriteMaskBitmapSubCurrentFrame = NULL; /* Sprite mask image for current frame (sub). */
HBITMAP fadeOutColourBitmapSub = NULL; /* Fade out processed colour image (sub). */
HBITMAP fadeOutMaskBitmapSub = NULL; /* Fade out processed mask image (sub). */
int spriteXInResourceImageSubCurrentFrame = 0; /* Sprite X-coordinate on resource image for current frame (sub). */
int spriteYInResourceImageSubCurrentFrame = 0; /* Sprite Y-coordinate on resource image for current frame (sub). */
int spriteXInResourceImageSubPreviousFrame = 0; /* Sprite X-coordinate on resource image for previous frame (sub). */
int spriteYInResourceImageSubPreviousFrameUnused = 0; /* Sprite Y-coordinate on resource image for previous frame (sub) (unused). */
WORD currentSpriteFramebufferIndexSub = 0; /* Current framebuffer index (sub). */
WORD renderOrUpdateWindowFlagSub = 0; /* 0 to render sprite; 1 to update window (sub). */
WORD unused_A872 = 0; /* Unused. */
HBITMAP spriteColourBitmapSubPreviousFrame = NULL; /* Sprite colour image for previous frame (sub). */
int screenXSubCurrentFrame = 0; /* Screen X-coordinate for current frame (sub). */
int screenYSubCurrentFrame = 0; /* Screen Y-coordinate for current frame (sub). */
int spriteWidthSubCurrentFrame = 0; /* Sprite width for current frame (sub). */
int spriteHeightSubCurrentFrame = 0; /* Sprite height for current frame (sub). */
int updateAreaRectXSubCurrentFrame = 0; /* Update area rectangle X-coordinate for current frame (sub). */
int updateAreaRectYSubCurrentFrame = 0; /* Update area rectangle Y-coordinate for current frame (sub). */
int updateAreaRectWidthSubCurrentFrame = 0; /* Update area rectangle width for current frame (sub). */
int updateAreaRectHeightSubCurrentFrame = 0; /* Update area rectangle height for current frame (sub). */
int updateAreaRectXSubPreviousFrame = 0; /* Update area rectangle X-coordinate for previous frame (sub). */
int updateAreaRectYSubPreviousFrame = 0; /* Update area rectangle Y-coordinate for previous frame (sub). */
int updateAreaRectWidthSubPreviousFrame = 0; /* Update area rectangle width for previous frame (sub). */
int updateAreaRectHeightSubPreviousFrame = 0; /* Update area rectangle height for previous frame (sub). */
int screenXSubPreviousFrame = 0; /* Screen X-coordinate for previous frame (sub). */
int screenYSubPreviousFrame = 0; /* Screen Y-coordinate for previous frame (sub). */
int spriteWidthSubPreviousFrame = 0; /* Sprite width for previous frame (sub). */
int spriteHeightSubPreviousFrame = 0; /* Sprite height for previous frame (sub). */
WORD unused_A898 = 0; /* Current frame rectangle and previous frame rectangle have no intersecion? (sub) (unused) */
WORD subWindowState = 0; /* State. */
spriteinfo spriteListSub[1024] = {{{NULL, NULL}, 0, 0, 0, 0}}; /* Sprite list. First 512 unflipped, last 512 flipped. */

#define timeCheckPeriodCounter timeCheckPeriodCounter
#define framePeriodCounter framePeriodCounter
#define targetXWindowEdgeAttachment targetXWindowEdgeAttachment
#define targetYWindowEdgeAttachment targetYWindowEdgeAttachment
#define bounceWhenFalling bounceWhenFalling
#define fallActionCaseNumber fallActionCaseNumber
#define collisionVerticalSpeedUnused collisionVerticalSpeedUnused
#define collisionSpinFrameCounterUnused collisionSpinFrameCounterUnused
#define collisionAnimationFrameIndex collisionAnimationFrameIndex
#define knownInstanceListUpdatePeriodCounter knownInstanceListUpdatePeriodCounter

#define doubleBufferSub doubleBufferSub
#define spriteRenderTargetSub spriteRenderTargetSub
#define spriteColourBitmapSubCurrentFrame spriteColourBitmapSubCurrentFrame
#define spriteMaskBitmapSubCurrentFrame spriteMaskBitmapSubCurrentFrame
#define fadeOutColourBitmapSub fadeOutColourBitmapSub
#define fadeOutMaskBitmapSub fadeOutMaskBitmapSub
#define spriteXInResourceImageSubCurrentFrame spriteXInResourceImageSubCurrentFrame
#define spriteYInResourceImageSubCurrentFrame spriteYInResourceImageSubCurrentFrame
#define spriteXInResourceImageSubPreviousFrame spriteXInResourceImageSubPreviousFrame
#define spriteYInResourceImageSubPreviousFrameUnused spriteYInResourceImageSubPreviousFrameUnused
#define currentSpriteFramebufferIndexSub currentSpriteFramebufferIndexSub
#define renderOrUpdateWindowFlagSub renderOrUpdateWindowFlagSub
#define unused_A872 unused_A872
#define spriteColourBitmapSubPreviousFrame spriteColourBitmapSubPreviousFrame
#define screenXSubCurrentFrame screenXSubCurrentFrame
#define screenYSubCurrentFrame screenYSubCurrentFrame
#define spriteWidthSubCurrentFrame spriteWidthSubCurrentFrame
#define spriteHeightSubCurrentFrame spriteHeightSubCurrentFrame
#define updateAreaRectXSubCurrentFrame updateAreaRectXSubCurrentFrame
#define updateAreaRectYSubCurrentFrame updateAreaRectYSubCurrentFrame
#define updateAreaRectWidthSubCurrentFrame updateAreaRectWidthSubCurrentFrame
#define updateAreaRectHeightSubCurrentFrame updateAreaRectHeightSubCurrentFrame
#define updateAreaRectXSubPreviousFrame updateAreaRectXSubPreviousFrame
#define updateAreaRectYSubPreviousFrame updateAreaRectYSubPreviousFrame
#define updateAreaRectWidthSubPreviousFrame updateAreaRectWidthSubPreviousFrame
#define updateAreaRectHeightSubPreviousFrame updateAreaRectHeightSubPreviousFrame
#define screenXSubPreviousFrame screenXSubPreviousFrame
#define screenYSubPreviousFrame screenYSubPreviousFrame
#define spriteWidthSubPreviousFrame spriteWidthSubPreviousFrame
#define spriteHeightSubPreviousFrame spriteHeightSubPreviousFrame
#define unused_A898 unused_A898
#define subWindowState subWindowState
#define spriteListSub spriteListSub
int noMouseActionConsecutivePeriodCount = 0; /* No mouse action consecutive period counter. */
UINT chimeEnabled = 0U; /* Configuration: Chime */
WORD screenMateOnTopOfSubwindow = 0; /* Screen Mate window on top of subwindow? (unused) */
HWND selfInstanceWindowHandle = NULL; /* Self instance window handle. */
HBITMAP ufoBeamRenderTarget = NULL; /* UFO beam render target. */
HBRUSH ufoBeamPaintBrush = NULL; /* UFO beam paint colour brush. */
UINT alwaysMovingEnabled = 0U; /* Configuration: Always moving */
HBITMAP ufoBeamColorBitmap = NULL; /* UFO beam colour rectangle image. */
WORD noUpdatePeriodsAfterClearing = 0; /* Remaining no-update periods after clearing windows. */
windowinfo visibleWindowList[32] = {{NULL, {0, 0, 0, 0}, {0}}}; /* Currently visible window list. */

#define visibleWindowList visibleWindowList
WORD preventSpecialActions = 0; /* Prevent special actions? */
WORD alwaysOnTopUnused = 0; /* Always on top? (unused) */
WORD alienTransformPending = 0; /* UFO beam-up will return alien sheep? */
WORD alienModeActive = 0; /* Temporary aggressive alien mode. */
int alienModeTicks = 0; /* Remaining ticks in alien mode. */
int alienKnockCooldown = 0; /* Frames until next alien knock. */
WORD systemCursorHiddenForGraze = 0; /* ShowCursor(FALSE) while sheep grazes the cursor. */
int cursorGrazeStage = 0; /* Bite stage of the cursor prop, 0 (full) to 3. */
int cursorGrazeSpinFrame = 0; /* Spin frame of the cursor prop, 0-7. */
WORD acidModeActive = 0; /* Acid trip: alien sheets with cycling horn/eye colours. */
int acidColourIndex = 0; /* Current entry of acidColours. */
int acidTicks = 0; /* Tick counter within the current acid phase. */
int acidBaseY = 0; /* spriteY before the bounce phase. */
RGBQUAD FAR * spritePaletteOverride = NULL; /* Horn shades 8/10/12 and pupil 40, applied by LoadSpriteImagesAndStoreHandles. */
/* {bright, mid, dark} horn shades per VRCURSOR colour; pupils use the bright one. */
RGBQUAD acidColours[9][4] = {
    {{0, 0, 255, 0}, {0, 0, 170, 0}, {0, 0, 96, 0}, {0, 0, 255, 0}},
    {{0, 255, 255, 0}, {0, 170, 170, 0}, {0, 96, 96, 0}, {0, 255, 255, 0}},
    {{0, 255, 0, 0}, {0, 170, 0, 0}, {0, 96, 0, 0}, {0, 255, 0, 0}},
    {{255, 0, 0, 0}, {170, 0, 0, 0}, {96, 0, 0, 0}, {255, 0, 0, 0}},
    {{255, 0, 255, 0}, {170, 0, 170, 0}, {96, 0, 96, 0}, {255, 0, 255, 0}},
    {{0, 128, 128, 0}, {0, 96, 96, 0}, {0, 56, 56, 0}, {0, 160, 160, 0}},
    {{128, 0, 128, 0}, {96, 0, 96, 0}, {56, 0, 56, 0}, {160, 0, 160, 0}},
    {{128, 0, 0, 0}, {96, 0, 0, 0}, {56, 0, 0, 0}, {160, 0, 0, 0}},
    {{0, 128, 0, 0}, {0, 96, 0, 0}, {0, 56, 0, 0}, {0, 160, 0, 0}}
};
int knownInstanceCount = 0; /* Known instance count. */
UINT gravityAlwaysEnabled = 0U; /* Configuration: Gravity always on */
HBRUSH ufoBeamMaskBrush = NULL; /* UFO beam mask colour brush. */
int fadeOutFrameCounter = 0; /* Fade out frame counter. */
int unusedCa48 = 0; /* Unused. */
HPALETTE windowPaletteInUse = NULL; /* Palette being used by window. */
int unusedCa4C = 0; /* Unused. */
int unusedCa4E = 0; /* Unused. */
int screenWidth = 0; /* Screen width. */
int screenHeight = 0; /* Screen height. */

#define screenWidth screenWidth
#define screenHeight screenHeight
WORD sleepTimeoutAction = 0; /* Temporarily holds sleep timeout action. */
WORD keepSubwindowOnPaint = 0; /* Not to clear subwindow? */
HINSTANCE currentInstance = NULL; /* Current instance. */
UINT cryEnabled = 0U; /* Configuration: Cry */
int ufoBeamHeightSub = 0; /* UFO beam height (sub). */
WORD unusedCa5E = 0; /* Unused. */
HWND knownInstanceWindows[9] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}; /* Known instance list. When no other instance exists, [8] is used to store subwindow handle. */
int ufoBeamHeight = 0; /* UFO beam height. */
int visibleWindowCount = 0; /* Number of currently visible windows. */
WORD sleepingAfterTimeout = 0; /* Sleeping after timeout? */
WORD unusedCa78 = 0; /* Unused. */
#ifdef _WIN32
HWND ownerWindowHandle = NULL;
#define ownerWindowHandle ownerWindowHandle
#endif

/* Backward-compatible aliases for decompiler-generated names. */
#define noMouseActionConsecutivePeriodCount noMouseActionConsecutivePeriodCount
#define chimeEnabled chimeEnabled
#define screenMateOnTopOfSubwindow screenMateOnTopOfSubwindow
#define selfInstanceWindowHandle selfInstanceWindowHandle
#define ufoBeamRenderTarget ufoBeamRenderTarget
#define ufoBeamPaintBrush ufoBeamPaintBrush
#define alwaysMovingEnabled alwaysMovingEnabled
#define ufoBeamColorBitmap ufoBeamColorBitmap
#define noUpdatePeriodsAfterClearing noUpdatePeriodsAfterClearing

#define preventSpecialActions preventSpecialActions
#define knownInstanceCount knownInstanceCount
#define gravityAlwaysEnabled gravityAlwaysEnabled
#define ufoBeamMaskBrush ufoBeamMaskBrush
#define fadeOutFrameCounter fadeOutFrameCounter

#define ufoBeamHeight ufoBeamHeight
#define visibleWindowCount visibleWindowCount
#define sleepingAfterTimeout sleepingAfterTimeout

#define alwaysOnTopUnused alwaysOnTopUnused
#define alienTransformPending alienTransformPending
#define alienModeActive alienModeActive
#define alienModeTicks alienModeTicks
#define alienKnockCooldown alienKnockCooldown
#define unusedCa48 unusedCa48
#define unusedCa4C unusedCa4C
#define unusedCa4E unusedCa4E
#define unusedCa5E unusedCa5E
#define unusedCa78 unusedCa78

#define sleepTimeoutAction sleepTimeoutAction
#define keepSubwindowOnPaint keepSubwindowOnPaint
#define currentInstance currentInstance
#define cryEnabled cryEnabled
#define ufoBeamHeightSub ufoBeamHeightSub
#define knownInstanceWindows knownInstanceWindows
#define windowPaletteInUse windowPaletteInUse

void PASCAL CreateMaskBitmapFromFirstPixel(void FAR *, void FAR *);
int PASCAL MakeMaskBitmapImageOutSpecificColourIndexPaletteSimulatingX86Assembly(void FAR *, void FAR *, int);
void PASCAL DecompressBitmapImage(void FAR *, void FAR *);
#define sub_414(p) ((((BITMAPINFOHEADER FAR *)p)->biClrUsed == 0) ? ((DWORD)1 << ((BITMAPINFOHEADER FAR *)p)->biBitCount) : (((BITMAPINFOHEADER FAR *)p)->biClrUsed))
WORD GetPaletteSize(void FAR *);
WORD GetNumberColoursPalette(void FAR *);
HPALETTE CreatePaletteBasedGivenBitmapImage(HDC, void FAR *);
HPALETTE CreateMaskPaletteBasedGivenRgbValues(HDC, BYTE, BYTE, BYTE);
void SetPaletteEntriesBasedAnotherPaletteNearestColours(HDC, HPALETTE, HPALETTE, int);
WORD GetColourIndexFirstPixel(void FAR *);
void FAR * GetPointerPixelBitsBitmapImage(void FAR *);
void FlipBitmapImageArg8Contains1FlipHorizontallyArg8Contains2FlipVertically(void FAR *, void FAR *, UINT);
int PASCAL WinMain(HINSTANCE, HINSTANCE, LPSTR, int);
void SetCursorPositionChangedFlag(void);
LRESULT CALLBACK ScreenMateMainWindowProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK ScreenMateSubWindowProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK ConfigDialogProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK DebugDialogProc(HWND, UINT, WPARAM, LPARAM);
void CreateSubwindow(void);
void HideSystemCursorForGraze(void);
void RestoreSystemCursorAfterGraze(void);
void DestroySubwindow(void);
void PlaceWindowTopmostPosition(HWND);
void PlaceWindowTopAnother(HWND, HWND);
BOOL LoadSpriteImagesAndStoreHandles(HDC, spriteinfo *, int, int);
void ReleaseSpriteImages(spriteinfo *);
void ReadConfigurationFile(void);
void SaveIndividualConfigurationFile(LPCSTR, LPCSTR, UINT, LPCSTR);
void SaveConfigurationsFile(void);
BOOL InitializeBitmapsMain(HWND);
void ReleaseBitmaps();
void UpdateWindowPositionSpriteBeActuallyUsed(int, int, int);
void ClearWindow(HWND);
void RenderSpriteDoubleBuffering(HWND);
void RenderUfoBeamIfAnyPresentRenderTargetsOntoWindow(HWND);
void Func(HWND, int, int, int, int);
BOOL IsWindowInKnownInstanceList(HWND);
int FindXCoordinatePossibleCollisionOtherInstancesReturnZeroWhenNoCollisionDetected(int, int, int, int);
HWND FindCollidedOtherInstanceWindow(int, int, int, int);
int AggressiveSpriteIndex(int);
void FaceNearestOtherSheep(void);
void PopulateKnownInstanceListSearchingVisibleWindowsNameMatch(HWND);
BOOL PopulateKnownInstanceListAndNotify(HWND);
void NotifyOtherInstancesSelfDestruction(HWND);
void AddWindowKnownInstanceList(HWND);
void RemoveWindowKnownInstanceList(HWND);
void PopulateKnownVisibleWindowList(void);
int FindXCoordinatePossibleCollisionWhichVisibleWindow(HWND *, int, int, int, int);
int FindYCoordinatePossibleLandingTopEdgeWhichVisibleWindow(HWND *, int, int, int, int);
int GetWindowTopYCoordinateIfItIsPossibleLandWindow(HWND, int, int, int, int);
void PlaySoundResourceIdAdditionalFlags(int, UINT, WORD);
void StopPlayingSound(void);
void PlaySoundName(LPCSTR);
void PlaySoundResourceIdAdditionalFlagsWhenOptionCryEnabled(int, UINT, WORD);
BOOL GenerateSpritesFromLoadedResourceImages(HDC);
void LinkSheetSprites(int);
void ReloadAcidSheets(void);
void ApplyAcidColour(int);
void RestoreAcidColours(void);
void AdvanceAcidTick(void);
void AcidRollStep(int);
void ReleaseResourceImages(void);
void TurnAroundWhenApproachingScreenBorderOtherwise120Probability(void);
void FlagControlledCollisionTurnAround(BOOL);
void SwitchToStandingSprite(void);
void ProcessChime(void);
void UpdateMainWindowSprite(int, int, int);
void UpdateSubWindowSprite(int, int, int);
BOOL IsNullOrValidWindow(HWND);
void GetWindowRectOrScreenRect(HWND, LPRECT);
void HandleOutOfViewOrTopPosition(int);
void HandleClimbingSideOfWindow(void);
void DetectCollisionOtherInstancesActionControlledFlag(int, int, int);
int DetectCollisionOtherInstancesFindXCoordinate(int, int);
void ResetSpriteState(void);
void UpdateSpriteStateOnTimer(void);
void ApplyEnvironmentActionChange(int);
void ProcessDebugWindowActionChange(WPARAM);
void MoveWindowOffset(int, int);
BOOL InitializeBitmapsSub(HWND);
void ReleaseBitmaps2();
void UpdateWindowPositionSpriteBeActuallyUsed2(int, int, int);
void ClearWindow2(HWND);
void RenderSpriteDoubleBufferingFadeOutEffect(HWND);
BOOL RenderUfoBeamAndPresentSubRenderTargets(HWND);

/* Make mask bitmap image out of the first pixel (by simulating x86 assembly). */
void PASCAL CreateMaskBitmapFromFirstPixel(void FAR * arg_4, void FAR * arg_0)
{
#define ax (LOWORD(eax))
#define cx (LOWORD(ecx))
#define STACK_SIZE (2 * sizeof(WORD))
    BYTE var_2;
    WORD var_4;
    WORD var_6;

    HLOCAL stack = LocalAlloc(LMEM_FIXED, STACK_SIZE);

    register DWORD eax;
    register BYTE al;
    register DWORD ebx;
    register DWORD ecx;
    register WORD dx;
    register BYTE FAR * source;
    register BYTE FAR * destination;
    register BYTE * sp = (BYTE *)stack + STACK_SIZE;

    al = 0; /* xor eax, eax */
    ecx = 0; /* xor ecx, ecx */
    dx = 0; /* xor edx, edx */
    source = arg_0; /* lds si, [bp+arg_0] */
    destination = arg_4; /* les di, [bp+arg_4] */
    eax = (WORD)sub_414(source); /* call sub_414 */
    sp -= sizeof(WORD); /* push ax */
    *(WORD *)sp = ax;
    *(DWORD FAR *)destination = *(DWORD FAR *)source; /* movsd; biSize */
    source += 4;
    destination += 4;
    eax = (*(DWORD FAR *)source); /* lodsd */
    source += 4;
    var_6 = LOWORD(eax); /* mov [bp+var_6], ax; var_6 = biWidth */
    *(DWORD FAR *)destination = eax; /* stosd; biWidth */
    destination += 4;
    eax = (*(DWORD FAR *)source); /* lodsd */
    source += 4;
    ecx = eax; /* mov ecx, eax; ecx = biHeight */
    *(DWORD FAR *)destination = eax; /* stosd; biHeight */
    destination += 4;
    eax = (*(DWORD FAR *)source); /* lodsd */
    source += 4;
    eax = 0x10001; /* mov eax, 10001h */
    *(DWORD FAR *)destination = eax; /* stosd; biPlanes = 1, biBitCount = 1 */
    destination += 4;
    *(DWORD FAR *)destination = *(DWORD FAR *)source; /* movsd; biCompression */
    source += 4;
    destination += 4;
    eax = (*(DWORD FAR *)source); /* lodsd; biSizeImage */
    source += 4;
    var_4 = (var_6 + 31) / 32; /* mov ax, [bp+var_6]; add ax, 31; shr ax, 5; mov [bp+var_4], ax */
    eax = var_4 * 4 * cx; /* shl ax, 2; mul cx */
    *(DWORD FAR *)destination = eax; /* stosd; biSizeImage = ceil(biWidth, 32) * 4 * biHeight */
    destination += 4;
    *(DWORD FAR *)destination = *(DWORD FAR *)source; /* movsd; biXPelsPerMeter */
    source += 4;
    destination += 4;
    *(DWORD FAR *)destination = *(DWORD FAR *)source; /* movsd; biYPelsPerMeter */
    source += 4;
    destination += 4;
    eax = (*(DWORD FAR *)source); /* lodsd */
    source += 4;
    eax = 2; /* mov eax, 2 */
    *(DWORD FAR *)destination = eax; /* stosd; biClrUsed */
    destination += 4;
    eax = (*(DWORD FAR *)source); /* lodsd */
    source += 4;
    eax = 2; /* mov eax, 2 */
    *(DWORD FAR *)destination = eax; /* stosd; biClrImportant */
    destination += 4;
    eax = *(WORD *)sp; /* pop ax */
    sp += sizeof(WORD);
    source += ax * sizeof(RGBQUAD); /* shl ax, 2; add si, ax */
    eax = 0x00FFFFFF; /* mov eax, 0FFFFFFh */
    *(DWORD FAR *)destination = eax; /* stosd */
    destination += 4;
    eax = 0; /* xor eax, eax */
    *(DWORD FAR *)destination = eax; /* stosd */
    destination += 4;
    var_2 = *source; /* mov al, [esi]; mov [bp+var_2], al; Read the first colour */
    do {
        sp -= sizeof(WORD); /* push cx */
        *(WORD *)sp = cx;
        dx = var_6; /* mov dx, [bp+var_6]; dx = biWidth */
        ecx = var_4; /* mov cx, [bp+var_4]; ceil(biWidth, 32) */
        do {
            sp -= sizeof(WORD); /* push cx */
            *(WORD *)sp = cx;
            ebx = 0; /* xor ebx, ebx */
            ecx = 8; /* mov cx, 8 */
            do {
                if ((short)dx > 0) { /* cmp dx, 0; jle short loc_F0 */
                    eax = (*(DWORD FAR *)source); /* lods dword ptr [esi] */
                    source += 4;
                    dx -= 4; /* sub dx, 4 */
                    al = LOBYTE(ax) == var_2 ? 1 : 0; /* cmp al, [bp+var_2]; setz al; */
                    ebx = ebx << 1 | (al == 0 ? 1 : 0); /* cmp al, 1; rcl ebx, 1 */
                    eax >>= 8; /* shr eax, 8 */
                    al = LOBYTE(ax) == var_2 ? 1 : 0; /* cmp al, [bp+var_2]; setz al; */
                    ebx = ebx << 1 | (al == 0 ? 1 : 0); /* cmp al, 1; rcl ebx, 1 */
                    eax >>= 8; /* shr eax, 8 */
                    al = LOBYTE(ax) == var_2 ? 1 : 0; /* cmp al, [bp+var_2]; setz al; */
                    ebx = ebx << 1 | (al == 0 ? 1 : 0); /* cmp al, 1; rcl ebx, 1 */
                    eax >>= 8; /* shr eax, 8 */
                    al = LOBYTE(ax) == var_2 ? 1 : 0; /* cmp al, [bp+var_2]; setz al; */
                    ebx = ebx << 1 | (al == 0 ? 1 : 0); /* cmp al, 1; rcl ebx, 1 */
                    eax >>= 8; /* shr eax, 8 */
                } else { /* jmp short loc_F4 */
                    ebx <<= 4; /* shl ebx, 4 */
                }
            } while (--ecx != 0); /* loop loc_A4; 8 */
            eax = (DWORD)LOBYTE(LOWORD(ebx)) << 24 | (DWORD)HIBYTE(LOWORD(ebx)) << 16 | (DWORD)LOBYTE(HIWORD(ebx)) << 8 | (DWORD)HIBYTE(HIWORD(ebx)); /* mov eax, ebx; xchg al, ah; ror eax, 16; xchg al, ah */
            *(DWORD FAR *)destination = eax; /* stos dword ptr es:[edi] */
            destination += 4;
            ecx = *(WORD *)sp; /* pop cx */
            sp += sizeof(WORD);
        } while (--ecx != 0); /* loop loc_9D; ceil(biWidth, 32) */
        ecx = *(WORD *)sp; /* pop cx */
        sp += sizeof(WORD);
    } while (--ecx != 0); /* loop loc_96; biHeight */

    LocalFree(stack);
#undef STACK_SIZE
#undef cx
#undef ax
}

/* Make mask bitmap image out of the specific colour index in the palette (by simulating x86 assembly) (unused). */
int PASCAL MakeMaskBitmapImageOutSpecificColourIndexPaletteSimulatingX86Assembly(void FAR * arg_6, void FAR * arg_2, int arg_0)
{
#define ax (LOWORD(eax))
#define bl (LOBYTE(LOWORD(ebx)))
#define cx (LOWORD(ecx))
#define STACK_SIZE (sizeof(WORD) + sizeof(BYTE FAR *))
    BYTE var_2;
    int var_4;
    WORD var_6;
    WORD var_8;

    HLOCAL stack = LocalAlloc(LMEM_FIXED, STACK_SIZE);

    register DWORD eax;
    register BYTE al;
    register DWORD ebx;
    register DWORD ecx;
    register WORD dx;
    register BYTE FAR * source;
    register BYTE FAR * destination;
    register BYTE * sp = (BYTE *)stack + STACK_SIZE;

    var_4 = 0; /* mov [bp+var_4], 0 */
    al = 0; /* xor eax, eax */
    ecx = 0; /* xor ecx, ecx */
    dx = 0; /* xor edx, edx */
    source = arg_2; /* lds si, [bp+arg_2] */
    destination = arg_6; /* les di, [bp+arg_6] */
    eax = (WORD)sub_414(source); /* call sub_414 */
    sp -= sizeof(WORD); /* push ax */
    *(WORD *)sp = ax;
    *(DWORD FAR *)destination = *(DWORD FAR *)source; /* movsd; biSize */
    source += 4;
    destination += 4;
    eax = (*(DWORD FAR *)source); /* lodsd */
    source += 4;
    var_8 = LOWORD(eax); /* mov [bp+var_8], ax; var_8 = biWidth */
    *(DWORD FAR *)destination = eax; /* stosd; biWidth */
    destination += 4;
    eax = (*(DWORD FAR *)source); /* lodsd */
    source += 4;
    ecx = eax; /* mov ecx, eax; ecx = biHeight */
    *(DWORD FAR *)destination = eax; /* stosd; biHeight */
    destination += 4;
    eax = (*(DWORD FAR *)source); /* lodsd */
    source += 4;
    eax = 0x10001; /* mov eax, 10001h */
    *(DWORD FAR *)destination = eax; /* stosd; biPlanes = 1, biBitCount = 1 */
    destination += 4;
    *(DWORD FAR *)destination = *(DWORD FAR *)source; /* movsd; biCompression */
    source += 4;
    destination += 4;
    eax = (*(DWORD FAR *)source); /* lodsd; biSizeImage */
    source += 4;
    var_6 = (var_8 + 31) / 32; /* mov ax, [bp+var_8]; add ax, 31; shr ax, 5; mov [bp+var_6], ax */
    eax = var_6 * 4 * cx; /* shl ax, 2; mul cx */
    *(DWORD FAR *)destination = eax; /* stosd; biSizeImage = ceil(biWidth, 32) * 4 * biHeight */
    destination += 4;
    *(DWORD FAR *)destination = *(DWORD FAR *)source; /* movsd; biXPelsPerMeter */
    source += 4;
    destination += 4;
    *(DWORD FAR *)destination = *(DWORD FAR *)source; /* movsd; biYPelsPerMeter */
    source += 4;
    destination += 4;
    eax = (*(DWORD FAR *)source); /* lodsd */
    source += 4;
    eax = 2; /* mov eax, 2 */
    *(DWORD FAR *)destination = eax; /* stosd; biClrUsed */
    destination += 4;
    eax = (*(DWORD FAR *)source); /* lodsd */
    source += 4;
    eax = 2; /* mov eax, 2 */
    *(DWORD FAR *)destination = eax; /* stosd; biClrImportant */
    destination += 4;
    sp -= sizeof(BYTE FAR *); /* push esi */
    *(BYTE FAR * *)sp = source;
    source += arg_0 * sizeof(RGBQUAD); /* mov ax, [bp+arg_0]; shl ax, 2; add si, ax */
    *(DWORD FAR *)destination = *(DWORD FAR *)source; /* movsd; Mask colour */
    source += 4;
    destination += 4;
    eax = 0; /* xor eax, eax */
    *(DWORD FAR *)destination = eax; /* stosd */
    destination += 4;
    source = *(BYTE FAR * *)sp; /* pop esi */
    sp += sizeof(BYTE FAR *);
    eax = *(WORD *)sp; /* pop ax */
    sp += sizeof(WORD);
    source += ax * sizeof(RGBQUAD); /* shl ax, 2; add si, ax */
    var_2 = LOBYTE(arg_0); /* mov ax, [bp+arg_0]; mov [bp+var_2], al; The specified colour */
    do {
        sp -= sizeof(WORD); /* push cx */
        *(WORD *)sp = cx;
        dx = var_8; /* mov dx, [bp+var_8]; dx = biWidth */
        ecx = var_6; /* mov cx, [bp+var_6]; ceil(biWidth, 32) */
        do {
            sp -= sizeof(WORD); /* push cx */
            *(WORD *)sp = cx;
            ebx = 0; /* xor ebx, ebx */
            ecx = 8; /* mov cx, 8 */
            do {
                if ((short)dx > 0) { /* cmp dx, 0; jle short loc_209 */
                    eax = (*(DWORD FAR *)source); /* lods dword ptr [esi] */
                    source += 4;
                    dx -= 4; /* sub dx, 4 */
                    al = LOBYTE(ax) == var_2 ? 1 : 0; /* cmp al, [bp+var_2]; setz al; */
                    ebx = ebx << 1 | (al == 0 ? 1 : 0); /* cmp al, 1; rcl ebx, 1 */
                    eax >>= 8; /* shr eax, 8 */
                    al = LOBYTE(ax) == var_2 ? 1 : 0; /* cmp al, [bp+var_2]; setz al; */
                    ebx = ebx << 1 | (al == 0 ? 1 : 0); /* cmp al, 1; rcl ebx, 1 */
                    eax >>= 8; /* shr eax, 8 */
                    al = LOBYTE(ax) == var_2 ? 1 : 0; /* cmp al, [bp+var_2]; setz al; */
                    ebx = ebx << 1 | (al == 0 ? 1 : 0); /* cmp al, 1; rcl ebx, 1 */
                    eax >>= 8; /* shr eax, 8 */
                    al = LOBYTE(ax) == var_2 ? 1 : 0; /* cmp al, [bp+var_2]; setz al; */
                    ebx = ebx << 1 | (al == 0 ? 1 : 0); /* cmp al, 1; rcl ebx, 1 */
                    eax >>= 8; /* shr eax, 8 */
                    al = bl; /* mov al, bl */
                    al &= 0x0F; /* and al, 0Fh */
                    var_4 += al < 0x0F ? 1 : 0; /* cmp al, 0Fh; adc [bp+var_4], 0 */
                } else { /* jmp short loc_20D */
                    ebx <<= 4; /* shl ebx, 4 */
                }
            } while (--ecx != 0); /* loop loc_1B3; 8 */
            eax = (DWORD)LOBYTE(LOWORD(ebx)) << 24 | (DWORD)HIBYTE(LOWORD(ebx)) << 16 | (DWORD)LOBYTE(HIWORD(ebx)) << 8 | (DWORD)HIBYTE(HIWORD(ebx)); /* mov eax, ebx; xchg al, ah; ror eax, 16; xchg al, ah */
            *(DWORD FAR *)destination = eax; /* stos dword ptr es:[edi] */
            destination += 4;
            ecx = *(WORD *)sp; /* pop cx */
            sp += sizeof(WORD);
        } while (--ecx != 0); /* loop loc_1AC; ceil(biWidth, 32) */
        ecx = *(WORD *)sp; /* pop cx */
        sp += sizeof(WORD);
    } while (--ecx != 0); /* loop loc_1A5; biHeight */

    LocalFree(stack);

    return var_4; /* mov ax, [bp+var_4]; Number of non-transparent blocks */
#undef STACK_SIZE
#undef cx
#undef bl
#undef ax
}

/* Decompress bitmap image. */
void PASCAL DecompressBitmapImage(void FAR * arg_4, void FAR * arg_0)
{
    BYTE FAR * source = arg_0;
    BYTE FAR * destination = arg_4;
    BYTE FAR * originalsource = NULL;
    BYTE FAR * originaldestination = NULL;
    WORD var_2 = (WORD)((BITMAPINFOHEADER FAR *)source)->biWidth;
    DWORD ebx = var_2 + 3 & (WORD)-4;
    LONG edx = ((BITMAPINFOHEADER FAR *)source)->biHeight;
    DWORD compression = ((BITMAPINFOHEADER FAR *)source)->biCompression;
    WORD bitcount = ((BITMAPINFOHEADER FAR *)source)->biBitCount;
    DWORD counter = ((BITMAPINFOHEADER FAR *)source)->biSize;
    BYTE bytebuffer[2] = {0, 0};
    for (; counter != 0; counter -= 1) {
        *destination++ = *source++;
    }
    ((BITMAPINFOHEADER FAR *)arg_4)->biCompression = BI_RGB;
    counter = sub_414(arg_0) * 4;
    for (; counter != 0; counter -= 1) {
        *destination++ = *source++;
    }
    if (compression == BI_RGB) {
        if (bitcount == 4) {
            ((BITMAPINFOHEADER FAR *)arg_4)->biBitCount = 8;
            if (((BITMAPINFOHEADER FAR *)arg_4)->biClrUsed == 0) {
                ((BITMAPINFOHEADER FAR *)arg_4)->biClrUsed = 16;
            }
            compression = edx;
            var_2 = (var_2 + 1) / 2 + 3 & -4;
            do {
                originalsource = source;
                originaldestination = destination;
                counter = ebx;
                do {
                    if ((counter & 1) == 0) {
                        bytebuffer[0] = *source++;
                        bytebuffer[1] = bytebuffer[0];
                        bytebuffer[0] >>= 4;
                    } else {
                        bytebuffer[0] = bytebuffer[1];
                        bytebuffer[0] &= 0x0F;
                    }
                    *destination++ = bytebuffer[0];
                } while (--counter != 0);
                destination = originaldestination + ebx;
                source = originalsource + var_2;
            } while (--compression != 0);
        } else {
            counter = ebx * edx;
            for (; counter != 0; counter -= 1) {
                *destination++ = *source++;
            }
        }
    } else if (compression == BI_RLE4) {
        ((BITMAPINFOHEADER FAR *)arg_4)->biBitCount = 8;
        if (((BITMAPINFOHEADER FAR *)arg_4)->biClrUsed == 0) {
            ((BITMAPINFOHEADER FAR *)arg_4)->biClrUsed = 16;
        }
        for (;;) {
            originaldestination = destination;
            for (;;) {
                bytebuffer[0] = *source++;
                bytebuffer[1] = *source++;
                if (bytebuffer[0] == 0) {
                    if (bytebuffer[1] == 0) {
                        destination = originaldestination + ebx;
                        break;
                    } else if (bytebuffer[1] == 1) {
                        return;
                    } else if (bytebuffer[1] == 2) {
                        bytebuffer[0] = *source++;
                        bytebuffer[1] = *source++;
                        if (bytebuffer[1] != 0) {
                            do {
                                destination += ebx;
                                originaldestination += ebx;
                            } while (bytebuffer[1] != 0);
                        }
                        destination += bytebuffer[0];
                    } else {
                        counter = bytebuffer[1];
                        do {
                            bytebuffer[0] = *source++;
                            bytebuffer[1] = *source++;
                            *destination++ = bytebuffer[0] >> 4;
                            if (--counter == 0) {
                                break;
                            }
                            *destination++ = bytebuffer[0] & 0x0F;
                            if (--counter == 0) {
                                break;
                            }
                            *destination++ = bytebuffer[1] >> 4;
                            if (--counter == 0) {
                                break;
                            }
                            *destination++ = bytebuffer[1] & 0x0F;
                        } while (--counter != 0);
                    }
                } else {
                    counter = bytebuffer[0] / 2;
                    for (; counter != 0; counter -= 1) {
                        *destination++ = bytebuffer[1] >> 4;
                        *destination++ = bytebuffer[1] & 0x0F;
                    }
                    if ((bytebuffer[0] & 1) != 0) {
                        *destination++ = bytebuffer[1] >> 4;
                    }
                }
            }
        }
    } else if (compression == BI_RLE8) {
        for (;;) {
            originaldestination = destination;
            for (;;) {
                bytebuffer[0] = *source++;
                bytebuffer[1] = *source++;
                if (bytebuffer[0] == 0) {
                    if (bytebuffer[1] == 0) {
                        destination = originaldestination + ebx;
                        break;
                    } else if (bytebuffer[1] == 1) {
                        return;
                    } else if (bytebuffer[1] == 2) {
                        bytebuffer[0] = *source++;
                        bytebuffer[1] = *source++;
                        if (bytebuffer[1] != 0) {
                            do {
                                destination += ebx;
                                originaldestination += ebx;
                            } while (bytebuffer[1] != 0);
                        }
                        destination += bytebuffer[0];
                    } else {
                        counter = bytebuffer[1];
                        do {
                            *destination++ = *source++;
                        } while (--counter != 0);
                        source += bytebuffer[1] & 1;
                    }
                } else {
                    do {
                        *destination++ = bytebuffer[1];
                    } while (--bytebuffer[0] != 0);
                }
            }
        }
    }
}

/* Get palette size. */
WORD GetPaletteSize(void FAR * arg_0)
{
    DWORD FAR * var_4;
    WORD var_6;
    var_4 = arg_0;
    var_6 = GetNumberColoursPalette(var_4);
    if (*var_4 == 12) {
        return var_6 * sizeof(RGBTRIPLE);
    } else {
        return var_6 * sizeof(RGBQUAD);
    }
}

/* Get number of colours in palette. */
WORD GetNumberColoursPalette(void FAR * arg_0)
{
    WORD var_2;
    BITMAPINFOHEADER FAR * var_6;
    BITMAPCOREHEADER FAR * var_A;
    var_6 = arg_0;
    var_A = arg_0;
    if (var_6->biSize != 12) {
        if (var_6->biClrUsed != 0) {
            return (WORD)var_6->biClrUsed;
        }
        var_2 = var_6->biBitCount;
    } else {
        var_2 = var_A->bcBitCount;
    }
    switch (var_2) {
    case 1:
        return 2;
    case 4:
        return 16;
    case 8:
        return 256;
    default:
        return 0;
    }
}

/* Create palette based on given bitmap image. */
HPALETTE CreatePaletteBasedGivenBitmapImage(HDC arg_0, void FAR * arg_2)
{
    const BYTE FAR * var_4;
    HPALETTE var_6;
    LOGPALETTE * var_8;
    int var_A;
    int var_C;
    if ((GetDeviceCaps(arg_0, RASTERCAPS) & RC_PALETTE) == 0) {
        var_A = 256;
    } else {
        var_A = arg_2 != NULL ? ((BITMAPINFOHEADER FAR *)arg_2)->biClrUsed != 0 ? (int)(((BITMAPINFOHEADER FAR *)arg_2)->biClrUsed) : (1 << ((BITMAPINFOHEADER FAR *)arg_2)->biBitCount) : GetDeviceCaps(arg_0, SIZEPALETTE);
    }
    var_8 = (LOGPALETTE *)LocalAlloc(LPTR, (var_A + 2) * sizeof(PALETTEENTRY));
    var_8->palVersion = 0x0300;
    var_8->palNumEntries = (WORD)var_A;
    if (arg_2 != NULL) {
        var_4 = (BYTE FAR *)arg_2 + ((BITMAPINFOHEADER FAR *)arg_2)->biSize;
        for (var_C = 0; var_C < var_A; var_C += 1) {
            var_8->palPalEntry[var_C].peRed = var_4[2];
            var_8->palPalEntry[var_C].peGreen = var_4[1];
            var_8->palPalEntry[var_C].peBlue = var_4[0];
            var_8->palPalEntry[var_C].peFlags = 0;
            var_4 += sizeof(PALETTEENTRY);
        }
    } else {
        GetSystemPaletteEntries(arg_0, 0U, var_A, var_8->palPalEntry);
    }
    var_6 = CreatePalette(var_8);
    LocalFree(var_8);
    return var_6;
}

/* Create mask palette based on given RGB values. */
HPALETTE CreateMaskPaletteBasedGivenRgbValues(HDC arg_0, BYTE arg_2, BYTE arg_3, BYTE arg_4)
{
    const BYTE FAR * var_4;
    HPALETTE var_6;
    LOGPALETTE * var_8;
    var_8 = (LOGPALETTE *)LocalAlloc(LPTR, 12U);
    var_8->palVersion = 0x0300;
    var_8->palNumEntries = 1;
    var_8->palPalEntry[0].peRed = arg_2;
    var_8->palPalEntry[0].peGreen = arg_3;
    var_8->palPalEntry[0].peBlue = arg_4;
    var_8->palPalEntry[0].peFlags = 0;
    var_4/* += 4 */;
    var_6 = CreatePalette(var_8);
    LocalFree(var_8);
    return var_6;
}

/* Set palette entries based on another palette with nearest colours (unused). */
void SetPaletteEntriesBasedAnotherPaletteNearestColours(HDC arg_0, HPALETTE arg_2, HPALETTE arg_4, int arg_6)
{
    PALETTEENTRY * var_2;
    PALETTEENTRY * var_4;
    int var_6;
    int var_8;
    COLORREF var_C;
    COLORREF var_10;
    int var_12;
    if ((GetDeviceCaps(arg_0, RASTERCAPS) & RC_PALETTE) == 0) {
        return;
    }
    var_6 = GetDeviceCaps(arg_0, SIZEPALETTE);
    if (var_6 == 0) {
        var_6 = 256;
    }
    var_2 = (PALETTEENTRY *)LocalAlloc(LPTR, arg_6 * sizeof(PALETTEENTRY));
    var_4 = (PALETTEENTRY *)LocalAlloc(LPTR, var_6 * sizeof(PALETTEENTRY));
    GetPaletteEntries(arg_4, 0U, arg_6, var_2);
    GetPaletteEntries(arg_2, 0U, var_6, var_4);
    for (var_8 = 0; var_8 < arg_6; var_8 += 1) {
        var_C = *(COLORREF *)&var_2[var_8];
        var_12 = GetNearestPaletteIndex(arg_2, var_C);
        var_10 = *(COLORREF *)&var_4[var_12];
        if (var_10 != var_C) {
            if (var_12 < 10 || var_12 > paletteSearchMaxIndexUnused) {
                if (paletteSearchMaxIndexUnused < 10) {
                    break;
                } else {
                    var_12 = paletteSearchMaxIndexUnused--;
                }
            }
            *(COLORREF *)&var_4[var_12] = var_C;
        }
    }
    SetPaletteEntries(arg_2, 0U, var_6, var_4);
    LocalFree(var_4);
    LocalFree(var_2);
}

/* Get colour index of first pixel. */
WORD GetColourIndexFirstPixel(void FAR * arg_0)
{
    BYTE FAR * var_4;
    BYTE var_6;
    var_4 = GetPointerPixelBitsBitmapImage(arg_0);
    var_6 = *var_4++;
    switch (((BITMAPINFOHEADER FAR *)arg_0)->biCompression) {
    case BI_RGB:
        if (((BITMAPINFOHEADER FAR *)arg_0)->biBitCount == 4) {
            var_6 >>= 4;
        }
        break;
    case BI_RLE8:
        if (var_6 == 0) {
            var_4 += 1;
        }
        var_6 = *var_4;
        break;
    case BI_RLE4:
        if (var_6 == 0) {
            var_4 += 1;
        }
        var_6 = *var_4 >> 4;
        break;
    default:
        break;
    }
    return var_6;
}

/* Get pointer to pixel bits in bitmap image. */
void FAR * GetPointerPixelBitsBitmapImage(void FAR * arg_0)
{
    return (void FAR *)((BYTE FAR *)arg_0 + *(WORD FAR *)arg_0 + ((BITMAPINFOHEADER FAR *)arg_0)->biClrUsed * sizeof(RGBQUAD));
}

/* Flip bitmap image. arg_8 contains 1: Flip horizontally, arg_8 contains 2: Flip vertically */
void FlipBitmapImageArg8Contains1FlipHorizontallyArg8Contains2FlipVertically(void FAR * arg_0, void FAR * arg_4, UINT arg_8)
{
    BYTE FAR * var_4;
    BYTE FAR * var_8;
    BYTE FAR * var_C;
    int var_E;
    int var_10;
    int var_12;
    int var_14;
    LONG var_18;
    var_4 = arg_0;
    var_8 = arg_4;
    var_C = GetPointerPixelBitsBitmapImage(arg_4);
    while (var_C > var_8) {
        *var_4++ = *var_8++;
    }
    var_E = ((BITMAPINFOHEADER FAR *)arg_4)->biBitCount == 4 ? (int)(-(((BITMAPINFOHEADER FAR *)arg_4)->biWidth % 8) & 7) : (int)(-(((BITMAPINFOHEADER FAR *)arg_4)->biWidth % 4) & 3);
    var_10 = ((BITMAPINFOHEADER FAR *)arg_4)->biBitCount == 4 ? (int)(((BITMAPINFOHEADER FAR *)arg_4)->biWidth + var_E) / 2 : (int)(((BITMAPINFOHEADER FAR *)arg_4)->biWidth + var_E);
    if ((arg_8 & 2) != 0) {
        var_C += (((BITMAPINFOHEADER FAR *)arg_4)->biHeight - 1) * var_10;
    }
    if ((arg_8 & 1) != 0) {
        var_18 = ((BITMAPINFOHEADER FAR *)arg_4)->biBitCount == 4 ? ((BITMAPINFOHEADER FAR *)arg_4)->biWidth / 2 : ((BITMAPINFOHEADER FAR *)arg_4)->biWidth;
        var_C += var_18 - 1;
    }
    for (var_12 = 0; ((BITMAPINFOHEADER FAR *)arg_4)->biHeight > var_12; var_12 += 1) {
        var_8 = var_C;
        if (((BITMAPINFOHEADER FAR *)arg_4)->biBitCount == 4) {
            if ((arg_8 & 1) != 0) {
                for (var_14 = 0; var_14 < var_10; var_14 += 1) {
                    *var_4 = *var_8 >> 4 & 0x0F | *var_8 << 4;
                    var_8 -= 1;
                    var_4 += 1;
                }
            } else {
                for (var_14 = 0; var_14 < var_10; var_14 += 1) {
                    *var_4++ = *var_8++;
                }
            }
        } else {
            if ((arg_8 & 1) != 0) {
                for (var_14 = 0; var_14 < var_10; var_14 += 1) {
                    *var_4++ = *var_8--;
                }
            } else {
                for (var_14 = 0; var_14 < var_10; var_14 += 1) {
                    *var_4++ = *var_8++;
                }
            }
        }
        var_18 = (arg_8 & 2) != 0 ? -var_10 : var_10;
        var_C += var_18;
    }
}

/* WinMain function. */
int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    HWND var_2;
    MSG var_14;
    WNDCLASS var_2E;
    INITCOMMONCONTROLSEX icc;
    icc.dwSize = sizeof(icc);
    icc.dwICC = ICC_STANDARD_CLASSES;
    InitCommonControlsEx(&icc);
    if (hPrevInstance == NULL) {
        var_2E.style = CS_DBLCLKS;
        var_2E.lpfnWndProc = ScreenMateMainWindowProc;
        var_2E.cbClsExtra = 0;
        var_2E.cbWndExtra = 8;
        var_2E.hInstance = hInstance;
        var_2E.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(100));
        var_2E.hCursor = LoadCursor(hInstance, MAKEINTRESOURCE(106));
        var_2E.hbrBackground = NULL;
        var_2E.lpszMenuName = NULL;
        var_2E.lpszClassName = "ScreenMatePoo";
        if (RegisterClass(&var_2E) == 0) {
            return 0;
        }
    }
    if (hPrevInstance == NULL) {
        var_2E.style = CS_DBLCLKS;
        var_2E.lpfnWndProc = ScreenMateSubWindowProc;
        var_2E.cbClsExtra = 0;
        var_2E.cbWndExtra = 0;
        var_2E.hInstance = hInstance;
        var_2E.hIcon = NULL;
        var_2E.hCursor = LoadCursor(NULL, IDC_ARROW);
        var_2E.hbrBackground = NULL;
        var_2E.lpszMenuName = NULL;
        var_2E.lpszClassName = "ScreenMatePooSub";
        if (RegisterClass(&var_2E) == 0) {
            return 0;
        }
    }
    currentInstance = hInstance;
#ifdef _WIN32
    /* In 32-bit Windows, popup window is now in the taskbar by default. Additional code is needed to hide the popup window from taskbar while keeping it in the Alt+Tab list. */
    var_2E.style = 0;
    var_2E.lpfnWndProc = DefWindowProc;
    var_2E.cbClsExtra = 0;
    var_2E.cbWndExtra = 0;
    var_2E.hInstance = hInstance;
    var_2E.hIcon = NULL;
    var_2E.hCursor = NULL;
    var_2E.hbrBackground = NULL;
    var_2E.lpszMenuName = NULL;
    var_2E.lpszClassName = "ScreenMatePooOwner";
    if (RegisterClass(&var_2E) == 0) {
        return 0;
    }
    /* Create a hidden owner top-level window to hide visible windows from taskbar. */
    ownerWindowHandle = CreateWindowEx(0L, "ScreenMatePooOwner", NULL, 0L, 0, 0, 0, 0, NULL, NULL, hInstance, NULL);
    if (ownerWindowHandle == NULL) {
        return 0;
    }
    /* Set the visible window to be owned by the hidden top-level window. */
    var_2 = CreateWindowEx(WS_EX_LAYERED, "ScreenMatePoo", "Screen Mate", WS_POPUP, 0, 0, 0, 0, ownerWindowHandle, NULL, hInstance, NULL);
#else
    var_2 = CreateWindowEx(0L, "ScreenMatePoo", "Screen Mate", WS_POPUP, 0, 0, 0, 0, NULL, NULL, hInstance, NULL);
#endif
    if (var_2 == NULL) {
        return 0;
    }
    ShowWindow(var_2, nShowCmd);
    UpdateWindow(var_2);
    while (GetMessage(&var_14, NULL, 0U, 0U)) {
        TranslateMessage(&var_14);
        DispatchMessage(&var_14);
    }
#ifdef _WIN32
    DestroyWindow(ownerWindowHandle);
#endif
    return (int)var_14.wParam;
}

/* Set cursor position changed flag (unused). */
void SetCursorPositionChangedFlag(void)
{
    cursorPositionChanged = 1;
}

/* Window procedure. */
LRESULT CALLBACK ScreenMateMainWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HDC var_2;
    void FAR * var_6;
    HGLOBAL var_8;
    HRSRC var_A;
    void FAR * var_E;
    int var_10;
    int var_12;
    RECT var_1A;
    POINT var_1E;
    char var_122[260];
    POINT var_126;
    UINT var_128;
    FARPROC proc;
    LPWINDOWPOS windowpos;
#ifdef _WIN32
    HANDLE user32;
#endif
    switch (uMsg) {
    case WM_CREATE:
        if (!PopulateKnownInstanceListAndNotify(hWnd)) {
            MessageBox(hWnd, "Screen Mate: maximum 8 instances", "Screen Mate", MB_ICONHAND | MB_OK);
            return -1;
        }
#ifdef _WIN32
        /* Additional code that allows drag-and-drop across different privileges in Windows Vista and higher. */
        user32 = GetModuleHandle("user32.dll");
        if (user32 != NULL) {
            BOOL (WINAPI * changewindowmessagefilterex)(HWND, UINT, DWORD, LPVOID) = (BOOL (WINAPI *)(HWND, UINT, DWORD, LPVOID))GetProcAddress(user32, "ChangeWindowMessageFilterEx");
            if (changewindowmessagefilterex != NULL) {
                changewindowmessagefilterex(hWnd, 0x0049, 1, NULL); /* WM_COPYGLOBALDATA */
                changewindowmessagefilterex(hWnd, 0x004A, 1, NULL); /* WM_COPYDATA */
                changewindowmessagefilterex(hWnd, WM_DROPFILES, 1, NULL);
            }
        }
#endif
        DragAcceptFiles(hWnd, TRUE);
        selfInstanceWindowHandle = hWnd;
        ReadConfigurationFile();
        if (alwaysOnTopUnused != 0) {
            SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
        }
        var_2 = GetDC(hWnd);
        var_A = FindResource(currentInstance, MAKEINTRESOURCE(101), RT_BITMAP);
        var_8 = LoadResource(currentInstance, var_A);
        var_6 = LockResource(var_8);
        var_E = (void FAR *)((BYTE FAR *)var_6 + *(WORD FAR *)var_6 + GetPaletteSize(var_6));
        windowPaletteInUse = CreatePaletteBasedGivenBitmapImage(var_2, var_6);
        FreeResource(var_8);
        SelectPalette(var_2, windowPaletteInUse, FALSE);
        RealizePalette(var_2);
        if (!GenerateSpritesFromLoadedResourceImages(var_2)) {
            MessageBox(hWnd, "Not enough memory", "Screen Mate", MB_ICONHAND | MB_OK);
            ReleaseDC(hWnd, var_2);
            return -1;
        }
        ReleaseDC(hWnd, var_2);
        if (!InitializeBitmapsMain(hWnd)) {
            MessageBox(hWnd, "Not enough memory", "Screen Mate", MB_ICONHAND | MB_OK);
            return -1;
        }
        SetTimer(hWnd, 1U, 108U, NULL);
        break;
    case WM_DROPFILES:
        if (knownInstanceWindows[8] == NULL) {
            if (DragQueryFile((HDROP)wParam, 0U, var_122, 260U) != 0U) {
                PlaySoundName(var_122);
                ApplyEnvironmentActionChange(4);
            }
        }
        DragFinish((HDROP)wParam);
        break;
    case WM_USER + 1:
        switch (lParam) {
        case 0x202:
        case 0x205:
            if (screenMateOnTopOfSubwindow != 0) {
                if (knownInstanceWindows[8] != NULL) {
                    PlaceWindowTopmostPosition(knownInstanceWindows[8]);
                }
                PlaceWindowTopmostPosition(hWnd);
            } else {
                PlaceWindowTopmostPosition(hWnd);
                if (knownInstanceWindows[8] != NULL) {
                    PlaceWindowTopmostPosition(knownInstanceWindows[8]);
                }
            }
            PlaceWindowTopmostPosition(hWnd);
            *(HMENU *)var_122 = CreatePopupMenu();
            AppendMenu(*(HMENU *)var_122, 0U, 101U, "Screen Mate Settings...");
            AppendMenu(*(HMENU *)var_122, 0U, IDCANCEL, "Screen Mate Exit");
            GetCursorPos(&var_126);
            TrackPopupMenu(*(HMENU *)var_122, 0U, var_126.x, var_126.y, 0, hWnd, NULL);
            DestroyMenu(*(HMENU *)var_122);
            return 0;
        default:
            break;
        }
        return 0;
    case WM_WINDOWPOSCHANGING:
        windowpos = (LPWINDOWPOS)lParam;
        if (unused_A7A2 != 0) {
            windowpos = (LPWINDOWPOS)lParam;
        }
        windowpos->flags |= SWP_NOCOPYBITS;
        doNotClearWindowOnPaint = 1;
        return 0;
    case WM_WINDOWPOSCHANGED:
        return 0;
    case WM_TIMER:
        if (noUpdatePeriodsAfterClearing != 0) {
            noUpdatePeriodsAfterClearing -= 1;
            return 0;
        }
        if (alwaysMovingEnabled == 0) {
            GetCursorPos(&var_1E);
            if (cursorPosition.x != var_1E.x || cursorPosition.y != var_1E.y) {
                cursorPosition.x = var_1E.x;
                cursorPosition.y = var_1E.y;
                cursorPositionChanged = 1;
            }
            if (sleepingAfterTimeout != 0) {
                if (cursorPositionChanged != 0) {
                    sleepingAfterTimeout = 0;
                    noMouseActionConsecutivePeriodCount = 0;
                    ApplyEnvironmentActionChange(0);
                }
            } else {
                if (cursorPositionChanged != 0) {
                    cursorPositionChanged = 0;
                    noMouseActionConsecutivePeriodCount = 0;
                } else {
                    if (noMouseActionConsecutivePeriodCount++ > 300) {
                        ApplyEnvironmentActionChange(3);
                    }
                }
            }
        }
        doNotClearWindowOnPaint = 0;
        selfInstanceWindowHandle = hWnd;
        UpdateSpriteStateOnTimer();
        RenderSpriteDoubleBuffering(hWnd);
        RenderUfoBeamIfAnyPresentRenderTargetsOntoWindow(hWnd);
        unused_A7A2 = 1;
        return 0;
    case WM_USER:
        if (wParam == 1) {
            AddWindowKnownInstanceList((HWND)lParam);
        }
        if (wParam == 2) {
            RemoveWindowKnownInstanceList((HWND)lParam);
        }
        if (wParam == 3) {
            /* Hit by alien: roll, or sometimes panic-flee in knock direction. */
            DestroySubwindow();
            ufoBeamHeight = 0;
            ufoBeamHeightSub = 0;
            facingDirection = (int)(short)lParam;
            if (facingDirection == 0) {
                facingDirection = 1;
            }
            animationFrameCounter = 0;
            framePeriodCounter = 0;
            if ((rand() & 1) != 0) {
                /* Scared flee: longer run away from alien. */
                collisionEnabled = 0;
                animationFrameCounter = 35 + rand() % 25;
                spriteIndex = 4;
                UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
                subWindowState = 8;
            } else {
                subWindowState = 65;
            }
        }
        return 0;
    case WM_PAINT:
        if (doNotClearWindowOnPaint != 0) {
            doNotClearWindowOnPaint = 0;
            ValidateRect(hWnd, NULL);
            return 0;
        }
        ClearWindow(hWnd);
        ValidateRect(hWnd, NULL);
        return 0;
        GetWindowRect(hWnd, &var_1A);
        if (screenMateWindowRect.top == var_1A.top && screenMateWindowRect.bottom == var_1A.bottom && screenMateWindowRect.left == var_1A.left && screenMateWindowRect.right == var_1A.right) {
            ClearWindow(hWnd);
            ValidateRect(hWnd, NULL);
            return 0;
        }
        GetWindowRect(hWnd, &screenMateWindowRect);
        ValidateRect(hWnd, NULL);
        return 0;
        break;
    case WM_QUIT:
        return 0;
    case WM_COMMAND:
        switch (wParam) {
        case 101U:
            SendMessage(hWnd, WM_USER + 2, 0, 0);
            break;
        case IDCANCEL:
            DestroyWindow(hWnd);
            return 0;
            break;
        default:
            break;
        }
        break;
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
        if (knownInstanceWindows[8] != NULL) {
            break;
        }
        if (draggingScreenMateWindow != 0) {
            break;
        }
        SetCapture(hWnd);
        GetWindowRect(hWnd, &var_1A);
        cursorScreenX = (short)LOWORD(lParam) + var_1A.left;
        cursorScreenY = (short)HIWORD(lParam) + var_1A.top;
        draggingScreenMateWindow = 1;
        ApplyEnvironmentActionChange(1);
        RenderSpriteDoubleBuffering(hWnd);
        RenderUfoBeamIfAnyPresentRenderTargetsOntoWindow(hWnd);
        break;
    case WM_MOUSEMOVE:
        if (draggingScreenMateWindow == 0) {
            break;
        }
        GetWindowRect(hWnd, &var_1A);
        var_10 = (short)LOWORD(lParam) + var_1A.left;
        var_12 = (short)HIWORD(lParam) + var_1A.top;
        if (cursorScreenX == var_10 && cursorScreenY == var_12) {
            break;
        }
        MoveWindowOffset(var_10 - cursorScreenX, var_12 - cursorScreenY);
        cursorScreenX = var_10;
        cursorScreenY = var_12;
        RenderSpriteDoubleBuffering(hWnd);
        RenderUfoBeamIfAnyPresentRenderTargetsOntoWindow(hWnd);
        break;
    case WM_RBUTTONUP:
        if (destroyScreenMateOnRightDoubleClick != 0) {
            DestroyWindow(hWnd);
            break;
        }
    case WM_LBUTTONUP:
        if (draggingScreenMateWindow != 0) {
            GetWindowRect(hWnd, &var_1A);
            var_10 = (short)LOWORD(lParam) + var_1A.left;
            var_12 = (short)HIWORD(lParam) + var_1A.top;
            MoveWindowOffset(var_10 - cursorScreenX, var_12 - cursorScreenY);
            ApplyEnvironmentActionChange(0);
            if (uMsg == WM_RBUTTONUP) {
                ApplyEnvironmentActionChange(2);
            }
            RenderSpriteDoubleBuffering(hWnd);
            RenderUfoBeamIfAnyPresentRenderTargetsOntoWindow(hWnd);
            ReleaseCapture();
            draggingScreenMateWindow = 0;
        }
        break;
    case WM_RBUTTONDBLCLK:
        destroyScreenMateOnRightDoubleClick = 1;
        break;
    case WM_LBUTTONDBLCLK:
    case WM_USER + 2:
        *(WORD *)var_122 = alwaysOnTopUnused;
        var_128 = gravityAlwaysEnabled;
        selfInstanceWindowHandle = hWnd;
        if ((HIBYTE(GetKeyState(VK_SHIFT)) & 0x80) != 0 && (HIBYTE(GetKeyState(VK_CONTROL)) & 0x80) != 0) {
            proc = MakeProcInstance((FARPROC)DebugDialogProc, currentInstance);
            DialogBox(currentInstance, MAKEINTRESOURCE(108), hWnd, (DLGPROC)proc);
        } else {
            proc = MakeProcInstance((FARPROC)ConfigDialogProc, currentInstance);
            DialogBox(currentInstance, MAKEINTRESOURCE(107), hWnd, (DLGPROC)proc);
        }
        FreeProcInstance(proc);
        ClearWindow(hWnd);
        if (*(WORD *)var_122 != alwaysOnTopUnused) {
            if (alwaysOnTopUnused != 0) {
                SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
            } else {
                SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
            }
        }
        if (var_128 != gravityAlwaysEnabled && gravityAlwaysEnabled != 0) {
            ApplyEnvironmentActionChange(2);
        }
        break;
    case WM_DESTROY:
        NotifyOtherInstancesSelfDestruction(hWnd);
        if (knownInstanceWindows[8] != NULL) {
            DestroySubwindow();
        }
        KillTimer(hWnd, 1U);
        StopPlayingSound();
        spritePaletteOverride = NULL;
        acidModeActive = 0;
        ReleaseBitmaps();
        ReleaseResourceImages();
        DeleteObject(windowPaletteInUse);
        DragAcceptFiles(hWnd, FALSE);
        PostQuitMessage(0);
        break;
    default:
        break;
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

/* Window procedure (sub). */
LRESULT CALLBACK ScreenMateSubWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LPWINDOWPOS var_4;
    switch (uMsg) {
    case WM_CREATE:
        if (!InitializeBitmapsSub(hWnd)) {
            DestroyWindow(hWnd);
            return 1;
        }
        SetTimer(hWnd, 1U, 108U, NULL);
        break;
    case WM_WINDOWPOSCHANGING:
        var_4 = (LPWINDOWPOS)lParam;
        var_4->flags |= SWP_NOCOPYBITS;
        doNotClearSubwindowOnPaint = 1;
        return 0;
    case WM_WINDOWPOSCHANGED:
        return 0;
    case WM_TIMER:
        doNotClearSubwindowOnPaint = 0;
        RenderSpriteDoubleBufferingFadeOutEffect(hWnd);
        if (!RenderUfoBeamAndPresentSubRenderTargets(hWnd)) {
            preventSpecialActions = 1;
            ResetSpriteState();
        }
        return 0;
    case WM_PAINT:
        if (doNotClearSubwindowOnPaint != 0) {
            doNotClearSubwindowOnPaint = 0;
            ValidateRect(hWnd, NULL);
            return 0;
        }
        ClearWindow2(hWnd);
        ValidateRect(hWnd, NULL);
        return 0;
    case WM_ERASEBKGND:
        return 0;
    case WM_DESTROY:
        ReleaseBitmaps2();
        KillTimer(hWnd, 1U);
        break;
    default:
        break;
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

/* Configuration window callback. */
BOOL CALLBACK ConfigDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
    case WM_INITDIALOG:
        SendDlgItemMessage(hDlg, 1001, BM_SETCHECK, (WPARAM)chimeEnabled, 0);
        SendDlgItemMessage(hDlg, 1002, BM_SETCHECK, (WPARAM)cryEnabled, 0);
        SendDlgItemMessage(hDlg, 1003, BM_SETCHECK, (WPARAM)alwaysMovingEnabled, 0);
        SendDlgItemMessage(hDlg, 1004, BM_SETCHECK, (WPARAM)gravityAlwaysEnabled, 0);
        return TRUE;
    case WM_COMMAND:
        if (wParam == IDRETRY) {
            WinHelp(hDlg, "Scmpoo16.hlp", HELP_INDEX, 0L);
            return TRUE;
        }
        if (wParam == IDOK) {
            chimeEnabled = IsDlgButtonChecked(hDlg, 1001);
            cryEnabled = IsDlgButtonChecked(hDlg, 1002);
            alwaysMovingEnabled = IsDlgButtonChecked(hDlg, 1003);
            gravityAlwaysEnabled = IsDlgButtonChecked(hDlg, 1004);
            SaveConfigurationsFile();
        }
        if (wParam == IDABORT) {
            DestroyWindow(selfInstanceWindowHandle);
        }
        if (wParam == IDOK || wParam == IDCANCEL || wParam == IDABORT) {
            EndDialog(hDlg, (int)wParam);
        }
        break;
    default:
        break;
    }
    return FALSE;
}

/* Debug window callback. */
BOOL CALLBACK DebugDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static const char *debugActionNames[32] = {
        "Normal", "Run", "Walk", "Handstand", "Pee-pee", "Sleep", "Blink", "Turn",
        "Collision", "Pee", "Yawn", "Bleat", "Scared", "Surprised", "Eat", "Sit",
        "Sneeze", "Burning", "Merry 1", "Merry 2", "Merry 3", "UFO 1", "UFO 2", "UFO 3",
        "Rolling", "Blush", "Slide", "Fall", "Jump", "Spin", "Alien", "Mushroom"
    };
    int i;
    int selectedAction;
    WORD controlId;
    WORD notifyCode;

    switch (uMsg) {
    case WM_INITDIALOG:
        for (i = 0; i < 32; i++) {
            SendDlgItemMessage(hDlg, 1040, LB_ADDSTRING, 0, (LPARAM)debugActionNames[i]);
        }
        SendDlgItemMessage(hDlg, 1040, LB_SETCURSEL, 0, 0);
        return TRUE;
    case WM_COMMAND:
        controlId = LOWORD(wParam);
        notifyCode = HIWORD(wParam);
        if (controlId == IDOK || controlId == IDCANCEL) {
            EndDialog(hDlg, (int)controlId);
            return TRUE;
        }
        if (controlId == 1040 && notifyCode == LBN_SELCHANGE) {
            selectedAction = (int)SendDlgItemMessage(hDlg, 1040, LB_GETCURSEL, 0, 0);
            if (selectedAction >= 0) {
                ProcessDebugWindowActionChange((WPARAM)selectedAction);
            }
            return TRUE;
        }
        switch (controlId) {
        case 1032:
            MoveWindowOffset(0, -20);
            break;
        case 1033:
            MoveWindowOffset(0, 20);
            break;
        case 1034:
            MoveWindowOffset(-20, 0);
            break;
        case 1035:
            MoveWindowOffset(20, 0);
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
    return FALSE;
}

/* Create subwindow. */
void CreateSubwindow(void)
{
    if (knownInstanceWindows[8] != NULL) {
        return;
    }
#ifdef _WIN32
    /* Set the visible window to be owned by the hidden top-level window. */
    knownInstanceWindows[8] = CreateWindowEx(WS_EX_LAYERED, "ScreenMatePooSub", "ScreenMate Sub", WS_POPUP, 0, 0, 0, 0, ownerWindowHandle, NULL, currentInstance, NULL);
#else
    knownInstanceWindows[8] = CreateWindowEx(0L, "ScreenMatePooSub", "ScreenMate Sub", WS_POPUP, 0, 0, 0, 0, NULL, NULL, currentInstance, NULL);
#endif
    if (knownInstanceWindows[8] != NULL) {
        ShowWindow(knownInstanceWindows[8], SW_SHOWNA);
        UpdateWindow(knownInstanceWindows[8]);
    } else {
        preventSpecialActions = 1;
        ResetSpriteState();
    }
}

/* Hide the real system cursor while the sheep grazes a cursor prop. */
void HideSystemCursorForGraze(void)
{
    if (systemCursorHiddenForGraze == 0) {
        ShowCursor(FALSE);
        systemCursorHiddenForGraze = 1;
    }
}

/* Restore the system cursor after a graze (or on abort). */
void RestoreSystemCursorAfterGraze(void)
{
    if (systemCursorHiddenForGraze != 0) {
        ShowCursor(TRUE);
        systemCursorHiddenForGraze = 0;
    }
}

/* Destroy subwindow. */
void DestroySubwindow(void)
{
    RestoreSystemCursorAfterGraze();
    if (knownInstanceWindows[8] != NULL) {
        DestroyWindow(knownInstanceWindows[8]);
        knownInstanceWindows[8] = NULL;
    }
}

/* Place window to topmost position. */
void PlaceWindowTopmostPosition(HWND arg_0)
{
    if (alwaysOnTopUnused == 0) {
        SetWindowPos(arg_0, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
        SetWindowPos(arg_0, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
    }
}

/* Place a window on top of another. */
void PlaceWindowTopAnother(HWND arg_0, HWND arg_2)
{
    if (alwaysOnTopUnused == 0) {
        SetWindowPos(arg_0, arg_2, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
    }
}

/* Load resource image, generate (flipped) colour and mask images, store handles into sprite info structure. */
BOOL LoadSpriteImagesAndStoreHandles(HDC arg_0, spriteinfo * arg_2, int arg_4, int arg_6)
{
    void FAR * var_4;
    void FAR * var_8;
    void FAR * var_C;
    void FAR * var_10;
    void FAR * var_14;
    HGLOBAL var_16;
    HGLOBAL var_18;
    HGLOBAL var_1A;
    var_16 = NULL;
    var_18 = NULL;
    var_1A = NULL;
    var_16 = LoadResource(currentInstance, FindResource(currentInstance, MAKEINTRESOURCE(arg_4), RT_BITMAP));
    var_C = LockResource(var_16);
    if (var_16 == NULL) {
        return FALSE;
    }
    arg_2->x = 0;
    arg_2->y = 0;
    arg_2->width = *(int FAR *)((BYTE FAR *)var_C + 4);
    arg_2->height = *(int FAR *)((BYTE FAR *)var_C + 8);
    if (arg_6 < 0) {
        var_18 = GlobalAlloc(GMEM_MOVEABLE, 65535);
        if (var_18 == NULL) {
            goto loc_2EB2;
        }
        var_10 = GlobalLock(var_18);
        if (arg_6 < -1) {
            var_1A = GlobalAlloc(GMEM_MOVEABLE, 65535);
            if (var_1A == NULL) {
                goto loc_2EB2;
            }
            var_14 = GlobalLock(var_1A);
            DecompressBitmapImage(var_14, var_C);
            if (arg_6 == -2) {
                FlipBitmapImageArg8Contains1FlipHorizontallyArg8Contains2FlipVertically(var_10, var_14, 2U);
            } else {
                FlipBitmapImageArg8Contains1FlipHorizontallyArg8Contains2FlipVertically(var_10, var_14, 1U);
            }
            GlobalFree(var_1A);
            var_1A = NULL;
        } else {
            DecompressBitmapImage(var_10, var_C);
        }
        if (spritePaletteOverride != NULL) {
            RGBQUAD FAR * palette = (RGBQUAD FAR *)((BYTE FAR *)var_10 + *(WORD FAR *)var_10);
            palette[8] = spritePaletteOverride[0];
            palette[10] = spritePaletteOverride[1];
            palette[12] = spritePaletteOverride[2];
            palette[40] = spritePaletteOverride[3];
        }
        var_4 = (void FAR *)((BYTE FAR *)var_10 + *(WORD FAR *)var_10 + GetPaletteSize(var_10));
        var_8 = (void FAR *)((BYTE FAR *)var_10 + *(WORD FAR *)var_10 + GetColourIndexFirstPixel(var_10) * sizeof(RGBQUAD));
        *(DWORD FAR *)var_8 = 0;
        arg_2->bitmaps[0] = CreateDIBitmap(arg_0, var_10, CBM_INIT, var_4, var_10, DIB_RGB_COLORS);
        if (arg_2->bitmaps[0] == NULL) {
            goto loc_2EB2;
        }
        CreateMaskBitmapFromFirstPixel(var_10, var_10);
        var_4 = (void FAR *)((BYTE FAR *)var_10 + *(WORD FAR *)var_10 + GetPaletteSize(var_10));
        arg_2->bitmaps[1] = CreateDIBitmap(arg_0, var_10, CBM_INIT, var_4, var_10, DIB_RGB_COLORS);
        if (arg_2->bitmaps[1] == NULL) {
            goto loc_2EB2;
        }
        FreeResource(var_16);
        var_16 = NULL;
        GlobalFree(var_18);
        var_18 = NULL;
        return TRUE;
    } else {
        var_4 = (void FAR *)((BYTE FAR *)var_C + *(WORD FAR *)var_C + GetPaletteSize(var_C));
        arg_2->bitmaps[0] = CreateDIBitmap(arg_0, var_C, CBM_INIT, var_4, var_C, DIB_RGB_COLORS);
        if (arg_2->bitmaps[0] == NULL) {
            goto loc_2EB2;
        }
        FreeResource(var_16);
        var_16 = NULL;
        if (arg_6 == 0) {
            arg_2->bitmaps[1] = NULL;
            return TRUE;
        }
        var_16 = LoadResource(currentInstance, FindResource(currentInstance, MAKEINTRESOURCE(arg_6), RT_BITMAP));
        var_C = LockResource(var_16);
        if (var_16 == NULL) {
            goto loc_2EB2;
        }
        arg_2->x = 0;
        arg_2->y = 0;
        arg_2->width = *(int FAR *)((BYTE FAR *)var_C + 4);
        arg_2->height = *(int FAR *)((BYTE FAR *)var_C + 8);
        var_4 = (void FAR *)((BYTE FAR *)var_C + *(WORD FAR *)var_C + GetPaletteSize(var_C));
        arg_2->bitmaps[1] = CreateDIBitmap(arg_0, var_C, CBM_INIT, var_4, var_C, DIB_RGB_COLORS);
        if (arg_2->bitmaps[1] == NULL) {
            goto loc_2EB2;
        }
        FreeResource(var_16);
        return TRUE;
    }
loc_2EB2:
    if (var_16 != NULL) {
        FreeResource(var_16);
    }
    if (var_18 != NULL) {
        GlobalFree(var_18);
    }
    if (var_1A != NULL) {
        GlobalFree(var_1A);
    }
    return FALSE;
}

/* Release sprite images. */
void ReleaseSpriteImages(spriteinfo * arg_0)
{
    if (arg_0->bitmaps[0] != NULL) {
        DeleteObject(arg_0->bitmaps[0]);
    }
    if (arg_0->bitmaps[1] != NULL) {
        DeleteObject(arg_0->bitmaps[1]);
    }
    arg_0->bitmaps[0] = NULL;
    arg_0->bitmaps[1] = NULL;
}

/* Read configuration from file. */
void ReadConfigurationFile(void)
{
    unusedCa78 = 1;
    alwaysOnTopUnused = 0;
    chimeEnabled = 0U;
    cryEnabled = 0U;
    cryEnabled = GetPrivateProfileInt("Stray", "Sound", 0, "scmate.ini");
    chimeEnabled = GetPrivateProfileInt("Stray", "Alarm", 0, "scmate.ini");
    alwaysMovingEnabled = GetPrivateProfileInt("Stray", "NoSleep", 0, "scmate.ini");
    gravityAlwaysEnabled = GetPrivateProfileInt("Stray", "GForce", 1, "scmate.ini");
}

/* Save individual configuration to file. */
void SaveIndividualConfigurationFile(LPCSTR arg_0, LPCSTR arg_4, UINT arg_8, LPCSTR arg_A)
{
    char var_28[40];
    wsprintf(var_28, "%u", arg_8);
    WritePrivateProfileString(arg_0, arg_4, var_28, arg_A);
}

/* Save configurations to file. */
void SaveConfigurationsFile(void)
{
    SaveIndividualConfigurationFile("Stray", "Sound", cryEnabled, "scmate.ini");
    SaveIndividualConfigurationFile("Stray", "Alarm", chimeEnabled, "scmate.ini");
    SaveIndividualConfigurationFile("Stray", "NoSleep", alwaysMovingEnabled, "scmate.ini");
    SaveIndividualConfigurationFile("Stray", "GForce", gravityAlwaysEnabled, "scmate.ini");
}

#ifdef _WIN32
/* Under desktop composition the screen can't be captured reliably as a background, so windows are layered and carry per-pixel alpha. */
typedef struct layeredsurface {
    HBITMAP bitmap;
    DWORD * bits;
    int width;
    int height;
} layeredsurface;

layeredsurface layeredSurfaceMain = {NULL, NULL, 0, 0};
layeredsurface layeredSurfaceSub = {NULL, NULL, 0, 0};

/* Make sure the 32-bit surface is at least the requested size. */
BOOL EnsureLayeredSurface(layeredsurface * surface, int width, int height)
{
    BITMAPINFO info;
    void * bits;
    if (surface->bitmap != NULL && surface->width >= width && surface->height >= height) {
        return TRUE;
    }
    width = max(width, surface->width);
    height = max(height, surface->height);
    if (surface->bitmap != NULL) {
        DeleteObject(surface->bitmap);
        surface->bitmap = NULL;
    }
    ZeroMemory(&info, sizeof(info));
    info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    info.bmiHeader.biWidth = width;
    info.bmiHeader.biHeight = -height;
    info.bmiHeader.biPlanes = 1;
    info.bmiHeader.biBitCount = 32;
    info.bmiHeader.biCompression = BI_RGB;
    surface->bitmap = CreateDIBSection(NULL, &info, DIB_RGB_COLORS, &bits, NULL, 0);
    if (surface->bitmap == NULL) {
        surface->bits = NULL;
        surface->width = 0;
        surface->height = 0;
        return FALSE;
    }
    surface->bits = (DWORD *)bits;
    surface->width = width;
    surface->height = height;
    return TRUE;
}

/* Release the 32-bit surface. */
void ReleaseLayeredSurface(layeredsurface * surface)
{
    if (surface->bitmap != NULL) {
        DeleteObject(surface->bitmap);
    }
    surface->bitmap = NULL;
    surface->bits = NULL;
    surface->width = 0;
    surface->height = 0;
}

/* Compose a masked sprite (and the UFO beam below it, if any) with alpha and show it on a layered window. */
void PresentLayeredSprite(HWND window, layeredsurface * surface, int x, int y, HBITMAP colour, HBITMAP mask, int sourceX, int sourceY, int width, int height, int beamHeight, BOOL alienBeam)
{
    HDC surfaceDC;
    HDC spriteDC;
    HGDIOBJ previousSurfaceBitmap;
    HGDIOBJ previousSpriteBitmap;
    POINT destination;
    POINT source;
    SIZE size;
    BLENDFUNCTION blend;
    DWORD beamPixel;
    DWORD * pixel;
    DWORD * maskPixel;
    int totalHeight;
    int beamWidth;
    int row;
    int column;
    if (width <= 0 || height <= 0) {
        return;
    }
    totalHeight = height + beamHeight;
    /* The mask is staged in the rows below the visible area. */
    if (!EnsureLayeredSurface(surface, width, totalHeight + height)) {
        return;
    }
    surfaceDC = CreateCompatibleDC(NULL);
    spriteDC = CreateCompatibleDC(NULL);
    previousSurfaceBitmap = SelectObject(surfaceDC, surface->bitmap);
    PatBlt(surfaceDC, 0, 0, width, totalHeight + height, BLACKNESS);
    previousSpriteBitmap = SelectObject(spriteDC, colour);
    BitBlt(surfaceDC, 0, 0, width, height, spriteDC, sourceX, sourceY, SRCCOPY);
    if (mask != NULL) {
        SelectObject(spriteDC, mask);
        SetTextColor(surfaceDC, RGB(0, 0, 0));
        SetBkColor(surfaceDC, RGB(255, 255, 255));
        BitBlt(surfaceDC, 0, totalHeight, width, height, spriteDC, sourceX, sourceY, SRCCOPY);
    }
    SelectObject(spriteDC, previousSpriteBitmap);
    GdiFlush();
    for (row = 0; row < height; row++) {
        pixel = surface->bits + row * surface->width;
        maskPixel = surface->bits + (totalHeight + row) * surface->width;
        for (column = 0; column < width; column++) {
            DWORD colourValue = pixel[column] & 0x00FFFFFF;
            DWORD maskValue = mask != NULL ? maskPixel[column] & 0x00FFFFFF : 0;
            if (maskValue != 0 && colourValue == 0) {
                pixel[column] = 0;
            } else {
                pixel[column] = colourValue | 0xFF000000;
            }
        }
    }
    if (beamHeight > 0) {
        /* Premultiplied ARGB: yellow tractor beam, or red while abducting into an alien. */
        beamPixel = alienBeam ? 0x90780000 : 0x80807400;
        beamWidth = min(width, 40);
        for (row = height; row < totalHeight; row++) {
            pixel = surface->bits + row * surface->width;
            for (column = 0; column < width; column++) {
                pixel[column] = column < beamWidth ? beamPixel : 0;
            }
        }
    }
    destination.x = x;
    destination.y = y;
    source.x = 0;
    source.y = 0;
    size.cx = width;
    size.cy = totalHeight;
    blend.BlendOp = AC_SRC_OVER;
    blend.BlendFlags = 0;
    blend.SourceConstantAlpha = 255;
    blend.AlphaFormat = AC_SRC_ALPHA;
    UpdateLayeredWindow(window, NULL, &destination, &size, surfaceDC, &source, 0, &blend, ULW_ALPHA);
    SelectObject(surfaceDC, previousSurfaceBitmap);
    DeleteDC(spriteDC);
    DeleteDC(surfaceDC);
}
#endif

/* Initialize bitmaps. */
BOOL InitializeBitmapsMain(HWND arg_0)
{
    HDC var_2;
    var_2 = GetDC(arg_0);
    doubleBufferMain[0] = CreateCompatibleBitmap(var_2, 100, 100);
    if (doubleBufferMain[0] == NULL) {
        goto loc_3104;
    }
    doubleBufferMain[1] = CreateCompatibleBitmap(var_2, 100, 100);
    if (doubleBufferMain[1] == NULL) {
        goto loc_3104;
    }
    spriteRenderTargetMain = CreateCompatibleBitmap(var_2, 100, 100);
    if (spriteRenderTargetMain == NULL) {
        goto loc_3104;
    }
    unusedCa4C = 0;
    unusedCa4E = 0;
    screenWidth = GetSystemMetrics(SM_CXSCREEN);
    screenHeight = GetSystemMetrics(SM_CYSCREEN);
    ReleaseDC(arg_0, var_2);
    return TRUE;
loc_3104:
    ReleaseDC(arg_0, var_2);
    return FALSE;
}

/* Release bitmaps. */
void ReleaseBitmaps()
{
    DeleteObject(doubleBufferMain[0]);
    DeleteObject(doubleBufferMain[1]);
    DeleteObject(spriteRenderTargetMain);
    if (ufoBeamPaintBrush != NULL) {
        DeleteObject(ufoBeamPaintBrush);
        ufoBeamPaintBrush = NULL;
    }
    if (ufoBeamMaskBrush != NULL) {
        DeleteObject(ufoBeamMaskBrush);
        ufoBeamMaskBrush = NULL;
    }
    if (ufoBeamRenderTarget != NULL) {
        DeleteObject(ufoBeamRenderTarget);
        ufoBeamRenderTarget = NULL;
    }
    if (ufoBeamColorBitmap != NULL) {
        DeleteObject(ufoBeamColorBitmap);
        ufoBeamColorBitmap = NULL;
    }
#ifdef _WIN32
    ReleaseLayeredSurface(&layeredSurfaceMain);
#endif
}

/* Update window position and sprite to be actually used. */
void UpdateWindowPositionSpriteBeActuallyUsed(int arg_0, int arg_2, int arg_4)
{
    screenXCurrentFrame = arg_0;
    screenYCurrentFrame = arg_2;
    spriteColourBitmapMain = spriteListSub[arg_4].bitmaps[0];
    spriteMaskBitmapMain = spriteListSub[arg_4].bitmaps[1];
    spriteXInResourceImageCurrentFrame = spriteListSub[arg_4].x;
    spriteYInResourceImageCurrentFrame = spriteListSub[arg_4].y;
    spriteWidthCurrentFrame = spriteListSub[arg_4].width;
    spriteHeightCurrentFrame = spriteListSub[arg_4].height;
}

/* Clear window. */
void ClearWindow(HWND arg_0)
{
    screenXPreviousFrame = 0;
    screenYPreviousFrame = 0;
    spriteWidthPreviousFrame = 0;
    spriteHeightPreviousFrame = 0;
    MoveWindow(arg_0, 0, 0, 0, 0, TRUE);
    unused_A7D4 = 1;
    noUpdatePeriodsAfterClearing = 1;
    spriteColourBitmapPreviousFrame = NULL;
}

/* Render sprite with double buffering. */
void RenderSpriteDoubleBuffering(HWND arg_0)
{
#ifndef _WIN32
    HDC var_2;
    HDC var_4;
    HDC var_6;
    int var_C;
    int var_E;
    int var_10;
    int var_12;
    int var_14;
    int var_16;
    int var_18;
    int var_1A;
    int var_1C;
    int var_1E;
#endif
    if (renderOrUpdateWindowFlag != 0) {
        return;
    }
    if (screenXPreviousFrame == screenXCurrentFrame && screenYPreviousFrame == screenYCurrentFrame && spriteColourBitmapPreviousFrame == spriteColourBitmapMain && spriteXInResourceImagePreviousFrame == spriteXInResourceImageCurrentFrame && ufoBeamHeight == 0) {
        return;
    }
#ifdef _WIN32
    updateAreaRectXCurrentFrame = screenXCurrentFrame;
    updateAreaRectYCurrentFrame = screenYCurrentFrame;
    updateAreaRectWidthCurrentFrame = spriteWidthCurrentFrame;
    updateAreaRectHeightCurrentFrame = spriteHeightCurrentFrame;
    if (spriteColourBitmapMain != NULL) {
        PresentLayeredSprite(arg_0, &layeredSurfaceMain, screenXCurrentFrame, screenYCurrentFrame, spriteColourBitmapMain, spriteMaskBitmapMain, spriteXInResourceImageCurrentFrame, spriteYInResourceImageCurrentFrame, spriteWidthCurrentFrame, spriteHeightCurrentFrame, ufoBeamHeight, alienTransformPending != 0);
    }
#else
    currentSpriteFramebufferIndex ^= 1;
    var_2 = GetDC(NULL);
    SelectPalette(var_2, windowPaletteInUse, FALSE);
    var_4 = CreateCompatibleDC(var_2);
    var_6 = CreateCompatibleDC(var_2);
    SelectPalette(var_6, windowPaletteInUse, FALSE);
    SelectPalette(var_4, windowPaletteInUse, FALSE);
    var_16 = max(screenXCurrentFrame, screenXPreviousFrame);
    var_14 = max(screenYCurrentFrame, screenYPreviousFrame);
    var_12 = min(spriteWidthCurrentFrame + screenXCurrentFrame, spriteWidthPreviousFrame + screenXPreviousFrame) - var_16;
    var_10 = min(screenYCurrentFrame + spriteHeightCurrentFrame, screenYPreviousFrame + spriteHeightPreviousFrame) - var_14;
    if (var_12 <= 0 || var_10 <= 0) {
        unused_A7FA = 1;
        if (unused_A7D4 != 0) {
            unused_A7D4 = 0;
        }
        updateAreaRectXCurrentFrame = screenXCurrentFrame;
        updateAreaRectYCurrentFrame = screenYCurrentFrame;
        updateAreaRectWidthCurrentFrame = spriteWidthCurrentFrame;
        updateAreaRectHeightCurrentFrame = spriteHeightCurrentFrame;
        SelectObject(var_4, doubleBufferMain[currentSpriteFramebufferIndex]);
        BitBlt(var_4, 0, 0, updateAreaRectWidthCurrentFrame, updateAreaRectHeightCurrentFrame, var_2, updateAreaRectXCurrentFrame, updateAreaRectYCurrentFrame, SRCCOPY);
    } else {
        unused_A7FA = 0;
        updateAreaRectXCurrentFrame = min(screenXCurrentFrame, screenXPreviousFrame);
        updateAreaRectYCurrentFrame = min(screenYCurrentFrame, screenYPreviousFrame);
        updateAreaRectWidthCurrentFrame = max(spriteWidthCurrentFrame + screenXCurrentFrame, spriteWidthPreviousFrame + screenXPreviousFrame) - updateAreaRectXCurrentFrame;
        updateAreaRectHeightCurrentFrame = max(screenYCurrentFrame + spriteHeightCurrentFrame, screenYPreviousFrame + spriteHeightPreviousFrame) - updateAreaRectYCurrentFrame;
        SelectObject(var_4, doubleBufferMain[currentSpriteFramebufferIndex]);
        BitBlt(var_4, 0, 0, updateAreaRectWidthCurrentFrame, updateAreaRectHeightCurrentFrame, var_2, updateAreaRectXCurrentFrame, updateAreaRectYCurrentFrame, SRCCOPY);
        var_1E = max(updateAreaRectXCurrentFrame, updateAreaRectXPreviousFrame);
        var_1C = max(updateAreaRectYCurrentFrame, updateAreaRectYPreviousFrame);
        var_1A = min(updateAreaRectWidthCurrentFrame + updateAreaRectXCurrentFrame, updateAreaRectWidthPreviousFrame + updateAreaRectXPreviousFrame) - var_1E;
        var_18 = min(updateAreaRectYCurrentFrame + updateAreaRectHeightCurrentFrame, updateAreaRectYPreviousFrame + updateAreaRectHeightPreviousFrame) - var_1C;
        var_16 = max(0, var_1E - updateAreaRectXCurrentFrame);
        var_14 = max(0, var_1C - updateAreaRectYCurrentFrame);
        var_E = max(0, var_1E - updateAreaRectXPreviousFrame);
        var_C = max(0, var_1C - updateAreaRectYPreviousFrame);
        if (var_1A > 0 && var_18 > 0) {
            SelectObject(var_6, doubleBufferMain[LOBYTE(currentSpriteFramebufferIndex) - 0xFF & 1]);
            BitBlt(var_4, var_16, var_14, var_1A, var_18, var_6, var_E, var_C, SRCCOPY);
        }
    }
    if (spriteColourBitmapMain != NULL) {
        SelectObject(var_6, spriteRenderTargetMain);
        BitBlt(var_6, 0, 0, updateAreaRectWidthCurrentFrame, updateAreaRectHeightCurrentFrame, var_4, 0, 0, SRCCOPY);
        var_16 = max(0, screenXCurrentFrame - updateAreaRectXCurrentFrame);
        var_14 = max(0, screenYCurrentFrame - updateAreaRectYCurrentFrame);
        if (spriteMaskBitmapMain != NULL) {
            SelectObject(var_4, spriteMaskBitmapMain);
            BitBlt(var_6, var_16, var_14, spriteWidthCurrentFrame, spriteHeightCurrentFrame, var_4, spriteXInResourceImageCurrentFrame, spriteYInResourceImageCurrentFrame, SRCAND);
            SelectObject(var_4, spriteColourBitmapMain);
            BitBlt(var_6, var_16, var_14, spriteWidthCurrentFrame, spriteHeightCurrentFrame, var_4, spriteXInResourceImageCurrentFrame, spriteYInResourceImageCurrentFrame, SRCPAINT);
        } else {
            SelectObject(var_4, spriteColourBitmapMain);
            BitBlt(var_6, var_16, var_14, spriteWidthCurrentFrame, spriteHeightCurrentFrame, var_4, spriteXInResourceImageCurrentFrame, spriteYInResourceImageCurrentFrame, SRCCOPY);
        }
        renderOrUpdateWindowFlag = 1;
        unusedCa5E = 1;
        MoveWindow(arg_0, updateAreaRectXCurrentFrame, updateAreaRectYCurrentFrame, updateAreaRectWidthCurrentFrame, updateAreaRectHeightCurrentFrame + ufoBeamHeight, TRUE);
        unusedCa5E = 0;
    }
    DeleteDC(var_4);
    DeleteDC(var_6);
#endif
    updateAreaRectXPreviousFrame = updateAreaRectXCurrentFrame;
    updateAreaRectYPreviousFrame = updateAreaRectYCurrentFrame;
    updateAreaRectWidthPreviousFrame = updateAreaRectWidthCurrentFrame;
    updateAreaRectHeightPreviousFrame = updateAreaRectHeightCurrentFrame;
    screenXPreviousFrame = screenXCurrentFrame;
    screenYPreviousFrame = screenYCurrentFrame;
    spriteWidthPreviousFrame = spriteWidthCurrentFrame;
    spriteHeightPreviousFrame = spriteHeightCurrentFrame;
    spriteColourBitmapPreviousFrame = spriteColourBitmapMain;
    spriteXInResourceImagePreviousFrame = spriteXInResourceImageCurrentFrame;
    spriteYInResourceImagePreviousFrameUnused = spriteYInResourceImageCurrentFrame;
#ifndef _WIN32
    ReleaseDC(NULL, var_2);
#endif
}

/* Render UFO beam (if any) and present render targets onto window. */
void RenderUfoBeamIfAnyPresentRenderTargetsOntoWindow(HWND arg_0)
{
    HDC var_2;
    HDC var_4;
    RECT var_C;
    HDC var_E;
#ifdef WIN32
    HDC screen;
#endif
    if (renderOrUpdateWindowFlag == 0) {
        return;
    }
    renderOrUpdateWindowFlag = 0;
    var_2 = GetDC(arg_0);
    SelectPalette(var_2, windowPaletteInUse, FALSE);
    RealizePalette(var_2);
    var_4 = CreateCompatibleDC(var_2);
    SelectPalette(var_4, windowPaletteInUse, FALSE);
    SelectObject(var_4, spriteRenderTargetMain);
    BitBlt(var_2, 0, 0, updateAreaRectWidthCurrentFrame, updateAreaRectHeightCurrentFrame, var_4, 0, 0, SRCCOPY);
    if (ufoBeamHeight != 0) {
        if (ufoBeamColorBitmap == NULL) {
            ufoBeamColorBitmap = CreateCompatibleBitmap(var_2, 40, screenHeight * 4 / 5);
            if (ufoBeamColorBitmap == NULL) {
                goto loc_398B;
            }
        }
        if (ufoBeamRenderTarget == NULL) {
            ufoBeamRenderTarget = CreateCompatibleBitmap(var_2, 40, screenHeight * 4 / 5);
            if (ufoBeamRenderTarget == NULL) {
                goto loc_398B;
            }
        }
        if (ufoBeamMaskBrush == NULL) {
            if (alienTransformPending != 0) {
                ufoBeamMaskBrush = CreateSolidBrush(RGB(255, 64, 64));
            } else {
                ufoBeamMaskBrush = CreateSolidBrush(RGB(255, 255, 0));
            }
        }
        if (ufoBeamPaintBrush == NULL) {
            if (alienTransformPending != 0) {
                ufoBeamPaintBrush = CreateSolidBrush(RGB(160, 0, 0));
            } else {
                ufoBeamPaintBrush = CreateSolidBrush(RGB(128, 128, 0));
            }
        }
        var_E = CreateCompatibleDC(var_2);
        SelectObject(var_E, ufoBeamRenderTarget);
#ifdef _WIN32
        /* Screen contents with height of only 40 pixels can be captured from window device context on Windows 10. Capture directly from screen instead. */
        screen = GetDC(NULL);
        BitBlt(var_E, 0, 0, 40, ufoBeamHeight, screen, updateAreaRectXCurrentFrame, updateAreaRectYCurrentFrame + 40, SRCCOPY);
        ReleaseDC(NULL, screen);
#else
        BitBlt(var_E, 0, 0, 40, ufoBeamHeight, var_2, 0, 40, SRCCOPY);
#endif
        var_C.left = 0;
        var_C.top = 0;
        var_C.right = 40;
        var_C.bottom = ufoBeamHeight;
        SelectObject(var_4, ufoBeamColorBitmap);
        FillRect(var_4, &var_C, ufoBeamMaskBrush);
        BitBlt(var_E, 0, 0, 40, ufoBeamHeight, var_4, 0, 0, SRCAND);
        FillRect(var_4, &var_C, ufoBeamPaintBrush);
        BitBlt(var_E, 0, 0, 40, ufoBeamHeight, var_4, 0, 0, SRCPAINT);
        BitBlt(var_2, 0, 40, 40, ufoBeamHeight, var_E, 0, 0, SRCCOPY);
        DeleteDC(var_E);
        DeleteDC(var_4);
    } else {
        if (ufoBeamPaintBrush != NULL) {
            DeleteObject(ufoBeamPaintBrush);
            ufoBeamPaintBrush = NULL;
        }
        if (ufoBeamMaskBrush != NULL) {
            DeleteObject(ufoBeamMaskBrush);
            ufoBeamMaskBrush = NULL;
        }
        if (ufoBeamRenderTarget != NULL) {
            DeleteObject(ufoBeamRenderTarget);
            ufoBeamRenderTarget = NULL;
        }
        if (ufoBeamColorBitmap != NULL) {
            DeleteObject(ufoBeamColorBitmap);
            ufoBeamColorBitmap = NULL;
        }
        DeleteDC(var_4);
    }
    ReleaseDC(arg_0, var_2);
    return;
loc_398B:
    ReleaseDC(arg_0, var_2);
}

/* Unused. */
void Func(HWND arg_0, int arg_2, int arg_4, int arg_6, int arg_8)
{
    MoveWindow(arg_0, arg_2, arg_4, arg_6, arg_8, FALSE);
    MoveWindow(arg_0, 0, 0, 0, 0, TRUE);
}

/* Find if a window has a match in known instance list. */
BOOL IsWindowInKnownInstanceList(HWND arg_0)
{
    int var_2;
    if (knownInstanceCount == 0) {
        return FALSE;
    }
    for (var_2 = 0; var_2 < 8; var_2 += 1) {
        if (knownInstanceWindows[var_2] == arg_0 && knownInstanceWindows[var_2] != NULL) {
            return TRUE;
        }
    }
    return FALSE;
}

/* Find X-coordinate of possible collision with other instances. Return zero when no collision detected. */
int FindXCoordinatePossibleCollisionOtherInstancesReturnZeroWhenNoCollisionDetected(int arg_0, int arg_2, int arg_4, int arg_6)
{
    int var_2;
    int var_4;
    int var_6;
    int var_8;
    int var_A;
    for (var_2 = 0; var_2 < 8; var_2 += 1) {
        if (knownInstanceWindows[var_2] != NULL) {
            var_4 = (short)GetWindowWord(knownInstanceWindows[var_2], 0);
            var_8 = (short)GetWindowWord(knownInstanceWindows[var_2], 2);
            var_6 = var_4 + 40;
            var_A = var_8 + 40;
            if (var_6 == 0) {
                continue;
            }
            if ((var_8 <= arg_4 && var_A > arg_4 || var_8 < arg_6 && var_A > arg_6) && var_6 > arg_0 && var_6 <= arg_2 && arg_2 > arg_0) {
                return var_6;
            }
            if ((var_8 <= arg_4 && var_A > arg_4 || var_8 < arg_6 && var_A > arg_6) && var_4 >= arg_2 && var_4 < arg_0 && arg_2 < arg_0) {
                return var_4;
            }
        }
    }
    return 0;
}

/* Find HWND of another instance overlapping the given rect. */
HWND FindCollidedOtherInstanceWindow(int arg_0, int arg_2, int arg_4, int arg_6)
{
    int var_2;
    int var_4;
    int var_6;
    int var_8;
    int var_A;
    /* Match DetectCollisionOtherInstancesActionControlledFlag hit box expansion. */
    if (arg_2 < arg_0) {
        arg_0 += 40;
        arg_2 = arg_0 - 80;
    } else {
        arg_2 = arg_0 + 80;
    }
    for (var_2 = 0; var_2 < 8; var_2 += 1) {
        if (knownInstanceWindows[var_2] != NULL) {
            var_4 = (short)GetWindowWord(knownInstanceWindows[var_2], 0);
            var_8 = (short)GetWindowWord(knownInstanceWindows[var_2], 2);
            var_6 = var_4 + 40;
            var_A = var_8 + 40;
            if (var_6 == 0) {
                continue;
            }
            if ((var_8 <= arg_4 && var_A > arg_4 || var_8 < arg_6 && var_A > arg_6) && var_6 > arg_0 && var_6 <= arg_2 && arg_2 > arg_0) {
                return knownInstanceWindows[var_2];
            }
            if ((var_8 <= arg_4 && var_A > arg_4 || var_8 < arg_6 && var_A > arg_6) && var_4 >= arg_2 && var_4 < arg_0 && arg_2 < arg_0) {
                return knownInstanceWindows[var_2];
            }
        }
    }
    return NULL;
}

/* Map normal sheep sprite index (sheets 101-111) to alien sheets 112-122. */
int AggressiveSpriteIndex(int arg_0)
{
    if (arg_0 >= 0 && arg_0 < 176) {
        return arg_0 + 176;
    }
    return arg_0;
}

/* Face the nearest other sheep instance (for alien chase). */
void FaceNearestOtherSheep(void)
{
    int i;
    int ox;
    int dist;
    int bestDist = 0x7fffffff;
    int bestX = spriteX;
    int found = 0;
    for (i = 0; i < 8; i += 1) {
        if (knownInstanceWindows[i] == NULL) {
            continue;
        }
        ox = (short)GetWindowWord(knownInstanceWindows[i], 0);
        dist = ox - spriteX;
        if (dist < 0) {
            dist = -dist;
        }
        if (dist < bestDist) {
            bestDist = dist;
            bestX = ox;
            found = 1;
        }
    }
    if (found != 0) {
        /* facingDirection > 0 moves left (decreasing X). */
        facingDirection = (bestX < spriteX) ? 1 : -1;
    }
}

/* Populate known instance list by searching for visible windows with name match. */
void PopulateKnownInstanceListSearchingVisibleWindowsNameMatch(HWND arg_0)
{
    HWND var_2;
    UINT var_4;
    int var_6;
    int var_8;
    char var_48[64];
    for (var_6 = 0; var_6 < 8; var_6 += 1) {
        knownInstanceWindows[var_6] = NULL;
    }
    var_2 = GetDesktopWindow();
    var_4 = GW_CHILD;
    var_6 = 0;
    var_8 = 0;
    while ((var_2 = GetWindow(var_2, var_4)) != NULL && var_6 < 64) {
        var_4 = GW_HWNDNEXT;
        if (var_2 == arg_0) {
            continue;
        }
        if ((GetWindowLong(var_2, GWL_STYLE) & WS_VISIBLE) != 0) {
            GetWindowText(var_2, var_48, 16);
            if (lstrcmp(var_48, "Screen Mate") == 0) {
                knownInstanceWindows[var_8] = var_2;
                var_8 += 1;
                if (var_8 > 8) {
                    return;
                }
            }
            var_6 += 1;
        }
    }
    knownInstanceCount = var_8;
}

/* Populate known instance list by searching for visible windows with name match, then notify other instances of self creation. */
BOOL PopulateKnownInstanceListAndNotify(HWND arg_0)
{
    HWND var_2;
    UINT var_4;
    int var_6;
    int var_8;
    char var_48[64];
    var_2 = GetDesktopWindow();
    var_4 = GW_CHILD;
    var_6 = 0;
    var_8 = 0;
    while ((var_2 = GetWindow(var_2, var_4)) != NULL && var_6 < 64) {
        var_4 = GW_HWNDNEXT;
        if (var_2 == arg_0) {
            continue;
        }
        if ((GetWindowLong(var_2, GWL_STYLE) & WS_VISIBLE) != 0) {
            GetWindowText(var_2, var_48, 16);
            if (lstrcmp(var_48, "Screen Mate") == 0) {
                knownInstanceWindows[var_8] = var_2;
                var_8 += 1;
                if (var_8 > 8) {
                    return FALSE;
                }
            }
            var_6 += 1;
        }
    }
    knownInstanceCount = var_8;
    unusedCa48 = var_8;
    for (var_6 = 0; var_6 < knownInstanceCount; var_6 += 1) {
        SendMessage(knownInstanceWindows[var_6], WM_USER, (WPARAM)1, (LPARAM)arg_0);
    }
    return TRUE;
}

/* Notify other instances of self destruction. */
void NotifyOtherInstancesSelfDestruction(HWND arg_0)
{
    int var_2;
    for (var_2 = 0; var_2 < 8; var_2 += 1) {
        if (knownInstanceWindows[var_2] != NULL) {
            SendMessage(knownInstanceWindows[var_2], WM_USER, (WPARAM)2, (LPARAM)arg_0);
        }
    }
}

/* Add window into known instance list. */
void AddWindowKnownInstanceList(HWND arg_0)
{
    int var_2;
    for (var_2 = 0; var_2 < 8; var_2 += 1) {
        if (knownInstanceWindows[var_2] == NULL) {
            knownInstanceCount += 1;
            knownInstanceWindows[var_2] = arg_0;
            break;
        }
    }
}

/* Remove window from known instance list. */
void RemoveWindowKnownInstanceList(HWND arg_0)
{
    int var_2;
    for (var_2 = 0; var_2 < 8; var_2 += 1) {
        if (knownInstanceWindows[var_2] == arg_0) {
            knownInstanceCount -= 1;
            knownInstanceWindows[var_2] = NULL;
            break;
        }
    }
}

/* Populate known visible window list. */
void PopulateKnownVisibleWindowList(void)
{
    HWND var_2;
    UINT var_4;
    int var_6;
    var_2 = GetDesktopWindow();
    var_4 = GW_CHILD;
    var_6 = 0;
    while ((var_2 = GetWindow(var_2, var_4)) != NULL && var_6 < 32) {
        var_4 = GW_HWNDNEXT;
        if (var_2 == selfInstanceWindowHandle) {
            continue;
        }
        if ((GetWindowLong(var_2, GWL_STYLE) & WS_VISIBLE) != 0) {
            GetWindowRect(var_2, &visibleWindowList[var_6].rect);
            visibleWindowList[var_6].window = var_2;
            var_6 += 1;
        }
    }
    visibleWindowCount = var_6;
}

/* Find X-coordinate of possible collision with which visible window. */
int FindXCoordinatePossibleCollisionWhichVisibleWindow(HWND * arg_0, int arg_2, int arg_4, int arg_6, int arg_8)
{
    int var_2;
    int var_4;
    RECT var_C;
    for (var_2 = 0; var_2 < visibleWindowCount; var_2 += 1) {
        if (arg_8 > arg_6) {
            if (visibleWindowList[var_2].rect.right >= arg_6 && visibleWindowList[var_2].rect.right < arg_8 && visibleWindowList[var_2].rect.top < arg_2 && visibleWindowList[var_2].rect.bottom > arg_4) {
                for (var_4 = 0; var_4 < var_2; var_4 += 1) {
                    if (visibleWindowList[var_4].rect.top <= arg_2 && visibleWindowList[var_4].rect.bottom >= arg_4 && visibleWindowList[var_4].rect.left <= arg_6 && visibleWindowList[var_4].rect.right >= arg_8) {
                        break;
                    }
                }
                if (var_4 == var_2) {
                    if (IsWindow(visibleWindowList[var_2].window)) {
                        GetWindowRect(visibleWindowList[var_2].window, &var_C);
                        if (visibleWindowList[var_2].rect.right == var_C.right) {
                            *arg_0 = visibleWindowList[var_2].window;
                            return visibleWindowList[var_2].rect.right;
                        }
                    }
                }
            }
        } else {
            if (visibleWindowList[var_2].rect.left <= arg_6 && visibleWindowList[var_2].rect.left > arg_8 && visibleWindowList[var_2].rect.top < arg_2 && visibleWindowList[var_2].rect.bottom > arg_4) {
                for (var_4 = 0; var_4 < var_2; var_4 += 1) {
                    if (visibleWindowList[var_4].rect.top <= arg_2 && visibleWindowList[var_4].rect.bottom >= arg_4 && visibleWindowList[var_4].rect.left <= arg_6 && visibleWindowList[var_4].rect.right >= arg_8) {
                        break;
                    }
                }
                if (var_4 == var_2) {
                    if (IsWindow(visibleWindowList[var_2].window)) {
                        GetWindowRect(visibleWindowList[var_2].window, &var_C);
                        if (visibleWindowList[var_2].rect.left == var_C.left) {
                            *arg_0 = visibleWindowList[var_2].window;
                            return visibleWindowList[var_2].rect.left;
                        }
                    }
                }
            }
        }
    }
    return 0;
}

/* Find Y-coordinate of possible landing top edge of which visible window. */
int FindYCoordinatePossibleLandingTopEdgeWhichVisibleWindow(HWND * arg_0, int arg_2, int arg_4, int arg_6, int arg_8)
{
    int var_2;
    int var_4;
    for (var_2 = 0; var_2 < visibleWindowCount; var_2 += 1) {
        if (visibleWindowList[var_2].rect.top <= arg_2 && visibleWindowList[var_2].rect.top > arg_4 && visibleWindowList[var_2].rect.left < arg_8 && visibleWindowList[var_2].rect.right > arg_6 && visibleWindowList[var_2].rect.top > 10) {
            for (var_4 = 0; var_4 < var_2; var_4 += 1) {
                if (visibleWindowList[var_4].rect.left <= arg_6 && visibleWindowList[var_4].rect.right >= arg_8 && visibleWindowList[var_4].rect.top <= arg_4 && visibleWindowList[var_4].rect.bottom >= arg_2) {
                    break;
                }
            }
            if (var_4 == var_2) {
                *arg_0 = visibleWindowList[var_2].window;
                return visibleWindowList[var_2].rect.top;
            }
        }
    }
    if (arg_2 >= screenHeight && arg_4 <= screenHeight) {
        *arg_0 = NULL;
        return screenHeight;
    }
    return 0;
}

/* Get window top Y-coordinate if it is possible to land on the window. */
int GetWindowTopYCoordinateIfItIsPossibleLandWindow(HWND arg_0, int arg_2, int arg_4, int arg_6, int arg_8)
{
    RECT var_8;
    if (IsWindow(arg_0)) {
        GetWindowRect(arg_0, &var_8);
        if (var_8.top <= arg_2 && var_8.top > arg_4 && var_8.left < arg_8 && var_8.right > arg_6) {
            return var_8.top;
        }
    }
    if (arg_2 > screenHeight) {
        return -1;
    }
    return 0;
}

/* Play sound by resource ID and additional flags. */
void PlaySoundResourceIdAdditionalFlags(int arg_0, UINT arg_2, WORD arg_4)
{
    LPCSTR lpszSoundName;
    if (waveResourceHandle != NULL) {
        sndPlaySound(NULL, SND_SYNC);
        GlobalUnlock(waveResourceHandle);
        FreeResource(waveResourceHandle);
        waveResourceHandle = NULL;
    }
    waveResourceHandle = LoadResource(currentInstance, FindResource(currentInstance, MAKEINTRESOURCE(arg_0), "WAVE"));
    lpszSoundName = LockResource(waveResourceHandle);
    sndPlaySound(lpszSoundName, arg_2 | (SND_ASYNC | SND_MEMORY));
}

/* Stop playing sound. */
void StopPlayingSound(void)
{
    sndPlaySound(NULL, SND_SYNC);
}

/* Play sound by name. */
void PlaySoundName(LPCSTR lpszSoundName)
{
    sndPlaySound(lpszSoundName, SND_ASYNC);
}

/* Play sound by resource ID and additional flags (when option "Cry" enabled). */
void PlaySoundResourceIdAdditionalFlagsWhenOptionCryEnabled(int arg_0, UINT arg_2, WORD arg_4)
{
    if (cryEnabled != 0U) {
        PlaySoundResourceIdAdditionalFlags(arg_0, arg_2, arg_4);
    }
}

/* Point the 16 unflipped and 16 flipped sprite entries of a sheet at its loaded bitmaps. */
void LinkSheetSprites(int sheet)
{
    int cell;
    for (cell = 0; cell < 16; cell += 1) {
        spriteListSub[sheet * 16 + cell].bitmaps[0] = resourceList[sheet].info.bitmaps[0];
        spriteListSub[sheet * 16 + cell].bitmaps[1] = resourceList[sheet].info.bitmaps[1];
        spriteListSub[sheet * 16 + cell].width = 40;
        spriteListSub[sheet * 16 + cell].height = 40;
        spriteListSub[sheet * 16 + cell].x = cell * 40;
        spriteListSub[sheet * 16 + cell].y = 0;
        spriteListSub[sheet * 16 + cell + 512].bitmaps[0] = flippedResourceList[sheet].info.bitmaps[0];
        spriteListSub[sheet * 16 + cell + 512].bitmaps[1] = flippedResourceList[sheet].info.bitmaps[1];
        spriteListSub[sheet * 16 + cell + 512].width = 40;
        spriteListSub[sheet * 16 + cell + 512].height = 40;
        spriteListSub[sheet * 16 + cell + 512].x = (15 - cell) * 40;
        spriteListSub[sheet * 16 + cell + 512].y = 0;
    }
}

/* Generate sprites from loaded resource images. */
BOOL GenerateSpritesFromLoadedResourceImages(HDC arg_0)
{
    int var_2;
    for (var_2 = 0; var_2 < 32; var_2 += 1) {
        if (resourceList[var_2].resource == 0) {
            break;
        }
        if (!LoadSpriteImagesAndStoreHandles(arg_0, &resourceList[var_2].info, resourceList[var_2].resource, -1)) {
            return FALSE;
        }
        if (!LoadSpriteImagesAndStoreHandles(arg_0, &flippedResourceList[var_2].info, resourceList[var_2].resource, -3)) {
            return FALSE;
        }
        LinkSheetSprites(var_2);
    }
    return TRUE;
}

/* Sheets redrawn with acid colours: 112.bmp (spin), 119.bmp (roll), 125.bmp (upside-down roll). */
int acidSheets[3] = {11, 18, 24};

/* Reload the acid sheets with the current spritePaletteOverride and redraw the current frame. */
void ReloadAcidSheets(void)
{
    HDC dc;
    int i;
    int sheet;
    dc = GetDC(selfInstanceWindowHandle);
    for (i = 0; i < 3; i += 1) {
        sheet = acidSheets[i];
        ReleaseSpriteImages(&resourceList[sheet].info);
        ReleaseSpriteImages(&flippedResourceList[sheet].info);
        LoadSpriteImagesAndStoreHandles(dc, &resourceList[sheet].info, resourceList[sheet].resource, -1);
        LoadSpriteImagesAndStoreHandles(dc, &flippedResourceList[sheet].info, resourceList[sheet].resource, -3);
        LinkSheetSprites(sheet);
    }
    ReleaseDC(selfInstanceWindowHandle, dc);
    UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
}

/* Switch horns and eyes of the acid sheets to acidColours[colour]. */
void ApplyAcidColour(int colour)
{
    acidColourIndex = colour % 9;
    spritePaletteOverride = acidColours[acidColourIndex];
    ReloadAcidSheets();
}

/* Put the original colours back on the acid sheets and leave acid mode. */
void RestoreAcidColours(void)
{
    if (acidModeActive == 0 && spritePaletteOverride == NULL) {
        return;
    }
    if (acidModeActive != 0) {
        StopPlayingSound();
    }
    spritePaletteOverride = NULL;
    acidModeActive = 0;
    ReloadAcidSheets();
}

/* Release resource images. */
void ReleaseResourceImages(void)
{
    int var_2;
    for (var_2 = 0; var_2 < 32; var_2 += 1) {
        if (resourceList[var_2].resource == 0) {
            break;
        }
        ReleaseSpriteImages(&resourceList[var_2].info);
        if (resourceList[var_2].flags == 1) {
            ReleaseSpriteImages(&flippedResourceList[var_2].info);
        }
    }
}

/* Turn around when approaching screen border or otherwise with 1/20 probability.  */
void TurnAroundWhenApproachingScreenBorderOtherwise120Probability(void)
{
    if (facingDirection > 0 && spriteX < 0) {
        subWindowState = 24;
    }
    if (facingDirection < 0 && screenWidth - spriteListSub[spriteIndex].width < spriteX) {
        subWindowState = 24;
    }
    if (facingDirection > 0 && screenWidth - spriteListSub[spriteIndex].width > spriteX && rand() % 20 == 0) {
        subWindowState = 24;
    }
    if (facingDirection < 0 && spriteX > 0 && rand() % 20 == 0) {
        subWindowState = 24;
    }
}

/* Flag-controlled collision and turn around. */
void FlagControlledCollisionTurnAround(BOOL arg_0)
{
    if (gravityEnabled == 0) {
        if (facingDirection > 0 && spriteX < 0) {
            subWindowState = 30;
        }
        if (facingDirection < 0 && screenWidth - spriteListSub[spriteIndex].width < spriteX) {
            subWindowState = 30;
        }
    }
    if (arg_0) {
        if (facingDirection > 0 && screenWidth - 80 > spriteX && rand() % 20 == 0) {
            subWindowState = 24;
        }
        if (facingDirection < 0 && spriteX > 40 && rand() % 20 == 0) {
            subWindowState = 24;
        }
    }
}

/* Switch to standing sprite after certain frames. */
void SwitchToStandingSprite(void)
{
    if (animationFrameCounter-- <= 0) {
        subWindowState = 42;
    }
}

/* Process chime. */
void ProcessChime(void)
{
    struct tm * var_2;
    time_t var_6;
    DWORD var_A;
    int var_C;
    if (remainingChimeTimes != 0) {
        var_A = GetTickCount();
        if (dword_A834 + 1000 < var_A) {
            dword_A834 = var_A;
            remainingChimeTimes -= 1;
            if (remainingChimeTimes != 0) {
                PlaySoundResourceIdAdditionalFlags(108, 0U, 0);
            } else if (sleepingAfterTimeout != 0) {
                subWindowState = 113;
            } else {
                subWindowState = 1;
            }
        }
    } else {
        if (timeCheckPeriodCounter++ < 10) {
            return;
        }
        timeCheckPeriodCounter = 0;
        time(&var_6);
        var_2 = localtime(&var_6);
        var_C = var_2->tm_hour % 12;
        if (var_C == 0) {
            var_C = 12;
        }
        if (var_2->tm_min == 0 && var_C != currentTimeHour) {
            DestroySubwindow();
            dword_A834 = 0;
            currentTimeHour = var_C;
            remainingChimeTimes = currentTimeHour + 1;
            subWindowState = 81;
        }
    }
}

/* Update window position and sprite to be painted. */
void UpdateMainWindowSprite(int arg_0, int arg_2, int arg_4)
{
    SetWindowWord(selfInstanceWindowHandle, 0, (short)spriteX);
    SetWindowWord(selfInstanceWindowHandle, 2, (short)spriteY);
    if (alienModeActive != 0 || acidModeActive != 0) {
        int flip = (arg_4 >= 512) ? 512 : 0;
        int i = arg_4 - flip;
        i = AggressiveSpriteIndex(i);
        arg_4 = i + flip;
    }
    if (arg_4 >= 9 && arg_4 <= 14) {
        UpdateWindowPositionSpriteBeActuallyUsed(arg_0, arg_2, arg_4);
    } else if (facingDirection > 0) {
        UpdateWindowPositionSpriteBeActuallyUsed(arg_0, arg_2, arg_4);
    } else {
        UpdateWindowPositionSpriteBeActuallyUsed(arg_0, arg_2, arg_4 + 512);
    }
}

/* Update window position and sprite to be painted (sub). */
void UpdateSubWindowSprite(int arg_0, int arg_2, int arg_4)
{
    if (arg_4 >= 9 && arg_4 <= 14) {
        UpdateWindowPositionSpriteBeActuallyUsed2(arg_0, arg_2, arg_4);
    } else if (facingDirectionSub > 0) {
        UpdateWindowPositionSpriteBeActuallyUsed2(arg_0, arg_2, arg_4);
    } else {
        UpdateWindowPositionSpriteBeActuallyUsed2(arg_0, arg_2, arg_4 + 512);
    }
}

/* Return TRUE if the window handle is NULL or if the handle contains an existing window. */
BOOL IsNullOrValidWindow(HWND arg_0)
{
    if (arg_0 == NULL) {
        return TRUE;
    } else {
        return IsWindow(arg_0);
    }
}

/* Get window rect. If the window handle is NULL, get a screen rect located right under the current screen. */
void GetWindowRectOrScreenRect(HWND arg_0, LPRECT arg_2)
{
    if (arg_0 == NULL) {
        arg_2->left = 0;
        arg_2->right = screenWidth;
        arg_2->top = screenHeight;
        arg_2->bottom = screenHeight * 2;
    } else {
        GetWindowRect(arg_0, arg_2);
    }
}

/* Process when out of screen view or at different positions on top of visible window. */
void HandleOutOfViewOrTopPosition(int arg_0)
{
    RECT var_8;
    if (gravityEnabled == 0) {
        return;
    }
    if (landingTargetWindow != NULL) {
        if (!IsNullOrValidWindow(landingTargetWindow)) {
            if (arg_0 == 2) {
                subWindowState = 94;
            } else {
                subWindowState = 102;
            }
            return;
        }
        GetWindowRectOrScreenRect(landingTargetWindow, &var_8);
        if (var_8.top > landingTargetWindowRect.top || spriteX + 40 < var_8.left || var_8.right < spriteX) {
            if (arg_0 == 2) {
                subWindowState = 94;
            } else {
                subWindowState = 102;
            }
            return;
        }
        if (var_8.top < landingTargetWindowRect.top) {
            spriteY = var_8.top - spriteListSub[spriteIndex].height;
            landingTargetWindowRect.top = var_8.top;
            landingTargetWindowRect.bottom = var_8.bottom;
            landingTargetWindowRect.left = var_8.left;
            landingTargetWindowRect.right = var_8.right;
            UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
            return;
        }
        if (arg_0 == 1) {
            if (spriteX + 8 < var_8.left && facingDirection > 0) {
                subWindowState = 105;
                spriteX = var_8.left - 10;
                return;
            }
            if (spriteX + 32 >= var_8.right && facingDirection < 0) {
                subWindowState = 105;
                spriteX = var_8.right - 30;
                return;
            }
            if (rand() % 20 - 1 == 0 && screenHeight - spriteY > 100) {
                subWindowState = 104;
                return;
            }
        }
        if (arg_0 == 2) {
            if (spriteX + 32 < var_8.left || spriteX + 8 > var_8.right) {
                subWindowState = 94;
                return;
            }
        }
    }
    if (spriteListSub[spriteIndex].width + spriteX < 0 || spriteX > screenWidth) {
        subWindowState = 0;
        return;
    }
}

/* Process when climbing up side of a window. */
void HandleClimbingSideOfWindow(void)
{
    RECT var_8;
    if (gravityEnabled == 0) {
        return;
    }
    if (landingTargetWindow != NULL) {
        if (!IsNullOrValidWindow(landingTargetWindow)) {
            subWindowState = 102;
            return;
        }
        GetWindowRectOrScreenRect(landingTargetWindow, &var_8);
        if (var_8.right < landingTargetWindowRect.right && facingDirection > 0 || var_8.left > landingTargetWindowRect.left && facingDirection < 0) {
            subWindowState = 102;
            return;
        }
        if (var_8.right > landingTargetWindowRect.right && facingDirection > 0 || var_8.left < landingTargetWindowRect.left && facingDirection < 0) {
            if (facingDirection > 0) {
                spriteX = var_8.right + 10;
            } else {
                spriteX = var_8.left - 50;
            }
            UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
            subWindowState = 102;
            return;
        }
    }
}

/* Detect collision with other instances, action controlled by a flag. */
void DetectCollisionOtherInstancesActionControlledFlag(int arg_0, int arg_2, int arg_4)
{
    if (arg_2 < arg_0) {
        arg_0 += 40;
        arg_2 = arg_0 - 80;
    } else {
        arg_2 = arg_0 + 80;
    }
    if (alienModeActive != 0) {
        HWND hit;
        if (alienKnockCooldown > 0) {
            alienKnockCooldown -= 1;
            return;
        }
        hit = FindCollidedOtherInstanceWindow(arg_0, arg_2, spriteY, spriteY + 40);
        if (hit != NULL) {
            SendMessage(hit, WM_USER, (WPARAM)3, (LPARAM)facingDirection);
            alienKnockCooldown = 4;
        }
        return;
    }
    if (FindXCoordinatePossibleCollisionOtherInstancesReturnZeroWhenNoCollisionDetected(arg_0, arg_2, spriteY, spriteY + 40) != 0) {
        if (arg_4 == 1) {
            subWindowState = 24;
        }
        if (arg_4 == 2) {
            subWindowState = 30;
        }
    }
}

/* Detect collision with other instances and find X-coordinate. */
int DetectCollisionOtherInstancesFindXCoordinate(int arg_0, int arg_2)
{
    if (arg_2 < arg_0) {
        arg_0 += 40;
        arg_2 = arg_0 - 80;
    } else {
        arg_2 = arg_0 + 80;
    }
    return FindXCoordinatePossibleCollisionOtherInstancesReturnZeroWhenNoCollisionDetected(arg_0, arg_2, spriteY, spriteY + 40);
}

/* Reinitialize state. */
void ResetSpriteState(void)
{
    subWindowState = 0;
}

/* Process state change on each timer expiration. */
void UpdateSpriteStateOnTimer(void)
{
    int var_2;
    int var_4;
    HWND var_6;
    HWND var_8;
    RECT var_10;
    POINT var_14;
    if (knownInstanceListUpdatePeriodCounter++ > 100) {
        PopulateKnownInstanceListSearchingVisibleWindowsNameMatch(selfInstanceWindowHandle);
        knownInstanceListUpdatePeriodCounter = 0;
    }
    if (chimeEnabled != 0) {
        ProcessChime();
    }
    if (acidModeActive != 0 && (subWindowState < 169 || subWindowState > 174)) {
        RestoreAcidColours();
    }
    if (alienModeActive != 0
        && subWindowState != 155
        && subWindowState != 156
        && subWindowState != 162 && subWindowState != 163) {
        if (--alienModeTicks <= 0 || (rand() % 400) == 0) {
            alienModeTicks = 0;
            subWindowState = 162;
        }
    }
stateLoopContinue:
    switch (subWindowState) {
    case 0:
        gravityEnabled = 0;
        srand((unsigned int)GetTickCount());
        spriteX = -80;
        spriteY = -80;
        subWindowState = 1;
    case 1:
        collisionVerticalSpeedUnused = 0;
        if (gravityAlwaysEnabled != 0U) {
            subWindowState = 2;
            goto stateLoopContinue;
        }
        ufoBeamHeight = 0;
        DestroySubwindow();
        if (sleepTimeoutAction != 0) {
            subWindowState = sleepTimeoutAction;
            sleepTimeoutAction = 0;
            break;
        }
        if (rand() % 20 == 5 && gravityEnabled == 0) {
            subWindowState = 85;
            break;
        }
        if (rand() % 20 == 5 && gravityEnabled == 0 && preventSpecialActions == 0) {
            subWindowState = 4;
            break;
        }
        subWindowState = normalActionTableGravityAlwaysOff[rand() % 80];
        if (alienModeActive != 0 && rand() % 5 != 0) {
            /* Alien: chase/run most of the time. */
            subWindowState = 7;
        }
        if (spriteX > screenWidth || spriteX < -40 || spriteY < -40 || spriteY > screenHeight) {
            if ((rand() & 1) == 0) {
                facingDirection = 1;
                spriteX = screenWidth + unusedCa4C;
                spriteY = rand() % (screenHeight - 64) + unusedCa4E;
            } else {
                facingDirection = -1;
                spriteX = -40;
                spriteY = rand() % (screenHeight - 64) + unusedCa4E;
            }
            subWindowState = 11;
        }
        break;
    case 2:
        gravityEnabled = 1;
        ufoBeamHeight = 0;
        DestroySubwindow();
        if (sleepTimeoutAction != 0) {
            subWindowState = sleepTimeoutAction;
            sleepTimeoutAction = 0;
            break;
        }
        subWindowState = normalActionTableGravityAlwaysOn[rand() % 80];
        if (alienModeActive != 0 && rand() % 5 != 0) {
            subWindowState = 7;
        }
        if (spriteX > screenWidth || spriteX < -40 || spriteY < -40 || spriteY > screenHeight) {
            if (rand() % 5 == 0 && preventSpecialActions == 0) {
                subWindowState = 6;
                break;
            }
            landingTargetWindow = GetActiveWindow();
            if (landingTargetWindow == selfInstanceWindowHandle || landingTargetWindow == knownInstanceWindows[8] || landingTargetWindow == NULL || IsWindowInKnownInstanceList(landingTargetWindow)) {
                subWindowState = 3;
                goto stateLoopContinue;
            }
            GetWindowRectOrScreenRect(landingTargetWindow, &landingTargetWindowRect);
            if (landingTargetWindowRect.top < 10) {
                subWindowState = 3;
                goto stateLoopContinue;
            }
            spriteX = (rand() % landingTargetWindowRect.right - landingTargetWindowRect.left) / 3 + (landingTargetWindowRect.right - landingTargetWindowRect.left) / 2 + landingTargetWindowRect.left - 20;
            spriteY = -40;
            bounceWhenFalling = 0;
            verticalSpeed = 0;
            horizontalSpeed = 0;
            fallActionCaseNumber = rand() % 2;
            subWindowState = 92;
            if (rand() % 3 == 0) {
                subWindowState = 3;
                goto stateLoopContinue;
            }
        }
        break;
    case 3:
        gravityEnabled = 1;
        spriteX = rand() % (screenWidth - 40);
        spriteY = -(rand() % 20 - (-40));
        bounceWhenFalling = 0;
        verticalSpeed = 0;
        horizontalSpeed = 0;
        fallActionCaseNumber = rand() % 2;
        if (rand() % 3 == 0) {
            PlaceWindowTopmostPosition(selfInstanceWindowHandle);
        }
        subWindowState = 97;
        break;
    case 153:
        break;
    case 154:
        break;
    case 4:
        if (screenWidth / 2 - 20 > spriteX) {
            facingDirection = 1;
        } else {
            facingDirection = -1;
        }
        spriteIndex = 4;
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        subWindowState = 5;
        break;
    case 5:
        if (framePeriodCounter++ < 1) {
            break;
        }
        framePeriodCounter = 0;
        spriteX -= facingDirection * 16;
        spriteIndex = spriteIndex == 4 ? 5 : 4;
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        if (spriteX < -40 || spriteX > screenWidth) {
            subWindowState = 6;
        }
        break;
    case 6:
        subWindowState = specialActionTable[rand() % 8];
        break;
    case 7:
        collisionEnabled = 0;
        if ((rand() & 1) == 0) {
            collisionEnabled = 1;
        }
        if (alienModeActive != 0) {
            collisionEnabled = 1;
            FaceNearestOtherSheep();
            animationFrameCounter = rand() % 20 + 30;
        } else {
            animationFrameCounter = rand() % 10 + 10;
        }
        if (collisionEnabled != 0) {
            PopulateKnownVisibleWindowList();
        }
        spriteIndex = 4;
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        subWindowState = 8;
        break;
    case 8:
        if (framePeriodCounter++ < 1) {
            break;
        }
        framePeriodCounter = 0;
        {
            int runStep = (alienModeActive != 0) ? 24 : 16;
            if (alienModeActive != 0 && (animationFrameCounter & 3) == 0) {
                FaceNearestOtherSheep();
            }
            if (collisionEnabled != 0) {
                if (facingDirection > 0) {
                    var_2 = FindXCoordinatePossibleCollisionWhichVisibleWindow(&var_6, spriteY, spriteY + 40, -(facingDirection * runStep - spriteX), spriteX);
                } else {
                    var_2 = FindXCoordinatePossibleCollisionWhichVisibleWindow(&var_6, spriteY, spriteY + 40, -(facingDirection * runStep - spriteX) + 40, spriteX + 40);
                }
                if (var_2 != 0) {
                    if (facingDirection > 0) {
                        spriteX = var_2;
                    } else {
                        spriteX = var_2 - 40;
                    }
                    UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
                    subWindowState = 30;
                    break;
                }
            }
            if (unusedA82C == 0) {
                spriteX -= facingDirection * runStep;
            }
            spriteIndex = spriteIndex == 4 ? 5 : 4;
            UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
            if (rand() % 50 == 0 && gravityEnabled != 0) {
                subWindowState = 9;
            }
            FlagControlledCollisionTurnAround(TRUE);
            SwitchToStandingSprite();
            HandleOutOfViewOrTopPosition(2);
            DetectCollisionOtherInstancesActionControlledFlag(-(facingDirection * runStep - spriteX), facingDirection * runStep + spriteX, 2);
        }
        break;
    case 9:
        verticalSpeed = -11;
        horizontalSpeed = -(facingDirection * 8);
        yCoordinateMemory = spriteY;
        subWindowState = 10;
    case 10:
        spriteX += horizontalSpeed;
        spriteY += verticalSpeed;
        verticalSpeed += 2;
        if (verticalSpeed >= -1 && verticalSpeed <= 1) {
            spriteIndex = 23;
        } else if (verticalSpeed < -1) {
            spriteIndex = 30;
        } else {
            spriteIndex = 24;
        }
        if (yCoordinateMemory <= spriteY) {
            spriteY = yCoordinateMemory;
            subWindowState = 7;
        }
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        FlagControlledCollisionTurnAround(FALSE);
        DetectCollisionOtherInstancesActionControlledFlag(horizontalSpeed + spriteX, spriteX - horizontalSpeed, 2);
        if (subWindowState == 30 && yCoordinateMemory != spriteY) {
            collisionVerticalSpeedUnused = spriteY - yCoordinateMemory;
        }
        break;
    case 11:
        collisionEnabled = 0;
        if ((gravityEnabled & !(rand() & 1)) != 0) {
            collisionEnabled = 1;
        }
        if (collisionEnabled != 0) {
            PopulateKnownVisibleWindowList();
        }
        animationFrameCounter = rand() % 10 + 10;
        spriteIndex = 2;
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        subWindowState = 12;
        break;
    case 12:
        if (framePeriodCounter++ < 1) {
            break;
        }
        framePeriodCounter = 0;
        if (collisionEnabled != 0) {
            if (facingDirection > 0) {
                var_2 = FindXCoordinatePossibleCollisionWhichVisibleWindow(&var_6, spriteY, spriteY + 40, -(facingDirection * 6 - spriteX), spriteX);
            } else {
                var_2 = FindXCoordinatePossibleCollisionWhichVisibleWindow(&var_6, spriteY, spriteY + 40, -(facingDirection * 6 - spriteX) + 40, spriteX + 40);
            }
            if (var_2 != 0) {
                if (facingDirection > 0) {
                    spriteX = var_2;
                } else {
                    spriteX = var_2 - 40;
                }
                landingTargetWindow = var_6;
                GetWindowRectOrScreenRect(landingTargetWindow, &landingTargetWindowRect);
                targetYWindowEdgeAttachment = landingTargetWindowRect.top - 12;
                gravityEnabled = 1;
                targetXWindowEdgeAttachment = spriteX;
                spriteIndex = 30;
                PlaceWindowTopAnother(selfInstanceWindowHandle, landingTargetWindow);
                subWindowState = 89;
                break;
            }
        }
        if (unusedA82C == 0) {
            spriteX -= facingDirection * 6;
        }
        spriteIndex = spriteIndex == 2 ? 3 : 2;
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        TurnAroundWhenApproachingScreenBorderOtherwise120Probability();
        SwitchToStandingSprite();
        HandleOutOfViewOrTopPosition(1);
        DetectCollisionOtherInstancesActionControlledFlag(-(facingDirection * 6 - spriteX), facingDirection * 6 + spriteX, 1);
        break;
    case 13:
        randomCaseNumberForAction = rand() % 2;
        animationFrameCounter = rand() % 4 + 4;
        if (randomCaseNumberForAction != 0) {
            spriteIndex = 88;
        } else {
            spriteIndex = 86;
        }
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        subWindowState = 14;
        break;
    case 14:
        if (framePeriodCounter++ < 3) {
            break;
        }
        framePeriodCounter = 0;
        if (unusedA82C == 0) {
            spriteX -= facingDirection * 6;
        }
        if (randomCaseNumberForAction != 0) {
            spriteIndex = spriteIndex == 88 ? 89 : 88;
        } else {
            spriteIndex = spriteIndex == 86 ? 87 : 86;
        }
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        TurnAroundWhenApproachingScreenBorderOtherwise120Probability();
        SwitchToStandingSprite();
        HandleOutOfViewOrTopPosition(1);
        break;
    case 15:
        randomCaseNumberForAction = rand() % 2;
        animationFrameCounter = rand() % 3 + 3;
        if (randomCaseNumberForAction != 0) {
            spriteIndex = 54;
        } else {
            spriteIndex = 52;
        }
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        subWindowState = 16;
        break;
    case 16:
        if (framePeriodCounter++ < 3) {
            break;
        }
        framePeriodCounter = 0;
        if (randomCaseNumberForAction != 0) {
            spriteIndex = spriteIndex == 54 ? 55 : 54;
        } else {
            spriteIndex = spriteIndex == 52 ? 53 : 52;
        }
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        TurnAroundWhenApproachingScreenBorderOtherwise120Probability();
        SwitchToStandingSprite();
        HandleOutOfViewOrTopPosition(0);
        break;
    case 17:
        spriteIndex = 6;
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        subWindowState = 18;
        break;
    case 18:
        if (framePeriodCounter++ < 1) {
            break;
        }
        framePeriodCounter = 0;
        spriteIndex += 1;
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        if (spriteIndex == 8) {
            spriteIndex = 0;
            subWindowState = 19;
            animationFrameCounter = rand() % 8 + 8;
        }
        HandleOutOfViewOrTopPosition(0);
        break;
    case 19:
        if (framePeriodCounter++ < 4) {
            break;
        }
        framePeriodCounter = 0;
        spriteIndex = spriteIndex == 0 ? 1 : 0;
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        SwitchToStandingSprite();
        HandleOutOfViewOrTopPosition(0);
        break;
    case 20:
        randomCaseNumberForAction = rand() % 3;
        if (randomCaseNumberForAction == 0) {
            spriteIndex = 6;
        } else if (randomCaseNumberForAction == 1) {
            spriteIndex = 31;
        } else {
            spriteIndex = 73;
        }
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        subWindowState = 21;
        framePeriodCounter = rand() % 15 + rand() % 15;
        HandleOutOfViewOrTopPosition(0);
        break;
    case 21:
        HandleOutOfViewOrTopPosition(0);
        if (framePeriodCounter-- > 0) {
            break;
        }
        subWindowState = 22;
        animationFrameCounter = 0;
        break;
    case 22:
        spriteIndex = blinkAnimationFrames[randomCaseNumberForAction][animationFrameCounter];
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        animationFrameCounter += 1;
        if (animationFrameCounter > 7) {
            subWindowState = 23;
            framePeriodCounter = rand() % 15 + rand() % 15;
        }
        HandleOutOfViewOrTopPosition(0);
        break;
    case 23:
        HandleOutOfViewOrTopPosition(0);
        if (framePeriodCounter-- > 0) {
            break;
        }
        subWindowState = 1;
        break;
    case 24:
        if (framePeriodCounter++ < 1) {
            break;
        }
        framePeriodCounter = 0;
        spriteIndex = 3;
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        if ((rand() & 1) != 0) {
            randomCaseNumberForAction = 0;
        } else {
            randomCaseNumberForAction = 1;
        }
        subWindowState = 25;
        animationFrameCounter = 0;
        break;
    case 25:
        if (framePeriodCounter++ < 1) {
            break;
        }
        framePeriodCounter = 0;
        if (randomCaseNumberForAction != 0) {
            if (facingDirection > 0) {
                spriteIndex = animationFrameCounter + 9;
            } else {
                spriteIndex = 11 - animationFrameCounter;
            }
        } else {
            if (facingDirection > 0) {
                spriteIndex = animationFrameCounter + 12;
            } else {
                spriteIndex = 14 - animationFrameCounter;
            }
        }
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        animationFrameCounter += 1;
        if (animationFrameCounter > 2) {
            facingDirection = -facingDirection;
            subWindowState = 26;
        }
        HandleOutOfViewOrTopPosition(0);
        break;
    case 26:
        if (framePeriodCounter++ < 1) {
            break;
        }
        framePeriodCounter = 0;
        spriteIndex = 3;
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        subWindowState = 1;
        HandleOutOfViewOrTopPosition(0);
        break;
    case 27:
        verticalSpeed = -10;
        horizontalSpeed = facingDirection * 8;
        yCoordinateMemory = spriteY;
        collisionAnimationFrameIndex = 0;
        subWindowState = 28;
    case 28:
        spriteX += horizontalSpeed;
        spriteY += verticalSpeed;
        verticalSpeed += 2;
        spriteIndex = collisionAnimationFramesWithHeightOffset[collisionAnimationFrameIndex];
        collisionAnimationFrameIndex += 1;
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        if (spriteIndex == 64) {
            fallActionCaseNumber = 3;
            subWindowState = 99;
            break;
        }
        break;
    case 29:
        framePeriodCounter = 0;
        animationFrameCounter = 0;
        randomCaseNumberForAction = 0;
        if ((rand() & 7) == 0) {
            randomCaseNumberForAction = 1;
        }
        if (rand() % 5 == 0) {
            randomCaseNumberForAction = 2;
        }
        subWindowState = 32;
        if (randomCaseNumberForAction != 0) {
            subWindowState = 34;
        }
        goto stateLoopContinue;
    case 30:
        if (gravityEnabled != 0) {
            subWindowState = 27;
            goto stateLoopContinue;
        } else {
            subWindowState = 24;
            goto stateLoopContinue;
        }
        framePeriodCounter = 0;
        animationFrameCounter = 0;
        randomCaseNumberForAction = 0;
        if ((rand() & 7) == 0) {
            randomCaseNumberForAction = 1;
        }
        if (rand() % 5 == 0) {
            randomCaseNumberForAction = 2;
        }
        subWindowState = 31;
    case 31:
        DetectCollisionOtherInstancesActionControlledFlag(facingDirection * 10 + spriteX, spriteX, 2);
        if (subWindowState == 30) {
            if (animationFrameCounter != 0) {
                collisionVerticalSpeedUnused -= collisionAnimationFramesWithHeightOffset[animationFrameCounter + 9];
            }
            break;
        }
        spriteIndex = collisionAnimationFramesWithHeightOffset[animationFrameCounter];
        UpdateMainWindowSprite(spriteX, spriteY - collisionAnimationFramesWithHeightOffset[animationFrameCounter + 10], spriteIndex);
        animationFrameCounter += 1;
        if (randomCaseNumberForAction != 0 && spriteIndex == 66) {
            if (collisionVerticalSpeedUnused != 0) {
                spriteY -= collisionVerticalSpeedUnused;
                spriteX += facingDirection * 10;
                UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
            }
            collisionSpinFrameCounterUnused = 3;
            subWindowState = 34;
            break;
        }
        if (animationFrameCounter > 8) {
            subWindowState = 32;
            break;
        }
        spriteX += facingDirection * 10;
        break;
    case 32:
        HandleOutOfViewOrTopPosition(0);
        if (framePeriodCounter++ < 8) {
            break;
        }
        framePeriodCounter = 0;
        facingDirection = -facingDirection;
        spriteIndex = 93;
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        subWindowState = 33;
        break;
    case 33:
        HandleOutOfViewOrTopPosition(0);
        if (framePeriodCounter++ < 15) {
            break;
        }
        framePeriodCounter = 0;
        subWindowState = 1;
        break;
    case 34:
        spriteX += facingDirection * 8;
        if (spriteIndex == 70) {
            spriteIndex = 63;
        } else {
            spriteIndex += 1;
        }
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        if (randomCaseNumberForAction == 2 && spriteIndex == 70) {
            subWindowState = 69;
            break;
        }
        if (spriteX > screenWidth || spriteX < -40) {
            subWindowState = 1;
        }
        DetectCollisionOtherInstancesActionControlledFlag(facingDirection * 8 + spriteX, -(facingDirection * 8 - spriteX), 2);
        if (subWindowState == 30) {
            if (collisionSpinFrameCounterUnused-- > 0) {
                facingDirection = -facingDirection;
                subWindowState = 34;
            } else {
                subWindowState = 34;
            }
        }
        HandleOutOfViewOrTopPosition(2);
        break;
    case 35:
        if (framePeriodCounter++ < 1) {
            break;
        }
        framePeriodCounter = 0;
        spriteIndex = 3;
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        subWindowState = 37;
        animationFrameCounter = 0;
        break;
    case 36:
        break;
    case 37:
        if (framePeriodCounter++ < 1) {
            break;
        }
        framePeriodCounter = 0;
        if (facingDirection > 0) {
            spriteIndex = animationFrameCounter + 12;
        } else {
            spriteIndex = 14 - animationFrameCounter;
        }
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        animationFrameCounter += 1;
        if (animationFrameCounter > 1) {
            spriteIndex = 103;
            subWindowState = 38;
        }
        HandleOutOfViewOrTopPosition(0);
        break;
    case 38:
        if (framePeriodCounter++ < 1) {
            break;
        }
        framePeriodCounter = 0;
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        spriteIndex += 1;
        if (spriteIndex > 104) {
            animationFrameCounter = 0;
            subWindowState = 39;
            break;
        }
        HandleOutOfViewOrTopPosition(0);
        break;
    case 39:
        if (animationFrameCounter == 0) {
            if (framePeriodCounter++ < 10) {
                break;
            }
            framePeriodCounter = 0;
        } else {
            if (framePeriodCounter++ < 1) {
                break;
            }
            framePeriodCounter = 0;
        }
        if (animationFrameCounter <= 8 || animationFrameCounter >= 12 && animationFrameCounter <= 12) {
            spriteIndex = spriteIndex == 105 ? 106 : 105;
        } else {
            spriteIndex = 104;
        }
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        if (animationFrameCounter++ > 15) {
            subWindowState = 40;
            spriteIndex = 104;
        }
        HandleOutOfViewOrTopPosition(0);
        break;
    case 40:
        if (framePeriodCounter++ < 1) {
            break;
        }
        framePeriodCounter = 0;
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        if (--spriteIndex < 103) {
            animationFrameCounter = 0;
            subWindowState = 41;
            break;
        }
        HandleOutOfViewOrTopPosition(0);
        break;
    case 41:
        if (framePeriodCounter++ < 1) {
            break;
        }
        framePeriodCounter = 0;
        if (facingDirection > 0) {
            spriteIndex = 13 - animationFrameCounter;
        } else {
            spriteIndex = animationFrameCounter + 13;
        }
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        animationFrameCounter += 1;
        if (animationFrameCounter > 1) {
            subWindowState = 42;
        }
        HandleOutOfViewOrTopPosition(0);
        break;
    case 42:
        HandleOutOfViewOrTopPosition(0);
        if (framePeriodCounter++ < 1) {
            break;
        }
        framePeriodCounter = 0;
        spriteIndex = 3;
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        subWindowState = 1;
        animationFrameCounter = 0;
        break;
    case 43:
        PlaySoundResourceIdAdditionalFlagsWhenOptionCryEnabled(109, 0U, 0);
        animationFrameCounter = 0;
        subWindowState = 44;
    case 44:
        if (framePeriodCounter++ < 1) {
            break;
        }
        framePeriodCounter = 0;
        spriteIndex = yawnAnimationFrames[animationFrameCounter];
        animationFrameCounter += 1;
        if (spriteIndex == 0) {
            subWindowState = 1;
            break;
        }
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        HandleOutOfViewOrTopPosition(0);
        break;
    case 45:
        PlaySoundResourceIdAdditionalFlagsWhenOptionCryEnabled(108, 0U, 0);
        animationFrameCounter = 0;
        subWindowState = 46;
    case 46:
        if (framePeriodCounter++ < 0) {
            break;
        }
        framePeriodCounter = 0;
        spriteIndex = baaAnimationFrames[animationFrameCounter];
        animationFrameCounter += 1;
        if (spriteIndex == 0) {
            subWindowState = 1;
            break;
        }
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        HandleOutOfViewOrTopPosition(0);
        break;
    case 47:
        animationFrameCounter = 0;
        subWindowState = 48;
    case 48:
        if (framePeriodCounter++ < 0) {
            break;
        }
        framePeriodCounter = 0;
        if (animationFrameCounter == 2) {
            PlaySoundResourceIdAdditionalFlagsWhenOptionCryEnabled(110, 0U, 0);
        }
        spriteIndex = sneezeAnimationFrames[animationFrameCounter];
        animationFrameCounter += 1;
        if (spriteIndex == 0) {
            subWindowState = 1;
            break;
        }
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        HandleOutOfViewOrTopPosition(0);
        break;
    case 49:
        animationFrameCounter = 0;
        subWindowState = 50;
    case 50:
        if (framePeriodCounter++ < 1) {
            break;
        }
        framePeriodCounter = 0;
        spriteIndex = amazedAnimationFrames[animationFrameCounter];
        animationFrameCounter += 1;
        if (spriteIndex == 0) {
            subWindowState = 1;
            break;
        }
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        HandleOutOfViewOrTopPosition(0);
        break;
    case 51:
        animationFrameCounter = 0;
        subWindowState = 52;
    case 52:
        if (framePeriodCounter++ < 0) {
            break;
        }
        framePeriodCounter = 0;
        spriteIndex = spriteIndex == 56 ? 57 : 56;
        if (animationFrameCounter++ > 30) {
            spriteIndex = 3;
            subWindowState = 1;
        }
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        HandleOutOfViewOrTopPosition(0);
        break;
    case 53:
        animationFrameCounter = 0;
        subWindowState = 54;
        CreateSubwindow();
        facingDirectionSub = facingDirection;
        spriteYSub = spriteY;
        spriteIndexSub = 149;
        if (facingDirection > 0) {
            spriteXSub = spriteX - 40;
        } else {
            spriteXSub = spriteX + 40;
        }
        UpdateSubWindowSprite(spriteXSub, spriteYSub, spriteIndexSub);
        screenMateOnTopOfSubwindow = 1;
        PlaceWindowTopAnother(knownInstanceWindows[8], selfInstanceWindowHandle);
        break;
    case 54:
        if (framePeriodCounter++ < 2) {
            break;
        }
        framePeriodCounter = 0;
        spriteIndex = eatAnimationFrames[animationFrameCounter];
        animationFrameCounter += 1;
        if (spriteIndex == 2) {
            spriteX -= facingDirection * 8;
            UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
            break;
        }
        if (spriteIndex >= 149 && spriteIndex <= 153) {
            spriteIndexSub = spriteIndex;
            if (spriteIndexSub == 153) {
                spriteIndexSub = 173;
            }
            UpdateSubWindowSprite(spriteXSub, spriteYSub, spriteIndexSub);
            spriteIndex = eatAnimationFrames[animationFrameCounter];
            animationFrameCounter += 1;
        }
        if (spriteIndex == 0) {
            DestroySubwindow();
            subWindowState = 1;
            break;
        }
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        break;
    case 55:
        break;
        if (framePeriodCounter++ < 1) {
            break;
        }
        framePeriodCounter = 0;
        spriteX -= facingDirection * 6;
        spriteIndex = 2;
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        TurnAroundWhenApproachingScreenBorderOtherwise120Probability();
        SwitchToStandingSprite();
        HandleOutOfViewOrTopPosition(1);
        DetectCollisionOtherInstancesActionControlledFlag(-(facingDirection * 6 - spriteX), facingDirection * 6 + spriteX, 1);
        subWindowState = 54;
        break;
    case 56:
        animationFrameCounter = 0;
        subWindowState = 57;
    case 57:
        if (framePeriodCounter++ < 2) {
            break;
        }
        framePeriodCounter = 0;
        spriteIndex = eatAnimationFrames[animationFrameCounter];
        animationFrameCounter += 1;
        if (spriteIndex >= 149 && spriteIndex <= 153) {
            spriteIndex = eatAnimationFrames[animationFrameCounter];
            animationFrameCounter += 1;
        }
        if (animationFrameCounter >= 16) {
            subWindowState = 42;
            break;
        }
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        HandleOutOfViewOrTopPosition(0);
        break;
    case 58:
        if (framePeriodCounter++ < 1) {
            break;
        }
        framePeriodCounter = 0;
        spriteIndex = 3;
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        subWindowState = 59;
        animationFrameCounter = 0;
        break;
    case 59:
        if (framePeriodCounter++ < 1) {
            break;
        }
        framePeriodCounter = 0;
        if (facingDirection > 0) {
            spriteIndex = animationFrameCounter + 9;
        } else {
            spriteIndex = 11 - animationFrameCounter;
        }
        animationFrameCounter += 1;
        if (animationFrameCounter > 2) {
            spriteIndex = 34;
            framePeriodCounter = -10;
            subWindowState = 60;
            animationFrameCounter = 0;
        }
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        HandleOutOfViewOrTopPosition(0);
        break;
    case 60:
        if (framePeriodCounter++ < 0) {
            break;
        }
        framePeriodCounter = 0;
        spriteIndex = blinkAnimationFrames[5][animationFrameCounter];
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        animationFrameCounter += 1;
        if (animationFrameCounter > 7) {
            animationFrameCounter = 0;
            subWindowState = 61;
            framePeriodCounter = -5;
        }
        HandleOutOfViewOrTopPosition(0);
        break;
    case 61:
        if (framePeriodCounter++ < 1) {
            break;
        }
        framePeriodCounter = 0;
        if (facingDirection > 0) {
            spriteIndex = 10 - animationFrameCounter;
        } else {
            spriteIndex = animationFrameCounter + 10;
        }
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        animationFrameCounter += 1;
        if (animationFrameCounter > 1) {
            subWindowState = 42;
        }
        HandleOutOfViewOrTopPosition(0);
        break;
    case 64:
        break;
    case 65:
        spriteIndex = 3;
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        animationFrameCounter = 0;
        subWindowState = 66;
        break;
    case 66:
        if (framePeriodCounter++ < 1) {
            break;
        }
        framePeriodCounter = 0;
        if (animationFrameCounter == 0) {
            if (facingDirection > 0) {
                spriteIndex = 9;
            } else {
                spriteIndex = 11;
            }
        } else {
            spriteIndex = 10;
        }
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        if (animationFrameCounter++ > 0) {
            subWindowState = 67;
            randomDurationCounter = (rand() % 4 + 4) * 8;
            animationFrameCounter = 0;
            break;
        }
        HandleOutOfViewOrTopPosition(0);
        break;
    case 67:
        if (--animationFrameCounter < 0) {
            animationFrameCounter = 79;
        }
        spriteX -= facingDirection * 8;
        spriteIndex = rollAnimationFrames[animationFrameCounter % 8];
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        if (facingDirection > 0 && spriteX < 0) {
            subWindowState = 30;
        }
        if (facingDirection < 0 && screenWidth - spriteListSub[spriteIndex].width < spriteX) {
            subWindowState = 30;
        }
        if (--randomDurationCounter <= 0) {
            subWindowState = 68;
            animationFrameCounter = 0;
        }
        HandleOutOfViewOrTopPosition(2);
        DetectCollisionOtherInstancesActionControlledFlag(-(facingDirection * 8 - spriteX), facingDirection * 8 + spriteX, 2);
        break;
    case 68:
        if (framePeriodCounter++ < 1) {
            break;
        }
        framePeriodCounter = 0;
        if (animationFrameCounter == 1) {
            if (facingDirection > 0) {
                spriteIndex = 9;
            } else {
                spriteIndex = 11;
            }
        } else if (animationFrameCounter == 0) {
            spriteIndex = 10;
        } else {
            spriteIndex = 3;
        }
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        if (animationFrameCounter++ > 1) {
            subWindowState = 1;
            break;
        }
        HandleOutOfViewOrTopPosition(0);
        break;
    case 62:
        subWindowState = 63;
        animationFrameCounter = 0;
        break;
    case 63:
        if (framePeriodCounter++ < 2) {
            break;
        }
        framePeriodCounter = 0;
        spriteIndex = blushAnimationFrames[animationFrameCounter];
        if (spriteIndex == 0) {
            subWindowState = 1;
            break;
        }
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        animationFrameCounter += 1;
        HandleOutOfViewOrTopPosition(0);
        break;
    case 75:
        animationFrameCounter = rand() % 8 + 8;
        randomDurationCounter = animationFrameCounter;
        spriteIndex = 131;
        if (facingDirection > 0) {
            spriteIndex = 12;
        } else {
            spriteIndex = 14;
        }
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        subWindowState = 76;
        break;
    case 76:
        if (framePeriodCounter++ < 1) {
            break;
        }
        framePeriodCounter = 0;
        spriteIndex = 13;
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        subWindowState = 77;
        break;
    case 77:
        if (framePeriodCounter++ < 2) {
            break;
        }
        framePeriodCounter = 0;
        spriteIndex = spriteIndex == 131 ? 132 : 131;
        spriteY -= 8;
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        if (animationFrameCounter-- <= 0) {
            animationFrameCounter = randomDurationCounter;
            subWindowState = 78;
        }
        break;
    case 78:
        spriteIndex = 133;
        spriteY += 8;
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        if (animationFrameCounter-- <= 0) {
            subWindowState = 79;
        }
        break;
    case 79:
        if (framePeriodCounter++ < 10) {
            break;
        }
        framePeriodCounter = 0;
        subWindowState = 80;
        animationFrameCounter = 3;
        break;
    case 80:
        if (framePeriodCounter++ < 1) {
            break;
        }
        framePeriodCounter = 0;
        if (facingDirection > 0) {
            spriteIndex = getUpAnimationFramesLeft[animationFrameCounter];
            animationFrameCounter += 1;
        } else {
            spriteIndex = getUpAnimationFramesRight[animationFrameCounter];
            animationFrameCounter += 1;
        }
        if (spriteIndex == 0) {
            subWindowState = 1;
            break;
        }
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        break;
    case 69:
        framePeriodCounter = 0;
        animationFrameCounter = 0;
        subWindowState = 70;
    case 70:
        if (facingDirection > 0) {
            spriteIndex = spinAnimationFrames[animationFrameCounter % 8];
        } else {
            spriteIndex = spinAnimationFrames[(animationFrameCounter + 4) % 8];
        }
        if (spriteIndex == 2) {
            spriteIndex = 3;
            if (facingDirection > 0) {
                facingDirection = -facingDirection;
                UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
                facingDirection = -facingDirection;
            } else {
                UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
            }
        } else if (spriteIndex == 3) {
            if (facingDirection < 0) {
                facingDirection = -facingDirection;
                UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
                facingDirection = -facingDirection;
            } else {
                UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
            }
        } else {
            UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        }
        if (animationFrameCounter++ >= 16) {
            spriteIndex = 70;
            UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
            subWindowState = 71;
        }
        HandleOutOfViewOrTopPosition(0);
        break;
    case 71:
        HandleOutOfViewOrTopPosition(0);
        if (framePeriodCounter++ < 14) {
            break;
        }
        framePeriodCounter = 0;
        spriteIndex = 96;
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        subWindowState = 72;
        break;
    case 72:
        HandleOutOfViewOrTopPosition(0);
        if (framePeriodCounter++ < 30) {
            break;
        }
        framePeriodCounter = 0;
        spriteIndex = 3;
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        subWindowState = 1;
        break;
    case 73:
        animationFrameCounter = 0;
        subWindowState = 74;
    case 74:
        if (framePeriodCounter++ < 2) {
            break;
        }
        framePeriodCounter = 0;
        spriteIndex = rollOverAnimationFrames[animationFrameCounter];
        animationFrameCounter += 1;
        if (spriteIndex == 0) {
            subWindowState = 1;
            break;
        }
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        HandleOutOfViewOrTopPosition(0);
        break;
    case 81:
        spriteIndex = 4;
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        subWindowState = 82;
        break;
    case 82:
        framePeriodCounter = 0;
        spriteIndex = spriteIndex == 4 ? 5 : 4;
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        break;
    case 83:
        break;
    case 84:
        subWindowState = 0;
        break;
    case 85:
        landingTargetWindow = GetActiveWindow();
        if (landingTargetWindow == selfInstanceWindowHandle || landingTargetWindow == knownInstanceWindows[8] || landingTargetWindow == NULL || IsWindowInKnownInstanceList(landingTargetWindow)) {
            subWindowState = 1;
            break;
        }
        GetWindowRectOrScreenRect(landingTargetWindow, &landingTargetWindowRect);
        if (landingTargetWindowRect.top < 10) {
            subWindowState = 1;
            break;
        }
        if (facingDirection > 0 && landingTargetWindowRect.right < spriteX && landingTargetWindowRect.top < spriteY && spriteY + 40 < landingTargetWindowRect.bottom || facingDirection < 0 && spriteX + 40 < landingTargetWindowRect.left && landingTargetWindowRect.top < spriteY && spriteY + 40 < landingTargetWindowRect.bottom) {
            subWindowState = 87;
            break;
        }
        targetXWindowEdgeAttachment = (rand() % landingTargetWindowRect.right - landingTargetWindowRect.left) / 3 + (landingTargetWindowRect.right - landingTargetWindowRect.left) / 2 + landingTargetWindowRect.left - 20;
        targetYWindowEdgeAttachment = landingTargetWindowRect.top - 40;
        if (screenWidth / 2 - 20 > spriteX) {
            facingDirection = 1;
        } else {
            facingDirection = -1;
        }
        spriteIndex = 4;
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        subWindowState = 86;
        break;
    case 86:
        if (framePeriodCounter++ < 1) {
            break;
        }
        framePeriodCounter = 0;
        spriteX -= facingDirection * 16;
        spriteIndex = spriteIndex == 4 ? 5 : 4;
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        if (spriteX < -40 || spriteX > screenWidth) {
            if (!IsNullOrValidWindow(landingTargetWindow)) {
                subWindowState = 1;
                break;
            }
            if (rand() % 3 == 0) {
                subWindowState = 3;
                goto stateLoopContinue;
            }
            bounceWhenFalling = 0;
            subWindowState = 92;
            gravityEnabled = 1;
            spriteX = targetXWindowEdgeAttachment;
            spriteY = -40;
            verticalSpeed = 0;
            horizontalSpeed = 0;
            fallActionCaseNumber = rand() % 2;
            if (fallActionCaseNumber != 0) {
                horizontalSpeed = -(facingDirection * 3);
            }
            PlaceWindowTopAnother(selfInstanceWindowHandle, landingTargetWindow);
        }
        break;
    case 87:
        PlaceWindowTopAnother(selfInstanceWindowHandle, landingTargetWindow);
        if (facingDirection > 0) {
            targetXWindowEdgeAttachment = landingTargetWindowRect.right;
            targetYWindowEdgeAttachment = landingTargetWindowRect.top;
        } else {
            targetXWindowEdgeAttachment = landingTargetWindowRect.left - 40;
            targetYWindowEdgeAttachment = landingTargetWindowRect.top;
        }
        subWindowState = 88;
    case 88:
        if (framePeriodCounter++ < 1) {
            break;
        }
        framePeriodCounter = 0;
        spriteX -= facingDirection * 16;
        spriteIndex = spriteIndex == 4 ? 5 : 4;
        if (targetXWindowEdgeAttachment >= spriteX && facingDirection > 0 || targetXWindowEdgeAttachment <= spriteX && facingDirection < 0) {
            if (!IsNullOrValidWindow(landingTargetWindow)) {
                subWindowState = 1;
                break;
            }
            GetWindowRectOrScreenRect(landingTargetWindow, &var_10);
            if (var_10.left == landingTargetWindowRect.left && var_10.right == landingTargetWindowRect.right && var_10.top < spriteY && spriteY + 40 < var_10.bottom) {
                targetYWindowEdgeAttachment = var_10.top - 12;
                gravityEnabled = 1;
                spriteX = targetXWindowEdgeAttachment;
                spriteIndex = 30;
                subWindowState = 89;
                break;
            } else {
                subWindowState = 1;
                break;
            }
        }
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        break;
    case 89:
        if (framePeriodCounter++ < 1) {
            break;
        }
        framePeriodCounter = 0;
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        spriteY -= 6;
        spriteIndex = spriteIndex == 15 ? 16 : 15;
        if (targetYWindowEdgeAttachment >= spriteY) {
            subWindowState = 90;
            break;
        }
        HandleClimbingSideOfWindow();
        break;
    case 90:
        if (framePeriodCounter++ < 2) {
            break;
        }
        framePeriodCounter = 0;
        spriteX -= facingDirection * 8;
        spriteY = targetYWindowEdgeAttachment - 20;
        spriteIndex = 76;
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        subWindowState = 91;
        break;
    case 91:
        if (framePeriodCounter++ < 2) {
            break;
        }
        framePeriodCounter = 0;
        spriteX += facingDirection * -24;
        spriteY -= 8;
        spriteIndex = 3;
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        subWindowState = 11;
        break;
    case 92:
        verticalSpeed += 4;
        yCoordinateMemory = spriteY;
        spriteX += horizontalSpeed;
        spriteY += verticalSpeed;
        if ((var_4 = GetWindowTopYCoordinateIfItIsPossibleLandWindow(landingTargetWindow, spriteListSub[spriteIndex].height + spriteY, spriteListSub[spriteIndex].height + yCoordinateMemory, spriteX, spriteListSub[spriteIndex].width + spriteX)) != 0) {
            if (var_4 == -1) {
                UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
                subWindowState = 0;
                break;
            }
            spriteY = var_4 - spriteListSub[spriteIndex].height;
            if (verticalSpeed < 64 && bounceWhenFalling == 0 || verticalSpeed < 8) {
                SetWindowPos(selfInstanceWindowHandle, landingTargetWindow, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
                bounceWhenFalling = 0;
                animationFrameCounter = 0;
                subWindowState = 93;
                if (verticalSpeed < 36) {
                    spriteIndex = 49;
                    framePeriodCounter = -4;
                } else {
                    if ((rand() & 3) == 0) {
                        spriteIndex = 48;
                    } else {
                        spriteIndex = 42;
                    }
                    framePeriodCounter = -12;
                }
                UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
                break;
            } else {
                verticalSpeed = verticalSpeed * 2 / -3;
                bounceWhenFalling = 1;
            }
        }
        if (fallActionCaseNumber != 0) {
            spriteIndex = spriteIndex == 4 ? 5 : 4;
        } else {
            spriteIndex = 42;
        }
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        break;
    case 93:
        if (framePeriodCounter++ < 1) {
            break;
        }
        framePeriodCounter = 0;
        if (fallActionCaseNumber != 0) {
            subWindowState = 11;
            spriteIndex = 2;
            break;
        }
        if (animationFrameCounter == 0) {
            spriteIndex = 13;
        } else if (animationFrameCounter == 1) {
            if (facingDirection > 0) {
                spriteIndex = 12;
            } else {
                spriteIndex = 14;
            }
        } else if (animationFrameCounter == 2) {
            spriteIndex = 3;
        }
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        if (animationFrameCounter++ >= 2) {
            subWindowState = 11;
        }
        break;
    case 94:
        gravityEnabled = 1;
        verticalSpeed = 0;
        horizontalSpeed = -(facingDirection * 8);
        fallActionCaseNumber = 1;
        subWindowState = 99;
        goto stateLoopContinue;
    case 95:
        gravityEnabled = 1;
        verticalSpeed = 0;
        horizontalSpeed = -(facingDirection * 3);
        fallActionCaseNumber = 1;
        subWindowState = 99;
        goto stateLoopContinue;
    case 96:
        gravityEnabled = 1;
        verticalSpeed = 0;
        horizontalSpeed = 0;
        fallActionCaseNumber = 0;
        subWindowState = 99;
        goto stateLoopContinue;
    case 97:
        gravityEnabled = 1;
        verticalSpeed = 0;
        horizontalSpeed = 0;
        fallActionCaseNumber = 1;
        subWindowState = 99;
        goto stateLoopContinue;
    case 98:
        gravityEnabled = 1;
        verticalSpeed = 0;
        horizontalSpeed = 0;
        fallActionCaseNumber = 2;
        subWindowState = 99;
        goto stateLoopContinue;
    case 99:
        PopulateKnownVisibleWindowList();
        verticalSpeed += 4;
        yCoordinateMemory = spriteY;
        spriteX += horizontalSpeed;
        spriteY += verticalSpeed;
        if (yCoordinateMemory > screenHeight) {
            UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
            subWindowState = 0;
            break;
        }
        if ((var_4 = FindYCoordinatePossibleLandingTopEdgeWhichVisibleWindow(&landingTargetWindow, spriteListSub[spriteIndex].height + spriteY, spriteListSub[spriteIndex].height + yCoordinateMemory, spriteX, spriteListSub[spriteIndex].width + spriteX)) != 0) {
            if (!IsNullOrValidWindow(landingTargetWindow)) {
                UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
                subWindowState = 0;
                break;
            }
            GetWindowRectOrScreenRect(landingTargetWindow, &landingTargetWindowRect);
            spriteY = var_4 - spriteListSub[spriteIndex].height;
            if (fallActionCaseNumber == 3) {
                spriteIndex = 66;
                UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
                subWindowState = 29;
                break;
            }
            if (verticalSpeed < 64 && bounceWhenFalling == 0 || verticalSpeed < 8) {
                if (landingTargetWindow != NULL) {
                    SetWindowPos(selfInstanceWindowHandle, landingTargetWindow, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
                }
                bounceWhenFalling = 0;
                animationFrameCounter = 0;
                subWindowState = 100;
                if (fallActionCaseNumber == 4) {
                    /* Landing pose: handstand sheet cell 4 or 6. */
                    spriteIndex = randomCaseNumberForAction;
                    framePeriodCounter = -8;
                } else if (verticalSpeed < 36) {
                    spriteIndex = 49;
                    framePeriodCounter = -4;
                } else {
                    if ((rand() & 3) == 0) {
                        spriteIndex = 48;
                    } else {
                        spriteIndex = 42;
                    }
                    framePeriodCounter = -10;
                }
                if (fallActionCaseNumber == 2) {
                    if (verticalSpeed < 36) {
                        spriteIndex = 41;
                    } else {
                        spriteIndex = 45;
                    }
                }
                UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
                break;
            } else {
                if ((rand() & 7) == 0 && bounceWhenFalling == 0) {
                    bounceWhenFalling = 0;
                    animationFrameCounter = 0;
                    subWindowState = 100;
                    spriteIndex = 48;
                    framePeriodCounter = -12;
                    if (fallActionCaseNumber == 2) {
                        spriteIndex = 45;
                    }
                    UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
                    break;
                }
                verticalSpeed = verticalSpeed * 2 / -3;
                bounceWhenFalling = 1;
            }
        }
        if (fallActionCaseNumber == 2) {
            spriteIndex = spriteIndex == 40 ? 41 : 40;
        } else if (fallActionCaseNumber == 4) {
            /* Alien drop mid-air: normal fall run sprites (land pose only on impact). */
            spriteIndex = spriteIndex == 4 ? 5 : 4;
        } else if (fallActionCaseNumber == 1) {
            spriteIndex = spriteIndex == 4 ? 5 : 4;
        } else if (fallActionCaseNumber == 0) {
            spriteIndex = 42;
        } else {
            spriteIndex = collisionAnimationFramesWithHeightOffset[collisionAnimationFrameIndex];
            collisionAnimationFrameIndex += 1;
            if (spriteIndex == 66) {
                collisionAnimationFrameIndex -= 1;
            }
        }
        if (fallActionCaseNumber == 3 && DetectCollisionOtherInstancesFindXCoordinate(spriteX, spriteX - horizontalSpeed) != 0) {
            facingDirection = -facingDirection;
            subWindowState = 30;
            break;
        }
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        break;
    case 100:
        if (framePeriodCounter++ < 1) {
            break;
        }
        framePeriodCounter = 0;
        if (fallActionCaseNumber == 4) {
            /* Alien drop landed: enter normal hub while alien mode stays active. */
            if (gravityAlwaysEnabled != 0U) {
                subWindowState = 2;
            } else {
                subWindowState = 1;
            }
            break;
        }
        if (fallActionCaseNumber == 1) {
            subWindowState = 11;
            spriteIndex = 2;
            break;
        }
        if (fallActionCaseNumber == 2) {
            animationFrameCounter = 0;
            subWindowState = 101;
            break;
        }
        if (animationFrameCounter == 0) {
            spriteIndex = 13;
        } else if (animationFrameCounter == 1) {
            if (facingDirection > 0) {
                spriteIndex = 12;
            } else {
                spriteIndex = 14;
            }
        } else if (animationFrameCounter == 2) {
            spriteIndex = 3;
        }
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        if (animationFrameCounter++ >= 2) {
            subWindowState = 11;
        }
        break;
    case 101:
        if (framePeriodCounter++ < 1) {
            break;
        }
        framePeriodCounter = 0;
        if (animationFrameCounter == 0) {
            spriteIndex = 31;
            framePeriodCounter = -8;
        } else if (animationFrameCounter == 2) {
            spriteIndex = 3;
        }
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        if (animationFrameCounter++ >= 6) {
            subWindowState = 11;
        }
        break;
    case 102:
        StopPlayingSound();
        animationFrameCounter = 6;
        spriteIndex = 3;
        randomCaseNumberForAction = 0;
        if (rand() % 3 == 0) {
            randomCaseNumberForAction = 1;
        }
        subWindowState = 103;
    case 103:
        if (randomCaseNumberForAction != 0) {
            spriteIndex = spriteIndex == 50 ? 51 : 50;
        } else {
            spriteIndex = spriteIndex == 4 ? 5 : 4;
        }
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        if (animationFrameCounter-- <= 0) {
            subWindowState = 97;
        }
        break;
    case 104:
        fallActionCaseNumber = 0;
        subWindowState = 106;
        goto stateLoopContinue;
    case 105:
        fallActionCaseNumber = 1;
        subWindowState = 106;
        goto stateLoopContinue;
    case 106:
        if (fallActionCaseNumber == 0) {
            var_14.x = spriteX;
            var_14.y = spriteY + 39;
            *(HWND *)&var_10 = WindowFromPoint(var_14);
            var_14.x = spriteX + 39;
            var_8 = WindowFromPoint(var_14);
            if (*(HWND *)&var_10 == selfInstanceWindowHandle && var_8 == selfInstanceWindowHandle) {
                PlaceWindowTopmostPosition(selfInstanceWindowHandle);
            } else if (*(HWND *)&var_10 == selfInstanceWindowHandle) {
                PlaceWindowTopAnother(selfInstanceWindowHandle, var_8);
            } else {
                PlaceWindowTopAnother(selfInstanceWindowHandle, *(HWND *)&var_10);
            }
            spriteIndex = 81;
        } else {
            spriteIndex = 78;
        }
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        subWindowState = 107;
        animationFrameCounter = 0;
        break;
    case 107:
        spriteIndex = blinkAnimationFrames[4 - fallActionCaseNumber][animationFrameCounter];
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        animationFrameCounter += 1;
        if (animationFrameCounter > 7) {
            if (fallActionCaseNumber != 0) {
                if ((rand() & 1) == 0) {
                    subWindowState = 111;
                } else {
                    subWindowState = 109;
                }
            } else {
                if ((rand() & 1) == 0) {
                    subWindowState = 111;
                } else {
                    subWindowState = 108;
                }
            }
        }
        break;
    case 108:
        if (framePeriodCounter++ < 10) {
            break;
        }
        framePeriodCounter = 0;
        spriteIndex = 3;
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        subWindowState = 1;
        break;
    case 109:
        horizontalSpeed = -(facingDirection * 14);
        spriteIndex = 23;
        spriteX += horizontalSpeed;
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        subWindowState = 95;
        animationFrameCounter = 0;
        break;
    case 110:
        spriteX += horizontalSpeed;
        horizontalSpeed += facingDirection;
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        if (animationFrameCounter++ > 3) {
            subWindowState = 95;
        }
        break;
    case 111:
        if (fallActionCaseNumber != 0) {
            spriteX += facingDirection * -26;
            spriteY += 35;
            facingDirection = -facingDirection;
        } else {
            randomCaseNumberForAction = rand() % 2;
            if (randomCaseNumberForAction != 0) {
                spriteY += 36;
            } else {
                spriteY += 20;
            }
        }
        animationFrameCounter = 0;
        subWindowState = 112;
    case 112:
        if (animationFrameCounter == 0) {
            if (framePeriodCounter++ < 10) {
                break;
            }
            framePeriodCounter = 0;
        } else {
            if (framePeriodCounter++ < 1) {
                break;
            }
            framePeriodCounter = 0;
        }
        if (fallActionCaseNumber != 0) {
            spriteIndex = spriteIndex == 40 ? 41 : 40;
        } else {
            spriteIndex = hangOnWindowTopEdgeAnimationFrames[randomCaseNumberForAction][animationFrameCounter % 4];
        }
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        animationFrameCounter += 1;
        if (animationFrameCounter > 12) {
            if (fallActionCaseNumber != 0) {
                subWindowState = 98;
            } else {
                subWindowState = 96;
            }
        }
        break;
    case 113:
        sleepingAfterTimeout = 1;
        spriteIndex = 6;
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        subWindowState = 114;
        break;
    case 114:
        if (framePeriodCounter++ < 1) {
            break;
        }
        framePeriodCounter = 0;
        spriteIndex += 1;
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        if (spriteIndex == 8) {
            spriteIndex = 0;
            subWindowState = 115;
        }
        HandleOutOfViewOrTopPosition(0);
        break;
    case 115:
        HandleOutOfViewOrTopPosition(0);
        if (framePeriodCounter++ < 4) {
            break;
        }
        framePeriodCounter = 0;
        spriteIndex = spriteIndex == 0 ? 1 : 0;
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        break;
    case 116:
        spriteX = screenWidth;
        spriteY = screenHeight * 7 / 8;
        spriteIndex = 4;
        facingDirection = 1;
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        subWindowState = 117;
        break;
    case 117:
        if (framePeriodCounter++ < 1) {
            break;
        }
        framePeriodCounter = 0;
        spriteX -= facingDirection * 16;
        spriteIndex = spriteIndex == 4 ? 5 : 4;
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        if (screenWidth / 2 - 20 >= spriteX) {
            subWindowState = 118;
        }
        break;
    case 118:
        CreateSubwindow();
        facingDirectionSub = -1;
        spriteXSub = -40;
        spriteYSub = screenHeight / 8;
        spriteIndexSub = 154;
        animationFrameCounter = 0;
        subWindowState = 119;
        screenMateOnTopOfSubwindow = 0;
        PlaceWindowTopmostPosition(selfInstanceWindowHandle);
        PlaceWindowTopmostPosition(knownInstanceWindows[8]);
        break;
    case 119:
        if (animationFrameCounter != 0) {
            spriteIndex = blinkAnimationFrames[2][animationFrameCounter];
            UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
            animationFrameCounter += 1;
            if (animationFrameCounter > 7) {
                animationFrameCounter = 0;
            }
        } else {
            spriteIndex = 73;
            UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
            if (rand() % 20 == 0) {
                animationFrameCounter = 1;
            }
        }
        if (framePeriodCounter++ < 1) {
            break;
        }
        framePeriodCounter = 0;
        spriteXSub -= facingDirectionSub * 16;
        spriteIndexSub = spriteIndexSub == 154 ? 155 : 154;
        UpdateSubWindowSprite(spriteXSub, spriteYSub, spriteIndexSub);
        if (spriteXSub > spriteX) {
            facingDirection = -1;
            UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        }
        if (spriteXSub > screenWidth) {
            DestroySubwindow();
            subWindowState = 120;
        }
        break;
    case 120:
        if (framePeriodCounter++ < 1) {
            break;
        }
        framePeriodCounter = 0;
        spriteX -= facingDirection * 16;
        spriteIndex = spriteIndex == 4 ? 5 : 4;
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        if (spriteX > screenWidth) {
            subWindowState = 1;
        }
        break;
    case 121:
        spriteX = screenWidth;
        spriteY = screenHeight * 7 / 8;
        spriteIndex = 4;
        facingDirection = 1;
        CreateSubwindow();
        facingDirectionSub = -1;
        spriteXSub = -40;
        spriteYSub = screenHeight * 7 / 8;
        spriteIndexSub = 154;
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        UpdateSubWindowSprite(spriteXSub, spriteYSub, spriteIndexSub);
        subWindowState = 122;
        screenMateOnTopOfSubwindow = 0;
        PlaceWindowTopmostPosition(selfInstanceWindowHandle);
        PlaceWindowTopmostPosition(knownInstanceWindows[8]);
        break;
    case 122:
        if (framePeriodCounter++ < 1) {
            break;
        }
        framePeriodCounter = 0;
        spriteX -= facingDirection * 16;
        spriteIndex = spriteIndex == 4 ? 5 : 4;
        spriteXSub -= facingDirectionSub * 16;
        spriteIndexSub = spriteIndexSub == 154 ? 155 : 154;
        if (spriteX - spriteXSub <= 46) {
            spriteX = screenWidth / 2 + 3;
            spriteXSub = screenWidth / 2 - 43;
            spriteIndex = 3;
            spriteIndexSub = 157;
            UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
            UpdateSubWindowSprite(spriteXSub, spriteYSub, spriteIndexSub);
            animationFrameCounter = 0;
            subWindowState = 123;
        } else {
            UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
            UpdateSubWindowSprite(spriteXSub, spriteYSub, spriteIndexSub);
        }
        break;
    case 123:
        if (framePeriodCounter++ < 3) {
            break;
        }
        framePeriodCounter = 0;
        spriteIndex = animationFrameCounter + 127;
        animationFrameCounter += 1;
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        if (animationFrameCounter >= 4) {
            subWindowState = 124;
        }
        break;
    case 124:
        if (framePeriodCounter++ < 4) {
            break;
        }
        framePeriodCounter = 0;
        fadeOutFrameCounter += 1;
        if (fadeOutFrameCounter > 8) {
            DestroySubwindow();
            animationFrameCounter = 0;
            subWindowState = 125;
        }
        break;
    case 125:
        spriteIndex = merry2AnimationFrames[animationFrameCounter];
        animationFrameCounter += 1;
        if (spriteIndex == 0) {
            subWindowState = 1;
            break;
        }
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        break;
    case 126:
        spriteX = screenWidth;
        spriteY = screenHeight * 7 / 8;
        spriteIndex = 4;
        facingDirection = 1;
        CreateSubwindow();
        facingDirectionSub = 1;
        spriteXSub = screenWidth + 46;
        spriteYSub = screenHeight * 7 / 8;
        spriteIndexSub = 154;
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        UpdateSubWindowSprite(spriteXSub, spriteYSub, spriteIndexSub);
        subWindowState = 127;
        screenMateOnTopOfSubwindow = 0;
        PlaceWindowTopmostPosition(selfInstanceWindowHandle);
        PlaceWindowTopmostPosition(knownInstanceWindows[8]);
        break;
    case 127:
        if (framePeriodCounter++ < 1) {
            break;
        }
        framePeriodCounter = 0;
        spriteX -= facingDirection * 16;
        spriteIndex = spriteIndex == 4 ? 5 : 4;
        spriteXSub -= facingDirectionSub * 16;
        spriteIndexSub = spriteIndexSub == 154 ? 155 : 154;
        if (spriteXSub < -40) {
            DestroySubwindow();
            subWindowState = 1;
        } else {
            UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
            UpdateSubWindowSprite(spriteXSub, spriteYSub, spriteIndexSub);
        }
        break;
    case 128:
        spriteX = screenWidth;
        spriteY = screenHeight * 7 / 8;
        spriteIndex = 4;
        facingDirection = 1;
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        subWindowState = 129;
        break;
    case 129:
        if (framePeriodCounter++ < 1) {
            break;
        }
        framePeriodCounter = 0;
        spriteX -= facingDirection * 16;
        spriteIndex = spriteIndex == 4 ? 5 : 4;
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        if (screenWidth / 2 - 20 >= spriteX) {
            subWindowState = 130;
        }
        break;
    case 130:
        CreateSubwindow();
        facingDirectionSub = -1;
        spriteXSub = -40;
        spriteYSub = screenHeight / 8;
        spriteIndexSub = 158;
        animationFrameCounter = 0;
        subWindowState = 131;
        screenMateOnTopOfSubwindow = 0;
        PlaceWindowTopmostPosition(selfInstanceWindowHandle);
        PlaceWindowTopmostPosition(knownInstanceWindows[8]);
        break;
    case 131:
        if (animationFrameCounter != 0) {
            spriteIndex = blinkAnimationFrames[2][animationFrameCounter];
            UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
            animationFrameCounter += 1;
            if (animationFrameCounter > 7) {
                animationFrameCounter = 0;
            }
        } else {
            spriteIndex = 73;
            UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
            if (rand() % 20 == 0) {
                animationFrameCounter = 1;
            }
        }
        spriteXSub -= facingDirectionSub * 16;
        if (spriteIndexSub == 161) {
            spriteIndexSub = 158;
        } else {
            spriteIndexSub += 1;
        }
        if (spriteXSub > spriteX) {
            spriteXSub = spriteX;
            spriteIndexSub = 162;
            subWindowState = 132;
        }
        UpdateSubWindowSprite(spriteXSub, spriteYSub, spriteIndexSub);
        break;
    case 132:
        spriteIndex = 73;
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        ufoBeamHeightSub += 40;
        if (spriteY - spriteYSub - 40 <= ufoBeamHeightSub) {
            ufoBeamHeightSub = spriteY - spriteYSub - 40;
            ufoBeamHeightSub -= 20;
            subWindowState = 133;
        }
        UpdateSubWindowSprite(spriteXSub, spriteYSub, spriteIndexSub);
        if (framePeriodCounter++ < 1) {
            break;
        }
        framePeriodCounter = 0;
        if (spriteIndexSub == 165) {
            spriteIndexSub = 162;
        } else {
            spriteIndexSub += 1;
        }
        break;
    case 133:
        ufoBeamHeightSub -= 20;
        if (ufoBeamHeightSub <= 0) {
            ufoBeamHeightSub = 0;
            spriteY = spriteYSub + 40;
            subWindowState = 134;
            spriteIndex = spriteIndex == 4 ? 5 : 4;
            UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
            spriteIndexSub = 158;
            UpdateSubWindowSprite(spriteXSub, spriteYSub, spriteIndexSub);
            break;
        }
        UpdateSubWindowSprite(spriteXSub, spriteYSub, spriteIndexSub);
        spriteIndex = spriteIndex == 4 ? 5 : 4;
        spriteY -= 20;
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        if (framePeriodCounter++ < 1) {
            break;
        }
        framePeriodCounter = 0;
        if (spriteIndexSub == 165) {
            spriteIndexSub = 162;
        } else {
            spriteIndexSub += 1;
        }
        break;
    case 134:
        spriteX = -80;
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        spriteXSub -= facingDirectionSub * 16;
        if (spriteIndexSub == 161) {
            spriteIndexSub = 158;
        } else {
            spriteIndexSub += 1;
        }
        if (spriteXSub > screenWidth) {
            DestroySubwindow();
            StopPlayingSound();
            if (alienTransformPending != 0) {
                /* No UFO return: alien sheep falls from the sky. */
                subWindowState = 156;
            } else {
                subWindowState = 1;
            }
            break;
        }
        UpdateSubWindowSprite(spriteXSub, spriteYSub, spriteIndexSub);
        break;
    case 135:
        spriteYSub = screenHeight * 7 / 8;
        facingDirection = -1;
        spriteX = -40;
        spriteY = screenHeight / 8;
        spriteIndex = 158;
        animationFrameCounter = 0;
        subWindowState = 136;
        break;
    case 136:
        spriteX -= facingDirection * 16;
        if (spriteIndex == 161) {
            spriteIndex = 158;
        } else {
            spriteIndex += 1;
        }
        if (screenWidth / 2 - 20 < spriteX) {
            spriteX = screenWidth / 2 - 20;
            spriteIndex = 162;
            subWindowState = 137;
        }
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        break;
    case 137:
        ufoBeamHeight += 40;
        if (spriteYSub - spriteY - 40 <= ufoBeamHeight) {
            ufoBeamHeight = spriteYSub - spriteY - 40;
            subWindowState = 138;
        }
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        if (framePeriodCounter++ < 1) {
            break;
        }
        framePeriodCounter = 0;
        if (spriteIndex == 165) {
            spriteIndex = 162;
        } else {
            spriteIndex += 1;
        }
        break;
    case 138:
        if (framePeriodCounter++ < 4) {
            break;
        }
        framePeriodCounter = 0;
        CreateSubwindow();
        spriteXSub = spriteX;
        spriteIndexSub = 167;
        UpdateSubWindowSprite(spriteXSub, spriteYSub, spriteIndexSub);
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        subWindowState = 139;
        screenMateOnTopOfSubwindow = 0;
        PlaceWindowTopmostPosition(selfInstanceWindowHandle);
        PlaceWindowTopmostPosition(knownInstanceWindows[8]);
        break;
    case 139:
        if (ufoBeamHeight != 0) {
            ufoBeamHeight -= 40;
            if (ufoBeamHeight <= 0) {
                spriteIndex = 158;
                ufoBeamHeight = 0;
            }
            if (spriteIndex == 165) {
                spriteIndex = 162;
            } else {
                spriteIndex += 1;
            }
        } else {
            spriteX -= facingDirection * 16;
            if (spriteIndex == 161) {
                spriteIndex = 158;
            } else {
                spriteIndex += 1;
            }
        }
        if (spriteX > screenWidth) {
            subWindowState = 140;
        }
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        if (framePeriodCounter++ < 1) {
            break;
        }
        framePeriodCounter = 0;
        spriteIndexSub = spriteIndexSub == 167 ? 168 : 167;
        UpdateSubWindowSprite(spriteXSub, spriteYSub, spriteIndexSub);
        break;
    case 140:
        spriteIndexSub = 166;
        UpdateSubWindowSprite(spriteXSub, spriteYSub, spriteIndexSub);
        if (framePeriodCounter++ < 1) {
            break;
        }
        framePeriodCounter = 0;
        fadeOutFrameCounter += 1;
        if (fadeOutFrameCounter > 8) {
            DestroySubwindow();
            StopPlayingSound();
            subWindowState = 1;
        }
        break;
    case 141:
        break;
    case 142:
        spriteX = -80;
        spriteY = screenHeight / 8;
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        CreateSubwindow();
        facingDirectionSub = -1;
        spriteXSub = -40;
        spriteYSub = screenHeight * 7 / 8;
        spriteIndexSub = 158;
        animationFrameCounter = 0;
        subWindowState = 143;
        screenMateOnTopOfSubwindow = 0;
        PlaceWindowTopmostPosition(selfInstanceWindowHandle);
        PlaceWindowTopmostPosition(knownInstanceWindows[8]);
        break;
    case 143:
        spriteXSub -= facingDirectionSub * 16;
        if (spriteIndexSub == 161) {
            spriteIndexSub = 158;
        } else {
            spriteIndexSub += 1;
        }
        if (screenHeight / 8 < spriteXSub) {
            spriteXSub = screenHeight / 8;
            spriteX = screenWidth;
            spriteY = spriteYSub;
            spriteIndex = 4;
            facingDirection = 1;
            subWindowState = 144;
        }
        UpdateSubWindowSprite(spriteXSub, spriteYSub, spriteIndexSub);
        break;
    case 144:
        if (framePeriodCounter++ < 1) {
            break;
        }
        framePeriodCounter = 0;
        if (spriteIndexSub == 161) {
            spriteIndexSub = 158;
        } else {
            spriteIndexSub += 1;
        }
        UpdateSubWindowSprite(spriteXSub, spriteYSub, spriteIndexSub);
        spriteX -= facingDirection * 16;
        spriteIndex = spriteIndex == 4 ? 5 : 4;
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        if (spriteXSub + 40 >= spriteX) {
            spriteX = -80;
            UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
            subWindowState = 145;
        }
        break;
    case 145:
        spriteYSub -= 40;
        if (spriteIndexSub == 161) {
            spriteIndexSub = 158;
        } else {
            spriteIndexSub += 1;
        }
        if (spriteYSub < -40) {
            DestroySubwindow();
            StopPlayingSound();
            subWindowState = 1;
            break;
        }
        UpdateSubWindowSprite(spriteXSub, spriteYSub, spriteIndexSub);
        break;
    case 146:
        break;
    case 147:
        CreateSubwindow();
        keepSubwindowOnPaint = 1;
        facingDirectionSub = 1;
        spriteIndexSub = 146;
        animationFrameCounter = 0;
        spriteX = screenWidth;
        spriteY = -40;
        facingDirection = 1;
        horizontalSpeed = screenWidth / -96;
        verticalSpeed = screenHeight / 96;
        spriteXSub = horizontalSpeed * 92 + screenWidth;
        spriteYSub = verticalSpeed * 92 - 20;
        subWindowState = 148;
        screenMateOnTopOfSubwindow = 1;
        PlaceWindowTopmostPosition(knownInstanceWindows[8]);
        PlaceWindowTopmostPosition(selfInstanceWindowHandle);
    case 148:
        if (framePeriodCounter++ < 0) {
            break;
        }
        framePeriodCounter = 0;
        UpdateSubWindowSprite(spriteXSub, spriteYSub, spriteIndexSub);
        spriteX += horizontalSpeed;
        spriteY += verticalSpeed;
        spriteIndex = burnAnimationFrames[animationFrameCounter / 3];
        animationFrameCounter += 1;
        if (spriteIndex == 0) {
            animationFrameCounter -= 1;
        }
        if (spriteIndex == 0 || spriteIndex == 144 || spriteIndex == 145) {
            spriteIndex = spriteIndex == 144 ? 145 : 144;
        }
        if (spriteIndex == 137 || spriteIndex == 138) {
            spriteIndex = spriteIndex == 137 ? 138 : 137;
        }
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        if (spriteXSub + 10 > spriteX || spriteYSub + 20 < spriteY) {
            animationFrameCounter = 0;
            subWindowState = 149;
            spriteIndex = 173;
            UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
            break;
        }
        break;
    case 149:
        if (framePeriodCounter++ < 1) {
            break;
        }
        framePeriodCounter = 0;
        spriteX = -80;
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        spriteIndexSub = burnBathtubSplashAnimationFrames[animationFrameCounter];
        animationFrameCounter += 1;
        if (spriteIndexSub == 0) {
            spriteX = spriteXSub;
            spriteY = spriteYSub;
            animationFrameCounter = 0;
            PlaySoundResourceIdAdditionalFlagsWhenOptionCryEnabled(108, 0U, 0);
            subWindowState = 150;
            break;
        }
        UpdateSubWindowSprite(spriteXSub, spriteYSub, spriteIndexSub);
        break;
    case 150:
        spriteIndex = 169;
        spriteIndex = burnGetOutOfBathtubAnimationFrames[animationFrameCounter];
        animationFrameCounter += 1;
        if (spriteIndex == 0) {
            spriteIndex = 3;
            subWindowState = 151;
            break;
        }
        if (spriteIndex >= 81 && spriteIndex <= 83) {
            UpdateMainWindowSprite(spriteX, spriteY - 20, spriteIndex);
        } else {
            UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        }
        break;
    case 151:
        if (framePeriodCounter++ < 1) {
            break;
        }
        framePeriodCounter = 0;
        spriteX -= facingDirection * 6;
        spriteIndex = spriteIndex == 2 ? 3 : 2;
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        if (spriteX < -40) {
            DestroySubwindow();
            subWindowState = 1;
            break;
        }
        break;
    case 152:
        break;
    case 155:
        /* Alien sheep: UFO abduction then aggressive return. */
        alienTransformPending = 1;
        alienModeActive = 0;
        alienModeTicks = 0;
        alienKnockCooldown = 0;
        ufoBeamHeight = 0;
        ufoBeamHeightSub = 0;
        fadeOutFrameCounter = 0;
        subWindowState = 128;
        break;
    case 156:
        /* Alien return: fall from sky; land pose cell 4/6 of 117.bmp on impact only. */
        alienTransformPending = 0;
        alienModeActive = 1;
        alienModeTicks = 400 + rand() % 301;
        alienKnockCooldown = 0;
        ufoBeamHeight = 0;
        ufoBeamHeightSub = 0;
        facingDirection = (rand() & 1) != 0 ? 1 : -1;
        spriteX = rand() % (screenWidth - 40);
        spriteY = -40;
        gravityEnabled = 1;
        bounceWhenFalling = 0;
        verticalSpeed = 0;
        horizontalSpeed = 0;
        /* Cells 4 and 6 of sheet 106/117 — used only when landing. */
        randomCaseNumberForAction = (rand() & 1) != 0 ? 84 : 86;
        spriteIndex = 4;
        fallActionCaseNumber = 4;
        PlaceWindowTopmostPosition(selfInstanceWindowHandle);
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        subWindowState = 99;
        break;
    case 162:
        /* Fade alien sheep via subwindow, then return to normal. */
        CreateSubwindow();
        spriteXSub = spriteX;
        spriteYSub = spriteY;
        facingDirectionSub = facingDirection;
        spriteIndexSub = AggressiveSpriteIndex(3);
        UpdateSubWindowSprite(spriteXSub, spriteYSub, spriteIndexSub);
        spriteX = -80;
        UpdateMainWindowSprite(spriteX, spriteY, AggressiveSpriteIndex(3));
        fadeOutFrameCounter = 0;
        framePeriodCounter = 0;
        subWindowState = 163;
        screenMateOnTopOfSubwindow = 0;
        PlaceWindowTopmostPosition(knownInstanceWindows[8]);
        PlaceWindowTopmostPosition(selfInstanceWindowHandle);
        break;
    case 163:
        if (framePeriodCounter++ < 1) {
            break;
        }
        framePeriodCounter = 0;
        fadeOutFrameCounter += 1;
        UpdateSubWindowSprite(spriteXSub, spriteYSub, spriteIndexSub);
        if (fadeOutFrameCounter > 8) {
            DestroySubwindow();
            spriteX = spriteXSub;
            spriteY = spriteYSub;
            facingDirection = facingDirectionSub;
            fadeOutFrameCounter = 0;
            alienModeActive = 0;
            alienModeTicks = 0;
            alienKnockCooldown = 0;
            spriteIndex = 3;
            UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
            subWindowState = 1;
        }
        break;
    case 164:
        /* Spawn the mushroom at the cursor, then run to it and graze. */
        {
            POINT cursorPt;
            GetCursorPos(&cursorPt);
            CreateSubwindow();
            cursorGrazeStage = 0;
            cursorGrazeSpinFrame = 0;
            spriteIndexSub = 352;
            /* Prop cells have ~7px empty at the bottom; nudge down so it sits on the ground. */
            spriteYSub = spriteY + 6;
            spriteXSub = cursorPt.x - 20;
            if (spriteXSub < 0) {
                spriteXSub = 0;
            }
            if (spriteXSub > screenWidth - 40) {
                spriteXSub = screenWidth - 40;
            }
            HideSystemCursorForGraze();
            if (spriteXSub + 20 < spriteX + 20) {
                facingDirection = 1;
            } else {
                facingDirection = -1;
            }
            facingDirectionSub = facingDirection;
            UpdateSubWindowSprite(spriteXSub, spriteYSub, spriteIndexSub);
            screenMateOnTopOfSubwindow = 1;
            PlaceWindowTopAnother(knownInstanceWindows[8], selfInstanceWindowHandle);
        }
        animationFrameCounter = 36;
        spriteIndex = 4;
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        subWindowState = 165;
        framePeriodCounter = 0;
        break;
    case 165:
        /* Keep the mushroom spinning while the sheep runs toward it. */
        if (knownInstanceWindows[8] != NULL) {
            cursorGrazeSpinFrame = (cursorGrazeSpinFrame + 1) % 8;
            spriteIndexSub = 352 + cursorGrazeSpinFrame;
            UpdateSubWindowSprite(spriteXSub, spriteYSub, spriteIndexSub);
        }
        if (framePeriodCounter++ < 1) {
            break;
        }
        framePeriodCounter = 0;
        {
            int dist;
            if (spriteXSub + 20 < spriteX + 20) {
                facingDirection = 1;
            } else {
                facingDirection = -1;
            }
            spriteX -= facingDirection * 16;
            if (spriteX < -40) {
                spriteX = -40;
            }
            if (spriteX > screenWidth) {
                spriteX = screenWidth;
            }
            spriteIndex = spriteIndex == 4 ? 5 : 4;
            UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
            /* Stop when the sheep is roughly mouth-adjacent to the mushroom. */
            dist = (spriteXSub + 20) - (spriteX + 20);
            if (dist < 0) {
                dist = -dist;
            }
            animationFrameCounter -= 1;
            if (dist <= 48 || animationFrameCounter <= 0) {
                subWindowState = 166;
                break;
            }
        }
        HandleOutOfViewOrTopPosition(1);
        break;
    case 166:
        /* Keep the mushroom where it spawned; park the sheep beside it to graze. */
        animationFrameCounter = 0;
        framePeriodCounter = 0;
        if (spriteXSub + 20 < spriteX + 20) {
            facingDirection = 1;
            spriteX = spriteXSub + 40;
        } else {
            facingDirection = -1;
            spriteX = spriteXSub - 40;
        }
        facingDirectionSub = facingDirection;
        cursorGrazeStage = 0;
        cursorGrazeSpinFrame = 0;
        spriteIndexSub = 352;
        UpdateSubWindowSprite(spriteXSub, spriteYSub, spriteIndexSub);
        UpdateMainWindowSprite(spriteX, spriteY, 3);
        screenMateOnTopOfSubwindow = 1;
        PlaceWindowTopAnother(knownInstanceWindows[8], selfInstanceWindowHandle);
        subWindowState = 167;
        break;
    case 167:
        if (knownInstanceWindows[8] != NULL) {
            cursorGrazeSpinFrame = (cursorGrazeSpinFrame + 1) % 8;
            spriteIndexSub = 352 + cursorGrazeStage * 8 + cursorGrazeSpinFrame;
            UpdateSubWindowSprite(spriteXSub, spriteYSub, spriteIndexSub);
        }
        if (framePeriodCounter++ < 2) {
            break;
        }
        framePeriodCounter = 0;
        spriteIndex = cursorGrazeAnimationFrames[animationFrameCounter];
        animationFrameCounter += 1;
        if (spriteIndex == 2) {
            spriteX -= facingDirection * 8;
            UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
            break;
        }
        if (spriteIndex >= CURSOR_GRAZE_BITE_1 && spriteIndex <= CURSOR_GRAZE_BITE_GONE) {
            if (spriteIndex == CURSOR_GRAZE_BITE_GONE) {
                DestroySubwindow();
            } else if (spriteIndex - CURSOR_GRAZE_BITE_1 + 1 > cursorGrazeStage) {
                cursorGrazeStage = spriteIndex - CURSOR_GRAZE_BITE_1 + 1;
            }
            spriteIndex = cursorGrazeAnimationFrames[animationFrameCounter];
            animationFrameCounter += 1;
        }
        if (spriteIndex == 0) {
            DestroySubwindow();
            animationFrameCounter = 0;
            framePeriodCounter = 0;
            acidTicks = 0;
            PlaySoundResourceIdAdditionalFlags(111, SND_LOOP, 0);
            subWindowState = 168;
            break;
        }
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        break;
    case 168:
        /* After the mushroom: scared twice, normal colours. */
        if (framePeriodCounter++ < 1) {
            break;
        }
        framePeriodCounter = 0;
        spriteIndex = amazedAnimationFrames[animationFrameCounter];
        animationFrameCounter += 1;
        if (spriteIndex == 0) {
            animationFrameCounter = 0;
            if (++acidTicks < 2) {
                break;
            }
            subWindowState = 169;
            break;
        }
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        break;
    case 169:
        alienModeActive = 0;
        acidModeActive = 1;
        acidTicks = 0;
        acidBaseY = spriteY;
        facingDirection = 1;
        ApplyAcidColour(0);
        subWindowState = 170;
        break;
    case 170:
        /* Spin in place, twice the normal speed. */
        spriteIndex = spinAnimationFrames[acidTicks % 8];
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        AdvanceAcidTick();
        if (acidTicks >= 16) {
            acidTicks = 0;
            subWindowState = 171;
        }
        break;
    case 171:
        /* Flip left/right while bouncing. */
        {
            static const int bounce[4] = {0, -6, -10, -6};
            if ((acidTicks & 1) == 0) {
                facingDirection = -facingDirection;
            }
            spriteY = acidBaseY + bounce[acidTicks % 4];
            spriteIndex = 3;
            UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        }
        AdvanceAcidTick();
        if (acidTicks >= 16) {
            spriteY = acidBaseY;
            acidTicks = 0;
            subWindowState = 172;
        }
        break;
    case 172:
        /* Roll. */
        AcidRollStep(rollAnimationFrames[acidTicks % 8]);
        AdvanceAcidTick();
        if (acidTicks >= 12) {
            acidTicks = 0;
            facingDirection = -facingDirection;
            subWindowState = 173;
        }
        break;
    case 173:
        /* Roll back upside down (125.bmp = 108.bmp flipped vertically). */
        AcidRollStep(384 + rollAnimationFrames[acidTicks % 8] - 112);
        AdvanceAcidTick();
        if (acidTicks >= 12) {
            acidTicks = 0;
            animationFrameCounter = 0;
            subWindowState = 174;
        }
        break;
    case 174:
        /* Dizzy blinks, then back to normal. */
        AdvanceAcidTick();
        if ((acidTicks & 1) != 0) {
            break;
        }
        spriteIndex = blinkAnimationFrames[0][animationFrameCounter];
        UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
        if (++animationFrameCounter >= 8) {
            RestoreAcidColours();
            spriteIndex = 3;
            UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
            subWindowState = 1;
        }
        break;
    default:
        break;
    }
}

/* One acid tick: the horn and eye colour moves on every second tick. */
void AdvanceAcidTick(void)
{
    acidTicks += 1;
    if ((acidTicks & 1) == 0) {
        ApplyAcidColour(acidColourIndex + 1);
    }
}

/* Move one roll step, bouncing off the screen edges. */
void AcidRollStep(int sprite)
{
    spriteX -= facingDirection * 8;
    if (spriteX < 0) {
        spriteX = 0;
        facingDirection = -1;
    } else if (spriteX > screenWidth - 40) {
        spriteX = screenWidth - 40;
        facingDirection = 1;
    }
    spriteIndex = sprite;
    UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
}

/* Environment-affected action change, controlled by a flag. */
void ApplyEnvironmentActionChange(int arg_0)
{
    switch (arg_0) {
    case 0:
        subWindowState = 1;
        if (gravityEnabled != 0) {
            subWindowState = 97;
        }
        break;
    case 1:
        subWindowState = 81;
        break;
    case 2:
        subWindowState = 97;
        break;
    case 3:
        sleepTimeoutAction = 113;
        break;
    case 4:
        subWindowState = 56;
        break;
    default:
        break;
    }
}

/* Process debug window action change. */
void ProcessDebugWindowActionChange(WPARAM arg_0)
{
    ufoBeamHeight = 0;
    ufoBeamHeightSub = 0;
    alienTransformPending = 0;
    alienModeActive = 0;
    alienModeTicks = 0;
    alienKnockCooldown = 0;
    fadeOutFrameCounter = 0;
    StopPlayingSound();
    DestroySubwindow();
    RestoreAcidColours();
    switch (arg_0) {
    case 0:
        subWindowState = 1;
        break;
    case 1:
        subWindowState = 7;
        break;
    case 2:
        subWindowState = 11;
        break;
    case 3:
        subWindowState = 13;
        break;
    case 4:
        subWindowState = 15;
        break;
    case 5:
        subWindowState = 17;
        break;
    case 6:
        subWindowState = 20;
        break;
    case 7:
        subWindowState = 24;
        break;
    case 8:
        subWindowState = 30;
        break;
    case 9:
        subWindowState = 35;
        break;
    case 10:
        subWindowState = 43;
        break;
    case 11:
        subWindowState = 45;
        break;
    case 12:
        subWindowState = 49;
        break;
    case 13:
        subWindowState = 51;
        break;
    case 14:
        subWindowState = 53;
        break;
    case 15:
        subWindowState = 58;
        break;
    case 16:
        subWindowState = 47;
        break;
    case 17:
        subWindowState = 147;
        break;
    case 18:
        subWindowState = 116;
        break;
    case 19:
        subWindowState = 121;
        break;
    case 20:
        subWindowState = 126;
        break;
    case 21:
        subWindowState = 128;
        break;
    case 22:
        subWindowState = 135;
        break;
    case 23:
        subWindowState = 142;
        break;
    case 24:
        subWindowState = 65;
        break;
    case 25:
        subWindowState = 62;
        break;
    case 26:
        subWindowState = 75;
        break;
    case 27:
        subWindowState = 96;
        break;
    case 28:
        subWindowState = 9;
        break;
    case 29:
        subWindowState = 69;
        break;
    case 30:
        subWindowState = 155;
        break;
    case 31:
        subWindowState = 164;
        break;
    default:
        break;
    }
}

/* Move window by offset. */
void MoveWindowOffset(int arg_0, int arg_2)
{
    spriteX += arg_0;
    spriteY += arg_2;
    UpdateMainWindowSprite(spriteX, spriteY, spriteIndex);
}

/* Initialize bitmaps (sub). */
BOOL InitializeBitmapsSub(HWND arg_0)
{
    HDC var_2;
    var_2 = GetDC(arg_0);
    doubleBufferSub[0] = CreateCompatibleBitmap(var_2, 100, 100);
    if (doubleBufferSub[0] == NULL) {
        goto bitmapSubInitFailedCleanup;
    }
    doubleBufferSub[1] = CreateCompatibleBitmap(var_2, 100, 100);
    if (doubleBufferSub[1] == NULL) {
        goto bitmapSubInitFailedCleanup;
    }
    spriteRenderTargetSub = CreateCompatibleBitmap(var_2, 100, 100);
    if (spriteRenderTargetSub == NULL) {
        goto bitmapSubInitFailedCleanup;
    }
    fadeOutColourBitmapSub = CreateCompatibleBitmap(var_2, 40, 40);
    if (fadeOutColourBitmapSub == NULL) {
        goto bitmapSubInitFailedCleanup;
    }
    fadeOutMaskBitmapSub = CreateCompatibleBitmap(var_2, 40, 40);
    if (fadeOutMaskBitmapSub == NULL) {
        goto bitmapSubInitFailedCleanup;
    }
    unusedCa4C = 0;
    unusedCa4E = 0;
    screenWidth = GetSystemMetrics(SM_CXSCREEN);
    screenHeight = GetSystemMetrics(SM_CYSCREEN);
    ReleaseDC(arg_0, var_2);
    fadeOutFrameCounter = 0;
    ufoBeamHeightSub = 0;
    keepSubwindowOnPaint = 0;
    screenXSubPreviousFrame = 0;
    screenYSubPreviousFrame = 0;
    spriteWidthSubPreviousFrame = 0;
    spriteHeightSubPreviousFrame = 0;
    return TRUE;
bitmapSubInitFailedCleanup:
    ReleaseDC(arg_0, var_2);
    return FALSE;
}

/* Release bitmaps (sub). */
void ReleaseBitmaps2()
{
    DeleteObject(fadeOutMaskBitmapSub);
    DeleteObject(fadeOutColourBitmapSub);
    DeleteObject(doubleBufferSub[0]);
    DeleteObject(doubleBufferSub[1]);
    DeleteObject(spriteRenderTargetSub);
#ifdef _WIN32
    ReleaseLayeredSurface(&layeredSurfaceSub);
#endif
}

/* Update window position and sprite to be actually used (sub). */
void UpdateWindowPositionSpriteBeActuallyUsed2(int arg_0, int arg_2, int arg_4)
{
    screenXSubCurrentFrame = arg_0;
    screenYSubCurrentFrame = arg_2;
    spriteColourBitmapSubCurrentFrame = spriteListSub[arg_4].bitmaps[0];
    spriteMaskBitmapSubCurrentFrame = spriteListSub[arg_4].bitmaps[1];
    spriteXInResourceImageSubCurrentFrame = spriteListSub[arg_4].x;
    spriteYInResourceImageSubCurrentFrame = spriteListSub[arg_4].y;
    spriteWidthSubCurrentFrame = spriteListSub[arg_4].width;
    spriteHeightSubCurrentFrame = spriteListSub[arg_4].height;
}

/* Clear window (sub). */
void ClearWindow2(HWND arg_0)
{
    if (keepSubwindowOnPaint != 0) {
        return;
    }
    screenXSubPreviousFrame = 0;
    screenYSubPreviousFrame = 0;
    spriteWidthSubPreviousFrame = 0;
    spriteHeightSubPreviousFrame = 0;
    MoveWindow(arg_0, 0, 0, 0, 0, TRUE);
    unused_A872 = 1;
    noUpdatePeriodsAfterClearing = 1;
    spriteColourBitmapSubPreviousFrame = NULL;
}

/* Render sprite with double buffering (with fade out effect) (sub). */
void RenderSpriteDoubleBufferingFadeOutEffect(HWND arg_0)
{
    HDC var_4;
    HDC var_6;
#ifndef _WIN32
    HDC var_2;
    int var_C;
    int var_E;
    int var_10;
    int var_12;
    int var_14;
    int var_16;
    int var_18;
    int var_1A;
    int var_1C;
    int var_1E;
#endif
    if (renderOrUpdateWindowFlagSub != 0) {
        return;
    }
    if (screenXSubPreviousFrame == screenXSubCurrentFrame && screenYSubPreviousFrame == screenYSubCurrentFrame && spriteColourBitmapSubPreviousFrame == spriteColourBitmapSubCurrentFrame && spriteXInResourceImageSubPreviousFrame == spriteXInResourceImageSubCurrentFrame && fadeOutFrameCounter == 0 && ufoBeamHeightSub == 0) {
        return;
    }
#ifdef _WIN32
    updateAreaRectXSubCurrentFrame = screenXSubCurrentFrame;
    updateAreaRectYSubCurrentFrame = screenYSubCurrentFrame;
    updateAreaRectWidthSubCurrentFrame = spriteWidthSubCurrentFrame;
    updateAreaRectHeightSubCurrentFrame = spriteHeightSubCurrentFrame;
    if (spriteColourBitmapSubCurrentFrame != NULL) {
        if (spriteMaskBitmapSubCurrentFrame != NULL && fadeOutFrameCounter != 0) {
            var_4 = CreateCompatibleDC(NULL);
            var_6 = CreateCompatibleDC(NULL);
            if (fadeOutFrameCounter == 1) {
                SelectObject(var_4, spriteMaskBitmapSubCurrentFrame);
                SelectObject(var_6, fadeOutMaskBitmapSub);
                BitBlt(var_6, 0, 0, 40, 40, var_4, spriteXInResourceImageSubCurrentFrame, spriteYInResourceImageSubCurrentFrame, SRCCOPY);
                SelectObject(var_4, spriteColourBitmapSubCurrentFrame);
                SelectObject(var_6, fadeOutColourBitmapSub);
                BitBlt(var_6, 0, 0, 40, 40, var_4, spriteXInResourceImageSubCurrentFrame, spriteYInResourceImageSubCurrentFrame, SRCCOPY);
            }
            SelectObject(var_4, fadeOutMaskBitmapSub);
            SelectObject(var_6, spriteListSub[172].bitmaps[0]);
            BitBlt(var_4, fadeOutFrameCounter - 1, fadeOutFrameCounter - 1, 41 - fadeOutFrameCounter, 40, var_6, spriteListSub[172].x, 0, SRCPAINT);
            SelectObject(var_4, fadeOutColourBitmapSub);
            SelectObject(var_6, spriteListSub[172].bitmaps[1]);
            BitBlt(var_4, fadeOutFrameCounter - 1, fadeOutFrameCounter - 1, 41 - fadeOutFrameCounter, 40, var_6, spriteListSub[172].x, 0, SRCAND);
            DeleteDC(var_4);
            DeleteDC(var_6);
            PresentLayeredSprite(arg_0, &layeredSurfaceSub, screenXSubCurrentFrame, screenYSubCurrentFrame, fadeOutColourBitmapSub, fadeOutMaskBitmapSub, 0, 0, spriteWidthSubCurrentFrame, spriteHeightSubCurrentFrame, ufoBeamHeightSub, alienTransformPending != 0);
        } else {
            PresentLayeredSprite(arg_0, &layeredSurfaceSub, screenXSubCurrentFrame, screenYSubCurrentFrame, spriteColourBitmapSubCurrentFrame, spriteMaskBitmapSubCurrentFrame, spriteXInResourceImageSubCurrentFrame, spriteYInResourceImageSubCurrentFrame, spriteWidthSubCurrentFrame, spriteHeightSubCurrentFrame, ufoBeamHeightSub, alienTransformPending != 0);
        }
    }
#else
    currentSpriteFramebufferIndexSub ^= 1;
    var_2 = GetDC(NULL);
    SelectPalette(var_2, windowPaletteInUse, FALSE);
    var_4 = CreateCompatibleDC(var_2);
    var_6 = CreateCompatibleDC(var_2);
    SelectPalette(var_6, windowPaletteInUse, FALSE);
    SelectPalette(var_4, windowPaletteInUse, FALSE);
    var_16 = max(screenXSubCurrentFrame, screenXSubPreviousFrame);
    var_14 = max(screenYSubCurrentFrame, screenYSubPreviousFrame);
    var_12 = min(spriteWidthSubCurrentFrame + screenXSubCurrentFrame, spriteWidthSubPreviousFrame + screenXSubPreviousFrame) - var_16;
    var_10 = min(screenYSubCurrentFrame + spriteHeightSubCurrentFrame, screenYSubPreviousFrame + spriteHeightSubPreviousFrame) - var_14;
    if (var_12 <= 0 || var_10 <= 0) {
        unused_A898 = 1;
        if (unused_A872 != 0) {
            unused_A872 = 0;
        }
        updateAreaRectXSubCurrentFrame = screenXSubCurrentFrame;
        updateAreaRectYSubCurrentFrame = screenYSubCurrentFrame;
        updateAreaRectWidthSubCurrentFrame = spriteWidthSubCurrentFrame;
        updateAreaRectHeightSubCurrentFrame = spriteHeightSubCurrentFrame;
        SelectObject(var_4, doubleBufferSub[currentSpriteFramebufferIndexSub]);
        BitBlt(var_4, 0, 0, updateAreaRectWidthSubCurrentFrame, updateAreaRectHeightSubCurrentFrame, var_2, updateAreaRectXSubCurrentFrame, updateAreaRectYSubCurrentFrame, SRCCOPY);
    } else {
        unused_A898 = 0;
        updateAreaRectXSubCurrentFrame = min(screenXSubCurrentFrame, screenXSubPreviousFrame);
        updateAreaRectYSubCurrentFrame = min(screenYSubCurrentFrame, screenYSubPreviousFrame);
        updateAreaRectWidthSubCurrentFrame = max(spriteWidthSubCurrentFrame + screenXSubCurrentFrame, spriteWidthSubPreviousFrame + screenXSubPreviousFrame) - updateAreaRectXSubCurrentFrame;
        updateAreaRectHeightSubCurrentFrame = max(screenYSubCurrentFrame + spriteHeightSubCurrentFrame, screenYSubPreviousFrame + spriteHeightSubPreviousFrame) - updateAreaRectYSubCurrentFrame;
        SelectObject(var_4, doubleBufferSub[currentSpriteFramebufferIndexSub]);
        BitBlt(var_4, 0, 0, updateAreaRectWidthSubCurrentFrame, updateAreaRectHeightSubCurrentFrame, var_2, updateAreaRectXSubCurrentFrame, updateAreaRectYSubCurrentFrame, SRCCOPY);
        var_1E = max(updateAreaRectXSubCurrentFrame, updateAreaRectXSubPreviousFrame);
        var_1C = max(updateAreaRectYSubCurrentFrame, updateAreaRectYSubPreviousFrame);
        var_1A = min(updateAreaRectWidthSubCurrentFrame + updateAreaRectXSubCurrentFrame, updateAreaRectWidthSubPreviousFrame + updateAreaRectXSubPreviousFrame) - var_1E;
        var_18 = min(updateAreaRectYSubCurrentFrame + updateAreaRectHeightSubCurrentFrame, updateAreaRectYSubPreviousFrame + updateAreaRectHeightSubPreviousFrame) - var_1C;
        var_16 = max(0, var_1E - updateAreaRectXSubCurrentFrame);
        var_14 = max(0, var_1C - updateAreaRectYSubCurrentFrame);
        var_E = max(0, var_1E - updateAreaRectXSubPreviousFrame);
        var_C = max(0, var_1C - updateAreaRectYSubPreviousFrame);
        if (var_1A > 0 && var_18 > 0) {
            SelectObject(var_6, doubleBufferSub[LOBYTE(currentSpriteFramebufferIndexSub) - 0xFF & 1]);
            BitBlt(var_4, var_16, var_14, var_1A, var_18, var_6, var_E, var_C, SRCCOPY);
        }
    }
    if (spriteColourBitmapSubCurrentFrame != NULL) {
        SelectObject(var_6, spriteRenderTargetSub);
        BitBlt(var_6, 0, 0, updateAreaRectWidthSubCurrentFrame, updateAreaRectHeightSubCurrentFrame, var_4, 0, 0, SRCCOPY);
        var_16 = max(0, screenXSubCurrentFrame - updateAreaRectXSubCurrentFrame);
        var_14 = max(0, screenYSubCurrentFrame - updateAreaRectYSubCurrentFrame);
        if (spriteMaskBitmapSubCurrentFrame != NULL) {
            if (fadeOutFrameCounter != 0) {
                if (fadeOutFrameCounter == 1) {
                    SelectObject(var_4, spriteMaskBitmapSubCurrentFrame);
                    SelectObject(var_6, fadeOutMaskBitmapSub);
                    BitBlt(var_6, 0, 0, 40, 40, var_4, spriteXInResourceImageSubCurrentFrame, spriteYInResourceImageSubCurrentFrame, SRCCOPY);
                    SelectObject(var_4, spriteColourBitmapSubCurrentFrame);
                    SelectObject(var_6, fadeOutColourBitmapSub);
                    BitBlt(var_6, 0, 0, 40, 40, var_4, spriteXInResourceImageSubCurrentFrame, spriteYInResourceImageSubCurrentFrame, SRCCOPY);
                }
                SelectObject(var_4, fadeOutMaskBitmapSub);
                SelectObject(var_6, spriteListSub[172].bitmaps[0]);
                BitBlt(var_4, fadeOutFrameCounter - 1, fadeOutFrameCounter - 1, 41 - fadeOutFrameCounter, 40, var_6, spriteListSub[172].x, 0, SRCPAINT);
                SelectObject(var_4, fadeOutColourBitmapSub);
                SelectObject(var_6, spriteListSub[172].bitmaps[1]);
                BitBlt(var_4, fadeOutFrameCounter - 1, fadeOutFrameCounter - 1, 41 - fadeOutFrameCounter, 40, var_6, spriteListSub[172].x, 0, SRCAND);
                SelectObject(var_6, spriteRenderTargetSub);
                SelectObject(var_4, fadeOutMaskBitmapSub);
                BitBlt(var_6, var_16, var_14, spriteWidthSubCurrentFrame, spriteHeightSubCurrentFrame, var_4, 0, 0, SRCAND);
                SelectObject(var_4, fadeOutColourBitmapSub);
                BitBlt(var_6, var_16, var_14, spriteWidthSubCurrentFrame, spriteHeightSubCurrentFrame, var_4, 0, 0, SRCPAINT);
            } else {
                SelectObject(var_4, spriteMaskBitmapSubCurrentFrame);
                BitBlt(var_6, var_16, var_14, spriteWidthSubCurrentFrame, spriteHeightSubCurrentFrame, var_4, spriteXInResourceImageSubCurrentFrame, spriteYInResourceImageSubCurrentFrame, SRCAND);
                SelectObject(var_4, spriteColourBitmapSubCurrentFrame);
                BitBlt(var_6, var_16, var_14, spriteWidthSubCurrentFrame, spriteHeightSubCurrentFrame, var_4, spriteXInResourceImageSubCurrentFrame, spriteYInResourceImageSubCurrentFrame, SRCPAINT);
            }
        } else {
            SelectObject(var_4, spriteColourBitmapSubCurrentFrame);
            BitBlt(var_6, var_16, var_14, spriteWidthSubCurrentFrame, spriteHeightSubCurrentFrame, var_4, spriteXInResourceImageSubCurrentFrame, spriteYInResourceImageSubCurrentFrame, SRCCOPY);
        }
        renderOrUpdateWindowFlagSub = 1;
        unusedCa5E = 1;
        MoveWindow(arg_0, updateAreaRectXSubCurrentFrame, updateAreaRectYSubCurrentFrame, updateAreaRectWidthSubCurrentFrame, updateAreaRectHeightSubCurrentFrame + ufoBeamHeightSub, TRUE);
        unusedCa5E = 0;
    }
    DeleteDC(var_4);
    DeleteDC(var_6);
#endif
    updateAreaRectXSubPreviousFrame = updateAreaRectXSubCurrentFrame;
    updateAreaRectYSubPreviousFrame = updateAreaRectYSubCurrentFrame;
    updateAreaRectWidthSubPreviousFrame = updateAreaRectWidthSubCurrentFrame;
    updateAreaRectHeightSubPreviousFrame = updateAreaRectHeightSubCurrentFrame;
    screenXSubPreviousFrame = screenXSubCurrentFrame;
    screenYSubPreviousFrame = screenYSubCurrentFrame;
    spriteWidthSubPreviousFrame = spriteWidthSubCurrentFrame;
    spriteHeightSubPreviousFrame = spriteHeightSubCurrentFrame;
    spriteColourBitmapSubPreviousFrame = spriteColourBitmapSubCurrentFrame;
    spriteXInResourceImageSubPreviousFrame = spriteXInResourceImageSubCurrentFrame;
    spriteYInResourceImageSubPreviousFrameUnused = spriteYInResourceImageSubCurrentFrame;
#ifndef _WIN32
    ReleaseDC(NULL, var_2);
#endif
}

/* Render UFO beam (if any) and present render targets onto window (sub). */
BOOL RenderUfoBeamAndPresentSubRenderTargets(HWND arg_0)
{
    HDC var_2;
    HDC var_4;
    RECT var_C;
    HDC var_E;
#ifdef _WIN32
    HDC screen;
#endif
    if (renderOrUpdateWindowFlagSub == 0) {
        return TRUE;
    }
    renderOrUpdateWindowFlagSub = 0;
    var_2 = GetDC(arg_0);
    SelectPalette(var_2, windowPaletteInUse, FALSE);
    RealizePalette(var_2);
    var_4 = CreateCompatibleDC(var_2);
    SelectPalette(var_4, windowPaletteInUse, FALSE);
    SelectObject(var_4, spriteRenderTargetSub);
    BitBlt(var_2, 0, 0, updateAreaRectWidthSubCurrentFrame, updateAreaRectHeightSubCurrentFrame, var_4, 0, 0, SRCCOPY);
    if (ufoBeamHeightSub != 0) {
        if (ufoBeamColorBitmap == NULL) {
            ufoBeamColorBitmap = CreateCompatibleBitmap(var_2, 40, screenHeight * 4 / 5);
            if (ufoBeamColorBitmap == NULL) {
                goto ufoBeamSubRenderFailedCleanupDestroyWindow;
            }
        }
        if (ufoBeamRenderTarget == NULL) {
            ufoBeamRenderTarget = CreateCompatibleBitmap(var_2, 40, screenHeight * 4 / 5);
            if (ufoBeamRenderTarget == NULL) {
                goto ufoBeamSubRenderFailedCleanupDestroyWindow;
            }
        }
        if (ufoBeamMaskBrush == NULL) {
            if (alienTransformPending != 0) {
                ufoBeamMaskBrush = CreateSolidBrush(RGB(255, 64, 64));
            } else {
                ufoBeamMaskBrush = CreateSolidBrush(RGB(255, 255, 0));
            }
        }
        if (ufoBeamPaintBrush == NULL) {
            if (alienTransformPending != 0) {
                ufoBeamPaintBrush = CreateSolidBrush(RGB(160, 0, 0));
            } else {
                ufoBeamPaintBrush = CreateSolidBrush(RGB(128, 128, 0));
            }
        }
        var_E = CreateCompatibleDC(var_2);
        SelectObject(var_E, ufoBeamRenderTarget);
#ifdef _WIN32
        /* Screen contents with height of only 40 pixels can be captured from window device context on Windows 10. Capture directly from screen instead. */
        screen = GetDC(NULL);
        BitBlt(var_E, 0, 0, 40, ufoBeamHeightSub, screen, updateAreaRectXSubCurrentFrame, updateAreaRectYSubCurrentFrame + 40, SRCCOPY);
        ReleaseDC(NULL, screen);
#else
        BitBlt(var_E, 0, 0, 40, ufoBeamHeightSub, var_2, 0, 40, SRCCOPY);
#endif
        var_C.left = 0;
        var_C.top = 0;
        var_C.right = 40;
        var_C.bottom = ufoBeamHeightSub;
        SelectObject(var_4, ufoBeamColorBitmap);
        FillRect(var_4, &var_C, ufoBeamMaskBrush);
        BitBlt(var_E, 0, 0, 40, ufoBeamHeightSub, var_4, 0, 0, SRCAND);
        FillRect(var_4, &var_C, ufoBeamPaintBrush);
        BitBlt(var_E, 0, 0, 40, ufoBeamHeightSub, var_4, 0, 0, SRCPAINT);
        BitBlt(var_2, 0, 40, 40, ufoBeamHeightSub, var_E, 0, 0, SRCCOPY);
        DeleteDC(var_E);
        DeleteDC(var_4);
    } else {
        if (ufoBeamPaintBrush != NULL) {
            DeleteObject(ufoBeamPaintBrush);
            ufoBeamPaintBrush = NULL;
        }
        if (ufoBeamMaskBrush != NULL) {
            DeleteObject(ufoBeamMaskBrush);
            ufoBeamMaskBrush = NULL;
        }
        if (ufoBeamRenderTarget != NULL) {
            DeleteObject(ufoBeamRenderTarget);
            ufoBeamRenderTarget = NULL;
        }
        if (ufoBeamColorBitmap != NULL) {
            DeleteObject(ufoBeamColorBitmap);
            ufoBeamColorBitmap = NULL;
        }
        DeleteDC(var_4);
    }
    ReleaseDC(arg_0, var_2);
    return TRUE;
ufoBeamSubRenderFailedCleanupDestroyWindow:
    ReleaseDC(arg_0, var_2);
    DestroyWindow(arg_0);
    return FALSE;
}
