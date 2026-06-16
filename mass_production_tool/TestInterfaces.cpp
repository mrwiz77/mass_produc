#include "pch.h"

#include "TestInterfaces.h"

namespace
{
	CString NormalizeKey(const CString& text)
	{
		CString key(text);
		key.Trim();
		key.MakeUpper();
		key.Remove(_T(' '));
		key.Remove(_T('\t'));
		key.Remove(_T('#'));
		return key;
	}

	BOOL TryParseUnsigned(const CString& text, unsigned long long maxValue, unsigned long long& value)
	{
		CString strValue(text);
		strValue.Trim();
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
				const TCHAR ch = strValue[i];
				if ((_T('A') <= ch && ch <= _T('F')) || (_T('a') <= ch && ch <= _T('f')))
				{
					nBase = 16;
					break;
				}
			}
		}

		TCHAR* pEnd = NULL;
		value = _tcstoui64(strValue, &pEnd, nBase);
		return pEnd != NULL && *pEnd == _T('\0') && value <= maxValue;
	}
}

MP_SYSTEM_TIMEOUT_CONFIG::MP_SYSTEM_TIMEOUT_CONFIG()
{
	Reset();
}

void MP_SYSTEM_TIMEOUT_CONFIG::Reset()
{
	ftmEnterNotiReceiveTimeoutMs = 0;
	bootStatusCheckCommandTimeoutMs = 0;
	testModeSettingCommandTimeoutMs = 0;
	dutVersionCheckCommandTimeoutMs = 0;
	ethernetTestCommandTimeoutMs = 0;
	modemInfoTestCommandTimeoutMs = 0;
	modemRfTestCommandTimeoutMs = 0;
	fanTestCommandTimeoutMs = 0;
	canTestCommandTimeoutMs = 0;
}

void MP_SYSTEM_TIMEOUT_CONFIG::SetTimeout(const CString& itemName, DWORD timeoutMs)
{
	const CString key = NormalizeKey(itemName);
	if (key == _T("FTMENTERNOTIRECEIVETIMEOUT"))
	{
		ftmEnterNotiReceiveTimeoutMs = timeoutMs;
	}
	else if (key == _T("BOOTSTATUSCHECKCOMMANDTIMEOUT"))
	{
		bootStatusCheckCommandTimeoutMs = timeoutMs;
	}
	else if (key == _T("TESTMODESETTINGCOMMANDTIMEOUT"))
	{
		testModeSettingCommandTimeoutMs = timeoutMs;
	}
	else if (key == _T("DUTVERSIONCHECKCOMMANDTIMEOUT"))
	{
		dutVersionCheckCommandTimeoutMs = timeoutMs;
	}
	else if (key == _T("ETHERNETTESTCOMMANDTIMEOUT"))
	{
		ethernetTestCommandTimeoutMs = timeoutMs;
	}
	else if (key == _T("MODEMINFOTESTCOMMANDTIMEOUT"))
	{
		modemInfoTestCommandTimeoutMs = timeoutMs;
	}
	else if (key == _T("MODEMRFTESTCOMMANDTIMEOUT"))
	{
		modemRfTestCommandTimeoutMs = timeoutMs;
	}
	else if (key == _T("FANTESTCOMMANDTIMEOUT"))
	{
		fanTestCommandTimeoutMs = timeoutMs;
	}
	else if (key == _T("CANTESTCOMMANDTIMEOUT"))
	{
		canTestCommandTimeoutMs = timeoutMs;
	}
}

MP_SYSTEM_CHECK_CONFIG::MP_SYSTEM_CHECK_CONFIG()
{
	Reset();
}

void MP_SYSTEM_CHECK_CONFIG::Reset()
{
	performFactoryTestModeEnterNotiCheck = FALSE;
	performBootStatusCheck = FALSE;
	mcuBootCompleteCheck = FALSE;
	apBootCompleteCheck = FALSE;
	modemBootCompleteCheck = FALSE;
}

void MP_SYSTEM_CHECK_CONFIG::SetCheck(const CString& groupName, const CString& itemName, BOOL enabled)
{
	const CString groupKey = NormalizeKey(groupName);
	const CString itemKey = NormalizeKey(itemName);
	if (groupKey == _T("FACTORYTESTMODE") &&
		itemKey.Find(_T("PERFORMFACTORY_TESTMODE")) >= 0 &&
		itemKey.Find(_T("NOTICHECK")) >= 0)
	{
		performFactoryTestModeEnterNotiCheck = enabled;
	}
	else if (groupKey == _T("BOOTSTATUSCHECK") && itemKey == _T("PERFORMBOOTSTATUSCHECK"))
	{
		performBootStatusCheck = enabled;
	}
	else if (groupKey == _T("BOOTSTATUSCHECK") &&
		itemKey.Find(_T("MCUBOOT")) >= 0 &&
		itemKey.Find(_T("CHECK")) >= 0)
	{
		mcuBootCompleteCheck = enabled;
	}
	else if (groupKey == _T("BOOTSTATUSCHECK") &&
		itemKey.Find(_T("APBOOT")) >= 0 &&
		itemKey.Find(_T("CHECK")) >= 0)
	{
		apBootCompleteCheck = enabled;
	}
	else if (groupKey == _T("BOOTSTATUSCHECK") &&
		itemKey.Find(_T("MODEMBOOT")) >= 0 &&
		itemKey.Find(_T("CHECK")) >= 0)
	{
		modemBootCompleteCheck = enabled;
	}
}

BOOL MpTryParseUInt32(const CString& text, DWORD& value)
{
	unsigned long long parsed = 0;
	if (!TryParseUnsigned(text, 0xFFFFFFFFULL, parsed))
	{
		return FALSE;
	}
	value = static_cast<DWORD>(parsed);
	return TRUE;
}

BOOL MpTryParseUInt16(const CString& text, WORD& value)
{
	unsigned long long parsed = 0;
	if (!TryParseUnsigned(text, 0xFFFFULL, parsed))
	{
		return FALSE;
	}
	value = static_cast<WORD>(parsed);
	return TRUE;
}

BOOL MpTryParseByte(const CString& text, BYTE& value)
{
	unsigned long long parsed = 0;
	if (!TryParseUnsigned(text, 0xFFULL, parsed))
	{
		return FALSE;
	}
	value = static_cast<BYTE>(parsed);
	return TRUE;
}

CString MpFormatHex32(DWORD value)
{
	CString text;
	text.Format(_T("%08X"), value);
	return text;
}

CString MpFormatDecimal(int value)
{
	CString text;
	text.Format(_T("%d"), value);
	return text;
}

int MpReadCountText(const CString& text)
{
	DWORD value = 0;
	if (!MpTryParseUInt32(text, value))
	{
		return 0;
	}
	return static_cast<int>(value);
}

BOOL MpTryGetSystemTestType(const CString& typeName, BYTE& typeValue)
{
	const CString key = NormalizeKey(typeName);
	if (key == _T("APSWVERSION"))
	{
		typeValue = MP_SYSTEM_TYPE_AP_SW_VERSION;
	}
	else if (key == _T("MICOMSWVERSION"))
	{
		typeValue = MP_SYSTEM_TYPE_MICOM_SW_VERSION;
	}
	else if (key == _T("ETHERNETSWITCHSWVERSION"))
	{
		typeValue = MP_SYSTEM_TYPE_ETHERNET_SWITCH_SW_VERSION;
	}
	else if (key == _T("MODEM1VERSIONINFO"))
	{
		typeValue = MP_SYSTEM_TYPE_MODEM_1_VERSION_INFO;
	}
	else if (key == _T("MODEM2VERSIONINFO"))
	{
		typeValue = MP_SYSTEM_TYPE_MODEM_2_VERSION_INFO;
	}
	else if (key == _T("MODEM3VERSIONINFO"))
	{
		typeValue = MP_SYSTEM_TYPE_MODEM_3_VERSION_INFO;
	}
	else if (key == _T("MODEM4VERSIONINFO"))
	{
		typeValue = MP_SYSTEM_TYPE_MODEM_4_VERSION_INFO;
	}
	else if (key == _T("MODEM5VERSIONINFO"))
	{
		typeValue = MP_SYSTEM_TYPE_MODEM_5_VERSION_INFO;
	}
	else
	{
		return FALSE;
	}
	return TRUE;
}

CSystemTestSimulInterface::CSystemTestSimulInterface()
	: m_lastTypeValue(0)
	, m_randomState(GetTickCount())
{
}

BOOL CSystemTestSimulInterface::WriteSystemRequest(BYTE typeValue)
{
	m_lastTypeValue = typeValue;
	TRACE(_T("[SIM][SYSTEM] WRITE|TYPE| = 0x%02X\n"), m_lastTypeValue);
	return TRUE;
}

BOOL CSystemTestSimulInterface::ReadSystemValue(DWORD expectedValue, DWORD& returnValue)
{
	m_randomState = m_randomState * 1103515245u + 12345u + m_lastTypeValue;
	const BOOL bPass = ((m_randomState >> 16) & 0x1) == 0;
	returnValue = bPass ? expectedValue : (expectedValue ^ 0x00000001);
	TRACE(_T("[SIM][SYSTEM] READ = 0x%08X, expected=0x%08X, result=%s\n"),
		returnValue, expectedValue, bPass ? _T("PASS") : _T("FAIL"));
	return TRUE;
}

BOOL CSystemTestSimulInterface::ReadSystemBytes(const std::vector<BYTE>& expectedBytes, std::vector<BYTE>& returnBytes)
{
	m_randomState = m_randomState * 1103515245u + 12345u + m_lastTypeValue;
	const BOOL bPass = ((m_randomState >> 16) & 0x1) == 0;
	returnBytes = expectedBytes;
	if (!bPass)
	{
		if (returnBytes.empty())
		{
			returnBytes.push_back(0x00);
		}
		else
		{
			returnBytes[returnBytes.size() - 1] ^= 0x01;
		}
	}

	TRACE(_T("[SIM][SYSTEM] READ bytes=%u, expectedBytes=%u, result=%s\n"),
		static_cast<unsigned int>(returnBytes.size()),
		static_cast<unsigned int>(expectedBytes.size()),
		bPass ? _T("PASS") : _T("FAIL"));
	return TRUE;
}

CRangeTestSimulInterface::CRangeTestSimulInterface()
	: m_lastIndexValue(0)
	, m_nextPass(TRUE)
{
}

BOOL CRangeTestSimulInterface::WriteRangeIndex(WORD indexValue)
{
	m_lastIndexValue = indexValue;
	TRACE(_T("[SIM][RANGE] WRITE|INDEX| = 0x%04X\n"), m_lastIndexValue);
	return TRUE;
}

BOOL CRangeTestSimulInterface::ReadRangeValue(DWORD minValue, DWORD maxValue, DWORD& returnValue)
{
	if (m_nextPass)
	{
		returnValue = minValue;
	}
	else
	{
		returnValue = maxValue == 0xFFFFFFFF ? minValue - 1 : maxValue + 1;
	}

	TRACE(_T("[SIM][RANGE] READ = 0x%08X, min=0x%08X, max=0x%08X, result=%s\n"),
		returnValue, minValue, maxValue, m_nextPass ? _T("PASS") : _T("FAIL"));
	m_nextPass = !m_nextPass;
	return TRUE;
}

CValueTestSimulInterface::CValueTestSimulInterface()
	: m_nextPass(TRUE)
{
}

BOOL CValueTestSimulInterface::WriteValueCommand(const std::vector<BYTE>& commandBytes)
{
	m_lastCommandBytes = commandBytes;
	TRACE(_T("[SIM][VALUE] WRITE bytes=%u\n"), static_cast<unsigned int>(m_lastCommandBytes.size()));
	return TRUE;
}

BOOL CValueTestSimulInterface::ReadValue(DWORD expectedValue, DWORD& returnValue)
{
	returnValue = m_nextPass ? expectedValue : (expectedValue ^ 0x00000001);
	TRACE(_T("[SIM][VALUE] READ = 0x%08X, expected=0x%08X, result=%s\n"),
		returnValue, expectedValue, m_nextPass ? _T("PASS") : _T("FAIL"));
	m_nextPass = !m_nextPass;
	return TRUE;
}
