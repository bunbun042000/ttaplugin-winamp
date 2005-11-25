// FileInfo.cpp : implementation file
//

#include "stdafx.h"
#include "in_tta.h"
#include "FileInfo.h"
//#include "Resource.h"

// CFileInfo dialog

IMPLEMENT_DYNAMIC(CFileInfo, CDialog)

CFileInfo::CFileInfo(CWnd* pParent /*=NULL*/, const char *filename)
: CDialog(CFileInfo::IDD, pParent)
, m_sFileName(_T(""))
, m_sID3v1_Name(_T(""))
, m_sID3v1_Artists(_T(""))
, m_sID3v1_Album(_T(""))
, m_sID3v1_Comment(_T(""))
, m_sID3v1_Year(_T(""))
, m_sID3v1_TrackNo(_T(""))
{
//	AFX_MANAGE_STATE(AfxGetStaticModuleHandle());
	m_sFileName = filename;
}

CFileInfo::~CFileInfo()
{
//	AFX_MANAGE_STATE(AfxGetStaticModuleHandle());
}

void CFileInfo::DoDataExchange(CDataExchange* pDX)
{
	//	AFX_MANAGE_STATE(AfxGetStaticModuleHandle());
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_FILENAME, m_sFileName);
	DDX_Text(pDX, IDC_ID3V1_NAME, m_sID3v1_Name);
	DDX_Text(pDX, IDC_ID3V1_ARTISTS, m_sID3v1_Artists);
	DDX_Text(pDX, IDC_ID3V1_ALBUM, m_sID3v1_Album);
	DDX_Text(pDX, IDC_ID3V1_COMMENT, m_sID3v1_Comment);
	DDX_Text(pDX, IDC_ID3V1_YEAR, m_sID3v1_Year);
	DDX_Text(pDX, IDC_ID3V1_TRACKNO, m_sID3v1_TrackNo);
}


BEGIN_MESSAGE_MAP(CFileInfo, CDialog)
	ON_BN_CLICKED(IDOK, &CFileInfo::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CFileInfo::OnBnClickedCancel)
END_MESSAGE_MAP()


// CFileInfo message handlers

void CFileInfo::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	OnOK();
}

void CFileInfo::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	OnCancel();
}


BOOL CFileInfo::OnInitDialog()
{
	CDialog::OnInitDialog();

	char buf[10];

	dlgtag.ReadTag(NULL, (LPCTSTR)m_sFileName);

	if(dlgtag.id3v1.hasTag()){
		m_sID3v1_Name = dlgtag.id3v1.GetTitle();
		m_sID3v1_Artists = dlgtag.id3v1.GetArtist();
		m_sID3v1_Album = dlgtag.id3v1.GetAlbum();
		m_sID3v1_Comment = dlgtag.id3v1.GetComment();
		m_sID3v1_Year = dlgtag.id3v1.GetYear();
		_ultoa_s((unsigned long)dlgtag.id3v1.GetTrack(), buf, sizeof(buf), 10);
		m_sID3v1_TrackNo = buf;
	}

	UpdateData(FALSE);
	return TRUE;
}