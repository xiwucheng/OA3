#pragma once


// CSerialNumberDlg dialog

class CSerialNumberDlg : public CDialog
{
	DECLARE_DYNAMIC(CSerialNumberDlg)

public:
	CSerialNumberDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSerialNumberDlg();
	virtual BOOL OnInitDialog();
	CString m_strSN;
// Dialog Data
	enum { IDD = IDD_SNDLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
};
