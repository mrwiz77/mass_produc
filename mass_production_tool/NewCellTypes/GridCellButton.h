#pragma once

#include "../GridCtrl_src/GridCell.h"

#define WM_GRID_BUTTON_CLICK (WM_APP + 0x120)

class CGridCellButton : public CGridCell
{
	DECLARE_DYNCREATE(CGridCellButton)

public:
	CGridCellButton();

	virtual void SetCoords(int nRow, int nCol);
	virtual CSize GetCellExtent(CDC* pDC);
	virtual BOOL Draw(CDC* pDC, int nRow, int nCol, CRect rect, BOOL bEraseBkgnd = TRUE);
	virtual BOOL Edit(int nRow, int nCol, CRect rect, CPoint point, UINT nID, UINT nChar);
	virtual void OnClick(CPoint PointCellRelative);
	virtual void OnClickDown(CPoint PointCellRelative);

protected:
	CRect GetButtonRect() const;
	void InvalidateButton();
	void NotifyButtonClick();

protected:
	int m_nRow;
	int m_nCol;
	CRect m_rectCell;
	BOOL m_bPressed;
};
