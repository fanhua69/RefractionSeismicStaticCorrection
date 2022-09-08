#if !defined(AFX_CMPINDEXDOC_H__6DA7ED61_79CF_11D4_AB15_0080AD72BDDD__INCLUDED_)
#define AFX_CMPINDEXDOC_H__6DA7ED61_79CF_11D4_AB15_0080AD72BDDD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CMPIndexDoc.h : header file
//
#include "StaticDoc.h"

/////////////////////////////////////////////////////////////////////////////
// CCMPIndexDoc document
class CCMPIndex
{
public:
	double nStation;  // the station of the CMP;
	double east;
	double north;
	double pos;
	long nGroupNumber;

	CCMPIndex()
	{
		nStation=0;
		east=north=0.0;
		pos=nGroupNumber=0;
	};

	operator =(CCMPIndex &other)
	{
		nStation=other.nStation ;
		east=other.east;
		north=other.north;
		pos=other.pos;
		nGroupNumber=other.nGroupNumber ;
	};
};

class CCMPGroupMsg
{
public:
	long nPosInFile;    // The start position of this group in the file, for non-same group length;
	double dStation;
	double dEast;
	double dNorth;
	
	long nShotOrder;  // in the relationship file is better, because you can find any info there.
	double dShotPh;
	double dShotNorth;
	double dShotEast;
	
	long nRcvOrder;   // of the shot 's groups;
	double dRcvPh;
	double dRcvNorth;
	double dRcvEast;

	long nGroupSamplePoint;


	CCMPGroupMsg()
	{
		nPosInFile=0;

		dStation=0;
		dEast=0;
		dNorth=0;
		
		nShotOrder=0;  // in the relationship file is better, because you can find any info there.
		dShotPh=0;
		dShotNorth=0;
		dShotEast=0;
		
		nRcvOrder=0;
		dRcvPh=0;
		dRcvNorth=0;
		dRcvEast=0;

		nGroupSamplePoint=0;
	};

	operator =(CCMPGroupMsg &other)
	{
		nPosInFile=other.nPosInFile ;

		dStation=other.dStation;
		dEast=other.dEast;
		dNorth=other.dNorth;
		
		nShotOrder=other.nShotOrder;
		dShotPh=other.dShotPh;
		dShotNorth=other.dShotNorth;
		dShotEast=other.dShotEast;
		
		nRcvOrder=other.nRcvOrder;
		dRcvPh=other.dRcvPh;
		dRcvNorth=other.dRcvNorth;
		dRcvEast=other.dRcvEast;

		nGroupSamplePoint=other.nGroupSamplePoint ;
	};
};

class CCMPIndexHead
{
public:
	long nCMPNumber;
	long nGroupHeadPointNumber;
	long nGroupMinPointNumber;
	long nGroupMaxPointNumber;
	long nSampleInterval;
	long nSEGY_PointSize;
	long nSEGY_GroupHeadSize;
	enumSeisDataSrc m_DataSrc; 

	

	CCMPIndex index[10000];

	CCMPIndexHead()
	{
  		nCMPNumber=0;
		nGroupHeadPointNumber=60;
		nGroupMinPointNumber=0;
		nGroupMaxPointNumber=0;

		nSampleInterval=0;
		nSEGY_PointSize=4;
		nSEGY_GroupHeadSize=240;
		m_DataSrc=MICROCOMPUTER;
		memset(index,0,sizeof(CCMPIndex)*10000);
	};


};


class CCMPIndexDoc 
{

public:
	///////////////////////////////////////////////////////////////
	// On the head of the index file is some parameters:
	CCMPIndexHead head;
	long m_nCMPLimit;  // The biggest CMP number in the CMP file;
	double m_dCMPDis;  // The common CMP distance ;

protected:
	FILE *m_fpIndexFile;
	CString m_sIndexExt;
	CCMPGroupMsg *m_pCMPGroupMsg; // contains information of all of the groups on the CMP;
	
public:
	CCMPIndexDoc();           // protected constructor used by dynamic creation
	~CCMPIndexDoc();
	
	bool OpenIndex(CString sCMPFile) ;
	CCMPGroupMsg* GetCMPMsg(long nCMPOrder,long &nGroupNumber);
	CCMPGroupMsg* GetCMPMsgByStation(long nCMPPh,long &nGroupNumber);
	CCMPGroupMsg* GetCMPMsg(long east,long north,long &nGroupNumber);

public:
	bool CreateIndex(CString sCMPFile);

public:
	bool CalcCMPGroupPos();
	void SortCMPGroupMsg(CCMPGroupMsg *pGroup,int nGroup);
	void SortCMPIndexMsg(CCMPIndex *pIndex, int nIndex);	

	bool SearchGroupOrderAmongCMPByShotRcv(double dShotPh,double dRcvPh,long &nCMPOrder,long &nGroupOrderInCMP);
	long SearchGroupOrderAmongCMPByRcv(long nCMPOrder, long nRcvPh);
	long SearchGroupOrderAmongCMPByShot(long nCMPOrder, long nShotPh);
	long SearchCMPStation(double dCMPPh); 

public:
	CCMPGroupMsg* GetGroupMsg(long nCMPOrder, long nGroupOrderInCMP);
	bool GetShotStation(CSegYGroup &group,double &nShotStation,double &nShotNorth,double &nShotEast);
	bool GetRcvStation(CSegYGroup &group,double &nRcvStation,double &nRcvNorth,double &nRcvEast);
	bool GetCMPStation(CSegYGroup &group,double &nCMPStation,double &nCMPNorth,double &nCMPEast);
	
	CString GetIndexFileName(CString sCMPFile);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CMPINDEXDOC_H__6DA7ED61_79CF_11D4_AB15_0080AD72BDDD__INCLUDED_)
