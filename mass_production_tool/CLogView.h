#pragma once
#include <afxrich.h>  // CRichEditCtrl 사용
#include <ctime>
//
enum class LogLevel
{
    DEBUG_U,
    INFO_U,
    NOTICE_U,
    WARN_U,
    ERROR_U
};

class CLogView : public CRichEditCtrl
{
public:
    void AppendLog(LogLevel level, const CString& message);
    void AppendLogMini(LogLevel level, const CString& message);

    // ★ 폰트 크기 변경 함수 추가
    void SetFontSize(int nPointSize);
private:
    static constexpr int MAX_TEXT_LENGTH = 1500000000;  // 약 1.5GB

protected:
    DECLARE_MESSAGE_MAP()
};
