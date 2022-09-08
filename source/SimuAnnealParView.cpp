// SimuAneallParView.cpp : implementation file
//

#include "stdafx.h"
#include "fdata.h"
#include "SimuAnnealParView.h"
#include "ProgDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSimuAnnealParView

IMPLEMENT_DYNCREATE(CSimuAnnealParView, CFHEditView)

CSimuAnnealParView::CSimuAnnealParView()
{
	m_nColNumber=3;
}

CSimuAnnealParView::~CSimuAnnealParView()
{
}


BEGIN_MESSAGE_MAP(CSimuAnnealParView, CFHEditView)
	//{{AFX_MSG_MAP(CSimuAnnealParView)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CSimuAnnealParView diagnostics

#ifdef _DEBUG
void CSimuAnnealParView::AssertValid() const
{
	CFHEditView::AssertValid();
}

void CSimuAnnealParView::Dump(CDumpContext& dc) const
{
	CFHEditView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CSimuAnnealParView message handlers

void CSimuAnnealParView::UpdateDoc()
{
   	CFHEditView::UpdateDoc ();
	CSimuAnnealParDoc *pDoc=GetDocument();

	// Set the data:
	long nLoopOrder;
	float fTemperature;
	for(long i=0;i<pDoc->GetRecordNumber ();i++){
		GetItem(i+1,1,&nLoopOrder);
		GetItem(i+1,2,&fTemperature);
		pDoc->SetPar(i,nLoopOrder,fTemperature);
	}

}

void CSimuAnnealParView::UpdateGrid()
{
	// TODO: Add your specialized code here and/or call the base class
	CFHEditView::UpdateGrid ();

	CSimuAnnealParDoc *pDoc=GetDocument();

	SetItem(0,0,"Order");
	SetItem(0,1,"Loop Order");
	SetItem(0,2,"Temperature");
  
	CProgressDlg dlg;
	dlg.Create();
	dlg.SetRange(0,pDoc->GetParNumber());
	dlg.SetStatus("Loading Simu Parameters...");

	CSimuPar *pPar;
	CString s;
	for(int i=0;i<pDoc->GetParNumber() ;i++){
		dlg.SetPos(i);
		pPar = pDoc->GetPar (i);

		SetItem(i+1,1,pPar->nLoopOrder );
		s.Format("%1.7lf",pPar->fTemperature );
		SetItem(i+1,2,s);
	}

	dlg.DestroyWindow();

	m_Grid.AutoSize ();
}

CSimuAnnealParDoc * CSimuAnnealParView::GetDocument()
{
	ASSERT_VALID(m_pDocument);
	return (CSimuAnnealParDoc * )m_pDocument;
}
