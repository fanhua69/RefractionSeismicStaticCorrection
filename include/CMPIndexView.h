#if !defined(AFX_CMPINDEXVIEW_H__FE11E061_7CBB_11D4_AB16_0080AD72BDDD__INCLUDED_)
#define AFX_CMPINDEXVIEW_H__FE11E061_7CBB_11D4_AB16_0080AD72BDDD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CMPIndexView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCMPIndexView view

class CCMPIndexView : public CScrollView
{
protected:
	CCMPIndexView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CCMPIndexView)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCMPIndexView)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnInitialUpdate();     // first time after construct
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CCMPIndexView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CCMPIndexView)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CMPINDEXVIEW_H__FE11E061_7CBB_11D4_AB16_0080AD72BDDD__INCLUDED_)
