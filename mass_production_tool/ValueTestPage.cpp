#include "pch.h"

#include "ValueTestPage.h"
#include "afxdialogex.h"
#include "ExcelXlsxHelper.h"

// CValueTestPage dialog

IMPLEMENT_DYNAMIC(CValueTestPage, CPropertyPage)

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

CValueTestPage::CValueTestPage()
	: CPropertyPage(IDD_PROPPAGE_VALUE_TEST)
	, m_backgroundColor(GetSysColor(COLOR_3DFACE))
	, m_resourceGridSize(0, 0)
	, m_valueInterface(new CValueTestSimulInterface())
	, m_bRunAllRunning(FALSE)
	, m_bRunAllStopRequested(FALSE)
	, m_bRunAllResetRequested(FALSE)
	, m_nRunAllResumeLoop(0)
	, m_nRunAllResumeRow(0)
{
	m_psp.dwFlags |= PSP_USETITLE;
	m_psp.pszTitle = _T("   VALUE TEST   ");
}

CValueTestPage::~CValueTestPage()
{
}

void CValueTestPage::SetColumnNames(const std::vector<CString>& columnNames)
{
	m_columnNames = columnNames;
}

BOOL CValueTestPage::SetGridCellColor(int row, int col, COLORREF rgb)
{
	const BOOL bResult = UpsertGridColor(m_gridCellColors, row, col, rgb, RGB(0, 0, 0));
	ApplyGridCellColors();
	return bResult;
}

BOOL CValueTestPage::SetGridCellThemeColor(int row, int col, MP_GRID_CELL_COLOR_THEME theme)
{
	const BOOL bResult = UpsertGridColor(m_gridCellColors, row, col, MpGridThemeBkColor(theme), MpGridThemeTextColor(theme));
	ApplyGridCellColors();
	return bResult;
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
	InitGrid();
	SetDlgItemText(IDC_ET_PROP_MAX_DATA_LEN, _T("8"));
	SetDlgItemText(IDC_VALUE_LOOP, _T("1"));
	MpCaptureChildLayout(this, m_initialClientSize, m_childLayouts);
	return TRUE;
}

BEGIN_MESSAGE_MAP(CValueTestPage, CPropertyPage)
	ON_MESSAGE(WM_GRID_BUTTON_CLICK, &CValueTestPage::OnGridButtonClick)
	ON_WM_SIZE()
	ON_WM_CTLCOLOR()
	ON_WM_ERASEBKGND()
	ON_BN_CLICKED(IDC_BTN_PRO_VALUE_READ_FILE, &CValueTestPage::OnBnClickedBtnProValueReadFile)
	ON_BN_CLICKED(IDC_BTN_PRO_VALUE_WRITE_FILE, &CValueTestPage::OnBnClickedBtnProValueWriteFile)
	ON_BN_CLICKED(IDC_BTN_PROP_VALUE_APPLY, &CValueTestPage::OnBnClickedBtnPropValueApply)
	ON_BN_CLICKED(IDC_VALUE_BT_RUN_ALL, &CValueTestPage::OnBnClickedValueBtRunAll)
	ON_BN_CLICKED(IDC_VALUE_BT_RESET, &CValueTestPage::OnBnClickedValueBtReset)
END_MESSAGE_MAP()

int CValueTestPage::GetMaxDataLengthFromEdit()
{
	CString strValue;
	GetDlgItemText(IDC_ET_PROP_MAX_DATA_LEN, strValue);
	strValue.Trim();

	const int nValue = _ttoi(strValue);
	return max(0, nValue);
}

void CValueTestPage::InitGrid()
{
	if (m_ctrlGrid.m_hWnd == NULL)
	{
		m_ctrlGrid.SubclassDlgItem(IDC_CUSTOM_VALUE_GRID, this);
		m_ctrlGrid.ModifyStyle(0, WS_VSCROLL | WS_HSCROLL, SWP_FRAMECHANGED);
	}
	ResizeGridToClient();

	CFont* pDialogFont = GetFont();
	if (pDialogFont != NULL)
	{
		m_ctrlGrid.SetFont(pDialogFont);
	}

	if (m_columnNames.empty())
	{
		m_columnNames = CExcelXlsxHelper::BuildValueColumns(8);
	}

	const int nColumnCount = static_cast<int>(m_columnNames.size());
	if (nColumnCount <= 0)
	{
		return;
	}

	const int nButtonCol = nColumnCount - 1;

	m_ctrlGrid.SetEditable(TRUE);
	m_ctrlGrid.EnableDragAndDrop(FALSE);
	m_ctrlGrid.SetTextBkColor(RGB(255, 255, 255));
	m_ctrlGrid.SetTextColor(RGB(0, 0, 0));
	m_ctrlGrid.SetBkColor(RGB(255, 255, 255));
	m_ctrlGrid.SetGridColor(RGB(0, 0, 0));
	m_ctrlGrid.SetColumnCount(nColumnCount);
	m_ctrlGrid.SetRowCount(20);
	m_ctrlGrid.SetFixedRowCount(1);
	m_ctrlGrid.SetFixedBkColor(RGB(181, 181, 181));
	m_ctrlGrid.SetFixedTextColor(RGB(255, 255, 255));
	m_ctrlGrid.SetColumnWidth(nButtonCol, 80);
	for (int nCol = 0; nCol < m_ctrlGrid.GetColumnCount(); nCol++)
	{
		m_ctrlGrid.SetColumnWidth(nCol, 80);
	}
	m_ctrlGrid.SetColumnWidth(nButtonCol, 100);
	m_ctrlGrid.SetColumnWidth(3, 100);
	RefreshGrid();
}

void CValueTestPage::ResizeGridToClient()
{
	if (m_ctrlGrid.GetSafeHwnd() == NULL)
	{
		return;
	}

	CRect rectClient;
	GetClientRect(&rectClient);

	const int gridWidth = max(10, rectClient.Width() - MP_GRID_CLIENT_X - 1);
	const int gridHeight = max(10, rectClient.Height() - MP_GRID_CLIENT_Y - 1);
	if (gridWidth > 0 && gridHeight > 0)
	{
		m_ctrlGrid.MoveWindow(MP_GRID_CLIENT_X, MP_GRID_CLIENT_Y, gridWidth, gridHeight);
	}
}

void CValueTestPage::RefreshGrid()
{
	const int nColumnCount = static_cast<int>(m_columnNames.size());
	if (nColumnCount <= 0 || m_ctrlGrid.GetSafeHwnd() == NULL)
	{
		return;
	}
	ResizeGridToClient();

	const int nButtonCol = nColumnCount - 1;
	const int nRowCount = static_cast<int>(m_gridRows.size()) + 1;

	m_ctrlGrid.SetRedraw(FALSE);
	m_ctrlGrid.SetColumnCount(nColumnCount);
	m_ctrlGrid.SetRowCount(nRowCount);
	m_ctrlGrid.SetFixedRowCount(1);

	for (int nCol = 0; nCol < m_ctrlGrid.GetColumnCount(); nCol++)
	{
		m_ctrlGrid.SetColumnWidth(nCol, 80);
	}
	m_ctrlGrid.SetColumnWidth(nButtonCol, 100);
	//m_ctrlGrid.SetColumnWidth(nButtonCol - 1, 130);
	//m_ctrlGrid.SetColumnWidth(nButtonCol - 2, 130);
	//m_ctrlGrid.SetColumnWidth(nButtonCol - 3, 130);
	if (nColumnCount > 3)
	{
		m_ctrlGrid.SetColumnWidth(3, 100);
	}

	for (int nRow = 0; nRow < m_ctrlGrid.GetRowCount(); nRow++)
	{
		for (int nCol = 0; nCol < m_ctrlGrid.GetColumnCount(); nCol++)
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
				if (nDataRow >= 0 && nDataRow < static_cast<int>(m_gridRows.size()) &&
					nCol < static_cast<int>(m_gridRows[nDataRow].size()))
				{
					item.strText = m_gridRows[nDataRow][nCol];
				}

				if (nCol == nButtonCol)
				{
					item.strText = _T("RUN");
				}
			}

			m_ctrlGrid.SetItem(&item);
		}
	}

	for (int nRow = 1; nRow < m_ctrlGrid.GetRowCount(); nRow++)
	{
		m_ctrlGrid.SetCellType(nRow, nButtonCol, RUNTIME_CLASS(CGridCellButton));
		m_ctrlGrid.SetItemText(nRow, nButtonCol, _T("RUN"));
	}

	m_ctrlGrid.SetRedraw(TRUE, TRUE);
	m_ctrlGrid.ShowScrollBar(SB_BOTH, TRUE);

	for (int i = 2; i < m_ctrlGrid.GetColumnCount()-5; ++i) {

		SetGridCellThemeColor(0, i, MP_GRID_COLOR_DARK_ORANGE);//min
	}
	int i = m_ctrlGrid.GetColumnCount() - 5;
	SetGridCellThemeColor(0, i, MP_GRID_COLOR_DARK_ORANGE);//min
	SetGridCellThemeColor(0, 0, MP_GRID_COLOR_DARK_ORANGE);//min
	ApplyGridCellColors();
	ApplyCountColumnTextColors();
	m_ctrlGrid.Refresh();
}

void CValueTestPage::ApplyGridCellColors()
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

void CValueTestPage::ApplyCountColumnTextColors()
{
	if (m_ctrlGrid.GetSafeHwnd() == NULL)
	{
		return;
	}

	const int nPassCol = FindColumnIndex(_T("PASS"));
	const int nFailCol = FindColumnIndex(_T("FAIL"));
	const COLORREF passTextColor = MpGridThemeBkColor(MP_GRID_COLOR_DARK_BLUE);
	const COLORREF failTextColor = MpGridThemeBkColor(MP_GRID_COLOR_DARK_RED);

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

int CValueTestPage::FindColumnIndex(LPCTSTR columnName) const
{
	CString target(columnName);
	target.Trim();
	for (size_t i = 0; i < m_columnNames.size(); ++i)
	{
		CString current(m_columnNames[i]);
		current.Trim();
		if (current.CompareNoCase(target) == 0)
		{
			return static_cast<int>(i);
		}
	}
	return -1;
}

DWORD CValueTestPage::GetValueTestDelayMs(int nGridRow) const
{
	const int nDelayCol = FindColumnIndex(_T("DELAY ms"));
	if (nGridRow <= 0 || nDelayCol < 0 || m_ctrlGrid.GetSafeHwnd() == NULL)
	{
		return 0;
	}

	DWORD delayMs = 0;
	MpTryParseUInt32(m_ctrlGrid.GetItemText(nGridRow, nDelayCol), delayMs);
	return delayMs;
}

BOOL CValueTestPage::DelayWithMessagePump(DWORD delayMs)
{
	const DWORD startTick = GetTickCount();
	while (GetTickCount() - startTick < delayMs)
	{
		if (m_bRunAllStopRequested)
		{
			return FALSE;
		}

		MSG msg;
		while (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
			if (m_bRunAllStopRequested)
			{
				return FALSE;
			}
		}

		const DWORD elapsed = GetTickCount() - startTick;
		const DWORD remain = delayMs > elapsed ? delayMs - elapsed : 0;
		Sleep(remain < 10 ? remain : 10);
	}
	return !m_bRunAllStopRequested;
}

void CValueTestPage::RunValueTestRow(int nGridRow, BOOL bUseDelay)
{
	if (nGridRow <= 0 || m_valueInterface.get() == NULL)
	{
		return;
	}

	const int nDataRow = nGridRow - 1;
	if (nDataRow < 0 || nDataRow >= static_cast<int>(m_gridRows.size()))
	{
		return;
	}

	const int nHdrCol = FindColumnIndex(_T("HDR"));
	const int nLengthCol = FindColumnIndex(_T("LENGTH"));
	const int nTypeCol = FindColumnIndex(_T("TYPE"));
	const int nCategoryCol = FindColumnIndex(_T("CATEGORY"));
	const int nOpcodeCol = FindColumnIndex(_T("OPCODE"));
	const int nExpectedCol = FindColumnIndex(_T("EXPECTED"));
	const int nReturnCol = FindColumnIndex(_T("RETURN"));
	const int nPassCol = FindColumnIndex(_T("PASS"));
	const int nFailCol = FindColumnIndex(_T("FAIL"));
	const int nTotalCol = FindColumnIndex(_T("TOTAL"));

	if (nHdrCol < 0 || nLengthCol < 0 || nTypeCol < 0 || nCategoryCol < 0 ||
		nOpcodeCol < 0 || nExpectedCol < 0 || nReturnCol < 0 || nPassCol < 0 || nFailCol < 0)
	{
		TRACE(_T("[VALUE] Required column is missing.\n"));
		return;
	}

	if (m_gridRows[nDataRow].size() < m_columnNames.size())
	{
		m_gridRows[nDataRow].resize(m_columnNames.size());
	}

	BYTE hdr = 0;
	BYTE length = 0;
	BYTE type = 0;
	BYTE category = 0;
	BYTE opcode = 0;
	DWORD expectedValue = 0;
	if (!MpTryParseByte(m_ctrlGrid.GetItemText(nGridRow, nHdrCol), hdr) ||
		!MpTryParseByte(m_ctrlGrid.GetItemText(nGridRow, nLengthCol), length) ||
		!MpTryParseByte(m_ctrlGrid.GetItemText(nGridRow, nTypeCol), type) ||
		!MpTryParseByte(m_ctrlGrid.GetItemText(nGridRow, nCategoryCol), category) ||
		!MpTryParseByte(m_ctrlGrid.GetItemText(nGridRow, nOpcodeCol), opcode) ||
		!MpTryParseUInt32(m_ctrlGrid.GetItemText(nGridRow, nExpectedCol), expectedValue))
	{
		TRACE(_T("[VALUE] Invalid command or EXPECTED value. row=%d\n"), nGridRow);
		return;
	}

	std::vector<BYTE> commandBytes;
	commandBytes.push_back(hdr);
	commandBytes.push_back(length);
	commandBytes.push_back(type);
	commandBytes.push_back(category);
	commandBytes.push_back(opcode);

	for (int nDataIndex = 0; nDataIndex < static_cast<int>(length); ++nDataIndex)
	{
		CString strDataColumn;
		strDataColumn.Format(_T("DATA[%d]"), nDataIndex);
		const int nDataCol = FindColumnIndex(strDataColumn);
		BYTE dataValue = 0;
		if (nDataCol >= 0 && MpTryParseByte(m_ctrlGrid.GetItemText(nGridRow, nDataCol), dataValue))
		{
			commandBytes.push_back(dataValue);
		}
		else
		{
			commandBytes.push_back(0);
		}
	}

	DWORD returnValue = 0;
	if (!m_valueInterface->WriteValueCommand(commandBytes))
	{
		TRACE(_T("[VALUE] Interface command failed. row=%d\n"), nGridRow);
		return;
	}

	if (bUseDelay)
	{
		const DWORD delayMs = GetValueTestDelayMs(nGridRow);
		TRACE(_T("[VALUE] row=%d delay=%lu ms\n"), nGridRow, delayMs);
		if (!DelayWithMessagePump(delayMs))
		{
			TRACE(_T("[VALUE] stop requested before read. row=%d\n"), nGridRow);
			return;
		}
	}

	if (!m_valueInterface->ReadValue(expectedValue, returnValue))
	{
		TRACE(_T("[VALUE] Interface read failed. row=%d\n"), nGridRow);
		return;
	}

	const BOOL bPass = (returnValue == expectedValue);
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

	m_ctrlGrid.SetItemText(nGridRow, nReturnCol, m_gridRows[nDataRow][nReturnCol]);
	m_ctrlGrid.SetItemText(nGridRow, nPassCol, m_gridRows[nDataRow][nPassCol]);
	m_ctrlGrid.SetItemText(nGridRow, nFailCol, m_gridRows[nDataRow][nFailCol]);
	if (nTotalCol >= 0)
	{
		m_ctrlGrid.SetItemText(nGridRow, nTotalCol, m_gridRows[nDataRow][nTotalCol]);
	}
	ApplyCountColumnTextColors();
	m_ctrlGrid.Refresh();

	TRACE(_T("[VALUE] row=%d return=0x%08X expected=0x%08X result=%s pass=%d fail=%d\n"),
		nGridRow, returnValue, expectedValue, bPass ? _T("PASS") : _T("FAIL"), nPassCount, nFailCount);
}

void CValueTestPage::ResetValueTestResults()
{
	const int nReturnCol = FindColumnIndex(_T("RETURN"));
	const int nPassCol = FindColumnIndex(_T("PASS"));
	const int nFailCol = FindColumnIndex(_T("FAIL"));
	const int nTotalCol = FindColumnIndex(_T("TOTAL"));

	for (size_t nDataRow = 0; nDataRow < m_gridRows.size(); ++nDataRow)
	{
		if (m_gridRows[nDataRow].size() < m_columnNames.size())
		{
			m_gridRows[nDataRow].resize(m_columnNames.size());
		}

		if (nReturnCol >= 0)
		{
			m_gridRows[nDataRow][nReturnCol].Empty();
		}
		if (nPassCol >= 0)
		{
			m_gridRows[nDataRow][nPassCol].Empty();
		}
		if (nFailCol >= 0)
		{
			m_gridRows[nDataRow][nFailCol].Empty();
		}
		if (nTotalCol >= 0)
		{
			m_gridRows[nDataRow][nTotalCol].Empty();
		}
	}

	if (m_ctrlGrid.GetSafeHwnd() != NULL)
	{
		RefreshGrid();
	}
}

LRESULT CValueTestPage::OnGridButtonClick(WPARAM wParam, LPARAM lParam)
{
	const int nRow = static_cast<int>(wParam);
	const int nCol = static_cast<int>(lParam);

	CString strMessage;
	strMessage.Format(_T("Value grid button clicked. row=%d, col=%d"), nRow, nCol);
	TRACE(_T("%s\n"), strMessage.GetString());
	RunValueTestRow(nRow, FALSE);

	return 0;
}

void CValueTestPage::OnSize(UINT nType, int cx, int cy)
{
	CPropertyPage::OnSize(nType, cx, cy);
	MpScaleChildLayout(this, m_initialClientSize, m_childLayouts, cx, cy);
	ResizeGridToClient();
	if (m_ctrlGrid.GetSafeHwnd() != NULL)
	{
		m_ctrlGrid.Refresh();
	}
}

void CValueTestPage::OnBnClickedBtnProValueReadFile()
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
	if (!CExcelXlsxHelper::LoadValueSheet(dlg.GetPathName(), m_columnNames, loadedRows))
	{
		AfxMessageBox(_T("Failed to read the Excel file. Check the .xlsx format and first worksheet data."));
		return;
	}

	m_gridRows.swap(loadedRows);
	RefreshGrid();
}

void CValueTestPage::OnBnClickedBtnProValueWriteFile()
{
	CFileDialog dlg(FALSE, _T("xlsx"), _T("value_test_result.xlsx"),
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

	if (!CExcelXlsxHelper::SaveValueSheet(strPath, m_columnNames, gridRows))
	{
		AfxMessageBox(_T("Failed to save the Excel file."));
	}
}

void CValueTestPage::OnBnClickedBtnPropValueApply()
{
	const int nDataLength = GetMaxDataLengthFromEdit();

	CString strNormalized;
	strNormalized.Format(_T("%d"), nDataLength);
	SetDlgItemText(IDC_ET_PROP_MAX_DATA_LEN, strNormalized);

	const std::vector<CString> oldColumnNames = m_columnNames;
	const std::vector<std::vector<CString>> oldRows = m_gridRows;
	m_columnNames = CExcelXlsxHelper::BuildValueColumns(nDataLength);
	m_gridRows = CExcelXlsxHelper::RemapRowsByHeader(oldColumnNames, oldRows, m_columnNames);
	RefreshGrid();
}

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

void CValueTestPage::OnBnClickedValueBtRunAll()
{
	if (m_bRunAllRunning)
	{
		m_bRunAllStopRequested = TRUE;
		TRACE(_T("[VALUE] RUN ALL stop requested.\n"));
		return;
	}

	CString strLoopCount;
	GetDlgItemText(IDC_VALUE_LOOP, strLoopCount);
	strLoopCount.Trim();

	const int nLoopCount = _ttoi(strLoopCount);
	if (nLoopCount <= 0)
	{
		TRACE(_T("[VALUE] RUN ALL skipped. invalid loop count=%s\n"), strLoopCount.GetString());
		return;
	}

	const int nDataRowCount = static_cast<int>(m_gridRows.size());
	if (nDataRowCount <= 0)
	{
		TRACE(_T("[VALUE] RUN ALL skipped. no value test row.\n"));
		return;
	}

	if (m_nRunAllResumeLoop >= nLoopCount || m_nRunAllResumeRow >= nDataRowCount)
	{
		m_nRunAllResumeLoop = 0;
		m_nRunAllResumeRow = 0;
	}

	m_bRunAllRunning = TRUE;
	m_bRunAllStopRequested = FALSE;
	m_bRunAllResetRequested = FALSE;

	TRACE(_T("[VALUE] RUN ALL start. loop=%d, rows=%d, resumeLoop=%d, resumeRow=%d\n"),
		nLoopCount, nDataRowCount, m_nRunAllResumeLoop, m_nRunAllResumeRow);
	for (int nLoop = m_nRunAllResumeLoop; nLoop < nLoopCount; ++nLoop)
	{
		if (m_bRunAllStopRequested)
		{
			break;
		}

		const int nStartRow = (nLoop == m_nRunAllResumeLoop) ? m_nRunAllResumeRow : 0;
		for (int nDataRow = nStartRow; nDataRow < nDataRowCount; ++nDataRow)
		{
			if (m_bRunAllStopRequested)
			{
				break;
			}

			m_nRunAllResumeLoop = nLoop;
			m_nRunAllResumeRow = nDataRow;

			const int nGridRow = nDataRow + 1;
			RunValueTestRow(nGridRow, TRUE);

			if (m_bRunAllStopRequested)
			{
				break;
			}

			if (nDataRow + 1 < nDataRowCount)
			{
				m_nRunAllResumeLoop = nLoop;
				m_nRunAllResumeRow = nDataRow + 1;
			}
			else
			{
				m_nRunAllResumeLoop = nLoop + 1;
				m_nRunAllResumeRow = 0;
			}
		}
	}

	const BOOL bStopped = m_bRunAllStopRequested;
	const BOOL bReset = m_bRunAllResetRequested;
	TRACE(_T("[VALUE] RUN ALL %s. loop=%d, rows=%d\n"),
		bReset ? _T("reset") : (bStopped ? _T("stopped") : _T("done")), nLoopCount, nDataRowCount);

	if (bReset || !bStopped)
	{
		m_nRunAllResumeLoop = 0;
		m_nRunAllResumeRow = 0;
	}

	m_bRunAllRunning = FALSE;
	m_bRunAllStopRequested = FALSE;
	m_bRunAllResetRequested = FALSE;
}

void CValueTestPage::OnBnClickedValueBtReset()
{
	m_bRunAllStopRequested = TRUE;
	m_bRunAllResetRequested = TRUE;
	m_nRunAllResumeLoop = 0;
	m_nRunAllResumeRow = 0;

	ResetValueTestResults();
	TRACE(_T("[VALUE] RESET. stop requested and all test counts cleared.\n"));
}
