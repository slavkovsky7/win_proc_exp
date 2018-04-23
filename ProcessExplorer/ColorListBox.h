#if !defined(AFX_COLORLISTBOX_H__5529A6B1_584A_11D2_A41A_006097BD277B__INCLUDED_)
#define AFX_COLORLISTBOX_H__5529A6B1_584A_11D2_A41A_006097BD277B__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif

typedef struct ListBoxColor
{
	CString strText;
	COLORREF color;
	ListBoxColor()
	{
		strText.Empty();
		color = RGB(0, 0, 0);
	}
}ListBoxColor;

class CColorListBox : public CListBox
{
public:
	CColorListBox();
	virtual ~CColorListBox();
public:
	using CListBox::AddString;
	int AddString(LPCSTR lpszItem, COLORREF rgb);					
	void SetItemColor(int nIndex, COLORREF rgb);
public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	virtual void ResetContent();
	virtual BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	virtual void OnTimer(UINT_PTR nIDEven);
	virtual afx_msg void OnDestroy();
protected:
	int m_nStep;
	DECLARE_MESSAGE_MAP()
};

#endif // !defined(AFX_COLORLISTBOX_H__5529A6B1_584A_11D2_A41A_006097BD277B__INCLUDED_)
