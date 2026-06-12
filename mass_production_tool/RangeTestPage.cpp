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
	InitGrid();
	return TRUE;
}

BEGIN_MESSAGE_MAP(CRangeTestPage, CPropertyPage)
	ON_WM_CTLCOLOR()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

void CRangeTestPage::InitGrid()
{
	// 변수에 Grid를 연결
	if (m_ctrlGrid.m_hWnd == NULL)
	{
		m_ctrlGrid.SubclassDlgItem(IDC_CUSTOM_RANGE_GRID, this);
	}



	const LPCTSTR NAMES[] = { _T("OPERATION"),_T("ADDR"),_T("DATA[0]"),_T("DATA[1]"),_T("DATA[2]"),_T("DATA[3]"),_T("DATA[4]"),_T("DATA[5]"),_T("DATA[6]"),_T("DATA[7]"),_T("DELAY ms"),_T("RESULT") };
	int nNameSize = sizeof(NAMES) / sizeof(LPCTSTR);

	m_ctrlGrid.SetEditable(TRUE);                    // 수정 가능
	//m_ctrlGrid.SetListMode(TRUE);                    // List Mode 설정 (Cell 클릭 시 한 줄(Row) 전체 선택)
	m_ctrlGrid.EnableDragAndDrop(FALSE);             // Drag And Drop 기능 비활성화
	m_ctrlGrid.SetTextBkColor(RGB(255, 255, 255));   // 기본 Cell 배경색
	m_ctrlGrid.SetTextColor(RGB(0, 0, 0));       // 기본 Cell 텍스트색
	m_ctrlGrid.SetBkColor(RGB(255, 255, 255));       // 기본 배경색 (TextBkColor에도 영향을 줍니다.)
	m_ctrlGrid.SetGridColor(RGB(0, 0, 0));         // Grid의 Line 색
	m_ctrlGrid.SetColumnCount(12);       // 기본으로 생성할 Column의 개수
	m_ctrlGrid.SetRowCount(20);           // 기본으로 생성할 Row의 개수
	m_ctrlGrid.SetFixedRowCount(1);                  // 고정할 Row의 개수
	m_ctrlGrid.SetFixedBkColor(RGB(120, 120, 120));            // 고정된 Cell의 배경색
	m_ctrlGrid.SetFixedTextColor(RGB(255, 255, 255));    // 고정된 Cell의 텍스트색

	for (int nRow = 0; nRow < nNameSize; nRow++)
	{
		for (int nCol = 0; nCol < 12; nCol++)
		{
			// Cell 아이템의 설정
			GV_ITEM item;
			item.mask = GVIF_TEXT | GVIF_FORMAT;
			item.nFormat = DT_CENTER | DT_WORDBREAK;
			item.row = nRow;
			item.col = nCol;

			// 첫 줄을 헤더로 사용하기 위함
			if (nRow == 0)
			{
				//case ColumnIdx::ID:
				item.strText = NAMES[nCol];

			}
			else
			{
				switch (nCol)
				{
				case 0:
					item.strText.Format(_T("%d"), nRow);
					break;
				case 1:
					item.strText.Format(_T("%s"), NAMES[nRow - 1]);
					break;
				case 2:
					item.strText.Format(_T("%d"), rand() % 30);
					break;
				}
			}

			// Cell 데이터 설정
			m_ctrlGrid.SetItem(&item);
		}
	}
}
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
