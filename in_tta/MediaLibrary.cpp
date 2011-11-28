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

// MediaLibrary.cpp: Implementation of CMediaLibrary class
//
//////////////////////////////////////////////////////////////////////
#include "MediaLibrary.h"
#include "common.h"
#include "resource.h"
#include <taglib/trueaudiofile.h>
#include <taglib/tag.h>
#include <taglib/id3v2tag.h>
#include <taglib/id3v1tag.h>
#include <taglib/attachedpictureframe.h>
#include "AlbumArt.h"
#include "../Winamp SDK/Agave/AlbumArt/svc_albumArtProvider.h"

//////////////////////////////////////////////////////////////////////
// Create / Destroy
//////////////////////////////////////////////////////////////////////

CMediaLibrary::CMediaLibrary()
{
	::InitializeCriticalSection(&CriticalSection);

	FlushCache();

}

CMediaLibrary::~CMediaLibrary()
{
	::DeleteCriticalSection(&CriticalSection);

}

void CMediaLibrary::FlushCache(void)
{
	::EnterCriticalSection(&CriticalSection);

	*Cache.FileName = '\0';
	GetTagTime = 0;
	Cache.Title[0] = '\0';
	Cache.Artist[0] = '\0';
	Cache.Comment[0] = '\0';
	Cache.Album[0] = '\0';
	Cache.AlbumArtist[0] = '\0';
	Cache.Year[0] = '\0';
	Cache.Genre[0] = '\0';
	Cache.Track[0] = '\0';
	Cache.Composer[0] = '\0';
	Cache.OrgArtist[0] = '\0';
	Cache.Copyright[0] = '\0';
	Cache.Encoder[0] = '\0';
	Cache.Publisher[0] = '\0';
	Cache.Disc[0] = '\0';
	Cache.BPM[0] = '\0';
	if(Cache.E_Image != NULL) {
		if(Cache.E_Image->Image != NULL) {
			delete [] Cache.E_Image->Image;
		} else {
			// do nothing
		}
		delete Cache.E_Image; 
	} else {
		// do nothing
	}
	Cache.E_Image = NULL;

	::LeaveCriticalSection(&CriticalSection);
}

bool CMediaLibrary::GetTagInfo()
{

	if (*Cache.FileName == NULL) {
		return false;
	}

	// If target file cannot access
	if (TagLib::File::isReadable(Cache.FileName) == false) {
		return false;
	}

	TagLib::TrueAudio::File TagFile(Cache.FileName);


	Cache.Length = (unsigned long) (TagFile.audioProperties()->length() * 1000.L);

	int Lengthbysec = TagFile.audioProperties()->length();
	int hour = Lengthbysec / 3600;
	int min  = Lengthbysec / 60;
	int sec  = Lengthbysec % 60;

	CString second;
	if(hour > 0) {
		second.Format("%d:%02d:%02d", hour, min, sec);
	} else if(min > 0) {
		second.Format("%d:%02d", min, sec);
	} else {
		second.Format("%d", sec);
	}

	CString temp;
	temp.Format(IDS_TTAFORMAT,
		TagFile.audioProperties()->ttaVersion(), 
		(int)TagFile.audioProperties()->bitsPerSample(), 
		TagFile.audioProperties()->sampleRate(),
		TagFile.audioProperties()->bitrate(), 
		TagFile.audioProperties()->channels(),
		(TagFile.audioProperties()->channels() == 2) ? "Stereo" : "Monoral",
		(LPCTSTR)second);
	_tcsncpy_s(Cache.Format, MAX_MUSICTEXT - 1, (LPCTSTR)temp, _TRUNCATE);

	if (NULL != TagFile.ID3v2Tag()) {
		temp = GetEncodingString(TagFile.ID3v2Tag()->title().toCString(true));
		_tcsncpy_s(Cache.Title, MAX_MUSICTEXT - 1, (LPCTSTR)temp, _TRUNCATE);
		temp = GetEncodingString(TagFile.ID3v2Tag()->artist().toCString(true));
		_tcsncpy_s(Cache.Artist, MAX_MUSICTEXT - 1, (LPCTSTR)temp, _TRUNCATE);
		temp = GetEncodingString(TagFile.ID3v2Tag()->albumArtist().toCString(true));
		_tcsncpy_s(Cache.AlbumArtist, MAX_MUSICTEXT - 1, (LPCTSTR)temp, _TRUNCATE);
		temp = GetEncodingString(TagFile.ID3v2Tag()->comment().toCString(true));
		_tcsncpy_s(Cache.Comment, MAX_MUSICTEXT -1, (LPCTSTR)temp, _TRUNCATE);
		temp = GetEncodingString(TagFile.ID3v2Tag()->album().toCString(true));
		_tcsncpy_s(Cache.Album, MAX_MUSICTEXT - 1, (LPCTSTR)temp, _TRUNCATE);
		temp = GetEncodingString(TagFile.ID3v2Tag()->stringYear().toCString(true));
		_tcsncpy_s(Cache.Year, MAX_YEAR + 1, (LPCTSTR)temp, _TRUNCATE);
		temp = GetEncodingString(TagFile.ID3v2Tag()->genre().toCString(true));
		_tcsncpy_s(Cache.Genre, MAX_MUSICTEXT - 1, (LPCTSTR)temp, _TRUNCATE);
		temp = GetEncodingString(TagFile.ID3v2Tag()->stringTrack().toCString(true));
		_tcsncpy_s(Cache.Track, MAX_MUSICTEXT - 1, (LPCTSTR)temp, _TRUNCATE);
		temp = GetEncodingString(TagFile.ID3v2Tag()->composers().toCString(true));
		_tcsncpy_s(Cache.Composer, MAX_MUSICTEXT - 1, (LPCTSTR)temp, _TRUNCATE);
		temp = GetEncodingString(TagFile.ID3v2Tag()->origArtist().toCString(true));
		_tcsncpy_s(Cache.OrgArtist, MAX_MUSICTEXT - 1, (LPCTSTR)temp, _TRUNCATE);
		temp = GetEncodingString(TagFile.ID3v2Tag()->copyright().toCString(true));
		_tcsncpy_s(Cache.Copyright, MAX_MUSICTEXT - 1, (LPCTSTR)temp, _TRUNCATE);
		temp = GetEncodingString(TagFile.ID3v2Tag()->publisher().toCString(true));
		_tcsncpy_s(Cache.Publisher, MAX_MUSICTEXT - 1, (LPCTSTR)temp, _TRUNCATE);
		temp = GetEncodingString(TagFile.ID3v2Tag()->disc().toCString(true));
		_tcsncpy_s(Cache.Disc, MAX_MUSICTEXT - 1, (LPCTSTR)temp, _TRUNCATE);
		temp = GetEncodingString(TagFile.ID3v2Tag()->BPM().toCString(true));
		_tcsncpy_s(Cache.BPM, MAX_MUSICTEXT - 1, (LPCTSTR)temp, _TRUNCATE);

		// read Album Art
		TagLib::String mimeType;
		TagLib::ByteVector AlbumArt = 
			TagFile.ID3v2Tag()->albumArt(TagLib::ID3v2::AttachedPictureFrame::FrontCover, mimeType);
		if(AlbumArt != TagLib::ByteVector::null) {
			size_t tag_size = AlbumArt.size();

			if (tag_size) {
				Cache.E_Image = new Embedded_Image;
				Cache.E_Image->Image = new char[tag_size];
				memcpy_s(Cache.E_Image->Image, tag_size, AlbumArt.data(), tag_size);
				Cache.E_Image->size = tag_size;
				Cache.E_Image->mimeType = mimeType;
			} else {
				Cache.E_Image = NULL;
			}
		} else {
			Cache.E_Image = NULL;
		}

	} else if (NULL != TagFile.ID3v1Tag()) {
		temp = TagFile.ID3v1Tag()->title().toCString(false);
		_tcsncpy_s(Cache.Title, MAX_MUSICTEXT - 1, (LPCTSTR)temp, _TRUNCATE);
		temp = TagFile.ID3v1Tag()->artist().toCString(false);
		_tcsncpy_s(Cache.Artist, MAX_MUSICTEXT - 1, (LPCTSTR)temp, _TRUNCATE);
		temp = TagFile.ID3v1Tag()->comment().toCString(false);
		_tcsncpy_s(Cache.Comment, MAX_MUSICTEXT -1, (LPCTSTR)temp, _TRUNCATE);
		temp = TagFile.ID3v1Tag()->album().toCString(false);
		_tcsncpy_s(Cache.Album, MAX_MUSICTEXT - 1, (LPCTSTR)temp, _TRUNCATE);
		temp.Format(_T("%d"), TagFile.ID3v1Tag()->year());
		_tcsncpy_s(Cache.Year, MAX_YEAR + 1, (LPCTSTR)temp, _TRUNCATE);
		temp = TagFile.ID3v1Tag()->genre().toCString(false);
		_tcsncpy_s(Cache.Genre, MAX_MUSICTEXT - 1, (LPCTSTR)temp, _TRUNCATE);
		temp.Format(_T("%d"), TagFile.ID3v1Tag()->track());
		_tcsncpy_s(Cache.Track, MAX_MUSICTEXT - 1, (LPCTSTR)temp, _TRUNCATE);

	} else { 
		// do nothing.
	}

	return true;
}

int CMediaLibrary::GetExtendedFileInfo(const char *fn, const char *data, char *dest, size_t destlen)
{

	bool FindTag;
	int RetCode;

    if (_stricmp(fn, Cache.FileName) != 0) {
		::EnterCriticalSection(&CriticalSection);

		::strncpy_s(Cache.FileName, MAX_PATHLEN - 1, fn, MAX_PATHLEN - 1);

		FindTag = GetTagInfo();
		::LeaveCriticalSection(&CriticalSection);

	} else {
		FindTag = true;
	}

	if (FindTag) {
		char	Buff[MAX_MUSICTEXT];
		char   *RetBuff;
		const char *MetaData = data;

		if(_stricmp(MetaData, "length") == 0) {
			_ultoa_s(Cache.Length , Buff, sizeof(Buff), 10);
			RetBuff = Buff;
			RetCode = 1;
		} else if(_stricmp(MetaData, "formatinformation") == 0) {
			RetBuff = Cache.Format;
			RetCode = 1;
		} else if(_stricmp(MetaData, "type") == 0) {
			Buff[0] = '0';
			Buff[1] = 0;
			RetBuff = Buff;
			RetCode = 1;
		} else if(_stricmp(MetaData, "family") == 0) {
			_tcsncpy_s(Buff, MAX_MUSICTEXT - 1, _T("The True Audio File"), _TRUNCATE);
			RetBuff = Buff;
			RetCode = 1;
		} else if(_stricmp(MetaData, "lossless") == 0) {
			Buff[0] = '1';
			RetBuff = Buff;
			RetCode = 1;
		} else if(_stricmp(MetaData, "title") == 0) {
			RetBuff = Cache.Title;
			RetCode = 1;
		} else if(_stricmp(MetaData, "artist") == 0) {
			RetBuff = Cache.Artist;
			RetCode = 1;
		} else if(_stricmp(MetaData, "albumartist") == 0) {
			RetBuff = Cache.AlbumArtist;
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
		} else if(_stricmp(MetaData, "composer") == 0) {
			RetBuff = Cache.Composer;
			RetCode = 1; 
		} else if(_stricmp(MetaData, "publisher") == 0) {
			RetBuff = Cache.Publisher;
			RetCode = 1;
		} else if(_stricmp(MetaData, "disc") == 0) {
			RetBuff = Cache.Disc;
			RetCode = 1;
		} else if(_stricmp(MetaData, "bpm") == 0) {
			RetBuff = Cache.BPM;
			RetCode = 1;
		} else {
			RetCode = 0;
		}

		if(RetCode && (destlen != 0)) {
			_tcsncpy_s(dest, destlen, RetBuff, _TRUNCATE);
		}
	} else {
		Cache.FileName[0] = '\0';
		RetCode = 0;
	}

	return RetCode;
}

int CMediaLibrary::SetExtendedFileInfo(const char *fn, const char *MetaData, const char *val)
{

	bool FindTag = false;
	int RetCode = 0;

    if (_stricmp(fn, Cache.FileName) != 0) {
		::EnterCriticalSection(&CriticalSection);

		::strncpy_s(Cache.FileName, MAX_PATHLEN - 1, fn, MAX_PATHLEN - 1);

		FindTag = GetTagInfo();
		::LeaveCriticalSection(&CriticalSection);

	} else {
		FindTag = true;
	}

	if (FindTag) {
		char   *SetBuff;

		if(_stricmp(MetaData, "title") == 0) {
			SetBuff = Cache.Title;
			RetCode = 1;
		} else if(_stricmp(MetaData, "artist") == 0) {
			SetBuff = Cache.Artist;
			RetCode = 1;
		} else if(_stricmp(MetaData, "albumartist") == 0) {
			SetBuff = Cache.AlbumArtist;
			RetCode = 1;
		} else if(_stricmp(MetaData, "comment") == 0) {
			SetBuff = Cache.Comment;
			RetCode = 1;
		} else if(_stricmp(MetaData, "album") == 0) {
			SetBuff = Cache.Album;
			RetCode = 1;
		} else if(_stricmp(MetaData, "year") == 0) {
			SetBuff = Cache.Year;
			RetCode = 1;
		} else if(_stricmp(MetaData, "genre") == 0) {
			SetBuff = Cache.Genre;
			RetCode = 1;
		} else if(_stricmp(MetaData, "track") == 0) {
			SetBuff = Cache.Track;
			RetCode = 1;
		} else if(_stricmp(MetaData, "composer") == 0) {
			SetBuff = Cache.Composer;
			RetCode = 1; 
		} else if(_stricmp(MetaData, "publisher") == 0) {
			SetBuff = Cache.Publisher;
			RetCode = 1;
		} else if(_stricmp(MetaData, "disc") == 0) {
			SetBuff = Cache.Disc;
			RetCode = 1;
		} else if(_stricmp(MetaData, "bpm") == 0) {
			SetBuff = Cache.BPM;
			RetCode = 1;
		} else {
			RetCode = 0;
		}

		if(RetCode) {
			_tcsncpy_s(SetBuff, MAX_MUSICTEXT, val, _TRUNCATE);
		}
	} else {
		Cache.FileName[0] = '\0';
		RetCode = 0;
	}


	return RetCode;
}

int CMediaLibrary::WriteExtendedFileInfo()
{

    ::EnterCriticalSection(&CriticalSection);

	if (*Cache.FileName == NULL) {
		return 0;
	}

	// If target file cannot access
	if (TagLib::File::isWritable(Cache.FileName) == false) {
		return 0;
	}

	TagLib::TrueAudio::File TagFile(Cache.FileName);

	if (NULL != TagFile.ID3v2Tag()) {
		TagLib::String temp;
		temp = TagLib::String(SetEncodingString(Cache.Title), TagLib::String::UTF8);
		TagFile.ID3v2Tag()->setTitle(temp);
		temp = TagLib::String(SetEncodingString(Cache.Artist), TagLib::String::UTF8);
		TagFile.ID3v2Tag()->setArtist(temp);
		temp = TagLib::String(SetEncodingString(Cache.AlbumArtist), TagLib::String::UTF8);
		TagFile.ID3v2Tag()->setAlbumArtist(temp);
		temp = TagLib::String(SetEncodingString(Cache.Comment), TagLib::String::UTF8);
		TagFile.ID3v2Tag()->setComment(temp);
		temp = TagLib::String(SetEncodingString(Cache.Album), TagLib::String::UTF8);
		TagFile.ID3v2Tag()->setAlbum(temp);
		temp = TagLib::String(SetEncodingString(Cache.Year), TagLib::String::UTF8);
		TagFile.ID3v2Tag()->setStringYear(temp);
		temp = TagLib::String(SetEncodingString(Cache.Genre), TagLib::String::UTF8);
		TagFile.ID3v2Tag()->setGenre(temp);
		temp = TagLib::String(SetEncodingString(Cache.Track), TagLib::String::UTF8);
		TagFile.ID3v2Tag()->setStringTrack(temp);
		temp = TagLib::String(SetEncodingString(Cache.Composer), TagLib::String::UTF8);
		TagFile.ID3v2Tag()->setComposers(temp);
		temp = TagLib::String(SetEncodingString(Cache.OrgArtist), TagLib::String::UTF8);
		TagFile.ID3v2Tag()->setOrigArtist(temp);
		temp = TagLib::String(SetEncodingString(Cache.Copyright), TagLib::String::UTF8);
		TagFile.ID3v2Tag()->setCopyright(temp);
		temp = TagLib::String(SetEncodingString(Cache.Publisher), TagLib::String::UTF8);
		TagFile.ID3v2Tag()->setPublisher(temp);
		temp = TagLib::String(SetEncodingString(Cache.Disc), TagLib::String::UTF8);
		TagFile.ID3v2Tag()->setDisc(temp);
		temp = TagLib::String(SetEncodingString(Cache.BPM), TagLib::String::UTF8);
		TagFile.ID3v2Tag()->setBPM(temp);

	} else if (NULL != TagFile.ID3v1Tag()) {
		TagFile.ID3v1Tag()->setTitle(Cache.Title);
		TagFile.ID3v1Tag()->setArtist(Cache.Artist);
		TagFile.ID3v1Tag()->setAlbum(Cache.Album);
		TagFile.ID3v1Tag()->setComment(Cache.Comment);
		TagFile.ID3v1Tag()->setYear(_ttoi(Cache.Year));
		TagFile.ID3v1Tag()->setTrack(_ttoi(Cache.Track));
		TagFile.ID3v1Tag()->setGenre(Cache.Genre);
	} else { 
		// do nothing.
	}

	TagFile.save();

	::LeaveCriticalSection(&CriticalSection);

	return 1;
}

int CMediaLibrary::GetAlbumArtData(const wchar_t *filename, const wchar_t *type, void **bits, size_t *len, wchar_t **mime_type)
{
	bool FindTag = false;
	size_t tag_size = 0;
    int retval = ALBUMARTPROVIDER_FAILURE;
	TagLib::String mimeType;
	char demandFile[MAX_PATHLEN];

	if(!filename || !*filename || _wcsicmp (type, L"cover")) {
        return retval;
	}

	if(!bits || !len || !mime_type) {
		return retval;
	}

	size_t origsize = wcslen(filename) + 1;
	size_t convertedChars = 0;
	int ret = WideCharToMultiByte(CP_ACP, 0, filename, origsize, demandFile, MAX_PATHLEN - 1, NULL, NULL);

	if(!ret) {
		return retval;
	}

    if (_stricmp(demandFile, Cache.FileName) != 0) {
		::EnterCriticalSection(&CriticalSection);

		::strncpy_s(Cache.FileName, MAX_PATHLEN - 1, (LPCTSTR)demandFile, MAX_PATHLEN - 1);

		FindTag = GetTagInfo();
		::LeaveCriticalSection(&CriticalSection);
	} else {
		FindTag = true;
	}

	if(!FindTag) {
		return retval;
	}

	if (Cache.E_Image == NULL) {
		return retval;
	} else if(Cache.E_Image->Image != NULL) {

		*bits = (char *)Wasabi_Malloc(Cache.E_Image->size);
		memcpy_s(*bits, Cache.E_Image->size, Cache.E_Image->Image, Cache.E_Image->size);
		*len = Cache.E_Image->size;

		retval = ALBUMARTPROVIDER_SUCCESS;

		size_t string_len;
		TagLib::String extension = Cache.E_Image->mimeType.substr(Cache.E_Image->mimeType.find("/") + 1);
		*mime_type = (wchar_t *)Wasabi_Malloc(extension.size() * 2 + 2);
		mbstowcs_s(&string_len, *mime_type, extension.size() + 1, extension.toCString(), _TRUNCATE);
	}

	if (retval) {
		Wasabi_Free(*bits);
	}

    return retval;
}
