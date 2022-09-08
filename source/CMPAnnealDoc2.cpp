// CMPDoc.cpp : implementation file
//

#include "stdafx.h"
#include "fdata.h"
#include "ProgDlg.h"
#include "SvSysDoc.h"
#include "SeisDoc.h"
#include "CMPAnnealDoc.h"
#include "MyPoint.h"
#include "segy.h"
#include "DlgSeisCutTime.h"
#include "ProgDlg.h"
#include "CMPCutTime.h"
#include "FHZoomViewGlobal.h"
#include "DlgSeisToCMP.h"
#include "MyGroupHead.h"
#include "templates.h"
#include "MemEqua.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



	   
void CCMPAnnealDoc::OnCMPDumpGroupHead() 
{
	// TODO: Add your command handler code here
	CFileDialog dlg(false);
	dlg.m_ofn.lpstrFilter="Text file to save the group heads(*.txt)\0*.txt";
	dlg.m_ofn.lpstrTitle="Save";
	dlg.m_ofn.lpstrDefExt=".txt";

	if(dlg.DoModal()==IDCANCEL)return ;
	CString sFile=dlg.GetPathName();

	FILE *fp=fopen(sFile,"wt");
	if(!fp){
		AfxMessageBox("Error: can not create :"+sFile);
		return;
	}
	
	//
	CCMPIndexHead &head=m_CMPIndexFile.m_head ;
	CProgressDlg dlgPro;
	dlgPro.Create ();
	dlgPro.SetRange(0,head.nCMPNumber );
	dlgPro.SetStatus ("Dumping CMP heads:");

	//
	CSegYGroup *pGrp=NULL;
	long i,j,nGroupNumber;
	bool bFailed=false;
	for(i=0;i<head.nCMPNumber ;i++){

		dlgPro.SetPos (i);
		nGroupNumber=head.index [i].nGroupNumber ;

		fprintf(fp,"CMP Order:%ld\n",i);
		fprintf(fp,"No.= CDP = S.N. = S.E. = R.N. = R.E. = CMP N. = CMP E. = S. File Num. = Grp In S. =\n");
		for(j=0;j<nGroupNumber;j++){
			pGrp=GetGroup(i,j,false);
			if(!pGrp){
				AfxMessageBox("Error: can not get the group:"+vtos(j)+" of CMP:"+vtos(i));
				bFailed=true;
				break;
			}

			fprintf(fp,"%ld %d %lf %10.0lf %10.0lf %10.0lf %10.0lf %10.0lf %10.0lf\n",
				j,
				
				pGrp->nCMPOrder,
				pGrp->nShotNorth,
				pGrp->nShotEast,

				pGrp->nRcvNorth ,
				pGrp->nRcvEast ,

				(pGrp->nShotNorth+pGrp->nRcvNorth)/2.0,
				(pGrp->nShotEast +pGrp->nRcvEast)/2.0,

				pGrp->nShotFileNumber,
				pGrp->nGroupOrder);
		}
		if(bFailed)break;
	}

	dlgPro.DestroyWindow ();

	fclose(fp);

	if(!bFailed){
		AfxMessageBox("OK! the CMP heads have been dump to :"+sFile);
	}

	return;
	
}

bool CCMPAnnealDoc::MakeStackSection(bool bWithStatic)
{
	//////////////////////////////////////////
	// create an empty stack file:
	CFdataApp *pApp=(CFdataApp*)AfxGetApp();
	CStackSectionDoc *pStackDoc=(CStackSectionDoc *)pApp->m_pStackSectionTemplate ->OpenDocumentFile(NULL,true);
	if(!pStackDoc){
		AfxMessageBox("Can not create the Empty stack file !");
		return false;
	}

	bool bFailed=false;
	CStaticDoc *pStaticDoc=NULL;
	if(!bWithStatic){
		if(!DoStack (&pStackDoc->m_Section ,NULL,NULL))bFailed=true;
	}
	else {
		DoStaticStack(&pStackDoc->m_Section ,NULL,true);
	}


	if(!bFailed){
		pStackDoc->CalcSectionPower ();
		pStackDoc->SetModifiedFlag (true);
		pStackDoc->UpdateAllViews(NULL);
	}
	else{
		pStackDoc->OnCloseDocument ();
	}

	return !bFailed;

}

void CCMPAnnealDoc::OnCMPStackNOStatic() 
{
	MakeStackSection(false);
}

void CCMPAnnealDoc::OnCMPStackWithStatic() 
{
	MakeStackSection(true);
	
}

void CCMPAnnealDoc::OnCMPCutCMPSection() 
{
	//////////////////////////////////////////
	//
	CCMPIndexHead &head=m_CMPIndexFile.m_head;

	CDlgCMPCutTime dlg;
	dlg.m_nStartCMP =0;
	dlg.m_nEndCMP =m_nCMPNumber-1;
	dlg.m_nStartTime =0;
	dlg.m_nEndTime = head.nGroupMaxPointNumber*head.nSampleInterval;
	do{
		if(dlg.DoModal ()==IDCANCEL)return;
	}while(dlg.m_sFileOutPut=="");

	if(DrawOutPartCMP(dlg.m_sFileOutPut,
		dlg.m_nStartCMP ,
		dlg.m_nEndCMP ,
		dlg.m_nStartTime ,
		dlg.m_nEndTime ))
	{

		CFdataApp *pApp=(CFdataApp *)AfxGetApp();
		pApp->m_pCMPDataTemplate ->OpenDocumentFile (dlg.m_sFileOutPut);
	}


	return ; //!bFailed;	
}

bool CCMPAnnealDoc::DoStaticStack(CSegYMultiGroup *pSection,CStaticDoc *pStaticDoc,bool bStaticOri,bool bShowProgress)
{
	
	CCMPIndexHead &head=m_CMPIndexFile.m_head ;
	bool bOpenedHere=false;
	if(!pStaticDoc){
		CString sStatic=CStaticDoc::GetFileForOpen ();
		if(sStatic==""){
			return false;
		}
		
		pStaticDoc=new CStaticDoc ;
		if(!pStaticDoc->OnOpenDocument (sStatic)){
			return false;
		}
		bOpenedHere=true;
	}

	if(bStaticOri){
		for(long i=0;i<pStaticDoc->m_nRecordNumber;i++){
			pStaticDoc->m_Data [i].st =-pStaticDoc->m_Data [i].st /head.nSampleInterval ;
			pStaticDoc->m_Data [i].rt =-pStaticDoc->m_Data [i].rt /head.nSampleInterval ;
		}
	}

	bool bFailed=false;
	if(!DoStack (pSection,pStaticDoc,NULL,bShowProgress)){
		bFailed=true;
	}
	
	if(bOpenedHere){
		delete pStaticDoc;
	}
	return !bFailed;

}

bool CCMPAnnealDoc::CalcCorelationAndMakeEquation(CString sFileA, CString sFileB)
{

	if(m_nCMPNumber==0)return false;

	///////////////////////////////////
	// Open the survey system document:
	if(!m_pSvSysDoc){
		CString sSvSys=CSvSysDoc::GetFileForOpen();
		if(sSvSys=="")return false;

		CFdataApp *pApp=(CFdataApp *)AfxGetApp();
		m_pSvSysDoc=(CSvSysDoc*)(pApp->m_pSvSysTemplate ->OpenDocumentFile (sSvSys,false));

		if(!m_pSvSysDoc)return false;
	}

	long nShotPhyNumber=m_pSvSysDoc->m_nShotPhyNumber;
	long nRcvPhyNumber=m_pSvSysDoc->m_nRcvPhyNumber;
	long nTotalPhyNumber=nShotPhyNumber+nRcvPhyNumber;
		
	///////////////////////////////////////
	// The Equation:
	CMemEquation equation;
	if(!equation.Construct (sFileA,sFileB,nTotalPhyNumber))return false;

	long nCorrelation[3];
	long i,j,nGroupNumber;

	//////////////////////////////////////////
	// Allocate the Block Memory:
	CBlockTrace block[4];
	
	//////////////////////////////////////////
	// Make the equation:
	CSegYGroup group[2];

	double dShotStation[4],dRcvStation[4];
	long nShotOrder[4],nRcvOrder[4];
	long nShotOrderInAll[4],nRcvOrderInAll[4];

	CProgressDlg dlg;
	dlg.Create ();
	dlg.SetRange (0,m_nCMPNumber);

	bool bFailed=false;
	for(i=0;i<m_nCMPNumber;i++){
		dlg.SetPos (i);
		nGroupNumber=GetGroupNumberInCMP(i);
		if(nGroupNumber==0)continue;

		// Get the other groups in the CMP and do correlation
		for(j=0;j<nGroupNumber-1;j++){
			
			// The first correlation:
			if(!GetGroup(i,j,false,&group[0]))continue;
			if(!GetGroup(i,j+1,false,&group[1]))continue;
			
			// Get the station order:
			for(k=0;k<2;k++){
				
				// Rcv Order:
				dRcvStation[k]=group[k].nRcvStation;				
				nRcvOrder[k]=m_pSvSysDoc->SearchRcvStation(dRcvStation[k]);
				if(nRcvOrder[k]==-1){
					AfxMessageBox("Error: can not find shot station in survey system:"+vtos(dRcvStation[k]));
					bFailed=true;
					break;
				}

				nRcvOrderInAll[k]=nRcvOrder[k];
				equation.SetTag (nRcvOrderInAll[k],dRcvStation[k]);

				// Shot Order:
				dShotStation[k]=group[k].nShotStation;				
				nShotOrder[k]=m_pSvSysDoc->SearchShotStation(dShotStation[k]);
				if(nShotOrder[k]==-1){
					AfxMessageBox("Error: can not find shot station in survey system:"+vtos(dShotStation[k]));
					bFailed=true;
					break;
				}

				nShotOrderInAll[k]=nShotOrder[k]+nRcvPhyNumber;
				equation.SetTag (nShotOrderInAll[k],dShotStation[k]);				
			}
			
			if(bFailed)break;

			// Do correlation:
			nCorrelation[0]=DoCorrelation(&group[1],&group[0],100);			
			
			// Set the block:
			block[0].m_nBlock=nShotOrderInAll[1];
			block[0].m_dLen =1;
			block[1].m_nBlock=nRcvOrderInAll[1];
			block[1].m_dLen =1;
			block[2].m_nBlock=nShotOrderInAll[0];
			block[2].m_dLen =-1;
			block[3].m_nBlock=nRcvOrderInAll[0];
			block[3].m_dLen =-1;
		
			// Appends the equation:
			if(!equation.AppeEqua (block,4,-nCorrelation[0]*group[0].nGroupSampleInterval)){
				CString sError="Error: can not append equation: CMP "+vtos(i)+" , group "+vtos(j)+",Continue?";
				if(AfxMessageBox(sError,MB_YESNO)==IDNO){
					break;
				}
				continue;
			}
		}
		if(bFailed)break;
	}

	equation.Close ();	
	dlg.DestroyWindow ();

	return !bFailed;	
}

long CCMPAnnealDoc::DoCorrelation(CSegYGroup *pGroupLast, CSegYGroup *pGroup, long nRange)
{	
	if(nRange>pGroupLast->nGroupSamplePoint /2)return 0;
	
	/////////////////////////////////////////////
	//
	nRange=70;

	long i,j,n,nValidPoint;
	long nHalfRange=nRange/2;
	double dMaxValue=0,dTemp;
	long nMaxPeak=0;
	long nStartPoint=200/4;
	long nEndPoint=900/4;

	double dPeakValue[200];
	long nPeakValid[200];


	bool b=false;
	
	if(pGroupLast->nShotStation ==11800&&
		pGroupLast->nRcvStation ==12600&&
		pGroup->nShotStation==11700&&
		pGroup->nRcvStation==12700){
		b=true;

		FILE *fp=fopen("d:\\1.dat","wb");
		fwrite(pGroupLast,sizeof(CSegYGroup),1,fp);
		fwrite(pGroup,sizeof(CSegYGroup),1,fp);
		fclose(fp);
	}

	CSegYGroup gPoint;
	CSegYGroup gValue1,gValue2;
	memset(&gValue1,0,sizeof(CSegYGroup));
	memset(&gValue2,0,sizeof(CSegYGroup));

	gValue1.nGroupSamplePoint = pGroup->nGroupSamplePoint ;
	gValue2.nGroupSamplePoint = pGroup->nGroupSamplePoint ;

	gPoint.nGroupSamplePoint = pGroup->nGroupSamplePoint ;


	for(i=-nHalfRange;i<nHalfRange;i++){
		dTemp=0.0;
		nValidPoint=0;
		for(j=nStartPoint; j<nEndPoint; j++){
			n=j+i;
			if(n<0||n>=pGroup->nGroupSamplePoint )continue;
			dTemp+=pGroupLast->data [j]*pGroup->data [j+i];
			nValidPoint++;

			if(i==28){
				gValue1.data [j]=pGroupLast->data [j]*pGroup->data [j+i];;
			}
			if(i==4){
				gValue2.data [j]=pGroupLast->data [j]*pGroup->data [j+i];;
			}


		}

		dTemp/=nValidPoint;
		if(dTemp>dMaxValue){
			dMaxValue=dTemp;
			nMaxPeak=i;
		}

		nPeakValid[i+nHalfRange]=nValidPoint;
		dPeakValue[i+nHalfRange]=dTemp;
	}

	if(b){
		FILE *fp=NULL;
		fp=fopen("d:\\co.dat","wb");
		fwrite(&gValue1,sizeof(CSegYGroup),1,fp);
		fwrite(&gValue2,sizeof(CSegYGroup),1,fp);
		fclose(fp);
	}

	int nStatic=(pGroup->nShotStatic +pGroup->nRcvStatic 
		-pGroupLast->nShotStatic -pGroupLast->nRcvStatic)/4 ;
	
	if(fabs((double)(nMaxPeak+nStatic))>2){
		int mm=0;
	}
	return nMaxPeak;


}

void CCMPAnnealDoc::OnCMPCorrelation() 
{	
	// TODO: Add your command handler code here
	CString sFileA="d:\\a.dat";
	CString sFileB="d:\\b.dat";

	if(!CalcCorelationAndMakeEquation(sFileA,sFileB))return;
	CMemEquation equation;
	if(!equation.Use (sFileA,sFileB))return;
	if(!equation.Resolve ())return;	

	CFileDialog dlg(false);
	dlg.m_ofn.lpstrFilter="text file to save result(*.txt)\0*.txt";
	dlg.m_ofn.lpstrTitle="Save";
	dlg.m_ofn.lpstrDefExt=".txt";	
	if(dlg.DoModal()==IDCANCEL)return ;

	CString sFile=dlg.GetPathName();

	FILE *fp=fopen(sFile,"wt");
	if(!fp){
		AfxMessageBox("Error: can not create file:"+sFile);
		return;
	}

	double *pX=equation.GetX();
	long n;
	long nPh;
	for(long i=0;i<m_pSvSysDoc->m_nRcvPhyNumber;i++){
		n=i;
		nPh=equation.GetTag (n);
		fprintf(fp,"%ld 0.0 0.0 0.0 0.0 0.0  0 %1.0lf 0\n",nPh,-pX[n]);
	}

	for(i=0;i<m_pSvSysDoc->m_nShotPhyNumber;i++){
		n=m_pSvSysDoc->m_nRcvPhyNumber+i;
		nPh=equation.GetTag (n);
		fprintf(fp,"%ld 0.0 0.0 0.0 0.0 0.0 %1.0lf 0 1\n",nPh,-pX[n]);
	}

	fclose(fp);
	equation.Close();

}


double CCMPAnnealDoc::CalcPowerSimply(CStaticDoc *pStaticDoc,CSegYMultiGroup *pSection)
{
	//
	if(!m_fpCMPData){
		AfxMessageBox("CCMPAnnealDoc::ReStack Report Error:There is NOT CMP file opening !");
		return 0;
	}

	if(pSection->GetGroupNumber()!=m_nCMPNumber)return 0;

	// Stack the groups:
	long nGroupMaxPointNumber=m_CMPIndexFile.m_head.nGroupMaxPointNumber ;
	

	CCMPGroupMsg *pGroupIndex;
	CSegYGroup *pSegYGroup=NULL,*pSegYGroupSection=NULL;

	long nShotStatic=0,nRcvStatic=0,nMovePoint=0;
	long nGroup;
	bool bFailed=false;
	long nGroupBodySize= m_CMPIndexFile.m_head.nGroupMaxPointNumber*sizeof(float);

	
	for(i=0;i<m_nCMPNumber;i++){ // the CMP group number

		pSegYGroupSection= pSection->GetGroup (i);
		if(!pSegYGroupSection){
			bFailed=true;
			break;
		}
		memset(pSegYGroupSection->data,0,nGroupBodySize);

		pGroupIndex=m_CMPIndexFile.GetCMPMsg (i,nGroup);				
		
		for(j=0;j<nGroup;j++){
			
			////////////////////////////////////////////
			// Get the statics:
			////////////////////////////////////////////

			// The shot static:
			n=pStaticDoc->SearchStation (pGroupIndex[j].dShotPh );
			if(n==-1){
				AfxMessageBox("Error: can not find the shot station in the static document:"+vtos(pGroupIndex[j].dShotPh ));
				bFailed=true;
				break;
			}
			nShotStatic=pStaticDoc->m_Data[n].st;
				
			// The rcv static:
			n=pStaticDoc->SearchStation (pGroupIndex[j].dRcvPh );
			if(n==-1){
				AfxMessageBox("Error: can not find the reciever station in the survey system:"+vtos(pGroupIndex[j].dRcvPh ));
				bFailed=true;
				break;
			}
			nRcvStatic=pStaticDoc->m_Data[n].rt ;

			// Total move point:
			nMovePoint=nShotStatic+nRcvStatic; 
		
			////////////////////////////////////////////
			// Get the group:
			////////////////////////////////////////////
			pSegYGroup=GetGroup(i,j,false);
			if(!pSegYGroup){
				AfxMessageBox("Error: can not get the group "+vtos(j)+" of CMP :"+vtos(i)+" when ReStack!");
				bFailed=true;
				break;
			}

			if(nMovePoint<0){
				for(k=-nMovePoint;k<pSegYGroup->nGroupSamplePoint ;k++){				
					pSegYGroupSection->data [k]+=pSegYGroup->data[k+nMovePoint];
				}
			}
			else{
				for(k=0;k<pSegYGroup->nGroupSamplePoint-nMovePoint;k++){				
					pSegYGroupSection->data [k]+=pSegYGroup->data[k+nMovePoint];					
				}
			}
		} // Groups in ONE CMP;

		if(bFailed)break;

	}	

	if(!bFailed){
		return pSection->CalcPower(m_nTargetFunction,m_nStartPoint,m_nEndPoint);
	}
	else{
		return false;
	}	
}

bool CCMPAnnealDoc::DrawOutPartCMPByStation(CString sOutFile,
									 CRange ShotStation, 
									 CRange RcvStation)
{
	//
	if(!m_fpCMPData){
		AfxMessageBox("CCMPAnnealDoc::ReStack Report Error:There is NOT CMP file opening !");
		return false;
	}

	FILE *fpOut=fopen(sOutFile,"wb");
	if(!fpOut){
		AfxMessageBox("Error: can not create file:"+sOutFile);
		return false;
	}

	////////////////////////////////////
	// Write the Vol head:
	CSegYVolHead head;
	GetVolHead(head);
	fwrite(&head,sizeof(CSegYVolHead),1,fpOut);

	
	CCMPGroupMsg *pGroupIndex;
	CSegYGroup *pSegYGroup=NULL;

	CProgressDlg dlgPro;
	dlgPro.Create();
	dlgPro.SetStatus("Stacking....");
	dlgPro.SetRange(0,m_nCMPNumber);

	bool bFailed=false;
	long nGroup,nGroupSize= m_CMPIndexFile.m_head.nGroupMaxPointNumber*sizeof(float)+240;
	long nTotalGroupSaved=0;
	
	for(i=0;i<m_nCMPNumber;i++){ // the CMP number
		dlgPro.SetPos(i);

		pGroupIndex=m_CMPIndexFile.GetCMPMsg (i,nGroup);				

		for(j=0;j<nGroup;j++){

			if(pGroupIndex[j].dShotPh >=ShotStation.start &&
				pGroupIndex[j].dShotPh <=ShotStation.end &&
				pGroupIndex[j].dRcvPh >=RcvStation.start &&
				pGroupIndex[j].dRcvPh <=RcvStation.end ){


				// Get the group:
				pSegYGroup=GetGroup(i,j,false);
				if(!pSegYGroup){
					AfxMessageBox("Error: can not get the group "+vtos(j)+" of CMP :"+vtos(i)+" when ReStack!");
					bFailed=true;
					break;
				}

				pSegYGroup->nGroupSamplePoint=m_CMPIndexFile.m_head.nGroupMaxPointNumber;
				fwrite(pSegYGroup,nGroupSize,1,fpOut);
				nTotalGroupSaved++;
			}

		} // Groups in ONE CMP;

		if(bFailed)break;
	}	
	
	AfxMessageBox("Info: "+vtos(nTotalGroupSaved)+" groups have been saved into :"+sOutFile);
	dlgPro.DestroyWindow ();
	fclose(fpOut);
	
	return !bFailed;




}

bool CCMPAnnealDoc::SetSeededMark(double  dStation, bool bShotStatic,bool bSetMark)
{
	long i,j;
	CSegYGroup *pGroup=NULL;
	CCMPGroupMsg *pCMPMsg=NULL;
	long nGroupNumber=0;
	for(i=0;i<m_nCMPNumber;i++){
		pCMPMsg=m_CMPIndexFile.GetCMPMsg (i,nGroupNumber);
		
		// For Shot Static:
		if(bShotStatic){
			for(j=0;j<nGroupNumber;j++){				
				if(pCMPMsg[j].dShotPh ==dStation){
					pGroup=GetGroup(i,j);
					if(!pGroup)continue;

					pGroup->bShotStaticFixed =bSetMark;

					SaveGroup(i,j,pGroup,true);
				}
			}
		}

		// For Reciever Static:
		else {
			for(j=0;j<nGroupNumber;j++){
				if(pCMPMsg[j].dRcvPh ==dStation){
					pGroup=GetGroup(i,j);
					if(!pGroup)continue;

					pGroup->bRcvStaticFixed =bSetMark;
					
					SaveGroup(i,j,pGroup,true);
				}
			}
		}
	}
	
	return true;
}

bool CCMPAnnealDoc::ClearSeededMark()
{
	long i,j;
	CSegYGroup *pGroup=NULL;
	CCMPGroupMsg *pCMPMsg=NULL;
	long nGroupNumber=0;
	for(i=0;i<m_nCMPNumber;i++){
		pCMPMsg=m_CMPIndexFile.GetCMPMsg (i,nGroupNumber);
		
		for(j=0;j<nGroupNumber;j++){				
			pGroup=GetGroup(i,j,false,NULL,true);
			if(!pGroup)continue;

			if(pGroup->bShotStaticFixed !=0||pGroup->bRcvStaticFixed !=0){
				pGroup->bShotStaticFixed =0;
				pGroup->bRcvStaticFixed =0;
				SaveGroup(i,j,pGroup,true);
			}

		}
	}

	return true;

}

bool CCMPAnnealDoc::InitCoreStack()
{
	///////////////////////////////////////////
	// Check if the environment is valid:
	if(!m_pStackSectionCurr){
		AfxMessageBox("Error: the stack section document in the CCMPAnnealDoc has not been set!");
		return false;
	}
	if(!m_pSvSysDoc){
		AfxMessageBox("CCMPAnnealDoc::CheckPowerIncOfShot() Error: the survey system has not been set in the CCMP document!");
		return false;
	}

	///////////////////////////////////////////////////
	//	Preparation:
	long i,j,k,n;
	bool bFailed=false;
	long nGroupNumber=0;
	double dShotStation,dRcvStation;
	int nShotStatic,nRcvStatic,nTotalStatic;
	CCMPGroupMsg *pMsg=NULL;
	CCMPIndexHead &head=m_CMPIndexFile.m_head ;
	CSegYGroup *pSectionGroup=NULL,*pCMPGroup=NULL;

	m_pStackSectionCurr->SetPar (m_nCMPNumber,head.nGroupMaxPointNumber ,head.nSampleInterval);
	m_pStackSectionTest->SetPar (m_nCMPNumber,head.nGroupMaxPointNumber ,head.nSampleInterval); // for restoring the section before plus groups

	///////////////////////////////////
	// Plus all of the CMPs:
	for(i=0;i<m_nCMPNumber;i++){

		///////////////////////////////////
		// Get the CMP Groups Messages:
		pMsg=m_CMPIndexFile.GetCMPMsg (i,nGroupNumber);
		if(!pMsg){
			AfxMessageBox("Error: can not get the CMP groups!");
			bFailed=true;
			break;
		}

		pSectionGroup=m_pStackSectionCurr->GetGroup (i);

		///////////////////////////////////
		// Plus all of the seeded groups:		
		for(j=0;j<nGroupNumber;j++){			
			
			// The reciever station:
			dRcvStation=pMsg[j].dRcvPh ;
			n=m_pSvSysDoc->SearchRcvStation (dRcvStation);
			if(n==-1){
				AfxMessageBox("Error: can not find the reciever station in the survey system:"+vtos(dRcvStation));
				bFailed=true;
				break;
			}

			if(!m_pSvSysDoc->m_pRcvPhyPar [n].bValid )continue;
			nRcvStatic=m_pSvSysDoc->m_pRcvPhyPar [n].rt ;

			// The shot station:
			dShotStation=pMsg[j].dShotPh ;
			n=m_pSvSysDoc->SearchShotStation (dShotStation);
			if(n==-1){
				AfxMessageBox("Error: can not find the shot station in the survey system:"+vtos(dShotStation));
				bFailed=true;
				break;
			}
			if(!m_pSvSysDoc->m_pShotPhyPar [n].bValid )continue;
			nShotStatic=m_pSvSysDoc->m_pShotPhyPar [n].st ;

			// Plus this group into the section:
			pCMPGroup=GetGroup(i,j);
			if(!pCMPGroup){
				AfxMessageBox("Error: can not get the CMP group in the CMP file!");
				bFailed=true;
				break;
			}			

			nTotalStatic=nShotStatic+nRcvStatic;
			if(nTotalStatic>0){			
				for(k=0;k<pCMPGroup->nGroupSamplePoint-nTotalStatic ;k++){
					pSectionGroup->data [k]+=pCMPGroup->data [k+nTotalStatic ];
				}
			}
			else{
				for(k=-nTotalStatic;k<pCMPGroup->nGroupSamplePoint;k++){
					pSectionGroup->data [k]+=pCMPGroup->data [k+nTotalStatic ];
				}
			}


		}
		if(bFailed)break;
	}

	//m_pStackSectionCurr->SaveToFile ();


	return !bFailed;
}

////////////////////////////////////////////////////
// Used in Core Calculation Method:
// nShotStatic: transfered here is the static point, not value;
double CCMPAnnealDoc::CalcCorePowerOfShotStation(double dShotStation,int nShotStatic,bool bCheckPower)
{
	///////////////////////////////////////////////////
	// Preparation:
	if(!m_pSvSysDoc)return 0;

	///////////////////////////////////////////////////
	// Get the recievers stations of the shot:
	long nRcvStationNumber;
	double *pRcvStation=m_pSvSysDoc->GetRcvStation (dShotStation,nRcvStationNumber);
	if(!pRcvStation)return 0;

	////////////////////////////////////////////////////////////////////
	// Save the section state for restoring after plus the groups:
	memset(m_pCMPStateChanged,0,sizeof(bool)*m_nCMPNumber);
	long nGroupSize=m_CMPIndexFile.m_head.nGroupMaxPointNumber *sizeof(float)+SEGY_HEAD_LEN;

	////////////////////////////////////////////////////////////////
	// Plus the groups of all of the recievers from the shot:
	CSegYGroup *pCMPGroup=NULL,*pTestSectionGroup=NULL,*pCurrSectionGroup=NULL;
	bool bFailed=false;
	long nCMPOrder,nGroupOrderInCMP;
	int nRcvStatic,nTotalStatic;
	long i,j,n;	
	
	for(i=0;i<nRcvStationNumber;i++){
		
		// Search the recievers that have been calculated out and get its static:
		n=m_pSvSysDoc->SearchRcvStation (pRcvStation[i]);
		if(n==-1){
			TRACEERROR("Error: can not find the reciever station in the survey system:"+vtos(pRcvStation[i]));
			bFailed=true;
			break;
		}
		if(!m_pSvSysDoc->m_pRcvPhyPar [n].bValid )continue;

		// Get the reciver station static:
		nRcvStatic=m_pSvSysDoc->m_pRcvPhyPar [n].rt ;

		// Get the group from the cmp file:
		pCMPGroup=GetCMPGroupByStation(dShotStation,pRcvStation[i],nCMPOrder,nGroupOrderInCMP);
		if(!pCMPGroup){
			TRACEERROR("Error: can not find the CMP group of shot and reciever station:"+vtos(dShotStation)+", "+vtos(pRcvStation[i]));
			bFailed=true;
			break;
		}
		
		// Save the state of the section group:
		pCurrSectionGroup=m_pStackSectionCurr->GetGroup (nCMPOrder);
		if(bCheckPower){
			m_pCMPStateChanged[nCMPOrder]=true;
			pTestSectionGroup=m_pStackSectionTest->GetGroup (nCMPOrder);			
			memcpy(pTestSectionGroup,pCurrSectionGroup,nGroupSize);
		}
		
		// Plus the group into the core stack section:
		nTotalStatic=nShotStatic+nRcvStatic;
		if(nTotalStatic>0){
			for(j=0;j<pCMPGroup->nGroupSamplePoint-nTotalStatic ;j++){
				pCurrSectionGroup->data [j]+=pCMPGroup->data [j+nShotStatic+nRcvStatic];
			}
		}
		else{
			for(j=-nTotalStatic;j<pCMPGroup->nGroupSamplePoint;j++){
				pCurrSectionGroup->data [j]+=pCMPGroup->data [j+nShotStatic+nRcvStatic];
			}
		}

	}

	///////////////////////////////////////////////////////////////////////
	// The Caller want to calculate the power after plusing the groups:
	// then the core section should be restored after plusing:
	double dPowerTest;
	if(bCheckPower){

		// Calc the power of the core section:
		dPowerTest=0.0;
		CSegYGroup *pSegYGroup=NULL,*pSegYGroupLast=NULL;
		if(m_nTargetFunction==CORRELATION){
			pSegYGroup=m_pStackSectionCurr->GetGroup(0);
			for(i=1;i<m_nCMPNumber;i++){
				pSegYGroupLast=pSegYGroup;
				pSegYGroup=m_pStackSectionCurr->GetGroup (i);
				for(j=m_nStartPoint;j<m_nEndPoint;j++){
					dPowerTest+=pSegYGroupLast->data[j]*pSegYGroup->data[j];
				}
			} 
		}
		else {		
			for(i=0;i<m_nCMPNumber;i++){
				pSegYGroup=m_pStackSectionCurr->GetGroup (i);
				for(j=m_nStartPoint;j<m_nEndPoint;j++){				
					dPowerTest+=pSegYGroup->data[j]*pSegYGroup->data[j];
				}
			} 
		}

		dPowerTest/=(m_nCMPNumber-1)*(m_nEndPoint-m_nStartPoint);

		// Restore the section state before plusing:
		for(i=0;i<m_nCMPNumber;i++){
			if(	m_pCMPStateChanged[i]){	
				pTestSectionGroup=m_pStackSectionTest->GetGroup (i);
				pCurrSectionGroup=m_pStackSectionCurr->GetGroup (i);
				memcpy(pCurrSectionGroup,pTestSectionGroup,nGroupSize);
			}
		}
	}

	//////////////////////////////////////////
	// return:
	return dPowerTest;
}

////////////////////////////////////////////////////
// Used in Core Calculation Method:
// nRcvStatic: transfered here is the static point, not value;
double CCMPAnnealDoc::CalcCorePowerOfRcvStation(double dRcvStation,int nRcvStatic,bool bCheckPower)
{
	///////////////////////////////////////////////////
	// Preparation:
	if(!m_pSvSysDoc)return 0;

	///////////////////////////////////////////////////
	// Get the recievers stations of the shot:
	long nShotStationNumber;
	double *pShotStation=m_pSvSysDoc->GetShotStation (dRcvStation,nShotStationNumber);
	if(!pShotStation)return 0;

	////////////////////////////////////////////////////////////////////
	// Save the section state for restoring after plus the groups:
	memset(m_pCMPStateChanged,0,sizeof(bool)*m_nCMPNumber);
	long nGroupSize=m_CMPIndexFile.m_head.nGroupMaxPointNumber *sizeof(float)+SEGY_HEAD_LEN;

	////////////////////////////////////////////////////////////////
	// Plus the groups of all of the recievers from the shot:
	CSegYGroup *pCMPGroup=NULL,*pTestSectionGroup=NULL,*pCurrSectionGroup=NULL;
	bool bFailed=false;
	long nCMPOrder,nGroupOrderInCMP;
	int nShotStatic,nTotalStatic;
	long i,j,n;	
	
	for(i=0;i<nShotStationNumber;i++){
		
		// Search the recievers that have been calculated out and get its static:
		n=m_pSvSysDoc->SearchShotStation (pShotStation[i]);
		if(n==-1){
			TRACEERROR("Error: can not find the reciever station in the survey system:"+vtos(pShotStation[i]));
			bFailed=true;
			break;
		}
		if(!m_pSvSysDoc->m_pShotPhyPar [n].bValid )continue;

		// Get the reciver station static:
		nShotStatic=m_pSvSysDoc->m_pShotPhyPar [n].st ;

		// Get the group from the cmp file:
		pCMPGroup=GetCMPGroupByStation(pShotStation[i],dRcvStation,nCMPOrder,nGroupOrderInCMP);
		if(!pCMPGroup){
			TRACEERROR("Error: can not find the CMP group of shot and reciever station:"+vtos(dRcvStation)+", "+vtos(pShotStation[i]));
			bFailed=true;
			break;
		}
		
		// Save the state of the section group:
		pCurrSectionGroup=m_pStackSectionCurr->GetGroup (nCMPOrder);
		if(bCheckPower){
			m_pCMPStateChanged[nCMPOrder]=true;
			pTestSectionGroup=m_pStackSectionTest->GetGroup (nCMPOrder);			
			memcpy(pTestSectionGroup,pCurrSectionGroup,nGroupSize);
		}
		
		// Plus the group into the core stack section:
		nTotalStatic=nShotStatic+nRcvStatic;
		if(nTotalStatic>0){
			for(j=0;j<pCMPGroup->nGroupSamplePoint-nTotalStatic ;j++){
				pCurrSectionGroup->data [j]+=pCMPGroup->data [j+nShotStatic+nRcvStatic];
			}
		}
		else{
			for(j=-nTotalStatic;j<pCMPGroup->nGroupSamplePoint;j++){
				pCurrSectionGroup->data [j]+=pCMPGroup->data [j+nShotStatic+nRcvStatic];
			}
		}
	}

	///////////////////////////////////////////////////////////////////////
	// The Caller want to calculate the power after plusing the groups:
	// then the core section should be restored after plusing:
	double dPowerTest;
	if(bCheckPower){

		// Calc the power of the core section:
		dPowerTest=0.0;
		CSegYGroup *pSegYGroup=NULL,*pSegYGroupLast=NULL;
		if(m_nTargetFunction==CORRELATION){
			pSegYGroup=m_pStackSectionCurr->GetGroup(0);
			for(i=1;i<m_nCMPNumber;i++){
				pSegYGroupLast=pSegYGroup;
				pSegYGroup=m_pStackSectionCurr->GetGroup (i);
				for(j=m_nStartPoint;j<m_nEndPoint;j++){
					dPowerTest+=pSegYGroupLast->data[j]*pSegYGroup->data[j];
				}
			} 
		}
		else {		
			for(i=0;i<m_nCMPNumber;i++){
				pSegYGroup=m_pStackSectionCurr->GetGroup (i);
				for(j=m_nStartPoint;j<m_nEndPoint;j++){				
					dPowerTest+=pSegYGroup->data[j]*pSegYGroup->data[j];
				}
			} 
		}

		dPowerTest/=(m_nCMPNumber-1)*(m_nEndPoint-m_nStartPoint);

		// Restore the section state before plusing:
		for(i=0;i<m_nCMPNumber;i++){
			if(	m_pCMPStateChanged[i]){	
				pTestSectionGroup=m_pStackSectionTest->GetGroup (i);
				pCurrSectionGroup=m_pStackSectionCurr->GetGroup (i);
				memcpy(pCurrSectionGroup,pTestSectionGroup,nGroupSize);
			}
		}
	}

	//////////////////////////////////////////
	// return:
	return dPowerTest;
}

