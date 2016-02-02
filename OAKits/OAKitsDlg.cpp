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
public:
//	afx_msg void OnTimer(UINT_PTR nIDEvent);
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
//	ON_WM_TIMER()
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
	if (strcmp(buf,"authorized") == 0 && len == 10)
	{
		p->m_bHandshake = TRUE;
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
}




COAKitsDlg::COAKitsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(COAKitsDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	memset(&m_KeyInfo,0,sizeof(KeyInfo));
	m_pClient=NULL;
	m_bAccept = FALSE;
	m_bHandshake = FALSE;
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
	ON_BN_CLICKED(IDC_CBR, &COAKitsDlg::OnBnClickedCbr)
	ON_WM_TIMER()
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
	m_pClient = new CTCPClient(this);
	m_pClient->m_OnError = OnError;
	m_pClient->m_OnDisconnect = OnDisconnect;
	m_pClient->m_OnRead = OnRead;

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
	CFile fp;
	DWORD dwLen;
	char* szBuf;
	memset(m_ip,0,sizeof(m_ip));
	m_port = 4000;
	GetDlgItem(IDC_START)->EnableWindow(0);
	GetDlgItem(IDC_CBR)->EnableWindow(0);
	GetDlgItem(IDOK)->EnableWindow(0);

	if (fp.Open(TEXT("oa3tool.cfg"),CFile::modeRead))
	{
		dwLen = (DWORD)fp.GetLength();
		szBuf = new char[dwLen];
		fp.Read(szBuf,dwLen);
		fp.Close();
		char *pt = strstr(szBuf,"<IPAddress>");
		char *pt2 = strstr(szBuf,"</IPAddress>");
		if (pt && pt2)
		{
			strncpy(m_ip,pt+11,pt2-pt-11);
		}
		delete szBuf;
	}

	if (inet_addr(m_ip) == INADDR_NONE)
	{
		MessageBox(TEXT("IP地址无效，请核对一下oa3tool.cfg文件中的IP地址是否正确"),TEXT("无效的IP地址"),MB_ICONERROR);
		GetDlgItem(IDC_START)->EnableWindow();
		GetDlgItem(IDC_CBR)->EnableWindow();
		GetDlgItem(IDOK)->EnableWindow();
		return;
	}
	KillTimer(1);
	m_hKeyThread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)KeyThread,this,0,NULL);
}

void COAKitsDlg::OnDestroy()
{
	CDialog::OnDestroy();

	// TODO: Add your message handler code here
}

void COAKitsDlg::AddLog(CString log)
{
	//CString buff,tmp;
	//GetDlgItem(IDC_LOG)->GetWindowText(buff);
	//tmp=log;
	//tmp+=TEXT("\r\n");
	//buff+=tmp;
	//CEdit * output=(CEdit *)GetDlgItem(IDC_LOG);
	//output->SetWindowText(buff);
	//output->LineScroll(output->GetLineCount());
}

UINT COAKitsDlg::KeyThread(LPVOID lp)
{
	COAKitsDlg* p = (COAKitsDlg*)lp;

	CSerialNumberDlg dlg(p);
	CString szSN,szBSN;
	HANDLE hReadPipe,hWritePipe;
	char szTmp[1024]={0}, szKeyID[32] = {0}, szKey[32] = {0}, szFileKeyID[32] = {0}, *szBuf=NULL, *pt=NULL;
	wchar_t wszTmp[1024]={0},wszPKID[64]={0};
	DWORD len,cnt,cnt2,retCode,dwLen,dwRead;
	BOOL bHasKey = FALSE,bHasCBR = FALSE,retval;
	CFile fp;
	int iCount;
	PROCESS_INFORMATION pi={0};
	STARTUPINFOA si={0};
	SECURITY_ATTRIBUTES sa={0};

	sa.bInheritHandle=1;
	sa.nLength=sizeof SECURITY_ATTRIBUTES;
	sa.lpSecurityDescriptor=NULL;
	retval=CreatePipe(&hReadPipe,&hWritePipe,&sa,0);
	si.cb=sizeof STARTUPINFOA;
	si.wShowWindow=SW_HIDE;
	si.dwFlags=STARTF_USESHOWWINDOW|STARTF_USESTDHANDLES;
	si.hStdOutput=si.hStdError=hWritePipe;

	p->SetDlgItemText(IDC_STATUS,TEXT("正在连接服务器......"));
	if (!p->m_pClient->IsOpen()) 
	{
		p->m_pClient->Open();
	}
	if (!p->m_pClient->Connect(p->m_ip,p->m_port))
	{
		p->MessageBox(TEXT("连接管理服务器失败"),TEXT("连接出错"),MB_ICONERROR);
		goto __end;
	}

	p->m_bHandshake = FALSE;
	if (!p->m_pClient->SendData("handshake",strlen("handshake")))
	{
		p->MessageBox(TEXT("与服务器匹配信息出错"),TEXT("认证出错"),MB_ICONERROR);
		goto __end;
	}
	cnt = 5;
	while (cnt-- >0 && !p->m_bHandshake)
	{
		Sleep(1000);
	}
	if (!p->m_bHandshake)
	{
		p->MessageBox(TEXT("与服务器匹配信息出错"),TEXT("认证出错"),MB_ICONERROR);
		goto __end;
	}

	p->SetDlgItemText(IDC_STATUS,TEXT("正在查询机器序列号......"));
	retval=CreateProcessA(NULL,"powershell.exe (get-wmiobject softwarelicensingservice).OA3xOriginalProductKey",&sa,&sa,TRUE,0,NULL,NULL,&si,&pi);
	if(retval)
	{
		WaitForSingleObject(pi.hThread,INFINITE);//等待命令行执行完毕
		dwLen=GetFileSize(hReadPipe,NULL);
		retval=ReadFile(hReadPipe,szKey,dwLen,&dwRead,NULL);
		if (strlen(szKey) == 31)
		{
			bHasKey = TRUE;
		}
	}

	if (!bHasKey)
	{
		p->SetDlgItemText(IDC_STATUS,TEXT("正在从服务器获取KEY......"));
		retval=CreateProcessA(NULL,"cmd.exe /c oa3tool.exe /assemble /configfile=oa3tool.cfg",&sa,&sa,0,0,NULL,NULL,&si,&pi);
		WaitForSingleObject(pi.hThread,INFINITE);
		GetExitCodeProcess(pi.hProcess,&retCode);
		if (retCode)
		{
			p->MessageBox(TEXT("获取Key失败"),TEXT("错误"),MB_ICONERROR);
			p->SetDlgItemText(IDC_STATUS,TEXT("获取Key失败......"));
			goto __end;
		}
		p->GetDeviceAddress();
		p->GetIMEI();
		Sleep(500);
		if (fp.Open(TEXT("oa3.xml"),CFile::modeRead))
		{
			dwLen = (DWORD)fp.GetLength();
			szBuf = new char[dwLen];
			fp.Read(szBuf,dwLen);
			fp.Close();
			pt = strstr(szBuf,"<ProductKeyID>");
			if (pt)
			{
				strncpy(p->m_KeyInfo.PKID,pt+14,13);
				strncpy(szFileKeyID,pt+14,13);
			}
			pt = strstr(szBuf,"<ProductKey>");
			if (pt)
			{
				strncpy(p->m_KeyInfo.KEY,pt+12,29);
			}
			delete szBuf;
		}
		else
		{
			p->MessageBox(TEXT("打开Key文件失败，请确认刷KEY工具或刷KEY盘中oa3.xml文件可读！"),TEXT("错误"),MB_ICONERROR);
			goto __end;
		}
		p->SetDlgItemText(IDC_STATUS,TEXT("等待主板序列号的输入......"));
repeat:
		if (dlg.DoModal()==IDOK)
		{
			szSN = dlg.m_strSN;
		}
		else
		{
			//szSN = TEXT("0000000000000000000000000");
			goto skip;
		}

		/*
		if (szSN.GetLength() == 0)
		{
			p->MessageBox(TEXT("主板序列号不能为空，请输入序列号按OK确认，取消则默认序列号为0"),TEXT("错误"),MB_ICONERROR);
			goto repeat;
		}
*/
		p->SetDlgItemText(IDC_STATUS,TEXT("正在写入序列号......"));
		szBSN = TEXT("cmd.exe /c amidewin.exe /bs \"");
		szBSN += szSN;
		szBSN += TEXT("\"");
		memset(wszTmp,0,sizeof(wszTmp));
		wcscpy(wszTmp,szBSN.GetBuffer(1024));
		wcstombs(szTmp,wszTmp,1024);
		szBSN.ReleaseBuffer();
		CreateProcessA(NULL,szTmp,&sa,&sa,0,0,NULL,NULL,&si,&pi);
		WaitForSingleObject(pi.hThread,INFINITE);

		//memset(wszTmp,0,sizeof(wszTmp));
		//wcscpy(wszTmp,szSN.GetBuffer(32));
		//wcstombs(p->m_KeyInfo.BSN,wszTmp,32);
		//szSN.ReleaseBuffer();
skip:
		p->SetDlgItemText(IDC_STATUS,TEXT("正在刷入KEY码......"));
		retval=CreateProcessA(NULL,"cmd.exe /c afuwin.exe /oad",0,0,0,0,NULL,NULL,&si,&pi);
		WaitForSingleObject(pi.hThread,INFINITE);
		CreateProcessA(NULL,"cmd.exe /c afuwin.exe /aoa3.bin",0,0,0,0,NULL,NULL,&si,&pi);
		WaitForSingleObject(pi.hThread,INFINITE);
		GetExitCodeProcess(pi.hProcess,&retCode);

		if (retCode)
		{
			p->MessageBox(TEXT("Key 刷写失败"),TEXT("错误"),MB_ICONERROR);
			p->SetDlgItemText(IDC_STATUS,TEXT("Key刷写失败！"));
			goto __end;
		}
		//----------------------------------------------------------------------
		retval=CreateProcessA(NULL,"cmd.exe /c amidewin.exe /bs",&sa,&sa,TRUE,0,NULL,NULL,&si,&pi);
		if(retval)
		{
			WaitForSingleObject(pi.hThread,INFINITE);//等待命令行执行完毕
			CloseHandle(pi.hThread);
			CloseHandle(pi.hProcess);
			dwLen=GetFileSize(hReadPipe,NULL);
			char *buf=new char[dwLen+1];
			retval=ReadFile(hReadPipe,buf,dwLen,&dwRead,NULL);
			if (strlen(buf))
			{
				char* p1=strchr(buf,'"');
				if (p1)
				{
					p1++;
					char* p2=strchr(p1,'"');
					if (p2)
					{
						*p2 = 0;
						strcpy(p->m_KeyInfo.BSN,p1);
					}
				}
			}
			delete buf;
		}
		//----------------------------------------------------------------------
		retval=CreateProcessA(NULL,"cmd.exe /c amidewin.exe /ss",&sa,&sa,TRUE,0,NULL,NULL,&si,&pi);
		if(retval)
		{
			WaitForSingleObject(pi.hThread,INFINITE);//等待命令行执行完毕
			CloseHandle(pi.hThread);
			CloseHandle(pi.hProcess);
			dwLen=GetFileSize(hReadPipe,NULL);
			char *buf=new char[dwLen+1];
			retval=ReadFile(hReadPipe,buf,dwLen,&dwRead,NULL);
			if (strlen(buf))
			{
				char* p1=strchr(buf,'"');
				if (p1)
				{
					p1++;
					char* p2=strchr(p1,'"');
					if (p2)
					{
						*p2 = 0;
						strcpy(p->m_KeyInfo.SSN,p1);
					}
				}
			}
			delete buf;
		}
		//----------------------------------------------------------------------

		len=sizeof(KeyInfo) - 4;
		p->m_KeyInfo.CRC = p->CRC32(0xFFFFFFFF,(BYTE*)&p->m_KeyInfo,len);
		cnt = 1, cnt2 = 10;
		p->m_bAccept = FALSE;
		while (cnt-- >0)
		{
			if (p->m_pClient->SendData((char*)&p->m_KeyInfo,sizeof(KeyInfo)))
			{
				cnt2 = 10;
				while (!p->m_bAccept && cnt2-- > 0)
				{
					Sleep(1000);
				}
				if (p->m_bAccept)
				{
					break;
				}
			}
		}
		if (!p->m_bAccept)
		{
			p->MessageBox(TEXT("平板信息上传出错了，需要回收此Key重刷！"),TEXT("上传出错"),MB_ICONERROR);
			p->SetDlgItemText(IDC_STATUS,TEXT("上传平板信息出错......"));
			goto __end;
		}

		p->SetDlgItemText(IDC_STATUS,TEXT("正在对比CBR报告，请稍候......"));

		memset(szKey,0,sizeof(szKey));
		retval=CreateProcessA(NULL,"powershell.exe (get-wmiobject softwarelicensingservice).OA3xOriginalProductKey",&sa,&sa,TRUE,0,NULL,NULL,&si,&pi);
		if(retval)
		{
			WaitForSingleObject(pi.hThread,INFINITE);//等待命令行执行完毕
			dwLen=GetFileSize(hReadPipe,NULL);
			retval=ReadFile(hReadPipe,szKey,dwLen,&dwRead,NULL);
		}

		if (strncmp(szKey,p->m_KeyInfo.KEY,strlen(p->m_KeyInfo.KEY)) == 0)//检查机器中的KEY是否和CBR中的一致
		{
			p->SetDlgItemText(IDC_STATUS,TEXT("正在上传CBR 报告......"));
			iCount = 5;
			while (iCount-- > 0)
			{
				retval=CreateProcessA(NULL,"cmd.exe /c oa3tool.exe /report /configfile=oa3tool.cfg",&sa,&sa,0,0,NULL,NULL,&si,&pi);
				WaitForSingleObject(pi.hThread,INFINITE);//等待命令行执行完毕
				GetExitCodeProcess(pi.hProcess,&retCode);
				if (retCode == 0)
				{
					bHasCBR = TRUE;
				}
				if (retCode == 0xc0000134)
				{
					break;
				}
			}
			if (retCode == 0xc0000134)
			{
				if (bHasCBR)
				{
					mbstowcs(wszPKID,p->m_KeyInfo.PKID,32);
					p->SetDlgItemText(IDC_PKID,wszPKID);
					p->MessageBox(TEXT("CBR 上传成功！"),TEXT("CBR"),MB_ICONINFORMATION);
					p->SetDlgItemText(IDC_STATUS,TEXT("CBR 上传成功！......"));
				}
				else
				{
					p->MessageBox(TEXT("CBR 已上传，请不要重复提交！"),TEXT("CBR"),MB_ICONWARNING);
					p->SetDlgItemText(IDC_STATUS,TEXT("CBR 已上传，请误重复提交！......"));
				}
				goto __end;
			}
			else
			{
				p->MessageBox(TEXT("CBR 上传失败！"),TEXT("CBR"),MB_ICONERROR);
				p->SetDlgItemText(IDC_STATUS,TEXT("CBR 上传失败......"));
				goto __end;
			}
		}
		else//刷完后两者不一致，需要重启才能生效
		{
			p->MessageBox(TEXT("需要重启机器才能上传CBR，按OK立即重启!"),TEXT("警告"),MB_ICONWARNING);
			p->Reboot();
			goto __end;
		}
	}
	else
	{
		p->SetDlgItemText(IDC_STATUS,TEXT("正在上传CBR 报告......"));
		iCount = 5;
		while (iCount-- > 0)
		{
			retval=CreateProcessA(NULL,"cmd.exe /c oa3tool.exe /report /configfile=oa3tool.cfg",&sa,&sa,0,0,NULL,NULL,&si,&pi);
			WaitForSingleObject(pi.hThread,INFINITE);//等待命令行执行完毕
			GetExitCodeProcess(pi.hProcess,&retCode);
			if (retCode == 0)
			{
				bHasCBR = TRUE;
			}
			if (retCode == 0xc0000134)
			{
				break;
			}
		}
		if (retCode == 0xc0000134)
		{
			if (bHasCBR)
			{
				mbstowcs(wszPKID,szKeyID,32);
				p->SetDlgItemText(IDC_PKID,wszPKID);
				p->MessageBox(TEXT("CBR 上传成功！"),TEXT("CBR"),MB_ICONINFORMATION);
				p->SetDlgItemText(IDC_STATUS,TEXT("CBR 上传成功！......"));
			}
			else
			{
				p->MessageBox(TEXT("CBR 已上传，请不要重复提交！"),TEXT("CBR"),MB_ICONWARNING);
				p->SetDlgItemText(IDC_STATUS,TEXT("CBR 已上传，请勿重复提交！......"));
			}
			goto __end;
		}
		else
		{
			p->MessageBox(TEXT("CBR 上传失败！"),TEXT("CBR"),MB_ICONERROR);
			p->SetDlgItemText(IDC_STATUS,TEXT("CBR 上传失败......"));
			goto __end;
		}
	}

__end:
	p->SetTimer(1,5000,NULL);
	CloseHandle(hWritePipe);
	CloseHandle(hReadPipe);
	p->GetDlgItem(IDC_START)->EnableWindow();
	p->GetDlgItem(IDC_CBR)->EnableWindow();
	p->GetDlgItem(IDOK)->EnableWindow();

	return 0;
}

void COAKitsDlg::OnBnClickedCbr()
{
	// TODO: Add your control notification handler code here
	GetDlgItem(IDC_START)->EnableWindow(0);
	GetDlgItem(IDC_CBR)->EnableWindow(0);
	GetDlgItem(IDOK)->EnableWindow(0);
	KillTimer(1);
	m_hCBRThread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)CBRThread,this,0,NULL);
}

UINT COAKitsDlg::CBRThread(LPVOID lp)
{
	COAKitsDlg* p = (COAKitsDlg*)lp;

	HANDLE hReadPipe,hWritePipe;
	char szKey[32] = {0};
	DWORD retCode,dwLen,dwRead;
	BOOL bHasKey = FALSE,retval;
	PROCESS_INFORMATION pi={0};
	STARTUPINFOA si={0};
	SECURITY_ATTRIBUTES sa={0};

	sa.bInheritHandle=1;
	sa.nLength=sizeof SECURITY_ATTRIBUTES;
	sa.lpSecurityDescriptor=NULL;
	retval=CreatePipe(&hReadPipe,&hWritePipe,&sa,0);
	si.cb=sizeof STARTUPINFOA;
	si.wShowWindow=SW_HIDE;
	si.dwFlags=STARTF_USESHOWWINDOW|STARTF_USESTDHANDLES;
	si.hStdOutput=si.hStdError=hWritePipe;

	p->SetDlgItemText(IDC_STATUS,TEXT("正在查询机器的KEY码......"));
	retval=CreateProcessA(NULL,"powershell.exe (get-wmiobject softwarelicensingservice).OA3xOriginalProductKey",&sa,&sa,TRUE,0,NULL,NULL,&si,&pi);
	if(retval)
	{
		WaitForSingleObject(pi.hThread,INFINITE);//等待命令行执行完毕
		dwLen=GetFileSize(hReadPipe,NULL);
		retval=ReadFile(hReadPipe,szKey,dwLen,&dwRead,NULL);
		if (strlen(szKey) == 31)
		{
			bHasKey = TRUE;
		}
	}

	if (bHasKey)
	{
		if (IDYES == p->MessageBox(TEXT("确定要擦除此设备中的KEY吗？"),TEXT("警告"),MB_YESNO|MB_ICONWARNING))
		{
			p->SetDlgItemText(IDC_STATUS,TEXT("正在删除KEY码......"));
			CreateProcessA(NULL,"cmd.exe /c afuwin.exe /oad",0,0,0,0,NULL,NULL,&si,&pi);
			WaitForSingleObject(pi.hThread,INFINITE);
			GetExitCodeProcess(pi.hProcess,&retCode);
			if (retCode == 0)
			{
				p->MessageBox(TEXT("KEY擦除成功，按确认后系统将重启！"),TEXT("提示"),MB_ICONINFORMATION);
				p->Reboot();
			}
			else
			{
				p->MessageBox(TEXT("此机器中的KEY已经擦除，需要重启才能生效！"),TEXT("提示"),MB_ICONINFORMATION);
				p->Reboot();
			}
			p->SetDlgItemText(IDC_STATUS,TEXT(""));
		}
	}
	else
	{
		p->MessageBox(TEXT("此机器中不存在KEY，无需擦除！"),TEXT("错误"),MB_ICONERROR);
		p->SetDlgItemText(IDC_STATUS,TEXT(""));
	}
	CloseHandle(hWritePipe);
	CloseHandle(hReadPipe);
	p->GetDlgItem(IDC_START)->EnableWindow();
	p->GetDlgItem(IDC_CBR)->EnableWindow();
	p->GetDlgItem(IDOK)->EnableWindow();
	p->SetTimer(1,5000,NULL);
	return 0;
}

int COAKitsDlg::Reboot(void)
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
	return 0;
}

void COAKitsDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	if (nIDEvent == 1)
	{
		KillTimer(1);
		SetDlgItemText(IDC_STATUS,TEXT(""));
	}
	CDialog::OnTimer(nIDEvent);
}
