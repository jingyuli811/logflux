#ifndef STRING_UTILS_HPP
#define STRING_UTILS_HPP

#include <vector>
#include <sstream>
#include <stdio.h>

// ------------------------------------------------------------------
/// const cast
template<class chartype>
inline chartype* ConstCast(const chartype* s) { return const_cast<chartype*>(s); }

// ------------------------------------------------------------------
template<class stringtype>
stringtype& StringTrim(stringtype& s, const stringtype& trim)
{
  // trim right
  s.erase(s.find_last_not_of(trim)+1);
  // trim left
  s.erase(0,s.find_first_not_of(trim));
  return s;
}


// ------------------------------------------------------------------
template<class chartype>
void StringTrim(chartype** s, const chartype trim)
{
  if( !s || !(*s) || !trim )
    return;

  chartype* begin = *s;
  while(begin)
  {
    if( *begin != trim )
      break;
    ++begin;
  }

  chartype* end = *s + strlen(*s) - 1;
  while(end)
  {
    if( *end != trim )
      break;
    --end;
  }

  if( begin <= end )
  {
    *(end+1) = '\0';
    *s = begin;
  }
}

// ------------------------------------------------------------------
template<class stringtype>
stringtype StringFind(const stringtype& s, const stringtype& d1, const stringtype& d2)
{
  size_t b = s.find(d1);
  if( b == stringtype::npos )
    return "";

  b += d1.length();

  size_t e = s.find(d2, b);
  if( e != stringtype::npos )
    return s.substr(b, e-b);

  return s.substr(b);
}

// ------------------------------------------------------------------
template<class stringtype>
stringtype StringFind(const stringtype s, 
                      const typename stringtype::value_type* d1, 
                      const typename stringtype::value_type* d2)
{
  return StringFind(s, stringtype(d1), stringtype(d2));
}

// ------------------------------------------------------------------
template<class chartype>
bool StringCopy(chartype* p_pcDest, size_t p_nDestLen, const chartype* p_pcSrc)
{
  if( _tcslen(p_pcSrc)  >= p_nDestLen )
    return false;
  _tcscpy(p_pcDest, p_pcSrc);

  return true;
}

// ------------------------------------------------------------------
template<>
inline bool StringCopy<>(wchar_t* p_pcDest, size_t p_nDestLen, const wchar_t* p_pcSrc)
{
  if( wcslen(p_pcSrc)  >= p_nDestLen )
    return false;
  wcscpy_s(p_pcDest, p_nDestLen, p_pcSrc);

  return true;
}

// ------------------------------------------------------------------
template<>
inline bool StringCopy<>(char* p_pcDest, size_t p_nDestLen, const char* p_pcSrc)
{
  if( strlen(p_pcSrc)  >= p_nDestLen )
    return false;
  strcpy_s(p_pcDest, p_nDestLen, p_pcSrc);

  return true;
}

// ------------------------------------------------------------------
template<class chartype>
bool StringCat(chartype* p_pcDest, size_t p_nDestLen, const chartype* p_pcSrc)
{
  if( _tcslen(p_pcDest) + _tcslen(p_pcSrc)  >= p_nDestLen )
    return false;
  _tcscat_s(p_pcDest, p_nDestLen, p_pcSrc);

  return true;
}

// ------------------------------------------------------------------
template<>
inline bool StringCat<>(wchar_t* p_pcDest, size_t p_nDestLen, const wchar_t* p_pcSrc)
{
  if( wcslen(p_pcDest) + wcslen(p_pcSrc)  >= p_nDestLen )
    return false;
  wcscat_s(p_pcDest, p_nDestLen, p_pcSrc);

  return true;
}

// ------------------------------------------------------------------
template<>
inline bool StringCat<>(char* p_pcDest, size_t p_nDestLen, const char* p_pcSrc)
{
  if( strlen(p_pcDest) + strlen(p_pcSrc)  >= p_nDestLen )
    return false;
  strcat_s(p_pcDest, p_nDestLen, p_pcSrc);

  return true;
}

// ------------------------------------------------------------------
inline int IntLength(int n)
{
  int len = ( n < 0 ? 1 : 0 );
  for( ; (n/=10) && (++len);  ){}
  return ++len;
}

// ------------------------------------------------------------------
template<class chartype>
bool StringCat(chartype* p_pwcDest, size_t p_nDestLen, int p_nSrc)
{
  const int srclen = IntLength(p_nSrc);
  if( _tcslen(p_pwcDest) + srclen  >= p_nDestLen )
    return false;

  // allocation on stack, no free should be called!!!
  chartype* srcbuf = (chartype*)_alloca((srclen+1)*sizeof(chartype));
  _stprintf(srcbuf, _T("%d"), p_nSrc);
  _tcscat(p_pwcDest, srcbuf);

  return true;
}

// ------------------------------------------------------------------
template<>
inline bool StringCat<>(wchar_t* p_pwcDest, size_t p_nDestLen, int p_nSrc)
{
  const int srclen = IntLength(p_nSrc);
  if( wcslen(p_pwcDest) + srclen  >= p_nDestLen )
    return false;

  // allocation on stack, no free should be called!!!
  wchar_t* srcbuf = (wchar_t*)_alloca((srclen+1)*sizeof(wchar_t));
  swprintf_s(srcbuf, srclen, L"%d", p_nSrc);
  wcscat_s(p_pwcDest, p_nDestLen, srcbuf);

  return true;
}

// ------------------------------------------------------------------
template<>
inline bool StringCat<>(char* p_pwcDest, size_t p_nDestLen, int p_nSrc)
{
  const int srclen = IntLength(p_nSrc);
  if( strlen(p_pwcDest) + srclen  >= p_nDestLen )
    return false;

  // allocation on stack, no free should be called!!!
  char* srcbuf = (char*)_alloca((srclen+1)*sizeof(char));
  sprintf_s(srcbuf, srclen, "%d", p_nSrc);
  strcat_s(p_pwcDest, p_nDestLen, srcbuf);

  return true;
}

// ------------------------------------------------------------------
template<class stringtype>
int StringToInt(const stringtype& s)
{
  return _ttoi( s.c_str() );
}

template<class chartype>
int StringToInt(const chartype* s)
{
  return _ttoi( s );
}

template<>
inline int StringToInt<>(const char* s)
{
  return atoi( s );
}

// ------------------------------------------------------------------
template<class stringtype>
std::vector<stringtype> StringToStrArray(const stringtype& s)
{
  std::vector<stringtype> ret;
  stringtype tmp;
  for( size_t i = 0; i < s.length(); i++ )
  {
    if( s[i] == ',' )
    {
      ret.push_back(tmp);
      tmp = "";
    }
    else
    {
      tmp.push_back(s[i]);
    }
  }

  if( tmp.length() )
    ret.push_back( tmp );

  return ret;
}


// ------------------------------------------------------------------
template<class stringtype>
std::vector<int> StringToIntArray(const stringtype& s)
{
  std::vector<int> ret;
  stringtype tmp;
  stringtype::value_type* pVal = (stringtype::value_type*)(s.c_str());
  int start = -1, end = -1;
  for( size_t i = 0; i < s.length(); i++ )
  {
    if( pVal[i] == ',' )
    {
      end = StringToInt( tmp );

      if( start == -1 )
      {
        ret.push_back( end );
      }
      else
      {
        for( int p = start; p <= end; p++ )
          ret.push_back( p );
      }

      tmp = "";
      start = end = -1;
      continue;
    }
    else if( pVal[i] == '>' )
    {
      start = atoi( tmp.c_str() );
      end = start;
      tmp = "";
    }

    if( pVal[i] >= '0' && pVal[i] <= '9' )
      tmp.push_back(pVal[i]);
  }

  if( tmp.length() )
    end = atoi(tmp.c_str());
  if( start == -1 )
  {
    ret.push_back( end );
  }
  else
  {
    for( int p = start; p <= end; p++ )
      ret.push_back( p );
  }

  return ret;
}

#endif // STRING_UTILS_HPP