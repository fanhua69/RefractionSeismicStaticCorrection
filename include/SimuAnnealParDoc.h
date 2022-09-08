#if !defined(AFX_SIMUANEALLPARDOC_H__A393B839_A63A_43BC_9475_816260285742__INCLUDED_)
#define AFX_SIMUANEALLPARDOC_H__A393B839_A63A_43BC_9475_816260285742__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SimuAneallParDoc.h : header file
//

class CSimuPar
{
public:
	long nLoopOrder;
	double  fTemperature;
};

/////////////////////////////////////////////////////////////////////////////
// CSimuAnnealParDoc document
#include "FHEditDoc.h"

class CSimuAnnealParDoc : public CFHEditDoc
{
public:
	CSimuAnnealParDoc();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CSimuAnnealParDoc)

// Attributes
protected:
	CSimuPar *m_pSimuPar;

// Operations
public:


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSimuAnnealParDoc)
	public:
	virtual void Serialize(CArchive& ar);   // overridden for document i/o
	protected:
	virtual BOOL OnNewDocument();
	//}}AFX_VIRTUAL

// Implementation
public:
	static CString GetFileForOpen();
	static CString GetFileForSave();

	double GetTemperature(long nLoop);
	long GetParNumber();
	CSimuPar * GetPar(long nPlace);
	bool SetPar(long nPlace,long nLoopOrder,float fTemperature);
	bool SetRecordNumber(long nSimuParNumber);
	virtual ~CSimuAnnealParDoc();
	BOOL OnOpenDocument(LPCTSTR lpszPathName) ;
	BOOL OnSaveDocument(LPCTSTR lpszPathName) ;
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CSimuAnnealParDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SIMUANEALLPARDOC_H__A393B839_A63A_43BC_9475_816260285742__INCLUDED_)
