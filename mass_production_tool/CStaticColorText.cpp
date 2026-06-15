#include "pch.h"

#include "CStaticColorText.h"

IMPLEMENT_DYNAMIC(CStaticColorText, CStatic)

CStaticColorText::CStaticColorText()
	: m_backColor(GetSysColor(COLOR_3DFACE))
	, m_textColor(GetSysColor(COLOR_WINDOWTEXT))
{
	RebuildBrush();
}

CStaticColorText::~CStaticColorText()
{
}

BEGIN_MESSAGE_MAP(CStaticColorText, CStatic)
	ON_WM_CTLCOLOR_REFLECT()
	ON_WM_PAINT()
END_MESSAGE_MAP()

void CStaticColorText::SetDisplayText(const CString& text)
{
	SetWindowText(text);
	Invalidate();
}

void CStaticColorText::SetBackColor(COLORREF color)
{
	m_backColor = color;
	RebuildBrush();
	Invalidate();
}

void CStaticColorText::SetTextColor(COLORREF color)
{
	m_textColor = color;
	Invalidate();
}

void CStaticColorText::SetThemeColor(MP_GRID_CELL_COLOR_THEME theme)
{
	m_backColor = MpGridThemeBkColor(theme);
	m_textColor = MpGridThemeTextColor(theme);
	RebuildBrush();
	Invalidate();
}

HBRUSH CStaticColorText::CtlColor(CDC* pDC, UINT /*nCtlColor*/)
{
	pDC->SetBkMode(OPAQUE);
	pDC->SetBkColor(m_backColor);
	pDC->SetTextColor(m_textColor);
	return static_cast<HBRUSH>(m_backBrush.GetSafeHandle());
}

void CStaticColorText::OnPaint()
{
	CPaintDC dc(this);

	CRect rect;
	GetClientRect(&rect);

	dc.FillSolidRect(rect, m_backColor);
	dc.SetBkMode(TRANSPARENT);
	dc.SetTextColor(m_textColor);

	CFont* pOldFont = dc.SelectObject(GetFont());

	CString text;
	GetWindowText(text);
	dc.DrawText(text, rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

	if (pOldFont != nullptr)
	{
		dc.SelectObject(pOldFont);
	}
}

void CStaticColorText::RebuildBrush()
{
	m_backBrush.DeleteObject();
	m_backBrush.CreateSolidBrush(m_backColor);
}
