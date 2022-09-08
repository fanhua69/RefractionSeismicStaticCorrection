#if !defined(AFX_CMPDOC_H__763E9A41_7A7A_11D4_AB15_0080AD72BDDD__INCLUDED_)
#define AFX_CMPDOC_H__763E9A41_7A7A_11D4_AB15_0080AD72BDDD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CMPDoc.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCMPDoc document

#include "CMPIndexDoc.h"
#include "StackSectionDoc.h"
#include "segy.h"
#include "SeisDoc.h"
#include "Range.h"
#include "StaticValueChange.h"
#include "TarFunction.h"

class CCMPDoc : public CDocument
{
public:
	CCMPDoc();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CCMPDoc)

// Attributes
protected:
	CSegYGroup m_SegYGroup;  // temporary group for getting the information of the groups heads.
	FILE *m_fpCMPData;
	enumPowerMethod m_nTargetFunction;	
	UINT m_nFullFoldTime;
	long i,j,k,m,n;								
	bool *m_pCMPStateChanged;  // Used in calc core section power:

public:
	double m_dPowerCurr;  // set it to public for using directly outside of CMP doc;
	CSvSysDoc *m_pSvSysDoc;
	CCMPIndexDoc m_CMPIndexDoc;
	
	long m_nStartCMP;
	long m_nEndCMP;
	long m_nStartPoint;		  // for stack, indicating the time window in which to stack;
	long m_nEndPoint;         // for stack, indicating the time window in which to stack;
	long m_nCMPNumber;

	CSegYMultiGroup *m_pStackSectionCurr;
	CSegYMultiGroup *m_pStackSectionTest;
	

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCMPDoc)
	public:
	virtual void Serialize(CArchive& ar);   // overridden for document i/o
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual void OnCloseDocument();
	protected:
	virtual BOOL OnNewDocument();
	//}}AFX_VIRTUAL

// Implementation
public:		
	CSegYMultiGroup * GetCurrStackSection();
	bool MakeCMPFile(CString &sCMPFile,CSeisDoc *pSeisDoc=NULL);
	bool MakeStackSection(bool bWithStatic);
	bool MakeCMPFileByHeadCMPCoor(CString sFile);
	bool SetCalcMethod( enumPowerMethod  nTarget);
	double GetCurrentPower(){return m_dPowerCurr;};
	bool AllocateMem();

	bool GetShotRcvStation(CSvSysDoc *pSvSysDoc,CString sGroupFile,long nTotalGroupNumber);
	bool GetShotRcvRelation(CSvSysDoc *pSvSysDoc,CString sGroupFile,long nTotalGroupNumber);



	bool SaveGroup(long nCMPOrder,long nGroupOrderInCMP,CSegYGroup *pSegYGroup,bool bOnlyHead=false);
	CSegYGroup *GetCMPGroupByShotStation(double nCMPPh, double nShotPh,long &nCMPOrder);
	CSegYGroup *GetCMPGroupByRcvStation(double nCMPPh, double nRcvPh,long &nCMPOrder);
	CSegYGroup *GetCMPGroupByShotStation(long nCMPOrder, double nShotPh,bool bBalance=false,CSegYGroup *pGroupReturn=NULL);
	CSegYGroup *GetCMPGroupByRcvStation(long nCMPOrder, double nRcvPh,bool bBalance=false,CSegYGroup *pGroupReturn=NULL);
	CSegYGroup *GetCMPGroupByStation(double dShotStation,double dRcvStation, long &nCMPOrder,long &nGroupOrderInCMP);


	CSegYGroup *GetGroup(long nCMPNo,long nGroupNoInCMP,bool bBalance=false,CSegYGroup *pGroup=NULL,bool bOnlyHead=false);

		
	bool CheckPowerIncOfShot(double nShotPh, int nStaticOld, int nStaticNew,bool &bFailedForReturn,double dTemperature,bool bReplaceIfInc,long &nStaticChangedInLoop,bool bSeedMethod);
	bool CheckPowerIncOfRcv(double nRcvPh,int nStaticOld, int nStaticNew,bool &bFailedForReturn,double dTemperature,bool bReplaceIfInc,long &nStaticChangedInLoop,bool bSeedMethod);

	void SetPointer(CSvSysDoc *pSvSysDoc);
	bool OpenSeisFile(CString sCMPFile);
	void SetPar(long nStaticSearchRange);
	bool GetVolHead(CSegYVolHead &head);

	bool DoStack(CSegYMultiGroup *pSection,CStaticDoc *pStaticDoc,CSegYMultiGroup *pSectionTest);

public:
	double CalcCorePowerOfRcvStation(double dRcvStation,int nRcvStatic, bool bCheckPower); // nShotStatic: transfered here is the static point, not value;
	double CalcCorePowerOfShotStation(double dShotStation,int nShotStatic, bool bCheckPower);// nShotStatic: transfered here is the static point, not value;
	bool ClearSeededMark();
	bool SetSeededMark(double nStation, bool bShotStatic,bool bSetMark=true);
	bool DrawOutPartCMPByStation(CString sOutFile,CRange ShotStation, CRange RcvStation);
	double CalcPowerSimply(CStaticDoc *pStaticDoc,CSegYMultiGroup *pSection);
	bool CalcCorelationAndMakeEquation(CString sFileA, CString sFileB);
	bool DoStaticStack(CSegYMultiGroup *pSection,CStaticDoc *pStaticDoc,bool bStaticOrg);

	enumSeisDataSrc GetDataSrc();
	long GetGroupNumberInCMP(long nCMPOrder);
	long GetCMPNumber();
	bool MakeSurveySystem(CSvSysDoc *pSvSysDoc);
	static CString GetFileForOpen();
	static CString GetFileForSave();

	long DoCorrelation(CSegYGroup *pGroupLast, CSegYGroup *pGroup, long nRange);
	bool InitCoreStack();

	
	// make a new CMP document;      
	virtual ~CCMPDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	bool IsPowerInc(double &dTemperature,
						 bool &bReplaceIfInc,
						 long &nStaticChangedInLoop,
						 long nRelatedPoint,
					 	 double dStation,
						 long nStaticOld,
						 long nStaticNew,
						 bool bShot,
						 bool bSeedMethod);

	bool PlusStatic(int nPlusStatic);
	//{{AFX_MSG(CCMPDoc)
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
