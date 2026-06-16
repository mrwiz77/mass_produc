#include "pch.h"

#include "RangeTestPage.h"
#include "afxdialogex.h"
#include "ExcelXlsxHelper.h"
// CRangeTestPage dialog

IMPLEMENT_DYNAMIC(CRangeTestPage, CPropertyPage)

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

CRangeTestPage::CRangeTestPage()
	: CPropertyPage(IDD_PROPPAGE_RANGE_TEST)
	, m_backgroundColor(GetSysColor(COLOR_3DFACE))
	, m_resourceGridSize(0, 0)
	, m_rangeInterface(new CRangeTestSimulInterface())
	, m_bRunAllRunning(FALSE)
	, m_bRunAllStopRequested(FALSE)
	, m_bRunAllResetRequested(FALSE)
	, m_nRunAllResumeLoop(0)
	, m_nRunAllResumeRow(0)
{
	m_psp.dwFlags |= PSP_USETITLE;
	m_psp.pszTitle = _T("    RANGE TEST   ");
}

CRangeTestPage::~CRangeTestPage()
{
}

void CRangeTestPage::SetColumnNames(const std::vector<CString>& columnNames)
{
	m_columnNames = columnNames;
}

BOOL CRangeTestPage::SetGridCellColor(int row, int col, COLORREF rgb)
{
	const BOOL bResult = UpsertGridColor(m_gridCellColors, row, col, rgb, RGB(0, 0, 0));
	ApplyGridCellColors();
	return bResult;
}

BOOL CRangeTestPage::SetGridCellThemeColor(int row, int col, MP_GRID_CELL_COLOR_THEME theme)
{
	const BOOL bResult = UpsertGridColor(m_gridCellColors, row, col, MpGridThemeBkColor(theme), MpGridThemeTextColor(theme));
	ApplyGridCellColors();
	return bResult;
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
	SetDlgItemText(IDC_RNG_LOOP, _T("1"));
	MpCaptureChildLayout(this, m_initialClientSize, m_childLayouts);
	return TRUE;
}

BEGIN_MESSAGE_MAP(CRangeTestPage, CPropertyPage)
	ON_MESSAGE(WM_GRID_BUTTON_CLICK, &CRangeTestPage::OnGridButtonClick)
	ON_WM_SIZE()
	ON_WM_CTLCOLOR()
	ON_WM_ERASEBKGND()
	ON_BN_CLICKED(IDC_BTN_RANGE_VALUE_READ_FILE, &CRangeTestPage::OnBnClickedBtnRangeValueReadFile)
	ON_BN_CLICKED(IDC_BTN_RANGE_VALUE_WRITE_FILE, &CRangeTestPage::OnBnClickedBtnRangeValueWriteFile)
	ON_BN_CLICKED(IDC_RNG_BT_RUN_ALL, &CRangeTestPage::OnBnClickedRngBtRunAll)
	ON_BN_CLICKED(IDC_RNG_BT_RESET, &CRangeTestPage::OnBnClickedRngBtReset)
END_MESSAGE_MAP()

void CRangeTestPage::InitGrid()
{
	if (m_ctrlGrid.m_hWnd == NULL)
	{
		m_ctrlGrid.SubclassDlgItem(IDC_CUSTOM_RANGE_GRID, this);
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
		m_columnNames = CExcelXlsxHelper::BuildRangeColumns();
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
	
	for (int nCol = 0; nCol < m_ctrlGrid.GetColumnCount(); nCol++)
	{
		m_ctrlGrid.SetColumnWidth(nCol, 130);
	}
	m_ctrlGrid.SetColumnWidth(nButtonCol, 100);
	//m_ctrlGrid.SetColumnWidth(3, 100);
	RefreshGrid();
}

void CRangeTestPage::ResizeGridToClient()
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


void CRangeTestPage::RefreshGrid()
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
		m_ctrlGrid.SetColumnWidth(nCol, 130);
	}
	m_ctrlGrid.SetColumnWidth(nButtonCol, 100);

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
	SetGridCellThemeColor(0, 0, MP_GRID_COLOR_DARK_ORANGE);//index
	//for (int i = 2; i < m_ctrlGrid.GetColumnCount(); ++i) {
	//
	//	SetGridCellThemeColor(0, i, MP_GRID_COLOR_BLUE);//min
	//}
	
	ApplyGridCellColors();
	ApplyCountColumnTextColors();
	m_ctrlGrid.Refresh();
}

void CRangeTestPage::ApplyGridCellColors()
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

void CRangeTestPage::ApplyCountColumnTextColors()
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

int CRangeTestPage::FindColumnIndex(LPCTSTR columnName) const
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

DWORD CRangeTestPage::GetRangeTestDelayMs(int nGridRow) const
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

BOOL CRangeTestPage::DelayWithMessagePump(DWORD delayMs)
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

void CRangeTestPage::RunRangeTestRow(int nGridRow, BOOL bUseDelay)
{
	if (nGridRow <= 0 || m_rangeInterface.get() == NULL)
	{
		return;
	}

	const int nDataRow = nGridRow - 1;
	if (nDataRow < 0 || nDataRow >= static_cast<int>(m_gridRows.size()))
	{
		return;
	}

	const int nIndexCol = FindColumnIndex(_T("INDEX"));
	const int nMinCol = FindColumnIndex(_T("MIN"));
	const int nMaxCol = FindColumnIndex(_T("MAX"));
	const int nReturnCol = FindColumnIndex(_T("RETURN"));
	const int nPassCol = FindColumnIndex(_T("PASS"));
	const int nFailCol = FindColumnIndex(_T("FAIL"));
	const int nTotalCol = FindColumnIndex(_T("TOTAL"));

	if (nIndexCol < 0 || nMinCol < 0 || nMaxCol < 0 || nReturnCol < 0 || nPassCol < 0 || nFailCol < 0)
	{
		TRACE(_T("[RANGE] Required column is missing.\n"));
		return;
	}

	if (m_gridRows[nDataRow].size() < m_columnNames.size())
	{
		m_gridRows[nDataRow].resize(m_columnNames.size());
	}

	WORD indexValue = 0;
	DWORD minValue = 0;
	DWORD maxValue = 0;
	if (!MpTryParseUInt16(m_ctrlGrid.GetItemText(nGridRow, nIndexCol), indexValue) ||
		!MpTryParseUInt32(m_ctrlGrid.GetItemText(nGridRow, nMinCol), minValue) ||
		!MpTryParseUInt32(m_ctrlGrid.GetItemText(nGridRow, nMaxCol), maxValue))
	{
		TRACE(_T("[RANGE] Invalid INDEX/MIN/MAX value. row=%d\n"), nGridRow);
		return;
	}

	DWORD returnValue = 0;
	if (!m_rangeInterface->WriteRangeIndex(indexValue))
	{
		TRACE(_T("[RANGE] Interface command failed. row=%d\n"), nGridRow);
		return;
	}

	if (bUseDelay)
	{
		const DWORD delayMs = GetRangeTestDelayMs(nGridRow);
		TRACE(_T("[RANGE] row=%d delay=%lu ms\n"), nGridRow, delayMs);
		if (!DelayWithMessagePump(delayMs))
		{
			TRACE(_T("[RANGE] stop requested before read. row=%d\n"), nGridRow);
			return;
		}
	}

	if (!m_rangeInterface->ReadRangeValue(minValue, maxValue, returnValue))
	{
		TRACE(_T("[RANGE] Interface read failed. row=%d\n"), nGridRow);
		return;
	}

	const BOOL bPass = (minValue <= returnValue && returnValue <= maxValue);
	const int nPassCount = MpReadCountText(m_ctrlGrid.GetItemText(nGridRow, nPassCol)) + (bPass ? 1 : 0);
	const int nFailCount = MpReadCountText(m_ctrlGrid.GetItemText(nGridRow, nFailCol)) + (bPass ? 0 : 1);
	const int nTotalCount = nPassCount + nFailCount;

	m_gridRows[nDataRow][nReturnCol] = MpFormatDecimal(static_cast<int>(returnValue));
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

	TRACE(_T("[RANGE] row=%d return=0x%08X result=%s pass=%d fail=%d\n"),
		nGridRow, returnValue, bPass ? _T("PASS") : _T("FAIL"), nPassCount, nFailCount);
}

void CRangeTestPage::ResetRangeTestResults()
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

LRESULT CRangeTestPage::OnGridButtonClick(WPARAM wParam, LPARAM lParam)
{
	const int nRow = static_cast<int>(wParam);
	const int nCol = static_cast<int>(lParam);

	CString strMessage;
	strMessage.Format(_T("Grid button clicked. row=%d, col=%d"), nRow, nCol);
	TRACE(_T("%s\n"), strMessage.GetString());
	RunRangeTestRow(nRow, FALSE);

	return 0;
}

void CRangeTestPage::OnSize(UINT nType, int cx, int cy)
{
	CPropertyPage::OnSize(nType, cx, cy);
	MpScaleChildLayout(this, m_initialClientSize, m_childLayouts, cx, cy);
	ResizeGridToClient();
	if (m_ctrlGrid.GetSafeHwnd() != NULL)
	{
		m_ctrlGrid.Refresh();
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

void CRangeTestPage::OnBnClickedBtnRangeValueReadFile()
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
	if (!CExcelXlsxHelper::LoadRangeSheet(dlg.GetPathName(), loadedRows))
	{
		AfxMessageBox(_T("Failed to read the Range Test Excel file."));
		return;
	}

	m_columnNames = CExcelXlsxHelper::BuildRangeColumns();
	m_gridRows.swap(loadedRows);
	RefreshGrid();
}

void CRangeTestPage::OnBnClickedBtnRangeValueWriteFile()
{
	CFileDialog dlg(FALSE, _T("xlsx"), _T("range_test_result.xlsx"),
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

	if (!CExcelXlsxHelper::SaveRangeSheet(strPath, gridRows))
	{
		AfxMessageBox(_T("Failed to save the Range Test Excel file."));
	}
}

void CRangeTestPage::OnBnClickedRngBtRunAll()
{
	if (m_bRunAllRunning)
	{
		m_bRunAllStopRequested = TRUE;
		TRACE(_T("[RANGE] RUN ALL stop requested.\n"));
		return;
	}

	CString strLoopCount;
	GetDlgItemText(IDC_RNG_LOOP, strLoopCount);
	strLoopCount.Trim();

	const int nLoopCount = _ttoi(strLoopCount);
	if (nLoopCount <= 0)
	{
		TRACE(_T("[RANGE] RUN ALL skipped. invalid loop count=%s\n"), strLoopCount.GetString());
		return;
	}

	const int nDataRowCount = static_cast<int>(m_gridRows.size());
	if (nDataRowCount <= 0)
	{
		TRACE(_T("[RANGE] RUN ALL skipped. no range test row.\n"));
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

	TRACE(_T("[RANGE] RUN ALL start. loop=%d, rows=%d, resumeLoop=%d, resumeRow=%d\n"),
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
			RunRangeTestRow(nGridRow, TRUE);

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
	TRACE(_T("[RANGE] RUN ALL %s. loop=%d, rows=%d\n"),
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

void CRangeTestPage::OnBnClickedRngBtReset()
{
	m_bRunAllStopRequested = TRUE;
	m_bRunAllResetRequested = TRUE;
	m_nRunAllResumeLoop = 0;
	m_nRunAllResumeRow = 0;

	ResetRangeTestResults();
	TRACE(_T("[RANGE] RESET. stop requested and all test counts cleared.\n"));
}
