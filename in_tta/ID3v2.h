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


const __int32 IDv2FrameIDLength = 4;

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


__inline static void pack_sint28(unsigned __int32 value, char *ptr) {
	ptr[0] = (value >> 21);
	ptr[1] = (value >> 14);
	ptr[2] = (value >>  7);
	ptr[3] = (value);
}

__inline static unsigned __int32 unpack_sint28 (unsigned const char *ptr) {
	unsigned int value = 0;

//	if (ptr[0] & 0x80) return 0;

	value =  value       | ptr[0];
	value = (value << 7) | ptr[1];
	value = (value << 7) | ptr[2];
	value = (value << 7) | ptr[3];
	return value;
}

__inline static void SetLength32 (unsigned __int32 value, char *ptr) {
	ptr[3] = value >> 24;
	ptr[2] = value >> 16;
	ptr[1] = value >> 8;
	ptr[0] = value;
}

__inline static unsigned __int32 GetLength32(const unsigned char *ptr) {
	return (((unsigned __int32)ptr[3]<<24) | ((unsigned __int32)ptr[2]<<16) | 
		((unsigned __int32)ptr[1]<<8) | ((unsigned __int32)ptr[0]));
}

__inline static __int16 Extract16(unsigned char buf[2]) {
	return ((__int16)buf[0] << 8) | (buf[1]);
}
struct v2header
{
	char  id[3];
	unsigned __int8 version;
	unsigned __int8 subversion;
	unsigned __int8 flags;
	unsigned __int8 size[4];
};

struct frameheader
{
	char  id[IDv2FrameIDLength];
	unsigned __int8  size[4];
	short flags;
};

class CID3v2Frame
{
public:
	CID3v2Frame();
	virtual ~CID3v2Frame();
	CID3v2Frame(const CID3v2Frame &obj);
	char *GetFrameID() {return m_ID;}
	unsigned char *GetComment() {return m_Comment;}
	void  SetComment(unsigned char *str) {m_Comment = str;}
	__int32 GetSize() {return m_dwSize;}
	void  SetSize(DWORD Size) {m_dwSize = Size;}
	void  SetEncoding(unsigned char enc) {m_Encoding = enc;}
	__int32 LoadFrame(unsigned char *pData, __int32 dwSize, unsigned __int8 version);

private:
	frameheader head;
	__int32 m_dwSize;
	unsigned char m_Encoding;
	unsigned char *m_Comment;
	char *m_ID;
	__int16 m_wFlags;
	void Release();

protected:

};

class CID3v2  
{
public:
	CID3v2();
	virtual ~CID3v2();
	__int32 ReadTag(const char *filename);
	bool    hasTag() {return m_bHastag;}
	__int32 TagLength() {return tag_length;}
	__int32 SaveTag();


	CString GetArtist();

	CString GetTitle();
	CString GetAlbum();
private:
	HANDLE	HFILE;
	CString	FileName;	// filename
	int		STATE;		// return code

	unsigned __int8 m_Encoding; // Strings Encoding;
	unsigned __int8 m_ver;
	map<CString, CID3v2Frame>m_frames;
	bool	m_bHastag;
	bool    m_bUnSynchronization;

	__int32 tag_length; // include header+frame+etcetera

	bool AddFrame(CID3v2Frame &frame);
	bool DelFrame(const char *name, int index);
	bool GetFrame(const char *name, int index, CID3v2Frame &strFrame);
	bool GetComment(const char *name, int index, CString &strValue);
	void GetFrameNames(CStringArray &strArray);
	__int32 GetTotalFrameLength();
	__int32 DecodeUnSynchronization(unsigned char *data, __int32 dwSize);
	__int32 EncodeUnSynchronization(unsigned char *srcData, __int32 dwSize, unsigned char *dstData);


};

#endif // !defined(AFX_ID3V2_H__FBAE6334_4C98_4C30_81D4_B402D55C9818__INCLUDED_)
