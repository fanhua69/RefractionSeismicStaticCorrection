// CRPDoc.cpp : implementation file
//

#include "stdafx.h"
#include "global.h"
#include "fdata.h"
#include "CRPDoc.h"
#include "ProgDlg.h"
#include "FHZoomViewGlobal.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCRPDoc

IMPLEMENT_DYNCREATE(CCRPDoc, CDocument)

CCRPDoc::CCRPDoc()
{
	m_nStartPoint=0;
	m_nEndPoint=0;
	m_nMaxGroupNumber=0;
	m_nSegYGroupHeadSize=240;

	m_fpCRPFile=NULL;
	m_pSvSysDoc=NULL;

	m_pStackSectionDocTest=NULL;
	m_pStackSectionDocCurr=NULL;

	memset (&m_nValidCMPRange[0],0,VALID_CMP_RANGE_LIMIT*sizeof(CRange));
	m_nValidCMPRangeNumber=0;

	m_pStackSectionDocCurr= new CStackSectionDoc;
	m_pStackSectionDocTest= new CStackSectionDoc;

}

BOOL CCRPDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// Make the CMP file:
	CString sCRPFile;
	if(!MakeCRPFile(NULL,NULL,sCRPFile))return false;

	// Open the CMP file just made :
	return OpenCRPFile(sCRPFile);
}

CCRPDoc::~CCRPDoc()
{
	if(m_fpCRPFile){
		fclose(m_fpCRPFile);
		m_fpCRPFile=NULL;
	}

	delete m_pStackSectionDocCurr;
	delete m_pStackSectionDocTest;
}


BEGIN_MESSAGE_MAP(CCRPDoc, CDocument)
	//{{AFX_MSG_MAP(CCRPDoc)
	ON_COMMAND(ID_CRP_PlusShotStatic, OnCRPPlusShotStatic)
	ON_COMMAND(ID_CRP_DumpGroupHeads, OnCRPDumpGroupHeads)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCRPDoc diagnostics

#ifdef _DEBUG
void CCRPDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CCRPDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CCRPDoc serialization

void CCRPDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CCRPDoc commands

BOOL CCRPDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;
	
	// TDO: Add your specialized creation code here
	return OpenCRPFile(lpszPathName );

}	  

BOOL CCRPDoc::OnSaveDocument(LPCTSTR lpszPathName) 
{
	// TODO: Add your specialized code here and/or call the base class		
	AfxMessageBox("Error: you should NOT save the CRP file here!");
	return true;
}

long CCRPDoc::SearchStation(long nRcvPh)
{
	CCRPIndexHead &head=m_CRPIndexDoc.m_indexHead ;

   	int nStart=0,nEnd=head.nCRPNumber -1,nMid;

	while(true){
		nMid=nStart+(nEnd-nStart)/2;
		
		//
		if(head.index [nMid].nRcvPh  ==nRcvPh)return nMid;
		if(nMid==nStart||nMid==nEnd){
			if(head.index [nEnd].nRcvPh ==nRcvPh)
				return nEnd;
			else if(head.index [nStart].nRcvPh  ==nRcvPh)
				return nStart;
			else
				return -1;
		}

		// 
		if(head.index [nMid].nRcvPh <nRcvPh)
			nStart=nMid;
		else
			nEnd=nMid;
		
	} 
}

bool CCRPDoc::OpenCRPFile(CString sCRPFile)
{
	// Open index :
	if(!m_CRPIndexDoc.OpenIndex (sCRPFile))return false;

	m_nMaxGroupNumber=0;
	for(long i=0;i<m_CRPIndexDoc.m_indexHead .nCRPNumber ;i++){
		if(m_CRPIndexDoc.m_indexHead .index [i].nGroupNumber>m_nMaxGroupNumber){
			m_nMaxGroupNumber=m_CRPIndexDoc.m_indexHead .index [i].nGroupNumber;
		}
	}
	
	// Open the CRP file:
	if(m_fpCRPFile){
		fclose(m_fpCRPFile);
		m_fpCRPFile=NULL;
	}
	m_fpCRPFile=fopen(sCRPFile,"rb+");

	return (m_fpCRPFile!=NULL);
}

CSegYGroup * CCRPDoc::GetGroup(long nGroup,bool bBalance)
{
	CCRPIndexHead &head=m_CRPIndexDoc.m_indexHead ;
	
	if(!m_fpCRPFile){
		AfxMessageBox("CCRPDoc Programming error: the CRP file handle not found!");
		return NULL;
	}

	if(nGroup<0||nGroup>=head.nGroupTotalNumber){
		AfxMessageBox("CCRPGroup::GetGroup error: group number is out of range :"+vtos(nGroup));
		return NULL;
	}

	long pos=sizeof(CSegYVolHead)+head.nGroupTotalPointNumber *sizeof(float)*nGroup;
	fseek(m_fpCRPFile,pos,SEEK_SET);
	UINT n=fread(&m_group,sizeof(float),head.nGroupTotalPointNumber ,m_fpCRPFile);
	if(n!=head.nGroupTotalPointNumber ){
		return NULL;
	}	

	if(bBalance){
		double dMax=0,dValue;
		for(long i=0;i<head.nGroupBodyPointNumber ;i++){
			dValue=fabs(m_group.data [i]);
			if(dValue>dMax)dMax=dValue;
		}

		for(i=0;i<head.nGroupBodyPointNumber ;i++){
			m_group.data [i]/=dMax;
		}
	}

	return &m_group;
}

CSegYGroup* CCRPDoc::GetGroup(long nCRPOrder, long nGroupOfCRP,bool bBalance)
{
	CCRPIndexHead &head=m_CRPIndexDoc.m_indexHead ;

	if(nCRPOrder<0||nCRPOrder>=head.nCRPNumber ){
		return NULL;
	}
	if(nGroupOfCRP<0||nGroupOfCRP>=head.index [nCRPOrder].nGroupNumber ){
		return NULL;
	}
	if(!m_fpCRPFile){		
		AfxMessageBox("CCRPDoc : file handle not found!");
		return NULL;
	}

	
	return GetGroup(head.index [nCRPOrder].pos +nGroupOfCRP,bBalance);
}

bool  CCRPDoc::SetSvSysDocPointer(CSvSysDoc *pSvSysDoc)
{
	if(pSvSysDoc==NULL)return false;
	m_pSvSysDoc=pSvSysDoc;

	return true;
}

bool CCRPDoc::SetCalcRange(long nStartPoint, long nEndPoint)
{
	if(nStartPoint<0||nEndPoint<0)return false;

	if(nStartPoint>nEndPoint){
		m_nStartPoint=nEndPoint;
		m_nEndPoint=nStartPoint;
	}
	else{
		m_nStartPoint=nStartPoint;
		m_nEndPoint=nEndPoint;
	}

	return true;

}


bool CCRPDoc::SetCMPRange(CRange *pRange, long nRangeNumber)
{
	if(nRangeNumber>VALID_CMP_RANGE_LIMIT||nRangeNumber<0){
		return false;
	}

	m_nValidCMPRangeNumber=nRangeNumber;
	for(long i=0;i<m_nValidCMPRangeNumber;i++){
		m_nValidCMPRange[i].start=pRange[i].start;
		m_nValidCMPRange[i].end=pRange[i].end;
	}

	return true;
}

bool CCRPDoc::CheckPowerInc(long nShotPh, int nOldStatic, int nNewStatic,bool bReplaceIfInc)
{																		
	if(!m_pStackSectionDocCurr||!m_pStackSectionDocTest){
		AfxMessageBox("Error: the stack section document has not been set!");
		return false;
	}

	long nShotPosInRel=m_pSvSysDoc->SearchShotStationInRel (nShotPh);

	// Loop for all of the shot stations on the rcv station:
	long i,j,nRcvPh,nCMPPh;
	CSegYGroup *pSegYGroup,*pSegYGroupTestSection,*pSegYGroupCurrSection;
	float *pDataWithStatic;
	long nGroupOrderInCRP;
	bool bAmongValidCMP;
	CCRPIndexHead &head=m_CRPIndexDoc.m_indexHead ;
	

	bool bFailed=false;
	for(i=0;i<m_pSvSysDoc->m_pShotRcvRel [nShotPosInRel].nRcvLine ;i+=1){
		for(nRcvPh=m_pSvSysDoc->m_pShotRcvRel [nShotPosInRel].RcvRange [i].start ;
			nRcvPh<=m_pSvSysDoc->m_pShotRcvRel [nShotPosInRel].RcvRange [i].end;
			nRcvPh+=m_pSvSysDoc->m_pShotRcvRel [nShotPosInRel].RcvRange [i].nInc ){

			// Check if the CMP point of the shot and reciever is among the valid CMP range:
			nCMPPh=(nRcvPh+nShotPh)/2;
			bAmongValidCMP=false;
			for(j=0;j<m_nValidCMPRangeNumber;j++){
				if(nCMPPh>=m_nValidCMPRange[j].start &&nCMPPh<=m_nValidCMPRange[j].end){
					bAmongValidCMP=true;
					break;
				}
			}
			if(!bAmongValidCMP)continue;

			// Calc the order of the rcv station among the CRP section:
			nGroupOrderInCRP=SearchRcvInIndex(nRcvPh);
			if(nGroupOrderInCRP==-1){
				AfxMessageBox("Error: can not find the station in the CRP file:"+vtos(nRcvPh));
				bFailed=true;
				break;
			}
			
			// Get the group:
			pSegYGroup=GetGroupByStation(nRcvPh,nShotPh);
			if(pSegYGroup==NULL){
				AfxMessageBox("Error: can not find the group in the CRP file, reciever:"+vtos(nRcvPh)+"shot station:"+vtos(nShotPh));
				bFailed=true;
				break;
			}
			
			// Change the pointer to CSegYGroup type:
			pSegYGroupTestSection=m_pStackSectionDocTest->GetGroup (nGroupOrderInCRP);
			pSegYGroupCurrSection=m_pStackSectionDocCurr->GetGroup (nGroupOrderInCRP);
  			if(!pSegYGroupTestSection||!pSegYGroupCurrSection){
				AfxMessageBox("Error: the memory has not been allocated in the stack section document!");
				break;
			}

			// firstly minus the group with the old static :
			pDataWithStatic=(float*)(&pSegYGroup->data[nOldStatic]);
			
			if(nOldStatic>0){
				for(j=0;j<head.nGroupBodyPointNumber-nOldStatic;j++){
					pSegYGroupTestSection->data [j]=pSegYGroupCurrSection->data[j]-pDataWithStatic[j];
				}
			}
			else{
				for(j=-nOldStatic;j<head.nGroupBodyPointNumber;j++){
					pSegYGroupTestSection->data [j]=pSegYGroupCurrSection->data[j]-pDataWithStatic[j];
				}
			}


			// then plus the group with the new static:
			pDataWithStatic=(float*)(&pSegYGroup->data [nNewStatic]);

			if(nNewStatic>0){
				for(j=0;j<head.nGroupBodyPointNumber-nNewStatic;j++){
					pSegYGroupTestSection->data [j]+=pDataWithStatic[j];
				}
			}
			else{
				for(j=-nNewStatic;j<head.nGroupBodyPointNumber;j++){
					pSegYGroupTestSection->data [j]+=pDataWithStatic[j];
				}
			}
		}
		if(bFailed)break;
	}

	// Calc the new power:
	double dPowerTest=0.0;
	for(i=0;i<head.nCRPNumber;i++){
		pSegYGroup=m_pStackSectionDocTest->GetGroup (i);
		for(j=m_nStartPoint;j<m_nEndPoint;j++){
			dPowerTest+=pSegYGroup->data[j]*pSegYGroup->data[j];
		}
	}
	dPowerTest/=head.nCRPNumber*(m_nEndPoint-m_nStartPoint);

	// Check if the power increased:
	bool bInc=false;
	if(dPowerTest>m_dPowerCurr){
		bInc=true;		
	}

	// If the power increased, Replace the current section with the new test section:
	if(bInc&&bReplaceIfInc){
		long nGroupSize=sizeof(float)*head.nGroupTotalPointNumber;
		for(i=0;i<head.nCRPNumber;i+=1){
			memcpy(m_pStackSectionDocCurr->GetGroup(i),m_pStackSectionDocTest->GetGroup(i),nGroupSize);
		}
		m_dPowerCurr=dPowerTest;
	}
	
	// or else replase the test data with the current data for later copying to current data:
	if(!bInc){
		long nGroupSize=sizeof(float)*head.nGroupTotalPointNumber;
		for(i=0;i<head.nCRPNumber;i+=1){
			memcpy(m_pStackSectionDocTest->GetGroup(i),m_pStackSectionDocCurr->GetGroup (i),nGroupSize);
		}
	}

	// return:
	return bInc;


}

CSegYGroup* CCRPDoc::GetGroupByStation(long nRcvPh, long nShotPh)
{
	long nOrderInIndex=SearchStation(nRcvPh);
	if(nOrderInIndex==-1)return NULL;
	
	long nOrderInCRP=SearchShotStationInCRP(nOrderInIndex,nShotPh);
	return GetGroup(nOrderInIndex, nOrderInCRP);
}

long CCRPDoc::SearchShotStationInCRP(long nCRPOrder, long nShotPh)
{ 	
	CCRPIndexHead head=m_CRPIndexDoc.m_indexHead ;
	if(nCRPOrder<0||nCRPOrder>=head.nCRPNumber )return -1;

	CCRPGroupMsg *pCRPMsg =m_CRPIndexDoc.GetCRPGroupMsg (nCRPOrder);
	if(!pCRPMsg)return -1;

	int nStart=0,nEnd=head.index [nCRPOrder].nGroupNumber ,nMid;

	while(true){
		nMid=nStart+(nEnd-nStart)/2;
		
		//
		if(pCRPMsg[nMid].nShotPh ==nShotPh)return nMid;
		if(nMid==nStart||nMid==nEnd){
			if(pCRPMsg[nEnd].nShotPh ==nShotPh)
				return nEnd;
			if(pCRPMsg[nStart].nShotPh ==nShotPh)
				return nStart;
			else
				return -1;
		}

		// 
		if(pCRPMsg[nMid].nShotPh <nShotPh)
			nStart=nMid;
		else
			nEnd=nMid;		
	} 
}

////////////////////////////////////////
// Plus the CRP groups:
bool CCRPDoc::ReStack(bool bTest)
{
	if(!m_pSvSysDoc){
		AfxMessageBox("CCRPDoc::ReStack() Error: the survey system document pointwer has not been set in the CRP document!");
		return false;
	}
	if(!m_pStackSectionDocTest){
		AfxMessageBox("Error: the stack section document in the CRPDoc has not been set!");
		return false;
	}

	CCRPIndexHead &head=m_CRPIndexDoc.m_indexHead ;
	CCRPGroupMsg *pCRPGroupMsg;

	CProgressDlg dlg;
	dlg.Create();
	dlg.SetWindowText("Stacking...");
	dlg.SetStatus("Station number has been dealed with:");
	dlg.SetRange (0,head.nCRPNumber );


	long i,j,k,n,nPoint,st;
	long nShotNumber; 
	CSegYGroup *pGroup=NULL;
	m_dPowerCurr=0.0;
	long nRcvPh,nShotPh,nCMPPh;
	bool bAmongValidCMP;

	bool bFailed=false;
	for(i=0;i<head.nCRPNumber;i++){
		dlg.SetPos(i);

		pCRPGroupMsg=m_CRPIndexDoc.GetCRPGroupMsg (i);
		if(!pCRPGroupMsg){
			AfxMessageBox("Error: can not get the information of the CRP:"+vtos(i));
			bFailed=true;
			break;
		}
		nRcvPh=head.index [i].nRcvPh ;
		nShotNumber=head.index [i].nGroupNumber ;

		memset(&m_group.data [0],0,sizeof(float)*head.nGroupTotalPointNumber );
		
		for(j=0;j<nShotNumber ;j++){

			// Check if the CMP is between the valid CMPs:				
			nShotPh=pCRPGroupMsg[j].nShotPh ;
			nCMPPh=(nRcvPh+nShotPh)/2;

			bAmongValidCMP=false;
			for(k=0;k<m_nValidCMPRangeNumber;k++){
				if(nCMPPh>=m_nValidCMPRange[k].start &&nCMPPh<=m_nValidCMPRange[k].end){
					bAmongValidCMP=true;
					break;
				}
			}
			if(!bAmongValidCMP)continue;

			// Plus the group:
			n=m_pSvSysDoc->SearchShotStation (nShotPh);
			if(n==-1){
				AfxMessageBox("Error: can not find shot station:"+vtos(nShotPh));
				bFailed=true;
				break;
			}
			st=m_pSvSysDoc->m_pShotPhyPar [n].st ;
			
			pGroup=GetGroup(i,j);
			if(!pGroup){
				AfxMessageBox("Error: can not read the group:"+vtos(j)+" of CRP:"+vtos(i));
				bFailed=true;
				break;
			}

			if(st>0){
				for(nPoint=0;nPoint<head.nGroupBodyPointNumber -st;nPoint++){
					m_group.data [nPoint]+=pGroup->data [nPoint+st];
				}
			}
			else {
				for(nPoint=-st;nPoint<head.nGroupBodyPointNumber;nPoint++){
					m_group.data [nPoint]+=pGroup->data [nPoint+st];
				}
			}
		}

		if(bFailed)break;

		// Calc the power:
		for(nPoint=m_nStartPoint;nPoint<m_nEndPoint;nPoint++){
			m_dPowerCurr+=m_group.data [nPoint]*m_group.data [nPoint];				
		}

		
		m_pStackSectionDocCurr ->SetGroup(i,&m_group);
		if(!bTest)m_pStackSectionDocTest ->SetGroup(i,&m_group);

	}

	m_dPowerCurr/=head.nCRPNumber *(m_nEndPoint-m_nStartPoint);

	dlg.DestroyWindow ();

	return (!bFailed);
}

long CCRPDoc::SearchRcvInIndex(long nRcvStation)
{
	CCRPIndexHead &head=m_CRPIndexDoc.m_indexHead ;
	int nStart=0,nEnd=head.nCRPNumber -1,nMid;

	while(true){
		nMid=nStart+(nEnd-nStart)/2;
		
		//
		if(head.index [nMid].nRcvPh  ==nRcvStation)return nMid;
		if(nMid==nStart||nMid==nEnd){
			if(head.index [nEnd].nRcvPh  ==nRcvStation)
				return nEnd;
			else if(head.index [nStart].nRcvPh  ==nRcvStation)
				return nStart;
			else
				return -1;
		}

		// 
		if(head.index [nMid].nRcvPh <nRcvStation)
			nStart=nMid;
		else
			nEnd=nMid;
		
	}
}
bool CCRPDoc::AllocateMem()
{
	CCRPIndexHead &head=m_CRPIndexDoc.m_indexHead ;

	m_pStackSectionDocCurr->SetParameter (
		head.nCRPNumber,
		head.nGroupBodyPointNumber,
		head.nSampleInterval);

	m_pStackSectionDocTest->SetParameter (
		head.nCRPNumber,
		head.nGroupBodyPointNumber,
		head.nSampleInterval);

	return ReStack();
}

double CCRPDoc::GetCurrentPower()
{
	return m_dPowerCurr;
}

void CCRPDoc::OnCRPPlusShotStatic() 
{
	// TODO: Add your command handler code here
	if(!m_fpCRPFile)return ;

	// Open a static document:
	CFileDialog dlg(true);
	dlg.m_ofn.lpstrFilter="Static Data file(*.T)\0*.t";
	dlg.m_ofn.lpstrTitle="Open ";
	dlg.m_ofn.lpstrDefExt=".T";
	
	if(dlg.DoModal()==IDCANCEL)return ;
	CString sStatic=dlg.GetPathName();

	CFdataApp *pApp=(CFdataApp*)AfxGetApp();
	CStaticDoc *pStaticDoc=(CStaticDoc *)(pApp->m_pStaticDataTemplate ->OpenDocumentFile (sStatic,false));

	if(!pStaticDoc)return;

	//
	CCRPIndexHead &head=m_CRPIndexDoc.m_indexHead ;
	CCRPGroupMsg *pCRPGroupMsg=NULL;

	CProgressDlg dlgPro;
	dlgPro.Create();
	dlgPro.SetRange(0,head.nCRPNumber);
	dlgPro.SetStatus("Adding static value to the CRP data file group heads:");

	long i,j;
	long nGroup=0,nPh,n;
	CSegYGroup *pSegYGroup=NULL;
	
	bool bFailed=false;
	for(i=0;i<head.nCRPNumber;i++){
		dlgPro.SetPos(i);
		pCRPGroupMsg=m_CRPIndexDoc.GetCRPGroupMsg (i);
		if(!pCRPGroupMsg){
			AfxMessageBox("Error: can not get the CRP information of:"+vtos(i));
			bFailed=true;
			break;
		}

		for(j=0;j<head.index [i].nGroupNumber ;j++){
			pSegYGroup=GetGroup(i,j,false);
			
			nPh=pSegYGroup->nShotStation ;
			n=pStaticDoc->SearchStation (nPh);
			if(n==-1){
				AfxMessageBox("CCRPDoc::OnCRPPlusShotStatic() error: can not find shot station in the static document:"+vtos(nPh));
				bFailed=true;
				break;
			}
			else{
				pSegYGroup->nShotStatic =pStaticDoc->m_Data [n].st ;
			}

			nPh=pSegYGroup->nRcvStation ;
			n=pStaticDoc->SearchStation (nPh);
			if(n==-1){
				AfxMessageBox("CCRPDoc::OnCRPPlusStatic() error: can not find reciever station in the static document:"+vtos(nPh));
				bFailed=true;
				break;
			}
			else{
				pSegYGroup->nRcvStatic =pStaticDoc->m_Data [n].rt ;
			}

			//
			if(!SaveGroup(i,j,pSegYGroup)){
				AfxMessageBox("Error: can not save the group into the CRP file, program will break!");
				bFailed=true;
				break;
			}


		}
		if(bFailed)break;
	}

	pStaticDoc->OnCloseDocument ();
	dlgPro.DestroyWindow ();

	if(!bFailed)UpdateAllViews(NULL);

	return;
	
}

bool CCRPDoc::SaveGroup(long nCRPOrder,long nGroupOrderInCRP,CSegYGroup *pSegYGroup)
{
	CCRPIndexHead &head=m_CRPIndexDoc.m_indexHead ;

	if(!m_fpCRPFile||nCRPOrder<0||nCRPOrder>=head.nCRPNumber)return false;
	if(nGroupOrderInCRP<0||nGroupOrderInCRP>=head.index [nCRPOrder].nGroupNumber )return false;
	
	long nPos=sizeof(CSegYVolHead)+head.nGroupTotalPointNumber*sizeof(float)*(head.index [nCRPOrder].pos +nGroupOrderInCRP) ;
	
	fseek(m_fpCRPFile,nPos,SEEK_SET);
	fwrite(pSegYGroup,sizeof(float),head.nGroupTotalPointNumber,m_fpCRPFile);

	return true;

}

bool CCRPDoc::CreateCRPFile(CString sCRPFile)
{
	// Open a static document:
	if(sCRPFile==""){
		CFileDialog dlg(false);
		dlg.m_ofn.lpstrFilter="CRP Data file(*.CRP)\0*.CRP";
		dlg.m_ofn.lpstrTitle="Save";
		dlg.m_ofn.lpstrDefExt=".CRP";
		
		if(dlg.DoModal()==IDCANCEL)return false;
		sCRPFile=dlg.GetPathName();
	}

	if(m_fpCRPFile){
		fclose(m_fpCRPFile);
		m_fpCRPFile=NULL;
	}
	m_fpCRPFile=fopen(sCRPFile,"wb");

	return (m_fpCRPFile!=NULL);

}

bool CCRPDoc::ConstructionOver()
{
	if(m_fpCRPFile){
		fclose(m_fpCRPFile);
		m_fpCRPFile=NULL;
		return true;
	}
	else{
		return false;
	}
}

bool CCRPDoc::SaveVolHead(CSegYVolHead head)
{
	if(!m_fpCRPFile)return false;

	fseek(m_fpCRPFile,0,SEEK_SET);
	int n=fwrite(&head,sizeof(CSegYVolHead),1,m_fpCRPFile);

	return (n==1);
}

bool CCRPDoc::SaveGroup(CSegYGroup *pSegYGroup, long nPos)
{
	if(pSegYGroup->nGroupSamplePoint ==0||!m_fpCRPFile)return false;

	long nGroupSize=sizeof(float)*pSegYGroup->nGroupSamplePoint +m_nSegYGroupHeadSize;
	fseek(m_fpCRPFile,sizeof(CSegYVolHead)+nGroupSize*nPos,SEEK_SET);

	fwrite(pSegYGroup,1,nGroupSize,m_fpCRPFile);

	return true;
}

bool CCRPDoc::SaveGroup(CSvSysDoc *pSvSysDoc,CSegYGroup *pSegYGroup, long nCRP,long nGroupOrderInCRP)
{
	long nPos=pSvSysDoc->m_pRcvShotRel [nCRP].nPos +nGroupOrderInCRP;
	return SaveGroup(pSegYGroup,nPos);
}


bool CCRPDoc::MakeCRPFile(CSeisDoc *pSeisDoc, CSvSysDoc *pSvSysDoc,CString &sCRPFile)
{
	///////////////////////////////////////////////////////////
	//	 Open various documents needed firstly;
	bool bSuccess=false;
	while(true){ // Only one loop, using only ite "break" function:
	
		CFdataApp *pApp=(CFdataApp*)AfxGetApp();	
		
		/////////////////////////////////////////////////
		// Open the seismic data file :
		if(!pSeisDoc){
			CFileDialog dlgSeis(true);
			dlgSeis.m_ofn.lpstrFilter="Seismic Data File(*.dat)\0*.dat";
			dlgSeis.m_ofn.lpstrTitle="Open";
			dlgSeis.m_ofn.lpstrDefExt=".dat";
			
			if(dlgSeis.DoModal()==IDCANCEL)break;
			CString sSeisFile=dlgSeis.GetPathName();
								  
			pSeisDoc=(CSeisDoc*)(pApp->m_pSeisViewTemplate ->OpenDocumentFile(sSeisFile,false));
			if(!pSeisDoc){
				AfxMessageBox("Error: when opening the survey system:"+sSeisFile);
				break;	
			}
		}


		/////////////////////////////////////////////////
		// Open the svsys document:
		if(!pSvSysDoc){
			CFileDialog dlgSvSys(true);
			dlgSvSys.m_ofn.lpstrFilter="Survey system file(*.sys)\0*.sys";
			dlgSvSys.m_ofn.lpstrTitle="Open Survey System File";
			dlgSvSys.m_ofn.lpstrDefExt=".sys";
			
			if(dlgSvSys.DoModal()==IDCANCEL)break;
			CString sSvSysFile=dlgSvSys.GetPathName();
								  
			pSvSysDoc=(CSvSysDoc*)(pApp->m_pSvSysTemplate->OpenDocumentFile(sSvSysFile,false));
			if(!pSvSysDoc){
				AfxMessageBox("Error: when opening the survey system:"+sSvSysFile);
				break;	
			}
		}

		//////////////////////////////////////////////////////////////////
		// We must save the CRP document here, because it is too large
		// We can  not save them in the memory:
		CFileDialog dlgSave(false);
		dlgSave.m_ofn.lpstrFilter="Seismic CRP File (*.crp)\0*.crp";
		dlgSave.m_ofn.lpstrTitle="Save the CRP File";
		dlgSave.m_ofn.lpstrDefExt=".crp";
		if(dlgSave.DoModal()==IDCANCEL)break;
		sCRPFile=dlgSave.GetPathName();

		bSuccess=true;
		break;
	} // while:  only one loop;

	//////////////////////////////////////////////////////
	// Has tried to open all of the files:
	if(!bSuccess){
		if(pSvSysDoc)pSvSysDoc->OnCloseDocument ();
		return false;
	}

	//////////////////////////////////////////////////////////
	// Make a CMP file  by the current documents:
	CSegYGroup *pSegYGroup=NULL;	
	while(true){  // only for break from it to the out side;

		bSuccess=true;

		if(!CreateCRPFile(sCRPFile)){
			AfxMessageBox("Error : Can not create the CRP file "+sCRPFile);
			bSuccess=false;
			break;
		}

		CSegYVolHead head;
		if(!pSeisDoc->GetVolHead(&head)){
			bSuccess=false;
			break;
		}

		if(!SaveVolHead(head)){
			bSuccess=false;
			break;
		}
		///////////////////////////////////////////////////////////
		//	 Make the CRP file and the index file:
		CProgressDlg dlgPro;
		dlgPro.Create();
		dlgPro.SetRange(0,pSvSysDoc->m_nRcvPhyNumber);
		dlgPro.SetStatus("Making CRP file from seismic data:");
		
		long nShotPh;
		long nShotOrder;
		long nShotFileNumber;
		long nShotPos;	
		long nGroup;
		
		bSuccess=true;
		long i,j;

		for(i=0;i<pSvSysDoc->m_nRcvPhyNumber ;i++){
			dlgPro.SetPos(i);

			for(j=0;j<pSvSysDoc->m_pRcvShotRel [i].nShotNumber ;j++){
				nShotPh=pSvSysDoc->m_pRcvShotRel [i].pGroup [j].nPhShot ;
				nShotOrder=pSvSysDoc->SearchShotStationInRel (nShotPh);
				if(nShotOrder==-1){
					bSuccess=false;
					break;
				}

				nShotFileNumber=pSvSysDoc->m_pShotRcvRel [nShotOrder].FileNumber ;
				nShotPos=pSeisDoc->SearchFileNumber (nShotFileNumber);				
				if(nShotPos==-1){
					AfxMessageBox("Error: can not find the shot file number:"+vtos(nShotFileNumber)+", in the seismic file!");
					bSuccess=false;
					break;
				}
				
				nGroup=pSvSysDoc->m_pRcvShotRel [i].pGroup [j].nOrderGroupInShot;				
				pSegYGroup=pSeisDoc->GetGroup (nShotPos,nGroup);
				if(!pSegYGroup){
 					AfxMessageBox("Error: Can not get the group of shot "+vtos(nShotPos));
					bSuccess=false;
					break;
				}

				pSegYGroup->nShotStation =pSvSysDoc->m_pRcvShotRel [i].pGroup [j].nPhShot ;
				pSegYGroup->nRcvStation =pSvSysDoc->m_pRcvShotRel [i].PhRcv ;

				///////////////////////////////////////////////////////
				// Save the group into the CRP file:
				if(!SaveGroup(pSvSysDoc,pSegYGroup,i,j)){
					AfxMessageBox("Error: can not save the group:"+vtos(j)+" of CRP:"+vtos(i));
					bSuccess=false;
					break;
				}
			}
			if(!bSuccess)break;
		}
		
		dlgPro.DestroyWindow ();

		if(!ConstructionOver()){
			bSuccess=false;
		}
		break;
	
	}  // use "while" 's break function only;

	if(pSvSysDoc)pSvSysDoc->OnCloseDocument ();
	if(pSeisDoc)pSeisDoc->OnCloseDocument ();
	
	return bSuccess;

}

void CCRPDoc::OnCRPDumpGroupHeads() 
{
	// TODO: Add your command handler code here
	CFileDialog dlg(true);
	dlg.m_ofn.lpstrFilter="Text file to store group heads(*.txt)\0*.txt";
	dlg.m_ofn.lpstrTitle="Open ";
	dlg.m_ofn.lpstrDefExt=".txt";
	
	if(dlg.DoModal()==IDCANCEL)return ;
	CString sFile=dlg.GetPathName();

	FILE *fp=fopen(sFile,"wt");
	if(!fp){
		AfxMessageBox("Error: can not create :"+sFile);
		return;
	}
	
	//
	CCRPIndexHead &head=m_CRPIndexDoc.m_indexHead ;
	
	CProgressDlg dlgPro;
	dlgPro.Create ();
	dlgPro.SetRange(0,head.nCRPNumber );
	dlgPro.SetStatus ("Dumping CRP heads:");

	//
	CSegYGroup *pSegYGroup=NULL;
	long i,j,nGroupNumber;
	bool bFailed=false;
	for(i=0;i<head.nCRPNumber ;i++){

		dlgPro.SetPos (i);
		nGroupNumber=head.index [i].nGroupNumber ;

		fprintf(fp,"CRP Order:%ld\n",i);
		for(j=0;j<nGroupNumber;j++){
			pSegYGroup=GetGroup(i,j,false);
			if(!pSegYGroup){
				AfxMessageBox("Error: can not get the group:"+vtos(j)+" of CRP:"+vtos(i));
				bFailed=true;
				break;
			}

			fprintf(fp,"Order:%ld,Shot Station:%ld,Shot North:%ld,Shot East :%ld,Rcv North:%ld,Rcv East:%ld\n",
				j,
				pSegYGroup->nShotStation ,				
				pSegYGroup->nShotNorth ,
				pSegYGroup->nShotEast ,
				pSegYGroup->nRcvStation,				
				pSegYGroup->nRcvNorth,
				pSegYGroup->nRcvEast);
		}
		if(bFailed)break;
	}

	dlgPro.DestroyWindow ();

	fclose(fp);

	if(!bFailed){
		AfxMessageBox("OK! the CRP heads have been dump to :"+sFile);
	}

	return;	
} 

long CCRPDoc::GetGroupNumberInCRP(long nCRPOrder)
{
	CCRPIndexHead &head=m_CRPIndexDoc.m_indexHead ;

	if(nCRPOrder<0||nCRPOrder>=head.nCRPNumber ){
		return 0;
	}

	return head.index [nCRPOrder].nGroupNumber ;
}

long CCRPDoc::GetCRPNumber()
{
	CCRPIndexHead &head=m_CRPIndexDoc.m_indexHead ;
	return 	head.nCRPNumber;
}
