#include "logflux.h"
#include "loglistview.h"
#include "clipboard.h"
#include "util.h"
#include "DragDrop.h"
#include "search_list_dialog.h"
#include "config.h"

#define MLC_BLEND(A, B, X) ((A + B * (X-1) + ((X+1)/2)) / X)

#define MLC_RGBBLEND(A, B, X) (                   \
	RGB(MLC_BLEND(GetRValue(A), GetRValue(B), X), \
	MLC_BLEND(GetGValue(A), GetGValue(B), X),     \
	MLC_BLEND(GetBValue(A), GetBValue(B), X))     \
)

#define REDRAW_ALL    

LogServerDelegate::LogServerDelegate(LogListView* p_pListView) : m_pLogListView(p_pListView)
{
}

LogServerDelegate::~LogServerDelegate()
{
}

void LogServerDelegate::OnReceive(char* p_pData, int p_nLen)
{
  m_pLogListView->Receive(p_pData, p_nLen);
}

void LogServerDelegate::OnReceiveTerm()
{

}

LogListView::LogListView() : 
m_pFilterPack(theConfig().GetNullFilterPack())
, m_bIsRunning(FALSE)
, m_nBufferSize(DEFAULT_BUFFER_SIZE)
, m_bSearchResultOnly(FALSE)
, m_bSearchHilight(FALSE)
, m_nScroll(-1)
, m_bDragging(FALSE)
, m_LogServerDelegate(this)
, m_LogServer(&m_LogServerDelegate)
{
  m_nFilterMax = 0;
  m_nFilterCur = 0;

  // log level
  for( size_t i=0; i<_countof(m_arrbLogLv); i++ )
  {
    m_arrbLogLv[i] = TRUE;
  }

  // log color
  m_dwColor[LOG_DEBUG][COLOR_FRONT] = RGB(0,0,0);
  m_dwColor[LOG_DEBUG][COLOR_BACK] = RGB(255,255,255);

  m_dwColor[LOG_INFO][COLOR_FRONT] = RGB(0,0,0);
  m_dwColor[LOG_INFO][COLOR_BACK] = RGB(255,255,255);

  m_dwColor[LOG_WARN][COLOR_FRONT] = RGB(0,0,0);
  m_dwColor[LOG_WARN][COLOR_BACK] = RGB(255,255,255);

  m_dwColor[LOG_ERROR][COLOR_FRONT] = RGB(255,0,0);
  m_dwColor[LOG_ERROR][COLOR_BACK] = RGB(255,255,255);

  if( m_bSearchHilight )
  {
    m_dwFilterColor[COLOR_FRONT] = RGB(0,0,0);
    m_dwFilterHilight[COLOR_FRONT] = RGB(0,0,0);
    m_dwFilterColor[COLOR_BACK] = RGB(255,255,128);
    m_dwFilterHilight[COLOR_BACK] = RGB(255,196,128);
  }
  else
  {
    m_dwFilterColor[COLOR_FRONT] = RGB(60,60,255);
    m_dwFilterHilight[COLOR_FRONT] = RGB(60,60,255);
    m_dwFilterColor[COLOR_BACK] = RGB(255,255,255);
    m_dwFilterHilight[COLOR_BACK] = RGB(255,255,0);
  }

  COLORREF crHighlight = ::GetSysColor(COLOR_HIGHLIGHT);
	COLORREF crWindow    = ::GetSysColor(COLOR_WINDOW);

	m_crFocusLine 		= crHighlight;
	m_crNoHighlight		= MLC_RGBBLEND(crHighlight, crWindow, 8);
	m_crNoFocusLine		= MLC_RGBBLEND(crHighlight, crWindow, 2);
	m_crHighlight		= MLC_RGBBLEND(crHighlight, crWindow, 4);
	m_crHighlightText	= ::GetSysColor(COLOR_WINDOWTEXT);
}

LogListView::~LogListView()
{
  Dbgp("LogListView::~LogListView %s:%d\n", m_LogServer.GetHost(), m_LogServer.GetPort());

  m_LogServer.End();

  // clear log
  ClearAll();

  DestroyWindow();
}

void LogListView::ClearAll()
{
  TLogPackExArrayIt it = m_arrLog.begin();
  while( it != m_arrLog.end() )
  {
    delete *it;
    ++it;
  }
  m_arrLog.clear();
  m_arrFilterRow.clear();

  m_nFilterCur = 0;
  m_nFilterMax = 0;
  //SetItemCountEx(0, true);
}

void LogListView::Delete(TIntArray& arrMark)
{
  // delete from end
  if( m_bSearchResultOnly )
  {
    for( int i = arrMark.size() - 1; i >= 0; i-- )
    {
      TLogPackExArrayIt it = m_arrLog.begin() + m_arrFilterRow[arrMark[i]];
      delete *it;
      m_arrLog.erase( it );
      m_arrFilterRow.erase(m_arrFilterRow.begin() + arrMark[i]);
      --m_nFilterMax;
    }
  }
  else
  {
    for( int i = arrMark.size() - 1; i >= 0; i-- )
    {
      TLogPackExArrayIt it = m_arrLog.begin() + arrMark[i];
      delete *it;
      m_arrLog.erase( it );
      m_arrFilterRow.erase(m_arrFilterRow.begin() + arrMark[i]);
    }
  }

  
  // select nothing
  SelectNone();

  // re-search again
  SetSearchText(NULL);
}

void LogListView::Initialize(HWND p_hParent, const char* host, int port)
{
  Create(p_hParent, rcDefault);
  //SetExtendedListViewStyle(LVS_EX_TRACKSELECT | LVS_EX_ONECLICKACTIVATE | LVS_EX_UNDERLINEHOT | LVS_EX_FULLROWSELECT);
  SetExtendedListViewStyle( LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);

  // Turning on LVS_EX_DOUBLEBUFFER also enables the transparent
  // selection marquee.
  SetExtendedListViewStyle( LVS_EX_DOUBLEBUFFER, LVS_EX_DOUBLEBUFFER );

  // Each tile will have 2 additional lines (3 lines total).
  LVTILEVIEWINFO lvtvi = { sizeof(LVTILEVIEWINFO), LVTVIM_COLUMNS };

  lvtvi.cLines = 2;
  lvtvi.dwFlags = LVTVIF_AUTOSIZE;
  SetTileViewInfo ( &lvtvi );

  // Create list view columns
  InsertColumn(0, _T("No"), LVCFMT_LEFT, 50, 0);
  InsertColumn(1, _T("Message"), LVCFMT_LEFT, 700, 1);

  // create popup menu
  /*
	if (m_PopupMenu)
		m_PopupMenu.DestroyMenu();

	m_PopupMenu.CreatePopupMenu();

	m_PopupMenu.AppendMenu(MF_STRING, ID_EVENT_DELETE,			_T("&Delete\tDelete"));
  m_PopupMenu.AppendMenu(MF_STRING, ID_EVENT_COPY, _T("&Copy\tCtrl+C"));
  //m_PopupMenu.AppendMenu(MF_SEPARATOR);
  //m_PopupMenu.AppendMenu(MF_STRING, ID_EVENT_SHOW_FILTER, _T("Search &Next\tF3"));
  //m_PopupMenu.AppendMenu(MF_STRING, ID_EVENT_SHOW_FILTER, _T("Search &Prev\tShift+F3"));
	m_PopupMenu.AppendMenu(MF_SEPARATOR);
	m_PopupMenu.AppendMenu(MF_STRING, ID_EVENT_SELECTALL, _T("Select &All\tCtrl+A"));
  */

  HFONT oldfont = GetFont();
  LOGFONT logfont;
  if( ::GetObject(oldfont, sizeof(LOGFONT), &logfont) != sizeof(LOGFONT) )
    return;

  m_nDefaultFontHeight = logfont.lfHeight;

  m_dragdrop.Register(this, TRUE);
 //m_dragdrop.AddTargetFormat(CF_TEXT);
  //for( int i=1; i<CF_MAX; i++)  
  //  m_dragdrop.AddTargetFormat(i);
  m_dragdrop.AddTargetFormat(CF_HDROP);

  m_LogServer.Initialize(host, port);
}

void LogListView::SetFilterPack( TFilterPack* filter )
{
  // ensure we always valid
  if( !filter )
    m_pFilterPack = theConfig().GetNullFilterPack();
  else
    m_pFilterPack = filter;
}

LRESULT LogListView::OnGetDispInfo(LPNMHDR pnmh)
{
  
  LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pnmh;
  LV_ITEM* pItem= &(pDispInfo)->item;

  int item = pItem->iItem;
  if( m_bSearchResultOnly )
  {
    if( (unsigned)item >= m_nFilterMax )
      return 0;
    item = m_arrFilterRow[item];
  }
  else
  {
    if( item >= (int)m_arrLog.size() )
      return 0;
  }

  // true item

  if (pItem->mask & LVIF_TEXT)
  {
    switch(pItem->iSubItem)
    {
    case 0:
      // no need to check cchTextMax because the length of "%05d" will most definitely be 5
      _stprintf_s(pItem->pszText, pItem->cchTextMax, _T("%05d"), m_arrLog[item]->m_nIndex);
      break;

    case 1:
      {
        // NOTE: cchTextMax is the Number of TCHARs in the buffer pointed to by pszText,
        //       including the terminating NULL
        int len = _tcslen(m_arrLog[item]->m_sLog);
        if( pItem->cchTextMax < len )
          len = pItem->cchTextMax - 1;
        _tcsncpy_s(pItem->pszText, pItem->cchTextMax, m_arrLog[item]->m_sLog, _TRUNCATE);
        //pItem->pszText[len] = _T('\0');
      }
      break;
    }
  }
  

  return 0;
}

LRESULT LogListView::OnOdCacheHint(LPNMHDR pnmh)
{

  return 0;
}

LRESULT LogListView::OnOdFindItem(LPNMHDR pnmh)
{

  return 0;
}


LRESULT LogListView::OnListModified( NMHDR* pNMHDR )
{
  __dbg_printf("OnListModified");

	NM_LISTVIEW *pNMListView = (NM_LISTVIEW*)pNMHDR;
  //this works because true is equal to 1 and false equal to 0
	BOOL notLast = pNMListView->iItem + 1 != GetItemCount();
	BOOL notFirst = pNMListView->iItem != 0;
	RedrawItems(pNMListView->iItem - notFirst, pNMListView->iItem + notLast);

  // update m_nFilterCur
  /*
  if( pNMListView->iItem >= 0 && m_nFilterMax > 0 )
  {
    size_t nFilterCur = 0;
    while( m_arrFilterRow[nFilterCur] < pNMListView->iItem )
    {
      ++nFilterCur;
    }
    if( nFilterCur >= 0 && nFilterCur < m_nFilterMax )
      //HilightFilter(nFilterCur);
      m__nF
    else
      HilightFilter(0);
  }
  */

	return 0;
}

LRESULT LogListView::OnBeginDrag( NMHDR* pNMHDR )
{
 // CComPtr<IDragSourceHelper> pdsh;

  NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

  // get all selected item
  DragItemInfoArray arrDragItem;
  for( int i=0; i < GetItemCount(); i++ )
  {
    if( LVIS_SELECTED == GetItemState(i, LVIS_SELECTED) )
      arrDragItem.push_back( DragItemInfo(i) );
  }

  // init dnd
  if( !m_dragdrop.Init(arrDragItem) )
    return 0;


  /*
  HRESULT hr = pdsh.CoCreateInstance ( CLSID_DragDropHelper );

  if ( SUCCEEDED(hr) )
  {
    CComQIPtr<IDataObject> pdo;
    if ( pdo = m_dragdrop.GetDataObject() )
      pdsh->InitializeFromWindow ( m_hWnd, &pNMListView->ptAction, pdo );
  }
*/

  m_bDragging = TRUE;

  DWORD dwEffects = m_dragdrop.DoDragDrop();

  return 0;
}


void LogListView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
  /*
  if (nChar == 'A' && ::GetAsyncKeyState(VK_CONTROL)<0)
  {
		// Ctrl+A: Select all items
		PostMessage(WM_COMMAND, ID_EVENT_SELECTALL, 0);
	}
	else if (nChar == 'S' && ::GetAsyncKeyState(VK_CONTROL)<0 )
	{
	  // Ctrl+S: Select Reverse items
	  PostMessage(WM_COMMAND, ID_EVENT_SELECT_REVERSE, 0);
	}
	else if (nChar == VK_DELETE )
	{
	  PostMessage(WM_COMMAND, ID_EVENT_DELETE, 0);
	}
	else if (nChar == 'C' && (GetKeyState(VK_CONTROL) & 0x8000))
	{
		// Ctrl+C: Copy keycombo
		SendMessage(WM_COMMAND, ID_EVENT_COPY);
	}
  */
  if (nChar == 'S' && ::GetAsyncKeyState(VK_CONTROL)<0 )
  {
    // Ctrl+S: Select Reverse items
    PostMessage(WM_COMMAND, ID_EVENT_SELECT_REVERSE, 0);
  }
  /*
	else if (nChar == 'F' && (GetKeyState(VK_CONTROL) & 0x8000) )
	{
		// Ctrl+F: Search item
	//	OnFindStart();
	}
  else if (nChar == VK_ADD && (GetKeyState(VK_SHIFT) & 0x8000) )
  {
    ChangeFontSize(1);
  }
  else if (nChar == VK_SUBTRACT && (GetKeyState(VK_SHIFT) & 0x8000) )
  {
    ChangeFontSize(-1);
  }
  else if (nChar == VK_MULTIPLY && (GetKeyState(VK_SHIFT) & 0x8000) )
  {
    ChangeFontSize(0);
  }
	else if (nChar == VK_F3 )
	{
		if (GetKeyState(VK_SHIFT) & 0x8000)
		{
			// Shift+F3: Search previous
			OnFindPrev();
		}
		else
		{
			// F3: Search next
			OnFindNext();
		}
	}
  */
	else
  {
    SetMsgHandled(FALSE);
  }
}

void LogListView::OnContextMenu(HWND hWnd, CPoint pos)
{
	CWaitCursor w;

	GetPopupMenuPos(*this, pos);
//	m_PopupMenu.TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON, pos.x, pos.y, m_hWnd);

  if( !m_PopupMenu.IsNull() )
  {
    m_PopupMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pos.x, pos.y, GetTopLevelWindow());
  }
}


void LogListView::Copy()
{
  // too many selected 
	if( GetSelectedCount() > 30000 &&
		AtlMessageBox(NULL,_T("too many rows selected to be copyed, are you sure?"), 
		_T("logflux"), MB_DEFBUTTON2 | MB_ICONQUESTION | MB_OKCANCEL ) == IDCANCEL )
	{
		return;
	}

  if( 0 == GetSelectedCount() )
    return;

  CWaitCursor w;
  CString str;
  TIntArray arrMark;

  // mark selected item
  int iSel = -1;
  while( (iSel = GetNextItem(iSel, LVIS_SELECTED)) != -1 )
  {
    arrMark.push_back(iSel);
  }

  GetSelText(arrMark, str);

  if( !str.IsEmpty() )
		Clipboard::SetText(NULL,str);
}

void LogListView::Delete()
{
  if( 0 == GetSelectedCount() )
    return;

  CWaitCursor w;

  TIntArray arrMark;

  // mark selected item
  int iSel = -1;
	while( (iSel = GetNextItem(iSel, LVIS_SELECTED)) != -1 )
	{
		arrMark.push_back(iSel);
  }
  Delete(arrMark);
}

void LogListView::SelectAll()
{
  if( GetSelectedCount() == GetItemCount() )
    return;

	LVITEM lvi = { 0 };
	lvi.state = LVIS_SELECTED;
	lvi.stateMask = LVIS_SELECTED;
	::SendMessage(m_hWnd, LVM_SETITEMSTATE, -1, (LPARAM)&lvi);
}

void LogListView::SelectReverse()
{
	int sz = GetItemCount();
	for( int i=0; i<sz; ++i )
	{
		LVITEM lvi = { 0 };
		lvi.state = ((UINT)::SendMessage(m_hWnd, LVM_GETITEMSTATE, i, LVIS_SELECTED)&LVIS_SELECTED)?0:LVIS_SELECTED;
		lvi.stateMask = LVIS_SELECTED;
		::SendMessage(m_hWnd, LVM_SETITEMSTATE, i, (LPARAM)&lvi);
	}
}

LRESULT LogListView::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
  LRESULT res = DefWindowProc(uMsg, wParam, lParam);
 
  return res;
}


LRESULT LogListView::OnEraseBkgnd(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
{
  return 1;
}

void LogListView::Receive(const char* p_sData, const int Len)
{
  if( !p_sData )
    return;

  int lv = LOG_NONE;

  if( strstr(p_sData, "Debug") )
  {
    lv = LOG_DEBUG;
  }
  else if( strstr(p_sData, "Info") )
  {
    lv = LOG_INFO;
  }
  else if( strstr(p_sData, "Warn") )
  {
    lv = LOG_WARN;
  }
  else if( strstr(p_sData, "Error") )
  {
    lv = LOG_ERROR;
  }
  else
  {
    return;
  }

  // black filter
  //if( theConfig().IsBlack(p_sData) )
  if( m_pFilterPack->m_BlackList.IsFiltered(p_sData) )
    return;

  if( IsLogLvEnabled(lv) )
  {
    m_arrLog.push_back( new LogPackEx(p_sData, m_nBufferSize, lv) );

    if( m_sSearchText.length() &&
      stristr((*m_arrLog.rbegin())->m_sLog, m_sSearchText.c_str()) )
    {
      (*m_arrLog.rbegin())->m_nFiltered = FILTERED_YES;
      m_arrFilterRow.insert( m_arrFilterRow.begin() + m_nFilterMax, m_arrLog.size()-1 );
      ++m_nFilterMax;
    }
    else
    {
      m_arrFilterRow.push_back( m_arrLog.size()-1 );  // index of this logpack
    }

    if( m_arrLog.size() > 32767 )
    {
      TLogPackExArrayIt it = m_arrLog.begin();
      delete *it;
      m_arrLog.erase( it );
      m_arrFilterRow.erase( m_arrFilterRow.begin() );
      ATLASSERT( m_arrLog.size() == m_arrFilterRow.size());
      //GetTopIndex();
      //GetCountPerPage();

      if( m_bSearchResultOnly )
        SetItemCountEx( m_nFilterMax );
      else
        SetItemCountEx(m_arrLog.size());
      RedrawItems( GetTopIndex(), GetTopIndex()+GetCountPerPage() );
    }
    else
    {
      if( m_bSearchResultOnly )
        SetItemCountEx( m_nFilterMax );
      else
        SetItemCountEx(m_arrLog.size());
    }
  }
}

BOOL LogListView::IsLogLvEnabled(int p_nLv)
{
  if( p_nLv >= 0 && p_nLv < LOG_LEVEL_COUNT )
  {
    return m_arrbLogLv[p_nLv];
  }
  return FALSE;
}


void LogListView::ToggleLogLv(int p_nLv)
{
  if( p_nLv >= 0 && p_nLv < LOG_LEVEL_COUNT )
  {
    m_arrbLogLv[p_nLv] = !m_arrbLogLv[p_nLv];
  }
}

void LogListView::SetSearchText(const TCHAR* p_sSearch)
{
  if( p_sSearch )
    m_sSearchText = p_sSearch;
  
  ATLASSERT(m_arrLog.size() == m_arrFilterRow.size());

  size_t row = 0;
  if( !m_sSearchText.length() )
  {
    for( size_t i = 0; i < m_arrLog.size(); i++ )
    {
      m_arrLog[i]->m_nFiltered = FILTERED_NO;
    }
  }
  else
  {
    for( size_t i = 0; i < m_arrLog.size(); i++ )
    {
      if( stristr(m_arrLog[i]->m_sLog, m_sSearchText.c_str()) )
      {
        m_arrFilterRow[row] = i;
        m_arrLog[i]->m_nFiltered = FILTERED_YES;
        row++;
      }
      else
      {
        m_arrLog[i]->m_nFiltered = FILTERED_NO;
      }
    }
  }

  // after filtering, filter list has $row rows
  m_nFilterMax = row;
  if( m_nFilterMax > 0 )
  {
    m_nFilterCur = 0;

    // highlight 
    int item = m_arrFilterRow[m_nFilterCur];
    m_arrLog[item]->m_nFiltered = FILTERED_HILIGHT;
    RedrawItems(item, item);
    EnsureVisible(item, TRUE);
  }

  if( m_bSearchResultOnly )
  {
    SetItemCountEx(m_nFilterMax, true);
  }
  else
  {
    SetItemCountEx(m_arrLog.size(), true);
  }
}

BOOL LogListView::SetItemCountEx(int nItems, bool bInvalidateAll)
{
  ATLASSERT(::IsWindow(m_hWnd));
  return (BOOL)::SendMessage(m_hWnd, LVM_SETITEMCOUNT, nItems, bInvalidateAll?LVSICF_NOSCROLL:LVSICF_NOSCROLL|LVSICF_NOINVALIDATEALL);
}

BOOL LogListView::SelectItemEx(int nIndex)
{
  ATLASSERT(::IsWindow(m_hWnd));

  BOOL bRet = SetItemState(nIndex, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
  if(bRet)
    bRet = EnsureVisible(nIndex, FALSE);
  return bRet;
}

void LogListView::SelectNone()
{
  LVITEM lvi = { 0 };
  lvi.state = 0;
	lvi.stateMask = LVIS_SELECTED;
	::SendMessage(m_hWnd, LVM_SETITEMSTATE, -1, (LPARAM)&lvi);
}

void LogListView::SaveFile()
{
  if( GetSelectedCount() <= 0 )
    return;

  CString str; 

  GetModuleFileName(NULL, str.GetBufferSetLength(MAX_PATH), MAX_PATH-1);
  str.ReleaseBuffer();
  str.Replace(_T(".exe"), _T(".txt"));

  CFileDialog dlg(TRUE, NULL, (LPCTSTR)str.GetBuffer(), OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST,
		_T("All Files (*.*)\0*.*\0"), m_hWnd );
  str.ReleaseBuffer();

	if( IDOK == dlg.DoModal(m_hWnd) )
	{
    TIntArray arrMark;

    // mark selected item
    int iSel = -1;
    while( (iSel = GetNextItem(iSel, LVIS_SELECTED)) != -1 )
    {
      arrMark.push_back(iSel);
    }

    GetSelText(arrMark, str);

    if( !str.IsEmpty() )
    {
		  FILE* fp = _tfopen(dlg.m_szFileName, _T("w,ccs=UNICODE"));
      if( fp )
      {
        fwrite(str.GetBuffer(), sizeof(TCHAR), str.GetLength(), fp);
        str.ReleaseBuffer();
        fclose(fp);
      }
    }
	}
}

void LogListView::ClearLog()
{
  CWaitCursor w;

  SetRedraw(FALSE);

  TIntArray arrMark;
  int count = GetItemCount();
  for( int i=0; i<count; i++)
  {
    arrMark.push_back(i);
  }
  
  Delete(arrMark);

  SetRedraw(TRUE);
}

bool LogListView::Start(const char* host, int port)
{
  // update server
  if( !m_LogServer.IsRunning() )
  {
    if( m_LogServer.Run(host, port) )
    {
      return true;
    }
    else
    {
      ::MessageBox(NULL, m_LogServer.ErrorStr(), _T("logflux"), MB_OK|MB_ICONERROR);
    }
  }

  return false;
}

bool LogListView::Stop()
{
  Dbgp("LogListView::Stop\n");

  if( m_LogServer.IsRunning() )
  {
    m_LogServer.End();
    return true;
  }

  return false;
}

void LogListView::GetSelText(TIntArray& arrMark, CString& p_sStr)
{
  int iSel = -1;
  p_sStr = _T("");
	p_sStr.Preallocate(256*arrMark.size());//GetSelectedCount());

  if( m_bSearchResultOnly )
  {
    for( size_t i = 0; i<arrMark.size(); i++ )
    {
      if (!p_sStr.IsEmpty())
        p_sStr.Append(_T("\n"),1);

      p_sStr.Append( m_arrLog[m_arrFilterRow[arrMark[i]]]->m_sLog );
    }
  }
	else
  {
    for( size_t i = 0; i<arrMark.size(); i++ )
    {
      if (!p_sStr.IsEmpty())
        p_sStr.Append(_T("\n"),1);

      p_sStr.Append( m_arrLog[arrMark[i]]->m_sLog );
    }
    /*
    while( (iSel = GetNextItem(iSel, LVIS_SELECTED)) != -1 )
	  {
		  if (!p_sStr.IsEmpty())
			  p_sStr.Append(_T("\n"),1);

      ATLASSERT( iSel < (int)m_arrLog.size() );

		  p_sStr.Append( m_arrLog[iSel]->m_sLog );
	  }
    */
  }
}

void LogListView::ChangeFontSize(int diff)
{
  HFONT oldfont = GetFont();

  LOGFONT logfont;
  if( ::GetObject(oldfont, sizeof(LOGFONT), &logfont) != sizeof(LOGFONT) )
    return;

  if( diff )
  {
    if( logfont.lfHeight < -1 )
      logfont.lfHeight -= diff;
    else if( logfont.lfHeight > 1 )
      logfont.lfHeight += diff;
    else if( diff > 1 ) // && lfHeight == 0
      logfont.lfHeight += diff;
  }
  else
    logfont.lfHeight = m_nDefaultFontHeight;

  /*
  logfont.lfWidth= 0;
  logfont.lfEscapement= 0;
  logfont.lfOrientation= 0;
  logfont.lfWeight= 400;
  logfont.lfItalic= FALSE;
  logfont.lfUnderline= FALSE;
  logfont.lfStrikeOut= FALSE;
  logfont.lfCharSet= DEFAULT_CHARSET;
  logfont.lfOutPrecision= OUT_DEFAULT_PRECIS;
  logfont.lfClipPrecision= CLIP_DEFAULT_PRECIS;
  logfont.lfQuality= DEFAULT_QUALITY;
  logfont.lfPitchAndFamily= DEFAULT_PITCH | FF_DONTCARE;
  _tcscpy(logfont.lfFaceName, _T("Arial"));
  HFONT font = CreateFontIndirect(&logfont);
  */
  
  static char buf[32];
  sprintf_s(buf, _countof(buf), "Info font size=%d", logfont.lfHeight);

  Receive(buf, 32);
  
  HFONT newfont = CreateFontIndirect(&logfont);
  SetFont(newfont, TRUE);

  ::DeleteObject(oldfont);

  HFONT fontheader = GetHeader().GetFont();
  if( ::GetObject(fontheader, sizeof(LOGFONT), &logfont) != sizeof(LOGFONT) )
    return;
  sprintf_s(buf, _countof(buf), "Info header font size=%d", logfont.lfHeight);
  
  Receive(buf, 32);

}

DWORD LogListView::OnPrePaint(int /*idCtrl*/, LPNMCUSTOMDRAW /*lpNMCustomDraw*/)
{
  return CDRF_NOTIFYITEMDRAW;//CDRF_DODEFAULT; // OnItemPrePaint
}

#define m_cxMoveMark (8)
#define m_cyMoveMark (8)
DWORD LogListView::OnItemPrePaint(int /*idCtrl*/, LPNMCUSTOMDRAW pNMCD )
{	

if(!m_bSearchHilight)
{
  LPNMLVCUSTOMDRAW pLVCD = (LPNMLVCUSTOMDRAW)pNMCD;
  
  DWORD_PTR item = pNMCD->dwItemSpec;

  if( m_bSearchResultOnly )
  {
    if( item >= m_nFilterMax )
      return CDRF_DODEFAULT;
    item = m_arrFilterRow[item];
  }
  else
  {
    if( (unsigned)item >= m_arrLog.size() )
      return CDRF_DODEFAULT;
  }

  if( m_arrLog[item]->m_nFiltered )
  {
    if( m_arrLog[item]->m_nFiltered == FILTERED_HILIGHT )
    {
      pLVCD->clrText = m_dwFilterHilight[COLOR_FRONT];
      pLVCD->clrTextBk = m_dwFilterHilight[COLOR_BACK];
    }
    else
    {
      pLVCD->clrText = m_dwFilterColor[COLOR_FRONT];
      pLVCD->clrTextBk = m_dwFilterColor[COLOR_BACK];
    }
  }
  else
  {
    pLVCD->clrText = m_dwColor[ m_arrLog[item]->m_nLevel ][COLOR_FRONT];
    pLVCD->clrTextBk = m_dwColor[ m_arrLog[item]->m_nLevel ][COLOR_BACK];
  }

	return CDRF_DODEFAULT;
		//return CDRF_NOTIFYSUBITEMDRAW;
}
else
{
  NMLVCUSTOMDRAW* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW*>( pNMCD );
  
  // This is the beginning of an item's paint cycle.
  int item = static_cast<int>( pLVCD->nmcd.dwItemSpec );
  int item_ = item;
  if( m_bSearchResultOnly )
  {
    if( (unsigned)item >= m_nFilterMax )
      return CDRF_DODEFAULT;
    item_ = m_arrFilterRow[item];
  }
  else
  {
    if( (unsigned)item >= m_arrLog.size() )
      return CDRF_DODEFAULT;
  }

	// dc from NMCD
	CDCHandle hDC = pNMCD->hdc;

	CRect rcClip;
	if ( hDC.GetClipBox( rcClip ) == ERROR )
	{
		return CDRF_SKIPDEFAULT;
	}

	// Get the image index and selected/focused state of the item being drawn.
	LVITEM   rItem;
	ZeroMemory ( &rItem, sizeof(LVITEM) );
	rItem.mask  = LVIF_IMAGE | LVIF_STATE;
	rItem.iItem = item;
	rItem.stateMask = 0xFFFF;     // get all state flags , old:LVIS_SELECTED | LVIS_FOCUSED;
	GetItem( &rItem );

	BOOL bListHasFocus = ( m_hWnd == ::GetFocus() || (GetStyle() & LVS_SHOWSELALWAYS) );
		
	// drop hilighted
	BOOL bDropHighlight = (rItem.state & LVIS_DROPHILITED);
	// Should the item be highlighted
	BOOL bHighlight = ( (rItem.state & LVIS_SELECTED) && bListHasFocus );

	// Get rectangles for drawing
	CRect rcIcon;
	rcIcon.left = LVIR_ICON;
	::SendMessage(m_hWnd, LVM_GETITEMRECT, (WPARAM)item, (LPARAM)&rcIcon);

	// 使用 icon 最左邊檢查，如果大於 clip region 最右邊
	// 表示之後都不在更新範圍，直接返回
	if ( rcIcon.left > rcClip.right )
		return CDRF_SKIPDEFAULT;

	CRect rcBounds;
	rcBounds.left = LVIR_BOUNDS;
	::SendMessage(m_hWnd, LVM_GETITEMRECT, (WPARAM)item, (LPARAM)&rcBounds);

	CMemoryDC memDC(hDC, rcBounds);
	// Save dc state
	int nSavedDC = hDC.SaveDC();
	// set font ( 不然會很醜)
	memDC.SelectFont(GetFont());

	if( bHighlight )
	{
		::SetTextColor(memDC.m_hDC, m_crHighlightText);
		memDC.FillSolidRect( rcBounds, m_crHighlight );
	}
	else
	{
     ::SetTextColor(memDC.m_hDC,  m_dwColor[ m_arrLog[item_]->m_nLevel ][COLOR_FRONT]);
		memDC.FillSolidRect( rcBounds, m_dwColor[ m_arrLog[item_]->m_nLevel ][COLOR_BACK] );


	}

	// 設定透明模式
	// 每列可能有不同背景色色，已經在前面用 FillSolidRect 畫好了
	// (正因為背景色和列數有關，所以沒辦法在 erase background 訊息處理)
	// 背景話好後之後的繪圖都是基於透明下畫上的
	::SetBkMode(memDC.m_hDC, TRANSPARENT);

	CRect rcLabel;

  // 畫選擇項/焦點項外框

  if( bHighlight || (rItem.state & LVIS_FOCUSED) )
  {
    // 上一項也是選擇狀態
    // 往上移一格，這樣子上一項更新時會把這條線蓋過去
    if((item != 0) && (GetItemState(item - 1, LVIS_SELECTED)))
    {
      --rcBounds.top;
    }

    // 下一項也是選擇狀態
    // 往下移一格，這樣子下一項更新時會把這條線蓋過去
    if((item + 1 != (UINT)GetItemCount()) && (GetItemState(item + 1, LVIS_SELECTED)))
    {
      ++rcBounds.bottom;
    }

    ++rcBounds.left;
    --rcBounds.right;

    if(bListHasFocus)
      memDC.FrameRect(&rcBounds, ::CreateSolidBrush(m_crFocusLine));
    else
      memDC.FrameRect(&rcBounds, ::CreateSolidBrush(m_crNoFocusLine));
  }

  // 如果第一欄被左拉到很小時，rcLabel右邊會小於rcIcon右邊
  // 且 rcIcon 都是固定 (4,20)，所以要修改
  // rcLabel.left += 3;
  // rcLabel.right -= 3;

  GetItemRect(item, rcLabel, LVIR_LABEL);
  if( rcClip.left <= rcLabel.right ) // <= rcLabel
  {
    // Draw normal and overlay icon
    CImageList& ImageList = GetImageList(LVSIL_SMALL);
    if(ImageList.m_hImageList != NULL)
    {
      if( rcLabel.right < rcLabel.left )
        rcIcon.right = rcLabel.right - 3;
      //ImageList.Draw(memDC, rItem.iImage, rcIcon.left, rcIcon.top, ILD_TRANSPARENT );
      ImageList.DrawEx(rItem.iImage, memDC, rcIcon.left, rcIcon.top,  
        rcIcon.right - rcIcon.left, rcIcon.bottom - rcIcon.top,
        0, 0,
        ILD_NORMAL|ILD_TRANSPARENT );
    }
  }

  // 在話完第一項後才畫 drop hilight (因為drop hilight要蓋住項目)
  if( bDropHighlight )
  {
    rcBounds.left += m_cxMoveMark / 2 - 1;

    CPen pen;
    pen.CreatePen(PS_SOLID, 1, m_crNoFocusLine);
    CBrush brush;
    brush.CreateSolidBrush(m_crNoFocusLine);

    memDC.SelectPen(pen);
    memDC.SelectBrush(brush);

    int x = rcBounds.left;
    int y = rcBounds.top;

    POINT ptsLeft[3] = { { x, y }, { x + m_cxMoveMark, y }, { x, y + m_cyMoveMark } };
    memDC.Polygon(ptsLeft, 3);
  }

		// remaining columns
		CRect rcCol = rcLabel;// 紀錄每個 column label rect

		LV_COLUMN lvc;
		lvc.mask = LVCF_FMT | LVCF_WIDTH;
    
		CHeaderCtrl& wndHeaderCtrl = GetHeader();
		int iCount = wndHeaderCtrl.GetItemCount();
		for(int iCurrent = 1; // 從1開始是可以的，因為就算全部顯示 0 也是第一欄
			iCurrent < iCount; 
			iCurrent++)
		{
			int iColumn = wndHeaderCtrl.OrderToIndex(iCurrent);
			if( iColumn == 0 )
				continue;

//			int cx = (int)::SendMessage(m_hWnd, LVM_GETCOLUMNWIDTH, iColumn, 0L);
			if( !GetColumn(iColumn, &lvc) )
				break;

//			ATLTRACE(_T("order: %d, index: %d, width: %d\n"), iCurrent, iColumn, lvc.cx);
			// 隱藏的項目會縮到最左邊
			// 寬度為零
			// ex: 0 1 2 3 4 5 隱藏 2 3 4 5
			// OrderToIndex 由左到右為 5 4 3 2 0 1
			if( lvc.cx == 0 )
				continue;

			rcCol.left = rcCol.right;
			rcCol.right += lvc.cx;

			// rcCol.Width() == 0 表示 lvc.cx == 0 所以不用再判斷一次
			if( rcCol.right < rcClip.left ) //|| rcCol.Width() == 0 )
				continue;
			if( rcCol.left > rcClip.right )
				break;
	    
      if( !m_arrLog[item_]->m_sLog[0] )
        continue;

			// Get the text justification
			UINT nJustify = DT_LEFT;
			switch(lvc.fmt & LVCFMT_JUSTIFYMASK)
			{
				case LVCFMT_RIGHT:
					nJustify = DT_RIGHT;
					break;
				case LVCFMT_CENTER:
					nJustify = DT_CENTER;
					break;
				default:
					break;
			}
	        
			rcLabel = rcCol;
			rcLabel.left += 3;
			rcLabel.right -= 3;
	        
      DWORD clrText = m_dwColor[ m_arrLog[item_]->m_nLevel ][COLOR_FRONT];
      DWORD clrBack = m_dwColor[ m_arrLog[item_]->m_nLevel ][COLOR_BACK];

      if( m_arrLog[item_]->m_nFiltered )
      {
        if( m_arrLog[item_]->m_nFiltered == FILTERED_HILIGHT )
        {
          clrText = m_dwFilterHilight[COLOR_FRONT];
          clrBack = m_dwFilterHilight[COLOR_BACK];
        }
        else
        {
          clrText = m_dwFilterColor[COLOR_FRONT];
          clrBack = m_dwFilterColor[COLOR_BACK];
        }
      }

      //memDC.FillSolidRect( rcBounds, clrBack );
      if( m_arrLog[item_]->m_nFiltered )
      {
        static TCHAR temp[4096] = {0};
        TCHAR* pOrig = m_arrLog[item_]->m_sLog;
        TCHAR* pSearch = stristr(pOrig, m_sSearchText.c_str());
        size_t filterlen = m_sSearchText.length();
        SIZE textsz;
        while(pSearch)
        {
          // orig
          if( pSearch > pOrig )
          {
            memset(temp, 0, _countof(temp));
//            _tcsncpy_s(temp, _countof(temp), pOrig, (int)(pSearch-pOrig));
            memcpy(temp, pOrig, (int)(pSearch-pOrig)*sizeof(TCHAR));
            
            memDC.GetTextExtent(temp, _tcslen(temp), &textsz);
            rcLabel.right = rcLabel.left + textsz.cx;

            ::SetTextColor(memDC.m_hDC,  m_dwColor[ m_arrLog[item_]->m_nLevel ][COLOR_FRONT]);
            memDC.DrawText(temp, -1, rcLabel, nJustify | DT_SINGLELINE | 
              DT_NOPREFIX | DT_VCENTER | DT_END_ELLIPSIS);
            rcLabel.left += textsz.cx;
          }
            
          // search
          memset(temp, 0, _countof(temp));
          memcpy(temp, pSearch, filterlen*sizeof(TCHAR));

          memDC.GetTextExtent(temp, _tcslen(temp), &textsz);
          rcLabel.right = rcLabel.left + textsz.cx;

          ::SetTextColor(memDC.m_hDC,  m_dwFilterHilight[COLOR_FRONT]);
          memDC.FillSolidRect( rcLabel, clrBack );
          memDC.DrawText(temp, -1, rcLabel, nJustify | DT_SINGLELINE | 
            DT_NOPREFIX | DT_VCENTER | DT_END_ELLIPSIS);

          rcLabel.left += textsz.cx;

          pOrig = pSearch + filterlen;
          pSearch = stristr(pOrig, m_sSearchText.c_str());
        }

        // orig
        if( pOrig )
        {

          memDC.GetTextExtent(pOrig, _tcslen(pOrig), &textsz);
          rcLabel.right = rcLabel.left + textsz.cx;

          ::SetTextColor(memDC.m_hDC,  m_dwColor[ m_arrLog[item_]->m_nLevel ][COLOR_FRONT]);
          memDC.DrawText(pOrig, -1, rcLabel, nJustify | DT_SINGLELINE | 
            DT_NOPREFIX | DT_VCENTER | DT_END_ELLIPSIS);
        }
      }
      else
      {
        ::SetTextColor(memDC.m_hDC,   m_dwColor[ m_arrLog[item_]->m_nLevel ][COLOR_FRONT]);
        memDC.DrawText(m_arrLog[item_]->m_sLog, -1, rcLabel, nJustify | DT_SINGLELINE | 
          DT_NOPREFIX | DT_VCENTER | DT_END_ELLIPSIS);
      }
		} 

		// Restore dc
		memDC.RestoreDC( nSavedDC );

		return CDRF_SKIPDEFAULT;

} // search hilight

}

DWORD LogListView::OnSubItemPrePaint(int /*idCtrl*/, LPNMCUSTOMDRAW pNMCD)
{
  return CDRF_SKIPDEFAULT;
}

void LogListView::HilightFilter(int index)
{
  // restore old
  int item = m_arrFilterRow[m_nFilterCur];
  m_arrLog[item]->m_nFiltered = FILTERED_YES;
  RedrawItems(item, item);

  if( (unsigned)index >= m_nFilterMax )  // wrap
    m_nFilterCur = 0;
  else if( index < 0 )
    m_nFilterCur = m_nFilterMax - 1;

  // highlight new
  item = m_arrFilterRow[m_nFilterCur];
  m_arrLog[item]->m_nFiltered = FILTERED_HILIGHT;
  RedrawItems(item, item);
//  SelectItem(item);
  EnsureVisible(item, TRUE);
}

void LogListView::OnFindNext()
{
  if( m_nFilterMax > 0 )
  {
    HilightFilter(m_nFilterCur+1);
  }
}

void LogListView::OnFindPrev()
{
  if( m_nFilterMax > 0 )
  {
    HilightFilter(m_nFilterCur-1);
  }
}

void LogListView::OnFindHilight()
{
  m_bSearchHilight = !m_bSearchHilight;

  if( m_bSearchHilight )
  {
    m_dwFilterColor[COLOR_FRONT] = RGB(0,0,0);
    m_dwFilterHilight[COLOR_FRONT] = RGB(0,0,0);
    m_dwFilterColor[COLOR_BACK] = RGB(255,255,128);
    m_dwFilterHilight[COLOR_BACK] = RGB(255,196,128);
  }
  else
  {
    m_dwFilterColor[COLOR_FRONT] = RGB(60,60,255);
    m_dwFilterHilight[COLOR_FRONT] = RGB(60,60,255);
    m_dwFilterColor[COLOR_BACK] = RGB(255,255,255);
    m_dwFilterHilight[COLOR_BACK] = RGB(255,255,0);
  }

  RECT rc;
  GetClientRect(&rc);
  InvalidateRect(&rc);
}

DWORD LogListView::OnDragEnter( FORMATETC& FormatEtc, STGMEDIUM& StgMedium, DWORD dwKeyState, CPoint point )
{
  if( m_bDragging )
    return DROPEFFECT_NONE;

  return DROPEFFECT_MOVE;
}

DWORD LogListView::OnDragOver( FORMATETC& FormatEtc, STGMEDIUM& StgMedium, DWORD dwKeyState, CPoint point )
{
  if( m_bDragging )
    return DROPEFFECT_NONE;

  return DROPEFFECT_MOVE;
}

BOOL LogListView::OnDrop( FORMATETC& FormatEtc, STGMEDIUM& StgMedium, DWORD dwEffect, CPoint point )
{
  if( m_bDragging )
    return DROPEFFECT_NONE;

  if( StgMedium.tymed == TYMED_HGLOBAL && 
      FormatEtc.tymed == TYMED_HGLOBAL &&
      FormatEtc.cfFormat == CF_HDROP )
  {
    SIZE_T sz = GlobalSize(StgMedium.hGlobal);
    DROPFILES* pDrop = (DROPFILES*)GlobalLock(StgMedium.hGlobal);
    SendMessage(GetTopLevelWindow(), WM_DROP_FILES, (WPARAM)sz, (LPARAM)pDrop);
    GlobalUnlock(StgMedium.hGlobal);
  }
  return TRUE;
}

void LogListView::OpenFile(tstring& filename)
{
  FILE* fp = _tfopen(filename.c_str(), _T("r"));
  if( fp )
  {
    char buffer[4096] = {0};
    // read bom
    if( fgets(buffer, _countof(buffer), fp) )
    {
      char* del = strrchr(buffer, '\n');
      if( del )
        *del = '\0';

      if( (unsigned char)buffer[0] == 0xEF &&
          (unsigned char)buffer[1] == 0xBB &&
          (unsigned char)buffer[2] == 0xBF )
      {
        Receive(&buffer[3], strlen(&buffer[3]));
      }
      else
      {
        Receive(buffer, strlen(buffer));
      }
    }

    while( fgets(buffer, _countof(buffer), fp ) )
    {
      char* del = strrchr(buffer, '\n');
      if( del )
        *del = '\0';

      // assume that file is utf8 encoded
      Receive(buffer, strlen(buffer));
    }
    fclose(fp);
  }
}

void LogListView::OnDragLeave()
{
}

BOOL LogListView::OnRenderData( FORMATETC& FormatEtc, STGMEDIUM *pStgMedium, BOOL bDropComplete )
{
  return FALSE;
}

DWORD LogListView::OnSourceDrop(CDragInfo& p_draginfo)
{
  /*
  int fd;
  int oflag = _O_CREAT|_O_RDWR|_O_TEXT;
  #ifdef _UNICODE
  oflag |= _O_U8TEXT;
  #endif

  if( !_tsopen_s( &fd, p_draginfo.m_sTempFilePath, oflag, _SH_DENYNO, _S_IREAD|_S_IWRITE) )
  {
  for( size_t i=0; i<p_draginfo.m_arrItemIndex.size(); i++)
  {
  _write(fd, m_arrLog[i].m_sLog, sizeof(m_arrLog[i].m_sLog));
  _write(fd, _T("\n"), sizeof(TCHAR));
  }
  _close( fd );
  return TRUE;
  }
  */

  FILE* fp = NULL;

  /*
#ifdef _UNICODE
  const TCHAR mode[] = _T("w, ccs=UNICODE");
#else
  const TCHAR mode[] = _T("rw");
#endif
  */
  const TCHAR mode[] = _T("w, ccs=UTF-8");

  _tfopen_s(&fp, p_draginfo.m_sTempFilePath, mode);
  if( fp )
  {
    for( size_t i=0; i<p_draginfo.m_arrItemIndex.size(); i++)
    {
      fwrite(m_arrLog[i]->m_sLog, _tcslen(m_arrLog[i]->m_sLog)*sizeof(TCHAR), 1, fp);
      _fputts( _T("\n"), fp);
    }
    fclose( fp );

    return TRUE;
  }

  return FALSE;
}


LRESULT LogListView::OnDbClick( NMHDR* pNMHDR )
{
  NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

  Dbgp(_T("item[%d] db clicked"), pNMListView->iItem);

  // toggle search result
  m_bSearchResultOnly = !m_bSearchResultOnly; 
  
  int item = pNMListView->iItem;

  if( m_bSearchResultOnly )
  {
    SetItemCountEx(m_nFilterMax, true);
    for( size_t i=0; i<m_nFilterMax && i<m_arrFilterRow.size(); i++)
    {
      if( m_arrFilterRow[i] >= item )
      {
        SelectNone();
        SelectItemEx(i);
        break;
      }
    }
  }
  else
  {
    SetItemCountEx(m_arrLog.size(), true);
    if( (unsigned)item < m_arrFilterRow.size() )
    {
      SelectNone();
      SelectItemEx(m_arrFilterRow[item]);
    }
  }

  int old = GetScrollPos(SB_VERT);
  if( m_nScroll != -1 )
  {
    SIZE sz = { 0, old};
    Scroll(sz);
  }
  m_nScroll = old;
  
  return 0;
}

void LogListView::SetWindowMenu(CMenuHandle menu)
{
  m_PopupMenu = menu;
}