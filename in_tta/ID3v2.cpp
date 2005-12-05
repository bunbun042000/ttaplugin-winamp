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
const unsigned __int32 HEADER_LENGTH = 10;
const unsigned __int32 FRAME_HEADER_LENGTH = 10;

const unsigned __int32 MAX_BUFFER_SIZE = 1000000;


//////////////////////////////////////////////////////////////////////
// 構築/消滅
//////////////////////////////////////////////////////////////////////

CID3v2::CID3v2()
{
	m_bHastag = false;
	m_frames.clear();
	m_Encoding = FIELD_TEXT_ISO_8859_1;
	m_ver = 0x04;
	m_subver = 0x00;
	m_Flags = 0x00;
	m_dwSize = 0;

}

CID3v2::~CID3v2()
{
	m_bHastag = false;
	m_frames.clear();
	m_Encoding = FIELD_TEXT_ISO_8859_1;
	m_ver = 0x04;
	m_subver = 0x00;
	m_Flags = 0x00;
	m_dwSize = 0;

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

bool CID3v2::DelFrame(const char *name)
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

bool CID3v2::GetComment(const char *name, CString &strValue)
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

bool CID3v2::GetFrame(const char *name, CID3v2Frame &strFrame)
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

__int32 CID3v2::SetComment(char *ID, CString &Comment)
{
	map<CString, CID3v2Frame>::iterator it = m_frames.find(CString(ID));
	if(it != m_frames.end()){
		it->second.SetComment(Comment, m_Encoding, m_ver);
		m_dwSize = it->second.GetSize();
	} else {
		CID3v2Frame frame(ID);
		frame.SetComment(Comment, m_Encoding, m_ver);
		m_frames.insert(pair<CString, CID3v2Frame>(CString(ID), frame));

	}
	return m_dwSize;
}

CString CID3v2::GetAlbum()
{
	CString Album;
	GetComment("TALB", Album);
	return Album;
}

CString CID3v2::GetTitle()
{
	CString Title;
	GetComment("TIT2", Title);
	return Title;
}

CString CID3v2::GetArtist()
{
	CString Artist;
	GetComment("TPE1", Artist);
	return Artist;
}

void CID3v2::SetArtist(CString &Artist)
{
	SetComment("TPE1", Artist);
}



__int32 CID3v2::ReadTag(const char *filename)
{
	FileName = filename;
	HANDLE	    HFILE;
	v2header	header;
	m_bHastag = false;
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

	m_dwSize = unpack_sint28(header.size); // size (extended header + frames + padding + footer)

	buffer = new unsigned char[m_dwSize + HEADER_LENGTH];

	if (!buffer) {
		CloseHandle(HFILE);
		return -1;
	}

	if(!ReadFile(HFILE, &buffer, m_dwSize + HEADER_LENGTH, &result, NULL) || result != m_dwSize + HEADER_LENGTH) {
		delete buffer;
		CloseHandle(HFILE);
		return -1;
	}

	__int32 dwRemainSize;
	if(header.flags & ID3_UNSYNCHRONISATION_FLAG) {
		dwRemainSize = DecodeUnSynchronization(buffer, m_dwSize + HEADER_LENGTH);
		m_bUnSynchronization = true;
	} else {
		dwRemainSize = m_dwSize + HEADER_LENGTH;
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

	DWORD  result;
	bool bTempFile = false;
//	unsigned char *buffer, *ptr;
//	unsigned char *tag_data, *tptr;
//	DWORD new_size, id3v2_size;
//	int indx, offset;
//	BOOL copy_data = TRUE;
//	BOOL safe_mode = FALSE;

	__int32 size = m_frames.size();
	map<CString,CID3v2Frame>::iterator it = m_frames.begin();
	__int32 totalLength = 0;
	unsigned __int32 EncLength = 0;
	char **tempframes = new char*[size];
	if(!*tempframes) return NULL;
	__int32 i = 0;
	while(it != m_frames.end()){
		tempframes[i++] = it->second.SetFrame();
		it++;
	}
	__int32 NumOfFrames = i;
	i = 0;
	char *frames = new char[totalLength];

	it = m_frames.begin();
	while(it != m_frames.end()){
		totalLength += it->second.GetSize() + 10;
		memcpy((frames + totalLength), tempframes[i++], it->second.GetSize() + 10);
		it++;
	}
	delete tempframes;

	char *tempData;
	if(m_bUnSynchronization) {
		tempData = new char[totalLength * 2];
		EncLength = EncodeUnSynchronization((unsigned char *)frames, totalLength, (unsigned char *)tempData);
		m_Flags |= ID3_UNSYNCHRONISATION_FLAG;
		delete frames;
	} else {
		tempData = frames;
		EncLength = totalLength;
		m_Flags &= ~ID3_UNSYNCHRONISATION_FLAG;
	}

	char *header = new char[EncLength + HEADER_LENGTH];
	if (!header) return NULL;

	memcpy(header, "ID3", 3);
	header[3] = m_ver;
	header[4] = m_subver;
	header[5] = m_Flags;
	memcpy((header + 10), tempData, EncLength);
	delete tempData;

	char TempPath[MAX_PATHLEN];
	char szTempFile[MAX_PATHLEN];

	if(EncLength > m_dwSize) {
		::pack_sint28(EncLength, (header + 6));
		HANDLE HFILE2;
		HFILE2 = CreateFile(FileName, GENERIC_READ, FILE_SHARE_READ, 
			NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if(HFILE2 == INVALID_HANDLE_VALUE) {
			dwWin32errorCode = GetLastError();
			delete header;
			return dwWin32errorCode;
		}
		DWORD dwDataSize = GetFileSize(HFILE2, NULL);
		if (m_dwSize != 0) dwDataSize -= m_dwSize + HEADER_LENGTH;
		char *pRawData = new char[dwDataSize];
		if(!pRawData) {
			dwWin32errorCode = GetLastError();
			CloseHandle(HFILE2);
			delete header;
			return dwWin32errorCode;
		}
		// Create Temporary File
		GetTempPath(MAX_PATHLEN, TempPath);
		if(!GetTempFileName(TempPath, "wat", 0, szTempFile)) {
			dwWin32errorCode = GetLastError();
			delete header;
			delete pRawData;
			CloseHandle(HFILE2);
			DeleteFile(szTempFile);
			return dwWin32errorCode;
		}
		if(SetFilePointer(HFILE2, m_dwSize == 0 ? 0 : m_dwSize + HEADER_LENGTH, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER) {
			dwWin32errorCode = GetLastError();
			delete header;
			delete pRawData;
			CloseHandle(HFILE2);
			DeleteFile(szTempFile);
			return dwWin32errorCode;
		}

		HFILE = CreateFile(szTempFile, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ, NULL,
			CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if(HFILE == INVALID_HANDLE_VALUE) {
			dwWin32errorCode = GetLastError();
			delete header;
			delete pRawData;
			DeleteFile(szTempFile);
			CloseHandle(HFILE2);
			return dwWin32errorCode;
		}

		if(SetFilePointer(HFILE, 0, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER) {
			dwWin32errorCode = GetLastError();
			delete header;
			delete pRawData;
			CloseHandle(HFILE);
			CloseHandle(HFILE2);
			DeleteFile(szTempFile);
			return dwWin32errorCode;
		}
		if(SetFilePointer(HFILE, EncLength + HEADER_LENGTH, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER) {
			dwWin32errorCode = GetLastError();
			delete header;
			delete pRawData;
			CloseHandle(HFILE);
			CloseHandle(HFILE2);
			DeleteFile(szTempFile);
			return dwWin32errorCode;
		}


		if(!ReadFile(HFILE2, pRawData, dwDataSize, &result, NULL) || dwDataSize != result) {
			dwWin32errorCode = GetLastError();
			CloseHandle(HFILE2);
			CloseHandle(HFILE);
			delete header;
			delete pRawData;
			return dwWin32errorCode;
		}
		if(!WriteFile(HFILE, pRawData, dwDataSize, &result, NULL) || dwDataSize != result) {
			dwWin32errorCode = GetLastError();
			CloseHandle(HFILE2);
			CloseHandle(HFILE);
			delete header;
			delete pRawData;
			return dwWin32errorCode;
		}
		delete pRawData;
		CloseHandle(HFILE2);
		bTempFile = true;

	} else {
		HFILE = CreateFile(FileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL, NULL);
		if(HFILE == INVALID_HANDLE_VALUE) {
			dwWin32errorCode = GetLastError();
			delete header;
			return dwWin32errorCode;
		}
		::pack_sint28(m_dwSize, (header + 6));
	}

	if(SetFilePointer(HFILE, 0, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER) {
		dwWin32errorCode = GetLastError();
		CloseHandle(HFILE);
		if(bTempFile)
			DeleteFile(szTempFile);
		delete header;
		return dwWin32errorCode;
	}

	if(!WriteFile(HFILE, header, ::unpack_sint28((unsigned char *)(header + 6)) + HEADER_LENGTH, &result, NULL)){
		dwWin32errorCode = GetLastError();
		CloseHandle(HFILE);
		if(bTempFile)
			DeleteFile(szTempFile);
		delete header;
		return dwWin32errorCode;
	}

	for(__int32 i = 0; i < ((__int32)m_dwSize - (__int32)EncLength) ; i++) {
		unsigned char pad = 0x00;
		if(!WriteFile(HFILE, &pad, 1, &result, NULL)) {
			dwWin32errorCode = GetLastError();
			CloseHandle(HFILE);
			if(bTempFile)
				DeleteFile(szTempFile);
			return dwWin32errorCode;
		}
	}

	CloseHandle(HFILE);
	if (bTempFile){
		char szPreFile[MAX_PATHLEN];
		if(!GetTempFileName(TempPath, "wat", 0, szPreFile)) {
			dwWin32errorCode = GetLastError();
			DeleteFile(szTempFile);
			return dwWin32errorCode;
		}
		DeleteFile(szPreFile);
		if(!MoveFileEx(FileName, szPreFile, MOVEFILE_COPY_ALLOWED)) {
			dwWin32errorCode = GetLastError();
			DeleteFile(szTempFile);
			return dwWin32errorCode;
		}
		if(!MoveFileEx(szTempFile, FileName, MOVEFILE_COPY_ALLOWED)) {
			dwWin32errorCode = GetLastError();
			MoveFile(szPreFile, FileName);
			DeleteFile(szTempFile);
			return dwWin32errorCode;
		}
		DeleteFile(szPreFile);
	}

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
	m_ID = new char[ID3v2FrameIDLength + 1];
	memcpy(m_ID, obj.m_ID, ID3v2FrameIDLength);
	m_ID[ID3v2FrameIDLength] = '\0';
	m_dwSize = obj.m_dwSize;
	m_Encoding = obj.m_Encoding;
	m_wFlags = obj.m_wFlags;
	m_Version = obj.m_Version;
}

CID3v2Frame::CID3v2Frame(const char *ID)
{
	Release();
	if(m_ID == NULL)
		m_ID = new char[ID3v2FrameIDLength + 1];
	strcpy_s(m_ID, ID3v2FrameIDLength + 1, ID);
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
	m_Version = 0x04;
	if (m_ID != NULL)
		delete m_ID;
}

__int32 CID3v2Frame::GetFrame(unsigned char *pData, __int32 dwSize, unsigned __int8 version)
{
	Release();
	if(dwSize < 10)
		return 0;
	m_ID = new char[ID3v2FrameIDLength + 1];
	if(!m_ID)
		return 0;
	memcpy(&m_ID, pData, ID3v2FrameIDLength);
	m_ID[ID3v2FrameIDLength] = '\0';

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

char *CID3v2Frame::SetFrame()
{
	unsigned char *tempchar;

	switch(m_Encoding) {
		case FIELD_TEXT_ISO_8859_1:
		default: {
			tempchar = new unsigned char[m_dwSize];
			if(!tempchar) return NULL;
			tempchar[0] = m_Encoding;
			strcpy_s((char *)tempchar + 1, m_dwSize - 1, (LPCTSTR)m_Comment);
			break;
		}
		case FIELD_TEXT_UTF_16:	{
			tempchar = new unsigned char[m_dwSize];
			if(!tempchar) return NULL;
			tempchar[0] = m_Encoding;
			memcpy(tempchar + 1, UTF16_BE, 2);
			::MultiByteToWideChar(CP_ACP, 0, m_Comment, -1, (WCHAR *)(tempchar + 3),
				(m_dwSize - 3) / sizeof(WCHAR));
			break;
		}
		case FIELD_TEXT_UTF_16BE: {
			tempchar = new unsigned char[m_dwSize];
			if(!tempchar) return NULL;
			tempchar[0] = m_Encoding;
			::MultiByteToWideChar(CP_ACP, 0, m_Comment, -1, (WCHAR *)(tempchar + 1), (m_dwSize - 1) / sizeof(WCHAR));
			UTF16toUTF16BE((WCHAR *)(tempchar + 1), (m_dwSize - 1) / sizeof(WCHAR));
			break;
		}
		case FIELD_TEXT_UTF_8: {
			tempchar = new unsigned char[m_dwSize];
			if(!tempchar) return NULL;
			tempchar[0] = m_Encoding;
			_tcscpy_s((char *)(tempchar + 1), m_dwSize + 1, m_Comment.GetBuffer());
			break;
		}
	}

	char *frame = new char[m_dwSize + 10];

	memcpy(frame, m_ID, ID3v2FrameIDLength);
	if(m_Version == 0x03)
		::SetLength32(m_dwSize, frame + 4);
	else if(m_Version == 0x04)
		::pack_sint28(m_dwSize, frame + 4);
	Compress16((unsigned char *)(frame + 8), m_wFlags);
	_tcscpy_s((frame + 10), m_dwSize, (const char *)tempchar);
	delete tempchar;

	return frame;
}

void CID3v2Frame::SetComment(CString str, unsigned __int8 Encoding, unsigned __int8 version)
{
	if((Encoding > FIELD_TEXT_MAX) || (version != 0x03 && version != 0x04)) return;

	m_Encoding = Encoding;
	m_Comment = str;
	m_Version = version;
	__int32 size;
	switch(m_Encoding) {
		case FIELD_TEXT_ISO_8859_1:
		default: {
			size = m_Comment.GetLength() + 2;
			break;
		}
		case FIELD_TEXT_UTF_16:	{
			size = ::MultiByteToWideChar(CP_ACP, 0, m_Comment, -1, 0, 0);
			size = size * sizeof(WCHAR) + 3;
			break;
		}
		case FIELD_TEXT_UTF_16BE: {
			size = ::MultiByteToWideChar(CP_ACP, 0, m_Comment, -1, 0, 0);
			size = size * sizeof(WCHAR) + 1;
			break;
		}
		case FIELD_TEXT_UTF_8: {
			size = m_Comment.GetLength() + 2;
			break;
		}
	}
	m_dwSize = size;

}

void CID3v2Frame::UTF16toUTF16BE(WCHAR *str, int len)
{
	for(int i = 0; i < len; i++)
		str[i] = (str[i] << 8) | (str[i] >> 8);
}

