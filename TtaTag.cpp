// TtaTag.cpp: CTtaTag クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////

// $ LastChangedDate: $

#include "stdafx.h"
#include "in_tta.h"
#include "TtaTag.h"
#include "id3genre.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// 構築/消滅
//////////////////////////////////////////////////////////////////////

CTtaTag::CTtaTag()
{

}

CTtaTag::~CTtaTag()
{
}

int CTtaTag::ReadTag(const char *filename)
{
	unsigned long result;

	// File open
	::strncpy(FileName, filename, MAX_PATHLEN);

	HFILE = CreateFile(FileName, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (HFILE == INVALID_HANDLE_VALUE) {
		STATE = OPEN_ERROR;
		return -1;
	}

	FILESIZE = GetFileSize(HFILE, NULL);

	//Read ID3v1.1
	id3v1.ReadTag(FileName);

	//Read ID3v2.3
	id3v2.ReadTag(FileName);

	//Read TTA Header
	ReadTTAheader();

}



