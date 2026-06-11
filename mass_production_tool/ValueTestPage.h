#pragma once

#include <afxdlgs.h>
#include "resource.h"

// CValueTestPage dialog
class CValueTestPage : public CPropertyPage
{
	DECLARE_DYNAMIC(CValueTestPage)

public:
	CValueTestPage();
	virtual ~CValueTestPage();

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PROPPAGE_VALUE_TEST };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();

	CBrush m_backgroundBrush;
	COLORREF m_backgroundColor;

	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);

	DECLARE_MESSAGE_MAP()
};
