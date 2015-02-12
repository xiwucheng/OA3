// OAKits.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// COAKitsApp:
// See OAKits.cpp for the implementation of this class
//

class COAKitsApp : public CWinApp
{
public:
	COAKitsApp();

// Overrides
	public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
	virtual int ExitInstance();
	private:
		HANDLE hMutex;
};

extern COAKitsApp theApp;