#ifndef TOOLBARCTRLEX_H
#define TOOLBARCTRLEX_H

#include "WTL/include/atlctrls.h"
#include "WTL/include/atlctrlw.h"
#include "WTL/include/atlctrlx.h"
#include <vector>

// TBSTYLE_LIST : to show text right after the button
#define ATL_SIMPLE_TOOLBAR_PANE_STYLE_EX	(ATL_SIMPLE_TOOLBAR_PANE_STYLE|TBSTYLE_LIST)

class CToolBarCtrlEx: public CWindowImpl<CToolBarCtrlEx, CToolBarCtrl>
{
public:
  DECLARE_WND_SUPERCLASS(_T("logflux_ToolBar"), CToolBarCtrl::GetWndClassName())

  CToolBarCtrlEx();
  ~CToolBarCtrlEx();

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
	BEGIN_MSG_MAP(CToolBarCtrlEx)
    //MESSAGE_HANDLER(WM_SIZE, OnSize)
    MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
		MESSAGE_HANDLER(WM_WINDOWPOSCHANGING, OnWindowPosChanging)
	//	MSG_WM_DESTROY(OnDestroy)
		//COMMAND_CODE_HANDLER(CBN_SELCHANGE, OnCBSelChange)
    //NOTIFY_CODE_HANDLER(TBN_DROPDOWN, OnTBDropDown)
    //CHAIN_MSG_MAP(CBase)
    MESSAGE_HANDLER(WM_HSCROLL, OnHScroll)
    FORWARD_NOTIFICATIONS()
	END_MSG_MAP()

  // insert a ctrl on a toolbar at index 
  HWND InsertCtrl(UINT nIndex, UINT nId, CWindow* p_pCtrl);
  CWindow* GetCtrl(UINT nIndex);
  
	LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
  LRESULT OnWindowPosChanging(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnEraseBkgnd(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled);
 

  LRESULT OnHScroll(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
  {
    return SendMessage(GetTopLevelParent(), uMsg, wParam, lParam);
  }

  int CalcWidth();

private:
  struct TCtrlInfo
  {
    CWindow* m_pCtrl;
    UINT m_nId;
    TCtrlInfo(CWindow* p_pCtrl, UINT p_nId) : m_pCtrl(p_pCtrl), m_nId(p_nId){}
  };
  std::vector<TCtrlInfo*>  m_arrCtrl;

};

#endif // TOOLBARCTRLEX_H