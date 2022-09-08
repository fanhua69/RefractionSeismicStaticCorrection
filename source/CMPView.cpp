// CMPView.cpp : implementation file
//

#include "stdafx.h"
#include "Global.h"
#include "fdata.h"
#include "CMPDoc.h"
#include "CMPView.h"
#include "ProgDlg.h"
#include "FHZoomViewGlobal.h"
#include "FHZoomView.h"
#include "DlgAskOneValue.h"
#include "Mainfrm.h"
#include "DlgDrawCMPByStation.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCMPView

IMPLEMENT_DYNCREATE(CCMPView, CInfoView)

CCMPView::CCMPView()
{
	m_nCurrentCMPOrder=0;
	m_nGatherQty=0;
	m_bShowWithStatic=false;

	m_nSampleInterval=4;

	CMainFrame *pFrame=(CMainFrame *)AfxGetMainWnd();
	this->SetToolBar(&pFrame->m_FHZoomViewToolBar );
}

CCMPView::~CCMPView()
{
}


BEGIN_MESSAGE_MAP(CCMPView, CInfoView)
	//{{AFX_MSG_MAP(CCMPView)
	ON_COMMAND(ID_CMP_ShowWithStatic, OnCMPShowWithStatic)
	ON_UPDATE_COMMAND_UI(ID_CMP_ShowWithStatic, OnUpdateCMPShowWithStatic)
	ON_COMMAND(ID_FHZoomViewNext, OnFHZoomViewNext)
	ON_UPDATE_COMMAND_UI(ID_FHZoomViewNext, OnUpdateFHZoomViewNext)
	ON_COMMAND(ID_FHZoomViewLast, OnFHZoomViewLast)
	ON_UPDATE_COMMAND_UI(ID_FHZoomViewLast, OnUpdateFHZoomViewLast)
	ON_WM_SETFOCUS()
	ON_COMMAND(ID_CMP_DrawOutMicroCMP, OnCMPDrawOutMicroCMP)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCMPView drawing

void CCMPView::OnInitialUpdate()
{
	CCMPDoc *pDoc=GetDocument();	
	m_nCurrentCMPOrder=0;
	
	CCMPIndexHead &head=pDoc->m_CMPIndexDoc .head ;
	m_nTimePointNumber=head.nGroupMaxPointNumber ;
	m_nSampleInterval=head.nSampleInterval ;

	CInfoView::OnInitialUpdate();
}

void CCMPView::OnDraw(CDC* pDC)
{
	// TODO: add draw code here
	CDrawGroup drawer;
	drawer.SetPar(pDC,5,2,3,3,RGB(0,0,0));
	   /*
	CSegYGroup g;
	FILE *fp=fopen("d:\\1.dat","rb");
	fread(&g,sizeof(CSegYGroup),1,fp);
	drawer.DrawGroup (&g,100,100);
	fread(&g,sizeof(CSegYGroup),1,fp);
	drawer.DrawGroup (&g,200,100);
	fclose(fp);


	CSegYGroup a;
	memset(&a,0,sizeof(CSegYGroup));
	fp=fopen("d:\\co.dat","rb");
	long n=fread(&a,sizeof(CSegYGroup),1,fp);
	drawer.DrawGroup (&a,300,100);

	n=fread(&a,sizeof(CSegYGroup),1,fp);
	drawer.DrawGroup (&a,400,100);
	fclose(fp);
	*/		  
	
	CInfoView::OnDraw(pDC);
}


	
/////////////////////////////////////////////////////////////////////////////
// CCMPView diagnostics

#ifdef _DEBUG
void CCMPView::AssertValid() const
{
	CInfoView::AssertValid();
}

void CCMPView::Dump(CDumpContext& dc) const
{
	CInfoView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CCMPView message handlers

CCMPDoc* CCMPView::GetDocument()
{
	ASSERT_VALID(m_pDocument);
	return (CCMPDoc*)m_pDocument;
}

void CCMPView::OnFHZoomViewNext()
{
	CCMPDoc *pDoc=GetDocument();
	m_nCurrentCMPOrder+=1;
	if(m_nCurrentCMPOrder>=pDoc->m_CMPIndexDoc .head .nCMPNumber ){
		m_nCurrentCMPOrder=pDoc->m_CMPIndexDoc .head .nCMPNumber -1;
	}
	else{
		DrawBitmap();
		Invalidate();
	}
}

void CCMPView::OnFHZoomViewLast()
{
	CCMPDoc *pDoc=GetDocument();
	m_nCurrentCMPOrder-=1;
	if(m_nCurrentCMPOrder<0){
		m_nCurrentCMPOrder=0;
	}
	else{
		DrawBitmap();
		Invalidate();
	}
}

CString CCMPView::GetInfo(double x, double y)
{

	if(!GetGroupInfo(x,y))return CString("Empty Area");

	CCMPDoc *pDoc=GetDocument();

	long nGroup=x;
	long nTimePoint=y;
	
	CSegYGroup * pGroup=pDoc->GetGroup(m_nCurrentCMPOrder,nGroup);
	if(!pGroup)return CString("Empty Area");


	long nFileNumber=pGroup->nShotFileNumber ;
	int  nGroupNumber=pGroup->nGroupOrder ;
	double dCMPNorth=pGroup->nCMPNorth;
	double dCMPEast=pGroup->nCMPEast;
	float fNoiseAmp=pGroup->dNoiseSignalRatio /1000.0;
	long nShotStation=pGroup->nShotStation ;
	long nRcvStation=pGroup->nRcvStation ;

	CString s;
	s.Format(" \r\n"
        "CMP:%ld\r\n"
		"Group: %ld\r\n"
		"Shot Station: %ld\r\n"
		"Rcv Station: %ld\r\n"
		"Time Point: %ld\r\n"
		"File Number: %i\r\n"
		"Group Number: %i\r\n"
		"CMP East: %1.0lf\r\n"
		"CMP North: %1.0lf\r\n"
		"Shot Static: %d\r\n"
		"Rcv Static: %d\r\n"
		"Sample Interval: %d\r\n"
		"Noise Amp:%1.2f",		  

		m_nCurrentCMPOrder,
		nGroup,
		nShotStation ,
		nRcvStation ,
		nTimePoint,
		nFileNumber,
		nGroupNumber,
		dCMPEast,
		dCMPNorth,
		pGroup->nShotStatic,
		pGroup->nRcvStatic,
		pGroup->nGroupSampleInterval,
		fNoiseAmp);	

	return s;
}



void CCMPView::OnCMPShowWithStatic() 
{
	// TODO: Add your command handler code here
	m_bShowWithStatic=!m_bShowWithStatic;
	DrawBitmap();
	Invalidate();
	
}

void CCMPView::OnUpdateCMPShowWithStatic(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck (m_bShowWithStatic);
	
}

void CCMPView::OnUpdateFHZoomViewNext(CCmdUI* pCmdUI) 
{
	CCMPDoc *pDoc=GetDocument();
	CCMPIndexHead &head=pDoc->m_CMPIndexDoc .head ;
	pCmdUI->Enable (m_nCurrentCMPOrder<head.nCMPNumber-1 );
}

void CCMPView::OnUpdateFHZoomViewLast(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable (m_nCurrentCMPOrder>0);
}


void CCMPView::SearchPage()
{
	CDlgAskOneValue dlg;
	dlg.m_sTitle ="CMP道集的查找";
	dlg.m_sAsk ="CMP道集序号:";
	dlg.m_sValue =vtos(m_nCurrentCMPOrder);
	if(dlg.DoModal ()==IDCANCEL)return;

	sscanf((LPCTSTR)dlg.m_sValue ,"%ld",&m_nCurrentCMPOrder);

	CCMPDoc *pDoc=GetDocument();

	long nMaxCMP=pDoc->GetGatherQty ();
	if(m_nCurrentCMPOrder<0||m_nCurrentCMPOrder>=nMaxCMP){
		m_nCurrentCMPOrder=nMaxCMP-1;
	}		
	
	DrawBitmap();
	Invalidate();	
}



void CCMPView::OnSetFocus(CWnd* pOldWnd) 
{
	CInfoView::OnSetFocus(pOldWnd);
}

CSegYGroup * CCMPView::GetGroup(long nOrder)
{
	CCMPDoc *pDoc=GetDocument();

	CSegYGroup *pSegYGroup=NULL;
	pSegYGroup=pDoc->GetGroup (m_nCurrentCMPOrder,nOrder,false);
	if(!pSegYGroup)return NULL;
		
	// Plus static on the group:
	// Get the move point number:
	if(m_bShowWithStatic){
		int nStatic,st,rt;
		long nPoint=pSegYGroup->nGroupSamplePoint ;
		st=pSegYGroup->nShotStatic ;
		rt=pSegYGroup->nRcvStatic ;
		nStatic=-(st+rt)/m_nSampleInterval;

		// Do the static correction:
		if(nStatic>0){
			for(long j=0;j<nPoint-nStatic;j++){
				pSegYGroup->data [j]=pSegYGroup->data[j+nStatic];
			}
			for(j=nPoint-nStatic;j<nPoint;j++){
				pSegYGroup->data [j]=0;
			}

		}
		else if(nStatic<0){
			for(long j=nPoint-1;j>-nStatic;j--){
				pSegYGroup->data [j]=pSegYGroup->data[j+nStatic];
			}
			for(j=-nStatic;j>=0;j--){
				pSegYGroup->data [j]=0;
			}
		}
	}

	return pSegYGroup;
}

long CCMPView::GetGroupNumber()
{
	CCMPDoc *pDoc=GetDocument();
	return pDoc->GetGroupNumberInGather (m_nCurrentCMPOrder);	
}

void CCMPView::OnCMPDrawOutMicroCMP() 
{
	CDlgDrawCMPByStation dlg;
	if(dlg.DoModal ()==IDCANCEL)return;

	// TODO: Add your command handler code here
	CCMPDoc *pDoc=GetDocument();
	  
	pDoc->DrawOutPartCMPByStation (dlg.m_sOutCMPFile ,
		CRange(dlg.m_dShotStart ,dlg.m_dShotEnd ),
		CRange(dlg.m_dRcvStart ,dlg.m_dRcvEnd ));
}
