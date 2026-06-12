#pragma once

#include <afxdlgs.h>
#include "resource.h"
#include "GridCtrl_src/GridCtrl.h"
// CRangeTestPage dialog
class CRangeTestPage : public CPropertyPage
{
	DECLARE_DYNAMIC(CRangeTestPage)

public:
	CRangeTestPage();
	virtual ~CRangeTestPage();

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PROPPAGE_RANGE_TEST };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();

	CBrush m_backgroundBrush;
	COLORREF m_backgroundColor;
	CGridCtrl m_ctrlGrid;
	void InitGrid();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);

	DECLARE_MESSAGE_MAP()
};
