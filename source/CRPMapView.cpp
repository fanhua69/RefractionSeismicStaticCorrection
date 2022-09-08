// CRPMapView.cpp : implementation file
//

#include "stdafx.h"
#include "fdata.h"
#include "CRPMapView.h"
#include "DlgAskOneValue.h"
#include "ProgDlg.h"
#include "global.h"
#include "FHZoomViewGlobal.h"
#include "mainfrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCRPMapView

IMPLEMENT_DYNCREATE(CCRPMapView, CInfoView)

CCRPMapView::CCRPMapView()
{
	m_nCurrentCRPOrder=0;
	m_nGroupBodyPointNumber=0;
	m_nSampleInterval=4;

	m_bDrawWithStatic=false;

	CMainFrame *pFrame=(CMainFrame *)AfxGetMainWnd();
	this->SetToolBar(&pFrame->m_FHZoomViewToolBar );
}

CCRPMapView::~CCRPMapView()
{
}


BEGIN_MESSAGE_MAP(CCRPMapView, CInfoView)
	//{{AFX_MSG_MAP(CCRPMapView)
	ON_COMMAND(ID_CRP_ShowWithStatic, OnCRPShowWithStatic)
	ON_UPDATE_COMMAND_UI(ID_CRP_ShowWithStatic, OnUpdateCRPShowWithStatic)
	ON_UPDATE_COMMAND_UI(ID_FHZoomViewLast, OnUpdateFHZoomViewLast)
	ON_UPDATE_COMMAND_UI(ID_FHZoomViewNext, OnUpdateFHZoomViewNext)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCRPMapView drawing

void CCRPMapView::OnInitialUpdate()
{
	CCRPDoc *pDoc=GetDocument(); 
	
	// Get the maximum CRP point number in this CRP document:
	CCRPIndexHead &head=pDoc->m_CRPIndexDoc .m_indexHead ;
	
	m_nGroupBodyPointNumber=head.nGroupBodyPointNumber ;
	m_nSampleInterval=head.nSampleInterval ;

	//
	long n=0;
	while(true){
		if(pDoc->m_CRPIndexDoc .m_indexHead.index [n].nGroupNumber >0)break;
		n++;
	}
	if(n>=head.nCRPNumber )
		m_nCurrentCRPOrder=0;
	else
		m_nCurrentCRPOrder=n;

	m_nTimePointNumber=head.nGroupBodyPointNumber ;
	
	CInfoView::OnInitialUpdate();
}

void CCRPMapView::OnDraw(CDC* pDC)
{
	CInfoView::OnDraw (pDC);
}

/////////////////////////////////////////////////////////////////////////////
// CCRPMapView diagnostics

#ifdef _DEBUG
void CCRPMapView::AssertValid() const
{
	CInfoView::AssertValid();
}

void CCRPMapView::Dump(CDumpContext& dc) const
{
	CInfoView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CCRPMapView message handlers

CCRPDoc* CCRPMapView::GetDocument()
{
	ASSERT_VALID(m_pDocument);
	return (CCRPDoc*)m_pDocument; 
}

void CCRPMapView::GotoNextPage()
{
	CCRPDoc *pDoc=GetDocument();
	CCRPIndexHead &head=pDoc->m_CRPIndexDoc .m_indexHead ;

	m_nCurrentCRPOrder+=1;
	if(m_nCurrentCRPOrder>=head.nCRPNumber ){ 
		m_nCurrentCRPOrder=head.nCRPNumber -1;
		return;
	}

	DrawBitmap();
	Invalidate();
}

void CCRPMapView::GotoLastPage()
{
	CCRPDoc *pDoc=GetDocument();
	m_nCurrentCRPOrder-=1;
	if(m_nCurrentCRPOrder<0){
		m_nCurrentCRPOrder=0;
		return;
	}
		
	DrawBitmap();
	Invalidate();

}

CString CCRPMapView::GetInfo(double x, double y)
{
	CCRPDoc *pDoc=GetDocument();
	CCRPIndexHead &head=pDoc->m_CRPIndexDoc .m_indexHead ;
	
	CSegYGroup *pGroup=pDoc->GetGroup(m_nCurrentCRPOrder,0L);
	if(!pGroup){
		CString s;
		s.Format("Current CRP: %i, Total CRP: %i",m_nCurrentCRPOrder,head.nCRPNumber );
		return s;
	}

	long nRcvPh=pGroup->nRcvStation ;
	long nShotPh=pGroup->nShotStation ;
	
	CString s;
	s.Format("Current CRP: %i, Total CRP: %i,Rcv Station:%i, Shot Station: %i",m_nCurrentCRPOrder,head.nCRPNumber,nRcvPh,nShotPh);
	return s;
}

void CCRPMapView::SearchPage()
{
	CDlgAskOneValue dlg;
	dlg.m_sTitle ="检波点道集快速检索";
	dlg.m_sAsk="检波点序号:";
	dlg.m_sValue=vtos(m_nCurrentCRPOrder);

	CCRPDoc *pDoc=GetDocument();
	CCRPIndexHead &head=pDoc->m_CRPIndexDoc .m_indexHead ;

	if(dlg.DoModal ()==IDCANCEL)return;
	sscanf((LPCTSTR)dlg.m_sValue,"%ld",&m_nCurrentCRPOrder);
	if(m_nCurrentCRPOrder<0)m_nCurrentCRPOrder=0;
	if(m_nCurrentCRPOrder>=head.nCRPNumber)m_nCurrentCRPOrder=head.nCRPNumber-1;

	DrawBitmap();
	Invalidate();
	
}

void CCRPMapView::OnCRPShowWithStatic() 
{
	// TODO: Add your command handler code here
	m_bDrawWithStatic=!m_bDrawWithStatic;
	DrawBitmap();
	Invalidate();
	
}

void CCRPMapView::OnUpdateCRPShowWithStatic(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck (m_bDrawWithStatic);	
}


CSegYGroup* CCRPMapView::GetGroup(long nOrder)
{
	CCRPDoc *pDoc=GetDocument();

	CSegYGroup *pSegYGroup=pDoc->GetGroup (m_nCurrentCRPOrder,nOrder,false);
	if(!pSegYGroup)return NULL;

	//////////////////////////////////////////////////////
	// Apply the static value:
	if(m_bDrawWithStatic){
		int st;
		st=-pSegYGroup->nShotStatic /m_nSampleInterval;
		 
		if(st>0){
			for(long j=0;j<m_nGroupBodyPointNumber -st;j++){
				pSegYGroup->data [j]=pSegYGroup->data [j+st];
			}
			memset(&(pSegYGroup->data [m_nGroupBodyPointNumber -st]),0,(st-1)*sizeof(float));
		}
		
		else if(st<0){
			for(long j=m_nGroupBodyPointNumber-1;j>(-st);j--){
				pSegYGroup->data [j]=pSegYGroup->data [j+st];
			}
			memset(&pSegYGroup->data [0],0,(-st+1)*sizeof(float));
		}  
	}

	return 	pSegYGroup;

}

void CCRPMapView::OnUpdateFHZoomViewLast(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable (m_nCurrentCRPOrder>0);
}

void CCRPMapView::OnUpdateFHZoomViewNext(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	CCRPDoc *pDoc=GetDocument();
	CCRPIndexHead &head=pDoc->m_CRPIndexDoc .m_indexHead ;
	pCmdUI->Enable (m_nCurrentCRPOrder<head.nCRPNumber -1);
}

long CCRPMapView::GetGroupNumber()
{
	CCRPDoc *pDoc=GetDocument();
	if(m_nCurrentCRPOrder<0||m_nCurrentCRPOrder>=pDoc->GetCRPNumber ())return 0;
	return pDoc->GetGroupNumberInCRP(m_nCurrentCRPOrder);
}
