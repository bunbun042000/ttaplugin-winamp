// TtaTag.cpp: CTtaTag クラスのインプリメンテーション
//
// $LastChangedDate$
//////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "in_tta.h"
#include "TtaTag.h"
#include "ttadec.h"
#include <winsock2.h>

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
	Flush();
}

CTtaTag::~CTtaTag()
{
}

char *CTtaTag::GetFileName()
{
	char *tempfn = new char[MAX_PATHLEN];
	strcpy_s(tempfn, FileName.GetLength(), (LPCTSTR)FileName);
	return tempfn;
}

void CTtaTag::Flush()
{
	FileName = "";
}


bool CTtaTag::ReadTag(const char *filename)
{

	// File open
	FileName = filename;

	//Read ID3v1.1
	id3v1.ReadTag(filename);

	//Read ID3v2.3 and 2.4
	id3v2.ReadTag(filename);

	//Read TTA Header
	HANDLE HFILE = CreateFile((LPCTSTR)FileName, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (HFILE == INVALID_HANDLE_VALUE || HFILE == NULL) {
		STATE = OPEN_ERROR;
		return false;
	}

	FILESIZE = ::GetFileSize(HFILE, NULL);

	int           datasize, origsize;
	unsigned long result;
	long		  checksum;

	SetFilePointer(HFILE, id3v2.GetTagLength(), NULL, FILE_BEGIN);
	// read TTA header
	if (!::ReadFile(HFILE, &ttaheader, sizeof(TTA_header), &result, NULL) ||
		result != sizeof(TTA_header)) {
		CloseHandle(HFILE);
		STATE = READ_ERROR;
		return false;
	}

	// check for TTA3 signature
	if (ttaheader.TTAid != TTA1_SIGN) {
		CloseHandle(HFILE);
		STATE = FORMAT_ERROR;
		return false;
	}

	checksum = crc32((unsigned char *) &ttaheader,
	sizeof(TTA_header) - sizeof(unsigned long));
	if (checksum != ttaheader.CRC32) {
		CloseHandle(HFILE);
		STATE = FILE_ERROR;
		return false;
	}

	// check for player supported formats
	if ((ttaheader.AudioFormat != WAVE_FORMAT_PCM &&
		ttaheader.AudioFormat != WAVE_FORMAT_IEEE_FLOAT &&
		ttaheader.AudioFormat != WAVE_FORMAT_EXTENSIBLE) ||
		ttaheader.BitsPerSample > MAX_BPS ||
		ttaheader.NumChannels > MAX_NCH) {
		CloseHandle(HFILE);
		STATE = PLAYER_ERROR;
		return false;
	}

	// fill the File Info
	NCH = ttaheader.NumChannels;
	BPS = ttaheader.BitsPerSample;
	BSIZE = (ttaheader.BitsPerSample + 7)/8;
	FORMAT = ttaheader.AudioFormat;
	SAMPLERATE = ttaheader.SampleRate;
	DATALENGTH = ttaheader.DataLength;
	FRAMELEN = (long) (FRAME_TIME * ttaheader.SampleRate);
	LENGTH = (unsigned long)(ttaheader.DataLength / ttaheader.SampleRate * 1000);

	datasize = FILESIZE - id3v2.GetTagLength();
	origsize = DATALENGTH * BSIZE * NCH;

	COMPRESS = ((double)datasize) / origsize;
	BITRATE = (long) ((COMPRESS * SAMPLERATE * NCH * BPS) / 1000);

	CloseHandle(HFILE);
	return true;
}



void CTtaTag::SetPlayTitle(char *title) {
	if (id3v2.hasTag() && 
		(id3v2.GetArtist() != "" || id3v2.GetTitle() != "" || id3v2.GetAlbum() != "")) {
		if (id3v2.GetArtist() != "" && id3v2.GetTitle() != "")
			wsprintf(title, "%s - %s", id3v2.GetArtist(), id3v2.GetTitle());
		else if (id3v2.GetArtist()  != "" && id3v2.GetAlbum() != "")
			wsprintf(title, "%s - %s", id3v2.GetArtist(), id3v2.GetAlbum());
		else if (id3v2.GetArtist() != "") lstrcpy(title, id3v2.GetArtist());
		else if (id3v2.GetTitle() != "") lstrcpy(title, id3v2.GetTitle());
	} else if (id3v1.hasTag() &&
		(id3v1.GetArtist() != "" || id3v1.GetTitle() != "" || id3v1.GetAlbum() != "")) {
		if (id3v1.GetArtist()  != "" && id3v1.GetTitle() != "")
			wsprintf(title, "%s - %s", id3v1.GetArtist(), id3v1.GetTitle());
		else if (id3v1.GetArtist() != "" && id3v1.GetAlbum() != "")
			wsprintf(title, "%s - %s", id3v1.GetArtist(), id3v1.GetArtist());
		else if (id3v1.GetArtist() != "") lstrcpy(title, id3v1.GetArtist());
		else if (id3v1.GetTitle() != "") lstrcpy(title, id3v1.GetTitle());
	} else {
		char p[MAX_PATHLEN];
		::GetFileTitle(FileName, p, MAX_PATHLEN - 1);
		lstrcpyn(title, p, strchr(p, '.') - p);
	}

}
