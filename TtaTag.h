// TtaTag.h: CTtaTag �N���X�̃C���^�[�t�F�C�X
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
    unsigned long TTAid;
    unsigned short AudioFormat;
    unsigned short NumChannels;
    unsigned short BitsPerSample;
    unsigned long SampleRate;
    unsigned long DataLength;
    unsigned long CRC32;
};

class CTtaTag  
{
public:
	CTtaTag();
	virtual ~CTtaTag();
	bool ReadTag(const char *filename);

	int GetNumberofChannel() {return NCH;}
	int GetBitsperSample() {return BPS;}
	int GetByteSize() {return BSIZE;}
	int GetFormat() {return FORMAT;}
	int GetSampleRate() {return SAMPLERATE;}
	int GetDataLength() {return DATALENGTH;}
	int GetLengthbyFrame() {return FRAMELEN;}
	int GetLengthbymsec() {return LENGTH;}
	int GetFileSize() {return FILESIZE;}
	double GetCompressRate() {return COMPRESS;}
	int GetBitrate() {return BITRATE;}
	bool HasID3v1Tag() {return id3v1.hasTag();}
	bool HasID3v2Tag() {return id3v2.hasTag();}

	CID3v1	id3v1;
	CID3v2	id3v2;

private:
	HANDLE	HFILE;
	char	FileName[MAX_PATHLEN];	// filename

	int		NCH;		// number of channels
	int		BPS;		// bits per sample
	int		BSIZE;		// byte size
	int		FORMAT;		// audio format
	int		SAMPLERATE;	// samplerate (sps)
	int		DATALENGTH;	// data length in samples
	int		FRAMELEN;	// frame length
	int		LENGTH;		// playback time (msec)
	int		FILESIZE;	// file size (byte)
	double	COMPRESS;	// compression ratio
	int		BITRATE;	// bitrate (kbps)
	int		STATE;		// return code

	TTA_header		ttaheader;

	int ReadTTAheader();
	
};

#endif // !defined(AFX_TTATAG_H__17D3307A_BCA3_4A1A_A6E1_7749937C8172__INCLUDED_)
