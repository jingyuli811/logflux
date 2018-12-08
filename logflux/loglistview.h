#ifndef LOG_LISTVIEW_H
#define LOG_LISTVIEW_H

#include "WTL/include/atlctrls.h"
#include "WTL/include/atlctrlw.h"
#include "WTL/include/atlctrlx.h"
#include <string>
#include <vector>
#include <deque>
#include "utils/stringconv.hpp"
#include "Resource.h"
#include "DragDrop.h"
#include "logserver.h"
#include "tabdata.h"

struct LogPack
{
  unsigned int m_nIndex;
  int m_nLevel;
  int m_nFiltered;        // FILTER_STATE
  TCHAR m_sLog[512];     // ccmax of item is 260
  LogPack(const char* p_sData, int level)
  {
    memset(m_sLog, 0, _countof(m_sLog));
    USES_CONVERSION;
    _tcsncpy_s(m_sLog, _countof(m_sLog), A2T((char*)p_sData), _countof(m_sLog)-1);
    m_sLog[_countof(m_sLog)-1] = _T('\0');
    static unsigned int cnt = 0;
    m_nIndex = ++cnt;
    m_nLevel = level;
    m_nFiltered = FILTERED_NO;
  }
};
typedef LogPack* PLogPack;

struct LogPackEx
{
  unsigned int m_nIndex;
  int m_nLevel;
  int m_nFiltered;        // FILTER_STATE
  TCHAR* m_sLog;
  int m_nBufSize;
  LogPackEx(const char* p_sData, int buffersize, int level):
  m_nIndex(0),
  m_nLevel(LOG_NONE),
  m_nFiltered(FILTERED_NO),
  m_sLog(NULL),
  m_nBufSize(0)
  {
    Create(p_sData, buffersize, level);
  }

  ~LogPackEx()
  {
    delete [] m_sLog;
    m_sLog = NULL;
    m_nBufSize = 0;
  }

  void Create(const char* p_sData, int buffersize, int level)
  {
    if( !m_sLog || m_nBufSize < buffersize )
    {
      delete [] m_sLog;

      m_nBufSize = buffersize;
      m_sLog = new TCHAR[buffersize];
    }
    memset(m_sLog, 0, m_nBufSize*sizeof(TCHAR));
    /*
    _tcsncpy_s(m_sLog, buffersize, CStringConv::AToW(p_sData, CP_UTF8).c_str(), buffersize-1);
    m_sLog[buffersize-1] = _T('\0');
    */
    std::wstring ws = CStringConv::AToW(p_sData, CP_UTF8);
    memcpy(m_sLog, ws.c_str(), (ws.length() < (unsigned)buffersize ? ws.length()*sizeof(TCHAR) : (buffersize-1)*sizeof(TCHAR)) );
    m_sLog[buffersize-1] = _T('\0');

    static unsigned int cnt = 0;
    m_nIndex = ++cnt;
    m_nLevel = level;
    m_nFiltered = FILTERED_NO;
  }
};
typedef LogPackEx* PLogPackEx;

typedef std::vector< PLogPack >    TLogPackArray;
typedef TLogPackArray::iterator TLogPackArrayIt;

typedef std::deque< PLogPackEx >    TLogPackExArray;
typedef TLogPackExArray::iterator TLogPackExArrayIt;

typedef std::deque<int>        TIntArray;
typedef TIntArray::iterator     TIntArrayIt;

class CLogData
{
public:
  CLogData();
  ~CLogData();

private:
#ifdef ENABLE_BUFFER_SIZE_CONFIG
  TLogPackExArray  m_arrLog;          // store log info
#else
  TLogPackArray  m_arrLog;          // store log info
#endif
  TIntArray      m_arrFilterRow;    // store index into m_arrLog
  int m_nFilterMax;              // all filter matched items
  int m_nFilterCur;              // current find target

  tstring m_sSearchText;
  CMenu	m_PopupMenu;

  BOOL m_arrbLogLv[LOG_LEVEL_COUNT];
  DWORD m_dwColor[LOG_LEVEL_COUNT][COLOR_TYPE_COUNT];
  DWORD m_dwFilterColor[COLOR_TYPE_COUNT];
  DWORD m_dwFilterHilight[COLOR_TYPE_COUNT];

  DWORD m_crFocusLine;
  DWORD m_crNoHighlight;
  DWORD m_crNoFocusLine;
  DWORD m_crHighlight;
  DWORD m_crHighlightText;

  int m_nDefaultFontHeight;
};

class LogListView;

class LogServerDelegate: public ILogServerDelegate
{
public:
  explicit LogServerDelegate(LogListView* p_pListView);
  ~LogServerDelegate();

  virtual void OnReceive(char* p_pData, int p_nLen);
  virtual void OnReceiveTerm();
private:
  LogListView* m_pLogListView;
};

typedef CWinTraitsOR< LVS_REPORT | LVS_OWNERDATA | LVS_SHOWSELALWAYS | LVS_EX_DOUBLEBUFFER, WS_EX_CLIENTEDGE|WS_EX_NOPARENTNOTIFY> CListTraits;
class LogListView : public CWindowImpl<LogListView, CListViewCtrl, CListTraits>,
					           public CCustomDraw<LogListView>
{
public:
  DECLARE_WND_SUPERCLASS(_T("logflux_LogListView"), CListViewCtrl::GetWndClassName())
  LogListView();
  ~LogListView();
  void Initialize(HWND p_hParent, const char* host, int port);
  void SetFilterPack( TFilterPack* filter );
  const TFilterPack* GetFilterPack() const { return m_pFilterPack; }

  BEGIN_MSG_MAP(LogListView)
    MESSAGE_HANDLER(WM_CREATE, OnCreate)
    //MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
    //MESSAGE_HANDLER(WM_WINDOWPOSCHANGING, OnWindowPosChanging)
    //MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
    MSG_WM_KEYDOWN(OnKeyDown)
		MSG_WM_CONTEXTMENU(OnContextMenu)

		REFLECTED_NOTIFY_CODE_HANDLER_EX(LVN_BEGINDRAG, OnBeginDrag)
    REFLECTED_NOTIFY_CODE_HANDLER_EX(LVN_GETDISPINFO, OnGetDispInfo)
    REFLECTED_NOTIFY_CODE_HANDLER_EX(LVN_ODCACHEHINT, OnOdCacheHint)
    REFLECTED_NOTIFY_CODE_HANDLER_EX(LVN_ODFINDITEM, OnOdFindItem)
    REFLECTED_NOTIFY_CODE_HANDLER_EX(LVN_ITEMCHANGED, OnListModified)
    REFLECTED_NOTIFY_CODE_HANDLER_EX(NM_DBLCLK, OnDbClick)

		// chain message map
		CHAIN_MSG_MAP_ALT(CCustomDraw<LogListView>, 1)

    // reflection
    DEFAULT_REFLECTION_HANDLER()
  END_MSG_MAP()

  LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnEraseBkgnd(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled);

  void Receive(const char* pData, const int Len);

  // Notification handlers
	DWORD OnPrePaint(int /*idCtrl*/, LPNMCUSTOMDRAW /*lpNMCustomDraw*/);
  DWORD OnItemPrePaint(int /*idCtrl*/, LPNMCUSTOMDRAW pNMCD );
  DWORD OnSubItemPrePaint(int /*idCtrl*/, LPNMCUSTOMDRAW pNMCD);

  void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	void OnContextMenu(HWND hWnd, CPoint pos);

  LRESULT OnGetDispInfo(LPNMHDR pnmh);
  LRESULT OnOdCacheHint(LPNMHDR pnmh);
  LRESULT OnOdFindItem(LPNMHDR pnmh);
  LRESULT OnListModified( NMHDR* pNMHDR );
  LRESULT OnBeginDrag( NMHDR* pNMHDR );
  LRESULT OnDbClick( NMHDR* pNMHDR );

	void Copy();
	void Delete();
	void SelectAll();
	void SelectReverse();

  void OnFindNext();
  void OnFindPrev();
  void OnFindHilight();

  void SetSearchText(const TCHAR* p_sSearch);
  void ToggleLogLv(int p_nLv);
  BOOL IsLogLvEnabled(int p_nLv);
  void SaveFile();
  void ClearLog();

  bool Start(const char* host, int port);
  bool Stop();

  // dragdrop
  DWORD OnDragEnter( FORMATETC& FormatEtc, STGMEDIUM& StgMedium, DWORD dwKeyState, CPoint point );
  DWORD OnDragOver( FORMATETC& FormatEtc, STGMEDIUM& StgMedium, DWORD dwKeyState, CPoint point );
  BOOL OnDrop( FORMATETC& FormatEtc, STGMEDIUM& StgMedium, DWORD dwEffect, CPoint point );
  void OnDragLeave();
  BOOL OnRenderData( FORMATETC& FormatEtc, STGMEDIUM *pStgMedium, BOOL bDropComplete );
  DWORD OnSourceDrop(CDragInfo& p_draginfo);
 
  void OpenFile(tstring& filename);

  BOOL IsRunning() { return m_LogServer.IsRunning(); }

  LogServer& GetLogServer() { return m_LogServer; }

  void SetBufferSize(int buffersz) { m_nBufferSize = buffersz; }
  int GetBufferSize() { return m_nBufferSize; }

  const TCHAR* GetSearchText() { return m_sSearchText.c_str(); }

  void SetWindowMenu(CMenuHandle menu);

  void Refresh();
  
private:
  BOOL SelectItemEx(int nIndex);
  BOOL SetItemCountEx(int nItems, bool bInvalidateAll=false);
  void SelectNone();
  void GetSelText(TIntArray& arrMark, CString& p_sStr);
  void HilightFilter(int index);

  // font size
  void ChangeFontSize(int diff);

  // clear log
  void ClearAll();
  void Delete(TIntArray& arrMark);

private:
#ifdef ENABLE_BUFFER_SIZE_CONFIG
  TLogPackExArray  m_arrLog;          // store log info
#else
  TLogPackArray  m_arrLog;          // store log info
#endif
  TFilterPack* m_pFilterPack;

  TIntArray      m_arrFilterRow;    // store index into m_arrLog
  unsigned int m_nFilterMax;              // all filter matched items
  unsigned int m_nFilterCur;              // current find target

  tstring m_sSearchText;
 
  BOOL m_arrbLogLv[LOG_LEVEL_COUNT];
  DWORD m_dwColor[LOG_LEVEL_COUNT][COLOR_TYPE_COUNT];
  DWORD m_dwFilterColor[COLOR_TYPE_COUNT];
  DWORD m_dwFilterHilight[COLOR_TYPE_COUNT];

  
	DWORD m_crFocusLine;
	DWORD m_crNoHighlight;
	DWORD m_crNoFocusLine;
	DWORD m_crHighlight;
	DWORD m_crHighlightText;

  int m_nDefaultFontHeight;
  CDragDrop<LogListView> m_dragdrop;
  LogServerDelegate m_LogServerDelegate;
  LogServer m_LogServer;
  BOOL m_bIsRunning;

  int m_nBufferSize;

  BOOL m_bSearchResultOnly;
  BOOL m_bSearchHilight;
  int m_nScroll;

  CMenuHandle m_PopupMenu;

  BOOL m_bDragging;
  BOOL m_bDirty;
};
typedef LogListView* PLogListView;

#endif