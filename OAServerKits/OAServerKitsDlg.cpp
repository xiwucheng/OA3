// OAServerKitsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "OAServerKits.h"
#include "OAServerKitsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CPtrList g_ptr;

// CAboutDlg dialog used for App About

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


// COAServerKitsDlg dialog


void COAServerKitsDlg::init_crc_table()  
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

DWORD COAServerKitsDlg::CRC32(DWORD crc,BYTE *buffer, DWORD size)  
{  
    DWORD i;  
    for (i = 0; i < size; i++) {  
        crc = CRC_Table[(crc ^ buffer[i]) & 0xff] ^ (crc >> 8);  
    }  
    return crc ;  
}  


COAServerKitsDlg::COAServerKitsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(COAServerKitsDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_pServer=NULL;
	m_bStart=FALSE;
}

void COAServerKitsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(COAServerKitsDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_MONITOR, &COAServerKitsDlg::OnBnClickedMonitor)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_CLEAR, &COAServerKitsDlg::OnBnClickedClear)
	ON_MESSAGE(WM_SHELLICON, &COAServerKitsDlg::OnNotifyIcon)
	ON_COMMAND(ID_FILE_SHOW, &COAServerKitsDlg::OnFileShow)
	ON_COMMAND(ID_FILE_HIDE, &COAServerKitsDlg::OnFileHide)
	ON_COMMAND(ID_FILE_EXIT, &COAServerKitsDlg::OnFileExit)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// COAServerKitsDlg message handlers

BOOL COAServerKitsDlg::OnInitDialog()
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
	memset(&nid,0,sizeof(nid));
	nid.cbSize=sizeof nid;
	nid.hIcon=m_hIcon;
	nid.hWnd=m_hWnd;
	nid.uFlags=NIF_ICON|NIF_MESSAGE|NIF_TIP;
	nid.uID=IDR_MAINFRAME;
	_tcscpy(nid.szTip,"OAServerKits");
	nid.uCallbackMessage=WM_SHELLICON;
	Shell_NotifyIcon(NIM_ADD,&nid);
	menu.LoadMenu(IDR_MAINFRAME);
	Menu=menu.GetSubMenu(0);

	// TODO: Add extra initialization here
	CComboBox* pBox=(CComboBox*)GetDlgItem(IDC_IPCOMBOX);
	char host[MAX_PATH]={0};
	gethostname(host,MAX_PATH);	
	struct hostent *p=gethostbyname(host);
	for (int i = 0; p->h_addr_list[i] != 0; ++i) 
	{ 
		struct in_addr addr; 
		memcpy(&addr, p->h_addr_list[i], sizeof(struct in_addr)); 
		pBox->AddString(inet_ntoa(addr));
	}
	if (pBox->GetCount())
	{
		pBox->SetCurSel(0);
	}
	SetDlgItemInt(IDC_PORT,300,0);
	m_pServer = new CTCPServer(this);
	m_pServer->m_OnClientConnect = OnClientConnect;
	m_pServer->m_OnClientClose = OnClientClose;
	m_pServer->m_OnClientRead = OnClientRead;
	m_pServer->m_OnClientError = OnClientError;
	m_pServer->m_OnServerError = OnServerError;

	SetDlgItemInt(IDC_RDX,0);
	init_crc_table();
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void COAServerKitsDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void COAServerKitsDlg::OnPaint()
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
HCURSOR COAServerKitsDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void COAServerKitsDlg::OnBnClickedMonitor()
{
	// TODO: Add your control notification handler code here
	if (m_bStart)
	{
		SetDlgItemText(IDC_MONITOR,"Start");
		m_pServer->Close();
		m_bStart=FALSE;
		AddLog("Server is stopped");
		GetDlgItem(IDC_IPCOMBOX)->EnableWindow();
		GetDlgItem(IDC_PORT)->EnableWindow();
		GetDlgItem(IDC_FILENAME)->EnableWindow();
	}
	else
	{
		int port=GetDlgItemInt(IDC_PORT,0,0);
		if (port < 300 || port >= 400)
		{
			MessageBox("please enter port number from 300 to 399","Out of range",MB_ICONERROR);
			return;
		}
		if (m_pServer->Open(port)!=1)
		{
			MessageBox("service not start","error",MB_ICONERROR);
			return;
		}
		SetDlgItemText(IDC_MONITOR,"Stop");
		m_bStart=TRUE;
		AddLog("Server is running");
		GetDlgItem(IDC_IPCOMBOX)->EnableWindow(0);
		GetDlgItem(IDC_PORT)->EnableWindow(0);
		GetDlgItem(IDC_FILENAME)->EnableWindow(0);
	}
}

LRESULT COAServerKitsDlg::OnNotifyIcon(WPARAM wParam, LPARAM lParam)
{
	switch(lParam)
	{
	case WM_RBUTTONUP:
		{
			CPoint pt;
			GetCursorPos(&pt);
			Menu->TrackPopupMenu(TPM_BOTTOMALIGN|TPM_RIGHTALIGN,pt.x,pt.y,AfxGetMainWnd());
		}
		break;
	case WM_LBUTTONDBLCLK:
			OnFileShow();
		break;
	default:
		break;
	}
	return 0L;
}

void COAServerKitsDlg::OnDestroy()
{
	CDialog::OnDestroy();
	Shell_NotifyIcon(NIM_DELETE,&nid);
	menu.DestroyMenu();
	if (m_pServer)
	{
		delete m_pServer;
		m_pServer=NULL;
	}
	// TODO: Add your message handler code here
}

void CALLBACK COAServerKitsDlg::OnClientConnect(void* pOwner, CTCPCustom* pCustom)
{
	CKeyInfo* pKeyInfo = new CKeyInfo();
	pKeyInfo->socket=pCustom->GetSocket();
	g_ptr.AddTail(pKeyInfo);
}

void CALLBACK COAServerKitsDlg::OnClientClose(void* pOwner, CTCPCustom* pCustom)
{
	COAServerKitsDlg* p = (COAServerKitsDlg*)pOwner;
	POSITION pos,temp;
	for (pos = g_ptr.GetHeadPosition(); pos!=NULL;)
	{
		temp = pos;
		CKeyInfo* pKeyInfo = (CKeyInfo*)g_ptr.GetNext(pos);
		if (pKeyInfo && pKeyInfo->socket == pCustom->GetSocket())
		{
			delete pKeyInfo;
			g_ptr.RemoveAt(temp);
			break;
		}
	}

}

void CALLBACK COAServerKitsDlg::OnClientRead(void* pOwner, CTCPCustom* pCustom, char* buf, int len)
{
	COAServerKitsDlg* p = (COAServerKitsDlg*)pOwner;
	CString log,szFile;
	CFile fp;
	char buff[1024]={0};
	int rdx=0;

	POSITION pos;
	for (pos = g_ptr.GetHeadPosition(); pos!=NULL;)
	{
		CKeyInfo* pKeyInfo = (CKeyInfo*)g_ptr.GetNext(pos);
		if (pKeyInfo && pKeyInfo->socket == pCustom->GetSocket())
		{
			if (len != sizeof(KeyInfo))
			{
				break;
			}
			memcpy(&pKeyInfo->m_KeyInfo,buf,sizeof(KeyInfo));
			DWORD crc = p->CRC32(0xFFFFFFFF,(BYTE*)&pKeyInfo->m_KeyInfo,sizeof(KeyInfo)-4);
			if (crc == pKeyInfo->m_KeyInfo.CRC)
			{
				pCustom->SendData("techvision",strlen("techvision"));
				p->AddLog(pKeyInfo->m_KeyInfo.PKID);
				rdx = p->GetDlgItemInt(IDC_RDX);
				rdx++;
				p->SetDlgItemInt(IDC_RDX,rdx);

				p->GetDlgItemText(IDC_FILENAME,szFile);
				if (szFile.GetLength() && szFile.Right(4).CompareNoCase(".txt") == 0)
				{
					fp.Open(szFile,CFile::modeReadWrite|CFile::modeCreate|CFile::modeNoTruncate);
				}
				else
				{
					fp.Open("tablet.txt",CFile::modeReadWrite|CFile::modeCreate|CFile::modeNoTruncate);
				}
				int len=(int)fp.GetLength();
				if (len == 0)
				{
					fp.Write("SN\t\t\t\tID\t\tKEY\t\t\t\tWIFI\t\t\tBT\t\t\tIMEI\r\n",(UINT)strlen("SN\t\t\t\tID\t\tKEY\t\t\t\tWIFI\t\t\tBT\t\t\tIMEI\r\n"));
				}
				fp.SeekToEnd();
				memset(buff,0,sizeof(buff));
				strcat(buff,pKeyInfo->m_KeyInfo.SN);
				strcat(buff,"\t");
				strcat(buff,pKeyInfo->m_KeyInfo.PKID);
				strcat(buff,"\t");
				strcat(buff,pKeyInfo->m_KeyInfo.KEY);
				strcat(buff,"\t");
				strcat(buff,pKeyInfo->m_KeyInfo.WIFI);
				strcat(buff,"\t");
				strcat(buff,pKeyInfo->m_KeyInfo.BT);
				strcat(buff,"\t");
				strcat(buff,pKeyInfo->m_KeyInfo.IMEI);
				strcat(buff,"\r\n");

				fp.Write(buff,(UINT)strlen(buff));
				fp.Close();
			}
			break;
		}
	}
}

void CALLBACK COAServerKitsDlg::OnClientError(void* pOwner, CTCPCustom* pCustom, int nErrorCode)
{
	COAServerKitsDlg* p = (COAServerKitsDlg*)pOwner;
	POSITION pos,temp;
	for (pos = g_ptr.GetHeadPosition(); pos!=NULL;)
	{
		temp = pos;
		CKeyInfo* pKeyInfo = (CKeyInfo*)g_ptr.GetNext(pos);
		if (pKeyInfo && pKeyInfo->socket == pCustom->GetSocket())
		{
			delete pKeyInfo;
			g_ptr.RemoveAt(temp);
			break;
		}
	}
}

void CALLBACK COAServerKitsDlg::OnServerError(void* pOwner, CTCPServer* pServer, int nErrorCode)
{
}

void COAServerKitsDlg::AddLog(CString log)
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

void COAServerKitsDlg::OnBnClickedClear()
{
	// TODO: Add your control notification handler code here
	SetDlgItemText(IDC_LOG,NULL);
	SetDlgItemInt(IDC_RDX,0);
}

void COAServerKitsDlg::OnFileShow()
{
	// TODO: Add your command handler code here
	if (IsWindowVisible())
	{
		SetForegroundWindow();
	}
	else
	{
		ShowWindow(SW_SHOW);
	}
}

void COAServerKitsDlg::OnFileHide()
{
	// TODO: Add your command handler code here
	ShowWindow(SW_HIDE);
}

void COAServerKitsDlg::OnFileExit()
{
	// TODO: Add your command handler code here
	CDialog::OnOK();
}

void COAServerKitsDlg::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	ShowWindow(SW_HIDE);
	//CDialog::OnClose();
}
