#ifndef STRING_CONV_HPP
#define STRING_CONV_HPP

#include <Windows.h>
#include <string>

#ifdef _UNICODE
#define AToT AToW
#define TToA WToA
#define WToT WToW
#define TToW WToW
#else
#define AToT AToA
#define TToA AToA
#define WToT WToA
#define TToW AToW
#endif

class CStringConv
{
public:
  CStringConv(){}
  ~CStringConv(){}

  // ------------------------------------------------------------------
  /// utf8 to big5 string
  static std::string Utf8ToBig5(const char* lpMultiByteStr)
  {
    // utf8 to unicode
    std::wstring uni = CStringConv::AToW(lpMultiByteStr, CP_UTF8);

    // unicode to big5
    return CStringConv::WToA(uni.c_str());
  }

  // ------------------------------------------------------------------
  /// big5 to utf8 string
  static std::string Big5ToUtf8(const char* lpMultiByteStr)
  {
    // big5 to unicode
    std::wstring uni = CStringConv::AToW(lpMultiByteStr);

    // unicode to utf8
    return CStringConv::WToA(uni.c_str(), CP_UTF8);
  }

  // ------------------------------------------------------------------
  /// MultiByte to WideChar, default codepage is CP_OEMCP
  /// NOTE: lpMultiByteStr must be null-terminated
  static std::wstring AToW(const char* lpMultiByteStr, UINT CodePage = CP_OEMCP)
  {
    // buffer size (include null terminated )
    int dwWCharReq = MultiByteToWideChar(CodePage, 0, lpMultiByteStr, -1, NULL, 0);

    // convert
    wchar_t* pWCharBuf = (wchar_t*)_alloca(dwWCharReq*sizeof(wchar_t));
    if( pWCharBuf )
      MultiByteToWideChar(CodePage, 0, lpMultiByteStr, -1, pWCharBuf, dwWCharReq);

    return std::wstring(pWCharBuf);
  }

  // ------------------------------------------------------------------
  /// WideChar to MultiByte, default codepage is CP_OEMCP
  /// NOTE: lpWideCharStr must be null-terminated
  static std::string WToA(const wchar_t*  lpWideCharStr, UINT CodePage = CP_OEMCP)
  {
    // buffer size (include null terminated )
    int dwCharReq = WideCharToMultiByte(CodePage, 0, lpWideCharStr, -1, NULL, 0, NULL, FALSE);

    // convert
    char* pCharBuf = (char*)_alloca(dwCharReq*sizeof(char));
    if(pCharBuf)
      WideCharToMultiByte(CodePage, 0, lpWideCharStr, -1, pCharBuf, dwCharReq, NULL, FALSE);

    return std::string(pCharBuf);
  }

  // ------------------------------------------------------------------
  /// A2A, no changes needed
  static inline const std::string& AToA(const std::string& s) { return s; }

  // ------------------------------------------------------------------
  /// W2W, no changes needed
  static inline const std::wstring& WToW(const std::wstring& s) { return s; }

};

#endif // STRING_CONV_HPP