/*
The ttaplugin-winamp project.
Copyright (C) 2005-2011 Yamagata Fumihiro

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#if !defined(AFX_MediaLibrary_H__997DC726_50DB_46B4_A156_DB5E92EC2BE8__INCLUDED_)
#define AFX_MediaLibrary_H__997DC726_50DB_46B4_A156_DB5E92EC2BE8__INCLUDED_

#include "common.h"
#include "..\Winamp SDK\Winamp\wa_ipc.h"
#include <taglib/tstring.h>

static const __int32 MAX_MUSICTEXT = 512;
static const __int32 MAX_YEAR = 10;

struct TagInfo
{
	unsigned long	Length;
	char            Format[MAX_MUSICTEXT];
	char			FileName[MAX_PATHLEN];
	char			Title[MAX_MUSICTEXT];
	char			Artist[MAX_MUSICTEXT];
	char            AlbumArtist[MAX_MUSICTEXT];
	char			Comment[MAX_MUSICTEXT];
	char			Album[MAX_MUSICTEXT];
	char			Year[MAX_MUSICTEXT];
	char			Genre[MAX_MUSICTEXT];
	char			Track[MAX_MUSICTEXT];
	char			Composer[MAX_MUSICTEXT];
	char			OrgArtist[MAX_MUSICTEXT];
	char			Copyright[MAX_MUSICTEXT];
	char			Encoder[MAX_MUSICTEXT];
	char            Publisher[MAX_MUSICTEXT];
	char            Disc[MAX_MUSICTEXT];
	char            BPM[MAX_MUSICTEXT];
};

struct TagInfoW
{
	unsigned long	Length;
	wchar_t			Format[MAX_MUSICTEXT];
	wchar_t			FileName[MAX_PATHLEN];
	wchar_t			Title[MAX_MUSICTEXT];
	wchar_t			Artist[MAX_MUSICTEXT];
	wchar_t			AlbumArtist[MAX_MUSICTEXT];
	wchar_t			Comment[MAX_MUSICTEXT];
	wchar_t			Album[MAX_MUSICTEXT];
	wchar_t			Year[MAX_MUSICTEXT];
	wchar_t			Genre[MAX_MUSICTEXT];
	wchar_t			Track[MAX_MUSICTEXT];
	wchar_t			Composer[MAX_MUSICTEXT];
	wchar_t			OrgArtist[MAX_MUSICTEXT];
	wchar_t			Copyright[MAX_MUSICTEXT];
	wchar_t			Encoder[MAX_MUSICTEXT];
	wchar_t			Publisher[MAX_MUSICTEXT];
	wchar_t			Disc[MAX_MUSICTEXT];
	wchar_t			BPM[MAX_MUSICTEXT];
};

class CMediaLibrary  
{
public:
	CMediaLibrary();
	virtual ~CMediaLibrary();
	__int32  GetExtendedFileInfo(const char *fn, const char *data, char *dest, size_t destlen);
	__int32	 GetExtendedFileInfo(const wchar_t *fn, const char *data, wchar_t *dest, size_t destlen);
	__int32  SetExtendedFileInfo(const char *fn, const char *MetaData, const char *val);
	__int32  SetExtendedFileInfo(const wchar_t *fn, const char *MetaData, const wchar_t *val);
	__int32  WriteExtendedFileInfo();

private:

	CRITICAL_SECTION	CriticalSection;
	DWORD				GetTagTime;
	TagInfo		        Cache;
	TagInfoW			CacheW;
	bool				is_wchar;

	void				FlushCache(void);
	bool				GetTagInfo();
	bool				GetTagInfoW();
	__int32				privateWriteExtendedFileInfo();
	__int32				privateWriteExtendedFileInfoW();

};

#endif // !defined(AFX_MediaLibrary_H__997DC726_50DB_46B4_A156_DB5E92EC2BE8__INCLUDED_)
