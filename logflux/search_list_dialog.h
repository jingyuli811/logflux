#ifndef SEARCH_LIST_DLG
#define SEARCH_LIST_DLG

#include "loglistview.h"

// TODO
class SearchListDlg : public CDialogImpl<SearchListDlg>
{
public:
  enum { IDD = IDD_SEARCH_LIST };

  SearchListDlg();
  ~SearchListDlg();

  BEGIN_MSG_MAP(SearchListDlg)
    MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
    COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
    COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
    NOTIFY_CODE_HANDLER(NM_DBLCLK, OnDbClick)
  END_MSG_MAP()

  LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
  {
    CenterWindow(GetParent());

    HWND hWndList = GetDlgItem(IDC_SEARCH_LIST);
    //m_wndLogList.SubclassWindow(hWndList);

    return TRUE;
  }

  LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
  {
    ShowWindow(SW_HIDE);
    return 0;
  }

  LRESULT OnDbClick(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/)
  {
    Dbgp(_T("SearchListDlg, OnDbClick"));

    return 0;
  }

private:
  //LogListView  m_wndLogList;
};

#endif // SEARCH_LIST_DLG
