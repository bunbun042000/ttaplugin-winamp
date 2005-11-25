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

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
public:
	afx_msg void OnBnClickedCancel();
};
