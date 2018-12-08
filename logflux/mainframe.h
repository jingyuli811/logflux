#ifndef MAINFRAME_H
#define MAINFRAME_H

#include "WTL/include/atlframe.h"
//#include <atlsplit.h>
#include "WTL/include/atlmisc.h"
#include "WTL/include/atlctrls.h"
#include "WTL/include/atlctrlw.h"
#include "WTL/include/atlctrlx.h"

#include "Resource.h"
#include "loglistview.h"
#include "toolbarctrlex.h"
#include "log_tab_view.h"
#include "search_pane.h"
#include "search_list_dialog.h"
#include "DragDrop.h"
#include "config.h"

class CMainFrame : public CFrameWindowImpl<CMainFrame>,
                   public CUpdateUI<CMainFrame>,
                   public CMessageFilter, 
                   public CIdleHandler
{
public:
  DECLARE_FRAME_WND_CLASS(NULL, IDR_MAINFRAME)

  CMainFrame();
  ~CMainFrame();

  virtual BOOL PreTranslateMessage(MSG* pMsg);
  virtual BOOL OnIdle();

  BEGIN_MSG_MAP(CMainFrame)
    MESSAGE_HANDLER(WM_CREATE, OnCreate)
    MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
    MESSAGE_HANDLER(WM_HSCROLL, OnHScroll)
    MESSAGE_HANDLER(WM_DROP_FILES, OnDropFiles)

    COMMAND_ID_HANDLER(ID_NEW_LOG, OnNewLog)
    COMMAND_ID_HANDLER(ID_START_LOG, OnStartLog)
    COMMAND_ID_HANDLER(ID_STAY_ON_TOP, OnStayOnTop)
    COMMAND_RANGE_HANDLER(ID_LOG_DEBUG, ID_LOG_ERROR, OnLogLv)
    COMMAND_ID_HANDLER(ID_SAVE_LOG, OnSaveLog)
    COMMAND_ID_HANDLER(ID_CLEAR_LOG, OnClearLog)
    COMMAND_ID_HANDLER(ID_CLEAR_ALL_LOG, OnClearAllLog)
    COMMAND_ID_HANDLER(ID_EDIT_SEARCH, OnSearchUpdate)

    COMMAND_ID_HANDLER(IDM_EXIT, OnExit)
    COMMAND_ID_HANDLER(ID_TOOLS_OPTIONS, OnToolsOptions)
    COMMAND_ID_HANDLER(ID_WINDOW_CLOSE, OnWindowClose)
    COMMAND_ID_HANDLER(ID_WINDOW_CLOSE_ALL, OnWindowCloseAll)
    COMMAND_ID_HANDLER(IDM_ABOUT, OnAbout)

    COMMAND_ID_HANDLER_EX(ID_EVENT_COPY, OnEventCopy)
    COMMAND_ID_HANDLER_EX(ID_EVENT_DELETE, OnEventDelete)
    COMMAND_ID_HANDLER_EX(ID_EVENT_SELECTALL, OnEventSelectAll)
    COMMAND_ID_HANDLER_EX(ID_EVENT_SELECT_REVERSE, OnEventSelectReverse)

    // search pane
    COMMAND_ID_HANDLER_EX(ID_SEARCH_OPEN, OnSearchOpen)
    COMMAND_ID_HANDLER_EX(ID_SEARCH_CLOSE, OnSearchClose)
    COMMAND_ID_HANDLER_EX(ID_SEARCH_PREV, OnSearchPrev)
    COMMAND_ID_HANDLER_EX(ID_SEARCH_NEXT, OnSearchNext)
    COMMAND_ID_HANDLER_EX(ID_SEARCH_HILIGHT, OnSearchHilight)


    COMMAND_RANGE_HANDLER(ID_WINDOW_TABFIRST, ID_WINDOW_TABLAST, OnWindowActivate)
    COMMAND_RANGE_HANDLER(ID_FILTER_FIRST, ID_FILTRR_LAST, OnFilterToggle)
    COMMAND_ID_HANDLER(ID_WINDOW_SHOWTABLIST, OnShowWindows)

    NOTIFY_CODE_HANDLER(TBVN_PAGEACTIVATED, OnTabViewPageActivated)
    NOTIFY_CODE_HANDLER(TBVN_CONTEXTMENU, OnTabViewContextMenu)
    
    NOTIFY_CODE_HANDLER(TBN_DROPDOWN, OnTBDropDown)

    CHAIN_MSG_MAP(CUpdateUI<CMainFrame>)
    CHAIN_MSG_MAP(CFrameWindowImpl<CMainFrame>)
    REFLECT_NOTIFICATIONS()
  END_MSG_MAP()

  BEGIN_UPDATE_UI_MAP(CMainFrame)
    UPDATE_ELEMENT(ID_START_LOG, UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
    UPDATE_ELEMENT(ID_STAY_ON_TOP, UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
    UPDATE_ELEMENT(ID_LOG_DEBUG, UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
    UPDATE_ELEMENT(ID_LOG_INFO, UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
    UPDATE_ELEMENT(ID_LOG_WARN, UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
    UPDATE_ELEMENT(ID_LOG_ERROR, UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
    UPDATE_ELEMENT(ID_SAVE_LOG, UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
    UPDATE_ELEMENT(ID_CLEAR_LOG, UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
    UPDATE_ELEMENT(ID_CLEAR_ALL_LOG, UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
    UPDATE_ELEMENT(ID_EVENT_COPY, UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
    UPDATE_ELEMENT(ID_EVENT_DELETE, UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
    UPDATE_ELEMENT(ID_EVENT_SELECTALL, UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
  END_UPDATE_UI_MAP()

  LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
  LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
  LRESULT OnHScroll(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
  LRESULT OnDropFiles(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
  
  LRESULT OnNewLog(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
  LRESULT OnStartLog(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
  LRESULT OnStayOnTop(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
  LRESULT OnLogLv(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
  LRESULT OnSaveLog(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
  LRESULT OnClearLog(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
  LRESULT OnClearAllLog(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
  LRESULT OnSearchUpdate(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
  LRESULT OnExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
  LRESULT OnToolsOptions(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
  LRESULT OnWindowClose(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
  LRESULT OnWindowCloseAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
  LRESULT OnAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

  void OnEventCopy( UINT uCode, int nID, HWND hwndCtrl );
  void OnEventDelete( UINT uCode, int nID, HWND hwndCtrl );
  void OnEventSelectAll( UINT uCode, int nID, HWND hwndCtrl );
  void OnEventSelectReverse( UINT uCode, int nID, HWND hwndCtrl );

  // search pane
  void OnSearchOpen( UINT uCode, int nID, HWND hwndCtrl );
  void OnSearchClose( UINT uCode, int nID, HWND hwndCtrl );
  void OnSearchPrev( UINT uCode, int nID, HWND hwndCtrl );
  void OnSearchNext( UINT uCode, int nID, HWND hwndCtrl );
  void OnSearchHilight( UINT uCode, int nID, HWND hwndCtrl );

  LRESULT OnWindowActivate(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
  LRESULT OnFilterToggle(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
  LRESULT OnShowWindows(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

  LRESULT OnTabViewPageActivated(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/);
  LRESULT OnTabViewContextMenu(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/);

  LRESULT OnTBDropDown(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled);


  // dragdrop
  DWORD OnDragEnter( FORMATETC& FormatEtc, STGMEDIUM& StgMedium, DWORD dwKeyState, CPoint point );
  DWORD OnDragOver( FORMATETC& FormatEtc, STGMEDIUM& StgMedium, DWORD dwKeyState, CPoint point );
  BOOL OnDrop( FORMATETC& FormatEtc, STGMEDIUM& StgMedium, DWORD dwEffect, CPoint point );
  void OnDragLeave();
  BOOL OnRenderData( FORMATETC& FormatEtc, STGMEDIUM *pStgMedium, BOOL bDropComplete );
  DWORD OnSourceDrop(CDragInfo& p_draginfo);
  void OpenFiles(std::vector<tstring>& arrFileNames);

private:
  HWND CreateCmdBar(HWND p_hParent);
  HWND CreateMainToolBar(HWND p_hParent);

  HWND CreateHostCtrl(HWND p_hParent);
  HWND CreatePortCtrl(HWND p_hParent);
  HWND CreateBufferSizeSlider(HWND p_hParent);

  PLogListView Createlogflux(HWND p_hParent, CTabInfo* pTabInfo);
  void Closelogflux(UINT page);

  HWND CreateSearchBar(HWND p_hParent);

  bool StartServer();

  bool IsPageActive() const
  {
    return (m_wndLogTabView.GetActivePage() != -1);
  }

private:
  CCommandBarCtrl m_wndCmdBar;
  CLogTabView m_wndLogTabView;
  //LogListView m_wndListView;
  std::vector<PLogListView> m_arrLogListView;

  CStatic m_wndStaticHost;
  CEdit   m_wndEditHost;
  CStatic m_wndStaticPort;
  CEdit   m_wndEditPort;
  CStatic m_wndStaticBuffer;
  CStatic m_wndStaticSize;
  CTrackBarCtrl m_wndSliderBufferSize;

  CHorSplitterWindow m_wndSplit;
  CToolBarCtrlEx m_wndMainToolBar;
  SearchPane m_wndSearchPane;
  SearchListDlg m_wndSearchListDlg;

  //LogServer m_LogServer;

  BOOL m_bStayOnTop;
  BOOL m_bStartLog;
  int m_nPrevPage;
  
  CDragDrop<CMainFrame> m_dragdrop;

};

#endif