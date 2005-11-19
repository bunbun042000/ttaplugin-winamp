#if !defined(AFX_INFODIALOG_H__E1486840_DFCD_4C7C_9129_E2C532E36BE4__INCLUDED_)
#define AFX_INFODIALOG_H__E1486840_DFCD_4C7C_9129_E2C532E36BE4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// InfoDialog.h : ヘッダー ファイル
//

#include "TtaTag.h"

/////////////////////////////////////////////////////////////////////////////
// CInfoDialog ダイアログ

class CInfoDialog : public CDialog
{
// コンストラクション
public:
	CInfoDialog(CWnd *pParent);   // 標準のコンストラクタ

// ダイアログ データ
	//{{AFX_DATA(CInfoDialog)
	enum { IDD = IDD_TAGINFO };
	CString	m_FileName;
	CString	m_StaticFileInfo;
	CString	m_ID3v2_Comment;
	CString	m_ID3v2_Year;
	CString	m_ID3v2_URL;
	//}}AFX_DATA


// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CInfoDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	//}}AFX_VIRTUAL

// インプリメンテーション
public:
		CTtaTag *ttaTag;

protected:

	// 生成されたメッセージ マップ関数
	//{{AFX_MSG(CInfoDialog)
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_INFODIALOG_H__E1486840_DFCD_4C7C_9129_E2C532E36BE4__INCLUDED_)
