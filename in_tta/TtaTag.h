// TtaTag.h: CTtaTag クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////
// $LastChangedDate$

#if !defined(AFX_TTATAG_H__17D3307A_BCA3_4A1A_A6E1_7749937C8172__INCLUDED_)
#define AFX_TTATAG_H__17D3307A_BCA3_4A1A_A6E1_7749937C8172__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ID3v1.h"
#include "ID3v2.h"
#include "crc32.h"

struct TTA_header
{
    unsigned __int32 TTAid;
    unsigned __int16 AudioFormat;
    unsigned __int16 NumChannels;
    unsigned __int16 BitsPerSample;
    unsigned __int32 SampleRate;
    unsigned __int32 DataLength;
    unsigned __int32 CRC32;
};

class CTtaTag  
{
public:
	CTtaTag();
	virtual ~CTtaTag();
	void Flush();
	bool ReadTag(HWND hMainWindow, const char *filename);
	void CloseFile();
	void SetPlayTitle(char *title);

	HANDLE GetHFILE() {return HFILE;}
	char *GetFileName();
	int GetNumberofChannel() {return NCH;}
	int GetBitsperSample() {return BPS;}
	int GetByteSize() {return BSIZE;}
	int GetFormat() {return FORMAT;}
	int GetSampleRate() {return SAMPLERATE;}
	int GetDataLength() {return DATALENGTH;}
	int GetLengthbyFrame() {return FRAMELEN;}
	unsigned long GetLengthbymsec() {return LENGTH;}
	int GetFileSize() {return FILESIZE;}
	double GetCompressRate() {return COMPRESS;}
	int GetBitrate() {return BITRATE;}
//	bool HasID3v1Tag() {return id3v1.hasTag();}
//	bool HasID3v2Tag() {return id3v2.hasTag();}

	CID3v1	id3v1;
	CID3v2	id3v2;

private:
	HANDLE	HFILE;
	CString	FileName;	// filename
//	char	FileName[MAX_PATHLEN];	// filename

	int		NCH;		// number of channels
	int		BPS;		// bits per sample
	int		BSIZE;		// byte size
	int		FORMAT;		// audio format
	int		SAMPLERATE;	// samplerate (sps)
	int		DATALENGTH;	// data length in samples
	int		FRAMELEN;	// frame length
	unsigned long		LENGTH;		// playback time (msec)
	int		FILESIZE;	// file size (byte)
	double	COMPRESS;	// compression ratio
	int		BITRATE;	// bitrate (kbps)
	int		STATE;		// return code

	TTA_header		ttaheader;

	
};

#endif // !defined(AFX_TTATAG_H__17D3307A_BCA3_4A1A_A6E1_7749937C8172__INCLUDED_)
