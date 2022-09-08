// CMPView.cpp : implementation file
//

#include "stdafx.h"
#include "Global.h"
#include "fdata.h"
#include "CMPAnnealDoc.h"
#include "CMPAnnealView.h"
#include "ProgDlg.h"
#include "FHZoomViewGlobal.h"
#include "FHZoomView.h"
#include "DlgAskOneValue.h"
#include "Mainfrm.h"
#include "DlgDrawCMPByStation.h"
#include ".\cmpannealview.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCMPAnnealView

IMPLEMENT_DYNCREATE(CCMPAnnealView, CCMPBaseView)

CCMPAnnealView::CCMPAnnealView()
{
	m_bShowWithStatic=false;

	m_nSampleInterval=4;
}

CCMPAnnealView::~CCMPAnnealView()
{
}


BEGIN_MESSAGE_MAP(CCMPAnnealView, CCMPBaseView)
	//{{AFX_MSG_MAP(CCMPAnnealView)
	ON_COMMAND(ID_CMP_ShowWithStatic, OnCMPShowWithStatic)
	ON_UPDATE_COMMAND_UI(ID_CMP_ShowWithStatic, OnUpdateCMPShowWithStatic)
	ON_WM_SETFOCUS()
	ON_COMMAND(ID_CMP_DrawOutMicroCMP, OnCMPDrawOutMicroCMP)
	//}}AFX_MSG_MAP
//	ON_WM_CONTEXTMENU()
ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCMPAnnealView drawing


void CCMPAnnealView::OnDraw(CDC* pDC)
{
	CCMPBaseView::OnDraw(pDC);
}


	
/////////////////////////////////////////////////////////////////////////////
// CCMPAnnealView diagnostics

#ifdef _DEBUG
void CCMPAnnealView::AssertValid() const
{
	CCMPBaseView::AssertValid();
}

void CCMPAnnealView::Dump(CDumpContext& dc) const
{
	CCMPBaseView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CCMPAnnealView message handlers

CCMPAnnealDoc* CCMPAnnealView::GetDocument()
{
	ASSERT_VALID(m_pDocument);
	return (CCMPAnnealDoc*)m_pDocument;
}


CString CCMPAnnealView::GetInfo(double x, double y)
{

	if(!GetGroupInfo(x,y))return CString("Empty Area");

	CCMPAnnealDoc *pDoc=GetDocument();

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



void CCMPAnnealView::OnCMPShowWithStatic() 
{
	// TODO: Add your command handler code here
	m_bShowWithStatic=!m_bShowWithStatic;
	DrawBitmap();
	Invalidate();
	
}

void CCMPAnnealView::OnUpdateCMPShowWithStatic(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck (m_bShowWithStatic);
	
}


void CCMPAnnealView::OnSetFocus(CWnd* pOldWnd) 
{
	CCMPBaseView::OnSetFocus(pOldWnd);
}

CSegYGroup * CCMPAnnealView::GetGroup(long nOrder)
{
	CCMPAnnealDoc *pDoc=GetDocument();

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

void CCMPAnnealView::OnCMPDrawOutMicroCMP() 
{
	CDlgDrawCMPByStation dlg;
	if(dlg.DoModal ()==IDCANCEL)return;

	// TODO: Add your command handler code here
	CCMPAnnealDoc *pDoc=GetDocument();
	  
	pDoc->DrawOutPartCMPByStation (dlg.m_sOutCMPFile ,
		CRange(dlg.m_dShotStart ,dlg.m_dShotEnd ),
		CRange(dlg.m_dRcvStart ,dlg.m_dRcvEnd ));
}


void CCMPAnnealView::OnContextMenu(CWnd *pWnd, CPoint point)
{
	// TODO: Add your message handler code here
	CInfoView::OnContextMenu (pWnd,point);
}
