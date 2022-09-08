#if !defined(AFX_CRPDOC_H__C0E476BD_5808_4BCC_81A3_43B001BA57BF__INCLUDED_)
#define AFX_CRPDOC_H__C0E476BD_5808_4BCC_81A3_43B001BA57BF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CRPDoc.h : header file
//
#include "SvSysDoc.h"
#include "SeisDoc.h"


/////////////////////////////////////////////////////////////////////////////
// CCRPDoc document

/////////////////////////////////////////////////////////////////////////////
// CCMPIndexDoc document

#define SHOT_LIMIT_ON_ONE_RCV 480

#include "StackSectionDoc.h"    // with class CGroup;
#include "SeisDoc.h"
#include "segy.h"
#include "CRPIndexDoc.h"


class CCRPDoc : public CDocument
{
public:
	CCRPDoc();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CCRPDoc)

public:
	CCRPIndexDoc m_CRPIndexDoc;
// Attributes

protected:
	FILE *m_fpCRPFile;
	long m_nSegYGroupHeadSize;
	CSvSysDoc *m_pSvSysDoc;  // this pointer belong only to outside , do not close it;

	CSegYGroup m_group;	

	CStackSectionDoc *m_pStackSectionDocTest;
	CStackSectionDoc *m_pStackSectionDocCurr;

public:	
	long m_nStartPoint;  // time point to start calculate
	long m_nEndPoint;    // time point to end calculation
	long m_nMaxGroupNumber;
	double m_dPowerCurr;	

	CRange m_nValidCMPRange[VALID_CMP_RANGE_LIMIT];
	long m_nValidCMPRangeNumber;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCRPDoc)
	public:
	virtual void Serialize(CArchive& ar);   // overridden for document i/o
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	protected:
	virtual BOOL OnNewDocument();
	//}}AFX_VIRTUAL

// Implementation
public:
	long GetCRPNumber();
	long GetGroupNumberInCRP(long nOrder);
	bool MakeCRPFile(CSeisDoc *pSeisDoc, CSvSysDoc *pSvSysDoc,CString &sCRPFile);
	bool SaveVolHead(CSegYVolHead head);
	bool ConstructionOver();
	bool CreateCRPFile(CString sCRPFile="");
	bool SaveGroupOfOneRcv(float *pCRPData,long nCRPOrder);
	double GetCurrentPower();
	long SearchRcvInIndex(long nRcvStation);
	long SearchShotStationInCRP(long nCRPOrder, long nShotPh);
	CSegYGroup* GetGroupByStation(long nRcvPh,long nShotPh);
	bool SetCMPRange(CRange *pRange, long nRangeNumber);	
	bool SetCalcRange(long nStartPoint,long nEndPoint);
	bool SetSvSysDocPointer(CSvSysDoc *pSvSysDoc);
	CSegYGroup* GetGroup(long nCRPNo,long nGroupOfCRP,bool bBalance=false);
	CSegYGroup * GetGroup(long nGroup,bool bBalance=false);
	bool OpenCRPFile(CString sCRPFile);
	long SearchStation(long nRcvPh);

	// Open:

	// Save:
	bool SaveGroup(CSegYGroup *pSegYGroup,long nPos);
	bool SaveGroup(CSvSysDoc *pSvSysDoc,CSegYGroup *pSegYGroup, long nCRP,long nOrderInCRP);
	bool SaveGroup(long nCRPOrder,long nGroupOrderInCRP,CSegYGroup *pSegYGroup);

	// For stoconj:
	bool CheckPowerInc(long nShotPh,int nOldStatic,int nNewStatic,bool bReplaceIfInc=true);
	bool ReStack(bool bTest=false);

	bool AllocateMem();

	virtual ~CCRPDoc();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CCRPDoc)
	afx_msg void OnCRPPlusShotStatic();
	afx_msg void OnCRPDumpGroupHeads();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CRPDOC_H__C0E476BD_5808_4BCC_81A3_43B001BA57BF__INCLUDED_)
