#pragma once

#include <memory>
#include <vector>
#include <afxdlgs.h>
#include "resource.h"
#include "GridCtrl_src/GridCtrl.h"
#include "NewCellTypes/GridCellButton.h"
#include "TestInterfaces.h"

// CValueTestPage dialog
class CValueTestPage : public CPropertyPage
{
	DECLARE_DYNAMIC(CValueTestPage)

public:
	CValueTestPage();
	virtual ~CValueTestPage();
	void SetColumnNames(const std::vector<CString>& columnNames);
	BOOL SetGridCellColor(int row, int col, COLORREF rgb);
	BOOL SetGridCellThemeColor(int row, int col, MP_GRID_CELL_COLOR_THEME theme);

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PROPPAGE_VALUE_TEST };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();

	CBrush m_backgroundBrush;
	COLORREF m_backgroundColor;
	CGridCtrl m_ctrlGrid;
	CSize m_resourceGridSize;
	std::unique_ptr<IValueTestInterface> m_valueInterface;
	BOOL m_bRunAllRunning;
	BOOL m_bRunAllStopRequested;
	BOOL m_bRunAllResetRequested;
	int m_nRunAllResumeLoop;
	int m_nRunAllResumeRow;
	std::vector<CString> m_columnNames;
	std::vector<std::vector<CString>> m_gridRows;
	std::vector<MP_GRID_CELL_COLOR> m_gridCellColors;
	CSize m_initialClientSize;
	std::vector<MP_CHILD_LAYOUT> m_childLayouts;
	void InitGrid();
	void ResizeGridToClient();
	int GetMaxDataLengthFromEdit();
	void RefreshGrid();
	void ApplyGridCellColors();
	void ApplyCountColumnTextColors();
	int FindColumnIndex(LPCTSTR columnName) const;
	DWORD GetValueTestDelayMs(int nGridRow) const;
	BOOL DelayWithMessagePump(DWORD delayMs);
	void RunValueTestRow(int nGridRow, BOOL bUseDelay);
	void ResetValueTestResults();
	afx_msg LRESULT OnGridButtonClick(WPARAM wParam, LPARAM lParam);
	afx_msg void OnSize(UINT nType, int cx, int cy);

	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnBnClickedBtnProValueReadFile();
	afx_msg void OnBnClickedBtnProValueWriteFile();
	afx_msg void OnBnClickedBtnPropValueApply();
	afx_msg void OnBnClickedValueBtRunAll();
	afx_msg void OnBnClickedValueBtReset();

	DECLARE_MESSAGE_MAP()
};
