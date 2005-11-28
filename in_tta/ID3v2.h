// ID3v2.h: CID3v2 クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ID3V2_H__FBAE6334_4C98_4C30_81D4_B402D55C9818__INCLUDED_)
#define AFX_ID3V2_H__FBAE6334_4C98_4C30_81D4_B402D55C9818__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <map>
#include <list>
using namespace std;

/* ID3 tag checked flags */

#define ID3_UNSYNCHRONISATION_FLAG		0x80
#define ID3_EXTENDEDHEADER_FLAG			0x40
#define ID3_EXPERIMENTALTAG_FLAG		0x20
#define ID3_FOOTERPRESENT_FLAG			0x10

/* ID3 frame checked flags */

#define FRAME_COMPRESSION_FLAG			0x0008
#define FRAME_ENCRYPTION_FLAG			0x0004
#define FRAME_UNSYNCHRONISATION_FLAG	0x0002

/* ID3 field text encoding */

#define FIELD_TEXT_ISO_8859_1	0x00
#define FIELD_TEXT_UTF_16		0x01
#define FIELD_TEXT_UTF_16BE		0x02
#define FIELD_TEXT_UTF_8		0x03

#ifndef TTADEC_H_

// return codes
#define OPEN_ERROR		1		// Can't open file
#define FORMAT_ERROR	2		// Unknown TTA format version
#define PLAYER_ERROR	3		// Not supported file format
#define FILE_ERROR		4		// File is corrupted
#define READ_ERROR		5		// Can't read from file
#define WRITE_ERROR		6		// Can't write to file
#define MEMORY_ERROR	7		// Memory allocation error
#define THREAD_ERROR	8		// Error killing thread

#endif

struct v2header
{
	char  id[3];
	unsigned short version;
	unsigned char  flags;
	char  size[4];
};

struct frame{
	char  id[4];
	unsigned char  size[4];
	short flags;
};

class CID3v2  
{
public:
	CID3v2();
	virtual ~CID3v2();
	int ReadTag(const char *filename);
	bool hasTag() {return false;}
	int  TagLength() {return tag_length;}
	int SaveTag();

	CString GetArtist() {return "";}
	CString GetTitle() {return "";}
	CString GetAlbum() {return "";}
private:
	HANDLE	HFILE;
	CString	FileName;	// filename
	int		STATE;		// return code

	multimap<CString, CString>m_frames;
	bool	has_tag;

	int  tag_length; // include header+frame+etcetera

	bool AddComment(const char *name, const char *value);
	bool DelComment(const char *name, int index);
	bool GetComment(const char *name, int index, CString &strValue);
	void GetCommentNames(CStringArray &strArray);
	
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

#endif // !defined(AFX_ID3V2_H__FBAE6334_4C98_4C30_81D4_B402D55C9818__INCLUDED_)
