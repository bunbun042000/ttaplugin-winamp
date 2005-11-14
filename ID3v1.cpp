// ID3v1.cpp: CID3v1 クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "in_tta.h"
#include "ID3v1.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// 構築/消滅
//////////////////////////////////////////////////////////////////////

CID3v1::CID3v1()
{

}

CID3v1::~CID3v1()
{

}

int CID3v1::ReadTag(const char *filename)
{
	// File open
	unsigned long result;

	::strncpy(FileName, filename, MAX_PATHLEN);

	HFILE = CreateFile(FileName, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (HFILE == INVALID_HANDLE_VALUE) {
		STATE = OPEN_ERROR;
		return -1;
	}
	SetFilePointer(HFILE, -(int) sizeof(v1tag), NULL, FILE_END);
	if (ReadFile(HFILE, &tag, sizeof(v1tag), &result, NULL) &&
		result == sizeof(v1tag) && !memcmp(tag.id, "TAG", 3)) {
		has_tag = true;
	}
	SetFilePointer(HFILE, 0, NULL, FILE_BEGIN);
	CloseHandle(HFILE);
	return (0);
}

bool CID3v1::SaveTag()
{
	int offset;
	unsigned long result;

	// update ID3V1 tag
	HFILE = CreateFile(FileName, GENERIC_READ|GENERIC_WRITE,
		FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if (HFILE == INVALID_HANDLE_VALUE) {
		error(OPEN_ERROR, FileName);
		return false;
	}

	offset = (has_tag) ? -(int) sizeof(v1tag) : 0;
	SetFilePointer(HFILE, offset, NULL, FILE_END);
	if (!WriteFile(HFILE, &tag, sizeof(v1tag), &result, 0) ||
		result != sizeof(v1tag)) {
		CloseHandle(HFILE);
		error(WRITE_ERROR, FileName);
		return false;
	}
	CloseHandle(HFILE);

	has_tag = true;
	return true;
}

void CID3v1::DeleteTag()
{
	if (!has_tag) return;

	// delete ID3V1 tag
	HFILE = CreateFile(FileName, GENERIC_READ|GENERIC_WRITE,
		FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if (HFILE == INVALID_HANDLE_VALUE) {
		error(OPEN_ERROR, FileName);
		return;
	}

	SetFilePointer(HFILE, -(int) sizeof(v1tag), NULL, FILE_END);
	SetEndOfFile(HFILE);
	CloseHandle(HFILE);

	has_tag = false;
}

void CID3v1::SetAlbum(const char *album)
{
	strncpy(tag.album, album, 30);
}

void CID3v1::SetArtist(const char *artist)
{
	strncpy(tag.artist, artist, 30);
}

void CID3v1::SetComment(const char *comment)
{
	strncpy(tag.comment, comment, 28);
}

void CID3v1::SetGenre(const char genre)
{
	tag.genre = genre;
}

void CID3v1::SetTitle(const char *title)
{
	strncpy(tag.title, title, 30);
}

void CID3v1::SetTrack(const char track)
{
	tag.track = track;
}

void CID3v1::SetYear(const char *year)
{
	strncpy(tag.year, year, 4);
}
