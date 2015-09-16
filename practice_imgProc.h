
// practice_imgProc.h : main header file for the practice_imgProc application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// Cpractice_imgProcApp:
// See practice_imgProc.cpp for the implementation of this class
//

class Cpractice_imgProcApp : public CWinApp
{
public:
	Cpractice_imgProcApp();


// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// Implementation

public:
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern Cpractice_imgProcApp theApp;
