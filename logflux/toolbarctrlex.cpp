#include "logflux.h"
#include "resource.h"
#include "toolbarctrlex.h"

CToolBarCtrlEx::CToolBarCtrlEx()
{
}

CToolBarCtrlEx::~CToolBarCtrlEx()
{
  for( size_t i=0; i<m_arrCtrl.size(); i++ )
  {
    delete m_arrCtrl[i];
  }
  m_arrCtrl.clear();
}

HWND CToolBarCtrlEx::InsertCtrl(UINT nIndex, UINT nId, CWindow* p_pCtrl)
{
  ATLASSERT(p_pCtrl && p_pCtrl->IsWindow());

  TCtrlInfo* pCtrlInfo = new TCtrlInfo(p_pCtrl, nId);
  m_arrCtrl.push_back(pCtrlInfo);

	// insert a placeholder
	CTBButton tb;
	tb.fsState = 0;
	tb.fsStyle = BTNS_SHOWTEXT;//SHOWTEXT;// will allow the button size to be altered
	tb.idCommand = nId;
  tb.dwData = (DWORD)pCtrlInfo;
  
	InsertButton(nIndex, &tb);

  // get rect of ctrl
  RECT rc;
  ::GetWindowRect(p_pCtrl->m_hWnd, &rc);

  //Dbgp(_T("InsertCtrl, index[%d], rect[%d %d %d %d], width[%d]\n"),
  //  nIndex, rc.left, rc.top, rc.right, rc.bottom, rc.right - rc.left );

	// Set the button width
	CTBButtonInfo tbi(TBIF_COMMAND|TBIF_SIZE);
	tbi.idCommand = nId;
  tbi.cx = static_cast<WORD>(rc.right - rc.left + DEF_CTRL_PADDING);
	SetButtonInfo(nId, &tbi);
  
	return p_pCtrl->m_hWnd;
}

CWindow* CToolBarCtrlEx::GetCtrl(UINT nIndex)
{
  for( int i = 0; i < GetButtonCount(); i++ )
  {
    CTBButton btn;
    if( GetButton(i, &btn) )
    {
      RECT rcItem;
      GetItemRect(i, &rcItem);

      // ctrl
      if( btn.dwData )
      {
        TCtrlInfo* pCtrlInfo = (TCtrlInfo*)btn.dwData;
        ATLASSERT( pCtrlInfo->m_pCtrl && pCtrlInfo->m_pCtrl->IsWindow() );
        return pCtrlInfo->m_pCtrl;
      }
    }
  }

  return NULL;
}

LRESULT CToolBarCtrlEx::OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	CRect rectClient;
	GetClientRect(&rectClient);

  int leftmost = 0;
  for( int i = 0; i < GetButtonCount(); i++ )
  {
    CTBButton btn;
    if( GetButton(i, &btn) )
    {
      RECT rcItem;
      GetItemRect(i, &rcItem);

      // ctrl
      if( btn.dwData )
      {
        TCtrlInfo* pCtrlInfo = (TCtrlInfo*)btn.dwData;
        RECT rcCtrl;
        
        ATLASSERT( pCtrlInfo->m_pCtrl && pCtrlInfo->m_pCtrl->IsWindow() );

        ::GetWindowRect(pCtrlInfo->m_pCtrl->m_hWnd, &rcCtrl);

        ::SetWindowPos( pCtrlInfo->m_pCtrl->m_hWnd, NULL, 
                        leftmost, 1, 
                        rcCtrl.right - rcCtrl.left,
                        rcCtrl.bottom - rcCtrl.top, 
                        0);
      }
       
      leftmost = rcItem.right;
    }
  }
	return 0;
}

LRESULT CToolBarCtrlEx::OnWindowPosChanging(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
  if(m_hWnd == NULL)
  {
    bHandled = FALSE;
    return 1;
  }

  LRESULT lRet = DefWindowProc(uMsg, wParam, lParam);

  CRect rectClient;
  GetClientRect(&rectClient);

  //Dbgp(_T("toolbar rect[%d %d %d %d]\n"), 
  //  rectClient.left, rectClient.top, rectClient.right, rectClient.bottom);
  
  int leftmost = 0;
  const int count = GetButtonCount();
  for( int i = 0; i < count; i++ )
  {
    CTBButton btn;
    if( GetButton(i, &btn) )
    {
      RECT rcItem;
      GetItemRect(i, &rcItem);

      //Dbgp(_T("btn[%d], left[%d]\n"), i, leftmost);

      // ctrl
      if( btn.dwData )
      {
        TCtrlInfo* pCtrlInfo = (TCtrlInfo*)btn.dwData;
        RECT rcCtrl;

        ATLASSERT( pCtrlInfo->m_pCtrl && pCtrlInfo->m_pCtrl->IsWindow() );

        ::GetWindowRect(pCtrlInfo->m_pCtrl->m_hWnd, &rcCtrl);

        ::SetWindowPos( pCtrlInfo->m_pCtrl->m_hWnd, NULL, 
          leftmost, 1, 
          rcCtrl.right - rcCtrl.left,
          rcCtrl.bottom - rcCtrl.top, 
          0);


        //Dbgp(_T("set ctrl[%d] left[%d] width[%d] height[%d]\n"), 
        //  i, leftmost, rcCtrl.right - rcCtrl.left, rcCtrl.bottom - rcCtrl.top);
      }

      leftmost = rcItem.right;
    }
  }
  
  return lRet;
}

int CToolBarCtrlEx::CalcWidth()
{
  RECT rcItem;
  GetItemRect(GetButtonCount()-1, &rcItem);
  return rcItem.right;
}

LRESULT CToolBarCtrlEx::OnEraseBkgnd(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
{
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
}