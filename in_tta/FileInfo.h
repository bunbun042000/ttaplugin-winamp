#pragma once

#include "TtaTag.h"
#include "afxwin.h"

#define	MAX_GENRE		148

extern char *genre[];

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
	CString m_sFileName;

// ID3v1
	CString m_sID3v1_Name;
	CString m_sID3v1_Artists;
	CString m_sID3v1_Album;
	CString m_sID3v1_Comment;
	CString m_sID3v1_Year;
	CString m_sID3v1_TrackNo;
	CComboBox m_ID3v1_Genre;

	CTtaTag dlgtag;

	CButton m_ID3v1_Check;
	BOOL m_bID3v1_save;
	CEdit m_ID3v1_TrackNo;
	CEdit m_ID3v1_Name;
	CEdit m_ID3v1_Artist;
	CEdit m_ID3v1_Album;
	CEdit m_ID3v1_Comment;
	CEdit m_ID3v1_Year;

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedId3v1Save();
	CString m_sFileFormat;
};
