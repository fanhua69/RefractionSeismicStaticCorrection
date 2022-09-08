// CRPIndexDoc.cpp : implementation file
//

#include "stdafx.h"
#include "global.h"
#include "fdata.h"
#include "CRPIndexDoc.h"
#include "ProgDlg.h"
#include "FHZoomViewGlobal.h"

CCRPIndexDoc::CCRPIndexDoc()
{
	m_sIndexExt=".rdx";
	m_fpCRPIndex=NULL;
	m_pCRPGroupMsg=NULL;
}


CCRPIndexDoc::~CCRPIndexDoc()
{			   
	if(m_pCRPGroupMsg){
		delete []m_pCRPGroupMsg;
		m_pCRPGroupMsg=NULL;
	}
	if(m_fpCRPIndex){
		fclose(m_fpCRPIndex);
		m_fpCRPIndex=NULL;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CCRPIndexDoc commands

bool CCRPIndexDoc::OpenIndex(CString sCRPFile) 
{
	CString sIndex=GetIndexFileName(sCRPFile);

	m_fpCRPIndex=fopen(sIndex,"rb");
	if(!m_fpCRPIndex){
		if(!CreateIndexFile(sCRPFile))return false;
		m_fpCRPIndex=fopen(sIndex,"rb");
		if(!m_fpCRPIndex){
			AfxMessageBox("Error: error occurred when creating the index:"+sIndex+" file for:"+sCRPFile);
			return false;
		}
	}

	//////////////////////////////////
	// Read the index documnet:
	fseek(m_fpCRPIndex,0,SEEK_SET);
	fread(&m_indexHead,sizeof(CCRPIndexHead ),1,m_fpCRPIndex);

	if(m_indexHead.nCRPNumber <1||m_indexHead.nCRPNumber >1000000||m_indexHead.nSampleInterval <1||m_indexHead.nSampleInterval >10000){
		AfxMessageBox("Error: the index file of the CRP file is error, the sample interval :"+vtos(m_indexHead.nSampleInterval )+" The CRP index number: "+vtos(m_indexHead.nCRPNumber )+", you can try to delete it, and open the CRP file again!");
		fclose(m_fpCRPIndex);
		return false;
	}

	if(m_indexHead.nSampleInterval >1000){
		m_indexHead.nSampleInterval /=1000;
	}

	//////////////////////////////
	// Get the max group number:
	long nMaxGroupNumber=0;
	long nGroupTotalNumber=0;
	for(long i=0;i<m_indexHead.nCRPNumber ;i++){		
		nGroupTotalNumber+=m_indexHead.index [i].nGroupNumber ;
		if(m_indexHead.index [i].nGroupNumber >nMaxGroupNumber) {
			nMaxGroupNumber=m_indexHead.index [i].nGroupNumber ;
		}
	}

	if(m_pCRPGroupMsg){
		delete []m_pCRPGroupMsg;
		m_pCRPGroupMsg=NULL;
	}
	m_pCRPGroupMsg=new CCRPGroupMsg[nMaxGroupNumber+10];

	m_indexHead.nMaxShotOnOneRcv =nMaxGroupNumber;
	m_indexHead.nGroupTotalNumber=nGroupTotalNumber;
	
	////////////////////////////
	//
	return TRUE;
}

bool CCRPIndexDoc::CreateIndexFile(CString sCRPFile)
{
	//////////////////////////////////////////////
	// Open the CRP aznd the index file:
	FILE *fpCRP=fopen(sCRPFile,"rb");
	if(!fpCRP){
		AfxMessageBox("Error: can not open the CRP file:"+sCRPFile);
		return false;
	}
	
	CString sIndex=GetIndexFileName(sCRPFile);
	FILE *fpIndex=fopen(sIndex,"wb");
	if(!fpIndex){
		AfxMessageBox("Error: can not create index file:"+sIndex);
		return false;
	}

	////////////////////////////////////////////////
	//
	CProgressDlg dlgPro;
	dlgPro.Create ();
	dlgPro.SetStatus("Creating index for CRP file:"+sCRPFile);
	fseek(fpCRP,0,SEEK_END);
	dlgPro.SetRange (0,ftell(fpCRP));

	///////////////////////////////////////////////
	// Get the CRP head out:	
	CSegYGroup group;
	fseek(fpCRP,sizeof(CSegYVolHead),SEEK_SET);
	fread(&group,sizeof(CSegYGroup),1,fpCRP);	

	long nGroupOnOneCRPLimit=10000;
	CCRPGroupMsg crpGroupMsg[10000];
	CCRPIndexHead crpHead;

	crpHead.nGroupHeadPointNumber =60;
	crpHead.nGroupBodyPointNumber =group.nGroupSamplePoint ;
	crpHead.nGroupTotalPointNumber =crpHead.nGroupHeadPointNumber +crpHead.nGroupBodyPointNumber ;
	crpHead.nSampleInterval =group.nGroupSampleInterval;

	long j,n,nRcvStationLast,nRcvStation;
	long nGroupSize=sizeof(float)*crpHead.nGroupTotalPointNumber ;

	nRcvStationLast=group.nRcvStation ;
	crpGroupMsg[0].nShotPh =group.nShotStation;

	long nShotNumber=1;
	long nGroupOrder=1;
	long nCRPOrder=0;	

	bool bFailed=false;
	while(!feof(fpCRP)){

		dlgPro.SetPos (ftell(fpCRP));

		if(nShotNumber>=nGroupOnOneCRPLimit){
			AfxMessageBox("Error: there are too many shots:"+vtos(nShotNumber)+" on the reciever:"+vtos(nRcvStationLast));
			bFailed=true;
			break;
		}
		
		fseek(fpCRP,sizeof(CSegYVolHead)+nGroupOrder*nGroupSize,SEEK_SET);
		n=fread(&group,1,nGroupSize,fpCRP);
		if(n==0){
			break;
		}
		else if(n<nGroupSize){
			AfxMessageBox("Error: the seismic data file is error on size:"+sCRPFile);
			bFailed=true;
			break;
		}

		nRcvStation=group.nRcvStation ;
		if(nRcvStation!=nRcvStationLast){

			if(nCRPOrder>=crpHead.nCRPLimit ){
				AfxMessageBox("Error: there are too many CRPs:"+vtos(nCRPOrder));
				bFailed=true;
				break;
			}

			///////////////////////////////////
			// The Rcv Station in The Head :
			crpHead.index [nCRPOrder].nRcvPh =nRcvStationLast;
			crpHead.index [nCRPOrder].nGroupNumber =nShotNumber;
			crpHead.index [nCRPOrder].pos =nGroupOrder-nShotNumber;

			//////////////////////////////////
			// Write the CCRPIndexMsg:
			for(j=0;j<nShotNumber;j++){
				crpGroupMsg[j].nGroupNumber =nShotNumber;
				crpGroupMsg[j].nRcvPh =nRcvStationLast;				
			}

			fseek(fpIndex,sizeof(CCRPIndexHead)+sizeof(CCRPGroupMsg)*crpHead.index [nCRPOrder].pos ,SEEK_SET);
			fwrite(&crpGroupMsg[0],sizeof(CCRPGroupMsg),nShotNumber,fpIndex);

			///////////////////////////////////////
			// increse the variables:
			nRcvStationLast=nRcvStation;
			nCRPOrder++;
			nShotNumber=0;
		}
		
		crpGroupMsg[nShotNumber].nShotPh =group.nShotStation ;
		nShotNumber++;
		nGroupOrder++;
	}
	
	////////////////////////////////////////////
	// Write the last CRP into the index file:
	crpHead.index [nCRPOrder].nRcvPh =nRcvStationLast;
	crpHead.index [nCRPOrder].nGroupNumber =nShotNumber;
	crpHead.index [nCRPOrder].pos =nGroupOrder-nShotNumber;

	crpHead.nCRPNumber =nCRPOrder+1;
	fseek(fpIndex,0,SEEK_SET);
	fwrite(&crpHead,sizeof(CCRPIndexHead),1,fpIndex);

	////////////////////////////////////////////////
	// Write last CRP groups message to the index :
	for(j=0;j<nShotNumber;j++){
		crpGroupMsg[j].nGroupNumber =nShotNumber;
		crpGroupMsg[j].nRcvPh =nRcvStationLast;				
	}
	fseek(fpIndex,sizeof(CCRPIndexHead)+sizeof(CCRPGroupMsg)*crpHead.index [nCRPOrder].pos ,SEEK_SET);
	fwrite(&crpGroupMsg[0],sizeof(CCRPGroupMsg),nShotNumber,fpIndex);

	/////////////////////////////
	//
	dlgPro.DestroyWindow ();

	fclose(fpCRP);
	fclose(fpIndex);

	return !bFailed;
}

CString CCRPIndexDoc::GetIndexFileName(CString sCRPFile)
{
	return SeperatePathName(sCRPFile)+m_sIndexExt;
}

bool CCRPIndexDoc::GetGroupMsg(long nCRPOrder, long nGroupOrderInCRP,CCRPGroupMsg *pGroupMsg)
{
	if(!m_fpCRPIndex){
		AfxMessageBox("Error: there is NO CRP index file handle!");
		return false;
	}
	if(nCRPOrder<0||nCRPOrder>=m_indexHead.nCRPNumber ){
		AfxMessageBox("Error: the wanted CRP is out of range:"+vtos(nCRPOrder));
		return false;
	}
	if(nGroupOrderInCRP<0||nGroupOrderInCRP>=m_indexHead.index [nCRPOrder].nGroupNumber ){
		AfxMessageBox("Error: the wanted group order is out of range:"+vtos(nGroupOrderInCRP));
		return false;
	}

	long nPos=sizeof(CCRPIndexHead)+(m_indexHead.index [nCRPOrder].pos + nGroupOrderInCRP)*sizeof(CCRPGroupMsg);
	fseek(m_fpCRPIndex,nPos,SEEK_SET);
	UINT n=fread(pGroupMsg,1,sizeof(CCRPGroupMsg),m_fpCRPIndex);

	return (n==sizeof(CCRPGroupMsg));
}

CCRPGroupMsg * CCRPIndexDoc::GetCRPGroupMsg(long nCRPOrder)
{	
	if(!m_fpCRPIndex){
		AfxMessageBox("Error: there is NO CRP index file handle!");
		return NULL;
	}
	if(nCRPOrder<0||nCRPOrder>=m_indexHead.nCRPNumber ){
		AfxMessageBox("Error: the wanted CRP is out of range:"+vtos(nCRPOrder));
		return NULL;
	}
	if(!m_pCRPGroupMsg){
		AfxMessageBox("Error: the index memory heas not been set!");
		return NULL;
	}

	long nPos=sizeof(CCRPIndexHead)+m_indexHead.index [nCRPOrder].pos *sizeof(CCRPGroupMsg);
	fseek(m_fpCRPIndex,nPos,SEEK_SET);
	UINT n=fread(m_pCRPGroupMsg,sizeof(CCRPGroupMsg),m_indexHead.index [nCRPOrder].nGroupNumber ,m_fpCRPIndex);

	if(n==m_indexHead.index [nCRPOrder].nGroupNumber )
		return m_pCRPGroupMsg;
	else
		return NULL;

}
