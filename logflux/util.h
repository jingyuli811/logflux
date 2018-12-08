#ifndef UTIL_H
#define UTIL_H

#include "WTL/include/atlctrls.h"
#include <tchar.h>
#include <windows.h>
#include <stdio.h>
#include <stdarg.h>

TCHAR* stristr(TCHAR* szStringToBeSearched, const TCHAR* szSubstringToSearchFor);

void GetPopupMenuPos(CListViewCtrl& lv, CPoint& point);


#ifdef _DEBUG
static void __dbg_printf (const char * format,...)
{
#define MAX_DBG_MSG_LEN (1024)
    char buf[MAX_DBG_MSG_LEN] = ("");
    va_list ap;

    va_start(ap, format);

    _vsnprintf_s(buf, _countof(buf), sizeof(buf), format, ap);
    OutputDebugStringA(buf);

    va_end(ap);
}
#define DBG __dbg_printf
#else
static void __dbg_printf (const char * format,...) {}
#define DBG  1?((void)(NULL)):__dbg_printf
#endif

#define _FLP_ "%s:%d: "
#define _FL_ __FUNCTION__, __LINE__

#define _FFLP_ "%s:" _FLP_
#define _FFL_ __FILE__, _FL_

// For example:
// DBG(_FLP_ "format string", _FL_, arg1, arg2);
// DBG(_FFLP_ "format string", _FFL, arg1, arg2);


#endif