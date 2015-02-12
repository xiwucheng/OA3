// SerialNumberDlg.cpp : implementation file
//

#include "stdafx.h"
#include "OAKits.h"
#include "SerialNumberDlg.h"


// CSerialNumberDlg dialog

IMPLEMENT_DYNAMIC(CSerialNumberDlg, CDialog)

CSerialNumberDlg::CSerialNumberDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSerialNumberDlg::IDD, pParent)
{

}

CSerialNumberDlg::~CSerialNumberDlg()
{
}

void CSerialNumberDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CSerialNumberDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CSerialNumberDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CSerialNumberDlg message handlers
BOOL CSerialNumberDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	//EnableMenuItem(::GetSystemMenu(m_hWnd,FALSE),SC_CLOSE,MF_BYCOMMAND|MF_DISABLED);
	return TRUE;  // return TRUE  unless you set the focus to a control
}


void CSerialNumberDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	GetDlgItemText(IDC_SN,m_strSN);
	OnOK();
}
