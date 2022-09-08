
// CMPDoc.cpp : implementation file
//

#include "stdafx.h"
#include "RefStatic.h"
#include "ProgDlg.h"
#include "SvSysBaseDoc.h"
#include "CMPAnnealDoc.h"
#include "MyPoint.h"
#include "segy.h"
#include "FHZoomViewGlobal.h"
#include "MyGroupHead.h"
#include "templates.h"
#include "MemEqua.h"
#include "reverse.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCMPAnnealDoc

IMPLEMENT_DYNCREATE(CCMPAnnealDoc, CCMPBaseDoc)

CCMPAnnealDoc::CCMPAnnealDoc()
{
	m_pSvSysDoc=NULL;
	m_fpCMPData=NULL;
	
	m_nStartPoint=-1;
	m_nEndPoint=-1;
	m_nCMPNumber=0;

	m_pStackSectionCurr=new CSegYMultiGroup;
	m_pStackSectionTest=new CSegYMultiGroup;

	m_dPowerCurr=0.0;

	m_pCMPStateChanged=NULL;

	m_nTargetFunction=POWER;
}

BOOL CCMPAnnealDoc::OnNewDocument()
{
	if (!CCMPBaseDoc::OnNewDocument())
		return FALSE;
	
	CString sCMPFile;
	if(!MakeCMPFile (sCMPFile,NULL)){
		AfxMessageBox("Warning: the CMP file was not craeted!");
		return false;
	}

	// Open the CMP file just made :
	return OpenCMPFile(sCMPFile);

}


CCMPAnnealDoc::~CCMPAnnealDoc()
{
	if(m_pStackSectionCurr){
		delete m_pStackSectionCurr;
		m_pStackSectionCurr=NULL;
	}

	if(m_pStackSectionTest){
		delete m_pStackSectionTest;
		m_pStackSectionCurr=NULL;
	}

}

BEGIN_MESSAGE_MAP(CCMPAnnealDoc, CCMPBaseDoc)
	//{{AFX_MSG_MAP(CCMPAnnealDoc)
	ON_COMMAND(ID_CMP_PlusStatic, OnCMPPlusStatic)
	ON_COMMAND(ID_CMP_PlusShotStatic, OnCMPPlusShotStatic)
	ON_COMMAND(ID_CMP_PlusRcvStatic, OnCMPPlusRcvStatic)
	ON_COMMAND(ID_CMP_DumpGroupHead, OnCMPDumpGroupHead)
	ON_COMMAND(ID_CMP_StackNOStatic, OnCMPStackNOStatic)
	ON_COMMAND(ID_CMP_StackWithStatic, OnCMPStackWithStatic)
	ON_COMMAND(ID_CMP_MoveGroupByStatic, OnCMPMoveGroupByStatic)
	ON_COMMAND(ID_CMP_CalcSurveySystem, OnCMPCalcSurveySystem)
	ON_COMMAND(ID_CMP_CutCMPSection, OnCMPCutCMPSection)
	ON_COMMAND(ID_CMP_Correlation, OnCMPCorrelation)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



void CCMPAnnealDoc::SetPar(CSvSysDoc *pSvSysDoc, long nStaticRange, enumPowerMethod nTarget)
{
	//////////////////////////////////////////
	// 设置观测系统指针：
	m_pSvSysDoc=pSvSysDoc;

	///////////////////////////////////////////
	// 设置静校正量范围:
	CCMPIndexHead &head=m_CMPIndexFile.m_head;
	long nLimit=head.nGroupMinPointNumber*head.nSampleInterval/3;
	if(nStaticRange<10)nStaticRange=10;
	if(nStaticRange>nLimit)nStaticRange=nLimit;

	m_nStartPoint=nStaticRange/head.nSampleInterval+2;
	m_nEndPoint=head.nGroupMinPointNumber -m_nStartPoint-2;	
	if(m_nEndPoint<m_nStartPoint){
		m_nStartPoint=m_nEndPoint=0;
	}

	//////////////////////////////////////////
	// 设置能量计算方法：
	m_nTargetFunction=nTarget;
	
	//////////////////////////////////////////
	// 返回：
	return ;

}


bool CCMPAnnealDoc::CheckPowerIncOfShot(double nShotPh, 
								  int nStaticOld, 
								  int nStaticNew,
								  bool &bFailedForReturn,
								  double dTemperature,
								  bool bReplaceIfInc,
								  bool bSeedMethod)
{

	if(!m_pStackSectionCurr||!m_pStackSectionTest){
		AfxMessageBox("Error: the stack section document in the CCMPAnnealDoc has not been set!");
		return false;
	}
	if(!m_pSvSysDoc){
		AfxMessageBox("CCMPAnnealDoc::CheckPowerIncOfShot() Error: the survey system has not been set in the CCMP document!");
		return false;
	}

	 long nShotPosInRel;
	nShotPosInRel=m_pSvSysDoc->SearchShotStationInRel (nShotPh);
	if(nShotPosInRel==-1){
		AfxMessageBox("Error:can not find the Shot station in the relation :"+vtos(nShotPh));
		return false;
	}
	

	// Loop for all of the shot stations on the rcv station:
	double nRcvPh,nCMPPh;
	CSegYGroup *pSegYGroup, *pSegYGroupTestSection,*pSegYGroupCurrSection;
	float *pDataWithStatic;
	long nCMPOrder;
	int nRT;
	bool bFailed;
	long nRcvStationNumber;

	double *pRcvPh=m_pSvSysDoc->GetRcvStation (nShotPh,nRcvStationNumber);
	
	nRT=0;
	bFailed=false;
	n=0;
	for(i=0;i<nRcvStationNumber ;i+=1){
		
		// The reciever and cmp station:
		nRcvPh=pRcvPh[i];
		nCMPPh=(nRcvPh+nShotPh)/2;

		// Find the reciever static:
		n=m_pSvSysDoc->SearchRcvStation (nRcvPh);
		if(n==-1){
			AfxMessageBox("Error: can not find the reciever station in the survey system, Shot: "+vtos(nRcvPh));
			bFailed=true;
			break;
		}
		nRT=m_pSvSysDoc->m_pRcvPhyPar [n].rt  ;
	
		

		// Get the group:
		pSegYGroup=GetCMPGroupByRcvStation(nCMPPh,nRcvPh,nCMPOrder);
		if(pSegYGroup==NULL){
			AfxMessageBox("Error: can not find the reciever group in the CMP file, reciever : "+vtos(nRcvPh)+", CMP station:"+vtos(nCMPPh));
			bFailed=true;
			break;			
		}

		// Change the pointer to CSegYGroup type:
		pSegYGroupTestSection=m_pStackSectionTest->GetGroup (nCMPOrder);
		pSegYGroupCurrSection=m_pStackSectionCurr->GetGroup (nCMPOrder);
  		if(!pSegYGroupTestSection||!pSegYGroupCurrSection){
			AfxMessageBox("Error: the memory has not been allocated in the stack section document!");
			bFailed=true;
			break;
		}
		
		// firstly minus the group with the old static :
		pDataWithStatic=(float*)(&pSegYGroup->data[nStaticOld+nRT]);				
		for(j=m_nStartPoint;j<m_nEndPoint;j++){
			pSegYGroupTestSection->data [j]=pSegYGroupCurrSection->data[j]-pDataWithStatic[j];
		}

		// then plus the group with the new static:
		pDataWithStatic=(float*)(&pSegYGroup->data [nStaticNew+nRT]);
		for(j=m_nStartPoint;j<m_nEndPoint;j++){
			pSegYGroupTestSection->data [j]+=pDataWithStatic[j];
		}
	}
//
	if(bFailed){
		bFailedForReturn=bFailed;
		return false;
	}

	return IsPowerInc(dTemperature,
		bReplaceIfInc,
		n,
		nShotPh,
		nStaticOld,
		nStaticNew,
		true,
		bSeedMethod);

}

bool CCMPAnnealDoc::CheckPowerIncOfRcv(double nRcvPh,
								 int nStaticOld, 
								 int nStaticNew,
								 bool &bFailedForReturn,
								 double dTemperature,
								 bool bReplaceIfInc,
								 bool bSeedMethod)
{
	if(!m_pStackSectionCurr||!m_pStackSectionTest){
		AfxMessageBox("Error: the stack section document in the CCMPAnnealDoc has not been set!");
		return false;
	}
	if(!m_pSvSysDoc){
		AfxMessageBox("CCMPAnnealDoc::CheckPowerIncOfShot() Error: the survey system has not been set in the CCMP document!");
		return false;
	}
  
	long nRcvPosInRel;
	nRcvPosInRel=m_pSvSysDoc->SearchRcvStationInRel (nRcvPh);
	if(nRcvPosInRel==-1){
		AfxMessageBox("Error:can not find the reciever station in the relation :"+vtos(nRcvPh));
		return false;
	}
	

	// Loop for all of the shot stations on the rcv station:
	double nShotPh,nCMPPh;
	CSegYGroup *pSegYGroup,*pSegYGroupTestSection,*pSegYGroupCurrSection;
	float *pDataWithStatic;
	long nCMPOrder;
	int nST;
	bool bFailed;

	nST=0;
	bFailed=false;
	for(i=0;i<m_pSvSysDoc->m_pRcvShotRel [nRcvPosInRel].nShotNumber ;i+=1){
		
		//
		nShotPh=m_pSvSysDoc->m_pRcvShotRel [nRcvPosInRel].pGroup [i].nPhShot ;

		// Find the shot static:
		n=m_pSvSysDoc->SearchShotStation (nShotPh);
		if(n==-1){
			AfxMessageBox("Error: can not find the Shot station in the survey system, Shot: "+vtos(nShotPh));
			bFailed=true;
			break;
		}
		nST=m_pSvSysDoc->m_pShotPhyPar [n].st ;
	
		nCMPPh=(nRcvPh+nShotPh)/2;

		// Get the group:
		pSegYGroup=GetCMPGroupByShotStation(nCMPPh,nShotPh,nCMPOrder);
		if(pSegYGroup==NULL){
			AfxMessageBox("Error: can not find the group with Shot Station in the CMP file, shot: "+vtos(nShotPh)+", CMP station:"+vtos(nCMPPh));
			bFailed=true;
			break;			
		}


		// Change the pointer to CSegYGroup type:
		pSegYGroupTestSection=m_pStackSectionTest->GetGroup(nCMPOrder);
		pSegYGroupCurrSection=m_pStackSectionCurr->GetGroup(nCMPOrder);
  		if(!pSegYGroupTestSection||!pSegYGroupCurrSection){
			AfxMessageBox("Error: the memory has not been allocated in the stack section document!");
			bFailed=true;
			break;
		}

		// firstly minus the group with the old static :		
		pDataWithStatic=(float*)(&pSegYGroup->data[nStaticOld+nST]);
		
		for(j=m_nStartPoint;j<m_nEndPoint;j++){
			pSegYGroupTestSection->data [j]=pSegYGroupCurrSection->data[j]-pDataWithStatic[j];
		}


		// then plus the group with the new static:
		pDataWithStatic=(float*)(&pSegYGroup->data [nStaticNew+nST]);

		for(j=m_nStartPoint;j<m_nEndPoint;j++){
			pSegYGroupTestSection->data [j]+=pDataWithStatic[j];
		}

		if(bFailed)break;
	}


	
	//
	/*
	ReStack(false);
	*/
	if(bFailed){
		bFailedForReturn=bFailed;
		return false;
	} 
	return IsPowerInc(dTemperature,
		bReplaceIfInc,
		m_pSvSysDoc->m_pRcvShotRel [nRcvPosInRel].nShotNumber,
		nRcvPh,
		nStaticOld,
		nStaticNew,
		false,
		bSeedMethod);
}










bool CCMPAnnealDoc::MakeCMPFile(CString &sCMPFile,CSeisDoc *pSeisDoc)
{
	///////////////////////////////////////////////////////	
	// A common dialog that will be used through this function:
	CFdataApp *pApp=(CFdataApp*)AfxGetApp();	

	bool bSuccess=false;
	CString sCMPIndex;
	CSvSysDoc *pSvSysDoc=NULL;
	CCMPIndexFile *pCMPIndexDoc=NULL;
	bool bSeisOpenedHere=false;
	double dCMPDis=0.5;
	bool bUseCoor=false;
	
	FILE *fpCMPData=NULL;

	while(true){  // only use its "break" function;

		CDlgSeisToCMP dlg;
		if(pSeisDoc){
			dlg.m_bSeisNeglect =true;
		}
		if(dlg.DoModal ()==IDCANCEL)return false;

		/////////////////////////////////////////////////
		// Open survey system file:
		// Read the shot point parameter file:
		if(!pSeisDoc){
			CString sSeisFile=dlg.m_sShotGatherFile ;
			if(sSeisFile=="")break;
			
			pSeisDoc=new CSeisDoc;
			pSeisDoc->OnOpenDocument (sSeisFile);
			if(!pSeisDoc){
				AfxMessageBox("Error: when opening the seismic data file!");
				break;
			}
			bSeisOpenedHere=true;
		}

		/////////////////////////////////////////////////
		// Open survey system file:
		// Read the shot point parameter file:		
		CString sSvSysFile=dlg.m_sSvSysFile ;
		if(sSvSysFile=="")break;

		pSvSysDoc=new CSvSysDoc;
		pSvSysDoc->OnOpenDocument (sSvSysFile);
		if(!pSvSysDoc){
			AfxMessageBox("Error: when opening the survey system file!");
			break;
		}
		dCMPDis=pSvSysDoc->m_dCMPDis*0.9;

		//////////////////////////////////////////////////////////////////
		// We must save the CMP document here, because it is too large
		// We can  not save them in the memory:
		sCMPFile=dlg.m_sCMPGatherFile ;
		if(sCMPFile=="")break;

		fpCMPData=fopen(sCMPFile,"wb");	  
		if(!fpCMPData){
			AfxMessageBox("Error: when creating the CMP file:"+sCMPFile);
			break;
		}

		bUseCoor=(dlg.m_nUseStation ==1);
		bSuccess=true;

		break;  // only use the while's break function
	}

	if(!bSuccess){
		if(pSvSysDoc){
			delete pSvSysDoc;
		}
		if(bSeisOpenedHere&&pSeisDoc){
			delete pSeisDoc;
		}
		if(fpCMPData){
			fclose(fpCMPData);
		}
		return false;
	}


	///////////////////////////////////////////////////////////////
	// Firstly check how many CMP points are there in this file:
	///////////////////////////////////////////////////////////////
	long i,j,p;

	double *pRcvStation=NULL;
	double dx,dy,dl,ShotEast,ShotNorth,RcvEast,RcvNorth,CMPEast,CMPNorth;
	long nShotStation,nRcvStation,n;
	long nRcvNumber;

	//////////////////////////////////////////////
	// Open a empty CMPIndex file, 
	// which will be used through this class:
	CCMPIndexHead &head=m_CMPIndexFile.m_head ;

	head.nCMPNumber =0;
	head.index [0].nGroupNumber =0;
	head.nGroupHeadPointNumber =60;
	head.nGroupMaxPointNumber =pSeisDoc->m_nGroupBodyPointNumber ;
	head.nGroupMinPointNumber =pSeisDoc->m_nGroupBodyPointNumber ;
	head.nSampleInterval =pSeisDoc->m_nSampleInterval ;


	CProgressDlg dlgProgress;
	dlgProgress.Create();
	dlgProgress.SetRange(0,pSvSysDoc->m_nRelNumber*2);
	dlgProgress.SetStatus ("Calculating the CMP points!");

	bSuccess=true;
	for(i=0;i<pSvSysDoc->m_nRelNumber ;i++){
		dlgProgress.SetPos (i);

		// Get the shot point position:
		nShotStation=pSvSysDoc->m_pShotRcvRel [i].PhShot;
		n=pSvSysDoc->SearchShotStation (nShotStation);
		if(n==-1){
			AfxMessageBox("Can not find the station "+vtos(nShotStation)+", this should be corrected!");
			bSuccess=false;
			break;
		}

		if(bUseCoor){
			ShotEast=pSvSysDoc->m_pShotPhyPar [n].east ;
			ShotNorth=pSvSysDoc->m_pShotPhyPar [n].north ;
		}
		else{
			ShotEast=pSvSysDoc->m_pShotPhyPar [n].Ph ;
			ShotNorth=0;
		}


		// Loop to find the reciever point position:
		pRcvStation=pSvSysDoc->GetRcvStation (nShotStation,nRcvNumber);

		for(j=0;j<nRcvNumber;j++){  
			
			nRcvStation=pRcvStation[j];

			// Get the rcv point position:
			n=pSvSysDoc->SearchRcvStation (nRcvStation);
			if(n==-1){
				AfxMessageBox("Can not find the reciever station from the reciever pars :"+vtos(nRcvStation)+", this should be mended!");
				bSuccess=false;
				break;
			}
			if(bUseCoor){
				RcvEast=pSvSysDoc->m_pRcvPhyPar [n].east ;
				RcvNorth=pSvSysDoc->m_pRcvPhyPar [n].north ;
			}
			else{
				RcvEast=pSvSysDoc->m_pRcvPhyPar [n].Ph ;
				RcvNorth=0;
			}

			CMPEast=(ShotEast+RcvEast)/2;
			CMPNorth=(ShotNorth+RcvNorth)/2;

			// check if there is this CMP point in the existing CMP Point
			// if not , the cmp point number will increase one:
			double dMinL=99999999999999999999999.9;
			int nCMP;
			for(p=0;p<head.nCMPNumber;p++){
				dx=head.index [p].east  -CMPEast;
				dy=head.index [p].north -CMPNorth;

				dl=sqrt(dx*dx+dy*dy);
				if(dl<dMinL){
					dMinL=dl;
					nCMP=p;
				}
			}

			// If there is this CMP point, then increase the group number of this CMP point:
			if(dMinL>=dCMPDis){
				if(head.nCMPNumber >=m_CMPIndexFile.m_nCMPLimit ){
					AfxMessageBox("Too many CMP points, error!");
					bSuccess=false;
					break;
				}
				head.index [head.nCMPNumber ].nStation =(nShotStation+nRcvStation)/2;
				head.index [head.nCMPNumber ].nGroupNumber =0;
				head.index [head.nCMPNumber ].east =CMPEast;
				head.index [head.nCMPNumber ].north=CMPNorth;
				head.nCMPNumber ++;
			}
		}
		if(!bSuccess)break;
	}

	if(!bSuccess){
		if(pSvSysDoc){
			pSvSysDoc->OnCloseDocument ();
		}
		if(bSeisOpenedHere&&pSeisDoc){
			pSeisDoc->OnCloseDocument ();
		}
		if(fpCMPData){
			fclose(fpCMPData);
		}
		return false;
	}
	
	/////////////////////////////////////////////////////////////
	// Calc how many groups are there in each CMP point:
	bSuccess=true;
	for(i=0;i<pSvSysDoc->m_nRelNumber ;i++){
		dlgProgress.SetPos (i+pSvSysDoc->m_nRelNumber);

		// Get the shot point position:
		nShotStation=pSvSysDoc->m_pShotRcvRel [i].PhShot;
		n=pSvSysDoc->SearchShotStation (nShotStation);
		if(n==-1){
			AfxMessageBox("Can not find the station "+vtos(nShotStation)+", this should be corrected!");
			bSuccess=false;
			break;
		}

		if(bUseCoor){
			ShotEast=pSvSysDoc->m_pShotPhyPar [n].east ;
			ShotNorth=pSvSysDoc->m_pShotPhyPar [n].north ;
		}
		else{
			ShotEast=pSvSysDoc->m_pShotPhyPar [n].Ph ;
			ShotNorth=0;
		}


		// Loop to find the reciever point position:
		pRcvStation=pSvSysDoc->GetRcvStation (nShotStation,nRcvNumber);
		for(j=0;j<nRcvNumber;j++){  
			
			nRcvStation=pRcvStation[j];
	
		
			// Get the rcv point position:
			n=pSvSysDoc->SearchRcvStation (nRcvStation);
			if(n==-1){
				AfxMessageBox("Can not find the reciever station from the reciever pars :"+vtos(nRcvStation)+", this should be mended!");
				bSuccess=false;
				break;
			}
			if(bUseCoor){
				RcvEast=pSvSysDoc->m_pRcvPhyPar [n].east ;
				RcvNorth=pSvSysDoc->m_pRcvPhyPar [n].north ;
			}
			else{
				RcvEast=pSvSysDoc->m_pRcvPhyPar [n].Ph ;
				RcvNorth=0;
			}

			CMPEast=(ShotEast+RcvEast)/2;
			CMPNorth=(ShotNorth+RcvNorth)/2;

			// check if there is this CMP point in the existing CMP Point
			// if not , the cmp point number will increase one:
			double dMinL=99999999999999999999999.9;
			int nCMP;
			for(p=0;p<head.nCMPNumber;p++){
				dx=head.index [p].east  -CMPEast;
				dy=head.index [p].north -CMPNorth;
				dl=sqrt(dx*dx+dy*dy);

				if(dl<dMinL){
					dMinL=dl;
					nCMP=p;
				}
			}

			// If there is this CMP point, then increase the group number of this CMP point:
			if(dMinL<=dCMPDis)
				head.index [nCMP].nGroupNumber ++;
			else{
				AfxMessageBox("Error: can not find this CMP in the calculated CMPs!");
				bSuccess=false;
				break;
			}
		}

	}
	dlgProgress.DestroyWindow ();
		
	if(!bSuccess){
		if(pSvSysDoc){
			pSvSysDoc->OnCloseDocument ();
		}
		if(bSeisOpenedHere&&pSeisDoc){
			pSeisDoc->OnCloseDocument ();
		}
		if(fpCMPData){
			fclose(fpCMPData);
		}
		return false;
	}
	
	//CalcCMPGroupPos();

	m_CMPIndexFile.SortCMPIndexMsg (head.index ,head.nCMPNumber );	

	head.nSEGY_PointSize =	pSeisDoc->GetDataPointLen ();	
	head.nGroupHeadPointNumber =pSeisDoc->m_nGroupHeadPointNumber ;
	head.nSampleInterval=pSeisDoc->m_nSampleInterval ;


	long *pCMPGroupNumber=new long[head.nCMPNumber +10];  // recording how many groups have been in the CMP;
	memset(pCMPGroupNumber,0,sizeof(long)*(head.nCMPNumber +10));

	///////////////////////////////////////////////
	// Save the groups into the CMP groups:
	long nShotGroup,nPos,nGroupSize,nShotPosInSeis;

	nGroupSize=sizeof(float)*pSeisDoc->m_nGroupTotalPointNumber ;

	CProgressDlg dlgCMP;
	dlgCMP.Create();
	dlgCMP.SetRange (0,pSvSysDoc->m_nRelNumber );
	dlgCMP.SetStatus("Constructing the CMP file...");

	CSegYVolHead volHead;
	pSeisDoc->GetVolHead(&volHead);	
	fseek(fpCMPData,0,SEEK_SET);
	fwrite(&volHead,sizeof(CSegYVolHead),1,fpCMPData);

	bSuccess=true;
	for(i=0;i<pSvSysDoc->m_nRelNumber ;i++){
		dlgCMP.SetPos(i);
		TRACE("Relation: %ld\n",i);

		// Get the shot point position:
		nShotStation=pSvSysDoc->m_pShotRcvRel [i].PhShot;
		n=pSvSysDoc->SearchShotStation (nShotStation);
		if(n==-1){
			AfxMessageBox("Can not find the station "+vtos(nShotStation)+", this must be corrected!");
			bSuccess=false;
			break;
		}

		if(bUseCoor){
			ShotEast=pSvSysDoc->m_pShotPhyPar [n].east ;
			ShotNorth=pSvSysDoc->m_pShotPhyPar [n].north ;
		}
		
		else{
			ShotEast=pSvSysDoc->m_pShotPhyPar [n].Ph ;
			ShotNorth=0;
		}

		//
		nShotPosInSeis=pSeisDoc->SearchFileNumber (pSvSysDoc->m_pShotRcvRel[i].FileNumber );
		if(nShotPosInSeis==-1){
			AfxMessageBox("Error: can not find the file number in the shot gather file"+vtos(pSvSysDoc->m_pShotRcvRel[i].FileNumber ));
			bSuccess=false;
			break;
		}		 

		// Loop to find the recieve point position:
		nShotGroup=-1;

		pRcvStation=pSvSysDoc->GetRcvStation (nShotStation,nRcvNumber);
		for(j=0;j<nRcvNumber;j++){  

			TRACE("Reciever: %ld,",j);
			
			nRcvStation=pRcvStation[j];

			nShotGroup++;
			
			//////////////////////////////////////
			// Get the rcv point position:
			n=pSvSysDoc->SearchRcvStation (nRcvStation);
			if(n==-1){
				AfxMessageBox("Can not find the reciever station "+vtos(nRcvStation)+" , this should be corrected!");
				bSuccess=false;
				break;
			}

			if(bUseCoor){
				RcvEast=pSvSysDoc->m_pRcvPhyPar [n].east ;
				RcvNorth=pSvSysDoc->m_pRcvPhyPar [n].north ;
			}
			else{
				RcvEast=pSvSysDoc->m_pRcvPhyPar [n].Ph;
				RcvNorth=0;
			}

			CMPEast=(ShotEast+RcvEast)/2;
			CMPNorth=(ShotNorth+RcvNorth)/2;

			///////////////////////////////////////////////////////////////
			// check if there is this CMP point in the existing CMP Point
			// if not , the cmp point number will increase one:
		 	long nCMP=-1;
			double dMinL=9999999999999999999.9;
			for(p=0;p<head.nCMPNumber ;p++){					
				dx=head.index [p].east  -CMPEast;
				dy=head.index [p].north -CMPNorth;
				dl=sqrt(dx*dx+dy*dy);
				if(dl<dMinL){
					dMinL=dl;
					nCMP=p;
				}
			}
			
			if(dMinL>dCMPDis){
				AfxMessageBox("Can not find the CMP while before  CAN find: Shot "+vtos(i)+" , Reciever Line :"+vtos(j)+" Reciever :"+vtos(nRcvStation));
				bSuccess=false;
				break;
			}

			// Check if too many groups in a CMP found:
			if(pCMPGroupNumber[nCMP]>=head.index [nCMP].nGroupNumber ){
				AfxMessageBox("Error! more CMP groups found than before!");
				bSuccess=false;
				break;
			}	

			// Save the group:
			CSegYGroup *pGroupData=pSeisDoc->GetGroup (nShotPosInSeis,nShotGroup);
			if(!pGroupData){
				AfxMessageBox("Error:can not get the seismic group:"+vtos(nShotGroup)+" , of shot:"+vtos(nShotStation));
				bSuccess=false;
				break;
			}
			
			pGroupData->nCMPOrder=nCMP;
			pGroupData->nCMPStation=head.index [nCMP].nStation ;
			pGroupData->nCMPEast=head.index [nCMP].east ;
			pGroupData->nCMPNorth=head.index [nCMP].north ;
			pGroupData->nShotStation=nShotStation;
			pGroupData->nRcvStation=nRcvStation;
			pGroupData->nGroupSamplePoint=head.nGroupMaxPointNumber ;
			
			nPos=sizeof(CSegYVolHead)+nGroupSize*(head.index [nCMP].pos+pCMPGroupNumber[nCMP]) ;
			TRACE("Position: %ld\n",nPos);
			fseek(fpCMPData,nPos,SEEK_SET);
			fwrite(pGroupData,sizeof(float),(head.nGroupHeadPointNumber + head.nGroupMaxPointNumber ),fpCMPData);
			
			pCMPGroupNumber[nCMP]++;
		}				
		if(!bSuccess)break;
	}
	dlgCMP.DestroyWindow ();

	// free space:
	delete []pCMPGroupNumber;

	if(pSvSysDoc){
		delete pSvSysDoc;
	}
	if(bSeisOpenedHere&&pSeisDoc){
		delete pSeisDoc;
	}
	if(fpCMPData){
		fclose(fpCMPData);
	}

	//
	return bSuccess;
}

CSegYMultiGroup * CCMPAnnealDoc::GetCurrStackSection()
{
	return m_pStackSectionCurr;
}



void CCMPAnnealDoc::OnCMPMoveGroupByStatic()
{
	/////////////////////////////////////////////////
	// 
	if(!m_fpCMPData){
		AfxMessageBox("CCMPAnnealDoc Report Error:There is NOT file being opening !");
		return ;
	}

	/////////////////////////////////////////////////
	// Get the other file name:
	CString sOtherCMP=GetFileForSave();
	if(sOtherCMP=="")return ;

	FILE *fpOther=fopen(sOtherCMP,"wb");
	if(!fpOther){
		AfxMessageBox("Error:can not create the new CMP file:"+sOtherCMP);
		return ;
	}

	///////////////////////////////////////
	// Firstly write the CMP file head 
	// which is just a CSegYVolHead:
	CSegYVolHead Volhead;
	GetVolHead(Volhead);
	fseek(fpOther,0,SEEK_SET);
	fwrite(&Volhead,sizeof(CSegYVolHead),1,fpOther);

	//////////////////////////////////////////////////////////////
	// If do static correction, then open the static document:	
	CCMPIndexHead &head=m_CMPIndexFile.m_head ;

	CString sStatic=CStaticDoc::GetFileForOpen ();
	if(sStatic==""){
		fclose(fpOther);
		return ;
	}
	
	CStaticDoc *pStaticDoc=new CStaticDoc ;
	if(!pStaticDoc->OnOpenDocument (sStatic)){
		fclose(fpOther);
		delete pStaticDoc;
		return ;
	}

	for(long i=0;i<pStaticDoc->m_nRecordNumber ;i++){
		pStaticDoc->m_Data [i].st =-pStaticDoc->m_Data [i].st /head.nSampleInterval ;
		pStaticDoc->m_Data [i].rt =-pStaticDoc->m_Data [i].rt /head.nSampleInterval ;
	}

	//////////////////////////////////
	//	  Move Groups and write it to the new cmp file:
	long nCMPNumber=head.nCMPNumber ;
	long nGroupHeadPointNumber=head.nGroupHeadPointNumber ;
	long nSampleInterval=head.nSampleInterval ;	
	
	long nGroupHeadSize=nGroupHeadPointNumber*sizeof(float);
	long nGroupBodySize;

	if(nSampleInterval>=1000)nSampleInterval/=1000;

	//	
	CProgressDlg dlgPro;
	dlgPro.Create();
	dlgPro.SetRange(0,nCMPNumber);
	dlgPro.SetStatus ("Moving groups by static...");

	long nPh,n;
	long nGroup,j;
	long st=0,rt=0,t=0;
	CCMPGroupMsg *pCMPGroup;
	CSegYGroup *pSegYGroup=NULL;
	CSegYGroup group;
	
	bool bFailed=false;
	for(i=0;i<nCMPNumber;i++){
		dlgPro.SetPos(i);

		pCMPGroup=m_CMPIndexFile.GetCMPMsg (i,nGroup);		
		if(!pCMPGroup){
			AfxMessageBox("Error: can not get the CMP index on "+vtos(i));
			bFailed=true;
			break;
		}

		for(j=0;j<nGroup;j++){
			
			/////////////////////////////////////////
			// Get the group data:
			pSegYGroup=GetGroup(i,j,false);
			if(!pSegYGroup){
				AfxMessageBox("Error: can not get the group:"+vtos(j)+" of CMP:"+vtos(i));
				bFailed=true;
				break;
			}

			//////////////////////////////////////////
			// Find the shot static value:
			nPh=pCMPGroup[j].dShotPh ;

			n=pStaticDoc->SearchStation (nPh);
			if(n==-1){
				AfxMessageBox("Error: can not find the shot station :"+vtos(nPh)+" in the static file!");
				bFailed=true;
				break;
			}
			st=pStaticDoc->m_Data[n].st;
			
			///////////////////////////////////////////
			// Find the reciever static value:
			nPh=pCMPGroup[j].dRcvPh;

			n=pStaticDoc->SearchStation (nPh);
			if(n==-1){
				AfxMessageBox("Error: can not find the reciever station :"+vtos(nPh)+" in the static file!");
				bFailed=true;
				break;
			}
			rt=pStaticDoc->m_Data[n].rt;

			t=st+rt;

			// Plus the group with the static value:
			memcpy(&group,pSegYGroup,nGroupHeadSize);

			nGroupBodySize=group.nGroupSamplePoint * sizeof(float);
			memset(group.data ,0,nGroupBodySize);
			if(t>0)
				memcpy(&group.data ,&pSegYGroup->data [t],nGroupBodySize-sizeof(float)*t);
			else
				memcpy(&group.data [-t],&pSegYGroup->data [0],nGroupBodySize+sizeof(float)*t);
			

			fwrite(&group,nGroupBodySize+nGroupHeadSize,1,fpOther);
		}

		if(bFailed)break;		
		
	}

	dlgPro.DestroyWindow ();


	// return:
	fclose(fpOther);

	if(pStaticDoc){
		delete pStaticDoc;
	}
	
	return ;
}
	

void CCMPAnnealDoc::OnCMPCalcSurveySystem() 
{
	// TODO: Add your command handler code here
	MakeSurveySystem(NULL);	

}

bool CCMPAnnealDoc::MakeSurveySystem(CSvSysDoc *pSvSysDoc)
{
	
	//////////////////////////////////////////////
	// 把有用的信息读出来，存成一个文件：
	//////////////////////////////////////////////	
	CFdataApp *pApp=(CFdataApp *)AfxGetApp();

	if(!pSvSysDoc){		
		pSvSysDoc=(CSvSysDoc*)(pApp->m_pSvSysTemplate ->OpenDocumentFile (NULL));
		if(!pSvSysDoc){
			AfxMessageBox("Error: can not create new survey system file!");
			return false;
		}
	}

	CString sTempFile="c:\\tempgrp.$$$";

	//////////////////////////////////////////////
	// 把有用的信息读出来，存成一个文件：
	//////////////////////////////////////////////
	FILE *fp=fopen(sTempFile,"wb");
	if(!fp){
		AfxMessageBox("Error: can not create temporary file: "+sTempFile);
		return false;
	}

	CMyGroupHead head;

	long i,j;
	long nCMPNumber=GetCMPNumber();
	long nGroupNumber,nTotalGroupNumber=0;
	CSegYGroup *pSegYGroup=NULL;

	CProgressDlg dlgPro;
	dlgPro.Create();
	dlgPro.SetStatus("Collecting group information:");
	dlgPro.SetRange (0,nCMPNumber);

	bool bFailed=false;

	bool bUseNorth=DecideUseNorth();


	for(i=0;i<nCMPNumber;i++){
		dlgPro.SetPos(i);

		nGroupNumber=GetGroupNumberInCMP(i);

		head.nCMPOrder =i;
		head.nGroupNumberInCMP=nGroupNumber;

		for(j=0;j<nGroupNumber;j++){
			pSegYGroup=GetGroup(i,j);
			if(!pSegYGroup){
				TRACEERROR("Error: can not get group "+vtos(j)+" of CMP "+vtos(i));
				continue;
			}

			head.nGroupOrderInCMP =j;
			head.nFileNumber =pSegYGroup->nShotFileNumber ;
			
			// The shot station:
			if(bUseNorth)
			{
				head.dShotStation =pSegYGroup->nShotNorth ;
			}
			else
			{
				head.dShotStation =pSegYGroup->nShotEast;
			}  
			
			// The reciever station:
			if(bUseNorth)
			{
				head.dRcvStation =pSegYGroup->nRcvNorth ;
			}
			else
			{
				head.dRcvStation =pSegYGroup->nRcvEast;
			}

			// the cmp station:
			head.dCMPStation =(head.dShotStation +head.dRcvStation )/2;
			
			head.dShotEast =pSegYGroup->nShotEast ;
			head.dShotNorth=pSegYGroup->nShotNorth ;
			head.dRcvEast =pSegYGroup->nRcvEast ;
			head.dRcvNorth=pSegYGroup->nRcvNorth ;

			fwrite(&head,sizeof(CMyGroupHead),1,fp);

			nTotalGroupNumber++;

		}
		if(bFailed)break;
	}

	fclose(fp);
	dlgPro.DestroyWindow ();

	if(!bFailed){
		if(!GetShotRcvStation(pSvSysDoc, sTempFile,nTotalGroupNumber))return false;
		if(!GetShotRcvRelation(pSvSysDoc, sTempFile,nTotalGroupNumber))return false;
	}

	return !bFailed;
}

bool CCMPAnnealDoc::GetShotRcvStation(CSvSysDoc *pSvSysDoc,CString sGroupFile,long nTotalGroupNumber)
{

	//////////////////////////////////////////////
	// 获得炮点的参数:
	//////////////////////////////////////////////
	CMyGroupHead head;
	bool beThere;
	long i,j;
	long nShotLimit=5000;
	CMyGroupHead *pShotGroup=new CMyGroupHead[nShotLimit];
	
	FILE *fp=fopen(sGroupFile,"rb");	
	long nShotStationNumber=0;
	for(i=0;i<nTotalGroupNumber;i++){

		if(nShotStationNumber>=nShotLimit){
			TRACEERROR("Error: too many shots, what's error?"+vtos(nShotLimit));
			break;
		}

		fread(&head,sizeof(CMyGroupHead),1,fp);

		beThere=false;
		for(j=0;j<nShotStationNumber;j++){
			if(pShotGroup[j].dShotStation ==head.dShotStation ){
				beThere=true;
				break;
			}
		}
		if(!beThere){
			pShotGroup[nShotStationNumber]=head;
			nShotStationNumber++;
		}
	}
	fclose(fp);

	pSvSysDoc->SetShotNumber (nShotStationNumber);
	for(i=0;i<nShotStationNumber;i++){
		pSvSysDoc->AddShotRecord (pShotGroup[i].dShotStation ,pShotGroup[i].dShotNorth ,pShotGroup[i].dShotEast);
	}

	delete []pShotGroup;

	////////////////////////////////////////////
	// Get the Rcv parameters:
	////////////////////////////////////////////
	long nRcvLimit=20000;
	CMyGroupHead *pRcvGroup=new CMyGroupHead[nRcvLimit];
	
	fp=fopen(sGroupFile,"rb");	
	long nRcvStationNumber=0;
	for(i=0;i<nTotalGroupNumber;i++){

		if(nRcvStationNumber>=nRcvLimit){
			TRACEERROR("Error: too many Rcvs, what's error?"+vtos(nRcvLimit));
			break;
		}

		fread(&head,sizeof(CMyGroupHead),1,fp);

		beThere=false;
		for(j=0;j<nRcvStationNumber;j++){
			if(pRcvGroup[j].dRcvStation ==head.dRcvStation ){
				beThere=true;
				break;
			}
		}
		if(!beThere){
			pRcvGroup[nRcvStationNumber]=head;
			nRcvStationNumber++;
		}
	}
	fclose(fp);

	pSvSysDoc->SetRcvNumber (nRcvStationNumber);
	for(i=0;i<nRcvStationNumber;i++){
		pSvSysDoc->AddRcvRecord (pRcvGroup[i].dRcvStation ,pRcvGroup[i].dRcvNorth ,pRcvGroup[i].dRcvEast);
	}

	delete []pRcvGroup;


	return true;
}

bool CCMPAnnealDoc::GetShotRcvRelation(CSvSysDoc *pSvSysDoc, CString sGroupMsgFile,long nTotalGroupNumber)
{	
	
	
	long nShotNumber=pSvSysDoc->GetShotNumber();
	CShotPhyPar *pShotPar=pSvSysDoc->GetShotPar ();

	if(nShotNumber==0||!pShotPar)return false;

	
	////////////////////////////////////////////////
	//	Create an temp relation file:
	CString sTempRelFile="c:\\temprel.$$$";
	FILE *fpRel=fopen(sTempRelFile,"wt");
	if(!fpRel){
		AfxMessageBox("Error: can not create a temporary file for shot-reciever relation:"+sTempRelFile);
		return false;
	}
	fprintf(fpRel,"<RELATION  ShotStation ShotFileNumber GrpOrderStart GrpOrderEnd GrpOrderInc GrpStationStart GrpStationEnd GrpStationInc>\n");

	long nRcvLimit=10000;	
	double *pRcvStation=new double [nRcvLimit];
	long nRcvNumber=0;
	bool bFailed=false;
	long nFileNumber,nOrderStart,nOrderEnd,nOrderInc;
	double dShotStation,nStationStart,nStationEnd,nInc,nIncLast;

	CProgressDlg dlgPro;
	dlgPro.Create();
	dlgPro.SetStatus("Shots have been calculated:");
	dlgPro.SetRange (0,nShotNumber);

	FILE *fp=NULL;
	
	CMyGroupHead head;
	long i,j,k;
	bool beThere;
	for(i=0;i<nShotNumber;i++){

		dlgPro.SetPos(i);
		
		/////////////////////////////////
		// 获得该炮点的所有检波点桩号:
		dShotStation=pShotPar[i].Ph ;
		bFailed=false;
		nRcvNumber=0;

		fp=fopen(sGroupMsgFile,"rb");

		for(j=0;j<nTotalGroupNumber;j++){
			fread(&head,sizeof(CMyGroupHead),1,fp);

			if(head.dShotStation==dShotStation){
				if(nRcvNumber>=nRcvLimit){
					AfxMessageBox("Error: too many recievers in one shot, Shot Station:"+vtos(dShotStation));
					bFailed=true;
					break;
				}
				
				beThere=false;
				for(k=0;k<nRcvNumber;k++){
					if(head.dRcvStation ==pRcvStation[k]){
						beThere=true;
						break;
					}
				}
				if(!beThere){
					pRcvStation[nRcvNumber]=head.dRcvStation ;
					nFileNumber=head.nFileNumber;
					nRcvNumber++;
				}
			}
		}

		fclose(fp);	

		if(bFailed)break;

		//////////////////////////////////
		// 对柬波点桩号排序:
		Sort(pRcvStation,nRcvNumber);

		/////////////////////////////////////////////
		// 把桩号系列变成CRange:
		nIncLast=pRcvStation[1]-pRcvStation[0];
		nStationStart=pRcvStation[0];
		nOrderStart=1;
		nOrderInc=1;

		for(j=2;j<nRcvNumber ;j++){
			nInc=pRcvStation[j]-pRcvStation[j-1];
			if(nInc!=nIncLast){
				nStationEnd=pRcvStation[j-1];
				nOrderEnd=j;
				
				if(nIncLast==0){
					if(nStationStart==nStationEnd){
						nOrderInc=100;
						nIncLast=100;
					}
					else{
						AfxMessageBox("Error: when calculating relation from CMP data, the group station increment is zero!");
						bFailed=true;
						break;
					}
				}

				fprintf(fpRel,"%ld %1.1lf %ld %ld %ld %ld %1.0lf %1.0lf %1.0lf \n",
						i,
						dShotStation,
						nFileNumber,
						nOrderStart,
						nOrderEnd,
						nOrderInc,
						nStationStart,
						nStationEnd,
						nIncLast);
		
				nIncLast=pRcvStation[j+1]-pRcvStation[j];


				nStationStart=pRcvStation[j];
				nOrderStart=j+1;  // 1 is the base , NOT 0;
				j=j+1;
			}
		}
		if(bFailed)break;

		nStationEnd=pRcvStation[nRcvNumber -1];
		nOrderEnd=nRcvNumber ;

		if(nIncLast<=0){
			if(nStationStart==nStationEnd){
				nOrderInc=100;
				nIncLast=100;
			}
		}


		fprintf(fpRel,"%ld %lf %ld %ld %ld %ld %1.0lf %1.0lf %1.0lf \n",
				i,
				dShotStation,
				nFileNumber,
				nOrderStart,
				nOrderEnd,
				nOrderInc,
				nStationStart,
				nStationEnd,
				nIncLast);
	}

	fprintf(fpRel,"<====== SHOT_RECIEVER RELATIONS OVER ======>\n");
	fclose(fpRel);

	dlgPro.DestroyWindow();

	////////////////////////////////////////////////////
	//
	if(!bFailed){
		if(!pSvSysDoc->ReadRelPar (sTempRelFile)){
			AfxMessageBox("Error: when reading the temporary file!");
			bFailed=true;
		}
		if(!pSvSysDoc->Over ()){
			AfxMessageBox("Error: when analysing the survey system!");
			bFailed=true;
		}

		pSvSysDoc->SetModifiedFlag ();
		pSvSysDoc->UpdateAllViews (NULL);
	}

	delete []pRcvStation;

	return (!bFailed);
}


bool CCMPAnnealDoc::IsPowerInc(double &dTemperature,
						 bool &bReplaceIfInc,
						 long nRelatedPoint,
						 double dStation,
						 long nStaticOld,
						 long nStaticNew,
						 bool bShot,
						 bool bSeedMethod)
{	
	CSegYGroup *pSegYGroup,*pSegYGroupLast;

	
	float p,a;
	bool bInc;

	/////////////////////////////////////////////
	// Calc the new power:
	double dPowerTest1=0.0;
	double dPowerTest2=0.0;
	double dPowerTest;

	if(m_nTargetFunction==CORRELATION){
		pSegYGroup=m_pStackSectionTest->GetGroup(0);
		for(i=0;i<m_nCMPNumber;i++){
			pSegYGroupLast=pSegYGroup;
			pSegYGroup=m_pStackSectionTest->GetGroup (i);
			for(j=m_nStartPoint;j<m_nEndPoint;j++){
				dPowerTest1+=pSegYGroupLast->data[j]*pSegYGroup->data[j];
			}
		} 
		dPowerTest=dPowerTest1;
	}
	else {		
		// Calc every cmp 's power:
		for(i=0;i<m_nCMPNumber;i++){
			pSegYGroup=m_pStackSectionTest->GetGroup (i);
			for(j=m_nStartPoint;j<m_nEndPoint;j++){				
				dPowerTest2+=pSegYGroup->data[j]*pSegYGroup->data[j];
			}
		} 

		dPowerTest=dPowerTest2;
	}

	////////////////////////////////////////////////////////
	// 对能量进行均衡:
	//dPowerTest=dPowerTest2+dPowerTest1*0.01;
	dPowerTest/=(m_nCMPNumber-1)*(m_nEndPoint-m_nStartPoint);
	
	//////////////////////////////////////////////////////////////
	// 罚函数:
	static double dMinusPower;
	static double dPowerBak;
	static int nValid,nTotal;
	dPowerBak=dPowerTest;
	
	
	
/*	// The shot:
	dMinusPower=0.0;
	long nMid=m_pSvSysDoc->m_nShotPhyNumber/2;
	for(i=0;i<m_pSvSysDoc->m_nShotPhyNumber;i++){
		dMinusPower+=(m_pSvSysDoc->m_pShotPhyPar[i].st*m_pSvSysDoc->m_pShotPhyPar[i].st)*fabs(i-nMid)/nMid;
	}

	dMinusPower/=m_pSvSysDoc->m_nShotPhyNumber;
	dPowerTest-=dPowerBak*dMinusPower/1000000;
	
	// The reciever:
	dMinusPower=0.0;
	nMid=m_pSvSysDoc->m_nRcvPhyNumber/2;
	for(i=0;i<m_pSvSysDoc->m_nRcvPhyNumber;i++){
		dMinusPower+=(m_pSvSysDoc->m_pRcvPhyPar[i].rt*m_pSvSysDoc->m_pRcvPhyPar[i].rt)*fabs(i-nMid)/nMid;
	}

	dMinusPower/=m_pSvSysDoc->m_nRcvPhyNumber;
	dPowerTest-=dPowerBak*dMinusPower/1000000;
	
  	*/
	// 简单法函数:
	dPowerTest-=dPowerTest/m_nMaxStatic*nStaticNew/10000;

	/*
	// 考虑炮点
	static nStatic1,nStatic2,dMinus;
	nStatic1=m_pSvSysDoc->m_pShotPhyPar[0].st;
	nStatic2=m_pSvSysDoc->m_pShotPhyPar[m_pSvSysDoc->m_nShotPhyNumber-1].st;
	dMinus=nStatic2-nStatic1;
	dPowerTest-=dPowerTest*(dMinus*dMinus)/100000000.0; // 考虑静校正量的单斜零空间问题;

	// 考虑检波点
	nStatic1=m_pSvSysDoc->m_pRcvPhyPar[0].rt;
	nStatic2=m_pSvSysDoc->m_pRcvPhyPar[m_pSvSysDoc->m_nRcvPhyNumber-1].rt;
	dMinus=nStatic2-nStatic1;
	dPowerTest-=dPowerTest*(dMinus*dMinus)/100000000.0; // 考虑静校正量的单斜零空间问题;
	*/

	/////////////////////////////////////////////
	// Check if the power increased:
	bInc=false;
	if(dPowerTest>m_dPowerCurr){
		bInc=true;		
	}
	else if (dTemperature>0){
		p=exp((dPowerTest-m_dPowerCurr)/(m_dPowerCurr*dTemperature));
		a=rand()/(double)RAND_MAX;
		if(a<p){
			bInc=true;
		}
	}

	// If the power increased, Replace the current section with the new test section:
	if(bInc&&bReplaceIfInc){		
		for(i=0;i<m_nCMPNumber ;i+=1){
			memcpy(m_pStackSectionCurr->GetGroup(i),
				m_pStackSectionTest->GetGroup (i),
				m_CMPIndexFile.m_head.nGroupMaxPointNumber*sizeof(float));
		}	
		m_dPowerCurr=dPowerTest;
	}
		
	// or else replace the test data with the current data for later copying to current data:	
	if(!bInc){		
		for(i=0;i<m_nCMPNumber  ;i+=1){
			memcpy(m_pStackSectionTest->GetGroup(i),
				m_pStackSectionCurr->GetGroup (i),
				m_CMPIndexFile.m_head.nGroupMaxPointNumber*sizeof(float));
		}
	}

	// return:
	return bInc;
}


bool CCMPAnnealDoc::AllocateMem()
{
	CCMPIndexHead &head=m_CMPIndexFile.m_head ;

	m_pStackSectionCurr->SetPar(head.nCMPNumber,head.nGroupMaxPointNumber,head.nSampleInterval);
	m_pStackSectionTest->SetPar(head.nCMPNumber,head.nGroupMaxPointNumber,head.nSampleInterval);

	return true;
}

//////////////////////////////////////////////////////
// NOTE: 
// 1.The values in the static document 
//   has been devided by nSampleInterval
bool CCMPAnnealDoc::DoStack(CSegYMultiGroup *pSection,CStaticDoc *pStaticDoc,CSegYMultiGroup *pSectionTest,bool bShowProgress)
{
	//
	if(!m_fpCMPData){
		AfxMessageBox("CCMPAnnealDoc::ReStack Report Error:There is NOT CMP file opening !");
		return false;
	}

	// Stack the groups:
	long nGroupMaxPointNumber=m_CMPIndexFile.m_head.nGroupMaxPointNumber ;
	if(!pSection->SetPar (m_nCMPNumber,nGroupMaxPointNumber,m_CMPIndexFile.m_head .nSampleInterval )){
		TRACEERROR("叠加错误：设置叠加道数时候出错："+vtos(m_nCMPNumber));
		return false;
	}
	

	CCMPGroupMsg *pGroupIndex;
	CSegYGroup *pSegYGroup=NULL,*pSegYGroupSection=NULL;

	CProgressDlg dlgPro;
	if(bShowProgress){
		dlgPro.Create();	
		dlgPro.SetStatus("Stacking....");
		dlgPro.SetRange(0,m_nCMPNumber);
	}

	long nShotStatic=0,nRcvStatic=0,nMovePoint=0;
	long nGroup;
	bool bFailed=false;
	long nGroupBodySize= m_CMPIndexFile.m_head.nGroupMaxPointNumber*sizeof(float);

	CSegYGroup g;
	pSegYGroup=&g;

	
	for(i=0;i<m_nCMPNumber;i++){ // the CMP group number
		if(bShowProgress){
			dlgPro.SetPos(i);
		}

		pSegYGroupSection= pSection->GetGroup (i);
		if(!pSegYGroupSection){
			bFailed=true;
			break;
		}
		memset(pSegYGroupSection->data,0,nGroupBodySize);

		pGroupIndex=m_CMPIndexFile.GetCMPMsg (i,nGroup);				

		for(j=0;j<nGroup;j++){
			if(pStaticDoc){
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
					AfxMessageBox("Error: can not find the reciever station in the Static Document:"+vtos(pGroupIndex[j].dRcvPh ));
					bFailed=true;
					break;
				}
				nRcvStatic=pStaticDoc->m_Data[n].rt ;

				// Total move point:
				nMovePoint=nShotStatic+nRcvStatic; 
			}

			// Get the group:
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

		// Set the data in to the test section:
		if(pSectionTest)pSectionTest->SetGroup(i,pSegYGroupSection);

		
	}	

	pSection->SetBalanced(false);
	if(pSectionTest)pSectionTest->SetBalanced(false);

	if(bShowProgress){
		dlgPro.DestroyWindow ();
	}

	// Calc the power:
	m_dPowerCurr=pSection->CalcPower(m_nTargetFunction,m_nStartPoint,m_nEndPoint);

	//
	return !bFailed;					 

}

void CCMPAnnealDoc::OnCMPPlusShotStatic() 
{
	// TODO: Add your command handler code here
	PlusStatic(0);
	
}

void CCMPAnnealDoc::OnCMPPlusRcvStatic() 
{
	PlusStatic(1);
}


void CCMPAnnealDoc::OnCMPPlusStatic() 
{
	// TODO: Add your command handler code here
	PlusStatic(2);
}


bool CCMPAnnealDoc::PlusStatic(int nPlusStatic)  // 0: shot static, 1: rcv static, 2: both
{
	// TODO: Add your command handler code here
	if(!m_fpCMPData)return false;

	// Open a static document:
	CFileDialog dlg(true);
	dlg.m_ofn.lpstrFilter="Static Data file(*.T)\0*.t";
	dlg.m_ofn.lpstrTitle="Open ";
	dlg.m_ofn.lpstrDefExt=".T";

	
	if(dlg.DoModal()==IDCANCEL)return false;
	CString sStatic=dlg.GetPathName();

	CFdataApp *pApp=(CFdataApp*)AfxGetApp();
	CStaticDoc *pStaticDoc=(CStaticDoc *)(pApp->m_pStaticDataTemplate ->OpenDocumentFile (sStatic,false));
	if(!pStaticDoc)return false;

	CCMPIndexHead &head=m_CMPIndexFile.m_head ;
	CProgressDlg dlg2;
	dlg2.Create();
	dlg2.SetRange(0,head.nCMPNumber );
	dlg2.SetStatus("Plusing static value into the group heads...");

	//
	long i,j;
	long nGroup=0,nPh,n;
	CSegYGroup *pSegYGroup=NULL;
	bool bFailed=false;
	for(i=0;i<head.nCMPNumber ;i++){
		
		dlg2.SetPos(i);

		if(dlg2.CheckCancelButton())break;
		
		// Get one CMP 's all groups:
		CCMPGroupMsg *pCMPMsg=m_CMPIndexFile.GetCMPMsg (i,nGroup);

		for(j=0;j<nGroup;j++){
			pSegYGroup=GetGroup(i,j);
			
			// Get the shot static:
			if(nPlusStatic==0||nPlusStatic==2){
				nPh=pCMPMsg[j].dShotPh ;
				n=pStaticDoc->SearchStation (nPh);
				if(n==-1){
					AfxMessageBox("CCMPAnnealDoc::OnCMPPlusStatic() error: can not find station in the static document:"+vtos(nPh));
				}
				else{
					pSegYGroup->nShotStatic =pStaticDoc->m_Data [n].st ;
				}
			}

			// Get the rcv static:
			if(nPlusStatic==1||nPlusStatic==2){
				nPh=pCMPMsg[j].dRcvPh;
				n=pStaticDoc->SearchStation (nPh);
				if(n==-1){
					AfxMessageBox("CCMPAnnealDoc::OnCMPPlusStatic() error: can not find station in the static document:"+vtos(nPh));
				}
				else{
					pSegYGroup->nRcvStatic =pStaticDoc->m_Data [n].rt ;
				}
			}

			if(!SaveGroup(i,j,pSegYGroup)){
				bFailed=true;
				AfxMessageBox("Error: can not save the group:"+vtos(j)+" of CMP :"+vtos(i));
				break;
			}
		}
		if(bFailed)break;		
	}

	pStaticDoc->OnCloseDocument ();

	if(!bFailed)UpdateAllViews(NULL);

	dlg2.DestroyWindow ();

	return true;	

}


BOOL CCMPAnnealDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	if (!CCMPBaseDoc::OnOpenDocument(lpszPathName))
		return FALSE;

	if(m_pCMPStateChanged){
		delete []m_pCMPStateChanged;
		m_pCMPStateChanged=NULL;
	}
	m_pCMPStateChanged=new bool[m_nCMPNumber+10];
	
	return TRUE;
}

BOOL CCMPAnnealDoc::DecideUseNorth()
{
	long nCMPQty=GetCMPNumber();
	long nGroupQty=0;
	CSegYGroup *pGroup=NULL;
	int nRcvNorthPre=0;
	int nRcvEastPre=0;
	int nRcvNorth=0;
	int nRcvEast=0;

	pGroup=GetGroup(0,0);
	nRcvNorthPre = pGroup->nRcvNorth ;
	nRcvEastPre  = pGroup->nRcvEast  ;

	long i,j;
	int nUseEast  = 0;
	int nUseNorth = 0;
	
	long nTotalGroupRead=0;
	for( i=0;i<nCMPQty;i++)
	{
		nGroupQty=GetGroupNumberInCMP(i);
		for(j=0;j<nGroupQty;j++)
		{
			pGroup=GetGroup(i,j);
			nRcvEast  = pGroup->nRcvEast ;
			nRcvNorth = pGroup->nRcvNorth ;

			if(nRcvEast!=nRcvEastPre)
			{
				nUseEast++;
			}

			if(nRcvNorth!=nRcvNorthPre)
			{
				nUseNorth++;
			}

			nRcvEastPre=nRcvEast;
			nRcvNorthPre=nRcvNorth;

			nTotalGroupRead++;
		}

		if(nTotalGroupRead>100)
		{
			break;
		}
	}

	return (nUseNorth>nUseEast);
}
	