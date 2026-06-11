#include "pch.h"

#include "ValueTestPage.h"
#include "afxdialogex.h"


// CValueTestPage dialog

IMPLEMENT_DYNAMIC(CValueTestPage, CPropertyPage)

CValueTestPage::CValueTestPage()
	: CPropertyPage(IDD_PROPPAGE_VALUE_TEST)
	, m_backgroundColor(GetSysColor(COLOR_3DFACE))
{
	m_psp.dwFlags |= PSP_USETITLE;
	m_psp.pszTitle = _T("Value Test");
}

CValueTestPage::~CValueTestPage()
{
}

void CValueTestPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}

BOOL CValueTestPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	m_backgroundColor = GetSysColor(COLOR_3DFACE);
	m_backgroundBrush.DeleteObject();
	m_backgroundBrush.CreateSolidBrush(m_backgroundColor);

	return TRUE;
}

BEGIN_MESSAGE_MAP(CValueTestPage, CPropertyPage)
	ON_WM_CTLCOLOR()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

HBRUSH CValueTestPage::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	if (nCtlColor == CTLCOLOR_DLG || nCtlColor == CTLCOLOR_STATIC)
	{
		pDC->SetBkColor(m_backgroundColor);
		return static_cast<HBRUSH>(m_backgroundBrush.GetSafeHandle());
	}

	return CPropertyPage::OnCtlColor(pDC, pWnd, nCtlColor);
}

BOOL CValueTestPage::OnEraseBkgnd(CDC* pDC)
{
	CRect rect;
	GetClientRect(&rect);
	pDC->FillSolidRect(&rect, m_backgroundColor);
	return TRUE;
}
