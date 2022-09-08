// CMPCutTime.cpp : implementation file
//

#include "stdafx.h"
#include "fdata.h"
#include "CMPCutTime.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgCMPCutter dialog


CDlgCMPCutter::CDlgCMPCutter(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgCMPCutter::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgCMPCutter)
	m_nEndTime = 0;
	m_nStartTime = 0;
	m_sFileOutPut = _T("");
	m_nStartCMP = 0;
	m_nEndCMP = 0;
	//}}AFX_DATA_INIT
}


void CDlgCMPCutter::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgCMPCutter)
	DDX_Text(pDX, IDC_nEndTime, m_nEndTime);
	DDV_MinMaxUInt(pDX, m_nEndTime, 0, 100000);
	DDX_Text(pDX, IDC_nStartTime, m_nStartTime);
	DDV_MinMaxUInt(pDX, m_nStartTime, 0, 100000);
	DDX_Text(pDX, IDC_sFileOutPut, m_sFileOutPut);
	DDX_Text(pDX, IDC_nStartCMP, m_nStartCMP);
	DDX_Text(pDX, IDC_nEndCMP, m_nEndCMP);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgCMPCutter, CDialog)
	//{{AFX_MSG_MAP(CDlgCMPCutter)
	ON_BN_CLICKED(IDC_BUTTON_OUTPUT_FILE, OnButtonOutputFile)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgCMPCutter message handlers

void CDlgCMPCutter::OnButtonOutputFile() 
{
	// TODO: Add your control notification handler code here
	UpdateData(true);

	CFileDialog dlg(false);
	dlg.m_ofn.lpstrFilter="CMP data file name(*.cmp)\0*.cmp";
	dlg.m_ofn.lpstrTitle="Save";
	dlg.m_ofn.lpstrDefExt=".cmp";	
	if(dlg.DoModal()==IDCANCEL)return ;

	m_sFileOutPut=dlg.GetPathName();
	UpdateData(false);

}
