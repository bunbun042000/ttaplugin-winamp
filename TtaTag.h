// TtaTag.h: CTtaTag クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////
// $LastChangedDate $

#if !defined(AFX_TTATAG_H__17D3307A_BCA3_4A1A_A6E1_7749937C8172__INCLUDED_)
#define AFX_TTATAG_H__17D3307A_BCA3_4A1A_A6E1_7749937C8172__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <map>
#include <list>
using namespace std;

// ID3 ver1.1
struct ID3v1tag
{
	char  id[3];
	char  title[30];
	char  artist[30];
	char  album[30];
	char  year[4];
	char  comment[28];
	char  zero;
	char  track;
	char  genre;
};

struct ID3v2_header
{
	char  id[3];
	short version;
	char  flags;
	char  size[4];
};

struct ID3v2_frame{
	char  id[4];
	char  size[4];
	short flags;
};

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
	int ReadTag(const char *filename);

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

	ID3v1_tag		id3v1tag;
	bool			have_id3v1;

	ID3v2_header	id3v2header;
	multimap<CString, CString>id3v2frames;
	bool			have_id3v2;

	int  id3v2_header_length;

	char	Title[MAX_MUSICTEXT];
	char	Artist[MAX_MUSICTEXT];
	char	Comment[MAX_MUSICTEXT];
	char	Album[MAX_MUSICTEXT];
	char	Year[MAX_MUSICTEXT];
	char	Genre[MAX_MUSICTEXT];
	char	Track[MAX_MUSICTEXT];
	char	Composer[MAX_MUSICTEXT];
	char	OrgArtist[MAX_MUSICTEXT];
	char	Copyright[MAX_MUSICTEXT];
	char	Encoder[MAX_MUSICTEXT];

	int ReadID3v1tag();
	int ReadID3v2tag();
	int ReadTTAheader();
	
	__inline static void pack_sint28 (unsigned int value, char *ptr) {
		ptr[0] = (value >> 21) & 0x7f;
		ptr[1] = (value >> 14) & 0x7f;
		ptr[2] = (value >>  7) & 0x7f;
		ptr[3] = (value & 0x7f);
	}

	__inline static unsigned int unpack_sint28 (const char *ptr) {
		unsigned int value = 0;
	
		if (ptr[0] & 0x80) return 0;
	
		value =  value       | (ptr[0] & 0x7f);
		value = (value << 7) | (ptr[1] & 0x7f);
		value = (value << 7) | (ptr[2] & 0x7f);
		value = (value << 7) | (ptr[3] & 0x7f);

		return value;
	}
};

#endif // !defined(AFX_TTATAG_H__17D3307A_BCA3_4A1A_A6E1_7749937C8172__INCLUDED_)
