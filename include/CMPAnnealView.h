#if !defined(AFX_CMPVIEW_H__1BB5DC61_8307_11D4_A8E0_0080AD72BDDD__INCLUDED_)
#define AFX_CMPVIEW_H__1BB5DC61_8307_11D4_A8E0_0080AD72BDDD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CMPView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCMPAnnealView view
#include "DrawGroup.h"
#include "CMPBaseView.h"


class CCMPAnnealView : public CCMPBaseView
{
protected:
	CCMPAnnealView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CCMPAnnealView)

private:
	bool m_bShowWithStatic;

// Attributes
public:

// Operations
public:
	CSegYGroup * GetGroup(long nOrder);
	CString GetInfo(double x,double y);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCMPAnnealView)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//}}AFX_VIRTUAL

// Implementation
protected:
	CCMPAnnealDoc* GetDocument();
	virtual ~CCMPAnnealView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CCMPAnnealView)
	afx_msg void OnCMPShowWithStatic();
	afx_msg void OnUpdateCMPShowWithStatic(CCmdUI* pCmdUI);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnCMPDrawOutMicroCMP();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
//	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CMPVIEW_H__1BB5DC61_8307_11D4_A8E0_0080AD72BDDD__INCLUDED_)
