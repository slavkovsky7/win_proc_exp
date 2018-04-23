
// ProcessExplorerDlg.h : header file
//

#pragma once
#include "ProcessReader.h"
#include "ColorListBox.h"
#include <memory>
#include <mutex>
#include <atomic>
#include <thread>

// CProcessExplorerDlg dialog
class CProcessExplorerDlg : public CDialogEx
{
// Construction
public:
	CProcessExplorerDlg(CWnd* pParent = NULL);	// standard constructor
	CColorListBox colorListBox;
	std::shared_ptr<ProcessReader> processReader;
	std::atomic<bool> threadRunning;
// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PROCESSEXPLORER_DIALOG };
#endif

protected:
	//CWinThread *thread;
	std::thread thread;
	std::mutex threadRunningMutex;
	CEdit edit1;
	CEdit edit2;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	LRESULT OnThreadMessage(WPARAM wParam, LPARAM);
	afx_msg void OnDestroy();

// Implementation
protected:
	HICON m_hIcon;
	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton1();
};
