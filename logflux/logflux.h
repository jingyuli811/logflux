#ifndef LOGFLUX_H
#define LOGFLUX_H

#include "resource.h"

// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//
// The following macros define the minimum required platform.  The minimum required platform
// is the earliest version of Windows, Internet Explorer etc. that has the necessary features to run 
// your application.  The macros work by enabling all features available on platform versions up to and 
// including the version specified.

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER                          // Specifies that the minimum required platform is Windows Vista.
#define WINVER 0x0600           // Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT            // Specifies that the minimum required platform is Windows Vista.
#define _WIN32_WINNT 0x0600     // Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINDOWS          // Specifies that the minimum required platform is Windows 98.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE                       // Specifies that the minimum required platform is Internet Explorer 7.0.
#define _WIN32_IE 0x0700        // Change this to the appropriate value to target other versions of IE.
#endif

#include "Resource.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include <shlobj.h>
#include <shlguid.h>
#include <shellapi.h>
#include <Shobjidl.h>

// ATL
#include <atltypes.h>

// WTL   
#define _WTL_NO_CSTRING		// disable WTL::CString
#define _WTL_NO_WTYPES		// disable WTL::CSize, CPoint, and CRect

#include <atlstr.h>			  // use ATL::CString 
#include <atlbase.h>
#include <WTL/include/atlapp.h>
#include "WTL/include/atlmisc.h"
#include "WTL/include/atlres.h"

extern CAppModule _Module;

#include <atlwin.h>
#include "WTL/include/atlsplit.h"
#include "WTL/include/atldlgs.h"
#include "WTL/include/atlcrack.h"


// Ctrl constants
const int DEF_CTRL_HEIGHT = 20;
const int DEF_CTRL_WIDTH = 80;
const int DEF_CTRL_PADDING = 3;
const int DEF_SEARCH_BAR_HEIGHT = 25;

// default config
#define DEFAULT_HOST  _T("127.0.0.1")
#define DEFAULT_PORT  _T("666")
const int DEFAULT_BUFFER_SIZE = 512;
const int MAX_BUFFER_SIZE = 4096;
const int SETP_BUFFER_SIZE = 512;

#define UDP_RECEIVE              WM_APP+1
#define WM_DROP_FILES           WM_APP+2
#define WM_COMBOMOUSEACTIVATE   (WM_APP + 10)


// output debug string helper
void DbgPrint(const char * fmt, ...);
void DbgPrint(const wchar_t * fmt, ...);

#if !defined(_DEBUG)
#define Dbgp 1?((void)(NULL)):DbgPrint
#else
#define Dbgp DbgPrint
#endif

#define ENABLE_BUFFER_SIZE_CONFIG

#define EDIT_MENU_POSITION 1
#define WINDOW_MENU_POSITION 4

#ifdef _UNICODE
#define tstring std::wstring
#else
#define tstring std::string
#endif


#endif // LOGFLUX_H
