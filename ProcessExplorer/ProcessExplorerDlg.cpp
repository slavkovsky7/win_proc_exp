
// ProcessExplorerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ProcessExplorer.h"
#include "ProcessExplorerDlg.h"
#include "afxdialogex.h"
#include "ProcessReader.h"
#include <thread>
#include <sstream>
#include <iomanip>
#include "StringUtils.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define WM_MY_THREAD_MESSAGE	WM_APP+100



CProcessExplorerDlg::CProcessExplorerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_PROCESSEXPLORER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	std::vector<std::string> specialProcSubstr = StringUtils::ReadFile("intput.txt");
	processReader = std::shared_ptr<ProcessReader>(new ProcessReader(specialProcSubstr));
	threadRunning = false;
}
void CProcessExplorerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, colorListBox);
	DDX_Control(pDX, IDC_EDIT2, edit2);
	DDX_Control(pDX, IDC_EDIT1, edit1);

}

BEGIN_MESSAGE_MAP(CProcessExplorerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(WM_MY_THREAD_MESSAGE, OnThreadMessage)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUTTON1, &CProcessExplorerDlg::OnBnClickedButton1)
END_MESSAGE_MAP()


LRESULT CProcessExplorerDlg::OnThreadMessage(WPARAM wParam, LPARAM lParam)
{
	int scroll = colorListBox.GetScrollPos(SB_VERT);
	int sel = colorListBox.GetCurSel();
	std::vector<ProcessInfo> procInfos = processReader->GetProcessList();

	colorListBox.SetRedraw(FALSE);
	colorListBox.ResetContent();
	for (auto procInfo : procInfos) {
		std::stringstream ss;
		ss << procInfo.GetProcesId() << " - " << procInfo.GetProcessName() << " - " << procInfo.GetProcessDescription();
		colorListBox.AddString(ss.str().c_str(), procInfo.IsSpecial() ? RGB(255, 0, 0) : RGB(0, 0, 0));
	}

	colorListBox.PostMessage(WM_VSCROLL, MAKEWPARAM(SB_THUMBPOSITION, scroll), 0);
	colorListBox.SetCurSel(sel);
	colorListBox.SetRedraw(TRUE);
	return 0;
}

//TODO premenovat
UINT ProcessUpdateThread(CProcessExplorerDlg *pThis)
{
	//CProcessExplorerDlg* pThis = (CProcessExplorerDlg*)pParam;
	bool privEnabled = pThis->processReader->EnableDebugPrivileges();
	if (!privEnabled) {
		MessageBox(NULL, "Damn. It will not work because of privileges", "Error", MB_OK);
	}

	while (pThis->threadRunning)
	{
		pThis->processReader->Update();
		pThis->SendMessage(WM_MY_THREAD_MESSAGE);
		Sleep(2000);
	}
	return 0;
}

void CProcessExplorerDlg::OnDestroy() {
	threadRunning = false;
	thread.join();
	CDialogEx::OnDestroy();
}

BOOL CProcessExplorerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	threadRunning = true;
	thread = std::thread(ProcessUpdateThread, this);
	edit2.SetWindowText("Special substrs from  input.txt");
	edit1.SetWindowText(StringUtils::JoinStrings(processReader->GetSpecialProcSubstrs()).c_str());
	return TRUE;
}


void CProcessExplorerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	CDialogEx::OnSysCommand(nID, lParam);
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CProcessExplorerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CProcessExplorerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CProcessExplorerDlg::OnBnClickedButton1()
{
	CString str;
	edit1.GetWindowText(str);

	processReader->SetSpecialProcSubstrs(StringUtils::ReadStream(std::string(str)));
}