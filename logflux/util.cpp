#include "logflux.h"
#include <stdlib.h>
#include "util.h"


TCHAR* stristr(TCHAR* szStringToBeSearched, const TCHAR* szSubstringToSearchFor)
{
  TCHAR* pPos = NULL;
  TCHAR* szCopy1 = NULL;
  TCHAR* szCopy2 = NULL;

  // verify parameters
  if( szStringToBeSearched == NULL || szSubstringToSearchFor == NULL ) 
  {
    return szStringToBeSearched;
  }

  // empty substring - return input (consistent with strstr)
  if ( _tcslen(szSubstringToSearchFor) == 0 ) 
  {
    return szStringToBeSearched;
  }

  szCopy1 = _tcslwr(_tcsdup(szStringToBeSearched));
  szCopy2 = _tcslwr(_tcsdup(szSubstringToSearchFor));

  if ( szCopy1 == NULL || szCopy2 == NULL  ) 
  {
    // another option is to raise an exception here
    free((void*)szCopy1);
    free((void*)szCopy2);
    return NULL;
  }

  pPos = _tcsstr(szCopy1, szCopy2);

  if ( pPos != NULL ) 
  {
    // map to the original string
    pPos = szStringToBeSearched + (pPos - szCopy1);
  }

  free((void*)szCopy1);
  free((void*)szCopy2);

  return pPos;
} 

void GetPopupMenuPos(CListViewCtrl& lv, CPoint& point)
{
	// If the context menu was not opened using the right mouse button,
	// but the keyboard (Shift+F10), get a useful position for the context menu.
	if (point.x == -1 && point.y == -1)
	{
		int iIdxItem = lv.GetNextItem(-1, LVNI_SELECTED | LVNI_FOCUSED);
		if (iIdxItem != -1)
		{
			CRect rc;
			if (lv.GetItemRect(iIdxItem, &rc, LVIR_BOUNDS))
			{
				point.x = rc.left + lv.GetColumnWidth(0) / 2;
				point.y = rc.top + rc.Height() / 2;
				lv.ClientToScreen(&point);
			}
		}
		else
		{
			point.x = 16;
			point.y = 32;
			lv.ClientToScreen(&point);
		}
	}
}