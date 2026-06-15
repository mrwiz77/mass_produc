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
	BOOL SetGridCellColor(int row, int col, COLORREF rgb);
	BOOL SetGridCellThemeColor(int row, int col, MP_GRID_CELL_COLOR_THEME theme);
	BOOL SetDutConfigGridCellColor(int row, int col, COLORREF rgb);
	BOOL SetDutConfigGridCellThemeColor(int row, int col, MP_GRID_CELL_COLOR_THEME theme);

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PROPPAGE_SYSTEM };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();

	CBrush m_backgroundBrush;
	COLORREF m_backgroundColor;
	CGridCtrl m_ctrlGrid;
	CGridCtrl m_dutConfigGrid;
	CSize m_resourceGridSize;
	std::vector<CString> m_columnNames;
	std::vector<std::vector<CString>> m_gridRows;
	std::vector<CString> m_dutConfigColumnNames;
	std::vector<std::vector<CString>> m_dutConfigRows;
	std::vector<MP_GRID_CELL_COLOR> m_gridCellColors;
	std::vector<MP_GRID_CELL_COLOR> m_dutConfigGridCellColors;

	void InitGrid();
	void ResizeGridToClient();
	void RefreshGrid();
	void ApplyGridCellColors();
	void InitDutConfigGrid();
	void ResizeDutConfigGridToClient();
	void RefreshDutConfigGrid();
	void ApplyDutConfigGridCellColors();

	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnBnClickedBtnSystemReadFile();
	afx_msg void OnBnClickedBtnSystemWriteFile();

	DECLARE_MESSAGE_MAP()
};
