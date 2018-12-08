#ifndef TAB_DATA_H
#define TAB_DATA_H

#include <tchar.h>

enum LOG_LEVEL
{
  LOG_NONE = -1,
  LOG_DEBUG = 0,
  LOG_INFO,
  LOG_WARN,
  LOG_ERROR,
  LOG_LEVEL_COUNT,
};

enum COLOR_TYPE
{
  COLOR_FRONT,
  COLOR_BACK,
  COLOR_TYPE_COUNT,
};

enum FILTER_STATE
{
  FILTERED_NO = 0,
  FILTERED_YES,
  FILTERED_HILIGHT,
};

struct TFilterList
{
public:
  ~TFilterList()
  {
    for( size_t i=0; i<m_list.size(); i++)
    {
      delete m_list[i];
    }
    m_list.clear();
  }

  size_t Size() const
  {
    return m_list.size();
  }

  void Add(const char* value_, int length, BOOL enable_)
  {
    m_list.push_back( new TFilterItem(value_, length, enable_) );
  }

  void Toggle(int index)
  {
    if( (unsigned)index < m_list.size() )
    {
      m_list[index]->enable = !m_list[index]->enable;
    }
  }

  void Delete(int index)
  {
    if( (unsigned)index < m_list.size() )
    {
      m_list.erase( m_list.begin() + index );
    }
  }

  const char* GetValue(int index) const
  {
    if( (unsigned)index < m_list.size() )
    {
      return m_list[index]->value;
    }
    return NULL;
  }

  BOOL IsEnabled(int index) const
  {
    if( (unsigned)index < m_list.size() )
    {
      return m_list[index]->enable;
    }
    return FALSE;
  }

  BOOL IsFiltered(const char* src)
  {
    if( !src )
      return FALSE;

    for( size_t i=0; i<m_list.size(); i++)
    {
      if( m_list[i]->enable && strstr(src, m_list[i]->value) )
        return TRUE;
    }

    return FALSE;
  }

protected:
  struct TFilterItem
  {
    char value[256];
    BOOL enable;
    TFilterItem(const char* value_, int length, BOOL enable_)
    { 
      value[0] = '\0'; 
      if( value_ )
      {
        if( length >= _countof(value) )
          length = _countof(value)-1;
        memcpy(value, value_, length*sizeof(char));
        value[length] = '\0';
      }
      enable = enable_; 
    }
  };
  typedef TFilterItem* PTFilterItem;

  std::vector< PTFilterItem > m_list;
};

struct TFilterPack
{
  void Toggle(int index)
  {
    if( (unsigned)index < m_BlackList.Size() )
    {
      m_BlackList.Toggle(index);
      return;
    }
    
    index -= m_BlackList.Size();
    if( (unsigned)index < m_WhiteList.Size() )
    {
      m_WhiteList.Toggle(index);
    }
  }

  TFilterList m_BlackList;
  TFilterList m_WhiteList;
};

class CTabInfo
{
public:
  CTabInfo()
  {
    m_nPort = 0;
    m_nBufSize = 512;
    for( int i=0; i<LOG_LEVEL_COUNT; i++)
      m_arrbLogLv[i] = 1;
  }
  ~CTabInfo(){}

public:
  tstring m_sTitle;
  // udp mode
  string m_sIP;
  int m_nPort;
  int m_nBufSize;
  // file mode
  tstring m_sFile;

  BOOL m_arrbLogLv[LOG_LEVEL_COUNT];
  TFilterPack m_Filter;
  /*
  DWORD m_dwColor[LOG_LEVEL_COUNT][COLOR_TYPE_COUNT];
  DWORD m_dwFilterColor[COLOR_TYPE_COUNT];
  DWORD m_dwFilterHilight[COLOR_TYPE_COUNT];

  DWORD m_crFocusLine;
  DWORD m_crNoHighlight;
  DWORD m_crNoFocusLine;
  DWORD m_crHighlight;
  DWORD m_crHighlightText;

  int m_nDefaultFontHeight;
  */
};
typedef std::vector<CTabInfo*> CTabInfoArray;


#endif