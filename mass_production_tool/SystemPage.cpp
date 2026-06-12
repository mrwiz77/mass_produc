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
	InitDutConfigGrid();

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
		m_ctrlGrid.ModifyStyle(0, WS_VSCROLL | WS_HSCROLL);
	}

	CFont* pDialogFont = GetFont();
	if (pDialogFont != NULL)
	{
		m_ctrlGrid.SetFont(pDialogFont);
	}

	m_columnNames = CExcelXlsxHelper::BuildSystemColumns();

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

void CSystemPage::InitDutConfigGrid()
{
#ifdef IDC_CUSTOM_SYS_DUT_CFG
	if (m_dutConfigGrid.m_hWnd == NULL)
	{
		m_dutConfigGrid.SubclassDlgItem(IDC_CUSTOM_SYS_DUT_CFG, this);
		m_dutConfigGrid.ModifyStyle(0, WS_VSCROLL | WS_HSCROLL);
	}

	CFont* pDialogFont = GetFont();
	if (pDialogFont != NULL)
	{
		m_dutConfigGrid.SetFont(pDialogFont);
	}

	m_dutConfigColumnNames = CExcelXlsxHelper::BuildSystemDutConfigColumns();

	m_dutConfigGrid.SetEditable(TRUE);
	m_dutConfigGrid.EnableDragAndDrop(FALSE);
	m_dutConfigGrid.SetTextBkColor(RGB(255, 255, 255));
	m_dutConfigGrid.SetTextColor(RGB(0, 0, 0));
	m_dutConfigGrid.SetBkColor(RGB(255, 255, 255));
	m_dutConfigGrid.SetGridColor(RGB(0, 0, 0));
	m_dutConfigGrid.SetFixedBkColor(RGB(120, 120, 120));
	m_dutConfigGrid.SetFixedTextColor(RGB(255, 255, 255));
	RefreshDutConfigGrid();
#endif
}

void CSystemPage::RefreshDutConfigGrid()
{
#ifdef IDC_CUSTOM_SYS_DUT_CFG
	if (m_dutConfigGrid.GetSafeHwnd() == NULL || m_dutConfigColumnNames.empty())
	{
		return;
	}

	const int nColumnCount = static_cast<int>(m_dutConfigColumnNames.size());
	const int nRowCount = static_cast<int>(m_dutConfigRows.size()) + 1;

	m_dutConfigGrid.SetRedraw(FALSE);
	m_dutConfigGrid.SetColumnCount(nColumnCount);
	m_dutConfigGrid.SetRowCount(nRowCount);
	m_dutConfigGrid.SetFixedRowCount(1);
	for (int nCol = 0; nCol < nColumnCount; ++nCol)
	{
		if (nCol == 0)
		{
			m_dutConfigGrid.SetColumnWidth(nCol, 150);
		}
		else if (nCol == 1)
		{
			m_dutConfigGrid.SetColumnWidth(nCol, 340);
		}
		else
		{
			m_dutConfigGrid.SetColumnWidth(nCol, 130);
		}
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
				item.strText = m_dutConfigColumnNames[nCol];
			}
			else
			{
				const int nDataRow = nRow - 1;
				if (nDataRow < static_cast<int>(m_dutConfigRows.size()) &&
					nCol < static_cast<int>(m_dutConfigRows[nDataRow].size()))
				{
					item.strText = m_dutConfigRows[nDataRow][nCol];
				}

				if (nCol == 0)
				{
					item.mask |= GVIF_BKCLR | GVIF_FGCLR;
					item.crBkClr = RGB(31, 78, 120);
					item.crFgClr = RGB(255, 255, 255);
				}
			}

			m_dutConfigGrid.SetItem(&item);
		}
	}

	m_dutConfigGrid.SetRedraw(TRUE, TRUE);
	m_dutConfigGrid.Refresh();
#endif
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

	m_ctrlGrid.SetRedraw(TRUE, TRUE);
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

	std::vector<CString> loadedColumns;
	std::vector<std::vector<CString>> loadedRows;
	if (!CExcelXlsxHelper::LoadSheetAll(dlg.GetPathName(), loadedColumns, loadedRows, 1))
	{
		AfxMessageBox(_T("Failed to read the System Excel file."));
		return;
	}

	m_columnNames.swap(loadedColumns);
	m_gridRows.swap(loadedRows);

	std::vector<CString> loadedDutConfigColumns;
	std::vector<std::vector<CString>> loadedDutConfigRows;
	if (CExcelXlsxHelper::LoadSheetAll(dlg.GetPathName(),
		loadedDutConfigColumns,
		loadedDutConfigRows,
		2))
	{
		m_dutConfigColumnNames.swap(loadedDutConfigColumns);
		m_dutConfigRows.swap(loadedDutConfigRows);
	}

	RefreshGrid();
	RefreshDutConfigGrid();
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
