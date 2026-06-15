#pragma once

#include <vector>
#include <afxdlgs.h>
#include "resource.h"
#include "GridCtrl_src/GridCtrl.h"
#include "NewCellTypes/GridCellButton.h"
// CRangeTestPage dialog
class CRangeTestPage : public CPropertyPage
{
	DECLARE_DYNAMIC(CRangeTestPage)

public:
	CRangeTestPage();
	virtual ~CRangeTestPage();
	void SetColumnNames(const std::vector<CString>& columnNames);
	BOOL SetGridCellColor(int row, int col, COLORREF rgb);
	BOOL SetGridCellThemeColor(int row, int col, MP_GRID_CELL_COLOR_THEME theme);

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PROPPAGE_RANGE_TEST };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();

	CBrush m_backgroundBrush;
	COLORREF m_backgroundColor;
	CGridCtrl m_ctrlGrid;
	CSize m_resourceGridSize;
	std::vector<CString> m_columnNames;
	std::vector<std::vector<CString>> m_gridRows;
	std::vector<MP_GRID_CELL_COLOR> m_gridCellColors;
	void InitGrid();
	void ResizeGridToClient();
	void RefreshGrid();
	void ApplyGridCellColors();
	afx_msg LRESULT OnGridButtonClick(WPARAM wParam, LPARAM lParam);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnRangeValueReadFile();
	afx_msg void OnBnClickedBtnRangeValueWriteFile();
};
