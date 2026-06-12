
// mass_production_toolDlg.cpp: 구현 파일
//

#include "pch.h"
#include "framework.h"
#include "mass_production_tool.h"
#include "mass_production_toolDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

// 구현입니다.
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CmassproductiontoolDlg 대화 상자



CmassproductiontoolDlg::CmassproductiontoolDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MASS_PRODUCTION_TOOL_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CmassproductiontoolDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CmassproductiontoolDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDCANCEL, &CmassproductiontoolDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


// CmassproductiontoolDlg 메시지 처리기

BOOL CmassproductiontoolDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	m_propertySheet.AddPage(&m_rangeTestPage);
	m_propertySheet.AddPage(&m_systemPage);
	m_propertySheet.AddPage(&m_valueTestPage);

	if (!m_propertySheet.Create(this, WS_CHILD | WS_VISIBLE | WS_TABSTOP, 0))
	{
		TRACE(_T("Failed to create property sheet.\n"));
		return TRUE;
	}

	m_propertySheet.ModifyStyleEx(0, WS_EX_CONTROLPARENT);
#if 0
	CWnd* pApplyButton = m_propertySheet.GetDlgItem(ID_APPLY_NOW);
	if (pApplyButton != nullptr)
	{
		pApplyButton->ShowWindow(SW_HIDE);
	}

	CWnd* pOkButton = m_propertySheet.GetDlgItem(IDOK);
	if (pOkButton != nullptr)
	{
		//pOkButton->ShowWindow(SW_HIDE);
	}

	CWnd* pCancelButton = m_propertySheet.GetDlgItem(IDCANCEL);
	if (pCancelButton != nullptr)
	{
		//pCancelButton->ShowWindow(SW_HIDE);
	}
#endif
	LayoutPropertySheet();
	initLogView();

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CmassproductiontoolDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 애플리케이션의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CmassproductiontoolDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CmassproductiontoolDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CmassproductiontoolDlg::LayoutPropertySheet()
{
	if (!::IsWindow(m_propertySheet.GetSafeHwnd()))
	{
		return;
	}

	CWnd* pSheetFrame = GetDlgItem(IDC_STATIC_PROP_SHEET_FRAME);
	if (pSheetFrame == nullptr || !::IsWindow(pSheetFrame->GetSafeHwnd()))
	{
		return;
	}

	CRect sheetRect;
	pSheetFrame->GetWindowRect(&sheetRect);
	ScreenToClient(&sheetRect);
	pSheetFrame->ShowWindow(SW_HIDE);
	m_propertySheet.MoveWindow(sheetRect);
}

int CmassproductiontoolDlg::initLogView()
{
	if (!m_logCtrl.SubclassDlgItem(IDC_MAIN_RICHEDIT2_SYS_LOG, this))
	{
		TRACE(_T("Failed to subclass log rich edit control.\n"));
		return -1;
	}

	m_logCtrl.SetOptions(ECOOP_OR, ECO_AUTOWORDSELECTION); // 단어 선택 등
	m_logCtrl.SetFontSize(10);
	// 수동으로 스타일 설정
	DWORD dwStyle = m_logCtrl.GetStyle();
	dwStyle |= ES_MULTILINE | ES_AUTOVSCROLL;   // 멀티라인 + 자동 세로 스크롤
	dwStyle &= ~ES_AUTOHSCROLL;                 // 가로 스크롤 제거 → 줄바꿈 허용
	m_logCtrl.ModifyStyle(0, dwStyle);
	m_logCtrl.SendMessage(EM_EXLIMITTEXT, 0, (LPARAM)-1);  // 2GB 허용

	// Consolas 10pt 폰트 설정
	CFont* pOldFont = m_logCtrl.GetFont();
	LOGFONT lf;
	memset(&lf, 0, sizeof(LOGFONT));
	if (pOldFont)
		pOldFont->GetLogFont(&lf);

	// 폰트 변경
	_tcscpy_s(lf.lfFaceName, _T("Consolas"));
	lf.lfHeight = -17; // 10pt 정도에 해당 (-MulDiv(10, dpi, 72))
	lf.lfWeight = FW_NORMAL;

	m_font.CreateFontIndirect(&lf);
	m_logCtrl.SetFont(&m_font);
	// 로그 출력
	for (int i = 0; i < 20; ++i)
	{
		CString msg;
		msg.Format(_T("스크롤 테스트 로그 #%d"), i + 1);
		m_logCtrl.AppendLog(LogLevel::DEBUG_U, msg);
	}
	m_logCtrl.AppendLog(LogLevel::INFO_U, _T("로그 시스템 초기화 완료"));
	m_logCtrl.AppendLog(LogLevel::DEBUG_U, _T("디버그 메시지입니다"));
	m_logCtrl.AppendLog(LogLevel::ERROR_U, _T("에러 발생!"));

	// 전체 선택 해제 (커서를 맨 뒤로 이동)
	long nEnd = m_logCtrl.GetTextLength();
	m_logCtrl.SetSel(nEnd, nEnd);

	// TODO: 여기에 구현 코드 추가.
	return 0;
}
void CmassproductiontoolDlg::OnBnClickedCancel()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CDialogEx::OnCancel();
}
