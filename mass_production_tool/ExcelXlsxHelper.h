#pragma once

#include <vector>
#include <afxstr.h>

// CExcelXlsxHelper
//
// Helper class for RangeTestPage and ValueTestPage .xlsx import/export.
// This class does not use Excel COM automation, so Microsoft Excel does not
// need to be installed on the target PC. It reads and writes the Open XML
// workbook package directly: .xlsx is handled as a ZIP package and worksheet
// data is handled as XML.
//
// Range Test usage:
// 1. BuildRangeColumns()
//    - Creates the fixed RangeTestPage column list.
//    - Returned column order is:
//      INDEX(hex), Description, MIN  ( int ), MAX  ( int ), DELAY ms,
//      RETURN  ( int ), PASS, FAIL, TOTAL, EXCUTION.
//
// 2. LoadRangeSheet(strFilePath, outRows)
//    - Reads the first worksheet and maps data by the fixed Range Test headers.
//    - The EXCUTION button column may be missing in the Excel file; it remains
//      empty in outRows and the grid fills the RUN button text while rendering.
//    - Numeric values are normalized by the common grid rules: COUNT columns are
//      decimal integer strings; other numeric values are uppercase HEX without 0x.
//
// 3. SaveRangeSheet(strFilePath, rows)
//    - Saves Range Test rows using the fixed Range Test columns.
//    - EXCUTION/EXECUTION is treated as the grid button column and is excluded
//      from the workbook.
//    - Header fill, PASS/FAIL colors, PASS, and FAIL colors use the
//      same formatting rules as Value Test export.
//
// Value Test usage:
// 1. BuildValueColumns(nDataLength)
//    - Creates the ValueTestPage grid column list.
//    - If nDataLength is 8, DATA[0] through DATA[7] are generated.
//    - Returned column order is:
//      HDR, LENGTH, TYPE, CATEGORY, OPCODE, DATA[n], DELAY ms, EXPECTED,
//      RETURN, PASS, FAIL, TOTAL, EXCUTION.
//
// 2. LoadValueSheet(strFilePath, currentColumns, outRows)
//    - Reads the first worksheet from strFilePath and returns row data mapped
//      to currentColumns order.
//    - If the first non-empty row is a header row, values are mapped by header
//      name instead of raw Excel column position.
//    - If the Excel file has more DATA[] columns than currentColumns, only the
//      DATA[] columns that exist in currentColumns are imported.
//    - If the Excel file has fewer DATA[] columns than currentColumns, missing
//      DATA[] values remain empty strings.
//    - Columns containing COUNT are normalized as decimal integer strings.
//      Other numeric values are normalized as uppercase HEX strings without 0x.
//    - Returns TRUE on successful parsing. outRows receives grid row data.
//
// 3. SaveValueSheet(strFilePath, columns, rows)
//    - Saves columns/rows to an .xlsx file.
//    - EXCUTION or EXECUTION is treated as the grid button column and is not
//      exported.
//    - Header cells are written with a light gray fill.
//    - PASS text is written in blue and FAIL text is written in red.
//    - PASS column text is written in dark blue.
//    - FAIL column text is written in dark red.
//    - Excel column widths are calculated from header/data text lengths so the
//      header text is less likely to be clipped in one cell.
//
// 4. RemapRowsByHeader(oldColumns, oldRows, newColumns)
//    - Use this when the grid column structure changes, for example when the
//      DATA length is changed by the Apply button.
//    - Existing values are copied to columns with the same header name.
//    - New columns are left as empty strings.
class CExcelXlsxHelper
{
public:
	static std::vector<CString> BuildValueColumns(int nDataLength);
	static std::vector<CString> BuildRangeColumns();
	static std::vector<CString> BuildSystemColumns();
	static std::vector<CString> BuildSystemTypes();
	static std::vector<CString> BuildSystemDutConfigColumns();
	static std::vector<std::vector<CString>> BuildDefaultSystemDutConfigRows();

	static BOOL LoadSheet(
		const CString& strFilePath,
		const std::vector<CString>& currentColumns,
		std::vector<std::vector<CString>>& outRows,
		int nSheetIndex);

	static BOOL LoadSheetAll(
		const CString& strFilePath,
		std::vector<CString>& outColumns,
		std::vector<std::vector<CString>>& outRows,
		int nSheetIndex);

	static BOOL LoadSystemSheet(
		const CString& strFilePath,
		std::vector<std::vector<CString>>& outRows);

	static BOOL SaveSystemSheet(
		const CString& strFilePath,
		const std::vector<std::vector<CString>>& rows);

	static BOOL LoadRangeSheet(
		const CString& strFilePath,
		std::vector<std::vector<CString>>& outRows);

	static BOOL SaveRangeSheet(
		const CString& strFilePath,
		const std::vector<std::vector<CString>>& rows);

	static BOOL LoadValueSheet(
		const CString& strFilePath,
		const std::vector<CString>& currentColumns,
		std::vector<std::vector<CString>>& outRows);

	static BOOL SaveValueSheet(
		const CString& strFilePath,
		const std::vector<CString>& columns,
		const std::vector<std::vector<CString>>& rows);

	static std::vector<std::vector<CString>> RemapRowsByHeader(
		const std::vector<CString>& oldColumns,
		const std::vector<std::vector<CString>>& oldRows,
		const std::vector<CString>& newColumns);
};
