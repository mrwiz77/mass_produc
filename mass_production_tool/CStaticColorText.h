#pragma once

class CStaticColorText : public CStatic
{
	DECLARE_DYNAMIC(CStaticColorText)

public:
	CStaticColorText();
	virtual ~CStaticColorText();

	void SetDisplayText(const CString& text);
	void SetBackColor(COLORREF color);
	void SetTextColor(COLORREF color);
	void SetThemeColor(MP_GRID_CELL_COLOR_THEME theme);

	COLORREF GetBackColor() const { return m_backColor; }
	COLORREF GetTextColor() const { return m_textColor; }

protected:
	afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	DECLARE_MESSAGE_MAP()

private:
	void RebuildBrush();

	COLORREF m_backColor;
	COLORREF m_textColor;
	CBrush m_backBrush;
};
