#if !defined(AFX_SIMUANEALLPARVIEW_H__C1E2FDAF_6A78_40AC_8440_4842F3A1239D__INCLUDED_)
#define AFX_SIMUANEALLPARVIEW_H__C1E2FDAF_6A78_40AC_8440_4842F3A1239D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SimuAneallParView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSimuAnnealParView view

#include "FHEditView.h"
#include "SimuAnnealParDoc.h"

class CSimuAnnealParView : public CFHEditView
{
protected:
	CSimuAnnealParView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CSimuAnnealParView)

// Attributes
public:

// Operations
public:
	CSimuAnnealParDoc * GetDocument();
	void UpdateGrid();
	void UpdateDoc();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSimuAnnealParView)
	protected:
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CSimuAnnealParView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CSimuAnnealParView)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SIMUANEALLPARVIEW_H__C1E2FDAF_6A78_40AC_8440_4842F3A1239D__INCLUDED_)
