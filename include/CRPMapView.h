#if !defined(AFX_CRPMAPVIEW_H__E24B73F1_F78B_4FC0_8CBB_9098A5C6DCE0__INCLUDED_)
#define AFX_CRPMAPVIEW_H__E24B73F1_F78B_4FC0_8CBB_9098A5C6DCE0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CRPMapView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCRPMapView view
#include "InfoView.H"
#include "CRPDoc.h"
#include "StackSectionDoc.h"

class CCRPMapView : public CInfoView
{
protected:
	CCRPMapView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CCRPMapView)


// Attributes
public:
	bool m_bDrawWithStatic;
	long m_nGroupBodyPointNumber;
	long m_nCurrentCRPOrder;
	long m_nSampleInterval;

// Operations
public:
	long GetGroupNumber();
	CSegYGroup* GetGroup(long nOrder);
	void SearchPage();
	CString GetInfo(double x,double y);
	void GotoLastPage();
	void GotoNextPage();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCRPMapView)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnInitialUpdate();     // first time after construct
	//}}AFX_VIRTUAL

// Implementation
protected:
	CCRPDoc* GetDocument();
	virtual ~CCRPMapView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CCRPMapView)
	afx_msg void OnCRPShowWithStatic();
	afx_msg void OnUpdateCRPShowWithStatic(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFHZoomViewLast(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFHZoomViewNext(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CRPMAPVIEW_H__E24B73F1_F78B_4FC0_8CBB_9098A5C6DCE0__INCLUDED_)
