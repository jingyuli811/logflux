#ifndef SEARCH_BAR_H
#define SEARCH_BAR_H

#include "toolbarctrlex.h"

class SearchPane : public CWindowImpl<SearchPane, CStatic>
{
protected:
  CEdit m_edit;
  CToolBarCtrlEx m_tb;
  SIZE m_sizeTB;
  SIZE m_sizeEdit;

public:
  DECLARE_WND_SUPERCLASS(_T("logflux_SearchBar"), CStatic::GetWndClassName())

  BEGIN_MSG_MAP(SearchPane)
    MESSAGE_HANDLER(WM_CREATE, OnCreate)
    MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
    MESSAGE_HANDLER(WM_WINDOWPOSCHANGING, OnWindowPosChanging)
    NOTIFY_CODE_HANDLER(TTN_GETDISPINFO, OnToolTipText)
    MESSAGE_HANDLER(WM_MOUSEACTIVATE, OnMouseActivate)
    COMMAND_ID_HANDLER_EX(ID_SEARCH_CLOSE, OnSearchClose)
    COMMAND_ID_HANDLER_EX(ID_SEARCH_PREV, OnSearchPrev)
    COMMAND_ID_HANDLER_EX(ID_SEARCH_NEXT, OnSearchNext)
    COMMAND_ID_HANDLER_EX(ID_SEARCH_HILIGHT, OnSearchHilight)
    COMMAND_HANDLER(ID_EDIT_SEARCH, EN_UPDATE, OnSearchUpdate)

    // chain message map
   // CHAIN_MSG_MAP(CWindowImpl<SearchPane, CStatic>)

    // reflection
    DEFAULT_REFLECTION_HANDLER()
  END_MSG_MAP()

  CEdit& GetEditCtrl() { return m_edit; }

  LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
  {
    LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;

    // let the pane initialize itself
    LRESULT lRet = DefWindowProc(uMsg, wParam, (LPARAM)&pcs);
    
    if(lRet != -1)
    {
      HWND hWndToolBar = AtlCreateSimpleToolBar(m_hWnd, IDR_SEARCHTOOLBAR, FALSE, ATL_SIMPLE_TOOLBAR_PANE_STYLE);

        CToolTipCtrl wndTip = (HWND)::SendMessage(hWndToolBar, TB_GETTOOLTIPS, 0, 0 );
        wndTip.ModifyStyle ( 0, TTS_NOPREFIX );

      RECT rc = { 0, 0, 300, DEF_CTRL_HEIGHT };

      m_edit.Create(hWndToolBar, rc, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_BORDER,
        WS_EX_LEFT | WS_EX_NOPARENTNOTIFY | WS_EX_CLIENTEDGE, ID_EDIT_SEARCH);//| CBS_DROPDOWN | CBS_AUTOHSCROLL);
      m_edit.SetFont(AtlGetDefaultGuiFont());

      m_tb.SubclassWindow(hWndToolBar);
      m_tb.InsertCtrl(1, ID_EDIT_SEARCH, &m_edit);   
      
      RECT rect;
      m_tb.GetItemRect(m_tb.GetButtonCount()-1, &rect);
      m_sizeTB.cx = rect.right;
      m_sizeTB.cy = rect.bottom;
    }

    return lRet;
  }

  LRESULT OnEraseBkgnd(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
  {  
    /*
    CDCHandle dc = (HDC)wParam;
    CWindow wndParent = GetParent();

    // Forward this to the parent window, rebar bands are transparent
  
    POINT pt = { 0, 0 };
    MapWindowPoints(wndParent, &pt, 1);
    dc.OffsetWindowOrg(pt.x, pt.y, &pt);
    LRESULT lRet = wndParent.SendMessage(WM_ERASEBKGND, (WPARAM)dc.m_hDC);
    dc.SetWindowOrg(pt.x, pt.y);

    bHandled = (lRet != 0);
    return lRet;
    */
    return 0;
  }

  LRESULT OnWindowPosChanging(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
  {
    if(m_tb.m_hWnd == NULL)
    {
      bHandled = FALSE;
      return 1;
    }

    LRESULT lRet = DefWindowProc(uMsg, wParam, lParam);

    m_tb.SetWindowPos(NULL, 0, 0, m_sizeTB.cx, m_sizeTB.cy, SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOOWNERZORDER);

    return lRet;
  }

  LRESULT OnToolTipText(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
  {
    SendMessage(GetParent(), WM_NOTIFY, idCtrl, (LPARAM)pnmh);
    return 0;
  }

  LRESULT OnMouseActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
  {
    LRESULT lRet = DefWindowProc(uMsg, wParam, lParam);
    SendMessage(GetTopLevelParent(), WM_COMBOMOUSEACTIVATE, 0, 0L);

    return lRet;
  }

  void OnSearchClose( UINT uCode, int nID, HWND hwndCtrl )
  {
    SendMessage(GetTopLevelParent(), WM_COMMAND, MAKEWPARAM(ID_SEARCH_CLOSE, 0), 
      (LPARAM)0);
  }

  void OnSearchNext( UINT uCode, int nID, HWND hwndCtrl )
  {
    SendMessage(GetTopLevelParent(), WM_COMMAND, MAKEWPARAM(ID_SEARCH_NEXT, 0), 
      (LPARAM)0);
  }

  void OnSearchHilight( UINT uCode, int nID, HWND hwndCtrl )
  {
    SendMessage(GetTopLevelParent(), WM_COMMAND, MAKEWPARAM(ID_SEARCH_HILIGHT, 0), 
      (LPARAM)0);
  }


  void OnSearchPrev( UINT uCode, int nID, HWND hwndCtrl )
  {
    SendMessage(GetTopLevelParent(), WM_COMMAND, MAKEWPARAM(ID_SEARCH_PREV, 0), 
      (LPARAM)0);
  }

  LRESULT OnSearchUpdate(WORD /*wNotifyCode*/, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/)
  {
    return SendMessage(GetTopLevelParent(), WM_COMMAND, MAKEWPARAM(ID_EDIT_SEARCH, 0), 
      (LPARAM)0);
  }
};


#endif