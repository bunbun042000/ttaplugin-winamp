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
, m_bID3v2_save(FALSE)
, m_sID3v2_Title(_T(""))
, m_sID3v2_Artists(_T(""))
, m_sID3v2_Album(_T(""))
, m_sID3v2_Year(_T(""))
, m_sID3v2_Genre(_T(""))
, m_sID3v2_Comment(_T(""))
, m_sID3v2_TrackNo(_T(""))
, m_sID3v2_Copyrights(_T(""))
, m_sID3v2_URI(_T(""))
, m_sID3v2_Words(_T(""))
, m_sID3v2_Composers(_T(""))
, m_sID3v2_Arrangements(_T(""))
, m_sID3v2_Original_Artists(_T(""))
, m_sID3v2_Encoding_Engineer(_T(""))
, m_bID3v2_UnSynchronization(FALSE)
{
//	AFX_MANAGE_STATE(AfxGetStaticModuleHandle());
	m_sFileName = filename;
	::InitializeCriticalSection(&CriticalSection);
}

CFileInfo::~CFileInfo()
{
//	AFX_MANAGE_STATE(AfxGetStaticModuleHandle());
	::DeleteCriticalSection(&CriticalSection);
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
	DDX_Check(pDX, IDC_ID3V2_SAVE, m_bID3v2_save);
	DDX_Control(pDX, IDC_ID3V2_TITLE, m_ID3v2_Title);
	DDX_Text(pDX, IDC_ID3V2_TITLE, m_sID3v2_Title);
	DDX_Control(pDX, IDC_ID3V2_ARTISTS, m_ID3v2_Artists);
	DDX_Text(pDX, IDC_ID3V2_ARTISTS, m_sID3v2_Artists);
	DDX_Control(pDX, IDC_ID3V2_ALBUM, m_ID3v2_Album);
	DDX_Text(pDX, IDC_ID3V2_ALBUM, m_sID3v2_Album);
	DDX_Text(pDX, IDC_ID3V2_YEAR, m_sID3v2_Year);
	DDX_Control(pDX, IDC_ID3V2_YEAR, m_ID3v2_Year);
	DDX_Control(pDX, IDC_ID3V2_GENRE, m_ID3v2_Genre);
	DDX_Text(pDX, IDC_ID3V2_GENRE, m_sID3v2_Genre);
	DDX_Control(pDX, IDC_ID3V2_COMMENT, m_ID3v2_Comment);
	DDX_Text(pDX, IDC_ID3V2_COMMENT, m_sID3v2_Comment);
	DDX_Control(pDX, IDC_ID3V2_TRACKNO, m_ID3v2_TrackNo);
	DDX_Text(pDX, IDC_ID3V2_TRACKNO, m_sID3v2_TrackNo);
	DDX_Control(pDX, IDC_ID3V2_COPYRIGHTS, m_ID3v2_Copyrights);
	DDX_Text(pDX, IDC_ID3V2_COPYRIGHTS, m_sID3v2_Copyrights);
	DDX_Control(pDX, IDC_ID3V2_URI, m_ID3v2_URI);
	DDX_Text(pDX, IDC_ID3V2_URI, m_sID3v2_URI);
	DDX_Text(pDX, IDC_ID3V2_WORDS, m_sID3v2_Words);
	DDX_Control(pDX, IDC_ID3V2_WORDS, m_ID3v2_Words);
	DDX_Control(pDX, IDC_ID3V2_COMPOSERS, m_ID3v2_Composers);
	DDX_Text(pDX, IDC_ID3V2_COMPOSERS, m_sID3v2_Composers);
	DDX_Control(pDX, IDC_ID3V2_ARRANGEMENTS, m_ID3v2_Arrangements);
	DDX_Text(pDX, IDC_ID3V2_ARRANGEMENTS, m_sID3v2_Arrangements);
	DDX_Control(pDX, IDC_ID3V2_ORIGINAL_ARTISTS, m_ID3v2_Original_Artists);
	DDX_Text(pDX, IDC_ID3V2_ORIGINAL_ARTISTS, m_sID3v2_Original_Artists);
	DDX_Control(pDX, IDC_ID3V2_ENCODING_ENGINEER, m_ID3v2_Encoding_Engineer);
	DDX_Text(pDX, IDC_ID3V2_ENCODING_ENGINEER, m_sID3v2_Encoding_Engineer);
	DDX_Control(pDX, IDC_ID3V2_VERSION, m_ID3v2_Version);
	DDX_Control(pDX, IDC_ID3V2_STRING_ENCODING, m_ID3v2_String_Encoding);
	DDX_Control(pDX, IDC_COPYFROMV2, m_ID3v1_CopyFromV2);
	DDX_Control(pDX, IDC_COPYFROMV1, m_ID3v2_CopyFromV1);
	DDX_Control(pDX, IDC_ID3V2_SYNCHRONIZATIOn, m_ID3v2_UnSynchronization);
	DDX_Check(pDX, IDC_ID3V2_SYNCHRONIZATIOn, m_bID3v2_UnSynchronization);
}


BEGIN_MESSAGE_MAP(CFileInfo, CDialog)
	ON_BN_CLICKED(IDOK, &CFileInfo::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CFileInfo::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_ID3V1_SAVE, &CFileInfo::OnBnClickedId3v1Save)
	ON_BN_CLICKED(IDC_ID3V2_SAVE, &CFileInfo::OnBnClickedId3v2Save)
	ON_BN_CLICKED(IDC_COPYFROMV1, &CFileInfo::OnBnClickedCopyfromv1)
	ON_CBN_SELCHANGE(IDC_ID3V2_VERSION, &CFileInfo::OnCbnSelchangeId3v2Version)
	ON_BN_CLICKED(IDC_COPYFROMV2, &CFileInfo::OnBnClickedCopyfromv2)
END_MESSAGE_MAP()


// CFileInfo message handlers

void CFileInfo::OnBnClickedOk()
{
	::EnterCriticalSection(&CriticalSection);
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	if (m_bID3v1_save) {
		dlgtag.id3v1.SetTitle(m_sID3v1_Name.GetBuffer());
		dlgtag.id3v1.SetArtist(m_sID3v1_Artists.GetBuffer());
		dlgtag.id3v1.SetAlbum(m_sID3v1_Album.GetBuffer());
		dlgtag.id3v1.SetComment(m_sID3v1_Comment.GetBuffer());
		dlgtag.id3v1.SetYear(m_sID3v1_Year.GetBuffer());
		if(m_sID3v1_TrackNo != "")
			dlgtag.id3v1.SetTrack((unsigned char)atoi((LPCTSTR)m_sID3v1_TrackNo));
		else
			dlgtag.id3v1.SetTrack(0xff);

		dlgtag.id3v1.SetGenre(GetID3v1Genre());
		dlgtag.id3v1.SaveTag(NULL);
	} else {
		if(dlgtag.id3v1.hasTag())
			if(AfxMessageBox(IDS_ID3V1DELETE, MB_OKCANCEL, 0) == IDOK)
				dlgtag.id3v1.DeleteTag(NULL);
			else
				return;
	}

	if (m_bID3v2_save) {
		dlgtag.id3v2.SetUnSynchronization(m_bID3v2_UnSynchronization);
		dlgtag.id3v2.SetEncoding((unsigned char)m_ID3v2_String_Encoding.GetCurSel());
		dlgtag.id3v2.SetVersion((unsigned char)m_ID3v2_Version.GetCurSel());
		dlgtag.id3v2.SetArtist(m_sID3v2_Artists);
		dlgtag.id3v2.SetTitle(m_sID3v2_Title);
		dlgtag.id3v2.SetAlbum(m_sID3v2_Album);
		dlgtag.id3v2.SetTrackNo(m_sID3v2_TrackNo);
		dlgtag.id3v2.SetYear(m_sID3v2_Year);
		dlgtag.id3v2.SetGenre(m_sID3v2_Genre);
		dlgtag.id3v2.SetComment(m_sID3v2_Comment);
		dlgtag.id3v2.SetCopyright(m_sID3v2_Copyrights);
		dlgtag.id3v2.SetURI(m_sID3v2_URI);
		dlgtag.id3v2.SetWords(m_sID3v2_Words);
		dlgtag.id3v2.SetComposers(m_sID3v2_Composers);
		dlgtag.id3v2.SetArrangements(m_sID3v2_Arrangements);
		dlgtag.id3v2.SetOrigArtist(m_sID3v2_Original_Artists);
		dlgtag.id3v2.SetEncEngineer(m_sID3v2_Encoding_Engineer);
		dlgtag.id3v2.SaveTag();
	} else {
		if(dlgtag.id3v2.hasTag()){
			if(AfxMessageBox(IDS_ID3V2DELETE, MB_OKCANCEL, 0) == IDOK)
				dlgtag.id3v2.DeleteTag(m_sFileName);
			else
				return;
		}
	}

	::LeaveCriticalSection(&CriticalSection);
	OnOK();
}

void CFileInfo::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	OnCancel();
}


BOOL CFileInfo::OnInitDialog()
{
	::EnterCriticalSection(&CriticalSection);
	CDialog::OnInitDialog();

	int nCount;
	__int32 i = 0;
	for(i = 0; i < GENRES; i++)	{
		nCount = m_ID3v1_Genre.AddString(genre[i]);
		if (nCount == CB_ERR)
			break;
	}

	char buf[10];

	dlgtag.ReadTag(NULL, (LPCTSTR)m_sFileName);


	// File Information (TTA)
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

	// ID3v1 Tag
	if(dlgtag.id3v1.hasTag()){
		m_bID3v1_save = TRUE;
		ShowHideID3v1Column();
		m_sID3v1_Name = dlgtag.id3v1.GetTitle();
		m_sID3v1_Artists = dlgtag.id3v1.GetArtist();
		m_sID3v1_Album = dlgtag.id3v1.GetAlbum();
		m_sID3v1_Comment = dlgtag.id3v1.GetComment();
		m_sID3v1_Year = dlgtag.id3v1.GetYear();
		if(dlgtag.id3v1.GetTrack() != 0xff) {
			_ultoa_s((unsigned long)dlgtag.id3v1.GetTrack(), buf, sizeof(buf), 10);
			m_sID3v1_TrackNo = buf;
		} else
			m_sID3v1_TrackNo = "";
		CString Genre(genre[dlgtag.id3v1.GetGenre()]);
		SetID3v1Genre(Genre);
	}
	else
	{
		m_bID3v1_save = FALSE;
		ShowHideID3v1Column();
	}

	// ID3v2 Tag
	i = 0;
	while(ID3v2Version[i].ver != NULL)
	{
		nCount = m_ID3v2_Version.AddString(ID3v2Version[i].ver);
		if (nCount == CB_ERR)
			break;
		i++;
	}

	if(dlgtag.id3v2.hasTag()){
		m_bID3v2_save = TRUE;
		ShowHideID3v2Column();
		m_sID3v2_Artists = dlgtag.id3v2.GetArtist();
		m_sID3v2_Title = dlgtag.id3v2.GetTitle();
		m_sID3v2_Album = dlgtag.id3v2.GetAlbum();
		m_sID3v2_TrackNo = dlgtag.id3v2.GetTrackNo();
		m_sID3v2_Year = dlgtag.id3v2.GetYear();
		m_sID3v2_Genre = dlgtag.id3v2.GetGenre();
		m_sID3v2_Comment = dlgtag.id3v2.GetComment();
		m_sID3v2_Copyrights = dlgtag.id3v2.GetCopyright();
		m_sID3v2_URI = dlgtag.id3v2.GetURI();
		m_sID3v2_Words = dlgtag.id3v2.GetWords();
		m_sID3v2_Composers = dlgtag.id3v2.GetComposers();
		m_sID3v2_Arrangements = dlgtag.id3v2.GetArrangements();
		m_sID3v2_Original_Artists = dlgtag.id3v2.GetOrigArtist();
		m_sID3v2_Encoding_Engineer = dlgtag.id3v2.GetEncEngineer();

		m_bID3v2_UnSynchronization = dlgtag.id3v2.GetUnSynchronization();
		SetVersionSpecificColumn();

	}
	else
	{
		m_bID3v2_save = FALSE;
		ShowHideID3v2Column();
	}


	UpdateData(FALSE);
	::LeaveCriticalSection(&CriticalSection);
	return TRUE;
}


void CFileInfo::SetVersionSpecificColumn()
{
	m_ID3v2_Version.SetCurSel((int)(dlgtag.id3v2.GetVersion() - ID3v2Version[0].flag));
	__int32 i = 0;
	__int32 nCount = 0;
	m_ID3v2_String_Encoding.ResetContent();
	while(ID3v2Version[dlgtag.id3v2.GetVersion() - ID3v2Version[0].flag].Str_Enc[i] != NULL) {
		nCount = m_ID3v2_String_Encoding.AddString(ID3v2Version[dlgtag.id3v2.GetVersion() 
			- ID3v2Version[0].flag].Str_Enc[i]);
		if (nCount == CB_ERR)
			break;
		i++;
	}
	m_ID3v2_String_Encoding.SetCurSel((int)(dlgtag.id3v2.GetEncoding()));
}

void CFileInfo::ShowHideID3v1Column()
{
	m_ID3v1_Album.EnableWindow(m_bID3v1_save);
	m_ID3v1_Artist.EnableWindow(m_bID3v1_save);
	m_ID3v1_Comment.EnableWindow(m_bID3v1_save);
	m_ID3v1_Genre.EnableWindow(m_bID3v1_save);
	m_ID3v1_Name.EnableWindow(m_bID3v1_save);
	m_ID3v1_TrackNo.EnableWindow(m_bID3v1_save);
	m_ID3v1_Year.EnableWindow(m_bID3v1_save);
	m_ID3v2_CopyFromV1.EnableWindow(m_bID3v1_save && m_bID3v2_save);
	m_ID3v1_CopyFromV2.EnableWindow(m_bID3v1_save && m_bID3v2_save);

	return;
}

void CFileInfo::OnBnClickedId3v1Save()
{
	// TODO: Add your control notification handler code here
	if(!m_bID3v1_save)
	{
		m_bID3v1_save = TRUE;
		ShowHideID3v1Column();
		if(m_ID3v1_Genre.GetCurSel() == CB_ERR) {
			CString Genre(genre[DEFAULT_GENRE]); // "Others"
			SetID3v1Genre(Genre);
		}
	}
	else
	{
		UpdateData(TRUE);
		m_bID3v1_save = FALSE;
		ShowHideID3v1Column();
	}

}

void CFileInfo::SetID3v1Genre(CString &Genre)
{
	int index;
	for (index = 0; index < m_ID3v1_Genre.GetCount(); index++) {
		CString current;
		m_ID3v1_Genre.GetLBText(index, current);
		if(current == Genre) {
			m_ID3v1_Genre.SetCurSel(index);
			return;
		}
	}
	for (index = 0; index < m_ID3v1_Genre.GetCount(); index++) {
		CString current;
		m_ID3v1_Genre.GetLBText(index, current);
		if(current == CString(genre[DEFAULT_GENRE]))  // "Others";
			break;
	}
	m_ID3v1_Genre.SetCurSel(index);
	return;
}

int CFileInfo::GetID3v1Genre()
{
	CString current;
	int index;
	index = m_ID3v1_Genre.GetCurSel();
	if (index == CB_ERR)
		return DEFAULT_GENRE;
	m_ID3v1_Genre.GetLBText(m_ID3v1_Genre.GetCurSel(), current);
	for (index = 0; index < m_ID3v1_Genre.GetCount(); index++) {
		if(current == CString(genre[index]))
			break;
	}
	if(index == m_ID3v1_Genre.GetCount())
		return index++;
	return index;
}

void CFileInfo::ShowHideID3v2Column()
{
	m_ID3v2_Album.EnableWindow(m_bID3v2_save);
	m_ID3v2_Artists.EnableWindow(m_bID3v2_save);
	m_ID3v2_Comment.EnableWindow(m_bID3v2_save);
	m_ID3v2_Genre.EnableWindow(m_bID3v2_save);
	m_ID3v2_Title.EnableWindow(m_bID3v2_save);
	m_ID3v2_TrackNo.EnableWindow(m_bID3v2_save);
	m_ID3v2_Year.EnableWindow(m_bID3v2_save);
	m_ID3v2_URI.EnableWindow(m_bID3v2_save);
	m_ID3v2_Words.EnableWindow(m_bID3v2_save);
	m_ID3v2_Composers.EnableWindow(m_bID3v2_save);
	m_ID3v2_Arrangements.EnableWindow(m_bID3v2_save);
	m_ID3v2_Copyrights.EnableWindow(m_bID3v2_save);
	m_ID3v2_Original_Artists.EnableWindow(m_bID3v2_save);
	m_ID3v2_Version.EnableWindow(m_bID3v2_save);
	m_ID3v2_Encoding_Engineer.EnableWindow(m_bID3v2_save);
	m_ID3v2_String_Encoding.EnableWindow(m_bID3v2_save);
	m_ID3v2_CopyFromV1.EnableWindow(m_bID3v1_save && m_bID3v2_save);
	m_ID3v1_CopyFromV2.EnableWindow(m_bID3v1_save && m_bID3v2_save);
	m_ID3v2_UnSynchronization.EnableWindow(m_bID3v2_save);

	return;
}
void CFileInfo::OnBnClickedId3v2Save()
{
	// TODO: Add your control notification handler code here
	if(!m_bID3v2_save)
	{
		m_bID3v2_save = TRUE;
		ShowHideID3v2Column();
		SetVersionSpecificColumn();
	}
	else
	{
		UpdateData(TRUE);
		m_bID3v2_save = FALSE;
		ShowHideID3v2Column();
	}

}

void CFileInfo::OnBnClickedCopyfromv1()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);

	m_sID3v2_Title = m_sID3v1_Name;
	m_sID3v2_Artists = m_sID3v1_Artists;
	m_sID3v2_Album = m_sID3v1_Album;
	m_sID3v2_Year = m_sID3v1_Year;
	m_sID3v2_Genre = genre[GetID3v1Genre()];
	m_sID3v2_Comment = m_sID3v1_Comment;
	m_sID3v2_TrackNo = 	m_sID3v1_TrackNo;

	UpdateData(FALSE);
}



void CFileInfo::OnCbnSelchangeId3v2Version()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	dlgtag.id3v2.SetVersion(ID3v2Version[m_ID3v2_Version.GetCurSel()].flag);
	dlgtag.id3v2.SetEncoding((unsigned char)m_ID3v2_String_Encoding.GetCurSel());
	__int32 i = 0;
	__int32 nCount = 0;
	m_ID3v2_String_Encoding.ResetContent();
	while(ID3v2Version[dlgtag.id3v2.GetVersion() - ID3v2Version[0].flag].Str_Enc[i] != NULL) {
		nCount = m_ID3v2_String_Encoding.AddString(ID3v2Version[dlgtag.id3v2.GetVersion() 
			- ID3v2Version[0].flag].Str_Enc[i]);
		if (nCount == CB_ERR)
			break;
		i++;
	}
	m_ID3v2_String_Encoding.SetCurSel((int)(dlgtag.id3v2.GetEncoding()));
}

void CFileInfo::OnBnClickedCopyfromv2()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);

	m_sID3v1_Name = m_sID3v2_Title;
	m_sID3v1_Artists = m_sID3v2_Artists;
	m_sID3v1_Album = m_sID3v2_Album;
	m_sID3v1_Year = m_sID3v2_Year;
	SetID3v1Genre(m_sID3v2_Genre);
	m_sID3v1_Comment = m_sID3v2_Comment;
	m_sID3v1_TrackNo = m_sID3v2_TrackNo;

	UpdateData(FALSE);
}
