#if !defined(AFX_CMPCUTTIME_H__DCAA3C98_C189_4376_9CA4_BAA68059BE3C__INCLUDED_)
#define AFX_CMPCUTTIME_H__DCAA3C98_C189_4376_9CA4_BAA68059BE3C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CMPCutTime.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgCMPCutter dialog

class CDlgCMPCutter : public CDialog
{
// Construction
public:
	CDlgCMPCutter(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgCMPCutter)
	enum { IDD = IDD_CMP_CUTTER };
	UINT	m_nEndTime;
	UINT	m_nStartTime;
	CString	m_sFileOutPut;
	UINT	m_nStartCMP;
	UINT	m_nEndCMP;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgCMPCutter)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgCMPCutter)
	afx_msg void OnButtonOutputFile();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CMPCUTTIME_H__DCAA3C98_C189_4376_9CA4_BAA68059BE3C__INCLUDED_)
