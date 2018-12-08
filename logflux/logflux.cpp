// logflux.cpp : Defines the entry point for the application.
//

#include "logflux.h"
#include "socket.h"
#include "config.h"
#include "WTL/include/atlframe.h"
#include "WTL/include/atlctrls.h"
#include "WTL/include/atldlgs.h"
#include "WTL/include/atlctrlw.h"
#include "mainframe.h"

CAppModule _Module;

class CThreadManager
{
public:
    // thread init param
    struct _RunData
    {
        LPTSTR lpstrCmdLine;
        int nCmdShow;
    };

    // thread proc
    static DWORD WINAPI RunThread(LPVOID lpData)
    {
        CMessageLoop theLoop;
        _Module.AddMessageLoop(&theLoop);

        _RunData* pData = (_RunData*)lpData;
        CMainFrame wndFrame;

        if (wndFrame.CreateEx() == NULL)
        {
            ATLTRACE(_T("Frame window creation failed!\n"));
            return 0;
        }

        wndFrame.ShowWindow(pData->nCmdShow);
        ::SetForegroundWindow(wndFrame);	// Win95 needs this
        delete pData;

        int nRet = theLoop.Run();

        _Module.RemoveMessageLoop();
        return nRet;
    }

    DWORD m_dwCount;
    HANDLE m_arrThreadHandles[MAXIMUM_WAIT_OBJECTS - 1];

    CThreadManager() : m_dwCount(0)
    { }

    // Operations
    DWORD AddThread(LPTSTR lpstrCmdLine, int nCmdShow)
    {
        if (m_dwCount == (MAXIMUM_WAIT_OBJECTS - 1))
        {
            ::MessageBox(NULL, _T("ERROR: Cannot create ANY MORE threads!!!"), _T("WTLExplorer"), MB_OK);
            return 0;
        }

        _RunData* pData = new _RunData;
        pData->lpstrCmdLine = lpstrCmdLine;
        pData->nCmdShow = nCmdShow;
        DWORD dwThreadID;
        HANDLE hThread = ::CreateThread(NULL, 0, RunThread, pData, 0, &dwThreadID);
        if (hThread == NULL)
        {
            ::MessageBox(NULL, _T("ERROR: Cannot create thread!!!"), _T("WTLExplorer"), MB_OK);
            return 0;
        }

        m_arrThreadHandles[m_dwCount] = hThread;
        m_dwCount++;
        return dwThreadID;
    }

    void RemoveThread(DWORD dwIndex)
    {
        if (dwIndex != (m_dwCount - 1))
            m_arrThreadHandles[dwIndex] = m_arrThreadHandles[m_dwCount - 1];
        m_dwCount--;
    }

    int Run(LPTSTR lpstrCmdLine, int nCmdShow)
    {
        MSG msg;
        // force message queue to be created
        ::PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);

        AddThread(lpstrCmdLine, nCmdShow);

        int nRet = m_dwCount;
        DWORD dwRet;
        while (m_dwCount > 0)
        {
            dwRet = ::MsgWaitForMultipleObjects(m_dwCount, m_arrThreadHandles, FALSE, INFINITE, QS_ALLINPUT);

            if (dwRet == 0xFFFFFFFF)
                ::MessageBox(NULL, _T("ERROR: Wait for multiple objects failed!!!"), _T("WTLExplorer"), MB_OK);
            else if (dwRet >= WAIT_OBJECT_0 && dwRet <= (WAIT_OBJECT_0 + m_dwCount - 1))
                RemoveThread(dwRet - WAIT_OBJECT_0);
            else if (dwRet == (WAIT_OBJECT_0 + m_dwCount))
            {
                ::GetMessage(&msg, NULL, 0, 0);
                if (msg.message == WM_USER)
                    AddThread(_T(""), SW_SHOWNORMAL);
                else
                    ::MessageBeep((UINT)-1);
            }
            else
                ::MessageBeep((UINT)-1);
        }

        return nRet;
    }
};

int Run(LPTSTR /*lpstrCmdLine*/ = NULL, int nCmdShow =
#ifndef _WIN32_WCE
    SW_SHOWDEFAULT
#else // _WIN32_WCE
    SW_SHOWNORMAL
#endif // _WIN32_WCE
)
{
    theConfig().LoadXml();

    if (!SocketInitialize())
    {
        MessageBoxA(NULL, SocketError(), "logflux", MB_OK | MB_ICONERROR);
        return 0;
    }

    ::CoInitialize(NULL);
    ::OleInitialize(NULL);

    CMessageLoop theLoop;
    _Module.AddMessageLoop(&theLoop);

    CMainFrame wndMain;

    if (wndMain.CreateEx() == NULL)
    {
        ATLTRACE(_T("Main window creation failed!\n"));
        return 0;
    }


    wndMain.ShowWindow(nCmdShow);

    int nRet = theLoop.Run();

    _Module.RemoveMessageLoop();

    ::CoUninitialize();
    ::OleUninitialize();

    SocketFinalize();

    return nRet;
}

int APIENTRY _tWinMain(HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPTSTR    lpCmdLine,
    int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    HRESULT hRes = _Module.Init(NULL, hInstance);
    ATLASSERT(SUCCEEDED(hRes));

    //CThreadManager mgr;
    int nRet = Run(lpCmdLine, nCmdShow);

    _Module.Term();

    return nRet;
}

//----------------------------------------------------------
void DbgPrint(const char * fmt, ...)
{
    static char szData[1024] = { 0 };

    va_list args;
    va_start(args, fmt);
    _vsnprintf(szData, _countof(szData) - 1, fmt, args);
    va_end(args);

    OutputDebugStringA(szData);
}

//----------------------------------------------------------
void DbgPrint(const wchar_t * fmt, ...)
{
    static wchar_t szData[1024] = { 0 };

    va_list args;
    va_start(args, fmt);
    _vsnwprintf(szData, _countof(szData) - 1, fmt, args);
    va_end(args);

    OutputDebugStringW(szData);
}
