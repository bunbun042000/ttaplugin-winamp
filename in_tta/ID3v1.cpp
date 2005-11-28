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
	has_tag = false;
}

CID3v1::~CID3v1()
{

}

bool CID3v1::ReadTag(HWND hMainWindow, const char *filename)
{
	// File open
	unsigned long result;
	has_tag = false;
	v1tag  tag;

	FileName = filename;
	ZeroMemory(&tag, sizeof(v1tag));

	HFILE = CreateFile((LPCTSTR)FileName, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (HFILE == INVALID_HANDLE_VALUE || HFILE == NULL) {
		error(hMainWindow, OPEN_ERROR);
		CloseHandle(HFILE);
		return false;
	}
	SetFilePointer(HFILE, -(int) sizeof(v1tag), NULL, FILE_END);
	if (ReadFile(HFILE, &tag, sizeof(v1tag), &result, NULL) &&
		result == sizeof(v1tag) && (memcmp(tag.id, "TAG", 3) == 0)) {
		has_tag = true;
	} else {
		CloseHandle(HFILE);
		has_tag = false;
		return false;
	}

	Title = tag.title;
	Artist = tag.artist;
	Album = tag.album;
	Year = tag.year;
	Comment = tag.comment;
	Track = tag.track;
	Genre = tag.genre;

	SetFilePointer(HFILE, 0, NULL, FILE_BEGIN);
	CloseHandle(HFILE);
	return true;
}

bool CID3v1::SaveTag(HWND hMainWindow)
{
	int offset;
	unsigned long result;
	v1tag  tag;

	// update ID3V1 tag
	HFILE = CreateFile(FileName, GENERIC_READ|GENERIC_WRITE,
		FILE_SHARE_READ, NULL,OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (HFILE == INVALID_HANDLE_VALUE) {
		CloseHandle(HFILE);
		return OPEN_ERROR;
	}
	ZeroMemory(&tag, sizeof(v1tag));

	CopyMemory(tag.id, "TAG", 3);
	CopyMemory(tag.title, 
		Title.GetBufferSetLength(sizeof(tag.title)), sizeof(tag.title));
	CopyMemory(tag.artist,
		Artist.GetBufferSetLength(sizeof(tag.artist)), sizeof(tag.artist));
	CopyMemory(tag.album, 
		Album.GetBufferSetLength(sizeof(tag.album)), sizeof(tag.album));
	CopyMemory(tag.year,
		Year.GetBufferSetLength(sizeof(tag.year)), sizeof(tag.year));
	CopyMemory(tag.comment,
		Comment.GetBufferSetLength(sizeof(tag.comment)), sizeof(tag.comment));
	tag.track = Track;
	tag.genre = Genre;

	offset = (has_tag) ? -(int) sizeof(v1tag) : 0;
	SetFilePointer(HFILE, offset, NULL, FILE_END);
	if (!WriteFile(HFILE, &tag, sizeof(v1tag), &result, 0) ||
		result != sizeof(v1tag)) {
		CloseHandle(HFILE);
		error(hMainWindow, WRITE_ERROR);
		return false;
	}
	CloseHandle(HFILE);

	has_tag = true;
	return true;
}

void CID3v1::DeleteTag(HWND hMainWindow)
{
	if (!has_tag) return;

	// delete ID3V1 tag
	HFILE = CreateFile(FileName, GENERIC_READ|GENERIC_WRITE,
		FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if (HFILE == INVALID_HANDLE_VALUE) {
		error(hMainWindow, OPEN_ERROR);
		return;
	}

	SetFilePointer(HFILE, -(int) sizeof(v1tag), NULL, FILE_END);
	SetEndOfFile(HFILE);
	CloseHandle(HFILE);

	has_tag = false;
}

void CID3v1::SetAlbum(const char *album)
{
	Album = album;
}

void CID3v1::SetArtist(const char *artist)
{
	Artist = artist;
}

void CID3v1::SetComment(const char *comment)
{
	Comment = comment;
}

void CID3v1::SetGenre(const unsigned char genre)
{
	if (genre >= 0 && genre < GENRES)
		Genre = genre;
	else
		Genre = NULL;
}

void CID3v1::SetTitle(const char *title)
{
	Title = title;
}

void CID3v1::SetTrack(const unsigned char track)
{
	if (track >= 0 && track < 255)
		Track = track;
	else
		Track = NULL;
}

void CID3v1::SetYear(const char *year)
{
	Year = year;
}

void CID3v1::error(HWND hMainWindow, int err_no)
{
	char message[1024];
	char *name = NULL;

	::GetFileTitle(FileName, name, MAX_PATHLEN - 1);

	switch (err_no) {
	case OPEN_ERROR:	wsprintf(message, "Can't open file:\n%s", name); break;
	case FORMAT_ERROR:	wsprintf(message, "Unknown TTA format version:\n%s", name); break;
	case PLAYER_ERROR:	wsprintf(message, "Not supported file format:\n%s", name); break;
	case FILE_ERROR:	wsprintf(message, "File is corrupted:\n%s", name); break;
	case READ_ERROR:	wsprintf(message, "Can't read from file:\n%s", name); break;
	case WRITE_ERROR:	wsprintf(message, "Can't write to file:\n%s", name); break;
	case MEMORY_ERROR:	wsprintf(message, "Insufficient memory available"); break;
	case THREAD_ERROR:	wsprintf(message, "Error killing thread"); break;
	default:			wsprintf(message, "Unknown TTA decoder error"); break;
	}

	MessageBox(hMainWindow, message, "TTA Decoder Error",
		MB_OK|MB_ICONERROR|MB_SYSTEMMODAL);
}
