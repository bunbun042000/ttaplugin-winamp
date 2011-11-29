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
#include <sstream>
#include <iomanip>

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
	FlushCache();

	::DeleteCriticalSection(&CriticalSection);

}

void CMediaLibrary::FlushCache(void)
{
	::EnterCriticalSection(&CriticalSection);

	Cache.FileName[0] = '\0';
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

	::LeaveCriticalSection(&CriticalSection);
}

bool CMediaLibrary::GetTagInfo()
{

	if (Cache.FileName[0] == '\0') {
		return false;
	} else { 
		// do nothing
	}

	// If target file cannot access
	if (TagLib::File::isReadable(Cache.FileName) == false) {
		return false;
	} else {
		// do nothing
	}

	TagLib::TrueAudio::File TagFile(Cache.FileName);

	if (!TagFile.isValid()) {
		return false;
	} else {
		// do nothing
	}

	Cache.Length = (unsigned long) (TagFile.audioProperties()->length() * 1000.L);

	int Lengthbysec = TagFile.audioProperties()->length();
	int hour = Lengthbysec / 3600;
	int min  = Lengthbysec / 60;
	int sec  = Lengthbysec % 60;

	std::stringstream second;
	if(hour > 0) {
		second << std::setw(2) << std::setfill('0') << hour << ":" << std::setw(2) 
			<< std::setfill('0') << min << ":" << std::setw(2) << std::setfill('0') << sec;
	} else if(min > 0) {
		second << std::setw(2) << std::setfill('0') << min << ":" << std::setw(2) 
			<< std::setfill('0') << sec;
	} else {
		second << std::setw(2) << std::setfill('0') << sec;
	}

	std::string channel_designation = (TagFile.audioProperties()->channels() == 2) ? "Stereo" : "Monoral";

	std::stringstream ttainfo_temp;
	ttainfo_temp << "Format\t\t: TTA" << TagFile.audioProperties()->ttaVersion() 
		<< "\nSample\t\t: " << (int)TagFile.audioProperties()->bitsPerSample()
		<< "bit\nSample Rate\t: " << TagFile.audioProperties()->sampleRate()
		<< "Hz\nBit Rate\t\t: " << TagFile.audioProperties()->bitrate()
		<< "kbit/s\nNum. of Chan.\t: " << TagFile.audioProperties()->channels()
		<< "(" << channel_designation
		<< ")\nLength\t\t: " << second.str();
	strncpy_s(Cache.Format, MAX_MUSICTEXT - 1, ttainfo_temp.str().c_str(), _TRUNCATE);

	std::string temp;
	if (NULL != TagFile.ID3v2Tag()) {
		temp = GetEncodingString(TagFile.ID3v2Tag()->title().toCString(true));
		strncpy_s(Cache.Title, MAX_MUSICTEXT - 1, temp.c_str(), _TRUNCATE);
		temp = GetEncodingString(TagFile.ID3v2Tag()->artist().toCString(true));
		strncpy_s(Cache.Artist, MAX_MUSICTEXT - 1, temp.c_str(), _TRUNCATE);
		temp = GetEncodingString(TagFile.ID3v2Tag()->albumArtist().toCString(true));
		strncpy_s(Cache.AlbumArtist, MAX_MUSICTEXT - 1, temp.c_str(), _TRUNCATE);
		temp = GetEncodingString(TagFile.ID3v2Tag()->comment().toCString(true));
		strncpy_s(Cache.Comment, MAX_MUSICTEXT -1, temp.c_str(), _TRUNCATE);
		temp = GetEncodingString(TagFile.ID3v2Tag()->album().toCString(true));
		strncpy_s(Cache.Album, MAX_MUSICTEXT - 1, temp.c_str(), _TRUNCATE);
		temp = GetEncodingString(TagFile.ID3v2Tag()->stringYear().toCString(true));
		strncpy_s(Cache.Year, MAX_YEAR + 1, temp.c_str(), _TRUNCATE);
		temp = GetEncodingString(TagFile.ID3v2Tag()->genre().toCString(true));
		strncpy_s(Cache.Genre, MAX_MUSICTEXT - 1, temp.c_str(), _TRUNCATE);
		temp = GetEncodingString(TagFile.ID3v2Tag()->stringTrack().toCString(true));
		strncpy_s(Cache.Track, MAX_MUSICTEXT - 1, temp.c_str(), _TRUNCATE);
		temp = GetEncodingString(TagFile.ID3v2Tag()->composers().toCString(true));
		strncpy_s(Cache.Composer, MAX_MUSICTEXT - 1, temp.c_str(), _TRUNCATE);
		temp = GetEncodingString(TagFile.ID3v2Tag()->origArtist().toCString(true));
		strncpy_s(Cache.OrgArtist, MAX_MUSICTEXT - 1, temp.c_str(), _TRUNCATE);
		temp = GetEncodingString(TagFile.ID3v2Tag()->copyright().toCString(true));
		strncpy_s(Cache.Copyright, MAX_MUSICTEXT - 1, temp.c_str(), _TRUNCATE);
		temp = GetEncodingString(TagFile.ID3v2Tag()->publisher().toCString(true));
		strncpy_s(Cache.Publisher, MAX_MUSICTEXT - 1, temp.c_str(), _TRUNCATE);
		temp = GetEncodingString(TagFile.ID3v2Tag()->disc().toCString(true));
		strncpy_s(Cache.Disc, MAX_MUSICTEXT - 1, temp.c_str(), _TRUNCATE);
		temp = GetEncodingString(TagFile.ID3v2Tag()->BPM().toCString(true));
		strncpy_s(Cache.BPM, MAX_MUSICTEXT - 1, temp.c_str(), _TRUNCATE);


	} else if (NULL != TagFile.ID3v1Tag()) {

		std::stringstream temp_year;
		std::stringstream temp_track;
		temp = TagFile.ID3v1Tag()->title().toCString(false);
		strncpy_s(Cache.Title, MAX_MUSICTEXT - 1, temp.c_str(), _TRUNCATE);
		temp = TagFile.ID3v1Tag()->artist().toCString(false);
		strncpy_s(Cache.Artist, MAX_MUSICTEXT - 1, temp.c_str(), _TRUNCATE);
		temp = TagFile.ID3v1Tag()->comment().toCString(false);
		strncpy_s(Cache.Comment, MAX_MUSICTEXT -1, temp.c_str(), _TRUNCATE);
		temp = TagFile.ID3v1Tag()->album().toCString(false);
		strncpy_s(Cache.Album, MAX_MUSICTEXT - 1, temp.c_str(), _TRUNCATE);
		temp_year << TagFile.ID3v1Tag()->year();
		strncpy_s(Cache.Year, MAX_YEAR + 1, temp_year.str().c_str(), _TRUNCATE);
		temp = TagFile.ID3v1Tag()->genre().toCString(false);
		strncpy_s(Cache.Genre, MAX_MUSICTEXT - 1, temp.c_str(), _TRUNCATE);
		temp_track << TagFile.ID3v1Tag()->track();
		strncpy_s(Cache.Track, MAX_MUSICTEXT - 1, temp_track.str().c_str(), _TRUNCATE);

	} else { 
		// do nothing.
	}

	return true;
}

int CMediaLibrary::GetExtendedFileInfo(const char *fn, const char *data, char *dest, size_t destlen)
{

	bool FindTag;
	int RetCode;

	::EnterCriticalSection(&CriticalSection);

	if (_stricmp(fn, Cache.FileName) != 0) {
		
		::strncpy_s(Cache.FileName, MAX_PATHLEN - 1, fn, MAX_PATHLEN - 1);
		FindTag = GetTagInfo();

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
			strncpy_s(Buff, MAX_MUSICTEXT - 1, "The True Audio File", _TRUNCATE);
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
			strncpy_s(dest, destlen, RetBuff, _TRUNCATE);
		}
	} else {
		Cache.FileName[0] = '\0';
		RetCode = 0;
	}

	::LeaveCriticalSection(&CriticalSection);
	return RetCode;
}

int CMediaLibrary::SetExtendedFileInfo(const char *fn, const char *MetaData, const char *val)
{

	bool FindTag = false;
	int RetCode = 0;

	::EnterCriticalSection(&CriticalSection);

	if (_stricmp(fn, Cache.FileName) != 0) {

		::strncpy_s(Cache.FileName, MAX_PATHLEN - 1, fn, MAX_PATHLEN - 1);

		FindTag = GetTagInfo();

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
			strncpy_s(SetBuff, MAX_MUSICTEXT, val, _TRUNCATE);
		}
	} else {
		Cache.FileName[0] = '\0';
		RetCode = 0;
	}

	::LeaveCriticalSection(&CriticalSection);
	return RetCode;
}

int CMediaLibrary::WriteExtendedFileInfo()
{
	
	if (Cache.FileName[0] == '\0') {
		return 0;
	}

	// If target file cannot access
	if (TagLib::File::isWritable(Cache.FileName) == false) {
		return 0;
	}

    ::EnterCriticalSection(&CriticalSection);

	TagLib::TrueAudio::File TagFile(Cache.FileName);
	if (!TagFile.isValid()) {
		return 0;
	} else {
		// do nothing
	}

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
		TagFile.ID3v1Tag()->setYear(atoi(Cache.Year));
		TagFile.ID3v1Tag()->setTrack(atoi(Cache.Track));
		TagFile.ID3v1Tag()->setGenre(Cache.Genre);
	} else { 
		// do nothing.
	}

	TagFile.save();

	::LeaveCriticalSection(&CriticalSection);

	return 1;
}
