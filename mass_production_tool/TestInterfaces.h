#pragma once

#include <vector>
#include <afxstr.h>

enum MP_SYSTEM_TEST_TYPE : BYTE
{
	MP_SYSTEM_TYPE_AP_SW_VERSION = 0x01,
	MP_SYSTEM_TYPE_MICOM_SW_VERSION = 0x02,
	MP_SYSTEM_TYPE_ETHERNET_SWITCH_SW_VERSION = 0x03,
	MP_SYSTEM_TYPE_MODEM_1_VERSION_INFO = 0x04,
	MP_SYSTEM_TYPE_MODEM_2_VERSION_INFO = 0x05,
	MP_SYSTEM_TYPE_MODEM_3_VERSION_INFO = 0x06,
	MP_SYSTEM_TYPE_MODEM_4_VERSION_INFO = 0x07,
	MP_SYSTEM_TYPE_MODEM_5_VERSION_INFO = 0x08
};

struct MP_SYSTEM_TIMEOUT_CONFIG
{
	DWORD ftmEnterNotiReceiveTimeoutMs;
	DWORD bootStatusCheckCommandTimeoutMs;
	DWORD testModeSettingCommandTimeoutMs;
	DWORD dutVersionCheckCommandTimeoutMs;
	DWORD ethernetTestCommandTimeoutMs;
	DWORD modemInfoTestCommandTimeoutMs;
	DWORD modemRfTestCommandTimeoutMs;
	DWORD fanTestCommandTimeoutMs;
	DWORD canTestCommandTimeoutMs;

	MP_SYSTEM_TIMEOUT_CONFIG();
	void Reset();
	void SetTimeout(const CString& itemName, DWORD timeoutMs);
};

struct MP_SYSTEM_CHECK_CONFIG
{
	BOOL performFactoryTestModeEnterNotiCheck;
	BOOL performBootStatusCheck;
	BOOL mcuBootCompleteCheck;
	BOOL apBootCompleteCheck;
	BOOL modemBootCompleteCheck;

	MP_SYSTEM_CHECK_CONFIG();
	void Reset();
	void SetCheck(const CString& groupName, const CString& itemName, BOOL enabled);
};

BOOL MpTryParseUInt32(const CString& text, DWORD& value);
BOOL MpTryParseUInt16(const CString& text, WORD& value);
BOOL MpTryParseByte(const CString& text, BYTE& value);
CString MpFormatHex32(DWORD value);
CString MpFormatDecimal(int value);
int MpReadCountText(const CString& text);
BOOL MpTryGetSystemTestType(const CString& typeName, BYTE& typeValue);

class ISystemTestInterface
{
public:
	virtual ~ISystemTestInterface() {}
	virtual BOOL WriteSystemRequest(BYTE typeValue) = 0;
	virtual BOOL ReadSystemValue(DWORD expectedValue, DWORD& returnValue) = 0;
};

class IRangeTestInterface
{
public:
	virtual ~IRangeTestInterface() {}
	virtual BOOL WriteRangeIndex(WORD indexValue) = 0;
	virtual BOOL ReadRangeValue(DWORD minValue, DWORD maxValue, DWORD& returnValue) = 0;
};

class IValueTestInterface
{
public:
	virtual ~IValueTestInterface() {}
	virtual BOOL WriteValueCommand(const std::vector<BYTE>& commandBytes) = 0;
	virtual BOOL ReadValue(DWORD expectedValue, DWORD& returnValue) = 0;
};

class CSystemTestSimulInterface : public ISystemTestInterface
{
public:
	CSystemTestSimulInterface();
	virtual BOOL WriteSystemRequest(BYTE typeValue);
	virtual BOOL ReadSystemValue(DWORD expectedValue, DWORD& returnValue);

private:
	BYTE m_lastTypeValue;
	BOOL m_nextPass;
};

class CRangeTestSimulInterface : public IRangeTestInterface
{
public:
	CRangeTestSimulInterface();
	virtual BOOL WriteRangeIndex(WORD indexValue);
	virtual BOOL ReadRangeValue(DWORD minValue, DWORD maxValue, DWORD& returnValue);

private:
	WORD m_lastIndexValue;
	BOOL m_nextPass;
};

class CValueTestSimulInterface : public IValueTestInterface
{
public:
	CValueTestSimulInterface();
	virtual BOOL WriteValueCommand(const std::vector<BYTE>& commandBytes);
	virtual BOOL ReadValue(DWORD expectedValue, DWORD& returnValue);

private:
	std::vector<BYTE> m_lastCommandBytes;
	BOOL m_nextPass;
};
