
// mass_production_toolDlg.h: 헤더 파일
//

#pragma once

#include <vector>
#include <afxdlgs.h>
#include <afxcmn.h>
#include "RangeTestPage.h"
#include "SystemPage.h"
#include "ValueTestPage.h"
#include "CLogView.h"
#include "CStaticColorText.h"
#include "TestInterfaces.h"

class CMpPropertySheet : public CPropertySheet
{
public:
	CMpPropertySheet();
	void SetBackgroundColor(COLORREF color);

protected:
	CBrush m_backgroundBrush;
	COLORREF m_backgroundColor;
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	DECLARE_MESSAGE_MAP()
};

// CmassproductiontoolDlg 대화 상자
class CmassproductiontoolDlg : public CDialogEx
{
// 생성입니다.
public:
	CmassproductiontoolDlg(CWnd* pParent = nullptr);	// 표준 생성자입니다.
	BOOL LoadSystemConfigFile(const CString& strFilePath, BOOL bLoadRangeGrid);
	void UpdateOverallTestStatus(BOOL bAllCompleted, BOOL bAnyFail);
	void ResetOverallTestStatus();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MASS_PRODUCTION_TOOL_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.
	CMpPropertySheet m_propertySheet;
	CRangeTestPage m_rangeTestPage;
	CSystemPage m_systemPage;
	CValueTestPage m_valueTestPage;
	CLogView m_logCtrl;
	CStaticColorText m_decHexStatic;
	CStaticColorText m_decNormalStatic;
	CStaticColorText m_networkStatic;
	CStaticColorText m_syslogStatic;
#ifdef IDC_MAIN_LIST
	CListCtrl m_mainListCtrl;
#endif
	CFont m_font;
	CBrush m_backgroundBrush;
	COLORREF m_backgroundColor;
	MP_SYSTEM_TIMEOUT_CONFIG m_timeoutConfig;
	MP_SYSTEM_CHECK_CONFIG m_checkConfig;
	std::vector<CString> m_rangeGridColumns;
	std::vector<CString> m_VauleColumns;
	CSize m_initialClientSize;
	std::vector<MP_CHILD_LAYOUT> m_childLayouts;
	void InitRangeGridColumns(int nDataLength);
	void InitValueGridColumns(int nDataLength);
	void LayoutPropertySheet();
	int initLogView();
	BOOL LoadStartupSystemConfigFile();
	void DisplaySystemConfigList(const std::vector<CString>& columns, const std::vector<std::vector<CString>>& rows);
	void ApplySystemConfigRows(const std::vector<CString>& columns, const std::vector<std::vector<CString>>& rows);
	int FindColumnIndex(const std::vector<CString>& columns, LPCTSTR columnName) const;
	CString GetMainInterfaceRadioName(UINT nCheckedId) const;
	void TraceMainInterfaceRadioState(LPCTSTR pszReason) const;
	void MakeMainInterfaceControlsTransparent();
	
// 구현입니다.
protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedMainInterfaceRadio();
};
