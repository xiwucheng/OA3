// OAServerKitsDlg.h : header file
//

#pragma once

#include "TCPSocket.h"
typedef struct _tagKeyInfo
{
	char BSN[32];
	char SSN[32];
	char PKID[32];
	char KEY[32];
	char WIFI[32];
	char BT[32];
	char IMEI[32];
	DWORD CRC;
}KeyInfo,*PKeyInfo;

class CKeyInfo
{
public:
	CKeyInfo(){ socket = 0;memset(&m_KeyInfo,0,sizeof(m_KeyInfo));};
	virtual ~CKeyInfo(){ socket = 0;memset(&m_KeyInfo,0,sizeof(m_KeyInfo));};
	SOCKET socket;
	KeyInfo m_KeyInfo;
};

// COAServerKitsDlg dialog
class COAServerKitsDlg : public CDialog
{
// Construction
public:
	COAServerKitsDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_OASERVERKITS_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;
	CTCPServer* m_pServer;
	BOOL m_bStart;
	NOTIFYICONDATA nid;
	CMenu menu,*Menu;
	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	DWORD CRC_Table[256];
	afx_msg void OnBnClickedMonitor();
	void init_crc_table();
	DWORD CRC32(DWORD crc,BYTE *buffer, DWORD size);
public:
	afx_msg void OnDestroy();
	static void CALLBACK OnClientConnect(void* pOwner, CTCPCustom* pCustom);
	static void CALLBACK OnClientClose(void* pOwner, CTCPCustom* pCustom);
	static void CALLBACK OnClientRead(void* pOwner, CTCPCustom* pCustom, char* buf, int len);
	static void CALLBACK OnClientError(void* pOwner, CTCPCustom* pCustom, int nErrorCode);
	static void CALLBACK OnServerError(void* pOwner, CTCPServer* pServer, int nErrorCode);
public:
	void AddLog(CString log);
	afx_msg void OnBnClickedClear();
	afx_msg void OnFileShow();
	afx_msg void OnFileHide();
	afx_msg void OnFileExit();
	afx_msg void OnClose();
	afx_msg LRESULT OnNotifyIcon(WPARAM wParam,LPARAM lParam);
};
