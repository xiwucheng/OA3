// OAKits.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "OAKits.h"
#include "OAKitsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// COAKitsApp

BEGIN_MESSAGE_MAP(COAKitsApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// COAKitsApp construction

COAKitsApp::COAKitsApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
	hMutex=NULL;
}


// The one and only COAKitsApp object

COAKitsApp theApp;


// COAKitsApp initialization

BOOL COAKitsApp::InitInstance()
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
	//SetRegistryKey(_T("Local AppWizard-Generated Applications"));
	hMutex=OpenMutex(MUTEX_ALL_ACCESS,FALSE,TEXT("OAKits"));
	if(hMutex)
	{
		//AfxMessageBox("程序已经启动，请双击对应的系统图标打开程序！",MB_ICONERROR);
		HWND hWnd=FindWindow(NULL,TEXT("OA3Client v6.3(for Indyde)"));
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
		hMutex=CreateMutex(NULL,FALSE,TEXT("OAKits"));
	}
	SetRegistryKey(_T("OAKits"));

	COAKitsDlg dlg;
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

int COAKitsApp::ExitInstance()
{
	// TODO: Add your specialized code here and/or call the base class
	if(hMutex)
		CloseHandle(hMutex);
	hMutex=NULL;
	return CWinApp::ExitInstance();
}
