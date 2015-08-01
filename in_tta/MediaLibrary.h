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
	std::string		Format;
	std::string		Title;
	std::string		Artist;
	std::string		AlbumArtist;
	std::string		Comment;
	std::string		Album;
	std::string		Year;
	std::string		Genre;
	std::string		Track;
	std::string		Composer;
	std::string		Publisher;
	std::string		Disc;
	std::string		BPM;
};
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
	TagInfo		        TagData;
	DWORD				GetTagTime;
	std::string			FileName;
	TagLib::TrueAudio::File *TTAFile;

	void FlushCache(void);
	bool GetTagInfo(const std::string fn);
};

#endif // !defined(AFX_MediaLibrary_H__997DC726_50DB_46B4_A156_DB5E92EC2BE8__INCLUDED_)
