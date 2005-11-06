// TagInfo.cpp: CTagInfo クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"

#include "TagInfo.h"
#include "in_tta.h"

//////////////////////////////////////////////////////////////////////
// 構築/消滅
//////////////////////////////////////////////////////////////////////

CTagInfo::CTagInfo()
{
	::InitializeCriticalSection(&CriticalSection);

	FlushCache();

}

CTagInfo::~CTagInfo()
{
	::DeleteCriticalSection(&CriticalSection);

}

void CTagInfo::FlushCache(void)
{
	::EnterCriticalSection(&CriticalSection);

	*Cache.FileName = '\0';
	GetTagTime = 0;
	

	::LeaveCriticalSection(&CriticalSection);
}

bool CTagInfo::GetTagInfo()
{
	tta_info ttainfo;
	ttainfo.id3v1.id3has = 0;
	ttainfo.id3v2.id3has = 0;
	char buf[MAX_MUSICTEXT];
	unsigned int genrenum;

	if (*Cache.FileName == NULL)
		return false;

	bool FindTag = true;

	if(open_tta_file((const char *)Cache.FileName, &ttainfo) == 0)
		Cache.Length = (int) ttainfo.LENGTH;
//	ttainfo.HFILE = CreateFile(Cache.FileName, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE,
//		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
//	if (ttainfo.HFILE == INVALID_HANDLE_VALUE) {
//		ttainfo.STATE = OPEN_ERROR;
//		FindTag = false;
//	} else {
//	// get file size
//	ttainfo.FILESIZE = GetFileSize(ttainfo.HFILE, NULL);
//	lstrcpyn(ttainfo.filename, Cache.FileName, sizeof(ttainfo.filename));
//	}

//	if (FindTag)
//	{
//		id3v1_tag id3v1;
//		unsigned long result;
//		FindTag = false;
//
//		SetFilePointer(ttainfo.HFILE, -(int) sizeof(id3v1_tag), NULL, FILE_END);
//		if (ReadFile(ttainfo.HFILE, &id3v1, sizeof(id3v1_tag), &result, NULL) &&
//			result == sizeof(id3v1_tag) && !memcmp(id3v1.id, "TAG", 3)) {
//			CopyMemory(ttainfo.id3v1.title, id3v1.title, 30);
//			CopyMemory(ttainfo.id3v1.artist, id3v1.artist, 30);
//			CopyMemory(ttainfo.id3v1.album, id3v1.album, 30);
//			CopyMemory(ttainfo.id3v1.year, id3v1.year, 4);
//			CopyMemory(ttainfo.id3v1.comment, id3v1.comment, 28);
//			ttainfo.id3v1.track = id3v1.track;
//			ttainfo.id3v1.genre = id3v1.genre;
//			ttainfo.id3v1.id3has = 1;
//			FindTag = true;
//		}
//		SetFilePointer(ttainfo.HFILE, 0, NULL, FILE_BEGIN);
//	}
//	MessageBox(0,_itoa((unsigned int)ttainfo.id3v1.genre, buf, MAX_MUSICTEXT - 1),0,0);
//
//
	if(FindTag) {
		if(ttainfo.id3v2.id3has) {
			strncpy(Cache.Title, (const char *)ttainfo.id3v2.title, MAX_MUSICTEXT - 1);
			strncpy(Cache.Artist, (const char *)ttainfo.id3v2.artist, MAX_MUSICTEXT - 1);
			strncpy(Cache.Comment, (const char *)ttainfo.id3v2.comment, MAX_MUSICTEXT -1);
			strncpy(Cache.Album, (const char *)ttainfo.id3v2.album, MAX_MUSICTEXT - 1);
			strncpy(Cache.Year, (const char *)ttainfo.id3v2.year, MAX_MUSICTEXT - 1);
			strncpy(Cache.Genre, (const char *)ttainfo.id3v2.genre, MAX_MUSICTEXT - 1);
			strncpy(Cache.Track, (const char *)ttainfo.id3v2.track, MAX_MUSICTEXT - 1);
		} else if (ttainfo.id3v1.id3has) {
//		if (ttainfo.id3v1.id3has) {
			lstrcpyn(Cache.Title, (const char *)ttainfo.id3v1.title, MAX_MUSICTEXT - 1);
			lstrcpyn(Cache.Artist, (const char *)ttainfo.id3v1.artist, MAX_MUSICTEXT - 1);
			lstrcpyn(Cache.Comment, (const char *)ttainfo.id3v1.comment, MAX_MUSICTEXT - 1);
			lstrcpyn(Cache.Year, (const char *)ttainfo.id3v1.year, MAX_MUSICTEXT - 1);
			lstrcpyn(Cache.Album, (const char *)ttainfo.id3v1.album, MAX_MUSICTEXT - 1);
			genrenum = (unsigned int)ttainfo.id3v1.genre;
			if (genrenum < GENRES)
				lstrcpyn(Cache.Genre, (const char *)genre[genrenum], MAX_MUSICTEXT - 1);
			lstrcpyn(Cache.Track, (const char *)_itoa((int)ttainfo.id3v1.track, buf, MAX_MUSICTEXT -1), MAX_MUSICTEXT -1 );
		} else {
			strncpy(Cache.Title, (const char *)Cache.FileName, MAX_PATHLEN - 1);
		}		
	}
//	MessageBox(0,_itoa(Cache.Length, buf, 10),0,0);

	return FindTag;
}

int CTagInfo::GetExtendedFileInfo(extendedFileInfoStruct *ExtendedFileInfo)
{
	::EnterCriticalSection(&CriticalSection);

	strncpy(Cache.FileName, ExtendedFileInfo->filename, MAX_PATHLEN);

	bool FindTag;
	int RetCode;
		
	FindTag = GetTagInfo();


	if (FindTag)
	{
		char	Buff[16];
		char   *RetBuff;
		const char *MetaData = ExtendedFileInfo->metadata;

		if(_stricmp(MetaData, "length") == 0) {
			RetBuff = _ultoa(Cache.Length , Buff, 10);
			RetCode = 1;
		} else if(_stricmp(MetaData, "title") == 0) {
			RetBuff = Cache.Title;
			RetCode = 1;
		} else if(_stricmp(MetaData, "artist") == 0) {
			RetBuff = Cache.Artist;
			RetCode = 1;
		} else if(_stricmp(MetaData, "comment") == 0) {
			RetBuff = Cache.Comment;
			RetCode = 1;
		} else if(_stricmp(MetaData, "album") == 0) {
			RetBuff = Cache.Album;
			RetCode = 1;
		} else if(_stricmp(MetaData, "year") == 0) {
			RetBuff = Cache.Year;
			RetCode = 1;
		} else if(_stricmp(MetaData, "genre") == 0) {
			RetBuff = Cache.Genre;
			RetCode = 1;
		} else if(_stricmp(MetaData, "track") == 0) {
			RetBuff = Cache.Track;
			RetCode = 1;
		} else {
			RetCode = 0;
		}

		if(RetCode && (ExtendedFileInfo->retlen != 0)) {
			strncpy(ExtendedFileInfo->ret, RetBuff, ExtendedFileInfo->retlen - 1);
		}
	} else {
		RetCode = 0;
	}

	::LeaveCriticalSection(&CriticalSection);

	return RetCode;
}

