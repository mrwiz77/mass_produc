#include "pch.h"

#include "SystemPage.h"
#include "afxdialogex.h"
#include "ExcelXlsxHelper.h"

// CSystemPage dialog

IMPLEMENT_DYNAMIC(CSystemPage, CPropertyPage)

CSystemPage::CSystemPage()
	: CPropertyPage(IDD_PROPPAGE_SYSTEM)
	, m_backgroundColor(GetSysColor(COLOR_3DFACE))
{
	m_psp.dwFlags |= PSP_USETITLE;
	m_psp.pszTitle = _T("System");
}

CSystemPage::~CSystemPage()
{
}

void CSystemPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}

BOOL CSystemPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	m_backgroundColor = GetSysColor(COLOR_3DFACE);
	m_backgroundBrush.DeleteObject();
	m_backgroundBrush.CreateSolidBrush(m_backgroundColor);
	InitGrid();

	return TRUE;
}

BEGIN_MESSAGE_MAP(CSystemPage, CPropertyPage)
	ON_WM_CTLCOLOR()
	ON_WM_ERASEBKGND()
	ON_BN_CLICKED(IDC_BTN_SYSTEM_READ_FILE, &CSystemPage::OnBnClickedBtnSystemReadFile)
	ON_BN_CLICKED(IDC_BTN_SYSTEM_WRITE_FILE, &CSystemPage::OnBnClickedBtnSystemWriteFile)
END_MESSAGE_MAP()

void CSystemPage::InitGrid()
{
	if (m_ctrlGrid.m_hWnd == NULL)
	{
		m_ctrlGrid.SubclassDlgItem(IDC_CUSTOM_PROP_SYSTEM_GRID, this);
	}

	CFont* pDialogFont = GetFont();
	if (pDialogFont != NULL)
	{
		m_ctrlGrid.SetFont(pDialogFont);
	}

	m_columnNames = CExcelXlsxHelper::BuildSystemColumns();
	if (m_gridRows.empty())
	{
		const std::vector<CString> systemTypes = CExcelXlsxHelper::BuildSystemTypes();
		m_gridRows.resize(systemTypes.size());
		for (size_t nRow = 0; nRow < systemTypes.size(); ++nRow)
		{
			m_gridRows[nRow].resize(m_columnNames.size());
			m_gridRows[nRow][0] = systemTypes[nRow];
		}
	}

	m_ctrlGrid.SetEditable(TRUE);
	m_ctrlGrid.EnableDragAndDrop(FALSE);
	m_ctrlGrid.SetTextBkColor(RGB(255, 255, 255));
	m_ctrlGrid.SetTextColor(RGB(0, 0, 0));
	m_ctrlGrid.SetBkColor(RGB(255, 255, 255));
	m_ctrlGrid.SetGridColor(RGB(0, 0, 0));
	m_ctrlGrid.SetFixedBkColor(RGB(120, 120, 120));
	m_ctrlGrid.SetFixedTextColor(RGB(255, 255, 255));
	RefreshGrid();
}

void CSystemPage::RefreshGrid()
{
	if (m_ctrlGrid.GetSafeHwnd() == NULL || m_columnNames.empty())
	{
		return;
	}

	const int nColumnCount = static_cast<int>(m_columnNames.size());
	const int nRowCount = static_cast<int>(m_gridRows.size()) + 1;

	m_ctrlGrid.SetRedraw(FALSE);
	m_ctrlGrid.SetColumnCount(nColumnCount);
	m_ctrlGrid.SetRowCount(nRowCount);
	m_ctrlGrid.SetFixedRowCount(1);

	for (int nCol = 0; nCol < nColumnCount; ++nCol)
	{
		m_ctrlGrid.SetColumnWidth(nCol, nCol == 0 ? 230 : 130);
	}

	for (int nRow = 0; nRow < nRowCount; ++nRow)
	{
		for (int nCol = 0; nCol < nColumnCount; ++nCol)
		{
			GV_ITEM item;
			item.mask = GVIF_TEXT | GVIF_FORMAT;
			item.nFormat = DT_CENTER | DT_WORDBREAK;
			item.row = nRow;
			item.col = nCol;

			if (nRow == 0)
			{
				item.strText = m_columnNames[nCol];
			}
			else
			{
				const int nDataRow = nRow - 1;
				if (nDataRow < static_cast<int>(m_gridRows.size()) &&
					nCol < static_cast<int>(m_gridRows[nDataRow].size()))
				{
					item.strText = m_gridRows[nDataRow][nCol];
				}

				if (nCol == 0)
				{
					item.mask |= GVIF_BKCLR | GVIF_FGCLR;
					item.crBkClr = RGB(31, 78, 120);
					item.crFgClr = RGB(255, 255, 255);
				}
			}

			m_ctrlGrid.SetItem(&item);
		}
	}

	m_ctrlGrid.SetRedraw(TRUE);
	m_ctrlGrid.Refresh();
}

void CSystemPage::OnBnClickedBtnSystemReadFile()
{
	CFileDialog dlg(TRUE, _T("xlsx"), NULL,
		OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST,
		_T("Excel Workbook (*.xlsx)|*.xlsx||"),
		this);

	if (dlg.DoModal() != IDOK)
	{
		return;
	}

	std::vector<std::vector<CString>> loadedRows;
	if (!CExcelXlsxHelper::LoadSystemSheet(dlg.GetPathName(), loadedRows))
	{
		AfxMessageBox(_T("Failed to read the System Excel file."));
		return;
	}

	m_columnNames = CExcelXlsxHelper::BuildSystemColumns();
	m_gridRows.swap(loadedRows);
	RefreshGrid();
}

void CSystemPage::OnBnClickedBtnSystemWriteFile()
{
	CFileDialog dlg(FALSE, _T("xlsx"), _T("system_result.xlsx"),
		OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST,
		_T("Excel Workbook (*.xlsx)|*.xlsx||"),
		this);

	if (dlg.DoModal() != IDOK)
	{
		return;
	}

	CString strPath = dlg.GetPathName();
	if (strPath.Right(5).CompareNoCase(_T(".xlsx")) != 0)
	{
		strPath += _T(".xlsx");
	}

	std::vector<std::vector<CString>> gridRows;
	for (int nRow = 1; nRow < m_ctrlGrid.GetRowCount(); ++nRow)
	{
		std::vector<CString> rowValues(m_columnNames.size());
		BOOL bHasValue = FALSE;
		for (int nCol = 0; nCol < m_ctrlGrid.GetColumnCount() && nCol < static_cast<int>(m_columnNames.size()); ++nCol)
		{
			CString strValue = m_ctrlGrid.GetItemText(nRow, nCol);
			strValue.Trim();
			rowValues[nCol] = strValue;
			if (!strValue.IsEmpty())
			{
				bHasValue = TRUE;
			}
		}

		if (bHasValue)
		{
			gridRows.push_back(rowValues);
		}
	}

	if (!CExcelXlsxHelper::SaveSystemSheet(strPath, gridRows))
	{
		AfxMessageBox(_T("Failed to save the System Excel file."));
	}
}

HBRUSH CSystemPage::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	if (nCtlColor == CTLCOLOR_DLG || nCtlColor == CTLCOLOR_STATIC)
	{
		pDC->SetBkColor(m_backgroundColor);
		return static_cast<HBRUSH>(m_backgroundBrush.GetSafeHandle());
	}

	return CPropertyPage::OnCtlColor(pDC, pWnd, nCtlColor);
}

BOOL CSystemPage::OnEraseBkgnd(CDC* pDC)
{
	CRect rect;
	GetClientRect(&rect);
	pDC->FillSolidRect(&rect, m_backgroundColor);
	return TRUE;
}
