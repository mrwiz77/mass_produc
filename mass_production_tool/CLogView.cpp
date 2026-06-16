#include "pch.h"
#include "CLogView.h"
//  유니
BEGIN_MESSAGE_MAP(CLogView, CRichEditCtrl)
END_MESSAGE_MAP()

namespace
{
    CLogView* g_pTraceLogView = NULL;

    void MpTrimTraceLine(CString& message)
    {
        message.TrimRight(_T("\r\n"));
    }

    LogLevel MpDetectTraceLogLevel(const CString& message)
    {
        CString upper(message);
        upper.MakeUpper();

        if (upper.Find(_T("ERR")) >= 0 ||
            upper.Find(_T("ERROR")) >= 0 ||
            upper.Find(_T("FAILED")) >= 0 ||
            upper.Find(_T("FAILURE")) >= 0 ||
            upper.Find(_T("INVALID")) >= 0 ||
            upper.Find(_T("UNKNOWN")) >= 0 ||
            upper.Find(_T("REQUIRED COLUMN IS MISSING")) >= 0)
        {
            return LogLevel::ERROR_U;
        }
        if (upper.Find(_T("WARN")) >= 0 || upper.Find(_T("WARNING")) >= 0)
        {
            return LogLevel::WARN_U;
        }
        if (upper.Find(_T("NOTI")) >= 0 || upper.Find(_T("NOTICE")) >= 0)
        {
            return LogLevel::NOTICE_U;
        }
        if (upper.Find(_T("INF")) >= 0 || upper.Find(_T("INFO")) >= 0)
        {
            return LogLevel::INFO_U;
        }
        return LogLevel::DEBUG_U;
    }

    void MpAppendTraceToLogView(const CString& message)
    {
        CString upper(message);
        upper.MakeUpper();
        if (upper.Find(_T("DRAW COUNTER TICKS")) >= 0)
        {
            return;
        }

        if (g_pTraceLogView != NULL && ::IsWindow(g_pTraceLogView->GetSafeHwnd()))
        {
            g_pTraceLogView->AppendLog(MpDetectTraceLogLevel(message), message);
        }
    }
}

void MpSetTraceLogView(CLogView* pLogView)
{
    g_pTraceLogView = pLogView;
}

void MpTrace(LPCTSTR pszFormat, ...)
{
    if (pszFormat == NULL)
    {
        return;
    }

    va_list args;
    va_start(args, pszFormat);
    CString message;
    message.FormatV(pszFormat, args);
    va_end(args);

    ::OutputDebugString(message);
    MpTrimTraceLine(message);
    MpAppendTraceToLogView(message);
}

void MpTrace(LPCSTR pszFormat, ...)
{
    if (pszFormat == NULL)
    {
        return;
    }

    va_list args;
    va_start(args, pszFormat);
    CStringA messageA;
    messageA.FormatV(pszFormat, args);
    va_end(args);

    CString message(messageA);
    ::OutputDebugString(message);
    MpTrimTraceLine(message);
    MpAppendTraceToLogView(message);
}

void MpTrace(UINT /*nCategory*/, UINT /*nLevel*/, LPCTSTR pszFormat, ...)
{
    if (pszFormat == NULL)
    {
        return;
    }

    va_list args;
    va_start(args, pszFormat);
    CString message;
    message.FormatV(pszFormat, args);
    va_end(args);

    ::OutputDebugString(message);
    MpTrimTraceLine(message);
    MpAppendTraceToLogView(message);
}

void MpTrace(UINT /*nCategory*/, UINT /*nLevel*/, LPCSTR pszFormat, ...)
{
    if (pszFormat == NULL)
    {
        return;
    }

    va_list args;
    va_start(args, pszFormat);
    CStringA messageA;
    messageA.FormatV(pszFormat, args);
    va_end(args);

    CString message(messageA);
    ::OutputDebugString(message);
    MpTrimTraceLine(message);
    MpAppendTraceToLogView(message);
}

void CLogView::AppendLog(LogLevel level, const CString& message)
{
    // 최대 길이 초과 시 FIFO로 한 줄씩 제거
    int totalLen = GetTextLength();  // 문자의 개수
    SetFontSize(10);
    while (totalLen > MAX_TEXT_LENGTH)
    {
        long firstLineStart = LineIndex(0);
        long secondLineStart = LineIndex(1);
        if (secondLineStart > firstLineStart)
        {
            SetSel(firstLineStart, secondLineStart);
            ReplaceSel(_T(""));
        }
        else
        {
            break; // 삭제 불가능 상태 → 빠져나감
        }
        totalLen = GetTextLength();
    }

    // 로그 포맷 생성
    CTime time = CTime::GetCurrentTime();
    //CString timeStr = time.Format(_T("[%Y-%m-%d %H:%M:%S] "));
    CString timeStr = time.Format(_T("%Y-%m-%d %H:%M:%S "));

    CString levelStr;
    CHARFORMAT2 cf;
    cf.cbSize = sizeof(cf);
    cf.dwMask = CFM_COLOR;
    cf.dwEffects = 0;

    switch (level)
    {
    case LogLevel::DEBUG_U:  levelStr = _T("DBG ");  cf.crTextColor = RGB(0, 0, 0); break;
    case LogLevel::INFO_U:   levelStr = _T("INF ");   cf.crTextColor = RGB(0, 128, 0); break;
    case LogLevel::NOTICE_U: levelStr = _T("NOTI "); cf.crTextColor = RGB(0, 0, 255); break;
    case LogLevel::WARN_U:   levelStr = _T("WARN ");   cf.crTextColor = RGB(255, 165, 0); break;
    case LogLevel::ERROR_U:  levelStr = _T("ERR ");  cf.crTextColor = RGB(255, 0, 0); break;
    }

    CString fullMessage = timeStr + levelStr + message + _T("\r\n");

    // 추가
    SetSel(-1, -1);
    SetSelectionCharFormat(cf);
    ReplaceSel(fullMessage);

    // 스크롤
    SetSel(-1, -1);
    SendMessage(WM_VSCROLL, SB_BOTTOM, 0);
}
void CLogView::SetFontSize(int nPointSize)
{
    CHARFORMAT2 cf;
    cf.cbSize = sizeof(CHARFORMAT2);
    cf.dwMask = CFM_SIZE;
    cf.dwEffects = 0;

    // RichEdit는 1pt = 20 twips
    cf.yHeight = nPointSize * 20;

    // 전체 텍스트에 적용하기 위해 전체 선택
    SetSel(0, -1);
    SetSelectionCharFormat(cf);

    // 선택 해제
    SetSel(-1, -1);
}

void CLogView::AppendLogMini(LogLevel level, const CString& message)
{
    // 최대 길이 초과 시 FIFO로 한 줄씩 제거
    int totalLen = GetTextLength();  // 문자의 개수
    SetFontSize(10);
    while (totalLen > MAX_TEXT_LENGTH)
    {
        long firstLineStart = LineIndex(0);
        long secondLineStart = LineIndex(1);
        if (secondLineStart > firstLineStart)
        {
            SetSel(firstLineStart, secondLineStart);
            ReplaceSel(_T(""));
        }
        else
        {
            break; // 삭제 불가능 상태 → 빠져나감
        }
        totalLen = GetTextLength();
    }
      
    // 로그 포맷 생성
    CTime time = CTime::GetCurrentTime();
   // CString timeStr = time.Format(_T("[%Y-%m-%d %H:%M:%S] "));
    CString timeStr = time.Format(_T("[%d %H:%M:%S] "));

    CString levelStr;
    CHARFORMAT2 cf;
    cf.cbSize = sizeof(cf);
    cf.dwMask = CFM_COLOR;
    cf.dwEffects = 0;

    switch (level)
    {
    case LogLevel::DEBUG_U:  levelStr = _T("[DEBUG] ");  cf.crTextColor = RGB(0, 0, 0); break;
    case LogLevel::INFO_U:   levelStr = _T("[INFO] ");   cf.crTextColor = RGB(0, 128, 0); break;
    case LogLevel::NOTICE_U: levelStr = _T("[NOTICE] "); cf.crTextColor = RGB(0, 0, 255); break;
    case LogLevel::WARN_U:   levelStr = _T("[WARN] ");   cf.crTextColor = RGB(255, 165, 0); break;
    case LogLevel::ERROR_U:  levelStr = _T("[ERROR] ");  cf.crTextColor = RGB(255, 0, 0); break;
    }

    //CString fullMessage = timeStr + levelStr + message + _T("\r\n");
    CString fullMessage = levelStr + message + _T("\r\n");

    // 추가
    SetSel(-1, -1);
    SetSelectionCharFormat(cf);
    ReplaceSel(fullMessage);

    // 스크롤
    SetSel(-1, -1);
    SendMessage(WM_VSCROLL, SB_BOTTOM, 0);
}
