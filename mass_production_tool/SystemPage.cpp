#include "pch.h"

#include "SystemPage.h"
#include "afxdialogex.h"
#include "ExcelXlsxHelper.h"

// CSystemPage dialog

IMPLEMENT_DYNAMIC(CSystemPage, CPropertyPage)

namespace
{
	BOOL UpsertGridColor(std::vector<MP_GRID_CELL_COLOR>& colors, int row, int col, COLORREF bkColor, COLORREF textColor)
	{
		if (row < 0 || col < 0)
		{
			return FALSE;
		}

		for (size_t i = 0; i < colors.size(); ++i)
		{
			if (colors[i].row == row && colors[i].col == col)
			{
				colors[i].bkColor = bkColor;
				colors[i].textColor = textColor;
				return TRUE;
			}
		}

		MP_GRID_CELL_COLOR color = { row, col, bkColor, textColor };
		colors.push_back(color);
		return TRUE;
	}

}

CSystemPage::CSystemPage()
	: CPropertyPage(IDD_PROPPAGE_SYSTEM)
	, m_backgroundColor(GetSysColor(COLOR_3DFACE))
	, m_resourceGridSize(0, 0)
{
	m_psp.dwFlags |= PSP_USETITLE;
	m_psp.pszTitle = _T("   SYSTEM   ");
}

CSystemPage::~CSystemPage()
{
}

BOOL CSystemPage::SetGridCellColor(int row, int col, COLORREF rgb)
{
	const BOOL bResult = UpsertGridColor(m_gridCellColors, row, col, rgb, RGB(0, 0, 0));
	ApplyGridCellColors();
	return bResult;
}

BOOL CSystemPage::SetGridCellThemeColor(int row, int col, MP_GRID_CELL_COLOR_THEME theme)
{
	const BOOL bResult = UpsertGridColor(m_gridCellColors, row, col, MpGridThemeBkColor(theme), MpGridThemeTextColor(theme));
	ApplyGridCellColors();
	return bResult;
}

BOOL CSystemPage::SetDutConfigGridCellColor(int row, int col, COLORREF rgb)
{
	const BOOL bResult = UpsertGridColor(m_dutConfigGridCellColors, row, col, rgb, RGB(0, 0, 0));
	ApplyDutConfigGridCellColors();
	return bResult;
}

BOOL CSystemPage::SetDutConfigGridCellThemeColor(int row, int col, MP_GRID_CELL_COLOR_THEME theme)
{
	const BOOL bResult = UpsertGridColor(m_dutConfigGridCellColors, row, col, MpGridThemeBkColor(theme), MpGridThemeTextColor(theme));
	ApplyDutConfigGridCellColors();
	return bResult;
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
		m_ctrlGrid.ModifyStyle(0, WS_VSCROLL | WS_HSCROLL, SWP_FRAMECHANGED);
	}
	ResizeGridToClient();

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

void CSystemPage::ResizeGridToClient()
{
	if (m_ctrlGrid.GetSafeHwnd() == NULL)
	{
		return;
	}

	CRect rectGrid;
	m_ctrlGrid.GetWindowRect(&rectGrid);
	ScreenToClient(&rectGrid);

	if (m_resourceGridSize.cx <= 0 || m_resourceGridSize.cy <= 0)
	{
		m_resourceGridSize.cx = rectGrid.Width();
		m_resourceGridSize.cy = rectGrid.Height();
	}

	if (m_resourceGridSize.cx > 0 && m_resourceGridSize.cy > 0)
	{
		m_ctrlGrid.MoveWindow(MP_GRID_CLIENT_X, MP_GRID_CLIENT_Y, m_resourceGridSize.cx, m_resourceGridSize.cy);
	}
}

void CSystemPage::InitDutConfigGrid()
{
#ifdef IDC_CUSTOM_SYS_DUT_CFG
	if (m_dutConfigGrid.m_hWnd == NULL)
	{
		m_dutConfigGrid.SubclassDlgItem(IDC_CUSTOM_SYS_DUT_CFG, this);
		m_dutConfigGrid.ModifyStyle(0, WS_VSCROLL | WS_HSCROLL);
	}
	ResizeDutConfigGridToClient();

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

void CSystemPage::ResizeDutConfigGridToClient()
{
#ifdef IDC_CUSTOM_SYS_DUT_CFG
	if (m_dutConfigGrid.GetSafeHwnd() == NULL)
	{
		return;
	}

	CRect rectGrid;
	m_dutConfigGrid.GetWindowRect(&rectGrid);
	ScreenToClient(&rectGrid);
	m_dutConfigGrid.MoveWindow(rectGrid.left, MP_GRID_CLIENT_Y, rectGrid.Width(), rectGrid.Height());
#endif
}

void CSystemPage::RefreshDutConfigGrid()
{
#ifdef IDC_CUSTOM_SYS_DUT_CFG
	if (m_dutConfigGrid.GetSafeHwnd() == NULL || m_dutConfigColumnNames.empty())
	{
		return;
	}
	ResizeDutConfigGridToClient();

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

			}

			m_dutConfigGrid.SetItem(&item);
		}
	}

	m_dutConfigGrid.SetRedraw(TRUE, TRUE);
	ApplyDutConfigGridCellColors();
	m_dutConfigGrid.Refresh();
#endif
}

void CSystemPage::ApplyDutConfigGridCellColors()
{
#ifdef IDC_CUSTOM_SYS_DUT_CFG
	if (m_dutConfigGrid.GetSafeHwnd() == NULL)
	{
		return;
	}

	for (size_t i = 0; i < m_dutConfigGridCellColors.size(); ++i)
	{
		const MP_GRID_CELL_COLOR& color = m_dutConfigGridCellColors[i];
		if (color.row >= 0 && color.row < m_dutConfigGrid.GetRowCount() &&
			color.col >= 0 && color.col < m_dutConfigGrid.GetColumnCount())
		{
			m_dutConfigGrid.SetItemBkColour(color.row, color.col, color.bkColor);
			m_dutConfigGrid.SetItemFgColour(color.row, color.col, color.textColor);
		}
	}
#endif
}

void CSystemPage::RefreshGrid()
{
	if (m_ctrlGrid.GetSafeHwnd() == NULL || m_columnNames.empty())
	{
		return;
	}
	ResizeGridToClient();

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

			}

			m_ctrlGrid.SetItem(&item);
		}
	}

	m_ctrlGrid.SetRedraw(TRUE, TRUE);
	ApplyGridCellColors();
	m_ctrlGrid.Refresh();
}

void CSystemPage::ApplyGridCellColors()
{
	if (m_ctrlGrid.GetSafeHwnd() == NULL)
	{
		return;
	}

	for (size_t i = 0; i < m_gridCellColors.size(); ++i)
	{
		const MP_GRID_CELL_COLOR& color = m_gridCellColors[i];
		if (color.row >= 0 && color.row < m_ctrlGrid.GetRowCount() &&
			color.col >= 0 && color.col < m_ctrlGrid.GetColumnCount())
		{
			m_ctrlGrid.SetItemBkColour(color.row, color.col, color.bkColor);
			m_ctrlGrid.SetItemFgColour(color.row, color.col, color.textColor);
		}
	}
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

	for (int row = 1; row < m_ctrlGrid.GetRowCount(); ++row)
	{
		SetGridCellThemeColor(row, 0, MP_GRID_COLOR_BLUE);
	}

#ifdef IDC_CUSTOM_SYS_DUT_CFG
	for (int row = 1; row < m_dutConfigGrid.GetRowCount(); ++row)
	{
		SetDutConfigGridCellThemeColor(row, 0, MP_GRID_COLOR_BLUE);
	}
#endif
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
