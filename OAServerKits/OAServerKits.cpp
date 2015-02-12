// OAServerKits.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "OAServerKits.h"
#include "OAServerKitsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// COAServerKitsApp

BEGIN_MESSAGE_MAP(COAServerKitsApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// COAServerKitsApp construction

COAServerKitsApp::COAServerKitsApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
	hMutex=NULL;
}


// The one and only COAServerKitsApp object

COAServerKitsApp theApp;


// COAServerKitsApp initialization

BOOL COAServerKitsApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}

	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	hMutex=OpenMutex(MUTEX_ALL_ACCESS,FALSE,"OAServerKits");
	if(hMutex)
	{
		//AfxMessageBox("程序已经启动，请双击对应的系统图标打开程序！",MB_ICONERROR);
		HWND hWnd=FindWindow(NULL,"OAServerKits v5.0");
		if (hWnd)
		{
			if(!IsWindowVisible(hWnd))
				ShowWindow(hWnd,SW_SHOW);
			SetForegroundWindow(hWnd);
		}
		return FALSE;
	}
	else
	{
		hMutex=CreateMutex(NULL,FALSE,"OAServerKits");
	}
	SetRegistryKey(_T("OAServerKits"));

	COAServerKitsDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

int COAServerKitsApp::ExitInstance()
{
	// TODO: Add your specialized code here and/or call the base class
	if(hMutex)
		CloseHandle(hMutex);
	hMutex=NULL;
	return CWinApp::ExitInstance();
}
