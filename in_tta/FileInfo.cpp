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
, m_bID3v1_save(FALSE)
, m_sFileFormat(_T(""))
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
	DDX_Control(pDX, IDC_ID3V1_GENRE, m_ID3v1_Genre);
	DDX_Control(pDX, IDC_ID3V1_SAVE, m_ID3v1_Check);
	DDX_Check(pDX, IDC_ID3V1_SAVE, m_bID3v1_save);
	DDX_Control(pDX, IDC_ID3V1_TRACKNO, m_ID3v1_TrackNo);
	DDX_Control(pDX, IDC_ID3V1_NAME, m_ID3v1_Name);
	DDX_Control(pDX, IDC_ID3V1_ARTISTS, m_ID3v1_Artist);
	DDX_Control(pDX, IDC_ID3V1_ALBUM, m_ID3v1_Album);
	DDX_Control(pDX, IDC_ID3V1_COMMENT, m_ID3v1_Comment);
	DDX_Control(pDX, IDC_ID3V1_YEAR, m_ID3v1_Year);
	DDX_Text(pDX, IDC_FILEFORMAT, m_sFileFormat);
}


BEGIN_MESSAGE_MAP(CFileInfo, CDialog)
	ON_BN_CLICKED(IDOK, &CFileInfo::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CFileInfo::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_ID3V1_SAVE, &CFileInfo::OnBnClickedId3v1Save)
END_MESSAGE_MAP()


// CFileInfo message handlers

void CFileInfo::OnBnClickedOk()
{
	char buf[MAX_PATHLEN];
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	if (m_bID3v1_save)
	{
		dlgtag.id3v1.SetTitle(m_sID3v1_Name.GetBuffer());
		dlgtag.id3v1.SetArtist(m_sID3v1_Artists.GetBuffer());
		dlgtag.id3v1.SetAlbum(m_sID3v1_Album.GetBuffer());
		dlgtag.id3v1.SetComment(m_sID3v1_Comment.GetBuffer());
		dlgtag.id3v1.SetYear(m_sID3v1_Year.GetBuffer());
		if(m_sID3v1_TrackNo != "")
			dlgtag.id3v1.SetTrack((unsigned char)atoi((LPCTSTR)m_sID3v1_TrackNo));
		else
			dlgtag.id3v1.SetTrack(NULL);

		dlgtag.id3v1.SetGenre((unsigned char)m_ID3v1_Genre.GetCurSel());
		dlgtag.id3v1.SaveTag(NULL);
	} 
	else
	{
		if(dlgtag.id3v1.hasTag())
			dlgtag.id3v1.DeleteTag(NULL);
	}


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

	int nCount;
	for(int i = 0; i < GENRES; i++)
	{
		nCount = m_ID3v1_Genre.AddString(genre[i]);
		if (nCount == CB_ERR)
			break;
	}

	char buf[10];

	dlgtag.ReadTag(NULL, (LPCTSTR)m_sFileName);


	int Lengthbysec = dlgtag.GetLengthbymsec() / 1000;
	int hour = Lengthbysec / 3600;
	int min  = Lengthbysec / 60;
	int sec  = Lengthbysec % 60;

	CString second;
	if(hour > 0)
		second.Format("%d:%d:%d (%dsec)", hour, min, sec, Lengthbysec);
	else if(min > 0)
		second.Format("%d:%d (%dsec)", min, sec, Lengthbysec);
	else
		second.Format("%d", sec);

	m_sFileFormat.Format(IDS_TTAFORMAT,
		dlgtag.GetFormat(), dlgtag.GetByteSize() * 8, dlgtag.GetSampleRate(),
		dlgtag.GetBitrate(),dlgtag.GetNumberofChannel(),
		(dlgtag.GetNumberofChannel() == 2) ? "Stereo" : "Monoral",
		dlgtag.GetLengthbyFrame(), second);

	if(dlgtag.id3v1.hasTag()){
		m_bID3v1_save = TRUE;
		ShowHideID3v1Column();
		m_sID3v1_Name = dlgtag.id3v1.GetTitle();
		m_sID3v1_Artists = dlgtag.id3v1.GetArtist();
		m_sID3v1_Album = dlgtag.id3v1.GetAlbum();
		m_sID3v1_Comment = dlgtag.id3v1.GetComment();
		m_sID3v1_Year = dlgtag.id3v1.GetYear();
		_ultoa_s((unsigned long)dlgtag.id3v1.GetTrack(), buf, sizeof(buf), 10);
		m_sID3v1_TrackNo = buf;
		m_ID3v1_Genre.SetCurSel(dlgtag.id3v1.GetGenre());
	}
	else
	{
		m_bID3v1_save = FALSE;
		ShowHideID3v1Column();
	}


	UpdateData(FALSE);
	return TRUE;
}



void CFileInfo::ShowHideID3v1Column()
{
	if(m_bID3v1_save)
	{
		m_ID3v1_Album.EnableWindow(true);
		m_ID3v1_Artist.EnableWindow(true);
		m_ID3v1_Comment.EnableWindow(true);
		m_ID3v1_Genre.EnableWindow(true);
		m_ID3v1_Name.EnableWindow(true);
		m_ID3v1_TrackNo.EnableWindow(true);
		m_ID3v1_Year.EnableWindow(true);
	}
	else
	{
		m_ID3v1_Album.EnableWindow(false);
		m_ID3v1_Artist.EnableWindow(false);
		m_ID3v1_Comment.EnableWindow(false);
		m_ID3v1_Genre.EnableWindow(false);
		m_ID3v1_Name.EnableWindow(false);
		m_ID3v1_TrackNo.EnableWindow(false);
		m_ID3v1_Year.EnableWindow(false);
	}
	return;
}

void CFileInfo::OnBnClickedId3v1Save()
{
	// TODO: Add your control notification handler code here
	if(!m_bID3v1_save)
	{
		m_bID3v1_save = TRUE;
		ShowHideID3v1Column();
	}
	else
	{
		UpdateData(TRUE);
		m_bID3v1_save = FALSE;
		ShowHideID3v1Column();
	}

}
