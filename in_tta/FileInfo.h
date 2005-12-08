#pragma once

#include "TtaTag.h"
#include "afxwin.h"

const static __int32 MAX_GENRE = 148;
const static __int32 DEFAULT_GENRE = 12; // "Others"

extern char *genre[];

struct version {
	unsigned __int8 flag;
	char *ver;
	char **Str_Enc;
};

static char *EncodingVer24[] =
{
	"ISO-8859-1",
	"UTF-16(UNICODE)",
	"UTF-16BE(UNICODE)",
	"UTF-8(UNICODE)",
	NULL
};
static char *EncodingVer23[] =
{
	"ISO-8859-1",
	"UTF-16(UNICODE)",
	NULL
};

const version ID3v2Version[] =
{
	{0x03, "ver.2.3", EncodingVer23},
	{0x04, "ver.2.4", EncodingVer24},
	{0xff, NULL, NULL}
};


// CFileInfo dialog

class CFileInfo : public CDialog
{
	DECLARE_DYNAMIC(CFileInfo)

public:
	CFileInfo(CWnd* pParent = NULL, const char *filename = NULL);   // standard constructor
	virtual ~CFileInfo();

// Dialog Data
	enum { IDD = IDD_TAGINFO };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	void ShowHideID3v1Column();
	void ShowHideID3v2Column();
	void SetVersionSpecificColumn();
	void SetID3v1Genre(CString &Genre);
	__int32  GetID3v1Genre();

	CRITICAL_SECTION	CriticalSection;
	CString m_sFileName;

// FileInformation
	CTtaTag dlgtag;

// ID3v1
	BOOL m_bID3v1_save;
	CString m_sID3v1_Name;
	CString m_sID3v1_Artists;
	CString m_sID3v1_Album;
	CString m_sID3v1_Comment;
	CString m_sID3v1_Year;
	CString m_sID3v1_TrackNo;
	CComboBox m_ID3v1_Genre;


	CButton m_ID3v1_Check;
	CEdit m_ID3v1_TrackNo;
	CEdit m_ID3v1_Name;
	CEdit m_ID3v1_Artist;
	CEdit m_ID3v1_Album;
	CEdit m_ID3v1_Comment;
	CEdit m_ID3v1_Year;
	CButton m_ID3v1_CopyFromV2;

// ID3v2
	BOOL m_bID3v2_save;
	CString m_sID3v2_Title;
	CString m_sID3v2_Artists;
	CString m_sID3v2_Album;
	CString m_sID3v2_Year;
	CString m_sID3v2_Genre;
	CString m_sID3v2_Comment;
	CString m_sID3v2_TrackNo;
	CString m_sID3v2_Copyrights;
	CString m_sID3v2_URI;
	CString m_sID3v2_Words;
	CString m_sID3v2_Composers;
	CString m_sID3v2_Arrangements;
	CString m_sID3v2_Original_Artists;
	CString m_sID3v2_Encoding_Engineer;
	BOOL m_bID3v2_UnSynchronization;

	CEdit m_ID3v2_Title;
	CEdit m_ID3v2_Artists;
	CEdit m_ID3v2_Album;
	CEdit m_ID3v2_Year;
	CEdit m_ID3v2_Genre;
	CEdit m_ID3v2_Comment;
	CEdit m_ID3v2_TrackNo;
	CEdit m_ID3v2_Copyrights;
	CEdit m_ID3v2_URI;
	CEdit m_ID3v2_Words;
	CEdit m_ID3v2_Composers;
	CEdit m_ID3v2_Arrangements;
	CEdit m_ID3v2_Original_Artists;
	CEdit m_ID3v2_Encoding_Engineer;
	CComboBox m_ID3v2_Version;
	CComboBox m_ID3v2_String_Encoding;
	CButton m_ID3v2_CopyFromV1;
	CButton m_ID3v2_UnSynchronization;

	CString m_sFileFormat;

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedId3v1Save();
	afx_msg void OnBnClickedId3v2Save();
	afx_msg void OnBnClickedCopyfromv1();
	afx_msg void OnCbnSelchangeId3v2Version();
	afx_msg void OnBnClickedCopyfromv2();
};
