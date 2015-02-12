// OAServerKits.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// COAServerKitsApp:
// See OAServerKits.cpp for the implementation of this class
//

class COAServerKitsApp : public CWinApp
{
public:
	COAServerKitsApp();

// Overrides
	public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
	virtual int ExitInstance();
	private:
		HANDLE hMutex;
};

extern COAServerKitsApp theApp;