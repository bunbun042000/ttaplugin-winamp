// TtaTag.cpp: CTtaTag クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////

// $ LastChangedDate: $

#include "stdafx.h"
#include "in_tta.h"
#include "TtaTag.h"
//#include "id3genre.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// 構築/消滅
//////////////////////////////////////////////////////////////////////

CTtaTag::CTtaTag()
{
	HFILE = INVALID_HANDLE_VALUE;

}

CTtaTag::~CTtaTag()
{
}

void CTtaTag::Flush()
{
	HFILE = INVALID_HANDLE_VALUE;
	FileName[0] = '\0';

}

void CTtaTag::CloseFile()
{
	if (HFILE != INVALID_HANDLE_VALUE)
		CloseHandle(HFILE);
}

bool CTtaTag::ReadTag(HWND hMainWindow, const char *filename)
{
//	unsigned long result;

	// File open
	::strncpy(FileName, filename, MAX_PATHLEN);

	HFILE = CreateFile(FileName, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (HFILE == INVALID_HANDLE_VALUE) {
		STATE = OPEN_ERROR;
		return false;
	}

	FILESIZE = ::GetFileSize(HFILE, NULL);

	//Read ID3v1.1
	id3v1.ReadTag(hMainWindow, FileName);

	//Read ID3v2.3
	id3v2.ReadTag(FileName);

	//Read TTA Header
	ReadTTAheader();

	return true;
}

int CTtaTag::ReadTTAheader()
{
	int           checksum, datasize, origsize;
	unsigned long result;

	// read TTA header
	if (!ReadFile(HFILE, &ttaheader, sizeof(TTA_header), &result, NULL) ||
		result != sizeof(TTA_header)) {
		CloseHandle(HFILE);
		STATE = READ_ERROR;
		return -1;
	}

	// check for TTA3 signature
	if (ttaheader.TTAid != TTA1_SIGN) {
		CloseHandle(HFILE);
		STATE = FORMAT_ERROR;
		return -1;
	}

	checksum = crc32((unsigned char *) &ttaheader,
	sizeof(TTA_header) - sizeof(long));
	if (checksum != ttaheader.CRC32) {
		CloseHandle(HFILE);
		STATE = FILE_ERROR;
		return -1;
	}

	// check for player supported formats
	if ((ttaheader.AudioFormat != WAVE_FORMAT_PCM &&
		ttaheader.AudioFormat != WAVE_FORMAT_IEEE_FLOAT &&
		ttaheader.AudioFormat != WAVE_FORMAT_EXTENSIBLE) ||
		ttaheader.BitsPerSample > MAX_BPS ||
		ttaheader.NumChannels > MAX_NCH) {
		CloseHandle(HFILE);
		STATE = PLAYER_ERROR;
		return -1;
	}

	// fill the File Info
	NCH = ttaheader.NumChannels;
	BPS = ttaheader.BitsPerSample;
	BSIZE = (ttaheader.BitsPerSample + 7)/8;
	FORMAT = ttaheader.AudioFormat;
	SAMPLERATE = ttaheader.SampleRate;
	DATALENGTH = ttaheader.DataLength;
	FRAMELEN = (long) (FRAME_TIME * ttaheader.SampleRate);
	LENGTH = ttaheader.DataLength / ttaheader.SampleRate * 1000;

	datasize = FILESIZE - id3v2.TagLength();
	origsize = DATALENGTH * BSIZE * NCH;

	COMPRESS = (float) datasize / origsize;
	BITRATE = (long) ((COMPRESS * SAMPLERATE * NCH * BPS) / 1000);
	return 0;

}


void CTtaTag::SetPlayTitle(char *title) {
	if (id3v2.hasTag() && 
		(id3v2.GetArtist() || id3v2.GetTitle())) {
		if (id3v2.GetArtist() && id3v2.GetTitle())
			wsprintf(title, "%s - %s", id3v2.GetArtist(), id3v2.GetTitle());
		else if (id3v2.GetArtist() && id3v2.GetAlbum())
			wsprintf(title, "%s - %s", id3v2.GetArtist(), id3v2.GetAlbum());
		else if (id3v2.GetArtist()) lstrcpy(title, id3v2.GetArtist());
		else if (id3v2.GetTitle()) lstrcpy(title, id3v2.GetTitle());
	} else if (id3v1.hasTag() &&
		(id3v1.GetArtist() || id3v1.GetTitle())) {
		if (id3v1.GetArtist() && id3v1.GetTitle())
			wsprintf(title, "%s - %s", id3v1.GetArtist(), id3v1.GetTitle());
		else if (id3v1.GetArtist() && id3v1.GetAlbum())
			wsprintf(title, "%s - %s", id3v1.GetArtist(), id3v1.GetArtist());
		else if (id3v1.GetArtist()) lstrcpy(title, id3v1.GetArtist());
		else if (id3v1.GetTitle()) lstrcpy(title, id3v1.GetTitle());
	} else {
		char p[MAX_PATHLEN];
		::GetFileTitle(FileName, p, MAX_PATHLEN - 1);
		lstrcpyn(title, p, _tcsrchr(p, '.') - p);
	}

}
