
#include "stdafx.h"
#include "MultiLineListBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CMultiLineListBox::CMultiLineListBox() {
}

CMultiLineListBox::~CMultiLineListBox() {
}


BEGIN_MESSAGE_MAP(CMultiLineListBox, CListBox)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


void CMultiLineListBox::AppendString(LPCSTR lpszText, COLORREF fgColor, COLORREF bgColor) {
	ListBoxColor* pInfo = new ListBoxColor;
	pInfo->strText.Format(_T("%s"), lpszText);
	pInfo->fgColor = fgColor; 
	pInfo->bgColor = bgColor;
	SetItemDataPtr(AddString(pInfo->strText), pInfo);
}

void CMultiLineListBox::InsertString(int index, LPCSTR lpszText, COLORREF fgColor, COLORREF bgColor) {
	ListBoxColor* pInfo = new ListBoxColor;
	pInfo->strText.Format(_T("%s"), lpszText);
	pInfo->fgColor = fgColor;
	pInfo->bgColor = bgColor;
	InsertString(index, pInfo->strText);
	SetItemDataPtr(index, pInfo);
}

void CMultiLineListBox::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct) {
	ASSERT(lpMeasureItemStruct->CtlType == ODT_LISTBOX);
	
	CString strText(_T(""));
	GetText(lpMeasureItemStruct->itemID, strText);
	if (!strText.IsEmpty()) {

		CRect rect;
		GetItemRect(lpMeasureItemStruct->itemID, &rect);

		CDC* pDC = GetDC();
		lpMeasureItemStruct->itemHeight = pDC->DrawText(strText, -1, rect, DT_WORDBREAK | DT_CALCRECT);
		ReleaseDC(pDC);
	}
}

void CMultiLineListBox::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) {
	ASSERT(lpDrawItemStruct->CtlType == ODT_LISTBOX);

	ListBoxColor* pListBox = (ListBoxColor*)GetItemDataPtr(lpDrawItemStruct->itemID);
	if (pListBox != INVALID_HANDLE_VALUE) {
		CDC dc;
		dc.Attach(lpDrawItemStruct->hDC);

		COLORREF crOldTextColor = dc.GetTextColor();
		COLORREF crOldBkColor = dc.GetBkColor();

		if ((lpDrawItemStruct->itemAction | ODA_SELECT) && (lpDrawItemStruct->itemState & ODS_SELECTED)) {
			dc.SetTextColor(pListBox->bgColor);
			dc.SetBkColor(pListBox->fgColor);
			dc.FillSolidRect(&lpDrawItemStruct->rcItem, pListBox->fgColor);
		}
		else {
			dc.SetTextColor(pListBox->fgColor);
			dc.SetBkColor(pListBox->bgColor);
			dc.FillSolidRect(&lpDrawItemStruct->rcItem, pListBox->bgColor);
		}

		lpDrawItemStruct->rcItem.left += 5;
		dc.DrawText(pListBox->strText, pListBox->strText.GetLength(), &lpDrawItemStruct->rcItem, DT_WORDBREAK);

		dc.SetTextColor(crOldTextColor);
		dc.SetBkColor(crOldBkColor);

		dc.Detach();
	}
	else {
		printf("DEBUG BAD");
	}
}


void CMultiLineListBox::OnDestroy() {
	CListBox::OnDestroy();
	int nCount = GetCount();
	for(int i=0; i<nCount; i++)	{
		ListBoxColor* pList = (ListBoxColor*)GetItemDataPtr(i);
		delete pList;
		pList = NULL;
	}
}
