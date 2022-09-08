#if !defined(AFX_CMPVIEW_H__1BB5DC61_8307_11D4_A8E0_0080AD72BDDD__INCLUDED_)
#define AFX_CMPVIEW_H__1BB5DC61_8307_11D4_A8E0_0080AD72BDDD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CMPView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCMPView view
#include "DrawGroup.h"
#include "InfoView.h"

class CCMPView : public CInfoView
{
protected:
	CCMPView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CCMPView)

private:
	bool m_bShowWithStatic;
	long m_nSampleInterval;


// Attributes
public:

// Operations
public:
	long GetGroupNumber();
	CSegYGroup * GetGroup(long nOrder);
	void SearchPage();
	CString GetInfo(double x,double y);
	long m_nCurrentCMPOrder;
	long m_nCMPNumber;


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCMPView)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnInitialUpdate();     // first time after construct
	//}}AFX_VIRTUAL

// Implementation
protected:
	CCMPDoc* GetDocument();
	virtual ~CCMPView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CCMPView)
	afx_msg void OnCMPShowWithStatic();
	afx_msg void OnUpdateCMPShowWithStatic(CCmdUI* pCmdUI);
	afx_msg void OnFHZoomViewNext();
	afx_msg void OnUpdateFHZoomViewNext(CCmdUI* pCmdUI);
	afx_msg void OnFHZoomViewLast();
	afx_msg void OnUpdateFHZoomViewLast(CCmdUI* pCmdUI);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnCMPDrawOutMicroCMP();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CMPVIEW_H__1BB5DC61_8307_11D4_A8E0_0080AD72BDDD__INCLUDED_)
