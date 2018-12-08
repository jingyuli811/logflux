#ifndef SYSTEM_UTILS
#define SYSTEM_UTILS

#include <Windows.h>
#include <string>

namespace vi_systemutils {

/// get current working directory, NOT necessarily be same as the application directory
/// if you want to get where the module is located, see the following function GetAppLocationA/W
const char* GetCurrentWorkingDirA();
const wchar_t* GetCurrentWorkingDirW();

/// get current application directory
const char* GetAppLocationA();
const wchar_t* GetAppLocationW();

/// get console HWND
HWND GetConsoleHwnd();

std::string ExtractResource( const wchar_t* lpszResName, int nResourceId );

void ExtractBinResource( const TCHAR* p_sResName, 
                         int nResourceId, 
                         const TCHAR* p_sOutputDir, 
                         const TCHAR* p_sOutputName );

tm GetDateTime( time_t timeToConv );


} //  namespace vi_systemutils 

#ifdef _UNICODE
#define GetAppLocation vi_systemutils::GetAppLocationW
#define GetCurrentWorkingDir vi_systemutils::GetCurrentWorkingDirW
#else
#define GetAppLocation vi_systemutils::GetAppLocationA
#define GetCurrentWorkingDir vi_systemutils::GetCurrentWorkingDirA
#endif

#endif