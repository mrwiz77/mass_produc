#include "stdafx.h"
#include "GridCellButton.h"
#include "../GridCtrl_src/GridCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CGridCellButton, CGridCell)

CGridCellButton::CGridCellButton()
	: m_nRow(-1)
	, m_nCol(-1)
	, m_bPressed(FALSE)
{
}

void CGridCellButton::SetCoords(int nRow, int nCol)
{
	m_nRow = nRow;
	m_nCol = nCol;
	CGridCell::SetCoords(nRow, nCol);
	SetState(GetState() | GVIS_READONLY);
}

CSize CGridCellButton::GetCellExtent(CDC* pDC)
{
	CSize size = CGridCell::GetCellExtent(pDC);
	size.cx = max(size.cx, 72);
	size.cy = max(size.cy, 24);
	return size;
}

BOOL CGridCellButton::Draw(CDC* pDC, int nRow, int nCol, CRect rect, BOOL bEraseBkgnd)
{
	m_rectCell = rect;

	COLORREF clrBack = GetBackClr();
	if (clrBack == CLR_DEFAULT)
	{
		CGridCellBase* pDefaultCell = GetDefaultCell();
		clrBack = pDefaultCell ? pDefaultCell->GetBackClr() : ::GetSysColor(COLOR_WINDOW);
	}

	pDC->FillSolidRect(rect, clrBack);

	CRect buttonRect = GetButtonRect();
	UINT nButtonState = DFCS_BUTTONPUSH;
	if (m_bPressed)
	{
		nButtonState |= DFCS_PUSHED;
	}

	pDC->DrawFrameControl(buttonRect, DFC_BUTTON, nButtonState);

	CString strText(GetText());
	if (strText.IsEmpty())
	{
		strText = _T("Button");
	}

	CRect textRect = buttonRect;
	if (m_bPressed)
	{
		textRect.OffsetRect(1, 1);
	}

	CFont* pOldFont = NULL;
	CFont* pFont = GetFontObject();
	if (pFont != NULL)
	{
		pOldFont = pDC->SelectObject(pFont);
	}

	pDC->SetBkMode(TRANSPARENT);
	pDC->DrawText(strText, textRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	if (pOldFont != NULL)
	{
		pDC->SelectObject(pOldFont);
	}

	return TRUE;
}

BOOL CGridCellButton::Edit(int nRow, int nCol, CRect rect, CPoint point, UINT nID, UINT nChar)
{
	UNUSED_ALWAYS(nRow);
	UNUSED_ALWAYS(nCol);
	UNUSED_ALWAYS(rect);
	UNUSED_ALWAYS(point);
	UNUSED_ALWAYS(nID);
	UNUSED_ALWAYS(nChar);
	return FALSE;
}

void CGridCellButton::OnClickDown(CPoint PointCellRelative)
{
	CPoint point = PointCellRelative + m_rectCell.TopLeft();
	m_bPressed = GetButtonRect().PtInRect(point);
	InvalidateButton();
}

void CGridCellButton::OnClick(CPoint PointCellRelative)
{
	CPoint point = PointCellRelative + m_rectCell.TopLeft();
	const BOOL bNotify = m_bPressed && GetButtonRect().PtInRect(point);

	m_bPressed = FALSE;
	InvalidateButton();

	if (bNotify)
	{
		NotifyButtonClick();
	}
}

CRect CGridCellButton::GetButtonRect() const
{
	CRect rect = m_rectCell;
	rect.DeflateRect(3, 3);
	return rect;
}

void CGridCellButton::InvalidateButton()
{
	CGridCtrl* pGrid = GetGrid();
	if (pGrid != NULL && pGrid->GetSafeHwnd() != NULL)
	{
		pGrid->RedrawCell(m_nRow, m_nCol);
	}
}

void CGridCellButton::NotifyButtonClick()
{
	CGridCtrl* pGrid = GetGrid();
	if (pGrid == NULL || pGrid->GetSafeHwnd() == NULL)
	{
		return;
	}

	CWnd* pParent = pGrid->GetParent();
	if (pParent != NULL && pParent->GetSafeHwnd() != NULL)
	{
		pParent->SendMessage(WM_GRID_BUTTON_CLICK,
			static_cast<WPARAM>(m_nRow),
			static_cast<LPARAM>(m_nCol));
	}
}
