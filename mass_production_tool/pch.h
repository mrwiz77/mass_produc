// pch.h: 미리 컴파일된 헤더 파일입니다.
// 아래 나열된 파일은 한 번만 컴파일되었으며, 향후 빌드에 대한 빌드 성능을 향상합니다.
// 코드 컴파일 및 여러 코드 검색 기능을 포함하여 IntelliSense 성능에도 영향을 미칩니다.
// 그러나 여기에 나열된 파일은 빌드 간 업데이트되는 경우 모두 다시 컴파일됩니다.
// 여기에 자주 업데이트할 파일을 추가하지 마세요. 그러면 성능이 저하됩니다.

#ifndef PCH_H
#define PCH_H

// 여기에 미리 컴파일하려는 헤더 추가
#include "framework.h"
#include <algorithm>
#include <utility>
#include <atlbase.h>
#include <comdef.h>
#include <ShlObj.h>
#include <Shldisp.h>

#define MP_GRID_CLIENT_X 0
#define MP_GRID_CLIENT_Y 48

enum MP_GRID_CELL_COLOR_THEME
{
	MP_GRID_COLOR_DEFAULT_GRAY = 0,
	MP_GRID_COLOR_DARK_GRAY,
	MP_GRID_COLOR_YELLOW,
	MP_GRID_COLOR_DARK_YELLOW,
	MP_GRID_COLOR_BLUE,
	MP_GRID_COLOR_DARK_BLUE,
	MP_GRID_COLOR_GREEN,
	MP_GRID_COLOR_DARK_GREEN,
	MP_GRID_COLOR_ORANGE,
	MP_GRID_COLOR_DARK_ORANGE,
	MP_GRID_COLOR_BK_WIHTE,
	MP_GRID_COLOR_BLACK
};

struct MP_GRID_CELL_COLOR
{
	int row;
	int col;
	COLORREF bkColor;
	COLORREF textColor;
};

inline COLORREF MpGridThemeBkColor(MP_GRID_CELL_COLOR_THEME theme)
{
	switch (theme)
	{
	case MP_GRID_COLOR_DARK_GRAY: return RGB(120, 120, 120);
	case MP_GRID_COLOR_YELLOW: return RGB(255, 217, 102);
	case MP_GRID_COLOR_DARK_YELLOW: return RGB(191, 143, 0);
	case MP_GRID_COLOR_BLUE: return RGB(31, 78, 120);
	case MP_GRID_COLOR_DARK_BLUE: return RGB(0, 32, 96);
	case MP_GRID_COLOR_GREEN: return RGB(112, 173, 71);
	case MP_GRID_COLOR_DARK_GREEN: return RGB(0, 97, 0);
	case MP_GRID_COLOR_ORANGE: return RGB(237, 125, 49);
	case MP_GRID_COLOR_DARK_ORANGE: return RGB(132, 60, 12);
	case MP_GRID_COLOR_BK_WIHTE: return RGB(225, 225, 225);
	case MP_GRID_COLOR_BLACK: return RGB(0, 0, 0);
	case MP_GRID_COLOR_DEFAULT_GRAY:
	default:
		return RGB(225, 225, 225);
	}
}

inline COLORREF MpGridThemeTextColor(MP_GRID_CELL_COLOR_THEME theme)
{
	switch (theme)
	{
	case MP_GRID_COLOR_DARK_YELLOW:
	case MP_GRID_COLOR_BLUE:
	case MP_GRID_COLOR_DARK_BLUE:
	case MP_GRID_COLOR_DARK_GREEN:
	case MP_GRID_COLOR_ORANGE:
	case MP_GRID_COLOR_DARK_ORANGE:
	case MP_GRID_COLOR_DARK_GRAY:
	case MP_GRID_COLOR_BLACK:
		return RGB(255, 255, 255);
	case MP_GRID_COLOR_DEFAULT_GRAY:
	case MP_GRID_COLOR_YELLOW:
	case MP_GRID_COLOR_GREEN:
	default:
		return RGB(0, 0, 0);
	}
}

inline LPCTSTR MpGridThemeText(MP_GRID_CELL_COLOR_THEME theme)
{
	switch (theme)
	{
	case MP_GRID_COLOR_DEFAULT_GRAY: return _T("MP_GRID_COLOR_DEFAULT_GRAY");
	case MP_GRID_COLOR_DARK_GRAY: return _T("MP_GRID_COLOR_DARK_GRAY");
	case MP_GRID_COLOR_YELLOW: return _T("MP_GRID_COLOR_YELLOW");
	case MP_GRID_COLOR_DARK_YELLOW: return _T("MP_GRID_COLOR_DARK_YELLOW");
	case MP_GRID_COLOR_BLUE: return _T("MP_GRID_COLOR_BLUE");
	case MP_GRID_COLOR_DARK_BLUE: return _T("MP_GRID_COLOR_DARK_BLUE");
	case MP_GRID_COLOR_GREEN: return _T("MP_GRID_COLOR_GREEN");
	case MP_GRID_COLOR_DARK_GREEN: return _T("MP_GRID_COLOR_DARK_GREEN");
	case MP_GRID_COLOR_ORANGE: return _T("MP_GRID_COLOR_ORANGE");
	case MP_GRID_COLOR_DARK_ORANGE: return _T("MP_GRID_COLOR_DARK_ORANGE");
	case MP_GRID_COLOR_BK_WIHTE: return _T("MP_GRID_COLOR_BK_WIHTE");
	case MP_GRID_COLOR_BLACK: return _T("MP_GRID_COLOR_BLACK");
	default:
		return _T("MP_GRID_COLOR_UNKNOWN");
	}
}

#endif //PCH_H
