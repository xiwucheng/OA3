// OAKitsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "OAKits.h"
#include "OAKitsDlg.h"
#include "SerialNumberDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
//MBN_CELLULAR_CLASS_CDMA

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// COAKitsDlg dialog
void COAKitsDlg::init_crc_table()  
{  
    DWORD c;  
    DWORD i, j;  
      
    for (i = 0; i < 256; i++) {  
        c = (DWORD)i;  
        for (j = 0; j < 8; j++) {  
            if (c & 1)  
                c = 0xedb88320L ^ (c >> 1);  
            else  
                c = c >> 1;  
        }  
        CRC_Table[i] = c;  
    }  
}  

DWORD COAKitsDlg::CRC32(DWORD crc,BYTE *buffer, DWORD size)  
{  
    DWORD i;  
    for (i = 0; i < size; i++) {  
        crc = CRC_Table[(crc ^ buffer[i]) & 0xff] ^ (crc >> 8);  
    }  
    return crc ;  
}  

void CALLBACK COAKitsDlg::OnError(void* pOwner, int nErrorCode)
{

}

void CALLBACK COAKitsDlg::OnDisconnect(void* pOwner)
{

}

void CALLBACK COAKitsDlg::OnRead(void* pOwner, char* buf, int len)
{
	COAKitsDlg* p = (COAKitsDlg*)pOwner;
	if (strcmp(buf,"techvision") == 0 && len == 10)
	{
		p->m_bAccept = TRUE;
	}
}


BOOL COAKitsDlg::GetDeviceAddress()
{
    DWORD dwSize = 0;
    DWORD dwRetVal = 0;
	BOOL bWIFI = FALSE,bBT=FALSE,bRet = TRUE;

    unsigned int i;

    /* variables used for GetIfTable and GetIfEntry */
    MIB_IFTABLE *pIfTable;
    MIB_IFROW *pIfRow;

    // Allocate memory for our pointers.
    pIfTable = (MIB_IFTABLE *) MALLOC(sizeof (MIB_IFTABLE));
    if (pIfTable == NULL) {
		//MessageBox("Error allocating memory needed to call GetIfTable","Error",MB_ICONERROR);
        return FALSE;
    }
    // Make an initial call to GetIfTable to get the
    // necessary size into dwSize
    dwSize = sizeof (MIB_IFTABLE);
    if (GetIfTable(pIfTable, &dwSize, FALSE) == ERROR_INSUFFICIENT_BUFFER) {
        FREE(pIfTable);
        pIfTable = (MIB_IFTABLE *) MALLOC(dwSize);
        if (pIfTable == NULL) {
			//MessageBox("Error allocating memory needed to call GetIfTable","Error",MB_ICONERROR); 
            return FALSE;
        }
    }

    // Make a second call to GetIfTable to get the actual
    // data we want.
    if ((dwRetVal = GetIfTable(pIfTable, &dwSize, FALSE)) == NO_ERROR) 
	{
        for (i = 0; i < pIfTable->dwNumEntries; i++) {
            pIfRow = (MIB_IFROW *) & pIfTable->table[i];
            switch (pIfRow->dwType) {
            case IF_TYPE_ETHERNET_CSMACD:
				{
					if (strstr((char*)pIfRow->bDescr,"Bluetooth") && !bBT)
					{
						sprintf(m_KeyInfo.BT,"%02X-%02X-%02X-%02X-%02X-%02X",pIfRow->bPhysAddr[0],
							pIfRow->bPhysAddr[1],
							pIfRow->bPhysAddr[2],
							pIfRow->bPhysAddr[3],
							pIfRow->bPhysAddr[4],
							pIfRow->bPhysAddr[5]);
						bBT = TRUE;
					}
				}
                break;
            case IF_TYPE_IEEE80211:
				{
					if (strstr((char*)pIfRow->bDescr,"802.11") && !bWIFI)
					{
						sprintf(m_KeyInfo.WIFI,"%02X-%02X-%02X-%02X-%02X-%02X",pIfRow->bPhysAddr[0],
							pIfRow->bPhysAddr[1],
							pIfRow->bPhysAddr[2],
							pIfRow->bPhysAddr[3],
							pIfRow->bPhysAddr[4],
							pIfRow->bPhysAddr[5]);
						bWIFI = TRUE;
					}
				}
                break;
            default:
                //printf("Unknown type %ld\n", pIfRow->dwType);
                break;
            }
        }
    }

    if (pIfTable != NULL)
	{
        FREE(pIfTable);
        pIfTable = NULL;
    }

	return bRet;
}

BOOL COAKitsDlg::GetIMEI()
{
    SAFEARRAY *psa = NULL;
	LONG lBound=0;
	BOOL bRet = FALSE;
	MBN_INTERFACE_CAPS InterfaceCaps;
	CComPtr<IMbnInterfaceManager>  pInterfaceMgr = NULL;
	CComPtr<IMbnInterface> pMbnInterface = NULL;
	HRESULT hr=CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if (FAILED(hr)) goto END;
    hr = CoCreateInstance(CLSID_MbnInterfaceManager,NULL,CLSCTX_ALL,IID_IMbnInterfaceManager,(void**)&pInterfaceMgr);
	if (FAILED(hr)) goto END;
	hr = pInterfaceMgr->GetInterfaces(&psa);
	if (FAILED(hr)) goto END;
	SafeArrayGetElement(psa, &lBound, &pMbnInterface);
	if (FAILED(hr)) goto END;
	hr = pMbnInterface->GetInterfaceCapability(&InterfaceCaps);
	if (FAILED(hr)) goto END;
	wchar_t* pBuf = InterfaceCaps.deviceID;
	wcstombs(m_KeyInfo.IMEI,pBuf,wcslen(pBuf));

    SysFreeString(InterfaceCaps.customDataClass);
    SysFreeString(InterfaceCaps.customBandClass);
    SysFreeString(InterfaceCaps.deviceID);
    SysFreeString(InterfaceCaps.manufacturer);
    SysFreeString(InterfaceCaps.model);
    SysFreeString(InterfaceCaps.firmwareInfo);

END:
	pInterfaceMgr = NULL;
	pMbnInterface = NULL;
	CoUninitialize();
	return bRet;
	return 0;
}




COAKitsDlg::COAKitsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(COAKitsDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	memset(&m_KeyInfo,0,sizeof(KeyInfo));
	m_pClient=NULL;
	m_bAccept = FALSE;
	init_crc_table();
}

void COAKitsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(COAKitsDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_START, &COAKitsDlg::OnBnClickedStart)
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_CBR, &COAKitsDlg::OnBnClickedCbr)
	ON_BN_CLICKED(IDC_CLEAR, &COAKitsDlg::OnBnClickedClear)
END_MESSAGE_MAP()


// COAKitsDlg message handlers

BOOL COAKitsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	EnableMenuItem(::GetSystemMenu(m_hWnd,FALSE),SC_CLOSE,MF_BYCOMMAND|MF_DISABLED);
	char szIP[MAX_PATH]={0};
	int port;
	char szDir[MAX_PATH];
	GetCurrentDirectory(MAX_PATH,szDir);
	strcat(szDir,"\\OAKits.ini");
	GetPrivateProfileString("OAKitsCfg","IP","192.168.1.10",szIP,MAX_PATH,szDir);
	CIPAddressCtrl* pIP = (CIPAddressCtrl*)GetDlgItem(IDC_IP);
	struct in_addr addr;
	addr.S_un.S_addr = inet_addr(szIP);
	pIP->SetAddress(addr.S_un.S_un_b.s_b1,addr.S_un.S_un_b.s_b2,addr.S_un.S_un_b.s_b3,addr.S_un.S_un_b.s_b4);
	port = GetPrivateProfileInt("OAKitsCfg","Port",300,szDir);
	SetDlgItemInt(IDC_PORT,port,0);
	m_pClient = new CTCPClient(this);
	m_pClient->m_OnError = OnError;
	m_pClient->m_OnDisconnect = OnDisconnect;
	m_pClient->m_OnRead = OnRead;
	//CloseHandle(CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)KeyIDThread,this,0,NULL));

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void COAKitsDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void COAKitsDlg::OnPaint()
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
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR COAKitsDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void COAKitsDlg::OnBnClickedStart()
{
	// TODO: Add your control notification handler code here
	char szDir[MAX_PATH];
	GetCurrentDirectory(MAX_PATH,szDir);
	strcat(szDir,"\\OAKits.ini");
	CIPAddressCtrl* pIP = (CIPAddressCtrl*)GetDlgItem(IDC_IP);
	CButton* pCheckBox = (CButton*)GetDlgItem(IDC_CHECK1);
	int nCheck = pCheckBox->GetCheck();
	DWORD addr;
	pIP->GetAddress(addr);
	m_ip.Format("%d.%d.%d.%d",(addr&0xff000000)>>24,(addr&0xff0000)>>16,(addr&0xff00)>>8,addr&0xff);
	m_port=GetDlgItemInt(IDC_PORT,0,0);
	WritePrivateProfileString("OAKitsCfg","IP",(LPCSTR)m_ip,szDir);
	CString ip;
	ip.Format("%d",m_port);
	WritePrivateProfileString("OAKitsCfg","Port",(LPCSTR)ip,szDir);
	GetDlgItem(IDC_START)->EnableWindow(0);
	GetDlgItem(IDC_CBR)->EnableWindow(0);
	GetDlgItem(IDOK)->EnableWindow(0);
	if (inet_addr(m_ip) == INADDR_NONE)
	{
		MessageBox("Please input a valid ip","IP Invalid",MB_ICONERROR);
		GetDlgItem(IDC_START)->EnableWindow();
		GetDlgItem(IDC_CBR)->EnableWindow();
		GetDlgItem(IDOK)->EnableWindow();
		return;
	}
	if (m_port < 300 || m_port > 399)
	{
		MessageBox("Please input a valid port[300-399]","Port Invalid",MB_ICONERROR);
		GetDlgItem(IDC_START)->EnableWindow();
		GetDlgItem(IDC_CBR)->EnableWindow();
		GetDlgItem(IDOK)->EnableWindow();
		return;
	}

	BOOL retval;
	PROCESS_INFORMATION pi={0};
	STARTUPINFO si={0};
	SECURITY_ATTRIBUTES sa={0};
	HANDLE hReadPipe,hWritePipe;

	sa.bInheritHandle=TRUE;
	sa.nLength=sizeof SECURITY_ATTRIBUTES;
	sa.lpSecurityDescriptor=NULL;
	retval=CreatePipe(&hReadPipe,&hWritePipe,&sa,0);
	if(retval)
	{
		si.cb=sizeof STARTUPINFO;
		si.wShowWindow=SW_HIDE;
		si.dwFlags=STARTF_USESHOWWINDOW|STARTF_USESTDHANDLES;
		si.hStdOutput=si.hStdError=hWritePipe;
		retval=CreateProcess(NULL,"powershell.exe (get-wmiobject softwarelicensingservice).OA3xOriginalProductKey",&sa,&sa,TRUE,0,NULL,NULL,&si,&pi);
		if(retval)
		{
			DWORD dwLen,dwRead;
			WaitForSingleObject(pi.hThread,INFINITE);//等待命令行执行完毕
			CloseHandle(pi.hThread);
			CloseHandle(pi.hProcess);
			dwLen=GetFileSize(hReadPipe,NULL);
			CString szCmdLine(' ',dwLen);
			retval=ReadFile(hReadPipe,(LPSTR)(LPCTSTR)szCmdLine,dwLen,&dwRead,NULL);
			CloseHandle(hWritePipe);
			CloseHandle(hReadPipe);
			if (szCmdLine.GetLength() != 31)
			{
				goto end;
			}
			//AddLog(szCmdLine);
			if (nCheck == BST_CHECKED)
			{
				if (IDYES == MessageBox("There is a valid key in BIOS, override it?","Key exist",MB_YESNO))
				{
					goto end;
				}
			}
			else
			{
				MessageBox("There is a valid key in BIOS","Key exist",MB_ICONWARNING);
			}
			GetDlgItem(IDC_START)->EnableWindow();
			GetDlgItem(IDC_CBR)->EnableWindow();
			GetDlgItem(IDOK)->EnableWindow();
			return;
		}
	}
end:
	GetDeviceAddress();
	GetIMEI();
	m_hKeyThread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)KeyThread,this,0,NULL);
}

void COAKitsDlg::OnDestroy()
{
	CDialog::OnDestroy();

	// TODO: Add your message handler code here
}

void COAKitsDlg::AddLog(CString log)
{
	CString buff,tmp;
	GetDlgItem(IDC_LOG)->GetWindowText(buff);
	tmp=log;
	tmp+="\r\n";
	buff+=tmp;
	CEdit * output=(CEdit *)GetDlgItem(IDC_LOG);
	output->SetWindowText(buff);
	output->LineScroll(output->GetLineCount());
}

void COAKitsDlg::OnBnClickedCbr()
{
	// TODO: Add your control notification handler code here
	char szDir[MAX_PATH];
	GetCurrentDirectory(MAX_PATH,szDir);
	strcat(szDir,"\\OAKits.ini");
	CIPAddressCtrl* pIP = (CIPAddressCtrl*)GetDlgItem(IDC_IP);
	DWORD addr;
	pIP->GetAddress(addr);
	m_ip.Format("%d.%d.%d.%d",(addr&0xff000000)>>24,(addr&0xff0000)>>16,(addr&0xff00)>>8,addr&0xff);
	m_port=GetDlgItemInt(IDC_PORT,0,0);
	WritePrivateProfileString("OAKitsCfg","IP",(LPCSTR)m_ip,szDir);
	CString ip;
	ip.Format("%d",m_port);
	WritePrivateProfileString("OAKitsCfg","Port",(LPCSTR)ip,szDir);
	GetDlgItem(IDC_START)->EnableWindow(0);
	GetDlgItem(IDC_CBR)->EnableWindow(0);
	GetDlgItem(IDOK)->EnableWindow(0);

	if (inet_addr(m_ip) == INADDR_NONE)
	{
		MessageBox("Please input a valid ip","IP Invalid",MB_ICONERROR);
		GetDlgItem(IDC_START)->EnableWindow();
		GetDlgItem(IDC_CBR)->EnableWindow();
		GetDlgItem(IDOK)->EnableWindow();
		return;
	}
	if (m_port < 300 || m_port > 399)
	{
		MessageBox("Please input a valid port[300-399]","Port Invalid",MB_ICONERROR);
		GetDlgItem(IDC_START)->EnableWindow();
		GetDlgItem(IDC_CBR)->EnableWindow();
		GetDlgItem(IDOK)->EnableWindow();
		return;
	}

	BOOL retval;
	PROCESS_INFORMATION pi={0};
	STARTUPINFO si={0};
	SECURITY_ATTRIBUTES sa={0};
	HANDLE hReadPipe,hWritePipe;
	CFile fp;
	char *szBuf;
	char szKey[64]={0},szTmp[64]={0};

	sa.bInheritHandle=TRUE;
	sa.nLength=sizeof SECURITY_ATTRIBUTES;
	sa.lpSecurityDescriptor=NULL;
	retval=CreatePipe(&hReadPipe,&hWritePipe,&sa,0);
	if(retval)
	{
		si.cb=sizeof STARTUPINFO;
		si.wShowWindow=SW_HIDE;
		si.dwFlags=STARTF_USESHOWWINDOW|STARTF_USESTDHANDLES;
		si.hStdOutput=si.hStdError=hWritePipe;
		retval=CreateProcess(NULL,"powershell.exe (get-wmiobject softwarelicensingservice).OA3xOriginalProductKey",&sa,&sa,TRUE,0,NULL,NULL,&si,&pi);
		if(retval)
		{
			DWORD dwLen,dwRead;
			WaitForSingleObject(pi.hThread,INFINITE);//等待命令行执行完毕
			CloseHandle(pi.hThread);
			CloseHandle(pi.hProcess);
			dwLen=GetFileSize(hReadPipe,NULL);
			CString szCmdLine(' ',dwLen);
			retval=ReadFile(hReadPipe,(LPSTR)(LPCTSTR)szCmdLine,dwLen,&dwRead,NULL);
			if (szCmdLine.GetLength() != 31)
			{
				MessageBox("There is no key in BIOS","CBR Error",MB_ICONERROR);
				GetDlgItem(IDC_START)->EnableWindow();
				GetDlgItem(IDC_CBR)->EnableWindow();
				GetDlgItem(IDOK)->EnableWindow();
				CloseHandle(hWritePipe);
				CloseHandle(hReadPipe);
				return;
			}
		}
		retval=CreateProcess(NULL,"powershell.exe (get-wmiobject win32_operatingsystem).serialnumber",&sa,&sa,TRUE,0,NULL,NULL,&si,&pi);
		if(retval)
		{
			DWORD dwLen,dwRead;
			WaitForSingleObject(pi.hThread,INFINITE);//等待命令行执行完毕
			CloseHandle(pi.hThread);
			CloseHandle(pi.hProcess);
			dwLen=GetFileSize(hReadPipe,NULL);
			CString szCmdLine(' ',dwLen);
			retval=ReadFile(hReadPipe,szKey,dwLen-2,&dwRead,NULL);
			CString str(szKey);
			str.Remove('-');
			str.Delete(0,2);
			str.Delete(str.GetLength()-5,5);
			memset(szKey,0,sizeof(szKey));
			strcpy(szKey,(LPCSTR)str);
			str.ReleaseBuffer();
			CloseHandle(hWritePipe);
			CloseHandle(hReadPipe);
		}
		if (fp.Open("oa3.xml",CFile::modeRead))
		{
			DWORD n = (DWORD)fp.GetLength();
			szBuf = new char[n];
			fp.Read(szBuf,n);
			fp.Close();
			char *pt = strstr(szBuf,"<ProductKeyID>");
			if (pt)
			{
				strncpy(szTmp,pt+14,13);
			}
			delete szBuf;
		}
		if (strcmp(szKey,szTmp))
		{
			MessageBox("key ID does not match with in BIOS","CBR Error",MB_ICONERROR);
			GetDlgItem(IDC_START)->EnableWindow();
			GetDlgItem(IDC_CBR)->EnableWindow();
			GetDlgItem(IDOK)->EnableWindow();
			return;
		}

	}
	AddLog(szKey);
	m_hCBRThread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)CBRThread,this,0,NULL);
}

UINT COAKitsDlg::KeyThread(LPVOID lp)
{
	COAKitsDlg* p = (COAKitsDlg*)lp;
	int iCount=5;
	CProgressCtrl* pProgBar = (CProgressCtrl*)p->GetDlgItem(IDC_PROGRESS1);
	//pProgBar->SetBkColor(RGB(0,224,0));
	pProgBar->SetRange(0,100);
	pProgBar->SendMessage(PBM_SETBARCOLOR, 0, RGB(0,224,0));
	pProgBar->SetPos(0);

	CSerialNumberDlg dlg(p);
	BOOL retval;
	CString szCmd;
	CString szSN,szBSN;
	PROCESS_INFORMATION pi={0};
	STARTUPINFO si={0};
	SECURITY_ATTRIBUTES sa={0};
	HANDLE hReadPipe,hWritePipe;
	sa.bInheritHandle=0;
	sa.nLength=sizeof SECURITY_ATTRIBUTES;
	sa.lpSecurityDescriptor=NULL;
	retval=CreatePipe(&hReadPipe,&hWritePipe,&sa,0);
	si.cb=sizeof STARTUPINFO;
	si.wShowWindow=SW_HIDE;
	si.dwFlags=STARTF_USESHOWWINDOW|STARTF_USESTDHANDLES;
	si.hStdOutput=si.hStdError=hWritePipe;
	DWORD retCode;
	CFile fp;
	char *szBuf=NULL;
	char *pt=NULL;
	DWORD n,len,cnt,cnt2;
	char buff[256] = {0};

	if (!p->m_pClient->IsOpen()) 
	{
		p->m_pClient->Open();
	}
	if (!p->m_pClient->Connect((LPSTR)(LPCSTR)p->m_ip,p->m_port))
	{
		p->MessageBox("connect server failed","Connectivity",MB_ICONERROR);
		goto __end;
	}
	//p->AddLog("get key from server\r\n");
	retval=CreateProcess(NULL,"cmd.exe /c oa3tool.exe /assemble /configfile=oa3tool.cfg",&sa,&sa,0,0,NULL,NULL,&si,&pi);
	WaitForSingleObject(pi.hThread,INFINITE);
	GetExitCodeProcess(pi.hProcess,&retCode);
	if (retCode)
	{
		p->MessageBox("Get key failed","Key",MB_ICONERROR);
		goto __end;
	}
	pProgBar->SetPos(25);
	if (fp.Open("oa3.xml",CFile::modeRead))
	{
		n = fp.GetLength();
		szBuf = new char[n];
		fp.Read(szBuf,n);
		fp.Close();
		pt = strstr(szBuf,"<ProductKeyID>");
		if (pt)
		{
			strncpy(p->m_KeyInfo.PKID,pt+14,13);
		}
		pt = strstr(szBuf,"<ProductKey>");
		if (pt)
		{
			strncpy(p->m_KeyInfo.KEY,pt+12,29);
		}
		delete szBuf;
	}
repeat:
	if (dlg.DoModal()==IDOK)
	{
		szSN = dlg.m_strSN;
	}
	else
	{
		szSN = "0000000000000000000000000";
	}

	if (szSN.GetLength() == 0)
	{
		p->MessageBox("The serial number is empty!","SerialNumber Error",MB_ICONERROR);
		goto repeat;
	}

	szBSN = "cmd.exe /c amidewin.exe /bs \"";
	szBSN += szSN;
	szBSN += "\"";
	CreateProcess(NULL,(LPSTR)(LPCSTR)szBSN,&sa,&sa,0,0,NULL,NULL,&si,&pi);
	WaitForSingleObject(pi.hThread,INFINITE);

	strcpy(p->m_KeyInfo.SN,szSN.GetBuffer(32));
	szSN.ReleaseBuffer();

	pProgBar->SetPos(50);

	//p->AddLog("erase key from bios\r\n");
	retval=CreateProcess(NULL,"cmd.exe /c afuwin.exe /oad",0,0,0,0,NULL,NULL,&si,&pi);
	WaitForSingleObject(pi.hThread,INFINITE);
	pProgBar->SetPos(75);
	//p->AddLog("write key into bios\r\n");
	retval=CreateProcess(NULL,"cmd.exe /c afuwin.exe /aoa3.bin",0,0,0,0,NULL,NULL,&si,&pi);
	WaitForSingleObject(pi.hThread,INFINITE);
	GetExitCodeProcess(pi.hProcess,&retCode);

	if (retCode)
	{
		p->MessageBox("Inject key to BIOS failed","Injection",MB_ICONERROR);
		goto __end;
	}
	len=sizeof(KeyInfo) - 4;
	p->m_KeyInfo.CRC = p->CRC32(0xFFFFFFFF,(BYTE*)&p->m_KeyInfo,len);
	cnt = 3, cnt2 = 10;
	p->m_bAccept = FALSE;
	while (cnt-- >0)
	{
		if (p->m_pClient->SendData((char*)&p->m_KeyInfo,sizeof(KeyInfo)))
		{
			cnt2 = 10;
			while (!p->m_bAccept && cnt2-- > 0)
			{
				Sleep(500);
			}
			if (p->m_bAccept)
			{
				break;
			}
		}
	}
	if (!p->m_bAccept)
	{
		p->MessageBox("Send key information to server failed!","Upload",MB_ICONERROR);
		goto __end;
	}
	pProgBar->SetPos(100);

	p->MessageBox("Send key information to server successfully!","Upload",MB_ICONINFORMATION);

	if (1)
	{
		HANDLE hToken;
		TOKEN_PRIVILEGES tkp;
		OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES|TOKEN_QUERY,&hToken);
		LookupPrivilegeValue(NULL,SE_SHUTDOWN_NAME,&tkp.Privileges[0].Luid);
		tkp.PrivilegeCount = 1;
		tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
		AdjustTokenPrivileges(hToken,FALSE,&tkp,sizeof(TOKEN_PRIVILEGES),NULL,0);
		ExitWindowsEx(EWX_FORCE|EWX_REBOOT,0);
		CloseHandle(hToken);
		goto __ok;
	}

__end:

	pProgBar->SendMessage(PBM_SETBARCOLOR, 0, RGB(255,0,0));
	//pProgBar->SetBkColor(RGB(255,0,0));
	pProgBar->SetPos(100);
__ok:
	CloseHandle(hWritePipe);
	CloseHandle(hReadPipe);
	p->GetDlgItem(IDC_START)->EnableWindow();
	p->GetDlgItem(IDC_CBR)->EnableWindow();
	p->GetDlgItem(IDOK)->EnableWindow();

	return 0;
}

UINT COAKitsDlg::KeyIDThread(LPVOID lp)
{
	COAKitsDlg* p = (COAKitsDlg*)lp;
	BOOL retval;
	PROCESS_INFORMATION pi={0};
	STARTUPINFO si={0};
	SECURITY_ATTRIBUTES sa={0};
	HANDLE hReadPipe,hWritePipe;
	DWORD dwLen,dwRead;
	char szKeyID[64]={0};

	sa.bInheritHandle=TRUE;
	sa.nLength=sizeof SECURITY_ATTRIBUTES;
	sa.lpSecurityDescriptor=NULL;
	retval=CreatePipe(&hReadPipe,&hWritePipe,&sa,0);
	si.cb=sizeof STARTUPINFO;
	si.wShowWindow=SW_HIDE;
	si.dwFlags=STARTF_USESHOWWINDOW|STARTF_USESTDHANDLES;
	si.hStdOutput=si.hStdError=hWritePipe;
	retval=CreateProcess(NULL,"powershell.exe (get-wmiobject win32_operatingsystem).serialnumber",&sa,&sa,TRUE,0,NULL,NULL,&si,&pi);
	WaitForSingleObject(pi.hThread,INFINITE);//等待命令行执行完毕
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
	dwLen=GetFileSize(hReadPipe,NULL);
	retval=ReadFile(hReadPipe,szKeyID,dwLen-2,&dwRead,NULL);
	CString str(szKeyID);
	str.Remove('-');
	str.Delete(0,2);
	str.Delete(str.GetLength()-5,5);
	memset(szKeyID,0,sizeof(szKeyID));
	strcpy(szKeyID,"Product Key ID: ");
	strcat(szKeyID,(LPCSTR)str);
	str.ReleaseBuffer();
	p->SetDlgItemText(IDC_KEYID,szKeyID);

	CloseHandle(hWritePipe);
	CloseHandle(hReadPipe);
	return 0;
}

UINT COAKitsDlg::CBRThread(LPVOID lp)
{
	COAKitsDlg* p = (COAKitsDlg*)lp;
	CProgressCtrl* pProgBar = (CProgressCtrl*)p->GetDlgItem(IDC_PROGRESS1);
	//pProgBar->SetBkColor(RGB(0,224,0));
	pProgBar->SetRange(0,100);
	pProgBar->SendMessage(PBM_SETBARCOLOR, 0, RGB(0,224,0));
	pProgBar->SetPos(0);

	BOOL retval,bOK = FALSE;
	CString szCmd;
	char buff[256] = {0};
	PROCESS_INFORMATION pi={0};
	STARTUPINFO si={0};
	SECURITY_ATTRIBUTES sa={0};
	HANDLE hReadPipe,hWritePipe;
	DWORD retCode;
	int iCount=5;
	sa.bInheritHandle=0;
	sa.nLength=sizeof SECURITY_ATTRIBUTES;
	sa.lpSecurityDescriptor=NULL;
	retval=CreatePipe(&hReadPipe,&hWritePipe,&sa,0);
	si.cb=sizeof STARTUPINFO;
	si.wShowWindow=SW_HIDE;
	si.dwFlags=STARTF_USESHOWWINDOW|STARTF_USESTDHANDLES;
	si.hStdOutput=si.hStdError=hWritePipe;


	if (!p->m_pClient->IsOpen()) 
	{
		p->m_pClient->Open();
	}
	if (!p->m_pClient->Connect((LPSTR)(LPCSTR)p->m_ip,p->m_port))
	{
		p->MessageBox("connect server failed","Connectivity",MB_ICONERROR);
		goto __end;
	}

	//p->AddLog("upload CBR to server\r\n");
	while (iCount-- > 0)
	{
		retval=CreateProcess(NULL,"cmd.exe /c oa3tool.exe /report /configfile=oa3tool.cfg",&sa,&sa,0,0,NULL,NULL,&si,&pi);
		WaitForSingleObject(pi.hThread,INFINITE);//等待命令行执行完毕
		GetExitCodeProcess(pi.hProcess,&retCode);
		if (retCode == 0xc0000134)
		{
			break;
		}
	}

	if (retCode != 0xc0000134)
	{
		p->MessageBox("CBR upload failed","CBR",MB_ICONERROR);
		goto __end;
	}
	pProgBar->SetPos(100);
	p->MessageBox("CBR upload successfully","CBR",MB_ICONINFORMATION);
	goto __ok;
__end:

	pProgBar->SendMessage(PBM_SETBARCOLOR, 0, RGB(255,0,0));
	pProgBar->SetPos(100);
__ok:
	CloseHandle(hWritePipe);
	CloseHandle(hReadPipe);
	p->GetDlgItem(IDC_START)->EnableWindow();
	p->GetDlgItem(IDC_CBR)->EnableWindow();
	p->GetDlgItem(IDOK)->EnableWindow();

	return 0;
}

void COAKitsDlg::OnBnClickedClear()
{
	// TODO: Add your control notification handler code here
	//SetDlgItemText(IDC_LOG,NULL);
}
