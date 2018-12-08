#include "stdafx.h"
#include "resource.h"
#include "searchtoolbar.h"

CSearchToolBar::CSearchToolBar()
{
}

CSearchToolBar::~CSearchToolBar()
{
  for( size_t i=0; i<m_arrCtrl.size(); i++ )
  {
    delete m_arrCtrl[i];
  }
  m_arrCtrl.clear();
}

HWND CSearchToolBar::InsertCtrl(UINT nIndex, UINT nId, HWND p_hCtrl)
{
  ATLASSERT(p_hCtrl);

  TCtrlInfo* pCtrlInfo = new TCtrlInfo(p_hCtrl, nId);
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
  ::GetWindowRect(p_hCtrl, &rc);

  // Set the button width
  CTBButtonInfo tbi(TBIF_COMMAND|TBIF_SIZE);
  tbi.idCommand = nId;
  tbi.cx = static_cast<WORD>(rc.right - rc.left + DEF_CTRL_PADDING);
  SetButtonInfo(nId, &tbi);

  return p_hCtrl;
}


LRESULT CSearchToolBar::OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
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
        ::GetWindowRect(pCtrlInfo->m_hCtrl, &rcCtrl);

        ::SetWindowPos( pCtrlInfo->m_hCtrl, NULL, 
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

int CSearchToolBar::CalcWidth()
{
  RECT rcItem;
  GetItemRect(GetButtonCount()-1, &rcItem);
  return rcItem.right;
}