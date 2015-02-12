// OAKitsDlg.h : header file
//
#define _WIN32_DCOM
#include <comdef.h>
#include <atlbase.h>
#include <rpcsal.h>
// headers needed to use Mobile Broadband APIs 
#pragma comment(lib, "mbnapi_uuid.lib")
#include "mbnapi.h"
#include "TCPSocket.h"
#pragma comment(lib, "IPHLPAPI.lib")
#include <iphlpapi.h>


#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))

#pragma once

// COAKitsDlg dialog
typedef struct _tagKeyInfo
{
	char SN[32];
	char PKID[32];
	char KEY[32];
	char WIFI[32];
	char BT[32];
	char IMEI[32];
	DWORD CRC;
}KeyInfo,*PKeyInfo;

class COAKitsDlg : public CDialog
{
// Construction
public:
	COAKitsDlg(CWnd* pParent = NULL);	// standard constructor
	CTCPClient* m_pClient;
	KeyInfo m_KeyInfo;
	DWORD CRC_Table[256];

// Dialog Data
	enum { IDD = IDD_OAKITS_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;
	HANDLE m_hKeyThread,m_hCBRThread;
	char m_ip[32];
	int m_port;
	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	void init_crc_table();
	DWORD CRC32(DWORD crc,BYTE *buffer, DWORD size);
	BOOL GetDeviceAddress();
	BOOL GetIMEI();
	afx_msg void OnBnClickedStart();
	volatile BOOL m_bAccept;
public:
	afx_msg void OnDestroy();
public:
	void AddLog(CString log);
public:
	static UINT KeyThread(LPVOID lp);
	static UINT CBRThread(LPVOID lp);
	static void CALLBACK OnError(void* pOwner, int nErrorCode);
	static void CALLBACK OnDisconnect(void* pOwner);
	static void CALLBACK OnRead(void* pOwner, char* buf, int len);
	afx_msg void OnBnClickedCbr();
	int Reboot(void);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
