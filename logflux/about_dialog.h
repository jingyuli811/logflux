#ifndef ABOUT_DIALOG_H
#define ABOUT_DIALOG_H

#include "atlwin.h"
#include "resource.h"
#include "version.h"

#define MAKE_VER_INFO(a,b,c) (a ## "\n" b ## "\n" c)

#define _ANSISTRING(text) #text
#define ANSISTRING(text) _ANSISTRING(text)

template <WORD t_wDlgTemplateID, BOOL t_bCenter  = TRUE >
class AboutDialog : public CSimpleDialog<t_wDlgTemplateID, t_bCenter>
{
public:

  typedef CSimpleDialog<t_wDlgTemplateID, t_bCenter>	thisClass;
  BEGIN_MSG_MAP(thisClass)
    MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
    CHAIN_MSG_MAP(thisClass)
  END_MSG_MAP()

  LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
  {
    // initialize controls in dialog with DLGINIT resource section
    thisClass::OnInitDialog(uMsg, wParam, lParam, bHandled);

    HWND ver = GetDlgItem(IDC_VER_INFO);

    ::SetWindowTextA(ver, "logflux");

    return TRUE;
  }
};

#endif