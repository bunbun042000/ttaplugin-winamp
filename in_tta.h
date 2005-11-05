// in_tta.h : IN_TTA アプリケーションのメイン ヘッダー ファイルです。
//

#if !defined(AFX_IN_TTA_H__41DD5DF7_1423_4D40_9DB2_D2F36B66A263__INCLUDED_)
#define AFX_IN_TTA_H__41DD5DF7_1423_4D40_9DB2_D2F36B66A263__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// メイン シンボル

#include "ttadec.h"

int open_tta_file(const char *filename, tta_info *ttainfo);
void get_id3v1_tag(tta_info *ttainfo);
static void get_id3v2_tag (tta_info *ttainfo);
static void save_id3v1_tag (tta_info *ttainfo);
static void save_id3v2_tag (tta_info *ttainfo);
static void del_id3v1_tag (tta_info *ttainfo);
static void del_id3v2_tag (tta_info *ttainfo);

#define MAX_PATHLEN		512
//#define WAVE_FORMAT_PCM	1

/////////////////////////////////////////////////////////////////////////////
// CIn_ttaApp
// このクラスの動作の定義に関しては in_tta.cpp ファイルを参照してください。
//

class CIn_ttaApp : public CWinApp
{
public:
	CIn_ttaApp();

// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CIn_ttaApp)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CIn_ttaApp)
		// メモ -  ClassWizard はこの位置にメンバ関数を追加または削除します。
		//         この位置に生成されるコードを編集しないでください。
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_IN_TTA_H__41DD5DF7_1423_4D40_9DB2_D2F36B66A263__INCLUDED_)
