
// ProcessExplorer.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols

// CProcessExplorerApp:
// See ProcessExplorer.cpp for the implementation of this class
//

class CProcessExplorerApp : public CWinApp
{
public:
	CProcessExplorerApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CProcessExplorerApp theApp;