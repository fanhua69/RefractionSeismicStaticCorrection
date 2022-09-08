// CMPIndexView.cpp : implementation file
//

#include "stdafx.h"
#include "fdata.h"
#include "CMPIndexView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCMPIndexView

IMPLEMENT_DYNCREATE(CCMPIndexView, CScrollView)

CCMPIndexView::CCMPIndexView()
{
}

CCMPIndexView::~CCMPIndexView()
{
}


BEGIN_MESSAGE_MAP(CCMPIndexView, CScrollView)
	//{{AFX_MSG_MAP(CCMPIndexView)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCMPIndexView drawing

void CCMPIndexView::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();

	CSize sizeTotal;
	// TODO: calculate the total size of this view
	sizeTotal.cx = sizeTotal.cy = 100;
	SetScrollSizes(MM_TEXT, sizeTotal);
}

void CCMPIndexView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// CCMPIndexView diagnostics

#ifdef _DEBUG
void CCMPIndexView::AssertValid() const
{
	CScrollView::AssertValid();
}

void CCMPIndexView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CCMPIndexView message handlers
