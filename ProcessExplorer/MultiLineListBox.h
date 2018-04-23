#if !defined(AFX_MULITLINELISTBOX_H__D705CB99_9FD0_424E_BD71_027547449AE5__INCLUDED_)
#define AFX_MULITLINELISTBOX_H__D705CB99_9FD0_424E_BD71_027547449AE5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MulitLineListBox.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMulitLineListBox window
typedef struct ListBoxColor
{
	CString strText;
	COLORREF fgColor;
	COLORREF bgColor;
	ListBoxColor()
	{
		strText.Empty();
		fgColor = RGB(0, 0, 0);
		bgColor = RGB(255, 255, 255);
	}
}ListBoxColor, *PListBoxColor;

class CMultiLineListBox : public CListBox
{
public:
	CMultiLineListBox();
	void AppendString(LPCSTR lpszText, COLORREF fgColor, COLORREF bgColor);

	using CListBox::InsertString;
	void InsertString(int index, LPCSTR lpszText, COLORREF fgColor, COLORREF bgColor);
	virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual ~CMultiLineListBox();
protected:
	afx_msg void OnDestroy();
	DECLARE_MESSAGE_MAP()
};

#endif
