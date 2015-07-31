/*
The ttaplugin-winamp project.
Copyright (C) 2005-2013 Yamagata Fumihiro

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
#include <taglib/trueaudiofile.h>

static const __int32 MAX_MUSICTEXT = 512;
static const __int32 MAX_YEAR = 10;

struct TagInfo
{
	unsigned long	Length;
	char            Format[MAX_MUSICTEXT];
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

class CMediaLibrary  
{
public:
	CMediaLibrary();
	virtual ~CMediaLibrary();
	__int32  GetExtendedFileInfo(const char *fn, const char *data, char *dest, size_t destlen);
	__int32  SetExtendedFileInfo(const char *fn, const char *MetaData, const char *val);
	__int32  WriteExtendedFileInfo();

private:

	CRITICAL_SECTION	CriticalSection;
	TagInfo		        Cache;
	DWORD				GetTagTime;
	TagLib::String		FileName;
	TagLib::TrueAudio::File *TTAFile;

	void FlushCache(void);
	bool GetTagInfo(const TagLib::String fn);
};

#endif // !defined(AFX_MediaLibrary_H__997DC726_50DB_46B4_A156_DB5E92EC2BE8__INCLUDED_)
