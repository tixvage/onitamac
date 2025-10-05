// copy paste from: https://github.com/raysan5/raylib/blob/master/src/external/fix_win32_compatibility.h
#ifndef FIX_WIN
#define FIX_WIN

// move the windows functions to new names
// note that you can't call these functions or structures from your code, but you should not neeed to
#define CloseWindow CloseWindowWin32
#define Rectangle RectangleWin32
#define ShowCursor ShowCursorWin32
#define LoadImageA LoadImageAWin32
#define LoadImageW LoadImageWin32
#define DrawTextA DrawTextAWin32
#define DrawTextW DrawTextWin32
#define DrawTextExA DrawTextExAWin32
#define DrawTextExW DrawTextExWin32
#define PlaySoundA PlaySoundAWin32
// include windows
#define WIN32_LEAN_AND_MEAN 
#include <windows.h>

// remove all our redfintions so that raylib can define them properly
#undef CloseWindow
#undef Rectangle
#undef ShowCursor
#undef LoadImage 
#undef LoadImageA
#undef LoadImageW
#undef DrawText 
#undef DrawTextA
#undef DrawTextW
#undef DrawTextEx 
#undef DrawTextExA
#undef DrawTextExW
#undef PlaySoundA

#endif