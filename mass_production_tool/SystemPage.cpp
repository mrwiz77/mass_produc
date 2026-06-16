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
	, m_systemInterface(new CSystemTestSimulInterface())
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
	SetDlgItemText(IDC_SYS_LOOP, _T("1"));
	MpCaptureChildLayout(this, m_initialClientSize, m_childLayouts);
	return TRUE;
}

BEGIN_MESSAGE_MAP(CSystemPage, CPropertyPage)
	ON_WM_SIZE()
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
	m_ctrlGrid.SetFixedBkColor(RGB(181, 181, 181));
	m_ctrlGrid.SetFixedTextColor(RGB(255, 255, 255));
	RefreshGrid();
}

void CSystemPage::ResizeGridToClient()
{
	if (m_ctrlGrid.GetSafeHwnd() == NULL)
	{
		return;
	}

	CRect rectClient;
	GetClientRect(&rectClient);
	const int gap = 20;
	const int gridTop = MP_GRID_CLIENT_Y;
	const int availableWidth = max(20, rectClient.Width() - MP_GRID_CLIENT_X - 1);
	const int gridWidth = max(10, (availableWidth - gap) / 2);
	const int gridHeight = max(10, rectClient.Height() - gridTop - 1);

	if (gridWidth > 0 && gridHeight > 0)
	{
		m_ctrlGrid.MoveWindow(MP_GRID_CLIENT_X, gridTop, gridWidth, gridHeight);
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
	m_dutConfigGrid.SetFixedBkColor(RGB(181, 181, 181));
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

	CRect rectClient;
	GetClientRect(&rectClient);
	const int gap = 20;
	const int gridTop = MP_GRID_CLIENT_Y;
	const int availableWidth = max(20, rectClient.Width() - MP_GRID_CLIENT_X - 1);
	const int leftGridWidth = max(10, (availableWidth - gap) / 2);
	const int rightGridLeft = MP_GRID_CLIENT_X + leftGridWidth + gap;
	const int rightGridWidth = max(10, rectClient.Width() - rightGridLeft - 1);
	const int gridHeight = max(10, rectClient.Height() - gridTop - 1);

	m_dutConfigGrid.MoveWindow(rightGridLeft, gridTop, rightGridWidth, gridHeight);
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
			m_dutConfigGrid.SetColumnWidth(nCol, MulDiv(150, 110, 100));
		}
		else if (nCol == 1)
		{
			m_dutConfigGrid.SetColumnWidth(nCol, MulDiv(340, 110, 100));
		}
		else
		{
			m_dutConfigGrid.SetColumnWidth(nCol, MulDiv(130, 110, 100));
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
		m_ctrlGrid.SetColumnWidth(nCol, MulDiv(nCol == 0 ? 230 : 130, 110, 100));
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

	const COLORREF passTextColor = MpGridThemeBkColor(MP_GRID_COLOR_DARK_BLUE);
	const COLORREF failTextColor = MpGridThemeBkColor(MP_GRID_COLOR_DARK_RED);
	int nPassCol = FindColumnIndex(m_columnNames, _T("PASS"));
	int nFailCol = FindColumnIndex(m_columnNames, _T("FAIL"));

	for (int nRow = 0; nRow < m_ctrlGrid.GetRowCount(); ++nRow)
	{
		if (nPassCol >= 0 && nPassCol < m_ctrlGrid.GetColumnCount())
		{
			m_ctrlGrid.SetItemFgColour(nRow, nPassCol, passTextColor);
		}

		if (nFailCol >= 0 && nFailCol < m_ctrlGrid.GetColumnCount())
		{
			m_ctrlGrid.SetItemFgColour(nRow, nFailCol, failTextColor);

		}
	}
}

int CSystemPage::FindColumnIndex(const std::vector<CString>& columns, LPCTSTR columnName) const
{
	CString target(columnName);
	target.Trim();
	for (size_t i = 0; i < columns.size(); ++i)
	{
		CString current(columns[i]);
		current.Trim();
		if (current.CompareNoCase(target) == 0)
		{
			return static_cast<int>(i);
		}
	}
	return -1;
}

void CSystemPage::LoadDutConfigFromGridRows()
{
	m_timeoutConfig.Reset();
	m_checkConfig.Reset();

	const int nGroupCol = FindColumnIndex(m_dutConfigColumnNames, _T("GROUP"));
	const int nItemCol = FindColumnIndex(m_dutConfigColumnNames, _T("ITEM"));
	const int nValueCol = FindColumnIndex(m_dutConfigColumnNames, _T("VALUE"));
	if (nGroupCol < 0 || nItemCol < 0 || nValueCol < 0)
	{
		return;
	}

	for (size_t nRow = 0; nRow < m_dutConfigRows.size(); ++nRow)
	{
		if (nGroupCol >= static_cast<int>(m_dutConfigRows[nRow].size()) ||
			nItemCol >= static_cast<int>(m_dutConfigRows[nRow].size()) ||
			nValueCol >= static_cast<int>(m_dutConfigRows[nRow].size()))
		{
			continue;
		}

		const CString strGroup = m_dutConfigRows[nRow][nGroupCol];
		const CString strItem = m_dutConfigRows[nRow][nItemCol];
		const CString strValue = m_dutConfigRows[nRow][nValueCol];

		CString strGroupUpper(strGroup);
		strGroupUpper.Trim();
		strGroupUpper.MakeUpper();
		DWORD numericValue = 0;
		MpTryParseUInt32(strValue, numericValue);

		if (strGroupUpper == _T("TIMEOUT"))
		{
			m_timeoutConfig.SetTimeout(strItem, numericValue);
		}
		else
		{
			m_checkConfig.SetCheck(strGroup, strItem, numericValue != 0);
		}
	}

	TRACE(_T("[SYSTEM CFG] Timeout/Check config loaded from DUT config grid rows.\n"));
}

void CSystemPage::RunSystemTests()
{
	if (m_systemInterface.get() == NULL)
	{
		return;
	}

	int nTypeCol = FindColumnIndex(m_columnNames, _T("TYPE"));
	int nExpectedCol = FindColumnIndex(m_columnNames, _T("EXPECTED"));
	if (nExpectedCol < 0)
	{
		nExpectedCol = FindColumnIndex(m_columnNames, _T("EXPECTEDVALUE"));
	}

	int nReturnCol = FindColumnIndex(m_columnNames, _T("RETURN"));
	if (nReturnCol < 0)
	{
		nReturnCol = FindColumnIndex(m_columnNames, _T("RETURN_VALUE"));
	}

	int nPassCol = FindColumnIndex(m_columnNames, _T("PASS"));
	int nFailCol = FindColumnIndex(m_columnNames, _T("FAIL"));
	int nTotalCol = FindColumnIndex(m_columnNames, _T("TOTAL"));

	if (nTypeCol < 0 || nExpectedCol < 0 || nReturnCol < 0 || nPassCol < 0 || nFailCol < 0)
	{
		TRACE(_T("[SYSTEM] Required column is missing.\n"));
		return;
	}

	for (int nDataRow = 0; nDataRow < static_cast<int>(m_gridRows.size()); ++nDataRow)
	{
		if (m_gridRows[nDataRow].size() < m_columnNames.size())
		{
			m_gridRows[nDataRow].resize(m_columnNames.size());
		}

		CString strType = m_gridRows[nDataRow][nTypeCol];
		BYTE typeValue = 0;
		if (!MpTryGetSystemTestType(strType, typeValue) && !MpTryParseByte(strType, typeValue))
		{
			TRACE(_T("[SYSTEM] Unknown TYPE. row=%d, type=%s\n"), nDataRow + 1, strType.GetString());
			continue;
		}

		DWORD expectedValue = 0;
		if (!MpTryParseUInt32(m_gridRows[nDataRow][nExpectedCol], expectedValue))
		{
			TRACE(_T("[SYSTEM] Invalid EXPECTED value. row=%d\n"), nDataRow + 1);
			continue;
		}

		DWORD returnValue = 0;
		if (!m_systemInterface->WriteSystemRequest(typeValue) ||
			!m_systemInterface->ReadSystemValue(expectedValue, returnValue))
		{
			TRACE(_T("[SYSTEM] Interface command failed. row=%d\n"), nDataRow + 1);
			continue;
		}

		const BOOL bPass = (returnValue == expectedValue);
		const int nGridRow = nDataRow + 1;
		const int nPassCount = MpReadCountText(m_ctrlGrid.GetItemText(nGridRow, nPassCol)) + (bPass ? 1 : 0);
		const int nFailCount = MpReadCountText(m_ctrlGrid.GetItemText(nGridRow, nFailCol)) + (bPass ? 0 : 1);
		const int nTotalCount = nPassCount + nFailCount;

		m_gridRows[nDataRow][nReturnCol] = MpFormatHex32(returnValue);
		m_gridRows[nDataRow][nPassCol] = MpFormatDecimal(nPassCount);
		m_gridRows[nDataRow][nFailCol] = MpFormatDecimal(nFailCount);
		if (nTotalCol >= 0)
		{
			m_gridRows[nDataRow][nTotalCol] = MpFormatDecimal(nTotalCount);
		}

		if (m_ctrlGrid.GetSafeHwnd() != NULL && nGridRow < m_ctrlGrid.GetRowCount())
		{
			m_ctrlGrid.SetItemText(nGridRow, nReturnCol, m_gridRows[nDataRow][nReturnCol]);
			m_ctrlGrid.SetItemText(nGridRow, nPassCol, m_gridRows[nDataRow][nPassCol]);
			m_ctrlGrid.SetItemText(nGridRow, nFailCol, m_gridRows[nDataRow][nFailCol]);
			if (nTotalCol >= 0)
			{
				m_ctrlGrid.SetItemText(nGridRow, nTotalCol, m_gridRows[nDataRow][nTotalCol]);
			}
		}

		TRACE(_T("[SYSTEM] row=%d type=0x%02X return=0x%08X expected=0x%08X result=%s pass=%d fail=%d total=%d\n"),
			nGridRow, typeValue, returnValue, expectedValue, bPass ? _T("PASS") : _T("FAIL"), nPassCount, nFailCount, nTotalCount);
	}

	if (m_ctrlGrid.GetSafeHwnd() != NULL)
	{
		m_ctrlGrid.Refresh();
	}
}

void CSystemPage::OnSize(UINT nType, int cx, int cy)
{
	CPropertyPage::OnSize(nType, cx, cy);
	MpScaleChildLayout(this, m_initialClientSize, m_childLayouts, cx, cy);
	ResizeGridToClient();
	ResizeDutConfigGridToClient();
	if (m_ctrlGrid.GetSafeHwnd() != NULL)
	{
		m_ctrlGrid.Refresh();
	}
#ifdef IDC_CUSTOM_SYS_DUT_CFG
	if (m_dutConfigGrid.GetSafeHwnd() != NULL)
	{
		m_dutConfigGrid.Refresh();
	}
#endif
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
	LoadDutConfigFromGridRows();
	RunSystemTests();

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
