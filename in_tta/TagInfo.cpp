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

bool CTagInfo::GetTagInfo(HWND hMainWindow)
{
	char buf[MAX_MUSICTEXT];
	unsigned int genrenum;

	if (*Cache.FileName == NULL)
		return false;

	bool FindTag = false;

	Cache.Title[0] = '\0';
	Cache.Artist[0] = '\0';
	Cache.Comment[0] = '\0';
	Cache.Album[0] = '\0';
	Cache.Year[0] = '\0';
	Cache.Genre[0] = '\0';
	Cache.Track[0] = '\0';
	Cache.Composer[0] = '\0';
	Cache.OrgArtist[0] = '\0';
	Cache.Copyright[0] = '\0';
	Cache.Encoder[0] = '\0';

	if(ttatag.ReadTag(hMainWindow, Cache.FileName))
	{
		Cache.Length = (unsigned long) ttatag.GetLengthbymsec();
		FindTag = true;
	}

	if(FindTag) {
		if(ttatag.id3v2.hasTag()) {
//			strncpy(Cache.Title, (const char *)ttainfo.id3v2.title, MAX_MUSICTEXT - 1);
//			strncpy(Cache.Artist, (const char *)ttainfo.id3v2.artist, MAX_MUSICTEXT - 1);
//			strncpy(Cache.Comment, (const char *)ttainfo.id3v2.comment, MAX_MUSICTEXT -1);
//			strncpy(Cache.Album, (const char *)ttainfo.id3v2.album, MAX_MUSICTEXT - 1);
//			strncpy(Cache.Year, (const char *)ttainfo.id3v2.year, MAX_MUSICTEXT - 1);
//			strncpy(Cache.Genre, (const char *)ttainfo.id3v2.genre, MAX_MUSICTEXT - 1);
//			strncpy(Cache.Track, (const char *)ttainfo.id3v2.track, MAX_MUSICTEXT - 1);
		} else if (ttatag.id3v1.hasTag()) {
			lstrcpyn(Cache.Title, (LPCTSTR)ttatag.id3v1.GetTitle(), MAX_MUSICTEXT - 1);
			lstrcpyn(Cache.Artist, (LPCTSTR)ttatag.id3v1.GetArtist(), MAX_MUSICTEXT - 1);
			lstrcpyn(Cache.Comment, (LPCTSTR)ttatag.id3v1.GetComment(), MAX_MUSICTEXT - 1);
			lstrcpyn(Cache.Year, (LPCTSTR)ttatag.id3v1.GetYear(), MAX_MUSICTEXT - 1);
			lstrcpyn(Cache.Album, (LPCTSTR)ttatag.id3v1.GetAlbum(), MAX_MUSICTEXT - 1);
			genrenum = (unsigned char)ttatag.id3v1.GetGenre();
			if (genrenum < GENRES)
				lstrcpyn(Cache.Genre, (const char *)genre[genrenum], MAX_MUSICTEXT - 1);
			_ultoa_s((unsigned long)ttatag.id3v1.GetTrack(), buf, MAX_MUSICTEXT -1, 10);
			lstrcpyn(Cache.Track, (const char *)buf, MAX_MUSICTEXT -1 );
		}		
	}
//	MessageBox(0, ttatag.id3v1.GetArtist(), 0, 0);
//	MessageBox(0,_itoa(Cache.Length, buf, 10),0,0);

	return FindTag;
}

int CTagInfo::GetExtendedFileInfo(HWND hMainWindow, extendedFileInfoStruct *ExtendedFileInfo)
{
	::EnterCriticalSection(&CriticalSection);

	::strncpy_s(Cache.FileName, MAX_PATHLEN - 1, ExtendedFileInfo->filename, MAX_PATHLEN - 1);

	bool FindTag;
	int RetCode;
		
	FindTag = GetTagInfo(hMainWindow);


	if (FindTag)
	{
		char	Buff[16];
		char   *RetBuff;
		const char *MetaData = ExtendedFileInfo->metadata;

		if(_stricmp(MetaData, "length") == 0) {
			_ultoa_s(Cache.Length , Buff, sizeof(Buff), 10);
			RetBuff = Buff;
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
			lstrcpyn(ExtendedFileInfo->ret, RetBuff, ExtendedFileInfo->retlen - 1);
		}
	} else {
		RetCode = 0;
	}

	::LeaveCriticalSection(&CriticalSection);

	return RetCode;
}

