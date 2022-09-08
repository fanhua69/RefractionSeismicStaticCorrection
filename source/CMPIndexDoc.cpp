// CMPIndexDoc.cpp : implementation file
//

#include "stdafx.h"
#include "global.h"
#include "fdata.h"
#include "CMPIndexDoc.h"
#include "math.h"
#include "ProgDlg.h"
#include "FHZoomViewGlobal.h"
#include <sys/types.h>
#include <sys/stat.h>


/////////////////////////////////////////////////////////////////////////////
// CCMPIndexDoc
CCMPIndexDoc::CCMPIndexDoc()
{
	m_fpIndexFile=NULL;
	m_pCMPGroupMsg=NULL;
	m_sIndexExt=".mdx";	 
	m_nCMPLimit=10000;
	m_dCMPDis=5;
}

bool CCMPIndexDoc::CreateIndex(CString sCMPFile)
{
	//////////////////////////////////////////////
	// Open the CMP and the index file:
	FILE *fpCMP=fopen(sCMPFile,"rb");
	if(!fpCMP){
		AfxMessageBox("Error: can not open the CMP file:"+sCMPFile);
		return false;
	}
	
	CString sIndex=GetIndexFileName(sCMPFile);
	FILE *fpIndex=fopen(sIndex,"wb");
	if(!fpIndex){
		AfxMessageBox("Error: can not create index file:"+sIndex);
		return false;
	}

	///////////////////////////////////////////////
	// Get the CRP head out:
	CSegYVolHead VolHead;
	fseek(fpCMP,0,SEEK_SET);
	fread(&VolHead,sizeof(CSegYVolHead),1,fpCMP);
	if(!VolHead.IsValid ())
	{
		VolHead.Reverse ();
		if(!VolHead.IsValid ())
		{
			AfxMessageBox("Error: the data volume head is error!");
			fclose(fpIndex);
			fclose(fpCMP);
			return false;
		}
		head.m_DataSrc =WORKSTATION;
	}
	else
	{
		head.m_DataSrc =MICROCOMPUTER;
	}


	////////////////////////////////////////////////
	//
	CProgressDlg dlgPro;
	dlgPro.Create ();
	dlgPro.SetStatus("Creating index for CMP file:"+sCMPFile);
	fseek(fpCMP,0,SEEK_END);
	long nFileLen=ftell(fpCMP);
	dlgPro.SetRange (0,nFileLen);

	long n,nFilePosition,nGroupOrder,nCMPOrder,nGroupNumberOnCMP;
	double nCMPStationLast,nCMPStation,nCMPEastLast,nCMPEast,nCMPNorthLast,nCMPNorth;
	double dShotStation,dRcvStation,dShotNorth,dShotEast,dRcvNorth,dRcvEast;
	double dx,dy,dl;

	CSegYGroup group;
	CCMPGroupMsg cmpGroupMsg;

	head.nGroupMaxPointNumber =0;
	head.nGroupMinPointNumber =100000000;

	bool bFailed=false;

	while(true){  // Use its break function only;

		fseek(fpCMP,sizeof(CSegYVolHead),SEEK_SET);
		n=fread(&group,1,sizeof(CSegYGroup),fpCMP);
		if(n==0){
			break;
		}
		
		if(head.m_DataSrc==WORKSTATION)group.ReverseGroup();		

		//////////////////////////////////////////////////////
		// Set the info of the CMP, and the first group:
		if(!GetCMPStation(group,nCMPStationLast,nCMPNorthLast,nCMPEastLast)){
			AfxMessageBox("Error: the group heads have not been set!");
			bFailed=true;
			break;
		}

		head.nGroupHeadPointNumber =60;
		head.nSampleInterval =group.nGroupSampleInterval ;
		head.nSEGY_PointSize =4;

		GetShotStation(group,dShotStation,dShotNorth,dShotEast);
		GetRcvStation(group,dRcvStation,dRcvNorth,dRcvEast);

		nGroupNumberOnCMP=1;
		nGroupOrder=1;
		nCMPOrder=0;	
		nFilePosition=sizeof(CSegYVolHead);

		cmpGroupMsg.nPosInFile = nFilePosition;
		cmpGroupMsg.dStation =nCMPStationLast;
		cmpGroupMsg.dEast =nCMPEastLast;
		cmpGroupMsg.dNorth=nCMPNorthLast;
		cmpGroupMsg.dShotPh =dShotStation;
		cmpGroupMsg.dShotNorth =dShotNorth;
		cmpGroupMsg.dShotEast=dShotEast;
		cmpGroupMsg.dRcvPh =dRcvStation;
		cmpGroupMsg.dRcvNorth =dRcvNorth;
		cmpGroupMsg.dRcvEast=dRcvEast;
		cmpGroupMsg.nGroupSamplePoint=group.nGroupSamplePoint ;

		fseek(fpIndex,sizeof(CCMPIndexHead),SEEK_SET);
		fwrite(&cmpGroupMsg,sizeof(CCMPGroupMsg),1,fpIndex);

		/////////////////////////////////////////////////////
		// Record the max and min sample point number
		if(group.nGroupSamplePoint>head.nGroupMaxPointNumber)
			head.nGroupMaxPointNumber=group.nGroupSamplePoint;
		if(group.nGroupSamplePoint<head.nGroupMinPointNumber)
			head.nGroupMinPointNumber=group.nGroupSamplePoint;



		/////////////////////////////////////
		// The later groups:
		while(true){  // Will break if 0 data has been read;

			if(group.nGroupSamplePoint <1||group.nGroupSamplePoint >20000){
				bFailed=true;
				break;
			}

			dlgPro.SetPos (nFilePosition);

			nFilePosition+=(head.nGroupHeadPointNumber +group.nGroupSamplePoint )*sizeof(float);

			cmpGroupMsg.nPosInFile = nFilePosition;
			
			fseek(fpCMP,nFilePosition,SEEK_SET);
			n=fread(&group,1,sizeof(CSegYGroup),fpCMP);				

			if(n==0){  // File End is Reached;
				break;
			}
			
			if(head.m_DataSrc==WORKSTATION)group.ReverseGroup();

			if(!GetCMPStation(group,nCMPStation,nCMPNorth,nCMPEast)){
				bFailed=true;
				break;
			}

			GetShotStation(group,dShotStation,dShotNorth,dShotEast);
			GetRcvStation(group,dRcvStation,dRcvNorth,dRcvEast);


			//////////////////////////////////
			// Write the Group Message:
			cmpGroupMsg.nPosInFile = nFilePosition;
			cmpGroupMsg.dStation =nCMPStation;
			cmpGroupMsg.dEast =nCMPEast;
			cmpGroupMsg.dNorth=nCMPNorth;
			cmpGroupMsg.dShotPh =dShotStation;
			cmpGroupMsg.dShotNorth =dShotNorth;
			cmpGroupMsg.dShotEast=dShotEast;
			cmpGroupMsg.dRcvPh =dRcvStation;
			cmpGroupMsg.dRcvNorth =dRcvNorth;
			cmpGroupMsg.dRcvEast=dRcvEast;
			cmpGroupMsg.nGroupSamplePoint=group.nGroupSamplePoint ;

			fseek(fpIndex,sizeof(CCMPIndexHead)+nGroupOrder*sizeof(CCMPGroupMsg),SEEK_SET);
			fwrite(&cmpGroupMsg,sizeof(CCMPGroupMsg),1,fpIndex);
			
			/////////////////////////////////////////////////////
			// Record the max and min sample point number
			if(group.nGroupSamplePoint>head.nGroupMaxPointNumber)
				head.nGroupMaxPointNumber=group.nGroupSamplePoint;
			if(group.nGroupSamplePoint<head.nGroupMinPointNumber)
				head.nGroupMinPointNumber=group.nGroupSamplePoint;

			///////////////////////////////////
			// Check if a new CMP has reached:
			dx=nCMPEastLast-nCMPEast;
			dy=nCMPNorthLast-nCMPNorth;
			dl=sqrt(dx*dx+dy*dy);
			//if(dl>m_dCMPDis){
			if(nCMPStation!=nCMPStationLast){
				if(nCMPOrder>m_nCMPLimit){
					AfxMessageBox("´íÎó£ºCMPÊýÄ¿Ì«¶à!");
					bFailed=true;
					break;
				}

				///////////////////////////////////
				// The Rcv Station in The Head :
				head.index [nCMPOrder].nStation =nCMPStationLast;
				head.index [nCMPOrder].east =nCMPEastLast;
				head.index [nCMPOrder].north =nCMPNorthLast;
				head.index [nCMPOrder].nGroupNumber = nGroupNumberOnCMP;
				head.index [nCMPOrder].pos = nGroupOrder-nGroupNumberOnCMP;			


				///////////////////////////////////////
				// increse the variables:
				nCMPStationLast=nCMPStation;
				nCMPEastLast=nCMPEast ;
				nCMPNorthLast=nCMPNorth;

				nCMPOrder++;
				nGroupNumberOnCMP=0;
			}
			
			nGroupNumberOnCMP++;
			nGroupOrder++;
		}

		break;  // from "while"
	}

	if(!bFailed){
		
		/////////////////////////////////////////////////////
		// Write the last one CMP Head into the index file:
		head.index [nCMPOrder].nStation =nCMPStation;
		head.index [nCMPOrder].east =nCMPEast;
		head.index [nCMPOrder].north=nCMPNorth;
		head.index [nCMPOrder].nGroupNumber = nGroupNumberOnCMP;
		head.index [nCMPOrder].pos = nGroupOrder-nGroupNumberOnCMP;
		head.nCMPNumber =nCMPOrder+1;

		fseek(fpIndex,0,SEEK_SET);
		int n=fwrite(&head,sizeof(CCMPIndexHead),1,fpIndex);		
		if(n!=1){
			AfxMessageBox("Error: can not write file head to the CMP index file:");
			bFailed=true;
		}
	}

	/////////////////////////////
	//
	dlgPro.DestroyWindow ();
	fclose(fpCMP);
	fclose(fpIndex);

	return !bFailed;
}

CCMPIndexDoc::~CCMPIndexDoc()
{
	if(m_fpIndexFile){
		fclose(m_fpIndexFile);
		m_fpIndexFile=NULL;
	}
	if(m_pCMPGroupMsg){
		delete []m_pCMPGroupMsg;
		m_pCMPGroupMsg=NULL;
	}
}


/////////////////////////////////////////////////////////////////////////////
// CCMPIndexDoc commands

bool CCMPIndexDoc::OpenIndex(CString sCMPFile) 
{
	// TODO: Add your specialized creation code here
	if(m_fpIndexFile){
		fclose(m_fpIndexFile);
		m_fpIndexFile=NULL;
	}

	//////////////////////////////////////////////////////////
	// Check if the index file is later than the CMP file:
	FILE *fp=fopen(sCMPFile,"rb");
	struct _stat stCMP;
	if(_fstat( _fileno(fp), &stCMP)!=0)return false;
	fclose(fp);	  

	//
	CString sIndexFile=GetIndexFileName(sCMPFile);
	m_fpIndexFile=fopen(sIndexFile,"rb");
	if(!m_fpIndexFile){
		AfxMessageBox("Can NOT open the CMP file index data file:"+sIndexFile+"!");
		return false;
	}

	struct _stat stCMPIndex;
	if(_fstat( _fileno(m_fpIndexFile), &stCMPIndex)!=0)return false;

	if(stCMP.st_mtime > stCMPIndex.st_mtime ){
		AfxMessageBox("The index is earlier than the CMP file:"+sIndexFile+"!");
		fclose(m_fpIndexFile);
		m_fpIndexFile=NULL;
		return false;
	}

	//////////////////////////////////////////////////////////
	// Read the index head:
	int n=fread(&head,sizeof(CCMPIndexHead),1,m_fpIndexFile);
	if(n!=1){
		fclose(m_fpIndexFile);
		m_fpIndexFile=NULL;
		return false;
	}

	if(head.nCMPNumber<1||head.nCMPNumber>m_nCMPLimit ){
		AfxMessageBox("Error: the CMP message file is error!");
		fclose(m_fpIndexFile);
		m_fpIndexFile=NULL;
		return false;
	}

	if(head.nSampleInterval >1000)head.nSampleInterval /=1000; // change to ms;

	CalcCMPGroupPos();
	SortCMPIndexMsg(&head.index [0], head.nCMPNumber );


	////////////////////////////////////////////////////
	//
	if(m_pCMPGroupMsg){
		delete []m_pCMPGroupMsg;
		m_pCMPGroupMsg=NULL;
	}
	long nMaxGroup=0;
	for(long i=0;i<head.nCMPNumber ;i++){
		if(head.index [i].nGroupNumber >nMaxGroup){
			nMaxGroup=head.index [i].nGroupNumber ;
		}
	}
	if(nMaxGroup<1||nMaxGroup>2000){
		AfxMessageBox("Error: there are too many groups on one CMP:"+vtos(nMaxGroup));
		fclose(m_fpIndexFile);
		m_fpIndexFile=NULL;
		return false;
	}
	m_pCMPGroupMsg=new CCMPGroupMsg[nMaxGroup+10];

	return true;

}

CCMPGroupMsg *CCMPIndexDoc::GetCMPMsg(long east, long north,long &nGroupNumber)
{
	////////////////////////////////////////////////
	//
	double dx,dy,dDis,dMinDis=99999999999999999.9;
	long nMinCMP=-1;
	for(long i=0;i<head.nCMPNumber ;i++){
		dx=east-head.index [i].east ;
		dy=north-head.index [i].north;

		dDis=sqrt(dx*dx+dy*dy);
		if(dDis<dMinDis){
			dMinDis=dDis;
			nMinCMP=i;
		}
	}

	if(nMinCMP==-1){
		AfxMessageBox("Program error, can not find the nearest CMP point!");
		return false;
	}

	//
	return GetCMPMsg(nMinCMP,nGroupNumber);
}



CCMPGroupMsg * CCMPIndexDoc::GetCMPMsgByStation(long nCMPPh, long &nGroupNumber)
{
	///////////////////////////////////
	//
	long nOrder=SearchCMPStation(nCMPPh);
	if(nOrder==-1){
		AfxMessageBox("Program error, can not find the CMP point of position:"+vtos(nCMPPh));
		return NULL;
	}

	////////////////////////////////////
	//
	return GetCMPMsg(nOrder,nGroupNumber);

}


////////////////////////////////////////////////////////////////
// Sort the group information according to the station number:
void CCMPIndexDoc::SortCMPGroupMsg(CCMPGroupMsg *pGroupMsg, int nGroup)
{
	// Set comparison offset to half the number of records in SortArray:
	int Offset = nGroup/ 2;
	int Limit;
	int Switch;
	CCMPGroupMsg temp;
	
	// Loop until offset gets to zero.
	while(Offset > 0){ //
		Limit = nGroup- Offset;
		do{
		   Switch = 0; // Assume no switches at this offset.

		   // Compare elements and switch ones out of order:
		   for(int Row = 0;Row<Limit;Row++){
			   if( pGroupMsg[Row].dRcvPh > pGroupMsg[Row + Offset].dRcvPh ){
				   temp=pGroupMsg[Row];
				   pGroupMsg[Row]=pGroupMsg[Row+Offset];
				   pGroupMsg[Row+Offset]=temp;
				   Switch = Row;
			   }
		   }

		   // Sort on next pass only to where last switch was made:
		   Limit = Switch - Offset+1;
		}while(Switch);

		// No switches at last offset, try one half as big:
		if(Offset==1)
			break;
		else
			Offset = (Offset +1)/ 2;
	}

	return ;
}

//////////////////////////////////////////////////////////////
// Sort the CMP POSITION according to the CMP Stations:
void CCMPIndexDoc::SortCMPIndexMsg(CCMPIndex *pIndex, int nIndex)
{
	// Set comparison offset to half the number of records in SortArray:
	int Offset = nIndex/ 2;
	int Limit;
	int Switch;
	CCMPIndex temp;
	
	// Loop until offset gets to zero.
	while(Offset > 0){ //
		Limit = nIndex- Offset;
		do{
		   Switch = 0; // Assume no switches at this offset.

		   // Compare elements and switch ones out of order:
		   for(int Row = 0;Row<Limit;Row++){
			   if( pIndex[Row].nStation > pIndex[Row + Offset].nStation){	
				   temp=pIndex[Row];
				   pIndex[Row]=pIndex[Row+Offset];
				   pIndex[Row+Offset]=temp;
				   Switch = Row;
			   }
		   }

		   // Sort on next pass only to where last switch was made:
		   Limit = Switch - Offset+1;
		}while(Switch);

		// No switches at last offset, try one half as big:
		if(Offset==1)
			break;
		else
			Offset = (Offset +1)/ 2;
	}

	return ;
}


bool CCMPIndexDoc::CalcCMPGroupPos()
{
	if(head.nCMPNumber<0)return false;

	head.index [0].pos =0;
	for(long i=1;i<head.nCMPNumber;i++){
		head.index [i].pos =head.index [i-1].pos +head.index [i-1].nGroupNumber ;
	}

	return true;
}

CCMPGroupMsg *CCMPIndexDoc::GetCMPMsg(long nCMPOrder,long &nGroupNumber)
{
	if(!m_fpIndexFile||!m_pCMPGroupMsg)return NULL;
	if(nCMPOrder<0||nCMPOrder>=head.nCMPNumber )return NULL;

	nGroupNumber=head.index [nCMPOrder].nGroupNumber ;
	
	long pos=sizeof(CCMPIndexHead)+head.index[nCMPOrder].pos *sizeof(CCMPGroupMsg);

	fseek(m_fpIndexFile,pos,SEEK_SET);	
	int n=fread(m_pCMPGroupMsg,sizeof(CCMPGroupMsg),nGroupNumber,m_fpIndexFile);
	if(n!=nGroupNumber)return NULL;

	return m_pCMPGroupMsg;
}

long CCMPIndexDoc::SearchCMPStation(double  dCMPPh)
{
	int nStart=0,nEnd=head.nCMPNumber -1,nMid;
	if(head.nCMPNumber==1)
	{
		if(fabs(head.index [0].nStation-dCMPPh)<0.1)
		{
			return 0;
		}
		else{
			return -1;
		}
	}
	
	float dLimit=0.6;	
	if(fabs(head.index [1].nStation-head.index [0].nStation)<5)
	{
		dLimit=0.01;
	}

	while(true)
	{
		nMid=nStart+(nEnd-nStart)/2;
		
		//
		if(fabs(head.index [nMid].nStation-dCMPPh)<0.6)return nMid;
		if(nMid==nStart||nMid==nEnd){
			if(fabs(head.index [nStart].nStation-dCMPPh)<dLimit)
				return nStart;
			else if(fabs(head.index [nEnd].nStation-dCMPPh)<dLimit)
				return nEnd;
			else
				return -1;
		}

		// 
		if(head.index [nMid].nStation <dCMPPh)
			nStart=nMid;
		else
			nEnd=nMid;
		
	}
}

long CCMPIndexDoc::SearchGroupOrderAmongCMPByShot(long nCMPOrder, long nShotPh)
{
	long nGroupNumber;
	CCMPGroupMsg *pMsg=GetCMPMsg(nCMPOrder,nGroupNumber);
	if(!pMsg)return -1;
	for(long i=0;i<nGroupNumber ;i++){
		if(pMsg[i].dShotPh ==nShotPh)return i;
	}
	return -1;
}

long CCMPIndexDoc::SearchGroupOrderAmongCMPByRcv(long nCMPOrder, long nRcvPh)
{	
	long nGroupNumber;
	CCMPGroupMsg *pMsg=GetCMPMsg(nCMPOrder,nGroupNumber);
	for(long i=0;i<nGroupNumber;i++){
		if(pMsg[i].dRcvPh ==nRcvPh)return i;
	}
	return -1;
}

bool CCMPIndexDoc::SearchGroupOrderAmongCMPByShotRcv(double dShotPh,
													 double dRcvPh,
													 long &nCMPOrder,
													 long &nGroupOrderInCMP)
{	
	// Search the CMP order:
	nCMPOrder=SearchCMPStation((dShotPh+dRcvPh)/2);
	if(nCMPOrder==-1)return false;

	// Search the group order in the CMP:
	long nGroupNumber;
	CCMPGroupMsg *pMsg=GetCMPMsg(nCMPOrder,nGroupNumber);
	for(long i=0;i<nGroupNumber;i++){
		if(fabs(pMsg[i].dRcvPh-dRcvPh)<0.00001&&
			fabs(pMsg[i].dShotPh-dShotPh)<0.00001){
			nGroupOrderInCMP=i;			
			return true;
		}
	}
	return false;
}



CString CCMPIndexDoc::GetIndexFileName(CString sCMPFile)
{
	return SeperatePathName(sCMPFile)+m_sIndexExt;
}


bool CCMPIndexDoc::GetCMPStation(CSegYGroup &group,double &nCMPStation,double &nCMPNorth,double &nCMPEast)
{
	nCMPEast=group.nCMPEast ;
	nCMPNorth=group.nCMPNorth;
	if(nCMPEast==0&&nCMPNorth==0){
		nCMPEast=(group.nShotEast +group.nRcvEast )/2;
		nCMPNorth=(group.nShotNorth +group.nRcvNorth )/2;
	}


	if(group.nCMPStation!=0)
		nCMPStation=group.nCMPStation ;
	else if(group.nShotStation !=0&&group.nRcvStation !=0)
		nCMPStation=(group.nShotStation +group.nRcvStation)/2;
	else if(group.nCMPEast !=0)
		nCMPStation=group.nCMPEast ;
	else if(group.nCMPNorth !=0)
		nCMPStation=group.nCMPNorth ;
	else if(nCMPNorth!=0)
		nCMPStation=nCMPNorth;
	else if(nCMPEast!=0)
		nCMPStation=nCMPEast;
	else
		nCMPStation=0;

	return nCMPStation!=0;
}

bool CCMPIndexDoc::GetShotStation(CSegYGroup &group,double &nShotStation,double &nShotNorth,double &nShotEast)
{
	// Shot North:
	if(group.nShotNorth!=0)
		nShotNorth=group.nShotNorth;
	else if(group.nShotStation !=0)
		nShotNorth=group.nShotStation;
	else
		nShotNorth=0;

	// Shot East:
	if(group.nShotEast!=0)
		nShotEast=group.nShotEast;
	else if(group.nShotStation !=0)
		nShotEast=group.nShotStation;
	else
		nShotEast=0;

	// Shot Station:
	if(group.nShotStation !=0)
		nShotStation=group.nShotStation ;
	else if(group.nShotNorth!=0)
		nShotStation=group.nShotNorth ;
	else if(group.nShotEast!=0)
		nShotStation=group.nShotEast ;

	return true;
}

bool CCMPIndexDoc::GetRcvStation(CSegYGroup &group,double &nRcvStation,double &nRcvNorth,double &nRcvEast)
{
	// Rcv North:
	if(group.nRcvNorth!=0)
		nRcvNorth=group.nRcvNorth;
	else if(group.nRcvStation !=0)
		nRcvNorth=group.nRcvStation;
	else
		nRcvNorth=0;

	// Rcv East:
	if(group.nRcvEast!=0)
		nRcvEast=group.nRcvEast;
	else if(group.nRcvStation !=0)
		nRcvEast=group.nRcvStation;
	else
		nRcvEast=0;

	// Rcv Station:
	if(group.nRcvStation !=0)
		nRcvStation=group.nRcvStation ;
	else if(group.nRcvNorth!=0)
		nRcvStation=group.nRcvNorth ;
	else if(group.nRcvEast!=0)
		nRcvStation=group.nRcvEast ;

	return true;

}

CCMPGroupMsg* CCMPIndexDoc::GetGroupMsg(long nCMPOrder, long nGroupOrderInCMP)
{
	/////////////////////////////////////////////////////
	// Check if the parameters are correct:
	if(!m_fpIndexFile||!m_pCMPGroupMsg)return NULL;
	if(nCMPOrder<0||nCMPOrder>=head.nCMPNumber )return NULL;
	if(nGroupOrderInCMP<0||nGroupOrderInCMP>=head.index [nCMPOrder].nGroupNumber )return NULL;

	//////////////////////////////////////////////////
	// Get the position of the wanted group in the CMP file:
	long nGroup=head.index [nCMPOrder].pos +nGroupOrderInCMP;	
	long pos=sizeof(CCMPIndexHead)+sizeof(CCMPGroupMsg)*nGroup;
	
	//////////////////////////////////////////////////
	// Get the Info of the cmp out from the file:
	fseek(m_fpIndexFile,pos,SEEK_SET);
	fread(m_pCMPGroupMsg,1,sizeof(CCMPGroupMsg),m_fpIndexFile);	

	//////////////////////////////////////////////////
	// return:
	return m_pCMPGroupMsg;
}

