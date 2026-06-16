#pragma once

#include <memory>
#include <vector>
#include <afxdlgs.h>
#include "resource.h"
#include "GridCtrl_src/GridCtrl.h"
#include "NewCellTypes/GridCellButton.h"
#include "TestInterfaces.h"
// CRangeTestPage dialog
class CRangeTestPage : public CPropertyPage
{
	DECLARE_DYNAMIC(CRangeTestPage)

public:
	CRangeTestPage();
	virtual ~CRangeTestPage();
	void SetColumnNames(const std::vector<CString>& columnNames);
	BOOL LoadRangeConfigFile(const CString& strFilePath);
	void UpdateGridLayout();
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
	std::unique_ptr<IRangeTestInterface> m_rangeInterface;
	std::unique_ptr<IValueTestInterface> m_valueInterface;
	std::unique_ptr<ISystemTestInterface> m_systemInterface;
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
	void RefreshGrid();
	void ApplyGridCellColors();
	void ApplyCountColumnTextColors();
	int FindColumnIndex(LPCTSTR columnName) const;
	BOOL IsIntegratedGrid() const;
	int GetHeaderRowCount() const;
	int GridRowToDataRow(int nGridRow) const;
	CString GetIntegratedTestType(int nGridRow) const;
	DWORD GetRangeTestDelayMs(int nGridRow) const;
	BOOL IsSuccessOnlyEnabled() const;
	BOOL DelayWithMessagePump(DWORD delayMs);
	void RunRangeTestRow(int nGridRow, BOOL bUseDelay);
	void RunIntegratedRangeTestRow(int nGridRow, BOOL bUseDelay);
	void RunIntegratedValueTestRow(int nGridRow, BOOL bUseDelay);
	void RunIntegratedSystemTestRow(int nGridRow, BOOL bUseDelay);
	void ResetRangeTestResults();
	void UpdateOverallTestStatus();
	afx_msg LRESULT OnGridButtonClick(WPARAM wParam, LPARAM lParam);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnRangeValueReadFile();
	afx_msg void OnBnClickedBtnRangeValueWriteFile();
	afx_msg void OnBnClickedRngBtRunAll();
	afx_msg void OnBnClickedRngBtReset();
};
