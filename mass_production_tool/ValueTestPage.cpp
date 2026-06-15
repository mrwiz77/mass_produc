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
	return TRUE;
}

BEGIN_MESSAGE_MAP(CValueTestPage, CPropertyPage)
	ON_MESSAGE(WM_GRID_BUTTON_CLICK, &CValueTestPage::OnGridButtonClick)
	ON_WM_CTLCOLOR()
	ON_WM_ERASEBKGND()
	ON_BN_CLICKED(IDC_BTN_PRO_VALUE_READ_FILE, &CValueTestPage::OnBnClickedBtnProValueReadFile)
	ON_BN_CLICKED(IDC_BTN_PRO_VALUE_WRITE_FILE, &CValueTestPage::OnBnClickedBtnProValueWriteFile)
	ON_BN_CLICKED(IDC_BTN_PROP_VALUE_APPLY, &CValueTestPage::OnBnClickedBtnPropValueApply)
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
	m_ctrlGrid.SetFixedBkColor(RGB(120, 120, 120));
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

	for (int i = 2; i < m_ctrlGrid.GetColumnCount()-6; ++i) {

		SetGridCellThemeColor(0, i, MP_GRID_COLOR_DARK_ORANGE);//min
	}
	int i = m_ctrlGrid.GetColumnCount() - 5;
	SetGridCellThemeColor(0, i, MP_GRID_COLOR_DARK_ORANGE);//min
	SetGridCellThemeColor(0, 0, MP_GRID_COLOR_DARK_ORANGE);//min
	ApplyGridCellColors();
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

LRESULT CValueTestPage::OnGridButtonClick(WPARAM wParam, LPARAM lParam)
{
	const int nRow = static_cast<int>(wParam);
	const int nCol = static_cast<int>(lParam);

	CString strMessage;
	strMessage.Format(_T("Value grid button clicked. row=%d, col=%d"), nRow, nCol);
	TRACE(_T("%s\n"), strMessage.GetString());

	return 0;
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
