// ID3v2.cpp: CID3v2 クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "in_tta.h"
#include "ID3v2.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/* ID3 tag checked flags */

const unsigned __int8 ID3_UNSYNCHRONISATION_FLAG	= 0x80;
const unsigned __int8 ID3_EXTENDEDHEADER_FLAG		= 0x40;
const unsigned __int8 ID3_EXPERIMENTALTAG_FLAG		= 0x20;
const unsigned __int8 ID3_FOOTERPRESENT_FLAG		= 0x10;

/* ID3 frame checked flags */

const unsigned __int8 FRAME_COMPRESSION_FLAG       = 0x08;
const unsigned __int8 FRAME_ENCRYPTION_FLAG        = 0x04;
const unsigned __int8 FRAME_UNSYNCHRONISATION_FLAG = 0x02;

/* ID3 field text encoding */

const unsigned __int8 FIELD_TEXT_ISO_8859_1	= 0x00;
const unsigned __int8 FIELD_TEXT_UTF_16		= 0x01;
const unsigned __int8 FIELD_TEXT_UTF_16BE	= 0x02;
const unsigned __int8 FIELD_TEXT_UTF_8		= 0x03;
const unsigned __int8 FIELD_TEXT_MAX        = FIELD_TEXT_UTF_8;

const unsigned __int8 UTF16_LE[] = {0xfe, 0xff};
const unsigned __int8 UTF16_BE[] = {0xff, 0xfe};


//////////////////////////////////////////////////////////////////////
// 構築/消滅
//////////////////////////////////////////////////////////////////////

CID3v2::CID3v2()
{
	m_bHastag = false;
	m_frames.clear();

}

CID3v2::~CID3v2()
{
	m_bHastag = false;
	m_frames.clear();

}

bool CID3v2::AddFrame(CID3v2Frame &frame)
{
	map<CString, CID3v2Frame>::iterator itp;
	CString _name(frame.GetFrameID());
	_name.MakeUpper();
	itp = m_frames.find(_name);
	if((itp == m_frames.end()) || !itp->second.GetSize())
	{
		m_frames.insert(pair<CString, CID3v2Frame>(_name, frame));
	}
	return true;
}

bool CID3v2::DelFrame(const char *name, int index)
{
	//nameのなかからdwIndexの値を取得
    map<CString, CID3v2Frame>::iterator itp = m_frames.find(CString(name));
	
	if(itp != m_frames.end())
	{
		m_frames.erase(itp->first);
		return true;
	}
	return false;
}

bool CID3v2::GetComment(const char *name,int index,CString &strValue)
{
	strValue = "";
	//nameのなかからdwIndexの値を取得
    map<CString, CID3v2Frame>::iterator itp = m_frames.find(CString(name));
	
	if(itp != m_frames.end())
	{
		strValue = itp->second.GetComment();
		return true;
	}

	return false;
}

bool CID3v2::GetFrame(const char *name, int index, CID3v2Frame &strFrame)
{
	map<CString, CID3v2Frame>::iterator itp = m_frames.find(CString(name));
	if(itp != m_frames.end()) {
		strFrame = itp->second;
		return true;
	}

	return false;
}


void CID3v2::GetFrameNames(CStringArray &strArray)
{
	//nameリストを返す
	map<CString, CID3v2Frame>::iterator it = m_frames.begin();
	
	CString strName;
	while(it != m_frames.end())
	{
		if(strName.Compare(it->first))
		{
			strArray.Add(it->first);
		}
		strName = it->first;
		it++;
	}
}



CString CID3v2::GetAlbum()
{
	CString Album;
	GetComment("TALB", 0, Album);
	return Album;
}

CString CID3v2::GetTitle()
{
	CString Title;
	GetComment("TIT2", 0, Title);
	return Title;
}

CString CID3v2::GetArtist()
{
	CString Artist;
	GetComment("TPE1", 0, Artist);
	return Artist;
}


__int32 CID3v2::ReadTag(const char *filename)
{
	FileName = filename;
	HANDLE	    HFILE;
	v2header	header;
	m_bHastag = false;
	tag_length = 0;
	__int32 dwWin32errorCode = ERROR_SUCCESS;

	HFILE = CreateFile((LPCTSTR)FileName, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (HFILE == INVALID_HANDLE_VALUE) {
		dwWin32errorCode = GetLastError();
		return dwWin32errorCode;
	}

	unsigned char *buffer;
	unsigned long result;

	if (!ReadFile(HFILE, &header, sizeof(v2header), &result, NULL) || result != sizeof(v2header)) {
		CloseHandle(HFILE);
		return -1;
	}

	if(memcmp(header.id, "ID3", 3) != 0) {
		SetFilePointer(HFILE, 0, NULL, FILE_BEGIN);
		CloseHandle(HFILE);
		return -1;
	}

	m_ver = header.version;
	if((m_ver != 0x03) && (m_ver != 0x04)) {
		CloseHandle(HFILE);
		return -1;
	}

	tag_length = unpack_sint28(header.size) + 10; // size + headersize(10byte)

	buffer = new unsigned char[tag_length];

	if (!buffer) {
		CloseHandle(HFILE);
		return -1;
	}

	if(!ReadFile(HFILE, &buffer, tag_length, &result, NULL) || result != tag_length) {
		delete buffer;
		CloseHandle(HFILE);
		return -1;
	}

	__int32 dwRemainSize;
	if(header.flags & ID3_UNSYNCHRONISATION_FLAG) {
		dwRemainSize = DecodeUnSynchronization(buffer, tag_length);
		m_bUnSynchronization = true;
	} else {
		dwRemainSize = tag_length;
		m_bUnSynchronization = false;
	}

	__int32 dwID3v2Size = dwRemainSize;

	if(header.flags & ID3_EXTENDEDHEADER_FLAG)
		dwRemainSize -= unpack_sint28(buffer) + 4;
	header.flags &= ~ID3_EXTENDEDHEADER_FLAG;

	__int32 dwReadSize; 
	CID3v2Frame frame;
	// read id3v2 frames
	while (dwRemainSize) {
		int comments = 0;

		// get frame header
		dwReadSize = 
			frame.GetFrame(buffer + (dwID3v2Size - dwRemainSize), dwRemainSize, m_ver);

		if(!dwReadSize)
			break;

		AddFrame(frame);
		dwRemainSize -= dwReadSize;
	}
	delete buffer;
	CloseHandle(HFILE);
	m_bHastag = false; // for debug
	return dwWin32errorCode;
}

__int32 CID3v2::SaveTag()
{
	HANDLE HFILE;
	__int32 dwWin32errorCode = ERROR_SUCCESS;

	v2header header;
	DWORD  result;
	__int32  id3v2_size;
//	unsigned char *buffer, *ptr;
//	unsigned char *tag_data, *tptr;
//	DWORD new_size, id3v2_size;
//	int indx, offset;
//	BOOL copy_data = TRUE;
//	BOOL safe_mode = FALSE;

	HFILE = CreateFile(FileName, GENERIC_READ|GENERIC_WRITE,
		FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if (HFILE == INVALID_HANDLE_VALUE) {
		dwWin32errorCode = GetLastError();
		return dwWin32errorCode;
	}

	if (!ReadFile(HFILE, &header, sizeof(v2header), &result, NULL) ||
		result != sizeof(v2header)) {
		CloseHandle(HFILE);
		return -1;
	}

	if (!memcmp(header.id, "ID3", 3)) {
		id3v2_size = unpack_sint28(header.size) + 10;
		if (header.flags & ID3_FOOTERPRESENT_FLAG) id3v2_size += 10;
	} else {
		ZeroMemory(&header, sizeof(v2header));
		CopyMemory(header.id, "ID3", 3);
		id3v2_size = 0;
	}

//	tag_data = (unsigned char *)HeapAlloc(heap, HEAP_ZERO_MEMORY,
//		id3v2_size + sizeof(id3v2_data));
//	tptr = tag_data + 10;
//
//	if (!(header.flags & ID3_UNSYNCHRONISATION_FLAG) &&
//		!(header.flags & ID3_EXPERIMENTALTAG_FLAG) &&
//		(header.version >= ID3_VERSION) && id3v2_size) {
//
//		hMap = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, id3v2_size, NULL);
//		if (!hMap) goto done;
//
//		buffer = (unsigned char *)MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, id3v2_size);
//		if (!buffer) {
//			CloseHandle(hMap);
//			goto done;
//		}
//
//		ptr = buffer + 10;
//
//		// copy extended header if present
//		if ((header.flags & ID3_EXTENDEDHEADER_FLAG)) {
//			int ext_size = (int) unpack_sint32(ptr);
//			CopyMemory(tptr, ptr, ext_size);
//			ptr += ext_size; tptr += ext_size;
//		}
//	} else copy_data = FALSE;
//
//	// add updated id3v2 frames
//	add_text_frame("TIT2", &tptr, ttainfo->id3v2.title);
//	add_text_frame("TPE1", &tptr, ttainfo->id3v2.artist);
//	add_text_frame("TALB", &tptr, ttainfo->id3v2.album);
//	add_text_frame("TRCK", &tptr, ttainfo->id3v2.track);
//	add_text_frame("TYER", &tptr, ttainfo->id3v2.year);
//	add_text_frame("TCON", &tptr, ttainfo->id3v2.genre);
//	add_comm_frame("COMM", &tptr, ttainfo->id3v2.comment);
//
//	if (!copy_data) goto save;
//
//	// copy unchanged frames
//	while ((unsigned long)abs(ptr - buffer) < id3v2_size) {
//		int data_size, frame_size;
//		int frame_id, comments = 0;
//		frame frame_header;
//
//		// get frame header
//		CopyMemory(&frame_header, ptr, sizeof(frame));
//		data_size = unpack_sint32(frame_header.size);
//		frame_size = sizeof(frame) + data_size;
//
//		if (!*frame_header.id) break;
//
//		if ((frame_id = get_frame_id(frame_header.id)))
//			if (frame_id != COMM || !comments++) {
//				ptr += frame_size; continue;
//			}
//
//		// copy frame
//		CopyMemory(tptr, ptr, frame_size);
//		tptr += frame_size; ptr += frame_size;
//	}
//
//	// copy footer if present
//	if (id3v2.flags & ID3_FOOTERPRESENT_FLAG) {
//		CopyMemory(tptr, ptr, 10);
//		tptr += 10; ptr += 10;
//	}
//
//save:
//	if (copy_data) {
//		UnmapViewOfFile((LPCVOID *) buffer);
//		CloseHandle(hMap);
//	}
//
//	new_size = tptr - tag_data;
//
//	// fill ID3v2 header
//	id3v2.flags &= ~ID3_UNSYNCHRONISATION_FLAG;
//	id3v2.flags &= ~ID3_EXPERIMENTALTAG_FLAG;
//	id3v2.version = ID3_VERSION;
//
//	// write data
//	if (new_size <= id3v2_size) {
//		pack_sint28(id3v2_size - 10, id3v2.size);
//		CopyMemory(tag_data, &id3v2, sizeof(id3v2_tag));
//
//		SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
//		if (!WriteFile(hFile, tag_data, id3v2_size, &result, 0) ||
//			result != id3v2_size) {
//			CloseHandle(hFile);
//			tta_error(WRITE_ERROR, ttainfo->filename);
//			return;
//		}
//		goto done;
//	}
//
//	pack_sint28(new_size - 10, id3v2.size);
//	CopyMemory(tag_data, &id3v2, sizeof(id3v2_tag));
//	offset = (int) new_size - id3v2_size;
//
//	hMap = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0,
//		ttainfo->FILESIZE + offset, NULL);
//	if (!hMap) goto done;
//
//	buffer = (unsigned char *)MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0,
//		ttainfo->FILESIZE + offset);
//	if (!buffer) {
//		CloseHandle(hMap);
//		goto done;
//	}
//
//	if (safe_mode) pause();
//
//	MoveMemory(buffer + ((int)id3v2_size + offset),
//		buffer + id3v2_size, ttainfo->FILESIZE);
//	CopyMemory(buffer, tag_data, new_size);
//
//	if (safe_mode) FlushViewOfFile((LPCVOID *) buffer, 0);
//	UnmapViewOfFile((LPCVOID *) buffer);
//	CloseHandle(hMap);
//
//	ttainfo->FILESIZE += offset;
//	ttainfo->id3v2.size = new_size;
//
//	if (safe_mode) {
//		info.FILESIZE = ttainfo->FILESIZE;
//		info.id3v2.size = ttainfo->id3v2.size;
//		seek_needed = decode_pos_ms;
//		unpause();
//	}
//
//done:
//	CloseHandle(hFile);
//	HeapFree(heap, 0, tag_data);
//
//	ttainfo->id3v2.id3has = 1;

	return dwWin32errorCode;
}

//static void del_id3v2_tag (tta_info *ttainfo) {
///	HANDLE hFile, hMap;
//	unsigned char *buffer;
//	int indx, result;
//	BOOL safe_mode = FALSE;
//
//	if (!ttainfo->id3v2.id3has) return;
//
//	if (!memcmp(ttainfo->filename, info.filename,
//		lstrlen(ttainfo->filename))) safe_mode = TRUE;
//
//	hFile = CreateFile(ttainfo->filename, GENERIC_READ|GENERIC_WRITE,
//		FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
//	if (hFile == INVALID_HANDLE_VALUE) {
//		tta_error(OPEN_ERROR, ttainfo->filename);
//		return;
//	}
//
//	hMap = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0, 0, NULL);
//	if (!hMap) {
//		CloseHandle(hFile);
//		CloseHandle(hMap);
//		return;
//	}
//
//	buffer = (unsigned char *)MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
//	if (!buffer) {
//		CloseHandle(hFile);
//		CloseHandle(hMap);
//		return;
//	}
//
//	if (safe_mode) pause();
//
//	MoveMemory(buffer, buffer + ttainfo->id3v2.size,
//		ttainfo->FILESIZE - ttainfo->id3v2.size);
//
//	if (safe_mode) FlushViewOfFile((LPCVOID *) buffer, 0);
//	UnmapViewOfFile((LPCVOID *) buffer);
//	CloseHandle(hMap);
//
//	SetFilePointer(hFile, -(int) ttainfo->id3v2.size, NULL, FILE_END);
//	SetEndOfFile(hFile);
//	CloseHandle(hFile);
//
//	ttainfo->FILESIZE -= ttainfo->id3v2.size;
//	ttainfo->id3v2.size = 0;
//
//	if (safe_mode) {
//		info.FILESIZE = ttainfo->FILESIZE;
//		info.id3v2.size = ttainfo->id3v2.size;
//		seek_needed = decode_pos_ms;
//		unpause();
//	}
//
//	ttainfo->id3v2.id3has = 0;
//}

__int32 CID3v2::DecodeUnSynchronization(unsigned char *data, __int32 dwSize)
{
	__int32 dwDecodeSize = 0;
	unsigned char *writePtr = data;
	bool bHitFF = false;

	for (__int32 i = 0; i < dwSize; i++)
	{
		if(data[i] == 0xff)
			bHitFF = true;
		else {
			if(bHitFF && (data[i] == 0x00))
			{
				bHitFF = false;
				continue;
			}
			bHitFF = false;
		}
		writePtr[dwDecodeSize] = data[i];
		dwDecodeSize++;
	}
	return dwDecodeSize;
}

__int32 CID3v2::EncodeUnSynchronization(unsigned char *srcData, __int32 dwSize, unsigned char *dstData)
{
	__int32 dwDecodeSize = 0;
	unsigned char *writePtr = dstData;
	bool bHitFF = false;

	for(__int32 i = 0; i < dwSize; i++)	{
		if(bHitFF && (((srcData[i]&0xe0) == 0xe0) || (srcData[i] == 0x00))) {
			writePtr[dwDecodeSize] = 0x00;
			dwDecodeSize++;
		}
		if(srcData[i] == 0xff)
			bHitFF = true;
		else
			bHitFF = false;
		writePtr[dwDecodeSize] = srcData[i];
		dwDecodeSize++;
	}
	return dwDecodeSize;
}



// CID3v2Frame

CID3v2Frame::CID3v2Frame()
{
	Release();
}

CID3v2Frame::CID3v2Frame(const CID3v2Frame &obj)
{
	m_Comment = obj.m_Comment;
	m_ID = new char[ID3v2FrameIDLength];
	memcpy(m_ID, obj.m_ID, ID3v2FrameIDLength);
	m_dwSize = obj.m_dwSize;
	m_Encoding = obj.m_Encoding;
	m_wFlags = obj.m_wFlags;
}

CID3v2Frame::CID3v2Frame(const char *ID)
{
	Release();
	memcpy(m_ID, ID, ID3v2FrameIDLength);
}

CID3v2Frame::~CID3v2Frame()
{
	Release();
}

void CID3v2Frame::Release()
{
	m_Encoding = FIELD_TEXT_ISO_8859_1;
	m_dwSize = 0;
	m_ID = NULL;
	m_wFlags = 0;
	if (m_ID != NULL)
		delete m_ID;
}

__int32 CID3v2Frame::GetFrame(unsigned char *pData, __int32 dwSize, unsigned __int8 version)
{
	Release();
	if(dwSize < 10)
		return 0;
	m_ID = new char[ID3v2FrameIDLength];
	memcpy(&m_ID, pData, ID3v2FrameIDLength);
	if(!m_ID)
		return 0;

	__int32 size;
	m_Version = version;
	if(m_Version == 0x03)
		size = ::GetLength32(pData + 4);
	else if(m_Version == 0x04)
		size = ::unpack_sint28(pData + 4);

	if((size + 10) > dwSize)
		return 0;

	pData += 4;
	memcpy(&m_Encoding, pData, sizeof(m_Encoding));
	pData++;

	switch(m_Encoding) {
		case FIELD_TEXT_ISO_8859_1:
		default: {
			char *tempchar = new char[size];
			if(!tempchar) break;
			strncpy_s(tempchar, size, (char *)pData, size - 2);
			tempchar[size - 1] = '\0';
			m_Comment = tempchar;
			delete tempchar;
			break;
		}
		case FIELD_TEXT_UTF_16:	{
			if(!(memcmp(pData, UTF16_LE, 2))) 
				UTF16toUTF16BE((WCHAR *)(pData + 3), (size - 3) / 2);

			size = ::WideCharToMultiByte(CP_ACP, 0, 
				(const WCHAR *)(pData + 3), (size - 3) / 2, 0, 0, NULL, NULL);
			size ++;
			char *tempchar = new char[size];
			if(!tempchar) break;
			::WideCharToMultiByte(CP_ACP, 0,
				(const WCHAR *)(pData + 3), (size - 3) / 2, tempchar, size, NULL, NULL);
			tempchar[size - 1] = '\0';
			m_Comment = tempchar;
			delete tempchar;
			break;
		}
		case FIELD_TEXT_UTF_16BE: {
			UTF16toUTF16BE((WCHAR *)(pData + 1), (size - 1) / 2);
			size = ::WideCharToMultiByte(CP_ACP, 0, (const WCHAR *)(pData + 1), (size - 1) / 2, 0, 0, NULL, NULL);
			size++;
			char *tempchar = new char[size];
			if(!tempchar) break;
			::WideCharToMultiByte(CP_ACP, 0, (const WCHAR *)(pData + 1), (size - 1) / 2, tempchar, size, NULL, NULL);
			tempchar[size - 1] = '\0';
			m_Comment = tempchar;
			delete tempchar;
			break;
		}
		case FIELD_TEXT_UTF_8: {
			int size = ::MultiByteToWideChar(CP_UTF8, 0, (char *)(pData + 1), m_dwSize - 1, NULL, 0);
			size++;
			WCHAR *tempchar = new WCHAR[size];
			if(!tempchar) break;
			::MultiByteToWideChar(CP_UTF8, 0, (char *)(pData + 1), m_dwSize - 1, tempchar, size - 1);
			tempchar[size - 1] = L'\0';

			size = ::WideCharToMultiByte(CP_UTF8, 0, tempchar, -1, 0, 0, NULL, NULL);
			char *tempchar2 = new char[size];
			if(!tempchar2) {
				delete tempchar;
				break;
			}
			::WideCharToMultiByte(CP_UTF8, 0, tempchar, -1, tempchar2, size, NULL, NULL);
			m_Comment = tempchar2;
			delete tempchar2;
			delete tempchar;
			break;
		}
	}

	m_dwSize = size;
	m_wFlags = Extract16(pData + 8);
	return (size + 10);
}

char *CID3v2Frame::SetFrame(unsigned __int8 enc, unsigned __int8 version)
{
	__int32 size;
	unsigned char *tempchar;

	switch(enc) {
		case FIELD_TEXT_ISO_8859_1:
		default: {
			m_Encoding = FIELD_TEXT_ISO_8859_1;
			size = m_Comment.GetLength() + 2;
			tempchar = new unsigned char[size];
			tempchar[0] = m_Encoding;
			strcpy_s((char *)tempchar + 1, size - 1, (LPCTSTR)m_Comment);
			break;
		}
		case FIELD_TEXT_UTF_16:	{
			m_Encoding = FIELD_TEXT_UTF_16;
			size = ::MultiByteToWideChar(CP_ACP, 0, m_Comment, -1, 0, 0);
			size = size * sizeof(WCHAR) + 3;
			tempchar = new unsigned char[size];
			if(!tempchar) break;
			tempchar[0] = m_Encoding;
			memcpy(tempchar + 1, UTF16_BE, 2);
			::MultiByteToWideChar(CP_ACP, 0, m_Comment, -1, (WCHAR *)(tempchar + 3),
				(size - 3) / sizeof(WCHAR));
			break;
		}
		case FIELD_TEXT_UTF_16BE: {
			m_Encoding = FIELD_TEXT_UTF_16BE;
			size = ::MultiByteToWideChar(CP_ACP, 0, m_Comment, -1, 0, 0);
			size = size * sizeof(WCHAR) + 1;
			tempchar = new unsigned char[size];
			if(!tempchar) break;
			tempchar[0] = m_Encoding;
			::MultiByteToWideChar(CP_ACP, 0, m_Comment, -1, (WCHAR *)(tempchar + 1), (size - 1) / sizeof(WCHAR));
			UTF16toUTF16BE((WCHAR *)(tempchar + 1), (size - 1) / sizeof(WCHAR));
			break;
		}
		case FIELD_TEXT_UTF_8: {
			m_Encoding = FIELD_TEXT_UTF_8;
			size = m_Comment.GetLength() + 2;
			tempchar = new unsigned char[size + 2];
			tempchar[0] = m_Encoding;
			wcscpy((WCHAR *)(tempchar + 1), (LPCTSTR)m_Comment);
			break;
		}
	}

	char *frame = new char[size + 10];

	strncpy_s(frame, ID3v2FrameIDLength, m_ID, ID3v2FrameIDLength);
	if(version == 0x03)
		::SetLength32(size, frame + 4);
	else if(version == 0x04)
		::pack_sint28(size, frame + 4);
	Compress16((unsigned char *)(frame + 8), m_wFlags);
	_tcsncpy_s((frame + 10), size, (const char *)tempchar, size);
	delete tempchar;

	return frame;
}




void CID3v2Frame::UTF16toUTF16BE(WCHAR *str, int len)
{
	for(int i = 0; i < len; i++)
		str[i] = (str[i] << 8) | (str[i] >> 8);
}

