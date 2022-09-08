// SimuAneallParDoc.cpp : implementation file
//

#include "stdafx.h"
#include "global.h"
#include "fdata.h"
#include "SimuAnnealParDoc.h"
#include "FHZoomViewGlobal.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSimuAnnealParDoc

IMPLEMENT_DYNCREATE(CSimuAnnealParDoc, CFHEditDoc)

CSimuAnnealParDoc::CSimuAnnealParDoc()
{
	m_pSimuPar=NULL;
	m_nRecordNumber=0;
}

BOOL CSimuAnnealParDoc::OnNewDocument()
{
	if (!CFHEditDoc::OnNewDocument())
		return FALSE;
	return TRUE;
}

CSimuAnnealParDoc::~CSimuAnnealParDoc()
{
	if(m_pSimuPar){
		delete []m_pSimuPar;
		m_pSimuPar=NULL;
	}
}


BEGIN_MESSAGE_MAP(CSimuAnnealParDoc, CFHEditDoc)
	//{{AFX_MSG_MAP(CSimuAnnealParDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSimuAnnealParDoc diagnostics

#ifdef _DEBUG
void CSimuAnnealParDoc::AssertValid() const
{
	CFHEditDoc::AssertValid();
}

void CSimuAnnealParDoc::Dump(CDumpContext& dc) const
{
	CFHEditDoc::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CSimuAnnealParDoc serialization

void CSimuAnnealParDoc::Serialize(CArchive& ar)
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
// CSimuAnnealParDoc commands

bool CSimuAnnealParDoc::SetRecordNumber(long nSimuParNumber)
{
	if(nSimuParNumber<1||nSimuParNumber>10000){
		AfxMessageBox("Error: the parameter number is out of range :"+vtos(nSimuParNumber));
		return false;
	}

	if(m_pSimuPar){
		delete []m_pSimuPar;
		m_pSimuPar=NULL;
	}

	m_pSimuPar=new CSimuPar[nSimuParNumber];
	m_nRecordNumber=nSimuParNumber;

	return true;
}

bool CSimuAnnealParDoc::SetPar(long nPlace, long nLoopOrder, float fTemperature)
{
	if(nPlace<0||nPlace>=m_nRecordNumber){
		AfxMessageBox("Error: the parameter can not be set to place :"+vtos(nPlace));
		return false;
	}

	m_pSimuPar[nPlace].nLoopOrder =nLoopOrder;
	m_pSimuPar[nPlace].fTemperature =fTemperature;

	return true;
}

CSimuPar * CSimuAnnealParDoc::GetPar(long nPlace)
{
	if(nPlace<0||nPlace>m_nRecordNumber){
		AfxMessageBox("Error: can not get par at place :"+vtos(nPlace));
		return NULL;
	}

	return &m_pSimuPar[nPlace];
}

long CSimuAnnealParDoc::GetParNumber()
{
	return m_nRecordNumber;
}

BOOL CSimuAnnealParDoc::OnSaveDocument(LPCTSTR lpszPathName) 
{
	FILE *fp=fopen(lpszPathName,"wt");
	if(!fp){
		AfxMessageBox("Error: can not open the file "+CString(lpszPathName));
		return false;
	}

	for(long i=0;i<m_nRecordNumber;i++){		
		fprintf(fp,"%ld %1.8lf\n",m_pSimuPar[i].nLoopOrder,m_pSimuPar[i].fTemperature);
	}

	fclose(fp);

	SetModifiedFlag(false);

	return true;
}

BOOL CSimuAnnealParDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	// Get the par number:
	FILE *fp=fopen(lpszPathName,"rt");
	if(!fp){
		AfxMessageBox("Error: can not open the file "+CString(lpszPathName));
		return false;
	}

	long nOrder,n;
	double dT;

	m_nRecordNumber=0;
	while(!feof(fp)){
		n=fscanf(fp,"%ld %lf\n",&nOrder,&dT);
		if(n<2)break;
		m_nRecordNumber++;
	}
	fclose(fp);

	SetRecordNumber(m_nRecordNumber);

	// Get the data:
	fp=fopen(lpszPathName,"rt");
	for(long i=0;i<m_nRecordNumber;i++){		
		fscanf(fp,"%ld %lf\n",&m_pSimuPar[i].nLoopOrder,&m_pSimuPar[i].fTemperature);
	}
	fclose(fp);

	return true;
}


double CSimuAnnealParDoc::GetTemperature(long nLoop)
{
	CSimuPar *pPar,*pParLast;
	pPar=GetPar(0);
	if(!pPar){
		return 0.0;
	}
	else if(nLoop<pPar->nLoopOrder ){
		return 1.0;
	}
	else{
		for(long j=1;j<GetParNumber();j++){
			pParLast=pPar;
			pPar=GetPar(j);
			if(nLoop>=pParLast->nLoopOrder &&nLoop<=pPar->nLoopOrder ){
				double kk=(pPar->fTemperature -pParLast->fTemperature )/(pPar->nLoopOrder -pParLast->nLoopOrder );
				return pParLast->fTemperature+(nLoop-pParLast->nLoopOrder )*kk;
			}
		}
		return 0.0;
	}
}

CString CSimuAnnealParDoc::GetFileForOpen()
{
	CFileDialog dlg(true);
	dlg.m_ofn.lpstrFilter="Simulate Annealing Parameter File(*.sap)\0*.sap";
	dlg.m_ofn.lpstrTitle="Open";
	dlg.m_ofn.lpstrDefExt=".sap";
	
	if(dlg.DoModal()==IDCANCEL)return CString("");
	return dlg.GetPathName();
}
CString CSimuAnnealParDoc::GetFileForSave()
{
	CFileDialog dlg(false);
	dlg.m_ofn.lpstrFilter="Simulate Annealing Parameter File(*.sap)\0*.sap";
	dlg.m_ofn.lpstrTitle="Save";
	dlg.m_ofn.lpstrDefExt=".sap";
	
	if(dlg.DoModal()==IDCANCEL)return CString("");
	return dlg.GetPathName();
}
