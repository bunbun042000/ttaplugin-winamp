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

	GetTagTime = 0;
	TagData.Title[0] = '\0';
	TagData.Artist[0] = '\0';
	TagData.Comment[0] = '\0';
	TagData.Album[0] = '\0';
	TagData.AlbumArtist[0] = '\0';
	TagData.Year[0] = '\0';
	TagData.Genre[0] = '\0';
	TagData.Track[0] = '\0';
	TagData.Composer[0] = '\0';
	TagData.OrgArtist[0] = '\0';
	TagData.Copyright[0] = '\0';
	TagData.Encoder[0] = '\0';
	TagData.Publisher[0] = '\0';
	TagData.Disc[0] = '\0';
	TagData.BPM[0] = '\0';

	FileName = "";

	if (TTAFile != NULL)
	{
		delete TTAFile;
		TTAFile = NULL;
	}
	else
	{
		// Do nothing
	}

	::LeaveCriticalSection(&CriticalSection);
}

bool CMediaLibrary::GetTagInfo(const std::string fn)
{

	if (TTAFile != NULL && FileName != fn)
	{
		delete TTAFile;
		TTAFile = NULL;
	}
	else
	{
		// Do nothing
	}

	if (TTAFile == NULL)
	{
		TTAFile = new TagLib::TrueAudio::File(fn.c_str());

		if (!TTAFile->isValid())
		{
			return false;
		}
		else
		{
			FileName = fn;
		}
	}
	else
	{
		// Do nothing
	}


	TagData.Length = (unsigned long) (TTAFile->audioProperties()->length() * 1000.L);

	int Lengthbysec = TTAFile->audioProperties()->length();
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

	std::string channel_designation = (TTAFile->audioProperties()->channels() == 2) ? "Stereo" : "Monoral";

	std::stringstream ttainfo_temp;
	ttainfo_temp << "Format\t\t: TTA" << TTAFile->audioProperties()->ttaVersion() 
		<< "\nSample\t\t: " << (int)TTAFile->audioProperties()->bitsPerSample()
		<< "bit\nSample Rate\t: " << TTAFile->audioProperties()->sampleRate()
		<< "Hz\nBit Rate\t\t: " << TTAFile->audioProperties()->bitrate()
		<< "kbit/s\nNum. of Chan.\t: " << TTAFile->audioProperties()->channels()
		<< "(" << channel_designation
		<< ")\nLength\t\t: " << second.str();
	TagData.Format = ttainfo_temp.str();

	std::string temp;
	if (NULL != TTAFile->ID3v2Tag()) {
		temp = GetEncodingString(TTAFile->ID3v2Tag()->title().toCString(true));
		strncpy_s(TagData.Title, MAX_MUSICTEXT - 1, temp.c_str(), _TRUNCATE);
		temp = GetEncodingString(TTAFile->ID3v2Tag()->artist().toCString(true));
		strncpy_s(TagData.Artist, MAX_MUSICTEXT - 1, temp.c_str(), _TRUNCATE);
		temp = GetEncodingString(TTAFile->ID3v2Tag()->albumArtist().toCString(true));
		strncpy_s(TagData.AlbumArtist, MAX_MUSICTEXT - 1, temp.c_str(), _TRUNCATE);
		temp = GetEncodingString(TTAFile->ID3v2Tag()->comment().toCString(true));
		strncpy_s(TagData.Comment, MAX_MUSICTEXT -1, temp.c_str(), _TRUNCATE);
		temp = GetEncodingString(TTAFile->ID3v2Tag()->album().toCString(true));
		strncpy_s(TagData.Album, MAX_MUSICTEXT - 1, temp.c_str(), _TRUNCATE);
		temp = GetEncodingString(TTAFile->ID3v2Tag()->stringYear().toCString(true));
		strncpy_s(TagData.Year, MAX_YEAR + 1, temp.c_str(), _TRUNCATE);
		temp = GetEncodingString(TTAFile->ID3v2Tag()->genre().toCString(true));
		strncpy_s(TagData.Genre, MAX_MUSICTEXT - 1, temp.c_str(), _TRUNCATE);
		temp = GetEncodingString(TTAFile->ID3v2Tag()->stringTrack().toCString(true));
		strncpy_s(TagData.Track, MAX_MUSICTEXT - 1, temp.c_str(), _TRUNCATE);
		temp = GetEncodingString(TTAFile->ID3v2Tag()->composers().toCString(true));
		strncpy_s(TagData.Composer, MAX_MUSICTEXT - 1, temp.c_str(), _TRUNCATE);
		temp = GetEncodingString(TTAFile->ID3v2Tag()->origArtist().toCString(true));
		strncpy_s(TagData.OrgArtist, MAX_MUSICTEXT - 1, temp.c_str(), _TRUNCATE);
		temp = GetEncodingString(TTAFile->ID3v2Tag()->copyright().toCString(true));
		strncpy_s(TagData.Copyright, MAX_MUSICTEXT - 1, temp.c_str(), _TRUNCATE);
		temp = GetEncodingString(TTAFile->ID3v2Tag()->publisher().toCString(true));
		strncpy_s(TagData.Publisher, MAX_MUSICTEXT - 1, temp.c_str(), _TRUNCATE);
		temp = GetEncodingString(TTAFile->ID3v2Tag()->disc().toCString(true));
		strncpy_s(TagData.Disc, MAX_MUSICTEXT - 1, temp.c_str(), _TRUNCATE);
		temp = GetEncodingString(TTAFile->ID3v2Tag()->BPM().toCString(true));
		strncpy_s(TagData.BPM, MAX_MUSICTEXT - 1, temp.c_str(), _TRUNCATE);


	} else if (NULL != TTAFile->ID3v1Tag()) {

		std::stringstream temp_year;
		std::stringstream temp_track;
		temp = TTAFile->ID3v1Tag()->title().toCString(false);
		strncpy_s(TagData.Title, MAX_MUSICTEXT - 1, temp.c_str(), _TRUNCATE);
		temp = TTAFile->ID3v1Tag()->artist().toCString(false);
		strncpy_s(TagData.Artist, MAX_MUSICTEXT - 1, temp.c_str(), _TRUNCATE);
		temp = TTAFile->ID3v1Tag()->comment().toCString(false);
		strncpy_s(TagData.Comment, MAX_MUSICTEXT -1, temp.c_str(), _TRUNCATE);
		temp = TTAFile->ID3v1Tag()->album().toCString(false);
		strncpy_s(TagData.Album, MAX_MUSICTEXT - 1, temp.c_str(), _TRUNCATE);
		temp_year << TTAFile->ID3v1Tag()->year();
		strncpy_s(TagData.Year, MAX_YEAR + 1, temp_year.str().c_str(), _TRUNCATE);
		temp = TTAFile->ID3v1Tag()->genre().toCString(false);
		strncpy_s(TagData.Genre, MAX_MUSICTEXT - 1, temp.c_str(), _TRUNCATE);
		temp_track << TTAFile->ID3v1Tag()->track();
		strncpy_s(TagData.Track, MAX_MUSICTEXT - 1, temp_track.str().c_str(), _TRUNCATE);

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

	if (std::string(fn) != FileName)
	{
		FindTag = GetTagInfo(fn);
	}
	else
	{
		FindTag = true;
	}

	if (FindTag) {
		char	Buff[MAX_MUSICTEXT];
		char   *RetBuff;
		const char *MetaData = data;

		if(_stricmp(MetaData, "length") == 0) {
			_ultoa_s(TagData.Length , Buff, sizeof(Buff), 10);
			strncpy_s(dest, destlen, Buff, _TRUNCATE);
			RetCode = 1;
		} else if(_stricmp(MetaData, "formatinformation") == 0) {
			strncpy_s(dest, destlen, TagData.Format.c_str(), _TRUNCATE);
			RetCode = 1;
		} else if(_stricmp(MetaData, "type") == 0) {
			Buff[0] = '0';
			Buff[1] = 0;
			RetBuff = Buff;
			strncpy_s(dest, destlen, Buff, _TRUNCATE);
			RetCode = 1;
		} else if(_stricmp(MetaData, "family") == 0) {
			strncpy_s(dest, destlen, "The True Audio File", _TRUNCATE);
			RetCode = 1;
		} else if(_stricmp(MetaData, "lossless") == 0) {
			Buff[0] = '1';
			strncpy_s(dest, destlen, Buff, _TRUNCATE);
			RetCode = 1;
		} else if(_stricmp(MetaData, "title") == 0) {
			strncpy_s(dest, destlen, TagData.Title, _TRUNCATE);
			RetCode = 1;
		} else if(_stricmp(MetaData, "artist") == 0) {
			strncpy_s(dest, destlen, TagData.Artist, _TRUNCATE);
			RetCode = 1;
		} else if(_stricmp(MetaData, "albumartist") == 0) {
			strncpy_s(dest, destlen, TagData.AlbumArtist, _TRUNCATE);
			RetCode = 1;
		} else if(_stricmp(MetaData, "comment") == 0) {
			strncpy_s(dest, destlen, TagData.Comment, _TRUNCATE);
			RetCode = 1;
		} else if(_stricmp(MetaData, "album") == 0) {
			strncpy_s(dest, destlen, TagData.Album, _TRUNCATE);
			RetCode = 1;
		} else if(_stricmp(MetaData, "year") == 0) {
			strncpy_s(dest, destlen, TagData.Year, _TRUNCATE);
			RetCode = 1;
		} else if(_stricmp(MetaData, "genre") == 0) {
			strncpy_s(dest, destlen, TagData.Genre, _TRUNCATE);
			RetCode = 1;
		} else if(_stricmp(MetaData, "track") == 0) {
			strncpy_s(dest, destlen, TagData.Track, _TRUNCATE);
			RetCode = 1;
		} else if(_stricmp(MetaData, "composer") == 0) {
			strncpy_s(dest, destlen, TagData.Composer, _TRUNCATE);
			RetCode = 1; 
		} else if(_stricmp(MetaData, "publisher") == 0) {
			strncpy_s(dest, destlen, TagData.Publisher, _TRUNCATE);
			RetCode = 1;
		} else if(_stricmp(MetaData, "disc") == 0) {
			strncpy_s(dest, destlen, TagData.Disc, _TRUNCATE);
			RetCode = 1;
		} else if(_stricmp(MetaData, "bpm") == 0) {
			strncpy_s(dest, destlen, TagData.BPM, _TRUNCATE);
			RetCode = 1;
		} else {
			RetCode = 0;
		}

	} else {
		FileName = "";
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

	if (std::string(fn) != FileName)
	{
		FindTag = GetTagInfo(FileName);
	}
	else
	{
		FindTag = true;
	}

	if (FindTag) {

		if(_stricmp(MetaData, "title") == 0) {
			strncpy_s(TagData.Title, MAX_MUSICTEXT - 1, val, _TRUNCATE);
			RetCode = 1;
		} else if(_stricmp(MetaData, "artist") == 0) {
			strncpy_s(TagData.Artist, MAX_MUSICTEXT - 1, val, _TRUNCATE);
			RetCode = 1;
		} else if(_stricmp(MetaData, "albumartist") == 0) {
			strncpy_s(TagData.AlbumArtist, MAX_MUSICTEXT - 1, val, _TRUNCATE);
			RetCode = 1;
		} else if(_stricmp(MetaData, "comment") == 0) {
			strncpy_s(TagData.Comment, MAX_MUSICTEXT - 1, val, _TRUNCATE);
			RetCode = 1;
		} else if(_stricmp(MetaData, "album") == 0) {
			strncpy_s(TagData.Album, MAX_MUSICTEXT - 1, val, _TRUNCATE);
			RetCode = 1;
		} else if(_stricmp(MetaData, "year") == 0) {
			strncpy_s(TagData.Year, MAX_MUSICTEXT - 1, val, _TRUNCATE);
			RetCode = 1;
		} else if(_stricmp(MetaData, "genre") == 0) {
			strncpy_s(TagData.Genre, MAX_MUSICTEXT - 1, val, _TRUNCATE);
			RetCode = 1;
		} else if(_stricmp(MetaData, "track") == 0) {
			strncpy_s(TagData.Track, MAX_MUSICTEXT - 1, val, _TRUNCATE);
			RetCode = 1;
		} else if(_stricmp(MetaData, "composer") == 0) {
			strncpy_s(TagData.Composer, MAX_MUSICTEXT - 1, val, _TRUNCATE);
			RetCode = 1; 
		} else if(_stricmp(MetaData, "publisher") == 0) {
			strncpy_s(TagData.Publisher, MAX_MUSICTEXT - 1, val, _TRUNCATE);
			RetCode = 1;
		} else if(_stricmp(MetaData, "disc") == 0) {
			strncpy_s(TagData.Disc, MAX_MUSICTEXT - 1, val, _TRUNCATE);
			RetCode = 1;
		} else if(_stricmp(MetaData, "bpm") == 0) {
			strncpy_s(TagData.BPM, MAX_MUSICTEXT - 1, val, _TRUNCATE);
			RetCode = 1;
		} else {
			RetCode = 0;
		}

	} else {
		FileName = "";
		RetCode = 0;
	}

	::LeaveCriticalSection(&CriticalSection);
	return RetCode;
}

int CMediaLibrary::WriteExtendedFileInfo()
{
	
	if (FileName.empty())
	{
		return 0;
	}
	else
	{
		// Do nothing
	}

    ::EnterCriticalSection(&CriticalSection);

	if (!TTAFile->isValid()) {
		return 0;
	} else {
		// do nothing
	}

	if (NULL != TTAFile->ID3v2Tag()) {
		TagLib::String temp;
		temp = TagLib::String(SetEncodingString(TagData.Title), TagLib::String::UTF8);
		TTAFile->ID3v2Tag()->setTitle(temp);
		temp = TagLib::String(SetEncodingString(TagData.Artist), TagLib::String::UTF8);
		TTAFile->ID3v2Tag()->setArtist(temp);
		temp = TagLib::String(SetEncodingString(TagData.AlbumArtist), TagLib::String::UTF8);
		TTAFile->ID3v2Tag()->setAlbumArtist(temp);
		temp = TagLib::String(SetEncodingString(TagData.Comment), TagLib::String::UTF8);
		TTAFile->ID3v2Tag()->setComment(temp);
		temp = TagLib::String(SetEncodingString(TagData.Album), TagLib::String::UTF8);
		TTAFile->ID3v2Tag()->setAlbum(temp);
		temp = TagLib::String(SetEncodingString(TagData.Year), TagLib::String::UTF8);
		TTAFile->ID3v2Tag()->setStringYear(temp);
		temp = TagLib::String(SetEncodingString(TagData.Genre), TagLib::String::UTF8);
		TTAFile->ID3v2Tag()->setGenre(temp);
		temp = TagLib::String(SetEncodingString(TagData.Track), TagLib::String::UTF8);
		TTAFile->ID3v2Tag()->setStringTrack(temp);
		temp = TagLib::String(SetEncodingString(TagData.Composer), TagLib::String::UTF8);
		TTAFile->ID3v2Tag()->setComposers(temp);
		temp = TagLib::String(SetEncodingString(TagData.OrgArtist), TagLib::String::UTF8);
		TTAFile->ID3v2Tag()->setOrigArtist(temp);
		temp = TagLib::String(SetEncodingString(TagData.Copyright), TagLib::String::UTF8);
		TTAFile->ID3v2Tag()->setCopyright(temp);
		temp = TagLib::String(SetEncodingString(TagData.Publisher), TagLib::String::UTF8);
		TTAFile->ID3v2Tag()->setPublisher(temp);
		temp = TagLib::String(SetEncodingString(TagData.Disc), TagLib::String::UTF8);
		TTAFile->ID3v2Tag()->setDisc(temp);
		temp = TagLib::String(SetEncodingString(TagData.BPM), TagLib::String::UTF8);
		TTAFile->ID3v2Tag()->setBPM(temp);

	} else if (NULL != TTAFile->ID3v1Tag()) {
		TTAFile->ID3v1Tag()->setTitle(TagData.Title);
		TTAFile->ID3v1Tag()->setArtist(TagData.Artist);
		TTAFile->ID3v1Tag()->setAlbum(TagData.Album);
		TTAFile->ID3v1Tag()->setComment(TagData.Comment);
		TTAFile->ID3v1Tag()->setYear(atoi(TagData.Year));
		TTAFile->ID3v1Tag()->setTrack(atoi(TagData.Track));
		TTAFile->ID3v1Tag()->setGenre(TagData.Genre);
	} else { 
		// do nothing.
	}

	TTAFile->save();

	::LeaveCriticalSection(&CriticalSection);

	return 1;
}
