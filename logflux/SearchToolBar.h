#ifndef SEARCH_TOOLBAR_H
#define SEARCH_TOOLBAR_H

#include <atlctrls.h>
#include <atlctrlw.h>
#include <atlctrlx.h>
#include <vector>

// TBSTYLE_LIST : to show text right after the button
#define ATL_SIMPLE_TOOLBAR_PANE_STYLE_EX	(ATL_SIMPLE_TOOLBAR_PANE_STYLE|TBSTYLE_LIST)

class  CSearchToolBar: public CWindowImpl<CSearchToolBar, CToolBarCtrl>
{
public:
  DECLARE_WND_SUPERCLASS(_T("WinLogView_ToolBar"), CToolBarCtrl::GetWndClassName())

  CSearchToolBar();
  ~CSearchToolBar();

  // wrapper for TBBUTTON .
  class CTBButton : public TBBUTTON
  {
  public:
    /// Constructor
    CTBButton()
    {
      memset(this, 0, sizeof(TBBUTTON));
    }
  };

  // wrapper  for TBBUTTONINFO.
  class CTBButtonInfo : public TBBUTTONINFO
  {
  public:
    // Constructor
    CTBButtonInfo(DWORD dwInitialMask = 0)
    {
      memset(this, 0, sizeof(TBBUTTONINFO));
      cbSize = sizeof(TBBUTTONINFO);
      dwMask = dwInitialMask;
    }
  };

  /// Message map
  BEGIN_MSG_MAP(CSearchToolBar)
    MESSAGE_HANDLER(WM_SIZE, OnSize)
    //		MESSAGE_HANDLER(WM_WINDOWPOSCHANGING, OnWindowPosChanging)
    //	MSG_WM_DESTROY(OnDestroy)
    //COMMAND_CODE_HANDLER(CBN_SELCHANGE, OnCBSelChange)
    //NOTIFY_CODE_HANDLER(TBN_DROPDOWN, OnTBDropDown)
  END_MSG_MAP()

  // insert a ctrl on a toolbar at index 
  HWND InsertCtrl(UINT nIndex, UINT nId, HWND p_hCtrl);

  LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

  int CalcWidth();

private:
  struct TCtrlInfo
  {
    HWND m_hCtrl;
    UINT m_nId;
    TCtrlInfo(HWND p_hCtrl, UINT p_nId) : m_hCtrl(p_hCtrl), m_nId(p_nId){}
  };
  std::vector<TCtrlInfo*>  m_arrCtrl;

};

#endif