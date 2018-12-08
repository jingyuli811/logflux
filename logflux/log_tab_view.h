#ifndef LOG_TAB_VIEW_H
#define LOG_TAB_VIEW_H

#include "logflux.h"

class CLogTabView : public CTabViewImpl<CLogTabView>
{
public:
	DECLARE_WND_CLASS_EX(_T("logflux_TabView"), 0, COLOR_APPWORKSPACE)

	CToolBarCtrl m_wndTB;
	int m_cxTB;

	CString m_strTooltip;

	CLogTabView() : m_cxTB(0)
	{ }

  ~CLogTabView()
  {
    Dbgp("~CLogTabView\n");
  }

// Overrideables - add a toolbar next to the tab control
	bool CreateTabControl()
	{
		bool bRet = CTabViewImpl<CLogTabView>::CreateTabControl();
		if(bRet)
		{
			DWORD dwStyle = (ATL_SIMPLE_TOOLBAR_STYLE | TBSTYLE_FLAT | CCS_NODIVIDER | CCS_NORESIZE | CCS_NOPARENTALIGN) & ~WS_VISIBLE;
			m_wndTB = AtlCreateSimpleToolBar(m_hWnd, IDR_TABTOOLBAR, FALSE, ATL_SIMPLE_TOOLBAR_PANE_STYLE);
      
			// BLOCK: set drop-down style for the window button
			{
				TBBUTTONINFO tbbi;
				tbbi.cbSize = sizeof(tbbi);
				tbbi.dwMask = TBIF_STYLE;
				tbbi.fsStyle = BTNS_BUTTON | BTNS_WHOLEDROPDOWN;
				m_wndTB.SetButtonInfo(ID_WINDOW_SHOW_VIEWS, &tbbi);
			}

			RECT rect = { 0 };
			m_wndTB.GetItemRect(m_wndTB.GetButtonCount() - 1, &rect);
			m_cxTB = rect.right;

			CToolTipCtrl tt = m_tab.GetTooltips();
			tt.SetMaxTipWidth(-1);
		}

		return bRet;
	}

	void UpdateLayout()
	{
		RECT rect;
		GetClientRect(&rect);

		if(m_tab.IsWindow() )//&& m_tab.IsWindowVisible())
			m_tab.SetWindowPos(NULL, 0, 0, rect.right - rect.left - m_cxTB, m_cyTabHeight, SWP_NOZORDER);

		if(m_wndTB.IsWindow() )//&& m_tab.IsWindowVisible())
			m_wndTB.SetWindowPos(NULL, rect.right - m_cxTB, 0, m_cxTB, m_cyTabHeight, SWP_NOZORDER);

		if(m_nActivePage != -1)
			::SetWindowPos(GetPageHWND(m_nActivePage), NULL, 0, m_cyTabHeight, rect.right - rect.left, rect.bottom - rect.top - m_cyTabHeight, SWP_NOZORDER);
	}

	void ShowTabControl(bool bShow)
	{
		m_tab.ShowWindow(bShow ? SW_SHOWNOACTIVATE : SW_HIDE);
		m_wndTB.ShowWindow(bShow ? SW_SHOWNOACTIVATE : SW_HIDE);
	}

	void UpdateTooltipText(LPNMTTDISPINFO pTTDI)
	{
		ATLASSERT(pTTDI != NULL);

    m_strTooltip = GetPageTitle(pTTDI->hdr.idFrom);
    m_strTooltip += _T("\n");

    PLogListView pView = (PLogListView)GetPageData(pTTDI->hdr.idFrom);
    if( pView && pView->IsRunning() )
    {
      TCHAR info[256];
      _stprintf_s(info, _countof(info), _T("%S:%d"), 
        pView->GetLogServer().GetHost(),
        pView->GetLogServer().GetPort());

      m_strTooltip += info;

    }

		pTTDI->lpszText = (LPTSTR)(LPCTSTR)m_strTooltip;
	}
};

#endif