#if !defined(AFX_CRPINDEXDOC_H__FACA45EA_6649_4CED_B1E7_C2DCE51F3B15__INCLUDED_)
#define AFX_CRPINDEXDOC_H__FACA45EA_6649_4CED_B1E7_C2DCE51F3B15__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CRPIndexDoc.h : header file
//	  
/////////////////////////////////////////////////////////////////////////////
// CCRPIndexDoc document
class CCRPIndex
{
public:
	long nRcvPh;	        // The location of the reciever point;
	long pos;           // How many groups are there before this reciever point;
	long nGroupNumber;  // How many groups (shots) on this reciever point;

	CCRPIndex()
	{
		nRcvPh=0;
		pos=0;
		nGroupNumber=0;
	};

	operator =(CCRPIndex &other)
	{
		nRcvPh=other.nRcvPh ;
		pos=other.pos;
		nGroupNumber=other.nGroupNumber ;
	};
};

class CCRPGroupMsg
{
public:
	long nRcvPh;
	long nGroupNumber;
	long nShotPh;
	CCRPGroupMsg()
	{
		nRcvPh=0;
		nGroupNumber=0;
		nShotPh=0;
	}
};

class CCRPIndexHead
{
public:
	long nCRPNumber;
	long nGroupHeadPointNumber;
	long nGroupBodyPointNumber;
	long nGroupTotalPointNumber;
	long nSampleInterval;
	long nMaxShotOnOneRcv;
	long nCRPLimit;
	long nGroupTotalNumber;
	CCRPIndex index[10000];

	CCRPIndexHead()
	{
		nCRPNumber=0;
		nGroupHeadPointNumber=0;
		nGroupBodyPointNumber=0;
		nGroupTotalPointNumber=0;
		nSampleInterval=0;
		nMaxShotOnOneRcv=0;
		nGroupTotalNumber=0;
		nCRPLimit=10000;
		memset(&index[0],0,sizeof(CCRPIndex)*nCRPLimit);
	};
};


class CCRPIndexDoc 
{
protected:
	FILE * m_fpCRPIndex;
	CString m_sIndexExt;

	CCRPGroupMsg *m_pCRPGroupMsg;

public:
	CCRPIndexHead m_indexHead;	

public:
	CCRPGroupMsg * GetCRPGroupMsg (long nCRPOrder);
	CCRPIndexDoc();           
	bool OpenIndex(CString sCRPFile);
	bool GetGroupMsg(long nCRPOrder,long nGroupOrderInCRP,CCRPGroupMsg *pMsg);
	CString GetIndexFileName(CString sCRPFile);
	bool CreateIndexFile(CString sCRPFile);
	virtual ~CCRPIndexDoc();

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CRPINDEXDOC_H__FACA45EA_6649_4CED_B1E7_C2DCE51F3B15__INCLUDED_)
