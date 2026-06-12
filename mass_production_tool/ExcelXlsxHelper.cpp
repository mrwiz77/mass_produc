#include "pch.h"

#include "ExcelXlsxHelper.h"

#include <msxml6.h>

namespace MSXML2
{
	typedef _com_ptr_t<_com_IIID<::IXMLDOMDocument2, &__uuidof(::IXMLDOMDocument2)>> IXMLDOMDocument2Ptr;
	typedef _com_ptr_t<_com_IIID<::IXMLDOMNode, &__uuidof(::IXMLDOMNode)>> IXMLDOMNodePtr;
	typedef _com_ptr_t<_com_IIID<::IXMLDOMNodeList, &__uuidof(::IXMLDOMNodeList)>> IXMLDOMNodeListPtr;
	typedef _com_ptr_t<_com_IIID<::IXMLDOMNamedNodeMap, &__uuidof(::IXMLDOMNamedNodeMap)>> IXMLDOMNamedNodeMapPtr;
}


namespace
{
	CString TrimText(const CString& strText)
	{
		CString strResult(strText);
		strResult.Trim();
		return strResult;
	}

	BOOL IsCountColumn(const CString& strColumnName)
	{
		CString strUpper(strColumnName);
		strUpper.MakeUpper();
		return strUpper.Find(_T("COUNT")) >= 0;
	}

	BOOL IsExecutionColumn(const CString& strColumnName)
	{
		CString strUpper(strColumnName);
		strUpper.MakeUpper();
		return strUpper == _T("EXCUTION") || strUpper == _T("EXECUTION");
	}

	BOOL IsPassText(const CString& strText)
	{
		CString strUpper = TrimText(strText);
		strUpper.MakeUpper();
		return strUpper == _T("PASS");
	}

	BOOL IsFailText(const CString& strText)
	{
		CString strUpper = TrimText(strText);
		strUpper.MakeUpper();
		return strUpper == _T("FAIL");
	}

	BOOL IsPassCountColumn(const CString& strColumnName)
	{
		CString strUpper(strColumnName);
		strUpper.MakeUpper();
		return strUpper == _T("PASS COUNT");
	}

	BOOL IsFailCountColumn(const CString& strColumnName)
	{
		CString strUpper(strColumnName);
		strUpper.MakeUpper();
		return strUpper == _T("FAIL COUNT");
	}

	BOOL TryParseInteger(const CString& strText, unsigned long long& value)
	{
		CString strValue = TrimText(strText);
		if (strValue.IsEmpty())
		{
			return FALSE;
		}

		int nBase = 10;
		if (strValue.GetLength() > 2 && strValue.Left(2).CompareNoCase(_T("0x")) == 0)
		{
			nBase = 16;
			strValue = strValue.Mid(2);
		}
		else
		{
			for (int i = 0; i < strValue.GetLength(); ++i)
			{
				if ((_T('A') <= strValue[i] && strValue[i] <= _T('F')) ||
					(_T('a') <= strValue[i] && strValue[i] <= _T('f')))
				{
					nBase = 16;
					break;
				}
			}
		}

		TCHAR* pEnd = NULL;
		value = _tcstoui64(strValue, &pEnd, nBase);
		return pEnd != NULL && *pEnd == _T('\0');
	}

	// Normalize an Excel cell value using the grid display rules.
	// COUNT columns stay decimal; other numeric columns become HEX without 0x.
	CString FormatGridValue(const CString& strText, const CString& strColumnName)
	{
		unsigned long long value = 0;
		if (!TryParseInteger(strText, value))
		{
			return TrimText(strText);
		}

		CString strResult;
		if (IsCountColumn(strColumnName))
		{
			strResult.Format(_T("%llu"), value);
		}
		else
		{
			strResult.Format(_T("%llX"), value);
		}
		return strResult;
	}

	int ColumnNameToIndex(const CString& strCellRef)
	{
		int nCol = 0;
		for (int i = 0; i < strCellRef.GetLength(); ++i)
		{
			const TCHAR ch = strCellRef[i];
			if (ch < _T('A') || ch > _T('Z'))
			{
				break;
			}
			nCol = nCol * 26 + (ch - _T('A') + 1);
		}
		return nCol - 1;
	}

	CString GetExcelColumnName(int nCol)
	{
		CString strName;
		int nValue = nCol + 1;
		while (nValue > 0)
		{
			const int nRemainder = (nValue - 1) % 26;
			strName.Insert(0, static_cast<TCHAR>(_T('A') + nRemainder));
			nValue = (nValue - 1) / 26;
		}
		return strName;
	}

	CString XmlEscape(const CString& strText)
	{
		CString strResult;
		for (int i = 0; i < strText.GetLength(); ++i)
		{
			switch (strText[i])
			{
			case _T('&'): strResult += _T("&amp;"); break;
			case _T('<'): strResult += _T("&lt;"); break;
			case _T('>'): strResult += _T("&gt;"); break;
			case _T('"'): strResult += _T("&quot;"); break;
			case _T('\''): strResult += _T("&apos;"); break;
			default: strResult += strText[i]; break;
			}
		}
		return strResult;
	}

	CString GetNodeText(MSXML2::IXMLDOMNodePtr spNode)
	{
		if (spNode == NULL)
		{
			return CString();
		}

		CComBSTR bstrText;
		if (FAILED(spNode->get_text(&bstrText)) || bstrText == NULL)
		{
			return CString();
		}
		return CString(static_cast<LPCWSTR>(bstrText));
	}

	CString GetNodeAttribute(MSXML2::IXMLDOMNodePtr spNode, LPCWSTR pszName)
	{
		if (spNode == NULL)
		{
			return CString();
		}

		MSXML2::IXMLDOMNamedNodeMapPtr spAttributes;
		if (FAILED(spNode->get_attributes(&spAttributes)) || spAttributes == NULL)
		{
			return CString();
		}

		MSXML2::IXMLDOMNodePtr spAttribute;
		if (FAILED(spAttributes->getNamedItem(CComBSTR(pszName), &spAttribute)) || spAttribute == NULL)
		{
			return CString();
		}

		return GetNodeText(spAttribute);
	}

	BOOL FileExists(const CString& strPath)
	{
		const DWORD dwAttributes = GetFileAttributes(strPath);
		return dwAttributes != INVALID_FILE_ATTRIBUTES && (dwAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
	}

	BOOL DirectoryExists(const CString& strPath)
	{
		const DWORD dwAttributes = GetFileAttributes(strPath);
		return dwAttributes != INVALID_FILE_ATTRIBUTES && (dwAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
	}

	BOOL LoadXmlDocument(const CString& strPath, MSXML2::IXMLDOMDocument2Ptr& spDocument)
	{
		HRESULT hr = spDocument.CreateInstance(__uuidof(DOMDocument60));
		if (FAILED(hr) || spDocument == NULL)
		{
			return FALSE;
		}

		spDocument->put_async(VARIANT_FALSE);
		spDocument->put_validateOnParse(VARIANT_FALSE);
		spDocument->put_resolveExternals(VARIANT_FALSE);
		spDocument->setProperty(CComBSTR(L"SelectionLanguage"), CComVariant(L"XPath"));
		spDocument->setProperty(CComBSTR(L"SelectionNamespaces"), CComVariant(L"xmlns:x='http://schemas.openxmlformats.org/spreadsheetml/2006/main'"));

		VARIANT_BOOL vbLoaded = VARIANT_FALSE;
		hr = spDocument->load(CComVariant(strPath), &vbLoaded);
		return SUCCEEDED(hr) && vbLoaded == VARIANT_TRUE;
	}

	// .xlsx is a ZIP package. For reads, Windows Shell extracts it to a temp folder,
	// then xl/worksheets/sheet1.xml and xl/sharedStrings.xml are parsed directly.
	BOOL CopyZipContent(const CString& strZipPath, const CString& strDestPath)
	{
		SHCreateDirectoryEx(NULL, strDestPath, NULL);

		CComPtr<IShellDispatch> spShell;
		HRESULT hr = spShell.CoCreateInstance(CLSID_Shell);
		if (FAILED(hr) || spShell == NULL)
		{
			return FALSE;
		}

		CComPtr<Folder> spZipFolder;
		hr = spShell->NameSpace(CComVariant(strZipPath), &spZipFolder);
		if (FAILED(hr) || spZipFolder == NULL)
		{
			return FALSE;
		}

		CComPtr<Folder> spDestFolder;
		hr = spShell->NameSpace(CComVariant(strDestPath), &spDestFolder);
		if (FAILED(hr) || spDestFolder == NULL)
		{
			return FALSE;
		}

		CComPtr<FolderItems> spItems;
		hr = spZipFolder->Items(&spItems);
		if (FAILED(hr) || spItems == NULL)
		{
			return FALSE;
		}

		hr = spDestFolder->CopyHere(CComVariant(spItems), CComVariant(FOF_NOCONFIRMATION | FOF_SILENT | FOF_NOERRORUI));
		if (FAILED(hr))
		{
			return FALSE;
		}

		const CString strSheetPath = strDestPath + _T("\\xl\\worksheets\\sheet1.xml");
		for (int i = 0; i < 100; ++i)
		{
			if (FileExists(strSheetPath))
			{
				return TRUE;
			}
			Sleep(50);
		}

		return FileExists(strSheetPath);
	}

	void DeleteDirectoryTree(const CString& strPath)
	{
		if (!DirectoryExists(strPath))
		{
			return;
		}

		CString strFrom(strPath);
		strFrom += _T('\0');

		SHFILEOPSTRUCT fileOp = {};
		fileOp.wFunc = FO_DELETE;
		fileOp.pFrom = strFrom;
		fileOp.fFlags = FOF_NOCONFIRMATION | FOF_SILENT | FOF_NOERRORUI;
		SHFileOperation(&fileOp);
	}

	void WriteUInt16(CFile& file, WORD value)
	{
		BYTE data[2] = {
			static_cast<BYTE>(value & 0xFF),
			static_cast<BYTE>((value >> 8) & 0xFF)
		};
		file.Write(data, sizeof(data));
	}

	void WriteUInt32(CFile& file, DWORD value)
	{
		BYTE data[4] = {
			static_cast<BYTE>(value & 0xFF),
			static_cast<BYTE>((value >> 8) & 0xFF),
			static_cast<BYTE>((value >> 16) & 0xFF),
			static_cast<BYTE>((value >> 24) & 0xFF)
		};
		file.Write(data, sizeof(data));
	}

	DWORD CalcCrc32(const BYTE* pData, DWORD dwSize)
	{
		DWORD crc = 0xFFFFFFFF;
		for (DWORD i = 0; i < dwSize; ++i)
		{
			crc ^= pData[i];
			for (int bit = 0; bit < 8; ++bit)
			{
				crc = (crc >> 1) ^ (0xEDB88320 & static_cast<DWORD>(-(static_cast<LONG>(crc & 1))));
			}
		}
		return ~crc;
	}

	struct ZipEntryInfo
	{
		CStringA name;
		DWORD crc;
		DWORD size;
		DWORD offset;
	};

	BOOL AddZipEntry(CFile& file, const CStringA& name, const CString& text, std::vector<ZipEntryInfo>& entries)
	{
		CW2A utf8Text(text, CP_UTF8);
		const BYTE* pData = reinterpret_cast<const BYTE*>(static_cast<LPCSTR>(utf8Text));
		const DWORD dwSize = static_cast<DWORD>(strlen(static_cast<LPCSTR>(utf8Text)));
		const DWORD dwCrc = CalcCrc32(pData, dwSize);
		const DWORD dwOffset = static_cast<DWORD>(file.GetPosition());

		WriteUInt32(file, 0x04034B50);
		WriteUInt16(file, 20);
		WriteUInt16(file, 0);
		WriteUInt16(file, 0);
		WriteUInt16(file, 0);
		WriteUInt16(file, 0);
		WriteUInt32(file, dwCrc);
		WriteUInt32(file, dwSize);
		WriteUInt32(file, dwSize);
		WriteUInt16(file, static_cast<WORD>(name.GetLength()));
		WriteUInt16(file, 0);
		file.Write(static_cast<LPCSTR>(name), name.GetLength());
		if (dwSize > 0)
		{
			file.Write(pData, dwSize);
		}

		ZipEntryInfo info;
		info.name = name;
		info.crc = dwCrc;
		info.size = dwSize;
		info.offset = dwOffset;
		entries.push_back(info);
		return TRUE;
	}

	// For writes, create the ZIP central directory directly instead of using Shell ZIP.
	// This avoids the previous empty 22-byte ZIP problem,
	// and produces a normal .xlsx package that Excel can open.
	BOOL CreateXlsxPackage(const CString& strXlsxPath, const std::vector<std::pair<CStringA, CString>>& files)
	{
		CFile file;
		if (!file.Open(strXlsxPath, CFile::modeCreate | CFile::modeWrite | CFile::typeBinary))
		{
			return FALSE;
		}

		std::vector<ZipEntryInfo> entries;
		for (size_t i = 0; i < files.size(); ++i)
		{
			if (!AddZipEntry(file, files[i].first, files[i].second, entries))
			{
				file.Close();
				return FALSE;
			}
		}

		const DWORD dwCentralDirOffset = static_cast<DWORD>(file.GetPosition());
		for (size_t i = 0; i < entries.size(); ++i)
		{
			const ZipEntryInfo& entry = entries[i];
			WriteUInt32(file, 0x02014B50);
			WriteUInt16(file, 20);
			WriteUInt16(file, 20);
			WriteUInt16(file, 0);
			WriteUInt16(file, 0);
			WriteUInt16(file, 0);
			WriteUInt16(file, 0);
			WriteUInt32(file, entry.crc);
			WriteUInt32(file, entry.size);
			WriteUInt32(file, entry.size);
			WriteUInt16(file, static_cast<WORD>(entry.name.GetLength()));
			WriteUInt16(file, 0);
			WriteUInt16(file, 0);
			WriteUInt16(file, 0);
			WriteUInt16(file, 0);
			WriteUInt32(file, 0);
			WriteUInt32(file, entry.offset);
			file.Write(static_cast<LPCSTR>(entry.name), entry.name.GetLength());
		}

		const DWORD dwCentralDirSize = static_cast<DWORD>(file.GetPosition()) - dwCentralDirOffset;
		WriteUInt32(file, 0x06054B50);
		WriteUInt16(file, 0);
		WriteUInt16(file, 0);
		WriteUInt16(file, static_cast<WORD>(entries.size()));
		WriteUInt16(file, static_cast<WORD>(entries.size()));
		WriteUInt32(file, dwCentralDirSize);
		WriteUInt32(file, dwCentralDirOffset);
		WriteUInt16(file, 0);

		file.Close();
		return TRUE;
	}
}

std::vector<CString> CExcelXlsxHelper::BuildValueColumns(int nDataLength)
{
	if (nDataLength < 0)
	{
		nDataLength = 0;
	}

	std::vector<CString> columns;
	columns.push_back(_T("HDR"));
	columns.push_back(_T("LENGTH"));
	columns.push_back(_T("TYPE"));
	columns.push_back(_T("CATEGORY"));
	columns.push_back(_T("OPCODE"));
	for (int nDataIndex = 0; nDataIndex < nDataLength; ++nDataIndex)
	{
		CString strColumnName;
		strColumnName.Format(_T("DATA[%d]"), nDataIndex);
		columns.push_back(strColumnName);
	}
	columns.push_back(_T("DELAY ms"));
	columns.push_back(_T("RETURN"));
	columns.push_back(_T("PASS COUNT"));
	columns.push_back(_T("FAIL COUNT"));
	columns.push_back(_T("TOTAL COUNT"));
	columns.push_back(_T("EXCUTION"));
	return columns;
}

std::vector<CString> CExcelXlsxHelper::BuildSystemColumns()
{
	std::vector<CString> columns;
	columns.push_back(_T("TYPE"));
	columns.push_back(_T("EXPECTEDVALUE"));
	columns.push_back(_T("RETURN_VALUE"));
	columns.push_back(_T("PASS"));
	columns.push_back(_T("FAIL"));
	return columns;
}

std::vector<CString> CExcelXlsxHelper::BuildSystemTypes()
{
	std::vector<CString> types;
	types.push_back(_T("AP SW Version"));
	types.push_back(_T("MICOM SW Version"));
	types.push_back(_T("Ethernet Switch SW Version"));
	types.push_back(_T("Modem #1 Version Info"));
	types.push_back(_T("Modem #2 Version Info"));
	types.push_back(_T("Modem #3 Version Info"));
	types.push_back(_T("Modem #4 Version Info"));
	types.push_back(_T("Modem #5 Version Info"));
	return types;
}

std::vector<CString> CExcelXlsxHelper::BuildSystemDutConfigColumns()
{
	std::vector<CString> columns;
	columns.push_back(_T("GROUP"));
	columns.push_back(_T("ITEM"));
	columns.push_back(_T("VALUE"));
	return columns;
}

std::vector<std::vector<CString>> CExcelXlsxHelper::BuildDefaultSystemDutConfigRows()
{
	std::vector<std::vector<CString>> rows;
	rows.push_back({ _T("Timeout"), _T("FTM Enter Noti Receive Timeout"), _T("0") });
	rows.push_back({ _T("Timeout"), _T("Boot Status Check Command Timeout"), _T("0") });
	rows.push_back({ _T("Timeout"), _T("Test Mode Setting Command Timeout"), _T("0") });
	rows.push_back({ _T("Timeout"), _T("DUT Version Check Command Timeout"), _T("0") });
	rows.push_back({ _T("Timeout"), _T("Ethernet Test Command Timeout"), _T("0") });
	rows.push_back({ _T("Timeout"), _T("Modem Info Test Command Timeout"), _T("0") });
	rows.push_back({ _T("Timeout"), _T("Modem RF Test Command Timeout"), _T("0") });
	rows.push_back({ _T("Timeout"), _T("FAN Test Command Timeout"), _T("0") });
	rows.push_back({ _T("Timeout"), _T("CAN Test Command Timeout"), _T("0") });
	rows.push_back({ _T("Factory Test Mode"), _T("Perform FACTORY_TEST Mode 진입 Noti Check"), _T("0") });
	rows.push_back({ _T("Boot Status Check"), _T("Perform Boot Status Check"), _T("0") });
	rows.push_back({ _T("Boot Status Check"), _T("MCU Boot 완료 Check"), _T("0") });
	rows.push_back({ _T("Boot Status Check"), _T("AP Boot 완료 Check"), _T("0") });
	rows.push_back({ _T("Boot Status Check"), _T("Modem Boot 완료 Check"), _T("0") });
	return rows;
}
BOOL CExcelXlsxHelper::LoadSystemSheet(
	const CString& strFilePath,
	std::vector<std::vector<CString>>& outRows)
{
	const std::vector<CString> columns = BuildSystemColumns();
	std::vector<std::vector<CString>> loadedRows;
	if (!LoadValueSheet(strFilePath, columns, loadedRows))
	{
		return FALSE;
	}

	const std::vector<CString> systemTypes = BuildSystemTypes();
	outRows.clear();
	outRows.resize(systemTypes.size());
	for (size_t nType = 0; nType < systemTypes.size(); ++nType)
	{
		outRows[nType].resize(columns.size());
		outRows[nType][0] = systemTypes[nType];
	}

	for (size_t nRow = 0; nRow < loadedRows.size(); ++nRow)
	{
		if (loadedRows[nRow].empty())
		{
			continue;
		}

		CString strType = TrimText(loadedRows[nRow][0]);
		for (size_t nType = 0; nType < systemTypes.size(); ++nType)
		{
			if (strType.CompareNoCase(systemTypes[nType]) == 0)
			{
				for (size_t nCol = 0; nCol < outRows[nType].size() && nCol < loadedRows[nRow].size(); ++nCol)
				{
					outRows[nType][nCol] = loadedRows[nRow][nCol];
				}
				outRows[nType][0] = systemTypes[nType];
				break;
			}
		}
	}

	return TRUE;
}

BOOL CExcelXlsxHelper::SaveSystemSheet(
	const CString& strFilePath,
	const std::vector<std::vector<CString>>& rows)
{
	return SaveValueSheet(strFilePath, BuildSystemColumns(), rows);
}
std::vector<CString> CExcelXlsxHelper::BuildRangeColumns()
{
	std::vector<CString> columns;
	columns.push_back(_T("INDEX(hex)"));
	columns.push_back(_T("Description"));
	columns.push_back(_T("MIN  ( int )"));
	columns.push_back(_T("MAX  ( int )"));
	columns.push_back(_T("DELAY ms"));
	columns.push_back(_T("RETURN  ( int )"));
	columns.push_back(_T("PASS COUNT"));
	columns.push_back(_T("FAIL COUNT"));
	columns.push_back(_T("TOTAL COUNT"));
	columns.push_back(_T("EXCUTION"));
	return columns;
}

BOOL CExcelXlsxHelper::LoadRangeSheet(
	const CString& strFilePath,
	std::vector<std::vector<CString>>& outRows)
{
	if (!LoadValueSheet(strFilePath, BuildRangeColumns(), outRows))
	{
		return FALSE;
	}

	const int decimalColumns[] = { 2, 3, 4, 5 };
	for (size_t nRow = 0; nRow < outRows.size(); ++nRow)
	{
		for (int nIndex = 0; nIndex < _countof(decimalColumns); ++nIndex)
		{
			const int nCol = decimalColumns[nIndex];
			if (nCol >= static_cast<int>(outRows[nRow].size()))
			{
				continue;
			}

			CString strValue = TrimText(outRows[nRow][nCol]);
			if (strValue.IsEmpty())
			{
				continue;
			}

			if (strValue.GetLength() > 2 && strValue.Left(2).CompareNoCase(_T("0x")) == 0)
			{
				strValue = strValue.Mid(2);
			}

			TCHAR* pEnd = NULL;
			const unsigned long long value = _tcstoui64(strValue, &pEnd, 16);
			if (pEnd != NULL && *pEnd == _T('\0'))
			{
				outRows[nRow][nCol].Format(_T("%llu"), value);
			}
		}
	}

	return TRUE;
}

BOOL CExcelXlsxHelper::SaveRangeSheet(
	const CString& strFilePath,
	const std::vector<std::vector<CString>>& rows)
{
	return SaveValueSheet(strFilePath, BuildRangeColumns(), rows);
}
std::vector<std::vector<CString>> CExcelXlsxHelper::RemapRowsByHeader(
	const std::vector<CString>& oldColumns,
	const std::vector<std::vector<CString>>& oldRows,
	const std::vector<CString>& newColumns)
{
	std::vector<std::vector<CString>> remappedRows;
	remappedRows.resize(oldRows.size());
	for (size_t nRow = 0; nRow < oldRows.size(); ++nRow)
	{
		remappedRows[nRow].resize(newColumns.size());
		for (size_t nNewCol = 0; nNewCol < newColumns.size(); ++nNewCol)
		{
			for (size_t nOldCol = 0; nOldCol < oldColumns.size(); ++nOldCol)
			{
				if (newColumns[nNewCol].CompareNoCase(oldColumns[nOldCol]) == 0)
				{
					if (nOldCol < oldRows[nRow].size())
					{
						remappedRows[nRow][nNewCol] = oldRows[nRow][nOldCol];
					}
					break;
				}
			}
		}
	}
	return remappedRows;
}

BOOL CExcelXlsxHelper::LoadSheet(
	const CString& strFilePath,
	const std::vector<CString>& currentColumns,
	std::vector<std::vector<CString>>& outRows,
	int nSheetIndex)
{
	outRows.clear();
	if (currentColumns.empty())
	{
		return FALSE;
	}

	TCHAR szTempPath[MAX_PATH] = { 0 };
	if (GetTempPath(MAX_PATH, szTempPath) == 0)
	{
		return FALSE;
	}

	CString strWorkFolder;
	strWorkFolder.Format(_T("%smp_tool_xlsx_%lu_%lu"), szTempPath, GetCurrentProcessId(), GetTickCount());
	if (!CreateDirectory(strWorkFolder, NULL) && GetLastError() != ERROR_ALREADY_EXISTS)
	{
		return FALSE;
	}

	const CString strZipPath = strWorkFolder + _T("\\workbook.zip");
	if (!CopyFile(strFilePath, strZipPath, FALSE))
	{
		DeleteDirectoryTree(strWorkFolder);
		return FALSE;
	}

	BOOL bNeedUninitialize = FALSE;
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	if (SUCCEEDED(hr))
	{
		bNeedUninitialize = TRUE;
	}
	else if (hr != RPC_E_CHANGED_MODE)
	{
		DeleteDirectoryTree(strWorkFolder);
		return FALSE;
	}

	const CString strExtractFolder = strWorkFolder + _T("\\xlsx");
	const BOOL bCopied = CopyZipContent(strZipPath, strExtractFolder);
	if (bNeedUninitialize)
	{
		CoUninitialize();
	}

	if (!bCopied)
	{
		DeleteDirectoryTree(strWorkFolder);
		return FALSE;
	}

	const CString strSharedStringsPath = strExtractFolder + _T("\\xl\\sharedStrings.xml");
	CString strSheetPath;
	strSheetPath.Format(_T("%s\\xl\\worksheets\\sheet%d.xml"), strExtractFolder.GetString(), max(1, nSheetIndex));

	std::vector<CString> sharedStrings;
	if (FileExists(strSharedStringsPath))
	{
		MSXML2::IXMLDOMDocument2Ptr spSharedDoc;
		if (LoadXmlDocument(strSharedStringsPath, spSharedDoc))
		{
			MSXML2::IXMLDOMNodeListPtr spStringNodes;
			spSharedDoc->selectNodes(CComBSTR(L"/x:sst/x:si"), &spStringNodes);
			long nStringCount = 0;
			if (spStringNodes != NULL)
			{
				spStringNodes->get_length(&nStringCount);
			}

			for (long i = 0; i < nStringCount; ++i)
			{
				MSXML2::IXMLDOMNodePtr spStringNode;
				spStringNodes->get_item(i, &spStringNode);

				CString strCombined;
				MSXML2::IXMLDOMNodeListPtr spTextNodes;
				if (spStringNode != NULL && SUCCEEDED(spStringNode->selectNodes(CComBSTR(L".//x:t"), &spTextNodes)) && spTextNodes != NULL)
				{
					long nTextCount = 0;
					spTextNodes->get_length(&nTextCount);
					for (long nText = 0; nText < nTextCount; ++nText)
					{
						MSXML2::IXMLDOMNodePtr spTextNode;
						spTextNodes->get_item(nText, &spTextNode);
						strCombined += GetNodeText(spTextNode);
					}
				}
				sharedStrings.push_back(strCombined);
			}
		}
	}

	MSXML2::IXMLDOMDocument2Ptr spSheetDoc;
	if (!LoadXmlDocument(strSheetPath, spSheetDoc))
	{
		DeleteDirectoryTree(strWorkFolder);
		return FALSE;
	}

	MSXML2::IXMLDOMNodeListPtr spRowNodes;
	spSheetDoc->selectNodes(CComBSTR(L"/x:worksheet/x:sheetData/x:row"), &spRowNodes);
	long nRowNodeCount = 0;
	if (spRowNodes != NULL)
	{
		spRowNodes->get_length(&nRowNodeCount);
	}

	const int nColumnCount = static_cast<int>(currentColumns.size());
	std::vector<int> excelToGridColumns;
	BOOL bHeaderMapped = FALSE;
	for (long nRow = 0; nRow < nRowNodeCount; ++nRow)
	{
		MSXML2::IXMLDOMNodePtr spRowNode;
		spRowNodes->get_item(nRow, &spRowNode);
		if (spRowNode == NULL)
		{
			continue;
		}

		std::vector<CString> excelRowValues;
		BOOL bHasValue = FALSE;

		MSXML2::IXMLDOMNodeListPtr spCellNodes;
		if (FAILED(spRowNode->selectNodes(CComBSTR(L"x:c"), &spCellNodes)) || spCellNodes == NULL)
		{
			continue;
		}

		long nCellCount = 0;
		spCellNodes->get_length(&nCellCount);
		for (long nCell = 0; nCell < nCellCount; ++nCell)
		{
			MSXML2::IXMLDOMNodePtr spCellNode;
			spCellNodes->get_item(nCell, &spCellNode);
			if (spCellNode == NULL)
			{
				continue;
			}

			CString strRef = GetNodeAttribute(spCellNode, L"r");
			strRef.MakeUpper();
			const int nCol = ColumnNameToIndex(strRef);
			if (nCol < 0)
			{
				continue;
			}

			if (nCol >= static_cast<int>(excelRowValues.size()))
			{
				excelRowValues.resize(nCol + 1);
			}

			CString strType = GetNodeAttribute(spCellNode, L"t");
			CString strRawValue;

			if (strType.CompareNoCase(_T("inlineStr")) == 0)
			{
				MSXML2::IXMLDOMNodePtr spInlineText;
				spCellNode->selectSingleNode(CComBSTR(L"x:is/x:t"), &spInlineText);
				strRawValue = GetNodeText(spInlineText);
			}
			else
			{
				MSXML2::IXMLDOMNodePtr spValueNode;
				spCellNode->selectSingleNode(CComBSTR(L"x:v"), &spValueNode);
				strRawValue = GetNodeText(spValueNode);

				if (strType.CompareNoCase(_T("s")) == 0)
				{
					unsigned long long nSharedIndex = 0;
					if (TryParseInteger(strRawValue, nSharedIndex) && nSharedIndex < sharedStrings.size())
					{
						strRawValue = sharedStrings[static_cast<size_t>(nSharedIndex)];
					}
				}
			}

			if (!TrimText(strRawValue).IsEmpty())
			{
				bHasValue = TRUE;
			}

			excelRowValues[nCol] = TrimText(strRawValue);
		}

		if (!bHasValue)
		{
			continue;
		}

		if (!bHeaderMapped)
		{
			excelToGridColumns.assign(excelRowValues.size(), -1);
			for (size_t nExcelCol = 0; nExcelCol < excelRowValues.size(); ++nExcelCol)
			{
				CString strExcelHeader = TrimText(excelRowValues[nExcelCol]);
				for (int nGridCol = 0; nGridCol < nColumnCount; ++nGridCol)
				{
					if (strExcelHeader.CompareNoCase(currentColumns[nGridCol]) == 0)
					{
						excelToGridColumns[nExcelCol] = nGridCol;
						break;
					}
				}
			}

			if (!excelToGridColumns.empty() && excelToGridColumns[0] == 0)
			{
				bHeaderMapped = TRUE;
				continue;
			}

			excelToGridColumns.resize(max(static_cast<int>(excelRowValues.size()), nColumnCount), -1);
			for (int nGridCol = 0; nGridCol < nColumnCount && nGridCol < static_cast<int>(excelToGridColumns.size()); ++nGridCol)
			{
				excelToGridColumns[nGridCol] = nGridCol;
			}
			bHeaderMapped = TRUE;
		}

		std::vector<CString> rowValues(nColumnCount);
		for (size_t nExcelCol = 0; nExcelCol < excelRowValues.size() && nExcelCol < excelToGridColumns.size(); ++nExcelCol)
		{
			const int nGridCol = excelToGridColumns[nExcelCol];
			if (nGridCol < 0 || nGridCol >= nColumnCount)
			{
				continue;
			}

			rowValues[nGridCol] = FormatGridValue(excelRowValues[nExcelCol], currentColumns[nGridCol]);
		}

		outRows.push_back(rowValues);
	}

	DeleteDirectoryTree(strWorkFolder);
	return TRUE;
}

BOOL CExcelXlsxHelper::LoadSheetAll(
	const CString& strFilePath,
	std::vector<CString>& outColumns,
	std::vector<std::vector<CString>>& outRows,
	int nSheetIndex)
{
	outColumns.clear();
	outRows.clear();

	TCHAR szTempPath[MAX_PATH] = { 0 };
	if (GetTempPath(MAX_PATH, szTempPath) == 0)
	{
		return FALSE;
	}

	CString strWorkFolder;
	strWorkFolder.Format(_T("%smp_tool_xlsx_%lu_%lu"), szTempPath, GetCurrentProcessId(), GetTickCount());
	if (!CreateDirectory(strWorkFolder, NULL) && GetLastError() != ERROR_ALREADY_EXISTS)
	{
		return FALSE;
	}

	const CString strZipPath = strWorkFolder + _T("\\workbook.zip");
	if (!CopyFile(strFilePath, strZipPath, FALSE))
	{
		DeleteDirectoryTree(strWorkFolder);
		return FALSE;
	}

	BOOL bNeedUninitialize = FALSE;
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	if (SUCCEEDED(hr))
	{
		bNeedUninitialize = TRUE;
	}
	else if (hr != RPC_E_CHANGED_MODE)
	{
		DeleteDirectoryTree(strWorkFolder);
		return FALSE;
	}

	const CString strExtractFolder = strWorkFolder + _T("\\xlsx");
	const BOOL bCopied = CopyZipContent(strZipPath, strExtractFolder);
	if (bNeedUninitialize)
	{
		CoUninitialize();
	}

	if (!bCopied)
	{
		DeleteDirectoryTree(strWorkFolder);
		return FALSE;
	}

	const CString strSharedStringsPath = strExtractFolder + _T("\\xl\\sharedStrings.xml");
	CString strSheetPath;
	strSheetPath.Format(_T("%s\\xl\\worksheets\\sheet%d.xml"), strExtractFolder.GetString(), max(1, nSheetIndex));

	std::vector<CString> sharedStrings;
	if (FileExists(strSharedStringsPath))
	{
		MSXML2::IXMLDOMDocument2Ptr spSharedDoc;
		if (LoadXmlDocument(strSharedStringsPath, spSharedDoc))
		{
			MSXML2::IXMLDOMNodeListPtr spStringNodes;
			spSharedDoc->selectNodes(CComBSTR(L"/x:sst/x:si"), &spStringNodes);
			long nStringCount = 0;
			if (spStringNodes != NULL)
			{
				spStringNodes->get_length(&nStringCount);
			}

			for (long i = 0; i < nStringCount; ++i)
			{
				MSXML2::IXMLDOMNodePtr spStringNode;
				spStringNodes->get_item(i, &spStringNode);

				CString strCombined;
				MSXML2::IXMLDOMNodeListPtr spTextNodes;
				if (spStringNode != NULL && SUCCEEDED(spStringNode->selectNodes(CComBSTR(L".//x:t"), &spTextNodes)) && spTextNodes != NULL)
				{
					long nTextCount = 0;
					spTextNodes->get_length(&nTextCount);
					for (long nText = 0; nText < nTextCount; ++nText)
					{
						MSXML2::IXMLDOMNodePtr spTextNode;
						spTextNodes->get_item(nText, &spTextNode);
						strCombined += GetNodeText(spTextNode);
					}
				}
				sharedStrings.push_back(strCombined);
			}
		}
	}

	MSXML2::IXMLDOMDocument2Ptr spSheetDoc;
	if (!LoadXmlDocument(strSheetPath, spSheetDoc))
	{
		DeleteDirectoryTree(strWorkFolder);
		return FALSE;
	}

	MSXML2::IXMLDOMNodeListPtr spRowNodes;
	spSheetDoc->selectNodes(CComBSTR(L"/x:worksheet/x:sheetData/x:row"), &spRowNodes);
	long nRowNodeCount = 0;
	if (spRowNodes != NULL)
	{
		spRowNodes->get_length(&nRowNodeCount);
	}

	BOOL bHeaderRead = FALSE;
	for (long nRow = 0; nRow < nRowNodeCount; ++nRow)
	{
		MSXML2::IXMLDOMNodePtr spRowNode;
		spRowNodes->get_item(nRow, &spRowNode);
		if (spRowNode == NULL)
		{
			continue;
		}

		std::vector<CString> excelRowValues;
		BOOL bHasValue = FALSE;

		MSXML2::IXMLDOMNodeListPtr spCellNodes;
		if (FAILED(spRowNode->selectNodes(CComBSTR(L"x:c"), &spCellNodes)) || spCellNodes == NULL)
		{
			continue;
		}

		long nCellCount = 0;
		spCellNodes->get_length(&nCellCount);
		for (long nCell = 0; nCell < nCellCount; ++nCell)
		{
			MSXML2::IXMLDOMNodePtr spCellNode;
			spCellNodes->get_item(nCell, &spCellNode);
			if (spCellNode == NULL)
			{
				continue;
			}

			CString strRef = GetNodeAttribute(spCellNode, L"r");
			strRef.MakeUpper();
			const int nCol = ColumnNameToIndex(strRef);
			if (nCol < 0)
			{
				continue;
			}

			if (nCol >= static_cast<int>(excelRowValues.size()))
			{
				excelRowValues.resize(nCol + 1);
			}

			CString strType = GetNodeAttribute(spCellNode, L"t");
			CString strRawValue;

			if (strType.CompareNoCase(_T("inlineStr")) == 0)
			{
				MSXML2::IXMLDOMNodePtr spInlineText;
				spCellNode->selectSingleNode(CComBSTR(L"x:is/x:t"), &spInlineText);
				strRawValue = GetNodeText(spInlineText);
			}
			else
			{
				MSXML2::IXMLDOMNodePtr spValueNode;
				spCellNode->selectSingleNode(CComBSTR(L"x:v"), &spValueNode);
				strRawValue = GetNodeText(spValueNode);

				if (strType.CompareNoCase(_T("s")) == 0)
				{
					unsigned long long nSharedIndex = 0;
					if (TryParseInteger(strRawValue, nSharedIndex) && nSharedIndex < sharedStrings.size())
					{
						strRawValue = sharedStrings[static_cast<size_t>(nSharedIndex)];
					}
				}
			}

			if (!TrimText(strRawValue).IsEmpty())
			{
				bHasValue = TRUE;
			}

			excelRowValues[nCol] = TrimText(strRawValue);
		}

		if (!bHasValue)
		{
			continue;
		}

		if (!bHeaderRead)
		{
			outColumns.swap(excelRowValues);
			bHeaderRead = TRUE;
			continue;
		}

		if (excelRowValues.size() < outColumns.size())
		{
			excelRowValues.resize(outColumns.size());
		}
		else if (excelRowValues.size() > outColumns.size())
		{
			excelRowValues.resize(outColumns.size());
		}

		for (size_t nCol = 0; nCol < excelRowValues.size(); ++nCol)
		{
			excelRowValues[nCol] = FormatGridValue(excelRowValues[nCol], outColumns[nCol]);
		}
		outRows.push_back(excelRowValues);
	}

	DeleteDirectoryTree(strWorkFolder);
	return bHeaderRead && !outColumns.empty();
}

BOOL CExcelXlsxHelper::LoadValueSheet(
	const CString& strFilePath,
	const std::vector<CString>& currentColumns,
	std::vector<std::vector<CString>>& outRows)
{
	return LoadSheet(strFilePath, currentColumns, outRows, 1);
}
BOOL CExcelXlsxHelper::SaveValueSheet(
	const CString& strFilePath,
	const std::vector<CString>& columns,
	const std::vector<std::vector<CString>>& rows)
{
	if (columns.empty())
	{
		return FALSE;
	}

	std::vector<int> exportColumns;
	std::vector<int> columnWidths;
	for (int nCol = 0; nCol < static_cast<int>(columns.size()); ++nCol)
	{
		if (IsExecutionColumn(columns[nCol]))
		{
			continue;
		}

		exportColumns.push_back(nCol);
		columnWidths.push_back(max(10, columns[nCol].GetLength() + 4));
	}

	if (exportColumns.empty())
	{
		return FALSE;
	}

	for (size_t nRow = 0; nRow < rows.size(); ++nRow)
	{
		for (size_t nExportCol = 0; nExportCol < exportColumns.size(); ++nExportCol)
		{
			const int nSourceCol = exportColumns[nExportCol];
			if (nSourceCol < static_cast<int>(rows[nRow].size()))
			{
				const CString strValue = TrimText(rows[nRow][nSourceCol]);
				columnWidths[nExportCol] = max(columnWidths[nExportCol], min(40, strValue.GetLength() + 4));
			}
		}
	}

	CString strContentTypes =
		_T("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>")
		_T("<Types xmlns=\"http://schemas.openxmlformats.org/package/2006/content-types\">")
		_T("<Default Extension=\"rels\" ContentType=\"application/vnd.openxmlformats-package.relationships+xml\"/>")
		_T("<Default Extension=\"xml\" ContentType=\"application/xml\"/>")
		_T("<Override PartName=\"/xl/workbook.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml\"/>")
		_T("<Override PartName=\"/xl/worksheets/sheet1.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml\"/>")
		_T("<Override PartName=\"/xl/styles.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.styles+xml\"/>")
		_T("</Types>");

	CString strPackageRels =
		_T("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>")
		_T("<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">")
		_T("<Relationship Id=\"rId1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument\" Target=\"xl/workbook.xml\"/>")
		_T("</Relationships>");

	CString strWorkbook =
		_T("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>")
		_T("<workbook xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" ")
		_T("xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\">")
		_T("<sheets><sheet name=\"ValueTest\" sheetId=\"1\" r:id=\"rId1\"/></sheets>")
		_T("</workbook>");

	CString strWorkbookRels =
		_T("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>")
		_T("<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">")
		_T("<Relationship Id=\"rId1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet\" Target=\"worksheets/sheet1.xml\"/>")
		_T("<Relationship Id=\"rId2\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles\" Target=\"styles.xml\"/>")
		_T("</Relationships>");

	CString strStyles =
		_T("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>")
		_T("<styleSheet xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\">")
		_T("<fonts count=\"5\">")
		_T("<font><sz val=\"11\"/><color theme=\"1\"/><name val=\"Calibri\"/><family val=\"2\"/></font>")
		_T("<font><sz val=\"11\"/><color rgb=\"FF0000FF\"/><name val=\"Calibri\"/><family val=\"2\"/></font>")
		_T("<font><sz val=\"11\"/><color rgb=\"FFFF0000\"/><name val=\"Calibri\"/><family val=\"2\"/></font>")
		_T("<font><sz val=\"11\"/><color rgb=\"FF003366\"/><name val=\"Calibri\"/><family val=\"2\"/></font>")
		_T("<font><sz val=\"11\"/><color rgb=\"FF990000\"/><name val=\"Calibri\"/><family val=\"2\"/></font>")
		_T("</fonts>")
		_T("<fills count=\"3\"><fill><patternFill patternType=\"none\"/></fill><fill><patternFill patternType=\"gray125\"/></fill>")
		_T("<fill><patternFill patternType=\"solid\"><fgColor rgb=\"FFD9D9D9\"/><bgColor indexed=\"64\"/></patternFill></fill></fills>")
		_T("<borders count=\"1\"><border><left/><right/><top/><bottom/><diagonal/></border></borders>")
		_T("<cellStyleXfs count=\"1\"><xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\"/></cellStyleXfs>")
		_T("<cellXfs count=\"6\">")
		_T("<xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\" xfId=\"0\"/>")
		_T("<xf numFmtId=\"0\" fontId=\"0\" fillId=\"2\" borderId=\"0\" xfId=\"0\" applyFill=\"1\" applyAlignment=\"1\"><alignment horizontal=\"center\"/></xf>")
		_T("<xf numFmtId=\"0\" fontId=\"1\" fillId=\"0\" borderId=\"0\" xfId=\"0\" applyFont=\"1\"/>")
		_T("<xf numFmtId=\"0\" fontId=\"2\" fillId=\"0\" borderId=\"0\" xfId=\"0\" applyFont=\"1\"/>")
		_T("<xf numFmtId=\"0\" fontId=\"3\" fillId=\"0\" borderId=\"0\" xfId=\"0\" applyFont=\"1\"/>")
		_T("<xf numFmtId=\"0\" fontId=\"4\" fillId=\"0\" borderId=\"0\" xfId=\"0\" applyFont=\"1\"/>")
		_T("</cellXfs><cellStyles count=\"1\"><cellStyle name=\"Normal\" xfId=\"0\" builtinId=\"0\"/></cellStyles>")
		_T("</styleSheet>");

	CString strSheet =
		_T("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>")
		_T("<worksheet xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" ")
		_T("xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\">");

	strSheet += _T("<cols>");
	for (size_t nCol = 0; nCol < exportColumns.size(); ++nCol)
	{
		CString strCol;
		const int nWidth = min(45, max(12, columnWidths[nCol]));
		strCol.Format(_T("<col min=\"%u\" max=\"%u\" width=\"%d\" customWidth=\"1\"/>"),
			static_cast<unsigned int>(nCol + 1),
			static_cast<unsigned int>(nCol + 1),
			nWidth);
		strSheet += strCol;
	}
	strSheet += _T("</cols><sheetData><row r=\"1\">");

	for (size_t nCol = 0; nCol < exportColumns.size(); ++nCol)
	{
		CString strCell;
		CString strRef;
		strRef.Format(_T("%s1"), GetExcelColumnName(static_cast<int>(nCol)).GetString());
		strCell.Format(_T("<c r=\"%s\" t=\"inlineStr\" s=\"1\"><is><t>%s</t></is></c>"),
			strRef.GetString(),
			XmlEscape(columns[exportColumns[nCol]]).GetString());
		strSheet += strCell;
	}
	strSheet += _T("</row>");

	for (size_t nRow = 0; nRow < rows.size(); ++nRow)
	{
		CString strRow;
		strRow.Format(_T("<row r=\"%u\">"), static_cast<unsigned int>(nRow + 2));
		strSheet += strRow;

		for (size_t nCol = 0; nCol < exportColumns.size(); ++nCol)
		{
			const int nSourceCol = exportColumns[nCol];
			CString strValue;
			if (nSourceCol < static_cast<int>(rows[nRow].size()))
			{
				strValue = TrimText(rows[nRow][nSourceCol]);
			}

			const CString strHeader = columns[nSourceCol];
			int nStyle = 0;
			if (IsPassCountColumn(strHeader))
			{
				nStyle = 4;
			}
			else if (IsFailCountColumn(strHeader))
			{
				nStyle = 5;
			}
			else if (IsPassText(strValue))
			{
				nStyle = 2;
			}
			else if (IsFailText(strValue))
			{
				nStyle = 3;
			}

			CString strRef;
			strRef.Format(_T("%s%u"), GetExcelColumnName(static_cast<int>(nCol)).GetString(), static_cast<unsigned int>(nRow + 2));

			CString strCell;
			if (nStyle > 0)
			{
				strCell.Format(_T("<c r=\"%s\" t=\"inlineStr\" s=\"%d\"><is><t>%s</t></is></c>"),
					strRef.GetString(), nStyle, XmlEscape(strValue).GetString());
			}
			else
			{
				strCell.Format(_T("<c r=\"%s\" t=\"inlineStr\"><is><t>%s</t></is></c>"),
					strRef.GetString(), XmlEscape(strValue).GetString());
			}
			strSheet += strCell;
		}

		strSheet += _T("</row>");
	}
	strSheet += _T("</sheetData></worksheet>");

	DeleteFile(strFilePath);
	std::vector<std::pair<CStringA, CString>> packageFiles;
	packageFiles.push_back(std::make_pair(CStringA("[Content_Types].xml"), strContentTypes));
	packageFiles.push_back(std::make_pair(CStringA("_rels/.rels"), strPackageRels));
	packageFiles.push_back(std::make_pair(CStringA("xl/workbook.xml"), strWorkbook));
	packageFiles.push_back(std::make_pair(CStringA("xl/_rels/workbook.xml.rels"), strWorkbookRels));
	packageFiles.push_back(std::make_pair(CStringA("xl/styles.xml"), strStyles));
	packageFiles.push_back(std::make_pair(CStringA("xl/worksheets/sheet1.xml"), strSheet));

	return CreateXlsxPackage(strFilePath, packageFiles) && FileExists(strFilePath);
}
