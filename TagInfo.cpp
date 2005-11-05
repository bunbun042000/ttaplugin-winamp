// TagInfo.cpp: CTagInfo �N���X�̃C���v�������e�[�V����
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"

#include "TagInfo.h"
#include "in_tta.h"

//////////////////////////////////////////////////////////////////////
// �\�z/����
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

int CTagInfo::GetExtendedFileInfo(extendedFileInfoStruct *ExtendedFileInfo)
{
	::EnterCriticalSection(&CriticalSection);

	tta_info *ttainfo = new tta_info;

	int RetCode;
	bool 	FindTag = 
		(open_tta_file((const char *)&ExtendedFileInfo->filename, ttainfo) == 0) 
		? true : false;

	if(FindTag) {
		strncpy(Cache.FileName, ExtendedFileInfo->filename, MAX_PATHLEN);
		get_id3v1_tag(ttainfo);
		
//		if((ttainfo->id3v2.id3has)) {
//			strncpy(Cache.Title, (const char *)ttainfo->id3v2.title, MAX_MUSICTEXT);
//			strncpy(Cache.Artist, (const char *)ttainfo->id3v2.artist, MAX_MUSICTEXT);
//			strncpy(Cache.Comment, (const char *)ttainfo->id3v2.comment, MAX_MUSICTEXT);
//			strncpy(Cache.Album, (const char *)ttainfo->id3v2.album, MAX_MUSICTEXT);
//			strncpy(Cache.Year, (const char *)ttainfo->id3v2.year, MAX_MUSICTEXT);
//			strncpy(Cache.Genre, (const char *)ttainfo->id3v2.genre, MAX_MUSICTEXT);
//			strncpy(Cache.Track, (const char *)ttainfo->id3v2.track, MAX_MUSICTEXT);
//		} else if (ttainfo->id3v1.id3has) {
		if (ttainfo->id3v1.id3has) {
			strncpy(Cache.Title, (const char *)ttainfo->id3v1.title, MAX_MUSICTEXT);
			strncpy(Cache.Artist, (const char *)ttainfo->id3v1.artist, MAX_MUSICTEXT);
			strncpy(Cache.Comment, (const char *)ttainfo->id3v1.comment, MAX_MUSICTEXT);
			strncpy(Cache.Album, (const char *)ttainfo->id3v1.album, MAX_MUSICTEXT);
			strncpy(Cache.Year, (const char *)ttainfo->id3v1.year, MAX_MUSICTEXT);
			strncpy(Cache.Genre, (const char *)ttainfo->id3v1.genre, MAX_MUSICTEXT);
			strncpy(Cache.Track, (const char *)ttainfo->id3v1.track, MAX_MUSICTEXT);
		} else {
			strncpy(Cache.Title, (const char *)ExtendedFileInfo->filename, MAX_PATHLEN);
		}		
		Cache.Length = ttainfo->LENGTH;
	}

	if (FindTag)
	{
		char	Buff[16];
		char   *RetBuff;
		const char*	MetaData = ExtendedFileInfo->metadata;

		if(_stricmp(MetaData, "length") == 0) {
			RetBuff = _ltoa((long)Cache.Length * 1000, Buff, 10);
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
	delete ttainfo;

	::LeaveCriticalSection(&CriticalSection);

	return RetCode;
}

