
#include "stdafx.h"
#include "ColorListBox.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CColorListBox::CColorListBox()
{
}

CColorListBox::~CColorListBox()
{
}


BEGIN_MESSAGE_MAP(CColorListBox, CListBox)
	ON_WM_DESTROY()
	ON_WM_MOUSEWHEEL()
	ON_WM_TIMER()
END_MESSAGE_MAP()

void CColorListBox::DrawItem(LPDRAWITEMSTRUCT lpDIS) 
{
	if ((int)lpDIS->itemID < 0)
		return; 
	CDC* pDC = CDC::FromHandle(lpDIS->hDC);

	COLORREF crText;
	ListBoxColor * itemData = (ListBoxColor*)lpDIS->itemData;
	CString sText = itemData->strText;

	COLORREF crNorm = (COLORREF)itemData->color;
	COLORREF crHilite = RGB(255-GetRValue(crNorm), 255-GetGValue(crNorm), 255-GetBValue(crNorm));

	if ((lpDIS->itemState & ODS_SELECTED) && (lpDIS->itemAction & (ODA_SELECT | ODA_DRAWENTIRE))) {
		CBrush brush(crNorm);
		pDC->FillRect(&lpDIS->rcItem, &brush);
	}

	if (!(lpDIS->itemState & ODS_SELECTED) &&	(lpDIS->itemAction & ODA_SELECT)){
		CBrush brush(::GetSysColor(COLOR_WINDOW));
		pDC->FillRect(&lpDIS->rcItem, &brush);
	}	 	

	if ((lpDIS->itemAction & ODA_FOCUS) && (lpDIS->itemState & ODS_FOCUS))
		pDC->DrawFocusRect(&lpDIS->rcItem); 

	if ((lpDIS->itemAction & ODA_FOCUS) &&	!(lpDIS->itemState & ODS_FOCUS))
		pDC->DrawFocusRect(&lpDIS->rcItem); 

	int nBkMode = pDC->SetBkMode(TRANSPARENT);

	if (lpDIS->itemData)		
	{
		if (lpDIS->itemState & ODS_SELECTED)
			crText = pDC->SetTextColor(crHilite);
		else if (lpDIS->itemState & ODS_DISABLED)
			crText = pDC->SetTextColor(::GetSysColor(COLOR_GRAYTEXT));
		else
			crText = pDC->SetTextColor(crNorm);
	} else {
		if (lpDIS->itemState & ODS_SELECTED)
			crText = pDC->SetTextColor(::GetSysColor(COLOR_HIGHLIGHTTEXT));
		else if (lpDIS->itemState & ODS_DISABLED)
			crText = pDC->SetTextColor(::GetSysColor(COLOR_GRAYTEXT));
		else
			crText = pDC->SetTextColor(::GetSysColor(COLOR_WINDOWTEXT));
	}
	CRect rect = lpDIS->rcItem;

	UINT nFormat = DT_LEFT | DT_SINGLELINE | DT_VCENTER;
	if (GetStyle() & LBS_USETABSTOPS)
		nFormat |= DT_EXPANDTABS;
	
	pDC->DrawText(sText, -1, &rect, nFormat | DT_CALCRECT);
	pDC->DrawText(sText, -1, &rect, nFormat);

	pDC->SetTextColor(crText); 
	pDC->SetBkMode(nBkMode);
}

void CColorListBox::MeasureItem(LPMEASUREITEMSTRUCT lpMIS) {
	lpMIS->itemHeight = ::GetSystemMetrics(SM_CYMENUCHECK);
}

int CColorListBox::AddString(LPCSTR lpszItem, COLORREF rgb) {
	ListBoxColor* pInfo = new ListBoxColor;
	pInfo->strText.Format(_T("%s"), lpszItem);
	pInfo->color = rgb;
	int i = AddString(pInfo->strText);
	SetItemDataPtr(i, pInfo);
	return i;
}

void CColorListBox::SetItemColor(int nIndex, COLORREF rgb) {
	SetItemData(nIndex, rgb);	
	RedrawWindow();
}

void CColorListBox::ResetContent() {
	std::vector<ListBoxColor*> ptrsToFree;
	for (int i = 0; i < GetCount(); i++) {
		ListBoxColor* pInfo = (ListBoxColor*)GetItemDataPtr(i);
		ptrsToFree.push_back(pInfo);
	}

	CListBox::ResetContent();

	for (ListBoxColor* pInfo : ptrsToFree) {
		delete pInfo;
		pInfo = NULL;
	}
}

void CColorListBox::OnDestroy() {
	CListBox::OnDestroy();
	for (int i = 0; i < GetCount(); i++) {
		ListBoxColor* pListBoxColor = (ListBoxColor*)GetItemDataPtr(i);
		delete pListBoxColor;
		pListBoxColor = NULL;
	}
}

#define TIMER_SCROLLING 8
BOOL CColorListBox::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
#if 1
	if (zDelta > 0) {
		SetTopIndex(GetTopIndex() - 1);
	} else {
		SetTopIndex(GetTopIndex() + 1);
	}

	return TRUE;
#endif
	KillTimer(TIMER_SCROLLING);
	const int SCROLL_DELTA = 3;
	if (zDelta > 0) {
		m_nStep -= SCROLL_DELTA;
	} else {
		m_nStep += SCROLL_DELTA;
	}
	SetTimer(TIMER_SCROLLING, 20, NULL);

	return TRUE;
}

void CColorListBox::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == TIMER_SCROLLING) {
		if (m_nStep < 0) {
			int nPos = GetTopIndex();
			if (nPos == 0) {
				m_nStep = 0;
				KillTimer(TIMER_SCROLLING);
			} else {
				SetTopIndex(nPos - 1);
				m_nStep++;
			}
		} else if (m_nStep > 0) {
			// scrolling down
			int nPos = GetTopIndex();
			if (nPos == GetCount() - 1) {
				m_nStep = 0;
				KillTimer(TIMER_SCROLLING);
			} else {
				SetTopIndex(nPos + 1);
				m_nStep--;
			}
		} else {
			KillTimer(TIMER_SCROLLING);
		}
	} else {
		CListBox::OnTimer(nIDEvent);
	}
}
