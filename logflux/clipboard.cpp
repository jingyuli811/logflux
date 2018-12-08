// Clipboard.cpp: implementation of the Clipboard class.
//
//////////////////////////////////////////////////////////////////////

#include "logflux.h"
#include "Clipboard.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

BOOL Clipboard::GetTextA(CString &str)
{
	if(!OpenClipboard(NULL))
		return FALSE;

	HANDLE hlocmem=GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE,sizeof(LCID));
	if(hlocmem)
	{
		PLCID lcid=(PLCID)GlobalLock(hlocmem);
		*lcid=MAKELCID(MAKELANGID(LANG_CHINESE,SUBLANG_CHINESE_TRADITIONAL),SORT_CHINESE_BIG5);
		GlobalUnlock(hlocmem);
		SetClipboardData(CF_LOCALE,hlocmem);
	}

	HANDLE hmem=GetClipboardData(CF_TEXT);
	if(hmem)
	{
		LPSTR data=(LPSTR)GlobalLock(hmem);
		str=data;
		GlobalUnlock(hmem);
	}
	CloseClipboard();
	return TRUE;
}

BOOL Clipboard::GetTextW(wchar_t** ppwstr)
{
	if(!OpenClipboard(NULL))
		return FALSE;

	HANDLE hmem=GetClipboardData(CF_UNICODETEXT);
	if(hmem)
	{
		wchar_t* data=(wchar_t*)GlobalLock(hmem);
		*ppwstr = new wchar_t[ wcslen(data) +1];
		wcscpy(*ppwstr, data);
		GlobalUnlock(hmem);
		CloseClipboard();
		return TRUE;
	}
	else {
		CloseClipboard();
		return FALSE;
	}
}

BOOL Clipboard::GetTextW(std::wstring& wstr)
{
	if(!OpenClipboard(NULL))
		return FALSE;

	HANDLE hmem=GetClipboardData(CF_UNICODETEXT);
	if(hmem)
	{
		wchar_t* data=(wchar_t*)GlobalLock(hmem);
		wstr.assign(data,wcslen(data));
		GlobalUnlock(hmem);
		CloseClipboard();
		return TRUE;
	}
	else {
		CloseClipboard();
		return FALSE;
	}
}

BOOL Clipboard::SetTextA(HWND owner, const char* str, int len)
{
	if(str && *str && OpenClipboard(owner))
	{
		EmptyClipboard();
		HANDLE hmem=GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE,len+1);
		HANDLE hlocmem=GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE,sizeof(LCID));
		if(hmem)
		{
			LPSTR buf=(LPSTR)GlobalLock(hmem);
			PLCID lcid=(PLCID)GlobalLock(hlocmem);
			memcpy(buf,str,len+1);
			// FIXME: Big5 should not be hard-coded since we have GB2312 version.
			*lcid=MAKELCID(MAKELANGID(LANG_CHINESE,SUBLANG_CHINESE_TRADITIONAL),SORT_CHINESE_BIG5);
			GlobalUnlock(hmem);
			GlobalUnlock(hlocmem);
			SetClipboardData(CF_TEXT,hmem);
			SetClipboardData(CF_LOCALE,hlocmem);
		}
		CloseClipboard();
		return TRUE;
	}
	return FALSE;
}

BOOL Clipboard::SetTextW(HWND owner, const wchar_t* str, int len)
{
	if(str && *str && OpenClipboard(owner))
	{
		EmptyClipboard();
		HANDLE hmem=GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE,len+4);
		if(hmem)
		{
			wchar_t* buf=(wchar_t*)GlobalLock(hmem);
			memcpy(buf,str,len+4);
			GlobalUnlock(hmem);
			SetClipboardData(CF_UNICODETEXT,hmem);
		}
		CloseClipboard();
		return TRUE;
	}
	return FALSE;
}
