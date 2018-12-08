#include "systemutils.h"
#include <tchar.h>
#include <stdio.h>
#include <Shlobj.h>
#include <fstream>
#include <iostream>
#include <time.h>

#ifdef _UNICODE
#define tstring std::wstring
#else
#define tstring std::string
#endif

namespace vi_systemutils {

// ------------------------------------------------------------------
const char* GetCurrentWorkingDirA()
{
  static char dir[MAX_PATH] = {0};

  if( !dir[0] )
    ::GetCurrentDirectoryA(MAX_PATH, dir);
  
  return dir;
}

// ------------------------------------------------------------------
const wchar_t* GetCurrentWorkingDirW()
{
  static wchar_t dir[MAX_PATH] = {0};

  if( !dir[0] )
    ::GetCurrentDirectoryW(MAX_PATH, dir);

  return dir;
}

// ------------------------------------------------------------------
const char* GetAppLocationA()
{
  static char fpath[MAX_PATH] = {0};

  if(!fpath[0])
  {
    ::GetModuleFileNameA(NULL, fpath, MAX_PATH-1);
    char* pch = strrchr(fpath, '\\');
    if(pch)
      *pch = '\0';
  }

  return fpath;
}

// ------------------------------------------------------------------
const wchar_t* GetAppLocationW()
{
  static wchar_t fpath[MAX_PATH] = {0};

  if(!fpath[0])
  {
    ::GetModuleFileNameW(NULL, fpath, MAX_PATH-1);
    wchar_t* pch = wcsrchr(fpath, L'\\');
    if(pch)
      *pch = L'\0';
  }

  return fpath;
}

// ------------------------------------------------------------------
HWND GetConsoleHwnd()
{
  #define MY_BUFSIZE 1024 // Buffer size for console window titles.
  HWND hwndFound;         // This is what is returned to the caller.
  TCHAR pszNewWindowTitle[MY_BUFSIZE]; // Contains fabricated
  // WindowTitle.
  TCHAR pszOldWindowTitle[MY_BUFSIZE]; // Contains original
  // WindowTitle.

  // Fetch current window title.
  GetConsoleTitle(pszOldWindowTitle, MY_BUFSIZE);

  // Format a "unique" NewWindowTitle.
  _stprintf_s(pszNewWindowTitle, _countof(pszNewWindowTitle), _T("%d/%d"),
    GetTickCount(),
    GetCurrentProcessId());

  // Change current window title.
  SetConsoleTitle(pszNewWindowTitle);

  // Ensure window title has been updated.
  Sleep(40);

  // Look for NewWindowTitle.
  hwndFound=FindWindow(NULL, pszNewWindowTitle);

  // Restore original window title.
  SetConsoleTitle(pszOldWindowTitle);

  return(hwndFound);
}


//----------------------------------------------------------
std::string ExtractResource( const wchar_t* lpszResName, int nResourceId )
{
  HGLOBAL hResourceLoaded;		// handle to loaded resource 
  HRSRC hRes;						// handle/ptr. to res. info. 
  void* lpResLock;				// pointer to resource data 
  DWORD dwSizeRes;

  HMODULE hMod = NULL;//GetModuleHandle(NULL);

  // find location of the resource and get handle to it
  hRes = FindResourceW( hMod, MAKEINTRESOURCEW(nResourceId), lpszResName );

  if( hRes == NULL )
  {
    return "";
  }

  // loads the specified resource into global memory. 
  hResourceLoaded = LoadResource( hMod, hRes ); 
  if( !hResourceLoaded )
  {
    return "";
  }

  // get a pointer to the loaded resource!
  lpResLock = LockResource( hResourceLoaded ); 

  if( !lpResLock )
  {
    return "";
  }
  // determine the size of the resource, so we know how much to write out to file!  
  dwSizeRes = SizeofResource( hMod, hRes );

  std::string ret(static_cast<char*>(lpResLock), dwSizeRes);

  UnlockResource( hResourceLoaded );
  FreeResource( hResourceLoaded );

  return ret;

}

//----------------------------------------------------------
void ExtractBinResource( const TCHAR* p_sResName, 
                         int nResourceId, 
                         const TCHAR* p_sOutputDir, 
                         const TCHAR* p_sOutputName )
{
  HGLOBAL hResourceLoaded;		// handle to loaded resource 
  HRSRC hRes;						// handle/ptr. to res. info. 
  char *lpResLock;				// pointer to resource data 
  DWORD dwSizeRes;
  tstring strOutputLocation;
  tstring strAppLocation; 

  // lets get the app location
  strAppLocation = GetAppLocation();
  strOutputLocation = strAppLocation;
  strOutputLocation += _T("\\");
  strOutputLocation += p_sOutputDir;
  strOutputLocation += _T("\\");

  ::SHCreateDirectoryEx(NULL, strOutputLocation.c_str(), NULL);

  strOutputLocation += p_sOutputName;

  tstring strDllPath = strAppLocation + _T("\\npcs.dll");
  HMODULE hMod = GetModuleHandle(strDllPath.c_str());
  if( !hMod )
  {
    return;
  }

  // find location of the resource and get handle to it
  hRes = FindResource( hMod, MAKEINTRESOURCE(nResourceId), p_sResName );

  if( hRes == NULL )
  {
    return;
  }

  // loads the specified resource into global memory. 
  hResourceLoaded = LoadResource( hMod, hRes ); 

  if( !hResourceLoaded )
  {
    return;
  }

  // get a pointer to the loaded resource!
  lpResLock = (char*)LockResource( hResourceLoaded ); 

  if( !lpResLock )
  {
    return;
  }
  // determine the size of the resource, so we know how much to write out to file!  
  dwSizeRes = SizeofResource( hMod, hRes );

  std::ofstream outputFile(strOutputLocation.c_str(), std::ios::binary);
  outputFile.write((const char*)lpResLock, dwSizeRes);
  outputFile.close();

  UnlockResource( hResourceLoaded );
  FreeResource( hResourceLoaded );

}

//----------------------------------------------------------
tm GetDateTime( time_t timeToConv ) 
{
  struct tm tmTime;
#ifdef WIN32
  errno_t res = localtime_s(&tmTime, &timeToConv);
  if (res != 0) {
    char buff[1024];
    _snprintf_s(buff, 1024, _TRUNCATE, "Invalid argument to localtime_s: %lld, code: %d.", timeToConv, res);
  }
#else
  tmTime = *localtime(&timeToConv);
#endif    
  return tmTime;
}

} // namespace vi_systemutils