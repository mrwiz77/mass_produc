#include "pch.h"

#include "RangeTestPage.h"
#include "afxdialogex.h"


// CRangeTestPage dialog

IMPLEMENT_DYNAMIC(CRangeTestPage, CPropertyPage)

CRangeTestPage::CRangeTestPage()
	: CPropertyPage(IDD_PROPPAGE_RANGE_TEST)
	, m_backgroundColor(GetSysColor(COLOR_3DFACE))
{
	m_psp.dwFlags |= PSP_USETITLE;
	m_psp.pszTitle = _T("Range Test");
}

CRangeTestPage::~CRangeTestPage()
{
}

void CRangeTestPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}

BOOL CRangeTestPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	m_backgroundColor = GetSysColor(COLOR_3DFACE);
	m_backgroundBrush.DeleteObject();
	m_backgroundBrush.CreateSolidBrush(m_backgroundColor);

	return TRUE;
}

BEGIN_MESSAGE_MAP(CRangeTestPage, CPropertyPage)
	ON_WM_CTLCOLOR()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

HBRUSH CRangeTestPage::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	if (nCtlColor == CTLCOLOR_DLG || nCtlColor == CTLCOLOR_STATIC)
	{
		pDC->SetBkColor(m_backgroundColor);
		return static_cast<HBRUSH>(m_backgroundBrush.GetSafeHandle());
	}

	return CPropertyPage::OnCtlColor(pDC, pWnd, nCtlColor);
}

BOOL CRangeTestPage::OnEraseBkgnd(CDC* pDC)
{
	CRect rect;
	GetClientRect(&rect);
	pDC->FillSolidRect(&rect, m_backgroundColor);
	return TRUE;
}
