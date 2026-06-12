#pragma once

#include <vector>
#include <afxdlgs.h>
#include "resource.h"
#include "GridCtrl_src/GridCtrl.h"

// CSystemPage dialog
class CSystemPage : public CPropertyPage
{
	DECLARE_DYNAMIC(CSystemPage)

public:
	CSystemPage();
	virtual ~CSystemPage();

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PROPPAGE_SYSTEM };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();

	CBrush m_backgroundBrush;
	COLORREF m_backgroundColor;
	CGridCtrl m_ctrlGrid;
	std::vector<CString> m_columnNames;
	std::vector<std::vector<CString>> m_gridRows;

	void InitGrid();
	void RefreshGrid();

	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnBnClickedBtnSystemReadFile();
	afx_msg void OnBnClickedBtnSystemWriteFile();

	DECLARE_MESSAGE_MAP()
};