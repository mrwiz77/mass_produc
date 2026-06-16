#include "pch.h"

#include "RangeTestPage.h"
#include "afxdialogex.h"
#include "ExcelXlsxHelper.h"
#include "mass_production_toolDlg.h"
// CRangeTestPage dialog

IMPLEMENT_DYNAMIC(CRangeTestPage, CPropertyPage)

namespace
{
	enum INTEGRATED_GRID_COLUMN
	{
		I_COL_TEST_TYPE = 0,

		I_SYS_TYPE = 1,
		I_SYS_EXPECTED = 2,
		I_SYS_RETURN = 3,
		I_SYS_PASS = 4,
		I_SYS_FAIL = 5,
		I_SYS_TOTAL = 6,
		I_SYS_EXECUTION = 7,

		I_RANGE_INDEX = 1,
		I_RANGE_DESC = 2,
		I_RANGE_MIN = 3,
		I_RANGE_MAX = 4,
		I_RANGE_DELAY = 5,
		I_RANGE_RETURN = 6,
		I_RANGE_PASS = 7,
		I_RANGE_FAIL = 8,
		I_RANGE_TOTAL = 9,
		I_RANGE_EXECUTION = 10,

		I_VALUE_HDR = 1,
		I_VALUE_LENGTH = 2,
		I_VALUE_TYPE = 3,
		I_VALUE_CATEGORY = 4,
		I_VALUE_OPCODE = 5,
		I_VALUE_DATA0 = 6,
		I_VALUE_DELAY = 14,
		I_VALUE_EXPECTED = 15,
		I_VALUE_RETURN = 16,
		I_VALUE_PASS = 17,
		I_VALUE_FAIL = 18,
		I_VALUE_TOTAL = 19,
		I_VALUE_EXECUTION = 20,
	};

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

	std::vector<BYTE> TextToSystemBytes(const CString& text)
	{
		CString strText(text);
		strText.Trim();
		CT2A ansiText(strText, CP_ACP);
		const char* pText = static_cast<LPCSTR>(ansiText);
		std::vector<BYTE> bytes;
		if (pText == NULL)
		{
			return bytes;
		}
		for (int i = 0; pText[i] != '\0'; ++i)
		{
			bytes.push_back(static_cast<BYTE>(pText[i]));
		}
		return bytes;
	}

	CString SystemBytesToText(const std::vector<BYTE>& bytes)
	{
		if (bytes.empty())
		{
			return CString();
		}

		CStringA ansiText;
		for (size_t i = 0; i < bytes.size(); ++i)
		{
			ansiText.AppendChar(static_cast<char>(bytes[i]));
		}
		return CString(CA2T(ansiText, CP_ACP));
	}

	BOOL IsIntegratedUsedColumn(const CString& strTestType, int nCol)
	{
		CString strKey(strTestType);
		strKey.Trim();
		strKey.MakeUpper();
		strKey.Replace(_T(" "), _T("_"));
		if (strKey == _T("SYS_TEST") || strKey == _T("SYSTEM_TEST") || strKey == _T("S"))
		{
			return nCol >= I_COL_TEST_TYPE && nCol <= I_SYS_EXECUTION;
		}
		if (strKey == _T("RANGE_TEST") || strKey == _T("R"))
		{
			return nCol >= I_COL_TEST_TYPE && nCol <= I_RANGE_EXECUTION;
		}
		if (strKey == _T("VALUE_TEST") || strKey == _T("V"))
		{
			return nCol >= I_COL_TEST_TYPE && nCol <= I_VALUE_EXECUTION;
		}
		return FALSE;
	}

	BOOL GetIntegratedPassFailColumns(const CString& strTestType, int& nPassCol, int& nFailCol)
	{
		nPassCol = -1;
		nFailCol = -1;
		CString strKey(strTestType);
		strKey.Trim();
		strKey.MakeUpper();
		strKey.Replace(_T(" "), _T("_"));
		if (strKey == _T("SYS_TEST") || strKey == _T("SYSTEM_TEST") || strKey == _T("S"))
		{
			nPassCol = I_SYS_PASS;
			nFailCol = I_SYS_FAIL;
			return TRUE;
		}
		if (strKey == _T("RANGE_TEST") || strKey == _T("R"))
		{
			nPassCol = I_RANGE_PASS;
			nFailCol = I_RANGE_FAIL;
			return TRUE;
		}
		if (strKey == _T("VALUE_TEST") || strKey == _T("V"))
		{
			nPassCol = I_VALUE_PASS;
			nFailCol = I_VALUE_FAIL;
			return TRUE;
		}
		return FALSE;
	}

	BOOL GetIntegratedResultColumns(const CString& strTestType, int& nPassCol, int& nFailCol, int& nTotalCol)
	{
		nPassCol = -1;
		nFailCol = -1;
		nTotalCol = -1;
		CString strKey(strTestType);
		strKey.Trim();
		strKey.MakeUpper();
		strKey.Replace(_T(" "), _T("_"));
		if (strKey == _T("SYS_TEST") || strKey == _T("SYSTEM_TEST") || strKey == _T("S"))
		{
			nPassCol = I_SYS_PASS;
			nFailCol = I_SYS_FAIL;
			nTotalCol = I_SYS_TOTAL;
			return TRUE;
		}
		if (strKey == _T("RANGE_TEST") || strKey == _T("R"))
		{
			nPassCol = I_RANGE_PASS;
			nFailCol = I_RANGE_FAIL;
			nTotalCol = I_RANGE_TOTAL;
			return TRUE;
		}
		if (strKey == _T("VALUE_TEST") || strKey == _T("V"))
		{
			nPassCol = I_VALUE_PASS;
			nFailCol = I_VALUE_FAIL;
			nTotalCol = I_VALUE_TOTAL;
			return TRUE;
		}
		return FALSE;
	}

	CString GetIntegratedTypeMarker(const CString& strTestType)
	{
		CString strUpper(strTestType);
		strUpper.Trim();
		strUpper.MakeUpper();
		strUpper.Replace(_T(" "), _T("_"));
		if (strUpper == _T("SYS_TEST") || strUpper == _T("SYSTEM_TEST"))
		{
			return _T("SYS TEST");
		}
		if (strUpper == _T("S"))
		{
			return _T("SYS TEST");
		}
		if (strUpper == _T("RANGE_TEST"))
		{
			return _T("RANGE TEST");
		}
		if (strUpper == _T("R"))
		{
			return _T("RANGE TEST");
		}
		if (strUpper == _T("VALUE_TEST"))
		{
			return _T("VALUE TEST");
		}
		if (strUpper == _T("V"))
		{
			return _T("VALUE TEST");
		}
		return strTestType;
	}

	CString GetIntegratedHeaderTestType(int nHeaderRow)
	{
		if (nHeaderRow == 0)
		{
			return _T("SYS_TEST");
		}
		if (nHeaderRow == 1)
		{
			return _T("RANGE_TEST");
		}
		if (nHeaderRow == 2)
		{
			return _T("VALUE_TEST");
		}
		return CString();
	}

}

CRangeTestPage::CRangeTestPage()
	: CPropertyPage(IDD_PROPPAGE_RANGE_TEST)
	, m_backgroundColor(GetSysColor(COLOR_3DFACE))
	, m_resourceGridSize(0, 0)
	, m_rangeInterface(new CRangeTestSimulInterface())
	, m_valueInterface(new CValueTestSimulInterface())
	, m_systemInterface(new CSystemTestSimulInterface())
	, m_bRunAllRunning(FALSE)
	, m_bRunAllStopRequested(FALSE)
	, m_bRunAllResetRequested(FALSE)
	, m_nRunAllResumeLoop(0)
	, m_nRunAllResumeRow(0)
{
	m_psp.dwFlags |= PSP_USETITLE;
	m_psp.pszTitle = _T("                   WHOLE SYSTEM TEST                  ");
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

BOOL CRangeTestPage::LoadRangeConfigFile(const CString& strFilePath)
{
	std::vector<std::vector<CString>> loadedRows;
	std::vector<std::vector<CString>> integratedRows;
	BOOL bIntegratedSheet = FALSE;
	if (CExcelXlsxHelper::LoadIntegratedTestSheet(strFilePath, 8, integratedRows))
	{
		for (size_t nRow = 0; nRow < integratedRows.size(); ++nRow)
		{
			if (!integratedRows[nRow].empty())
			{
				CString strTestType = integratedRows[nRow][0];
				strTestType.Trim();
				if (!strTestType.IsEmpty())
				{
					bIntegratedSheet = TRUE;
					break;
				}
			}
		}
	}

	if (bIntegratedSheet)
	{
		m_columnNames = CExcelXlsxHelper::BuildIntegratedTestColumns(8);
		m_gridRows.swap(integratedRows);
		RefreshGrid();
		TRACE(_T("[RANGE] Loaded integrated config sheet. file=%s, rows=%d\n"),
			strFilePath.GetString(),
			static_cast<int>(m_gridRows.size()));
		return TRUE;
	}

	if (!CExcelXlsxHelper::LoadRangeSheet(strFilePath, loadedRows))
	{
		TRACE(_T("[RANGE] Failed to read config file. file=%s\n"), strFilePath.GetString());
		return FALSE;
	}

	m_columnNames = CExcelXlsxHelper::BuildRangeColumns();
	m_gridRows.swap(loadedRows);
	RefreshGrid();
	TRACE(_T("[RANGE] Loaded range config sheet. file=%s, rows=%d\n"),
		strFilePath.GetString(),
		static_cast<int>(m_gridRows.size()));
	return TRUE;
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
	CheckDlgButton(IDC_CK_SUCCESS, BST_CHECKED);
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
		m_ctrlGrid.ModifyStyleEx(WS_EX_CLIENTEDGE | WS_EX_STATICEDGE, 0, SWP_FRAMECHANGED);
	}
	ResizeGridToClient();

	CFont* pDialogFont = GetFont();
	if (pDialogFont != NULL)
	{
		m_ctrlGrid.SetFont(pDialogFont);
	}

	if (m_columnNames.empty())
	{
		m_columnNames = CExcelXlsxHelper::BuildIntegratedTestColumns(8);
	}

	const int nColumnCount = static_cast<int>(m_columnNames.size());
	if (nColumnCount <= 0)
	{
		return;
	}

	const int nButtonCol = nColumnCount - 1;

	m_ctrlGrid.SetEditable(FALSE);
	m_ctrlGrid.EnableDragAndDrop(FALSE);
	m_ctrlGrid.SetTextBkColor(RGB(255, 255, 255));
	m_ctrlGrid.SetTextColor(RGB(0, 0, 0));
	m_ctrlGrid.SetBkColor(m_backgroundColor);
	m_ctrlGrid.SetGridColor(RGB(0, 0, 0));
	m_ctrlGrid.SetColumnCount(nColumnCount);
	m_ctrlGrid.SetRowCount(20);
	m_ctrlGrid.SetFixedRowCount(1);
	m_ctrlGrid.SetFixedBkColor(RGB(181, 181, 181));
	m_ctrlGrid.SetFixedTextColor(RGB(255, 255, 255));
	
	for (int nCol = 0; nCol < m_ctrlGrid.GetColumnCount(); nCol++)
	{
		m_ctrlGrid.SetColumnWidth(nCol, MulDiv(130, 110, 100));
	}
	m_ctrlGrid.SetColumnWidth(nButtonCol, MulDiv(100, 110, 100));
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

	const int gridRightMargin = max(GetSystemMetrics(SM_CXVSCROLL) + 2, rectClient.Width() / 10);
	const int currentGridWidth = max(10, rectClient.Width() - MP_GRID_CLIENT_X - gridRightMargin);
	const int maxGridWidth = max(10, rectClient.Width() - MP_GRID_CLIENT_X - GetSystemMetrics(SM_CXVSCROLL) - 2);
	const int gridWidth = min(maxGridWidth, MulDiv(currentGridWidth, 115, 100));
	const int gridHeight = max(10, rectClient.Height() - MP_GRID_CLIENT_Y - GetSystemMetrics(SM_CYHSCROLL) - 2);
	if (gridWidth > 0 && gridHeight > 0)
	{
		m_ctrlGrid.MoveWindow(MP_GRID_CLIENT_X, MP_GRID_CLIENT_Y, gridWidth, gridHeight);
	}
}

void CRangeTestPage::UpdateGridLayout()
{
	if (m_ctrlGrid.GetSafeHwnd() == NULL)
	{
		return;
	}

	ResizeGridToClient();
	m_ctrlGrid.ShowScrollBar(SB_BOTH, TRUE);
	m_ctrlGrid.Invalidate(FALSE);
	m_ctrlGrid.UpdateWindow();
}


void CRangeTestPage::RefreshGrid()
{
	const int nColumnCount = static_cast<int>(m_columnNames.size());
	if (nColumnCount <= 0 || m_ctrlGrid.GetSafeHwnd() == NULL)
	{
		return;
	}
	ResizeGridToClient();

	const BOOL bIntegratedGrid = IsIntegratedGrid();
	const int nHeaderRowCount = GetHeaderRowCount();
	const int nButtonCol = nColumnCount - 1;
	const int nRowCount = bIntegratedGrid ?
		max(static_cast<int>(m_gridRows.size()) + nHeaderRowCount, 30) :
		static_cast<int>(m_gridRows.size()) + nHeaderRowCount;

	m_ctrlGrid.SetRedraw(FALSE);
	m_ctrlGrid.SetColumnCount(nColumnCount);
	m_ctrlGrid.SetRowCount(nRowCount);
	m_ctrlGrid.SetFixedRowCount(nHeaderRowCount);

	for (int nCol = 0; nCol < m_ctrlGrid.GetColumnCount(); nCol++)
	{
		m_ctrlGrid.SetColumnWidth(nCol, MulDiv(130, 110, 100));
	}
	if (!bIntegratedGrid)
	{
		m_ctrlGrid.SetColumnWidth(nButtonCol, MulDiv(100, 110, 100));
	}

	std::vector<std::vector<CString>> integratedHeaderRows;
	if (bIntegratedGrid)
	{
		integratedHeaderRows.resize(3);
		for (int nRow = 0; nRow < 3; ++nRow)
		{
			integratedHeaderRows[nRow].resize(nColumnCount);
		}
		integratedHeaderRows[0][I_COL_TEST_TYPE] = _T("SYS TEST");
		integratedHeaderRows[0][I_SYS_TYPE] = _T("TYPE");
		integratedHeaderRows[0][I_SYS_EXPECTED] = _T("EXPECTED");
		integratedHeaderRows[0][I_SYS_RETURN] = _T("RETURN");
		integratedHeaderRows[0][I_SYS_PASS] = _T("PASS");
		integratedHeaderRows[0][I_SYS_FAIL] = _T("FAIL");
		integratedHeaderRows[0][I_SYS_TOTAL] = _T("TOTAL");
		integratedHeaderRows[0][I_SYS_EXECUTION] = _T("EXCUTION");

		integratedHeaderRows[1][I_COL_TEST_TYPE] = _T("RANGE TEST");
		integratedHeaderRows[1][I_RANGE_INDEX] = _T("INDEX");
		integratedHeaderRows[1][I_RANGE_DESC] = _T("Description");
		integratedHeaderRows[1][I_RANGE_MIN] = _T("MIN");
		integratedHeaderRows[1][I_RANGE_MAX] = _T("MAX");
		integratedHeaderRows[1][I_RANGE_DELAY] = _T("DELAY ms");
		integratedHeaderRows[1][I_RANGE_RETURN] = _T("RETURN");
		integratedHeaderRows[1][I_RANGE_PASS] = _T("PASS");
		integratedHeaderRows[1][I_RANGE_FAIL] = _T("FAIL");
		integratedHeaderRows[1][I_RANGE_TOTAL] = _T("TOTAL");
		integratedHeaderRows[1][I_RANGE_EXECUTION] = _T("EXCUTION");

		integratedHeaderRows[2][I_COL_TEST_TYPE] = _T("VALUE TEST");
		integratedHeaderRows[2][I_VALUE_HDR] = _T("HDR");
		integratedHeaderRows[2][I_VALUE_LENGTH] = _T("LENGTH");
		integratedHeaderRows[2][I_VALUE_TYPE] = _T("TYPE");
		integratedHeaderRows[2][I_VALUE_CATEGORY] = _T("CATEGORY");
		integratedHeaderRows[2][I_VALUE_OPCODE] = _T("OPCODE");
		for (int nData = 0; nData < 8 && I_VALUE_DATA0 + nData < nColumnCount; ++nData)
		{
			integratedHeaderRows[2][I_VALUE_DATA0 + nData].Format(_T("D[%d]"), nData);
		}
		integratedHeaderRows[2][I_VALUE_DELAY] = _T("DELAY ms");
		integratedHeaderRows[2][I_VALUE_EXPECTED] = _T("EXPECTED");
		integratedHeaderRows[2][I_VALUE_RETURN] = _T("RETURN");
		integratedHeaderRows[2][I_VALUE_PASS] = _T("PASS");
		integratedHeaderRows[2][I_VALUE_FAIL] = _T("FAIL");
		integratedHeaderRows[2][I_VALUE_TOTAL] = _T("TOTAL");
		integratedHeaderRows[2][I_VALUE_EXECUTION] = _T("EXCUTION");

		CClientDC dc(&m_ctrlGrid);
		CFont* pGridFont = m_ctrlGrid.GetFont();
		CFont* pOldFont = pGridFont != NULL ? dc.SelectObject(pGridFont) : NULL;
		for (int nCol = 0; nCol < m_ctrlGrid.GetColumnCount(); ++nCol)
		{
			int nWidth = nCol == I_COL_TEST_TYPE ? 24 : 18;
			for (int nRow = 0; nRow < nHeaderRowCount && nRow < static_cast<int>(integratedHeaderRows.size()); ++nRow)
			{
				if (nCol >= static_cast<int>(integratedHeaderRows[nRow].size()))
				{
					continue;
				}

				CString strText = integratedHeaderRows[nRow][nCol];
				strText.Trim();
				if (strText.IsEmpty())
				{
					continue;
				}

				CSize textSize = dc.GetTextExtent(strText);
				nWidth = max(nWidth, textSize.cx + 18);
			}
			m_ctrlGrid.SetColumnWidth(nCol, MulDiv(nWidth, 110, 100));
		}
		if (pOldFont != NULL)
		{
			dc.SelectObject(pOldFont);
		}
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

			if (bIntegratedGrid && nRow < nHeaderRowCount)
			{
				item.strText = integratedHeaderRows[nRow][nCol];
			}
			else if (!bIntegratedGrid && nRow == 0)
			{
				item.strText = m_columnNames[nCol];
			}
			else
			{
				const int nDataRow = GridRowToDataRow(nRow);
				if (nDataRow >= 0 && nDataRow < static_cast<int>(m_gridRows.size()) &&
					nCol < static_cast<int>(m_gridRows[nDataRow].size()))
				{
					item.strText = m_gridRows[nDataRow][nCol];
					if (bIntegratedGrid && nCol == I_COL_TEST_TYPE)
					{
						item.strText = GetIntegratedTypeMarker(item.strText);
					}
				}

				if (!bIntegratedGrid && nCol == nButtonCol)
				{
					item.strText = _T("RUN");
				}
				else if (bIntegratedGrid)
				{
					CString strTestType = item.strText;
					if (nCol != I_COL_TEST_TYPE && nDataRow >= 0 && nDataRow < static_cast<int>(m_gridRows.size()) &&
						I_COL_TEST_TYPE < static_cast<int>(m_gridRows[nDataRow].size()))
					{
						strTestType = m_gridRows[nDataRow][I_COL_TEST_TYPE];
					}
					strTestType.Trim();
					strTestType.MakeUpper();
					if (((strTestType == _T("SYS_TEST") || strTestType == _T("SYSTEM_TEST")) && nCol == I_SYS_EXECUTION) ||
						(strTestType == _T("RANGE_TEST") && nCol == I_RANGE_EXECUTION) ||
						(strTestType == _T("VALUE_TEST") && nCol == I_VALUE_EXECUTION))
					{
						item.strText = _T("RUN");
					}
				}
			}

			m_ctrlGrid.SetItem(&item);
		}
	}

	for (int nRow = nHeaderRowCount; nRow < m_ctrlGrid.GetRowCount(); nRow++)
	{
		if (bIntegratedGrid)
		{
			const CString strTestType = GetIntegratedTestType(nRow);
			if (strTestType == _T("SYS_TEST") || strTestType == _T("SYSTEM_TEST"))
			{
				m_ctrlGrid.SetCellType(nRow, I_SYS_EXECUTION, RUNTIME_CLASS(CGridCellButton));
				m_ctrlGrid.SetItemText(nRow, I_SYS_EXECUTION, _T("RUN"));
			}
			else if (strTestType == _T("RANGE_TEST"))
			{
				m_ctrlGrid.SetCellType(nRow, I_RANGE_EXECUTION, RUNTIME_CLASS(CGridCellButton));
				m_ctrlGrid.SetItemText(nRow, I_RANGE_EXECUTION, _T("RUN"));
			}
			else if (strTestType == _T("VALUE_TEST"))
			{
				m_ctrlGrid.SetCellType(nRow, I_VALUE_EXECUTION, RUNTIME_CLASS(CGridCellButton));
				m_ctrlGrid.SetItemText(nRow, I_VALUE_EXECUTION, _T("RUN"));
			}
		}
		else
		{
			m_ctrlGrid.SetCellType(nRow, nButtonCol, RUNTIME_CLASS(CGridCellButton));
			m_ctrlGrid.SetItemText(nRow, nButtonCol, _T("RUN"));
		}
	}

	m_ctrlGrid.SetRedraw(TRUE, TRUE);
	m_ctrlGrid.ShowScrollBar(SB_BOTH, TRUE);
	if (bIntegratedGrid)
	{
		for (int nRow = 0; nRow < m_ctrlGrid.GetRowCount(); ++nRow)
		{
			CString strTestType = nRow < nHeaderRowCount ? GetIntegratedHeaderTestType(nRow) : GetIntegratedTestType(nRow);

			MP_GRID_CELL_COLOR_THEME headerTheme = MP_GRID_COLOR_DARK_GRAY;
			MP_GRID_CELL_COLOR_THEME contentTheme = MP_GRID_COLOR_LIGHT_GRAY;
			if (strTestType == _T("RANGE_TEST"))
			{
				headerTheme = MP_GRID_COLOR_DARK_GREEN;
				contentTheme = MP_GRID_COLOR_LIGHT_GREEN;
			}
			else if (strTestType == _T("VALUE_TEST"))
			{
				headerTheme = MP_GRID_COLOR_DARK_YELLOW;
				contentTheme = MP_GRID_COLOR_LIGHT_YELLOW;
			}

			for (int nCol = 0; nCol < m_ctrlGrid.GetColumnCount(); ++nCol)
			{
				if (!IsIntegratedUsedColumn(strTestType, nCol))
				{
					continue;
				}
				const MP_GRID_CELL_COLOR_THEME theme = (nRow < nHeaderRowCount || nCol == I_COL_TEST_TYPE) ? headerTheme : contentTheme;
				m_ctrlGrid.SetItemBkColour(nRow, nCol, MpGridThemeBkColor(theme));
				m_ctrlGrid.SetItemFgColour(nRow, nCol, MpGridThemeTextColor(theme));
			}
		}
	}
	else
	{
		m_ctrlGrid.SetItemBkColour(0, 0, MpGridThemeBkColor(MP_GRID_COLOR_DARK_ORANGE));
		m_ctrlGrid.SetItemFgColour(0, 0, MpGridThemeTextColor(MP_GRID_COLOR_DARK_ORANGE));
	}
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

	const COLORREF passTextColor = MpGridThemeBkColor(MP_GRID_COLOR_DARK_BLUE);
	const COLORREF failTextColor = MpGridThemeBkColor(MP_GRID_COLOR_DARK_RED);

	if (IsIntegratedGrid())
	{
		for (int nRow = 0; nRow < m_ctrlGrid.GetRowCount(); ++nRow)
		{
			CString strTestType = nRow < GetHeaderRowCount() ? GetIntegratedHeaderTestType(nRow) : GetIntegratedTestType(nRow);
			strTestType.Trim();
			strTestType.MakeUpper();

			int nPassCol = -1;
			int nFailCol = -1;
			if (!GetIntegratedPassFailColumns(strTestType, nPassCol, nFailCol))
			{
				continue;
			}

			if (nPassCol >= 0 && nPassCol < m_ctrlGrid.GetColumnCount())
			{
				m_ctrlGrid.SetItemFgColour(nRow, nPassCol, passTextColor);
			}
			if (nFailCol >= 0 && nFailCol < m_ctrlGrid.GetColumnCount())
			{
				m_ctrlGrid.SetItemFgColour(nRow, nFailCol, failTextColor);
			}
		}
		return;
	}

	const int nPassCol = FindColumnIndex(_T("PASS"));
	const int nFailCol = FindColumnIndex(_T("FAIL"));

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

BOOL CRangeTestPage::IsIntegratedGrid() const
{
	return FindColumnIndex(_T("TEST_TYPE")) >= 0;
}

int CRangeTestPage::GetHeaderRowCount() const
{
	return IsIntegratedGrid() ? 3 : 1;
}

int CRangeTestPage::GridRowToDataRow(int nGridRow) const
{
	return nGridRow - GetHeaderRowCount();
}

CString CRangeTestPage::GetIntegratedTestType(int nGridRow) const
{
	CString strTestType;
	const int nDataRow = GridRowToDataRow(nGridRow);
	if (nDataRow >= 0 && nDataRow < static_cast<int>(m_gridRows.size()) &&
		I_COL_TEST_TYPE < static_cast<int>(m_gridRows[nDataRow].size()))
	{
		strTestType = m_gridRows[nDataRow][I_COL_TEST_TYPE];
	}
	else if (m_ctrlGrid.GetSafeHwnd() != NULL && nGridRow >= 0 && nGridRow < m_ctrlGrid.GetRowCount())
	{
		strTestType = m_ctrlGrid.GetItemText(nGridRow, I_COL_TEST_TYPE);
	}

	strTestType.Trim();
	strTestType.MakeUpper();
	strTestType.Replace(_T(" "), _T("_"));
	if (strTestType == _T("S"))
	{
		return _T("SYS_TEST");
	}
	if (strTestType == _T("R"))
	{
		return _T("RANGE_TEST");
	}
	if (strTestType == _T("V"))
	{
		return _T("VALUE_TEST");
	}
	return strTestType;
}

DWORD CRangeTestPage::GetRangeTestDelayMs(int nGridRow) const
{
	if (nGridRow < GetHeaderRowCount() || m_ctrlGrid.GetSafeHwnd() == NULL)
	{
		return 0;
	}

	int nDelayCol = FindColumnIndex(_T("DELAY ms"));
	if (IsIntegratedGrid())
	{
		const CString strTestType = GetIntegratedTestType(nGridRow);
		if (strTestType == _T("RANGE_TEST"))
		{
			nDelayCol = I_RANGE_DELAY;
		}
		else if (strTestType == _T("VALUE_TEST"))
		{
			nDelayCol = I_VALUE_DELAY;
		}
		else
		{
			return 0;
		}
	}

	if (nDelayCol < 0 || nDelayCol >= m_ctrlGrid.GetColumnCount())
	{
		return 0;
	}

	DWORD delayMs = 0;
	MpTryParseUInt32(m_ctrlGrid.GetItemText(nGridRow, nDelayCol), delayMs);
	return delayMs;
}

BOOL CRangeTestPage::IsSuccessOnlyEnabled() const
{
	CWnd* pCheck = GetDlgItem(IDC_CK_SUCCESS);
	if (pCheck == NULL || pCheck->GetSafeHwnd() == NULL)
	{
		return FALSE;
	}
	return IsDlgButtonChecked(IDC_CK_SUCCESS) == BST_CHECKED;
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
	if (nGridRow < GetHeaderRowCount() || m_rangeInterface.get() == NULL)
	{
		return;
	}

	if (IsIntegratedGrid())
	{
		const CString strTestType = GetIntegratedTestType(nGridRow);
		if (strTestType == _T("RANGE_TEST"))
		{
			RunIntegratedRangeTestRow(nGridRow, bUseDelay);
			return;
		}
		if (strTestType == _T("VALUE_TEST"))
		{
			RunIntegratedValueTestRow(nGridRow, bUseDelay);
			return;
		}
		if (strTestType == _T("SYS_TEST") || strTestType == _T("SYSTEM_TEST"))
		{
			RunIntegratedSystemTestRow(nGridRow, bUseDelay);
			return;
		}
	}

	const int nDataRow = GridRowToDataRow(nGridRow);
	if (nDataRow < 0 || nDataRow >= static_cast<int>(m_gridRows.size()))
	{
		return;
	}

	const int nIndexCol = FindColumnIndex(_T("INDEX"));
	const int nMinCol = FindColumnIndex(_T("MIN"));
	const int nMaxCol = FindColumnIndex(_T("MAX"));
	int nReturnCol = FindColumnIndex(_T("RETURN"));
	if (nReturnCol < 0)
	{
		nReturnCol = FindColumnIndex(_T("RETURN_VALUE"));
	}
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
	if (IsSuccessOnlyEnabled())
	{
		returnValue = minValue;
		TRACE(_T("[RANGE] Success-only checked. force PASS. row=%d\n"), nGridRow);
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
	UpdateOverallTestStatus();

	TRACE(_T("[RANGE] row=%d return=0x%08X result=%s pass=%d fail=%d\n"),
		nGridRow, returnValue, bPass ? _T("PASS") : _T("FAIL"), nPassCount, nFailCount);
}

void CRangeTestPage::RunIntegratedRangeTestRow(int nGridRow, BOOL bUseDelay)
{
	const int nIndexCol = I_RANGE_INDEX;
	const int nMinCol = I_RANGE_MIN;
	const int nMaxCol = I_RANGE_MAX;
	const int nReturnCol = I_RANGE_RETURN;
	const int nPassCol = I_RANGE_PASS;
	const int nFailCol = I_RANGE_FAIL;
	const int nTotalCol = I_RANGE_TOTAL;
	if (nTotalCol >= m_ctrlGrid.GetColumnCount())
	{
		TRACE(_T("[INTEGRATED][RANGE] Required column is missing.\n"));
		return;
	}

	WORD indexValue = 0;
	DWORD minValue = 0;
	DWORD maxValue = 0;
	if (!MpTryParseUInt16(m_ctrlGrid.GetItemText(nGridRow, nIndexCol), indexValue) ||
		!MpTryParseUInt32(m_ctrlGrid.GetItemText(nGridRow, nMinCol), minValue) ||
		!MpTryParseUInt32(m_ctrlGrid.GetItemText(nGridRow, nMaxCol), maxValue))
	{
		TRACE(_T("[INTEGRATED][RANGE] Invalid INDEX/MIN/MAX. row=%d\n"), nGridRow);
		return;
	}

	if (!m_rangeInterface->WriteRangeIndex(indexValue))
	{
		TRACE(_T("[INTEGRATED][RANGE] Interface command failed. row=%d\n"), nGridRow);
		return;
	}

	if (bUseDelay && !DelayWithMessagePump(GetRangeTestDelayMs(nGridRow)))
	{
		TRACE(_T("[INTEGRATED][RANGE] stop requested before read. row=%d\n"), nGridRow);
		return;
	}

	DWORD returnValue = 0;
	if (!m_rangeInterface->ReadRangeValue(minValue, maxValue, returnValue))
	{
		TRACE(_T("[INTEGRATED][RANGE] Interface read failed. row=%d\n"), nGridRow);
		return;
	}
	if (IsSuccessOnlyEnabled())
	{
		returnValue = minValue;
		TRACE(_T("[INTEGRATED][RANGE] Success-only checked. force PASS. row=%d\n"), nGridRow);
	}

	const BOOL bPass = (minValue <= returnValue && returnValue <= maxValue);
	const int nPassCount = MpReadCountText(m_ctrlGrid.GetItemText(nGridRow, nPassCol)) + (bPass ? 1 : 0);
	const int nFailCount = MpReadCountText(m_ctrlGrid.GetItemText(nGridRow, nFailCol)) + (bPass ? 0 : 1);
	const int nTotalCount = nPassCount + nFailCount;
	const int nDataRow = GridRowToDataRow(nGridRow);
	if (nDataRow >= 0 && nDataRow < static_cast<int>(m_gridRows.size()))
	{
		if (m_gridRows[nDataRow].size() < m_columnNames.size())
		{
			m_gridRows[nDataRow].resize(m_columnNames.size());
		}
		m_gridRows[nDataRow][nReturnCol] = MpFormatDecimal(static_cast<int>(returnValue));
		m_gridRows[nDataRow][nPassCol] = MpFormatDecimal(nPassCount);
		m_gridRows[nDataRow][nFailCol] = MpFormatDecimal(nFailCount);
		if (nTotalCol >= 0)
		{
			m_gridRows[nDataRow][nTotalCol] = MpFormatDecimal(nTotalCount);
		}
	}

	m_ctrlGrid.SetItemText(nGridRow, nReturnCol, MpFormatDecimal(static_cast<int>(returnValue)));
	m_ctrlGrid.SetItemText(nGridRow, nPassCol, MpFormatDecimal(nPassCount));
	m_ctrlGrid.SetItemText(nGridRow, nFailCol, MpFormatDecimal(nFailCount));
	if (nTotalCol >= 0)
	{
		m_ctrlGrid.SetItemText(nGridRow, nTotalCol, MpFormatDecimal(nTotalCount));
	}
	ApplyCountColumnTextColors();
	m_ctrlGrid.Refresh();
	UpdateOverallTestStatus();
}

void CRangeTestPage::RunIntegratedValueTestRow(int nGridRow, BOOL bUseDelay)
{
	if (m_valueInterface.get() == NULL)
	{
		return;
	}

	const int nHdrCol = I_VALUE_HDR;
	const int nLengthCol = I_VALUE_LENGTH;
	const int nTypeCol = I_VALUE_TYPE;
	const int nCategoryCol = I_VALUE_CATEGORY;
	const int nOpcodeCol = I_VALUE_OPCODE;
	const int nExpectedCol = I_VALUE_EXPECTED;
	const int nReturnCol = I_VALUE_RETURN;
	const int nPassCol = I_VALUE_PASS;
	const int nFailCol = I_VALUE_FAIL;
	const int nTotalCol = I_VALUE_TOTAL;
	if (nTotalCol >= m_ctrlGrid.GetColumnCount())
	{
		TRACE(_T("[INTEGRATED][VALUE] Required column is missing.\n"));
		return;
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
		TRACE(_T("[INTEGRATED][VALUE] Invalid command or EXPECTED. row=%d\n"), nGridRow);
		return;
	}

	std::vector<BYTE> commandBytes;
	commandBytes.push_back(hdr);
	commandBytes.push_back(length);
	commandBytes.push_back(type);
	commandBytes.push_back(category);
	commandBytes.push_back(opcode);
	for (int nDataIndex = 0; nDataIndex < static_cast<int>(length) && nDataIndex < 8; ++nDataIndex)
	{
		const int nDataCol = I_VALUE_DATA0 + nDataIndex;
		BYTE dataValue = 0;
		if (nDataCol < m_ctrlGrid.GetColumnCount() && MpTryParseByte(m_ctrlGrid.GetItemText(nGridRow, nDataCol), dataValue))
		{
			commandBytes.push_back(dataValue);
		}
		else
		{
			commandBytes.push_back(0);
		}
	}

	if (!m_valueInterface->WriteValueCommand(commandBytes))
	{
		TRACE(_T("[INTEGRATED][VALUE] Interface command failed. row=%d\n"), nGridRow);
		return;
	}
	if (bUseDelay && !DelayWithMessagePump(GetRangeTestDelayMs(nGridRow)))
	{
		TRACE(_T("[INTEGRATED][VALUE] stop requested before read. row=%d\n"), nGridRow);
		return;
	}

	DWORD returnValue = 0;
	if (!m_valueInterface->ReadValue(expectedValue, returnValue))
	{
		TRACE(_T("[INTEGRATED][VALUE] Interface read failed. row=%d\n"), nGridRow);
		return;
	}
	if (IsSuccessOnlyEnabled())
	{
		returnValue = expectedValue;
		TRACE(_T("[INTEGRATED][VALUE] Success-only checked. force PASS. row=%d\n"), nGridRow);
	}

	const BOOL bPass = (returnValue == expectedValue);
	const int nPassCount = MpReadCountText(m_ctrlGrid.GetItemText(nGridRow, nPassCol)) + (bPass ? 1 : 0);
	const int nFailCount = MpReadCountText(m_ctrlGrid.GetItemText(nGridRow, nFailCol)) + (bPass ? 0 : 1);
	const int nTotalCount = nPassCount + nFailCount;
	const CString strReturn = MpFormatHex32(returnValue);
	const int nDataRow = GridRowToDataRow(nGridRow);
	if (nDataRow >= 0 && nDataRow < static_cast<int>(m_gridRows.size()))
	{
		if (m_gridRows[nDataRow].size() < m_columnNames.size())
		{
			m_gridRows[nDataRow].resize(m_columnNames.size());
		}
		m_gridRows[nDataRow][nReturnCol] = strReturn;
		m_gridRows[nDataRow][nPassCol] = MpFormatDecimal(nPassCount);
		m_gridRows[nDataRow][nFailCol] = MpFormatDecimal(nFailCount);
		if (nTotalCol >= 0)
		{
			m_gridRows[nDataRow][nTotalCol] = MpFormatDecimal(nTotalCount);
		}
	}

	m_ctrlGrid.SetItemText(nGridRow, nReturnCol, strReturn);
	m_ctrlGrid.SetItemText(nGridRow, nPassCol, MpFormatDecimal(nPassCount));
	m_ctrlGrid.SetItemText(nGridRow, nFailCol, MpFormatDecimal(nFailCount));
	if (nTotalCol >= 0)
	{
		m_ctrlGrid.SetItemText(nGridRow, nTotalCol, MpFormatDecimal(nTotalCount));
	}
	ApplyCountColumnTextColors();
	m_ctrlGrid.Refresh();
	UpdateOverallTestStatus();
}

void CRangeTestPage::RunIntegratedSystemTestRow(int nGridRow, BOOL bUseDelay)
{
	if (m_systemInterface.get() == NULL)
	{
		return;
	}

	const int nTypeCol = I_SYS_TYPE;
	const int nExpectedCol = I_SYS_EXPECTED;
	const int nReturnCol = I_SYS_RETURN;
	const int nPassCol = I_SYS_PASS;
	const int nFailCol = I_SYS_FAIL;
	const int nTotalCol = I_SYS_TOTAL;
	if (nTotalCol >= m_ctrlGrid.GetColumnCount())
	{
		TRACE(_T("[INTEGRATED][SYSTEM] Required column is missing.\n"));
		return;
	}

	BYTE typeValue = 0;
	CString strType = m_ctrlGrid.GetItemText(nGridRow, nTypeCol);
	if (!MpTryGetSystemTestType(strType, typeValue) && !MpTryParseByte(strType, typeValue))
	{
		TRACE(_T("[INTEGRATED][SYSTEM] Unknown TYPE. row=%d, type=%s\n"), nGridRow, strType.GetString());
		return;
	}

	const CString strExpected = m_ctrlGrid.GetItemText(nGridRow, nExpectedCol);
	const std::vector<BYTE> expectedBytes = TextToSystemBytes(strExpected);
	if (expectedBytes.empty())
	{
		TRACE(_T("[INTEGRATED][SYSTEM] Invalid EXPECTED. row=%d\n"), nGridRow);
		return;
	}

	if (!m_systemInterface->WriteSystemRequest(typeValue))
	{
		TRACE(_T("[INTEGRATED][SYSTEM] Interface command failed. row=%d\n"), nGridRow);
		return;
	}
	if (bUseDelay && !DelayWithMessagePump(GetRangeTestDelayMs(nGridRow)))
	{
		TRACE(_T("[INTEGRATED][SYSTEM] stop requested before read. row=%d\n"), nGridRow);
		return;
	}

	std::vector<BYTE> returnBytes;
	if (!m_systemInterface->ReadSystemBytes(expectedBytes, returnBytes))
	{
		TRACE(_T("[INTEGRATED][SYSTEM] Interface read failed. row=%d\n"), nGridRow);
		return;
	}
	if (IsSuccessOnlyEnabled())
	{
		returnBytes = expectedBytes;
		TRACE(_T("[INTEGRATED][SYSTEM] Success-only checked. force PASS. row=%d\n"), nGridRow);
	}

	const BOOL bPass = (returnBytes == expectedBytes);
	const int nPassCount = MpReadCountText(m_ctrlGrid.GetItemText(nGridRow, nPassCol)) + (bPass ? 1 : 0);
	const int nFailCount = MpReadCountText(m_ctrlGrid.GetItemText(nGridRow, nFailCol)) + (bPass ? 0 : 1);
	const int nTotalCount = nPassCount + nFailCount;
	const CString strReturn = SystemBytesToText(returnBytes);
	const int nDataRow = GridRowToDataRow(nGridRow);
	if (nDataRow >= 0 && nDataRow < static_cast<int>(m_gridRows.size()))
	{
		if (m_gridRows[nDataRow].size() < m_columnNames.size())
		{
			m_gridRows[nDataRow].resize(m_columnNames.size());
		}
		m_gridRows[nDataRow][nReturnCol] = strReturn;
		m_gridRows[nDataRow][nPassCol] = MpFormatDecimal(nPassCount);
		m_gridRows[nDataRow][nFailCol] = MpFormatDecimal(nFailCount);
		m_gridRows[nDataRow][nTotalCol] = MpFormatDecimal(nTotalCount);
	}

	m_ctrlGrid.SetItemText(nGridRow, nReturnCol, strReturn);
	m_ctrlGrid.SetItemText(nGridRow, nPassCol, MpFormatDecimal(nPassCount));
	m_ctrlGrid.SetItemText(nGridRow, nFailCol, MpFormatDecimal(nFailCount));
	m_ctrlGrid.SetItemText(nGridRow, nTotalCol, MpFormatDecimal(nTotalCount));
	ApplyCountColumnTextColors();
	m_ctrlGrid.Refresh();
	UpdateOverallTestStatus();
}

void CRangeTestPage::ResetRangeTestResults()
{
	if (IsIntegratedGrid())
	{
		for (size_t nDataRow = 0; nDataRow < m_gridRows.size(); ++nDataRow)
		{
			if (m_gridRows[nDataRow].size() < m_columnNames.size())
			{
				m_gridRows[nDataRow].resize(m_columnNames.size());
			}

			CString strTestType = m_gridRows[nDataRow][I_COL_TEST_TYPE];
			strTestType.Trim();
			strTestType.MakeUpper();
			if (strTestType == _T("RANGE_TEST"))
			{
				m_gridRows[nDataRow][I_RANGE_RETURN].Empty();
				m_gridRows[nDataRow][I_RANGE_PASS].Empty();
				m_gridRows[nDataRow][I_RANGE_FAIL].Empty();
				m_gridRows[nDataRow][I_RANGE_TOTAL].Empty();
			}
			else if (strTestType == _T("VALUE_TEST"))
			{
				m_gridRows[nDataRow][I_VALUE_RETURN].Empty();
				m_gridRows[nDataRow][I_VALUE_PASS].Empty();
				m_gridRows[nDataRow][I_VALUE_FAIL].Empty();
				m_gridRows[nDataRow][I_VALUE_TOTAL].Empty();
			}
			else if (strTestType == _T("SYS_TEST") || strTestType == _T("SYSTEM_TEST"))
			{
				m_gridRows[nDataRow][I_SYS_RETURN].Empty();
				m_gridRows[nDataRow][I_SYS_PASS].Empty();
				m_gridRows[nDataRow][I_SYS_FAIL].Empty();
				m_gridRows[nDataRow][I_SYS_TOTAL].Empty();
			}
		}

		if (m_ctrlGrid.GetSafeHwnd() != NULL)
		{
			RefreshGrid();
		}
		CmassproductiontoolDlg* pMainDlg = dynamic_cast<CmassproductiontoolDlg*>(AfxGetMainWnd());
		if (pMainDlg != NULL)
		{
			pMainDlg->ResetOverallTestStatus();
		}
		return;
	}

	int nReturnCol = FindColumnIndex(_T("RETURN"));
	if (nReturnCol < 0)
	{
		nReturnCol = FindColumnIndex(_T("RETURN_VALUE"));
	}
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

	CmassproductiontoolDlg* pMainDlg = dynamic_cast<CmassproductiontoolDlg*>(AfxGetMainWnd());
	if (pMainDlg != NULL)
	{
		pMainDlg->ResetOverallTestStatus();
	}
}

void CRangeTestPage::UpdateOverallTestStatus()
{
	CmassproductiontoolDlg* pMainDlg = dynamic_cast<CmassproductiontoolDlg*>(AfxGetMainWnd());
	if (pMainDlg == NULL)
	{
		return;
	}

	BOOL bAnyFail = FALSE;
	BOOL bAllCompleted = FALSE;
	int nTestRowCount = 0;
	int nCompletedRowCount = 0;

	if (IsIntegratedGrid())
	{
		for (size_t nDataRow = 0; nDataRow < m_gridRows.size(); ++nDataRow)
		{
			if (I_COL_TEST_TYPE >= static_cast<int>(m_gridRows[nDataRow].size()))
			{
				continue;
			}

			int nPassCol = -1;
			int nFailCol = -1;
			int nTotalCol = -1;
			if (!GetIntegratedResultColumns(m_gridRows[nDataRow][I_COL_TEST_TYPE], nPassCol, nFailCol, nTotalCol))
			{
				continue;
			}

			++nTestRowCount;
			const int nFailCount = (nFailCol >= 0 && nFailCol < static_cast<int>(m_gridRows[nDataRow].size())) ?
				MpReadCountText(m_gridRows[nDataRow][nFailCol]) : 0;
			const int nTotalCount = (nTotalCol >= 0 && nTotalCol < static_cast<int>(m_gridRows[nDataRow].size())) ?
				MpReadCountText(m_gridRows[nDataRow][nTotalCol]) : 0;

			if (nFailCount > 0)
			{
				bAnyFail = TRUE;
			}
			if (nTotalCount > 0)
			{
				++nCompletedRowCount;
			}
		}
	}
	else
	{
		const int nFailCol = FindColumnIndex(_T("FAIL"));
		const int nTotalCol = FindColumnIndex(_T("TOTAL"));
		if (nFailCol >= 0 && nTotalCol >= 0)
		{
			for (size_t nDataRow = 0; nDataRow < m_gridRows.size(); ++nDataRow)
			{
				++nTestRowCount;
				const int nFailCount = (nFailCol < static_cast<int>(m_gridRows[nDataRow].size())) ?
					MpReadCountText(m_gridRows[nDataRow][nFailCol]) : 0;
				const int nTotalCount = (nTotalCol < static_cast<int>(m_gridRows[nDataRow].size())) ?
					MpReadCountText(m_gridRows[nDataRow][nTotalCol]) : 0;

				if (nFailCount > 0)
				{
					bAnyFail = TRUE;
				}
				if (nTotalCount > 0)
				{
					++nCompletedRowCount;
				}
			}
		}
	}

	bAllCompleted = (nTestRowCount > 0 && nCompletedRowCount == nTestRowCount);
	pMainDlg->UpdateOverallTestStatus(bAllCompleted, bAnyFail);
}

LRESULT CRangeTestPage::OnGridButtonClick(WPARAM wParam, LPARAM lParam)
{
	const int nRow = static_cast<int>(wParam);
	const int nCol = static_cast<int>(lParam);

	CString strMessage;
	strMessage.Format(_T("Grid button clicked. row=%d, col=%d"), nRow, nCol);
	TRACE(_T("%s\n"), strMessage.GetString());
	if (nRow < GetHeaderRowCount())
	{
		return 0;
	}
	RunRangeTestRow(nRow, FALSE);

	return 0;
}

void CRangeTestPage::OnSize(UINT nType, int cx, int cy)
{
	CPropertyPage::OnSize(nType, cx, cy);
	MpScaleChildLayout(this, m_initialClientSize, m_childLayouts, cx, cy);
	UpdateGridLayout();
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

	CmassproductiontoolDlg* pMainDlg = dynamic_cast<CmassproductiontoolDlg*>(AfxGetMainWnd());
	if (pMainDlg != NULL && pMainDlg->LoadSystemConfigFile(dlg.GetPathName(), TRUE))
	{
		return;
	}

	if (!LoadRangeConfigFile(dlg.GetPathName()))
	{
		AfxMessageBox(_T("Failed to read the Range Test Excel file."));
		return;
	}
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
	for (int nRow = GetHeaderRowCount(); nRow < m_ctrlGrid.GetRowCount(); ++nRow)
	{
		std::vector<CString> rowValues(m_columnNames.size());
		BOOL bHasValue = FALSE;
		CString strIntegratedTestType;
		if (IsIntegratedGrid())
		{
			strIntegratedTestType = GetIntegratedTestType(nRow);
		}
		for (int nCol = 0; nCol < m_ctrlGrid.GetColumnCount() && nCol < static_cast<int>(m_columnNames.size()); ++nCol)
		{
			if (IsIntegratedGrid() &&
				(((strIntegratedTestType == _T("SYS_TEST") || strIntegratedTestType == _T("SYSTEM_TEST")) && nCol == I_SYS_EXECUTION) ||
				(strIntegratedTestType == _T("RANGE_TEST") && nCol == I_RANGE_EXECUTION) ||
				(strIntegratedTestType == _T("VALUE_TEST") && nCol == I_VALUE_EXECUTION)))
			{
				continue;
			}

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

	const BOOL bIntegratedSheet = IsIntegratedGrid();
	BOOL bSaved = FALSE;
	if (bIntegratedSheet)
	{
		std::vector<CString> sysHeader(m_columnNames.size());
		sysHeader[I_COL_TEST_TYPE] = _T("SYS TEST");
		sysHeader[I_SYS_TYPE] = _T("TYPE");
		sysHeader[I_SYS_EXPECTED] = _T("EXPECTED");
		sysHeader[I_SYS_RETURN] = _T("RETURN");
		sysHeader[I_SYS_PASS] = _T("PASS");
		sysHeader[I_SYS_FAIL] = _T("FAIL");
		sysHeader[I_SYS_TOTAL] = _T("TOTAL");
		sysHeader[I_SYS_EXECUTION] = _T("EXCUTION");

		std::vector<CString> rangeHeader(m_columnNames.size());
		rangeHeader[I_COL_TEST_TYPE] = _T("RANGE TEST");
		rangeHeader[I_RANGE_INDEX] = _T("INDEX");
		rangeHeader[I_RANGE_DESC] = _T("Description");
		rangeHeader[I_RANGE_MIN] = _T("MIN");
		rangeHeader[I_RANGE_MAX] = _T("MAX");
		rangeHeader[I_RANGE_DELAY] = _T("DELAY ms");
		rangeHeader[I_RANGE_RETURN] = _T("RETURN");
		rangeHeader[I_RANGE_PASS] = _T("PASS");
		rangeHeader[I_RANGE_FAIL] = _T("FAIL");
		rangeHeader[I_RANGE_TOTAL] = _T("TOTAL");
		rangeHeader[I_RANGE_EXECUTION] = _T("EXCUTION");

		std::vector<CString> valueHeader(m_columnNames.size());
		valueHeader[I_COL_TEST_TYPE] = _T("VALUE TEST");
		valueHeader[I_VALUE_HDR] = _T("HDR");
		valueHeader[I_VALUE_LENGTH] = _T("LENGTH");
		valueHeader[I_VALUE_TYPE] = _T("TYPE");
		valueHeader[I_VALUE_CATEGORY] = _T("CATEGORY");
		valueHeader[I_VALUE_OPCODE] = _T("OPCODE");
		for (int nData = 0; nData < 8 && I_VALUE_DATA0 + nData < static_cast<int>(valueHeader.size()); ++nData)
		{
			valueHeader[I_VALUE_DATA0 + nData].Format(_T("D[%d]"), nData);
		}
		valueHeader[I_VALUE_DELAY] = _T("DELAY ms");
		valueHeader[I_VALUE_EXPECTED] = _T("EXPECTED");
		valueHeader[I_VALUE_RETURN] = _T("RETURN");
		valueHeader[I_VALUE_PASS] = _T("PASS");
		valueHeader[I_VALUE_FAIL] = _T("FAIL");
		valueHeader[I_VALUE_TOTAL] = _T("TOTAL");
		valueHeader[I_VALUE_EXECUTION] = _T("EXCUTION");

		std::vector<std::vector<CString>> exportRows;
		exportRows.push_back(rangeHeader);
		exportRows.push_back(valueHeader);
		exportRows.insert(exportRows.end(), gridRows.begin(), gridRows.end());
		bSaved = CExcelXlsxHelper::SaveValueSheet(strPath, sysHeader, exportRows);
	}
	else
	{
		bSaved = CExcelXlsxHelper::SaveRangeSheet(strPath, gridRows);
	}
	if (!bSaved)
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

			const int nGridRow = nDataRow + GetHeaderRowCount();
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
