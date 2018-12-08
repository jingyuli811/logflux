#include "logflux.h"
#include "about_dialog.h"
#include "mainframe.h"
#include "config.h"
#include <shlobj.h>
#include <shlguid.h>

CMainFrame::CMainFrame(): m_bStartLog(FALSE), 
m_bStayOnTop(FALSE), 
m_nPrevPage(-1)
{
}

CMainFrame::~CMainFrame()
{
  Dbgp("CMainFrame::~CMainFrame\n");
}


BOOL CMainFrame::PreTranslateMessage(MSG* pMsg)
{
  if(m_wndLogTabView.PreTranslateMessage(pMsg) != FALSE)
    return TRUE;

  return CFrameWindowImpl<CMainFrame>::PreTranslateMessage(pMsg);
}

BOOL CMainFrame::OnIdle()
{
  // globally
  UISetCheck(ID_STAY_ON_TOP, m_bStayOnTop);
  
  bool bActive = IsPageActive();

  UIEnable(ID_START_LOG, bActive);

  // log view specifically
  if( bActive )
  {
    PLogListView pView = (PLogListView)m_wndLogTabView.GetPageData(m_wndLogTabView.GetActivePage());
    if( pView )
    {
      // toolbar
      for( int i = LOG_DEBUG; i < LOG_LEVEL_COUNT; i++ )
        UISetCheck(ID_LOG_DEBUG+i, pView->IsLogLvEnabled(i));

      UISetCheck(ID_START_LOG, pView->IsRunning());

      ::EnableWindow(m_wndEditHost, !pView->IsRunning());
      ::EnableWindow(m_wndEditPort, !pView->IsRunning());
      ::EnableWindow(m_wndSliderBufferSize, !pView->IsRunning());

      // menu
      UIEnable(ID_SAVE_LOG, pView->GetItemCount()>0);
      UIEnable(ID_CLEAR_LOG, pView->GetItemCount()>0);
      UIEnable(ID_CLEAR_ALL_LOG, pView->GetItemCount()>0);
      UIEnable(ID_EVENT_COPY, pView->GetSelectedCount()>0);
      UIEnable(ID_EVENT_DELETE, pView->GetSelectedCount()>0);
      UIEnable(ID_EVENT_SELECTALL, pView->GetSelectedCount()!=pView->GetItemCount());

      // listview
      //pView->Refresh();
    }
  }
  else
  { 
    UISetCheck(ID_START_LOG, FALSE);
    ::EnableWindow(m_wndEditHost, FALSE);
    ::EnableWindow(m_wndEditPort, FALSE);
    ::EnableWindow(m_wndSliderBufferSize, FALSE);
    UIEnable(ID_SAVE_LOG, FALSE);
    UIEnable(ID_CLEAR_LOG, FALSE);
    UIEnable(ID_CLEAR_ALL_LOG, FALSE);
    UIEnable(ID_EVENT_COPY, FALSE);
    UIEnable(ID_EVENT_DELETE, FALSE);
    UIEnable(ID_EVENT_SELECTALL, FALSE);
  }

  UIEnable(ID_WINDOW_CLOSE, bActive);
  UIEnable(ID_WINDOW_CLOSE_ALL, bActive);
  UIEnable(ID_WINDOW_SHOW_VIEWS, bActive);

  UIUpdateToolBar();

  return FALSE;
}

LRESULT CMainFrame::OnCreate(UINT, WPARAM, LPARAM, BOOL&)
{
  CreateSimpleReBar(ATL_SIMPLE_REBAR_STYLE);

  ////////////////////////////////////////////
  // command bar
  CreateCmdBar(m_hWnd);
  
  ////////////////////////////////////////////
  // tool bar
  HWND hWndMainToolBar = CreateMainToolBar(m_hWnd);  
  AddSimpleReBarBand(hWndMainToolBar, NULL, TRUE, m_wndMainToolBar.CalcWidth(), TRUE);

  ////////////////////////////////////////////
  // split
  m_wndSplit.Create( m_hWnd, rcDefault );

  // create tab view
  HWND hWndTabView = m_wndLogTabView.Create(m_wndSplit, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
  m_wndLogTabView.SetTitleBarWindow(m_hWnd);
  
  m_wndLogTabView.m_bWindowsMenuItem = true;
  CMenuHandle menuMain = m_wndCmdBar.GetMenu();
  m_wndLogTabView.SetWindowMenu(menuMain.GetSubMenu(WINDOW_MENU_POSITION));

  HWND hWndSearchBar = CreateSearchBar(m_wndSplit);

  //HWND hWndRe = CreateSimpleReBarCtrl(m_wndSplit);
  //AddSimpleReBarBandCtrl(hWndRe, hWndSearchBar, 0, NULL, TRUE, m_wndSearchPane.CalcWidth(), TRUE);

  //AddSimpleReBarBand(hWndSearchBar, NULL, TRUE, m_wndSearchPane.CalcWidth(), TRUE);
  
  m_wndSplit.SetSplitterPanes( hWndTabView, hWndSearchBar );

	m_wndSplit.SetSplitterExtendedStyle(SPLIT_BOTTOMALIGNED|SPLIT_NONINTERACTIVE);
  m_wndSplit.SetActivePane(SPLIT_PANE_TOP);
  m_wndSplit.SetSinglePaneMode(SPLIT_PANE_TOP);
	m_wndSplit.m_cxySplitBar = 0;
	m_wndSplit.m_cxyMin = DEF_SEARCH_BAR_HEIGHT;

  m_hWndClient = m_wndSplit;

  ///////////////////////////////////////////
  //
  m_wndSearchListDlg.Create(m_hWnd);

  ///////////////////////////////////////////
  // status bar
  CreateSimpleStatusBar();
  
  UpdateLayout();

  ///////////////////////////////////////////
  // Update UI
  UIAddToolBar(m_wndMainToolBar);

  ///////////////////////////////////////////
  // Message Filter
  CMessageLoop* pLoop = _Module.GetMessageLoop();
  pLoop->AddMessageFilter(this);
  pLoop->AddIdleHandler(this);

  ///////////////////////////////////////////
  // update host & ip
  m_wndEditHost.SetWindowText(DEFAULT_HOST);
  m_wndEditPort.SetWindowText(DEFAULT_PORT);

  // init from config
  const CTabInfoArray& tabinfoarr = theConfig().TabInfoArray();
  for( size_t i = 0; i<tabinfoarr.size(); i++ )
  {
    PLogListView plogflux = Createlogflux(m_wndLogTabView, tabinfoarr[i]);
  }

  // init drag drop
  m_dragdrop.Register(this, TRUE);
  m_dragdrop.AddTargetFormat(CF_HDROP);

  return 0;
}

LRESULT CMainFrame::OnDestroy(UINT, WPARAM, LPARAM, BOOL& bHandled)
{
  for( size_t i=0; i<m_arrLogListView.size(); i++)
  {
    if( m_arrLogListView[i] )
    {
      delete m_arrLogListView[i];
    }
  }
  m_arrLogListView.clear();

  bHandled = FALSE;
  return 1;
}

LRESULT CMainFrame::OnHScroll(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
  if( (HWND)lParam != m_wndSliderBufferSize )
  {
    bHandled = FALSE;
    return 1;
  }
 
  PLogListView pView = (PLogListView)m_wndLogTabView.GetPageData(m_wndLogTabView.GetActivePage());
  if( pView )
  {
    pView->SetBufferSize(m_wndSliderBufferSize.GetPos());

    TCHAR tip[MAX_PATH];
    _stprintf(tip, _T("%d Words"), m_wndSliderBufferSize.GetPos());
    m_wndStaticSize.SetWindowText(tip);
  }

  return 0;
}

HWND CMainFrame::CreateCmdBar(HWND p_hParent)
{
  // create command bar window
  RECT rcCmdBar = { 0, 0, 100, 100 };
  HWND hWndCmdBar = m_wndCmdBar.Create(m_hWnd, rcCmdBar, NULL, ATL_SIMPLE_CMDBAR_PANE_STYLE);
  // atach menu
  m_wndCmdBar.AttachMenu(GetMenu());
  // load command bar images
  m_wndCmdBar.SetImageMaskColor(RGB(255, 0, 255));
  m_wndCmdBar.LoadImages(IDR_MAINFRAME);
  // remove old menu
  SetMenu(NULL);

  AddSimpleReBarBand(m_wndCmdBar);

  return m_wndCmdBar;
}

HWND CMainFrame::CreateMainToolBar(HWND p_hParent)
{
  HWND hWndToolBar = CreateSimpleToolBarCtrl(p_hParent, IDR_MAINFRAME, FALSE, ATL_SIMPLE_TOOLBAR_PANE_STYLE);

//  CToolTipCtrl wndTip = (HWND)::SendMessage(hWndToolBar, TB_GETTOOLTIPS, 0, 0 );
//  wndTip.ModifyStyle ( 0, TTS_NOPREFIX );

  CreateHostCtrl(hWndToolBar);
  CreatePortCtrl(hWndToolBar); 
  CreateBufferSizeSlider(hWndToolBar);

  m_wndMainToolBar.SubclassWindow(hWndToolBar);
  m_wndMainToolBar.InsertCtrl(2, ID_STATIC_HOST, &m_wndStaticHost);
  m_wndMainToolBar.InsertCtrl(3, ID_EDIT_HOST, &m_wndEditHost);
  m_wndMainToolBar.InsertCtrl(4, ID_STATIC_PORT, &m_wndStaticPort);
  m_wndMainToolBar.InsertCtrl(5, ID_EDIT_PORT, &m_wndEditPort);
  m_wndMainToolBar.InsertCtrl(6, ID_STATIC_BUFFER, &m_wndStaticBuffer);
  m_wndMainToolBar.InsertCtrl(7, IDC_SLIDER_BUFFER_SIZE, &m_wndSliderBufferSize);
  m_wndMainToolBar.InsertCtrl(8, ID_STATIC_SIZE, &m_wndStaticSize);

  {
    TBBUTTONINFO tbbi;
    tbbi.cbSize = sizeof(tbbi);
    tbbi.dwMask = TBIF_STYLE;
    tbbi.fsStyle = BTNS_BUTTON | BTNS_WHOLEDROPDOWN;
    m_wndMainToolBar.SetButtonInfo(ID_FILTER_LIST, &tbbi);
  }

  return m_wndMainToolBar;
}

HWND CMainFrame::CreateSearchBar(HWND p_hParent)
{
  RECT rc = { 0, 0, DEF_CTRL_WIDTH, DEF_CTRL_HEIGHT };

  m_wndSearchPane.Create(p_hParent, rc, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN ,
      WS_EX_LEFT, ID_SEARCH_PANE );

  m_wndSearchPane.SetFont(AtlGetDefaultGuiFont());

  return m_wndSearchPane;
}

HWND CMainFrame::CreateHostCtrl(HWND p_hParent)
{
  RECT rc = { 0, 0, 200, DEF_CTRL_HEIGHT };

  m_wndEditHost.Create(p_hParent, rc, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_BORDER,
         WS_EX_LEFT | WS_EX_NOPARENTNOTIFY | WS_EX_CLIENTEDGE, ID_EDIT_HOST);//| CBS_DROPDOWN | CBS_AUTOHSCROLL);
  m_wndEditHost.SetFont(AtlGetDefaultGuiFont());

  RECT rc2 = { 0, 0, 25, DEF_CTRL_HEIGHT };
  m_wndStaticHost.Create(p_hParent, rc2, 0, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | SS_LEFT | SS_CENTERIMAGE,
         WS_EX_LEFT | WS_EX_NOPARENTNOTIFY, ID_STATIC_HOST);
  m_wndStaticHost.SetWindowText(_T("Host"));
  m_wndStaticHost.SetFont(AtlGetDefaultGuiFont());

  return m_wndEditHost;
}

HWND CMainFrame::CreatePortCtrl(HWND p_hParent)
{
  RECT rc = { 0, 0, 50, DEF_CTRL_HEIGHT };

  m_wndEditPort.Create(p_hParent, rc, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_BORDER | ES_NUMBER,
     WS_EX_LEFT | WS_EX_NOPARENTNOTIFY | WS_EX_CLIENTEDGE, ID_EDIT_PORT);//| CBS_DROPDOWN | CBS_AUTOHSCROLL);
  m_wndEditPort.SetFont(AtlGetDefaultGuiFont());

  RECT rc2 = { 0, 0, 25, DEF_CTRL_HEIGHT };
  m_wndStaticPort.Create(p_hParent, rc2, 0, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | SS_LEFT | SS_CENTERIMAGE,
         WS_EX_LEFT | WS_EX_NOPARENTNOTIFY, ID_STATIC_HOST);
  m_wndStaticPort.SetWindowText(_T("Port"));
  m_wndStaticPort.SetFont(AtlGetDefaultGuiFont());

  return m_wndEditPort;
}

HWND CMainFrame::CreateBufferSizeSlider(HWND p_hParent)
{
  RECT rc = { 0, 0, 120, DEF_CTRL_HEIGHT };

  m_wndSliderBufferSize.Create(p_hParent, rc, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | TBS_HORZ ,
    WS_EX_LEFT, IDC_SLIDER_BUFFER_SIZE );//| CBS_DROPDOWN | CBS_AUTOHSCROLL);
  m_wndSliderBufferSize.SetFont(AtlGetDefaultGuiFont());
  m_wndSliderBufferSize.SetRange(DEFAULT_BUFFER_SIZE, MAX_BUFFER_SIZE);
  m_wndSliderBufferSize.SetLineSize(SETP_BUFFER_SIZE);
  m_wndSliderBufferSize.SetPageSize(SETP_BUFFER_SIZE);

   RECT rc2 = { 0, 0, 35, DEF_CTRL_HEIGHT };
   m_wndStaticBuffer.Create(p_hParent, rc2, 0, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | SS_LEFT | SS_CENTERIMAGE,
     WS_EX_LEFT | WS_EX_NOPARENTNOTIFY, ID_STATIC_BUFFER);
   m_wndStaticBuffer.SetWindowText(_T("Buffer"));
   m_wndStaticBuffer.SetFont(AtlGetDefaultGuiFont());

   RECT rc3 = { 0, 0, 60, DEF_CTRL_HEIGHT };
   m_wndStaticSize.Create(p_hParent, rc3, 0, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | SS_LEFT | SS_CENTERIMAGE,
     WS_EX_LEFT | WS_EX_NOPARENTNOTIFY, ID_STATIC_BUFFER);
   TCHAR tip[MAX_PATH];
   _stprintf(tip, _T("%d Words"), m_wndSliderBufferSize.GetPos());
   m_wndStaticSize.SetWindowText(tip);
   m_wndStaticSize.SetFont(AtlGetDefaultGuiFont());

  return m_wndSliderBufferSize;
}

PLogListView CMainFrame::Createlogflux(HWND p_hParent, CTabInfo* pTabInfo)
{
  PLogListView plogflux = new LogListView();

  if( plogflux )
  {  
    // from tab config
    if( pTabInfo )
    {
      if( pTabInfo->m_sIP.length() && pTabInfo->m_nPort )
      {
        plogflux->SetBufferSize( pTabInfo->m_nBufSize );
        plogflux->Initialize(p_hParent, pTabInfo->m_sIP.c_str(), pTabInfo->m_nPort);
        plogflux->SetFilterPack( &pTabInfo->m_Filter );
        plogflux->Start( pTabInfo->m_sIP.c_str(), pTabInfo->m_nPort );
      }
      else if( pTabInfo->m_sFile.length() )
      {
        plogflux->Initialize(p_hParent, "", 0);
        plogflux->OpenFile(pTabInfo->m_sFile);
      }
    }
    // from UI setting
    else
    {
      TCHAR port[128] = {0};
      m_wndEditPort.GetWindowText(port, _countof(port));

      TCHAR host[128] = {0};
      m_wndEditHost.GetWindowText(host, _countof(host));

      USES_CONVERSION;
      plogflux->Initialize(p_hParent, T2A(host), _ttoi(port));
    }

    CMenuHandle menuMain = m_wndCmdBar.GetMenu();
    plogflux->SetWindowMenu(menuMain.GetSubMenu(EDIT_MENU_POSITION));

    m_wndLogTabView.AddPage(plogflux->m_hWnd, (pTabInfo && pTabInfo->m_sTitle.length())?pTabInfo->m_sTitle.c_str():_T("NewView"), 0, plogflux);
    m_arrLogListView.push_back(plogflux);

    m_nPrevPage = m_wndLogTabView.GetActivePage();

  }

  return plogflux;
}

void CMainFrame::Closelogflux(UINT page)
{
  Dbgp("CMainFrame::Closelogflux\n");

  // stop log server
  PLogListView pView = (PLogListView)m_wndLogTabView.GetPageData(page);

  // remove from tab view
  m_wndLogTabView.SetPageData(page, NULL);
  m_wndLogTabView.RemovePage( page );

  if( pView )
  {
    pView->Stop();
    for( std::vector<PLogListView>::iterator it = m_arrLogListView.begin(); 
         it != m_arrLogListView.end(); it++ )
    {
      if( pView == (*it) )
      {
        delete pView;
        m_arrLogListView.erase(it);
        break;
      }
    }
  }
}

LRESULT CMainFrame::OnNewLog(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
  Createlogflux(m_wndLogTabView, NULL);
  return 0;
}

LRESULT CMainFrame::OnStartLog(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
  // server
  /*
  if( StartServer(!m_bStartLog) )
  {
    m_bStartLog = !m_bStartLog;

    // ui
    ::EnableWindow(m_wndEditHost, !m_bStartLog);
    ::EnableWindow(m_wndEditPort, !m_bStartLog);
  }
  */
  StartServer();
  return 0;
}

LRESULT CMainFrame::OnStayOnTop(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
  m_bStayOnTop = !m_bStayOnTop; 
  
  SetWindowPos(m_bStayOnTop?HWND_TOPMOST:HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE );

//long oldstyle = ::GetWindowLong(m_hWnd, GWL_EXSTYLE);
//oldstyle |= WS_EX_TOPMOST;
//::SetWindowLong(m_hWnd, GWL_EXSTYLE, bOnTop ? (oldstyle | WS_EX_TOPMOST) : (oldstyle & ~WS_EX_TOPMOST));
 
  return 0;
}

LRESULT CMainFrame::OnLogLv(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
  int index = wID - ID_LOG_DEBUG;
  PLogListView pView = (PLogListView)m_wndLogTabView.GetPageData(m_wndLogTabView.GetActivePage());
  if( pView )
  {
    pView->ToggleLogLv(index);
  }

  return 0;
}

LRESULT CMainFrame::OnSaveLog(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{  
  PLogListView pView = (PLogListView)m_wndLogTabView.GetPageData(m_wndLogTabView.GetActivePage());
  if( pView )
  {
    pView->SaveFile();
  }

  return 0;
}

LRESULT CMainFrame::OnClearLog(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
  PLogListView pView = (PLogListView)m_wndLogTabView.GetPageData(m_wndLogTabView.GetActivePage());
  if( pView )
  {
    pView->ClearLog();
  }
  return 0;
}

LRESULT CMainFrame::OnClearAllLog(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
  int cnt = m_wndLogTabView.GetPageCount();
  for( int i=0; i<cnt; i++)
  {
    PLogListView pView = (PLogListView)m_wndLogTabView.GetPageData(i);
    if( pView )
    {
      pView->ClearLog();
    }
  }
  return 0;
}

LRESULT CMainFrame::OnSearchUpdate(WORD /*wNotifyCode*/, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/)
{
  TCHAR searchstr[1024];
  m_wndSearchPane.GetEditCtrl().GetWindowText( searchstr, _countof(searchstr));

  PLogListView pView = (PLogListView)m_wndLogTabView.GetPageData(m_wndLogTabView.GetActivePage());
  if( pView )
  {
    pView->SetSearchText(searchstr);
  }

  return 0;
}

LRESULT CMainFrame::OnExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
  ::PostMessage(m_hWnd, WM_CLOSE, 0, 0);
  return 0;
}

LRESULT CMainFrame::OnToolsOptions(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
  //CSimpleDialog<IDD_OPTIONS> dlg;
  //dlg.DoModal();
  return 0;
}

LRESULT CMainFrame::OnWindowClose(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
  if( IsPageActive() )
  {
    m_nPrevPage = -1;
    Closelogflux(m_wndLogTabView.GetActivePage());
  }
  return 0;
}

LRESULT CMainFrame::OnWindowCloseAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
  m_nPrevPage = -1;
  m_wndLogTabView.RemoveAllPages();

  for( std::vector<PLogListView>::iterator it = m_arrLogListView.begin(); 
    it != m_arrLogListView.end(); it++ )
  {
    if( PLogListView pView = (*it) )
    {
      pView->Stop();
    }
  }
  m_arrLogListView.clear();


  return 0;
}

LRESULT CMainFrame::OnAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
  AboutDialog<IDD_ABOUTBOX> dlg;
  dlg.DoModal();

  return 0;
}

void CMainFrame::OnEventCopy( UINT nCode, int nID, HWND hwndCtrl )
{
  PLogListView pView = (PLogListView)m_wndLogTabView.GetPageData(m_wndLogTabView.GetActivePage());
  if( pView )
  {
    pView->Copy();
  }
}

void CMainFrame::OnEventDelete( UINT nCode, int nID, HWND hwndCtrl )
{
  PLogListView pView = (PLogListView)m_wndLogTabView.GetPageData(m_wndLogTabView.GetActivePage());
  if( pView )
  {
    pView->Delete();
  }
}

void CMainFrame::OnEventSelectAll( UINT uCode, int nID, HWND hwndCtrl )
{
  PLogListView pView = (PLogListView)m_wndLogTabView.GetPageData(m_wndLogTabView.GetActivePage());
  if( pView )
  {
    pView->SelectAll();
  }
}

void CMainFrame::OnEventSelectReverse( UINT uCode, int nID, HWND hwndCtrl )
{
  PLogListView pView = (PLogListView)m_wndLogTabView.GetPageData(m_wndLogTabView.GetActivePage());
  if( pView )
  {
    pView->SelectReverse();
  }
}

void CMainFrame::OnSearchOpen( UINT uCode, int nID, HWND hwndCtrl )
{
  m_wndSplit.SetSinglePaneMode(SPLIT_PANE_NONE);

  m_wndSearchPane.GetEditCtrl().SetSel(0, -1);
  m_wndSearchPane.GetEditCtrl().SetFocus();
}

void CMainFrame::OnSearchClose( UINT uCode, int nID, HWND hwndCtrl )
{
  m_wndSplit.SetSinglePaneMode(SPLIT_PANE_TOP);
  PLogListView pView = (PLogListView)m_wndLogTabView.GetPageData(m_wndLogTabView.GetActivePage());
  if( pView )
  {
    pView->SetFocus();
  }
}

void CMainFrame::OnSearchPrev( UINT uCode, int nID, HWND hwndCtrl )
{
  PLogListView pView = (PLogListView)m_wndLogTabView.GetPageData(m_wndLogTabView.GetActivePage());
  if( pView )
  {
    pView->OnFindPrev();
  }
}

void CMainFrame::OnSearchNext( UINT uCode, int nID, HWND hwndCtrl )
{
  PLogListView pView = (PLogListView)m_wndLogTabView.GetPageData(m_wndLogTabView.GetActivePage());
  if( pView )
  {
    pView->OnFindNext();
  }
}

void CMainFrame::OnSearchHilight( UINT uCode, int nID, HWND hwndCtrl )
{
  PLogListView pView = (PLogListView)m_wndLogTabView.GetPageData(m_wndLogTabView.GetActivePage());
  if( pView )
  {
    pView->OnFindHilight();
  }
}

LRESULT CMainFrame::OnDropFiles(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
  SIZE_T sz = (SIZE_T)wParam;
  DROPFILES* pDrop = (DROPFILES*)lParam;
  if( !pDrop )
    return 1;

  std::vector<tstring> arrFileNames;
  if( pDrop->fWide )
  {
    //std::wstring FileName;
    SIZE_T cur = pDrop->pFiles;
    wchar_t* pFnPtr = (wchar_t*)((unsigned char*)pDrop+cur);
    while( cur < sz && pFnPtr && pFnPtr[0] )
    {
      arrFileNames.push_back(pFnPtr);
      cur += (arrFileNames.rbegin()->length()+1)*sizeof(wchar_t);
      pFnPtr = (wchar_t*)((unsigned char*)pDrop+cur);
    }
  }
  else
  {
    SIZE_T cur = pDrop->pFiles;
    char* pFnPtr = (char*)((unsigned char*)pDrop+cur);
    while( cur < sz && pFnPtr && pFnPtr[0] )
    {
      USES_CONVERSION;
      arrFileNames.push_back(A2W(pFnPtr));
      cur += strlen(pFnPtr);
      pFnPtr = (char*)((unsigned char*)pDrop+cur);
    }
  }

  OpenFiles(arrFileNames);

  return 0;
}

bool CMainFrame::StartServer()
{
  PLogListView pView = (PLogListView)m_wndLogTabView.GetPageData(m_wndLogTabView.GetActivePage());
  if( pView )
  {
    if( !pView->GetLogServer().IsRunning() )
    {
      TCHAR port[128] = {0};
      m_wndEditPort.GetWindowText(port, _countof(port));

      TCHAR host[128] = {0};
      m_wndEditHost.GetWindowText(host, _countof(host));

      USES_CONVERSION;
      return pView->Start(T2A(host), _ttoi(port));
    }
    else
    {
      return pView->Stop();
    }
  }

  return false;
}


LRESULT CMainFrame::OnWindowActivate(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
  int nPage = wID - ID_WINDOW_TABFIRST;
  m_wndLogTabView.SetActivePage(nPage);

  return 0;
}

LRESULT CMainFrame::OnFilterToggle(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
  int nItem = wID - ID_FILTER_FIRST;
  PLogListView pView = (PLogListView)m_wndLogTabView.GetPageData(m_wndLogTabView.GetActivePage());
  if( pView )
  {
    TFilterPack* filterpack = (TFilterPack*)pView->GetFilterPack();
    if( filterpack )
    {
      filterpack->Toggle(nItem);
    }
  }

  return 0;
}

LRESULT CMainFrame::OnShowWindows(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
  //CWindowsDlg dlg(&m_view);
  //dlg.DoModal(m_hWnd);

  return 0;
}

LRESULT CMainFrame::OnTabViewPageActivated(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/)
{
  if( m_nPrevPage != -1 && m_wndLogTabView.IsValidPageIndex(m_nPrevPage) )
  {  
    PLogListView pView = (PLogListView)m_wndLogTabView.GetPageData(m_nPrevPage);
    if( pView )
    {
      // save old: 
      TCHAR port[128] = {0};
      m_wndEditPort.GetWindowText(port, _countof(port));

      TCHAR host[128] = {0};
      m_wndEditHost.GetWindowText(host, _countof(host));

      TCHAR searchstr[128] = {0};
      m_wndSearchPane.GetEditCtrl().GetWindowText(searchstr, _countof(searchstr));

      USES_CONVERSION;
      pView->GetLogServer().SetHost(T2A(host));
      pView->GetLogServer().SetAddrPort(_ttoi(port));

      pView->SetBufferSize( m_wndSliderBufferSize.GetPos() );
      pView->SetSearchText( searchstr );
    }
  }

  m_nPrevPage = m_wndLogTabView.GetActivePage();

  if( m_nPrevPage == -1 )
    return 1;

  PLogListView pView = (PLogListView)m_wndLogTabView.GetPageData(m_nPrevPage);
  if( pView )
  {
    // update new
    TCHAR port[128] = {0};
    _stprintf(port, _T("%d"), pView->GetLogServer().GetPort());

    USES_CONVERSION;
    m_wndEditHost.SetWindowText(A2T(pView->GetLogServer().GetHost()));
    m_wndEditPort.SetWindowText(port);

    m_wndSliderBufferSize.SetPos( pView->GetBufferSize() );

    TCHAR tip[MAX_PATH];
    _stprintf(tip, _T("%d Words"), m_wndSliderBufferSize.GetPos());
    m_wndStaticSize.SetWindowText(tip);


    m_wndSearchPane.GetEditCtrl().SetWindowText( pView->GetSearchText() );

  }

  return 0;
}

LRESULT CMainFrame::OnTabViewContextMenu(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/)
{
  LPTBVCONTEXTMENUINFO lpcmi = (LPTBVCONTEXTMENUINFO)pnmh;
  CMenuHandle menuMain = m_wndCmdBar.GetMenu();
  CMenuHandle menuPopup = menuMain.GetSubMenu(WINDOW_MENU_POSITION);
  int nRet = (int)m_wndCmdBar.TrackPopupMenu(menuPopup, TPM_RETURNCMD | TPM_LEFTALIGN | TPM_RIGHTBUTTON, lpcmi->pt.x, lpcmi->pt.y);
  if(nRet == ID_WINDOW_CLOSE)
  {
    Closelogflux(pnmh->idFrom);
  }
  else
  {
    SendMessage(WM_COMMAND, MAKEWPARAM(nRet, 0));
  }

  return 0;
}


LRESULT CMainFrame::OnTBDropDown(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled)
{
  LPNMTOOLBAR lpTB = (LPNMTOOLBAR)pnmh;

  if(lpTB->iItem == ID_WINDOW_SHOW_VIEWS)	// something else
  {
    CToolBarCtrl wndToolBar = pnmh->hwndFrom;
    int nIndex = wndToolBar.CommandToIndex(ID_WINDOW_SHOW_VIEWS);
    RECT rect;
    wndToolBar.GetItemRect(nIndex, &rect);
    wndToolBar.ClientToScreen(&rect);

    CMenu menu;
    menu.CreatePopupMenu();

    m_wndLogTabView.BuildWindowMenu(menu);

    m_wndCmdBar.TrackPopupMenu(menu, TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_VERTICAL, rect.left, rect.bottom);

    return TBDDRET_DEFAULT;
  }
  else if( lpTB->iItem == ID_FILTER_LIST )
  {
    PLogListView pView = (PLogListView)m_wndLogTabView.GetPageData(m_wndLogTabView.GetActivePage());
    if( pView )
    {
      const TFilterPack* filter = pView->GetFilterPack();

      CMenuHandle menuPopup;
      if( !menuPopup.CreatePopupMenu() )
        return 0;

      MENUITEMINFO menuinfo = { sizeof(MENUITEMINFO) };

      menuinfo.fMask = MIIM_FTYPE;
      menuinfo.fType = MFT_SEPARATOR;
      menuPopup.InsertMenuItem(0, TRUE, &menuinfo);

      int posinc = 1;
      for( int i=0; i<(int)filter->m_BlackList.Size(); i++)
      {
        menuinfo.fMask = MIIM_CHECKMARKS|MIIM_FTYPE|MIIM_ID|MIIM_STATE|MIIM_STRING;
        menuinfo.fType = MFT_STRING;
        menuinfo.fState = filter->m_BlackList.IsEnabled(i) ? MFS_CHECKED : MFS_UNCHECKED;
        menuinfo.wID = ID_FILTER_FIRST + i;

        tstring title = CStringConv::AToT(filter->m_BlackList.GetValue(i));
        menuinfo.dwTypeData = (TCHAR*)title.c_str();
        menuinfo.cch = title.length();
        menuPopup.InsertMenuItem(i+posinc, TRUE, &menuinfo);
      }
      posinc += filter->m_BlackList.Size();

      menuinfo.fMask = MIIM_FTYPE;
      menuinfo.fType = MFT_SEPARATOR;
      menuPopup.InsertMenuItem(posinc++, TRUE, &menuinfo);

      for( size_t i = 0; i<filter->m_WhiteList.Size(); i++ )
      {
        menuinfo.fMask = MIIM_CHECKMARKS|MIIM_FTYPE|MIIM_ID|MIIM_STATE|MIIM_STRING;
        menuinfo.fType = MFT_STRING;
        menuinfo.fState = filter->m_WhiteList.IsEnabled(i) ? MFS_CHECKED : MFS_UNCHECKED;
        menuinfo.wID = ID_FILTER_FIRST + i + filter->m_WhiteList.Size();

        tstring title = CStringConv::AToT(filter->m_BlackList.GetValue(i));
        menuinfo.dwTypeData = (TCHAR*)title.c_str();
        menuinfo.cch = title.length();
        menuPopup.InsertMenuItem(i+posinc, TRUE, &menuinfo);
      }

      CToolBarCtrl wndToolBar = pnmh->hwndFrom;
      int nIndex = wndToolBar.CommandToIndex(ID_FILTER_LIST);
      RECT rect;
      wndToolBar.GetItemRect(nIndex, &rect);
      wndToolBar.ClientToScreen(&rect);
      menuPopup.TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTALIGN | TPM_RIGHTBUTTON, 
        rect.left, rect.bottom, m_hWnd);


      // TODO: create popup menu
      /*
      LPTBVCONTEXTMENUINFO lpcmi = (LPTBVCONTEXTMENUINFO)pnmh;
      CMenuHandle menuMain = m_wndCmdBar.GetMenu();
      CMenuHandle menuPopup = menuMain.GetSubMenu(WINDOW_MENU_POSITION);
      int nRet = (int)m_wndCmdBar.TrackPopupMenu(menuPopup, TPM_RETURNCMD | TPM_LEFTALIGN | TPM_RIGHTBUTTON, lpcmi->pt.x, lpcmi->pt.y);
      */

    }
  }

  bHandled = FALSE;
  return 1;
}

DWORD CMainFrame::OnDragEnter( FORMATETC& FormatEtc, STGMEDIUM& StgMedium, DWORD dwKeyState, CPoint point )
{
  return DROPEFFECT_MOVE;
}

DWORD CMainFrame::OnDragOver( FORMATETC& FormatEtc, STGMEDIUM& StgMedium, DWORD dwKeyState, CPoint point )
{
  return DROPEFFECT_MOVE;
}

BOOL CMainFrame::OnDrop( FORMATETC& FormatEtc, STGMEDIUM& StgMedium, DWORD dwEffect, CPoint point )
{
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

void CMainFrame::OpenFiles(std::vector<tstring>& arrFileNames)
{
  for( size_t i=0; i<arrFileNames.size(); i++)
  {
    CTabInfo* pTabInfo = theConfig().NewTabInfo( arrFileNames[i].c_str() );
    if( pTabInfo )
    {
      Createlogflux(m_wndLogTabView, pTabInfo );
    }
    else
    {
      MessageBox(_T("fail to create new tab info"), _T("logflux"), MB_OK|MB_ICONERROR);
    }
  }
}

void CMainFrame::OnDragLeave()
{
}

BOOL CMainFrame::OnRenderData( FORMATETC& FormatEtc, STGMEDIUM *pStgMedium, BOOL bDropComplete )
{
  return FALSE;
}

DWORD CMainFrame::OnSourceDrop(CDragInfo& p_draginfo)
{
  return FALSE;
}