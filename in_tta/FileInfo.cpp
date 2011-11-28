// FileInfo.cpp : implementation file
/*
The ttaplugin-winamp project.
Copyright (C) 2005-2010  bunbun04200

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "resource.h"
#include "FileInfo.h"

#include "common.h"

#include <taglib/id3v2tag.h>
#include <taglib/id3v1tag.h>
#include <taglib/id3v1genres.h>
#include <taglib/id3v2header.h>
#include <taglib/trueaudiofile.h>
#include <taglib/tstring.h>
#include <taglib/tfile.h>

// CFileInfo dialog

//IMPLEMENT_DYNAMIC(CFileInfo, CDialog)

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
//, m_bID3v2_UnSynchronization(FALSE)
{
//	AFX_MANAGE_STATE(AfxGetStaticModuleHandle());
	m_sFileName = filename;
//	fileinfo = new TagLib::TrueAudio::File(TagLib::FileName((LPCTSTR)m_sFileName));
	::InitializeCriticalSection(&CriticalSection);
}

CFileInfo::~CFileInfo()
{
//	AFX_MANAGE_STATE(AfxGetStaticModuleHandle());
	::DeleteCriticalSection(&CriticalSection);
	delete fileinfo;
}

void CFileInfo::DoDataExchange(CDataExchange* pDX)
{
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
//	DDX_Control(pDX, IDC_ID3V2_VERSION, m_ID3v2_Version);
//	DDX_Control(pDX, IDC_ID3V2_STRING_ENCODING, m_ID3v2_String_Encoding);
	DDX_Control(pDX, IDC_COPYFROMV2, m_ID3v1_CopyFromV2);
	DDX_Control(pDX, IDC_COPYFROMV1, m_ID3v2_CopyFromV1);
//	DDX_Control(pDX, IDC_ID3V2_SYNCHRONIZATIOn, m_ID3v2_UnSynchronization);
//	DDX_Check(pDX, IDC_ID3V2_SYNCHRONIZATIOn, m_bID3v2_UnSynchronization);
}


BEGIN_MESSAGE_MAP(CFileInfo, CDialog)
	ON_BN_CLICKED(IDOK, &CFileInfo::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CFileInfo::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_ID3V1_SAVE, &CFileInfo::OnBnClickedId3v1Save)
	ON_BN_CLICKED(IDC_ID3V2_SAVE, &CFileInfo::OnBnClickedId3v2Save)
	ON_BN_CLICKED(IDC_COPYFROMV1, &CFileInfo::OnBnClickedCopyfromv1)
//	ON_CBN_SELCHANGE(IDC_ID3V2_VERSION, &CFileInfo::OnCbnSelchangeId3v2Version)
	ON_BN_CLICKED(IDC_COPYFROMV2, &CFileInfo::OnBnClickedCopyfromv2)
END_MESSAGE_MAP()


// CFileInfo message handlers

void CFileInfo::OnBnClickedOk()
{
	::EnterCriticalSection(&CriticalSection);
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	if (m_bID3v1_save) {
		if (NULL == fileinfo->ID3v1Tag()) {
			fileinfo->ID3v1Tag(true);
		} else {
			// do nothing.
		}
		fileinfo->ID3v1Tag()->setTitle(m_sID3v1_Name.GetBuffer());
		fileinfo->ID3v1Tag()->setArtist(m_sID3v1_Artists.GetBuffer());
		fileinfo->ID3v1Tag()->setAlbum(m_sID3v1_Album.GetBuffer());
		fileinfo->ID3v1Tag()->setComment(m_sID3v1_Comment.GetBuffer());
		fileinfo->ID3v1Tag()->setYear((LPCTSTR)m_sID3v1_Year);
		fileinfo->ID3v1Tag()->setTrack((LPCTSTR)m_sID3v1_TrackNo);

		CString current;
		unsigned __int8 index;
		index = (unsigned __int8)m_ID3v1_Genre.GetCurSel();
		if (index == CB_ERR) {
			index =  DEFAULT_GENRE;
		}
		m_ID3v1_Genre.GetLBText(index, current);
		for (index = 0; index < m_ID3v1_Genre.GetCount(); index++) {
			if(current == TagLib::ID3v1::genre(index).toCString()) {
				break;
			}
		}
		fileinfo->ID3v1Tag()->setGenre((LPCTSTR)current);
		fileinfo->save();
	} else {
		if(NULL != fileinfo->ID3v1Tag()) {
			if(AfxMessageBox(IDS_ID3V1DELETE, MB_OKCANCEL, 0) == IDOK) {
				fileinfo->strip(TagLib::TrueAudio::File::ID3v1);
				fileinfo->save();
			} 
		} else {
			// do nothing
		}
	}

	if (m_bID3v2_save) {
//		fileinfo->ID3v2Tag()->header()->->unsynchronisation()n(m_bID3v2_UnSynchronization);
//		fileinfo->setID3v2FrameFactory(GetID3v2Encoding());
//		fileinfo->ID3v2Tag()->header()->setMajorVersion(m_ID3v2_Version.GetCurSel()+1);
		if (NULL == fileinfo->ID3v2Tag()) {
			fileinfo->ID3v2Tag(true);
		} else {
			// do nothing.
		}
		TagLib::String tempStr;
		tempStr  = TagLib::String(SetEncodingString(m_sID3v2_Artists), TagLib::String::UTF8);
		fileinfo->ID3v2Tag()->setArtist(tempStr);
		tempStr = TagLib::String(SetEncodingString(m_sID3v2_Title), TagLib::String::UTF8);
		fileinfo->ID3v2Tag()->setTitle(tempStr);
		tempStr = TagLib::String(SetEncodingString(m_sID3v2_Album), TagLib::String::UTF8);
		fileinfo->ID3v2Tag()->setAlbum(tempStr);
		tempStr = TagLib::String(SetEncodingString(m_sID3v2_TrackNo), TagLib::String::UTF8);
		fileinfo->ID3v2Tag()->setTrack(tempStr);
		tempStr = TagLib::String(SetEncodingString(m_sID3v2_Year), TagLib::String::UTF8);
		fileinfo->ID3v2Tag()->setYear(tempStr);
		tempStr = TagLib::String(SetEncodingString(m_sID3v2_Genre), TagLib::String::UTF8);
		fileinfo->ID3v2Tag()->setGenre(tempStr);
		tempStr = TagLib::String(SetEncodingString(m_sID3v2_Comment), TagLib::String::UTF8);
		fileinfo->ID3v2Tag()->setComment(tempStr);
		tempStr = TagLib::String(SetEncodingString(m_sID3v2_Copyrights), TagLib::String::UTF8);
		fileinfo->ID3v2Tag()->setCopyright(tempStr);
		tempStr = TagLib::String(SetEncodingString(m_sID3v2_URI), TagLib::String::UTF8);
		fileinfo->ID3v2Tag()->setURI(tempStr);
		tempStr = TagLib::String(SetEncodingString(m_sID3v2_Words), TagLib::String::UTF8);
		fileinfo->ID3v2Tag()->setWords(tempStr);
		tempStr = TagLib::String(SetEncodingString(m_sID3v2_Composers), TagLib::String::UTF8);
		fileinfo->ID3v2Tag()->setComposers(tempStr);
		tempStr = TagLib::String(SetEncodingString(m_sID3v2_Arrangements), TagLib::String::UTF8);
		fileinfo->ID3v2Tag()->setArrangements(tempStr);
		tempStr = TagLib::String(SetEncodingString(m_sID3v2_Original_Artists), TagLib::String::UTF8);
		fileinfo->ID3v2Tag()->setOrigArtist(tempStr);
		tempStr = TagLib::String(SetEncodingString(m_sID3v2_Encoding_Engineer), TagLib::String::UTF8);
		fileinfo->ID3v2Tag()->setEncEngineer(tempStr);

		fileinfo->save();
	} else {
		if(NULL != fileinfo->ID3v2Tag()){
			if(AfxMessageBox(IDS_ID3V2DELETE, MB_OKCANCEL, 0) == IDOK) {
				fileinfo->strip(TagLib::TrueAudio::File::ID3v2);
				fileinfo->save();
			} else {
				// do nothing
			}
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
	unsigned __int8 i = 0;
	for(i = 0; (TagLib::ID3v1::genre(i).length() != 0); i++) {
		nCount = m_ID3v1_Genre.AddString(TagLib::ID3v1::genre(i).toCString());
		if (nCount == CB_ERR)
			break;
	}

	fileinfo = new TagLib::TrueAudio::File((LPCTSTR)m_sFileName);

	// File Information (TTA)
	int Lengthbysec = fileinfo->audioProperties()->length();
	int hour = Lengthbysec / 3600;
	int min  = Lengthbysec / 60;
	int sec  = Lengthbysec % 60;

	CString second;
	if(hour > 0) {
		second.Format("%d:%02d:%02d (%dsec)", hour, min, sec, Lengthbysec);
	} else if(min > 0) {
		second.Format("%d:%02d (%dsec)", min, sec, Lengthbysec);
	} else {
		second.Format("%d", sec);
	}

	m_sFileFormat.Format(IDS_TTAFORMAT,
		fileinfo->audioProperties()->ttaVersion(), (int)fileinfo->audioProperties()->bitsPerSample(), 
		fileinfo->audioProperties()->sampleRate(),
		fileinfo->audioProperties()->bitrate(), 
		fileinfo->audioProperties()->channels(),
		(fileinfo->audioProperties()->channels() == 2) ? "Stereo" : "Monoral", second);

	// ID3v1 Tag
	if(NULL != fileinfo->ID3v1Tag()){
		m_bID3v1_save = TRUE;
		ShowHideID3v1Column();
		m_sID3v1_Name = fileinfo->ID3v1Tag()->title().toCString(false);
		m_sID3v1_Artists = fileinfo->ID3v1Tag()->artist().toCString(false);
		m_sID3v1_Album = fileinfo->ID3v1Tag()->album().toCString(false);
		m_sID3v1_Comment = fileinfo->ID3v1Tag()->comment().toCString(false);
		m_sID3v1_Year = fileinfo->ID3v1Tag()->year().toCString(false);
		m_sID3v1_TrackNo = fileinfo->ID3v1Tag()->track().toCString(false);
		SetID3v1Genre(fileinfo->ID3v1Tag()->genre().toCString(false));
	} else {
		m_bID3v1_save = FALSE;
		ShowHideID3v1Column();
	}

	// ID3v2 Tag
//	i = 0;
//	while(ID3v2Version[i].ver != NULL)
//	{
//		nCount = m_ID3v2_Version.AddString(ID3v2Version[i].ver);
//		if (nCount == CB_ERR)
//			break;
//		i++;
//	}

	if(NULL != fileinfo->ID3v2Tag()){
		if (!fileinfo->ID3v2Tag()->isEmpty()) {
			m_bID3v2_save = TRUE;
			ShowHideID3v2Column();
			m_sID3v2_Artists = GetEncodingString(fileinfo->ID3v2Tag()->artist().toCString(true));
			m_sID3v2_Title = GetEncodingString(fileinfo->ID3v2Tag()->title().toCString(true));
			m_sID3v2_Album = GetEncodingString(fileinfo->ID3v2Tag()->album().toCString(true));
			m_sID3v2_TrackNo = GetEncodingString(fileinfo->ID3v2Tag()->track().toCString(true));
			m_sID3v2_Year = GetEncodingString(fileinfo->ID3v2Tag()->year().toCString(true));
			m_sID3v2_Genre = GetEncodingString(fileinfo->ID3v2Tag()->genre().toCString(true));
			m_sID3v2_Comment = GetEncodingString(fileinfo->ID3v2Tag()->comment().toCString(true));
			m_sID3v2_Copyrights = GetEncodingString(fileinfo->ID3v2Tag()->copyright().toCString(true));

			int length = (int)fileinfo->ID3v2Tag()->URI().length();
			CString temp;
			if ((length > fileinfo->ID3v2Tag()->URI().find("]")) && (fileinfo->ID3v2Tag()->URI().find("]") > 0)) {
				temp = GetEncodingString(fileinfo->ID3v2Tag()->URI().substr(fileinfo->ID3v2Tag()->URI().find("]") + 2).toCString(true));
			} else {
				temp = GetEncodingString(fileinfo->ID3v2Tag()->URI().toCString(true));
			}
			m_sID3v2_URI = temp.TrimLeft();
			m_sID3v2_Words = GetEncodingString(fileinfo->ID3v2Tag()->words().toCString(true));
			m_sID3v2_Composers = GetEncodingString(fileinfo->ID3v2Tag()->composers().toCString(true));
			m_sID3v2_Arrangements = GetEncodingString(fileinfo->ID3v2Tag()->arrangements().toCString(true));
			m_sID3v2_Original_Artists = GetEncodingString(fileinfo->ID3v2Tag()->origArtist().toCString(true));
			m_sID3v2_Encoding_Engineer = GetEncodingString(fileinfo->ID3v2Tag()->encEngineer().toCString(true));

			//		m_bID3v2_UnSynchronization = fileinfo->ID3v2Tag()->header()->unsynchronisation();
			//		SetVersionSpecificColumn();
		} else {
			m_bID3v2_save = FALSE;
			ShowHideID3v2Column();
		}
	} else {
		m_bID3v2_save = FALSE;
		ShowHideID3v2Column();
	}


	UpdateData(FALSE);
	::LeaveCriticalSection(&CriticalSection);
	return TRUE;
}


//void CFileInfo::SetVersionSpecificColumn()
//{
//	m_ID3v2_Version.SetCurSel((int)(fileinfo->ID3v2Tag()->header()->majorVersion() - ID3v2Version[0].flag));
//	__int32 i = 0;
//	__int32 nCount = 0;
//	m_ID3v2_String_Encoding.ResetContent();
//	while(ID3v2Version[fileinfo->ID3v2Tag()->header()->majorVersion() - ID3v2Version[0].flag].Str_Enc[i] != NULL) {
//		nCount = m_ID3v2_String_Encoding.AddString(ID3v2Version[fileinfo->ID3v2Tag()->header()->majorVersion() 
//			- ID3v2Version[0].flag].Str_Enc[i]);
//		if (nCount == CB_ERR)
//			break;
//		i++;
//	}
//	m_ID3v2_String_Encoding.SetCurSel((int)(SetID3v2Encoding(fileinfo->dlgtag.id3v2.GetEncoding()));
//}

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
			//fileinfo->tag()->genre().toCString()); // "Others"
			SetID3v1Genre(Default_Genre);
		}
	}
	else
	{
		UpdateData(TRUE);
		m_bID3v1_save = FALSE;
		ShowHideID3v1Column();
	}

}

void CFileInfo::SetID3v1Genre(const char *Genre)
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
		if (current == Default_Genre) {
			m_ID3v1_Genre.SetCurSel(index);
			return;
		}
	}

	return;
}

unsigned __int8 CFileInfo::GetID3v1Genre()
{
	CString current;
	unsigned __int8 index;
	index = (unsigned __int8)m_ID3v1_Genre.GetCurSel();
	if (index == CB_ERR)
		return DEFAULT_GENRE;
	m_ID3v1_Genre.GetLBText(index, current);
	index = TagLib::ID3v1::genreIndex((LPCSTR)current);
	if (index == INVALID_GENRE_NAME) {
		index = DEFAULT_GENRE;
	} else {
		// do nothing.
	}
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
	m_ID3v2_Copyrights.EnableWindow(m_bID3v2_save);
	m_ID3v2_URI.EnableWindow(m_bID3v2_save);
	m_ID3v2_Words.EnableWindow(m_bID3v2_save);
	m_ID3v2_Composers.EnableWindow(m_bID3v2_save);
	m_ID3v2_Arrangements.EnableWindow(m_bID3v2_save);
	m_ID3v2_Original_Artists.EnableWindow(m_bID3v2_save);
	m_ID3v2_Encoding_Engineer.EnableWindow(m_bID3v2_save);
//	m_ID3v2_Version.EnableWindow(m_bID3v2_save);
//	m_ID3v2_String_Encoding.EnableWindow(m_bID3v2_save);
	m_ID3v2_CopyFromV1.EnableWindow(m_bID3v1_save && m_bID3v2_save);
	m_ID3v1_CopyFromV2.EnableWindow(m_bID3v1_save && m_bID3v2_save);
//	m_ID3v2_UnSynchronization.EnableWindow(m_bID3v2_save);

	return;
}
void CFileInfo::OnBnClickedId3v2Save()
{
	// TODO: Add your control notification handler code here
	if(!m_bID3v2_save)
	{
		m_bID3v2_save = TRUE;
		ShowHideID3v2Column();
//		SetVersionSpecificColumn();
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
	m_ID3v1_Genre.GetLBText(m_ID3v1_Genre.GetCurSel(), m_sID3v2_Genre);
	m_sID3v2_Comment = m_sID3v1_Comment;
	m_sID3v2_TrackNo = 	m_sID3v1_TrackNo;

	UpdateData(FALSE);
}



//void CFileInfo::OnCbnSelchangeId3v2Version()
//{
//	// TODO: Add your control notification handler code here
//	UpdateData(TRUE);
//	dlgtag.id3v2.SetVersion(ID3v2Version[m_ID3v2_Version.GetCurSel()].flag);
//	dlgtag.id3v2.SetEncoding((unsigned char)m_ID3v2_String_Encoding.GetCurSel());
//	__int32 i = 0;
//	__int32 nCount = 0;
//	m_ID3v2_String_Encoding.ResetContent();
//	while(ID3v2Version[dlgtag.id3v2.GetVersion() - ID3v2Version[0].flag].Str_Enc[i] != NULL) {
//		nCount = m_ID3v2_String_Encoding.AddString(ID3v2Version[dlgtag.id3v2.GetVersion() 
//			- ID3v2Version[0].flag].Str_Enc[i]);
//		if (nCount == CB_ERR)
//			break;
//		i++;
//	}
//	m_ID3v2_String_Encoding.SetCurSel(SetID3v2Encoding(fileinfo->ID3v2Tag()->header()->));
//}

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
