
// mass_production_toolDlg.h: 헤더 파일
//

#pragma once

#include <afxdlgs.h>
#include "RangeTestPage.h"
#include "SystemPage.h"
#include "ValueTestPage.h"
#include "CLogView.h"

// CmassproductiontoolDlg 대화 상자
class CmassproductiontoolDlg : public CDialogEx
{
// 생성입니다.
public:
	CmassproductiontoolDlg(CWnd* pParent = nullptr);	// 표준 생성자입니다.

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MASS_PRODUCTION_TOOL_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.
	CPropertySheet m_propertySheet;
	CRangeTestPage m_rangeTestPage;
	CSystemPage m_systemPage;
	CValueTestPage m_valueTestPage;
	CLogView m_logCtrl;
	CFont m_font;
	void LayoutPropertySheet();
	int initLogView();
// 구현입니다.
protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedCancel();
};
