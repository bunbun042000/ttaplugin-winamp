#if !defined(AFX_INFODIALOG_H__E1486840_DFCD_4C7C_9129_E2C532E36BE4__INCLUDED_)
#define AFX_INFODIALOG_H__E1486840_DFCD_4C7C_9129_E2C532E36BE4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// InfoDialog.h : �w�b�_�[ �t�@�C��
//

#include "TtaTag.h"

/////////////////////////////////////////////////////////////////////////////
// CInfoDialog �_�C�A���O

class CInfoDialog : public CDialog
{
// �R���X�g���N�V����
public:
	CInfoDialog(CWnd *pParent);   // �W���̃R���X�g���N�^

// �_�C�A���O �f�[�^
	//{{AFX_DATA(CInfoDialog)
	enum { IDD = IDD_TAGINFO };
	CString	m_FileName;
	CString	m_StaticFileInfo;
	CString	m_ID3v2_Comment;
	CString	m_ID3v2_Year;
	CString	m_ID3v2_URL;
	//}}AFX_DATA


// �I�[�o�[���C�h
	// ClassWizard �͉��z�֐��̃I�[�o�[���C�h�𐶐����܂��B
	//{{AFX_VIRTUAL(CInfoDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �T�|�[�g
	//}}AFX_VIRTUAL

// �C���v�������e�[�V����
public:
		CTtaTag *ttaTag;

protected:

	// �������ꂽ���b�Z�[�W �}�b�v�֐�
	//{{AFX_MSG(CInfoDialog)
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ �͑O�s�̒��O�ɒǉ��̐錾��}�����܂��B

#endif // !defined(AFX_INFODIALOG_H__E1486840_DFCD_4C7C_9129_E2C532E36BE4__INCLUDED_)
