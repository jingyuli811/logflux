// Clipboard.h: interface for the Clipboard class.
#ifndef CLIPBOARD_H
#define CLIPBOARD_H

#include <string>

class Clipboard  
{
public:
	static BOOL SetTextA(HWND owner, const char* str,int len);
	static BOOL GetTextA(CString& str);
	static BOOL SetTextA(HWND owner, const char* str);		// inline

	static BOOL SetTextW(HWND owner, const wchar_t* str,int len);
	static BOOL GetTextW(wchar_t** ppwstr);
	static BOOL GetTextW(std::wstring& wstr);
	static BOOL SetTextW(HWND owner, const wchar_t* str);	// inline

};

inline BOOL Clipboard::SetTextA(HWND owner, const char* str)
{
	return SetTextA(owner,str,strlen(str));
}

inline BOOL Clipboard::SetTextW(HWND owner, const wchar_t* str)
{
	return SetTextW(owner,str, wcslen(str)*sizeof(wchar_t) ); //in bytes
}

#ifdef _UNICODE
#define SetText	SetTextW
#else
#define SetText SetTextA
#endif

#endif 
