// InfoDialog.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "in_tta.h"
#include "InfoDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CInfoDialog ダイアログ


CInfoDialog::CInfoDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CInfoDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CInfoDialog)
	m_FileName = _T("");
	m_StaticFileInfo = _T("");
	m_ID3v2_Comment = _T("");
	m_ID3v2_Year = _T("");
	m_ID3v2_URL = _T("");
	//}}AFX_DATA_INIT
}


void CInfoDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CInfoDialog)
	DDX_Text(pDX, IDC_FILENAME, m_FileName);
	DDX_Text(pDX, IDC_STATIC_FILE_INFORMATION, m_StaticFileInfo);
	DDX_LBString(pDX, IDC_LIST_ID3V2_COMMENT, m_ID3v2_Comment);
	DDX_Text(pDX, IDC_EDIT_ID3V2_YEAR, m_ID3v2_Year);
	DDX_Text(pDX, IDC_EDIT_ID3V2_URL, m_ID3v2_URL);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CInfoDialog, CDialog)
	//{{AFX_MSG_MAP(CInfoDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInfoDialog メッセージ ハンドラ

void CInfoDialog::OnOK() 
{
	// TODO: この位置にその他の検証用のコードを追加してください
	
	CDialog::OnOK();
}

void CInfoDialog::OnCancel() 
{
	// TODO: この位置に特別な後処理を追加してください。
	
	CDialog::OnCancel();
}

BOOL CInfoDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: この位置に初期化の補足処理を追加してください
//	m_FileName = ttaTag->GetFileName();	

	return TRUE;  // コントロールにフォーカスを設定しないとき、戻り値は TRUE となります
	              // 例外: OCX プロパティ ページの戻り値は FALSE となります
}
