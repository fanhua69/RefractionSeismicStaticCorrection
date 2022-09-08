#if !defined(AFX_CMPDOC_H__763E9A41_7A7A_11D4_AB15_0080AD72BDDD__INCLUDED_)
#define AFX_CMPDOC_H__763E9A41_7A7A_11D4_AB15_0080AD72BDDD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CMPDoc.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCMPAnnealDoc document

#include "StackSectionDoc.h"
#include "segy.h"
#include "SeisDoc.h"
#include "Range.h"
#include "StaticValueChange.h"
#include "TarFunction.h"
#include "CMPBaseDoc.h"

#include "RefStatic.h"

class _REFSTATIC_EXT_CLASS CCMPAnnealDoc : public CCMPBaseDoc
{
public:
	CCMPAnnealDoc();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CCMPAnnealDoc)

// Attributes
protected:

	enumPowerMethod m_nTargetFunction;	
	long i,j,k,m,n;								

public:
	CSvSysDoc *m_pSvSysDoc;
	double m_dPowerCurr;
	long m_nStartPoint;		  // for stack, indicating the time window in which to stack;
	long m_nEndPoint;         // for stack, indicating the time window in which to stack;


	CSegYMultiGroup *m_pStackSectionCurr;
	CSegYMultiGroup *m_pStackSectionTest;

	bool *m_pCMPStateChanged;
	long m_nMaxStatic;


// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCMPAnnealDoc)
	public:
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	protected:
	virtual BOOL OnNewDocument();
	//}}AFX_VIRTUAL

// Implementation
public:		
	CSegYMultiGroup * GetCurrStackSection();
	bool MakeCMPFile(CString &sCMPFile,CSeisDoc *pSeisDoc=NULL);
	bool MakeStackSection(bool bWithStatic);
	double GetCurrentPower(){return m_dPowerCurr;};
	bool AllocateMem();

	bool GetShotRcvStation(CSvSysDoc *pSvSysDoc,CString sGroupFile,long nTotalGroupNumber);
	bool GetShotRcvRelation(CSvSysDoc *pSvSysDoc,CString sGroupFile,long nTotalGroupNumber);

	bool CheckPowerIncOfShot(double nShotPh, int nStaticOld, int nStaticNew,bool &bFailedForReturn,double dTemperature,bool bReplaceIfInc,bool bSeedMethod);
	bool CheckPowerIncOfRcv(double nRcvPh,int nStaticOld, int nStaticNew,bool &bFailedForReturn,double dTemperature,bool bReplaceIfInc,bool bSeedMethod);

	void SetPar(CSvSysDoc *pSvSysDoc, long nStaticRange, enumPowerMethod nTarget);

	bool DoStack(CSegYMultiGroup *pSection,CStaticDoc *pStaticDoc,CSegYMultiGroup *pSectionTest,bool bShowProgress=true);

public:
	double CalcCorePowerOfRcvStation(double dRcvStation,int nRcvStatic, bool bCheckPower); // nShotStatic: transfered here is the static point, not value;
	double CalcCorePowerOfShotStation(double dShotStation,int nShotStatic, bool bCheckPower);// nShotStatic: transfered here is the static point, not value;
	bool ClearSeededMark();
	bool SetSeededMark(double nStation, bool bShotStatic,bool bSetMark=true);
	bool DrawOutPartCMPByStation(CString sOutFile,CRange ShotStation, CRange RcvStation);
	double CalcPowerSimply(CStaticDoc *pStaticDoc,CSegYMultiGroup *pSection);
	bool CalcCorelationAndMakeEquation(CString sFileA, CString sFileB);
	bool DoStaticStack(CSegYMultiGroup *pSection,CStaticDoc *pStaticDoc,bool bStaticOrg,bool bShowProgress=true);

	bool MakeSurveySystem(CSvSysDoc *pSvSysDoc);

	long DoCorrelation(CSegYGroup *pGroupLast, CSegYGroup *pGroup, long nRange);
	bool InitCoreStack();

	
	// make a new CMP document;      
	virtual ~CCMPAnnealDoc();
#ifdef _DEBUG
#endif

	// Generated message map functions
protected:
	bool IsPowerInc(double &dTemperature,
						 bool &bReplaceIfInc,
						 long nRelatedPoint,
					 	 double dStation,
						 long nStaticOld,
						 long nStaticNew,
						 bool bShot,
						 bool bSeedMethod);

	bool PlusStatic(int nPlusStatic);

	BOOL DecideUseNorth();

	//{{AFX_MSG(CCMPAnnealDoc)
	afx_msg void OnCMPPlusStatic();
	afx_msg void OnCMPPlusShotStatic();
	afx_msg void OnCMPPlusRcvStatic();
	afx_msg void OnCMPDumpGroupHead();
	afx_msg void OnCMPStackNOStatic();
	afx_msg void OnCMPStackWithStatic();
	afx_msg void OnCMPMoveGroupByStatic();
	afx_msg void OnCMPCalcSurveySystem();
	afx_msg void OnCMPCutCMPSection();
	afx_msg void OnCMPCorrelation();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CMPDOC_H__763E9A41_7A7A_11D4_AB15_0080AD72BDDD__INCLUDED_)
