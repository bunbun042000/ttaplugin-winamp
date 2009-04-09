// ID3v2.cpp: CID3v2 クラスのインプリメンチEEション
//
// $LastChangedDate$
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

static const __int32 COMM_Lang_Length = 3;


//////////////////////////////////////////////////////////////////////
// 構篁E消?E//////////////////////////////////////////////////////////////////////

CID3v2::CID3v2()
{
	Release();
}

CID3v2::~CID3v2()
{
	Release();
}

void CID3v2::Release()
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



__int32 CID3v2::SetComment(char *ID, CString &Description, CString &Comment, char *sLanguage)
{
	map<CString, CID3v2Frame>::iterator it = m_frames.find(CString(ID));
	if(it != m_frames.end()){
		// if comment is exist
		if(Comment != "")
			it->second.SetComment(Description, Comment, sLanguage, m_Encoding, m_ver);
		else
			DelFrame(ID);
	} else if(Comment != "") {
		CID3v2Frame frame(ID);
		frame.SetComment(Description, Comment, sLanguage, m_Encoding, m_ver);
		m_frames.insert(pair<CString, CID3v2Frame>(CString(ID), frame));

	}
	return m_dwSize;
}

__int32 CID3v2::GetTotalFrameLength()
{
	__int32 length = 0;
	map<CString, CID3v2Frame>::iterator it = m_frames.begin();
	while(it != m_frames.end()) {
		length += it->second.GetSize() + FRAME_HEADER_LENGTH;
		it++;
	}
	// Total frame length
	return length;
}


void CID3v2::SetVersion(unsigned __int8 ver)
{
	// only version 2.3 or 2.4 
	if (ver != 0x03 && ver != 0x04)
		return;
	m_ver = ver;
}

bool CID3v2::SetEncoding(unsigned __int8 enc)
{
	//encoding ver2.3 ISO-8859-1 and UTF-16
	//encoding ver2.4 ISO-8859-1 UTF-16 UTF-16BE and UTF-8
	if ((m_ver == 0x04 && enc > FIELD_TEXT_MAX + 1)	|| (m_ver != 0x03 && m_ver != 0x04))
		return false;
	if(m_ver == 0x03 && (enc < FIELD_TEXT_MAX + 1) && (enc > FIELD_TEXT_UTF_16))
		m_Encoding = FIELD_TEXT_ISO_8859_1;
	m_Encoding = enc;
	return true;
}

CString CID3v2::GetAlbum()
{
	CString Album;
	GetComment("TALB", Album);
	return Album;
}

void CID3v2::SetAlbum(CString &Album)
{
	SetComment("TALB", CString(""), Album, NULL);
}

CString CID3v2::GetTitle()
{
	CString Title;
	GetComment("TIT2", Title);
	return Title;
}

void CID3v2::SetTitle(CString &Title)
{
	SetComment("TIT2", CString(""), Title, NULL);
}

CString CID3v2::GetArtist()
{
	CString Artist;
	GetComment("TPE1", Artist);
	return Artist;
}

void CID3v2::SetArtist(CString &Artist)
{
	SetComment("TPE1", CString(""), Artist, NULL);
}

CString CID3v2::GetTrackNo()
{
	CString TrackNo;
	GetComment("TRCK", TrackNo);
	return TrackNo;
}

void CID3v2::SetTrackNo(CString &TrackNo)
{
	SetComment("TRCK", CString(""), TrackNo, NULL);
}

CString CID3v2::GetYear()
{
	CString Year;
	if(m_ver == 0x03) {
		GetComment("TYER", Year);
	} else if (m_ver == 0x04){
		GetComment("TDRC", Year);
	}
	return Year;
}

void CID3v2::SetYear(CString &Year)
{
	if(m_ver == 0x03) {
		CString tempYear;
		tempYear = Year.Left(4);
		SetComment("TYER", CString(""), tempYear, NULL);
		SetComment("TDRC", CString(""), CString(""), NULL);
		SetComment("TDRL", CString(""), CString(""), NULL);
	} else if(m_ver == 0x04) {
		SetComment("TYER", CString(""), CString(""), NULL);
		SetComment("TDRC", CString(""), Year, NULL);
		SetComment("TDRL", CString(""), CString(""), NULL);
	}
}

CString CID3v2::GetGenre()
{
	CString Genre;
	GetComment("TCON", Genre);
	return Genre;
}

void CID3v2::SetGenre(CString &Genre)
{
	SetComment("TCON", CString(""), Genre, NULL);
}

CString CID3v2::GetComment()
{
	CString Comment;
	GetComment("COMM", Comment);
	return Comment;
}

void CID3v2::SetComment(CString &Description, CString &Comment)
{
	SetComment("COMM", Description, Comment, "eng");
}

CString CID3v2::GetCopyright()
{
	CString Copyright;
	GetComment("TCOP", Copyright);
	return Copyright;
}

void CID3v2::SetCopyright(CString &Copyright)
{
	SetComment("TCOP", CString(""), Copyright, NULL);
}

CString CID3v2::GetURI()
{
	CString URI;
	GetComment("WXXX", URI);
	return URI;
}

void CID3v2::SetURI(CString &Description, CString &URI)
{
	SetComment("WXXX", Description, URI, NULL);
}

CString CID3v2::GetWords()
{
	CString Words;
	GetComment("TEXT", Words);
	return Words;
}

void CID3v2::SetWords(CString &Words)
{
	SetComment("TEXT", CString(""), Words, NULL);
}

CString CID3v2::GetComposers()
{
	CString Composers;
	GetComment("TCOM", Composers);
	return Composers;
}

void CID3v2::SetComposers(CString &Composers)
{
	SetComment("TCOM", CString(""), Composers, NULL);
}

CString CID3v2::GetArrangements()
{
	CString Arrangements;
	GetComment("TPE4", Arrangements);
	return Arrangements;
}

void CID3v2::SetArrangements(CString &Arrangements)
{
	SetComment("TPE4", CString(""), Arrangements, NULL);
}

CString CID3v2::GetOrigArtist()
{
	CString OrigArtist;
	GetComment("TOPE", OrigArtist);
	return OrigArtist;
}

void CID3v2::SetOrigArtist(CString &OrigArtist)
{
	SetComment("TOPE", CString(""), OrigArtist, NULL);
}

CString CID3v2::GetEncEngineer()
{
	CString EncEngineer;
	GetComment("TENC", EncEngineer);
	return EncEngineer;
}

void CID3v2::SetEncEngineer(CString &EncEngineer)
{
	SetComment("TENC", CString(""), EncEngineer, NULL);
}


__int32 CID3v2::ReadTag(const char *filename)
{
	Release();

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

	//only ver2.3 and 2.4
	m_ver = header.version;
	if((m_ver != 0x03) && (m_ver != 0x04)) {
		CloseHandle(HFILE);
		return -1;
	}

	// decode header size(without tagheader)
	m_dwSize = unpack_sint28(header.size); // size (extended header + frames + padding + footer)

	buffer = new unsigned char[m_dwSize];

	if (!buffer) {
		CloseHandle(HFILE);
		return -1;
	}

	SetFilePointer(HFILE, HEADER_LENGTH, NULL, FILE_BEGIN);

	if(!ReadFile(HFILE, buffer, m_dwSize, &result, NULL) || result != m_dwSize) {
		delete buffer;
		CloseHandle(HFILE);
		return -1;
	}

	__int32 dwRemainSize;
	if(header.flags & ID3_UNSYNCHRONISATION_FLAG) {
		dwRemainSize = DecodeUnSynchronization(buffer, m_dwSize);
		m_bUnSynchronization = true;
	} else {
		dwRemainSize = m_dwSize;
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

		// get frame header
		dwReadSize = 
			frame.GetFrame(buffer + (dwID3v2Size - dwRemainSize), dwRemainSize, m_ver);

		if(!dwReadSize)
			break;

		AddFrame(frame);
		m_Encoding = frame.GetEncoding();
		dwRemainSize -= dwReadSize;
	}
	delete buffer;
	CloseHandle(HFILE);
	m_bHastag = true; // for debug
	return dwWin32errorCode;
}

__int32 CID3v2::SaveTag()
{
	HANDLE HFILE;
	__int32 dwWin32errorCode = ERROR_SUCCESS;

	DWORD  result;
	bool bTempFile = false;

	map<CString,CID3v2Frame>::iterator it = m_frames.begin();
	__int32 totalLength = 0;
	unsigned __int32 EncLength = 0;

	//frame length(without tag header)
	totalLength = GetTotalFrameLength();

	if(totalLength == 0) {
		if(m_dwSize != 0)
			return DeleteTag(FileName);
		else
			return dwWin32errorCode;
	}

	char *frames = new char[totalLength];

	// all tags set to buffer
	__int32 dwOffset = 0;
	while(it != m_frames.end()){
		memcpy_s((frames + dwOffset), totalLength - dwOffset, it->second.SetFrame(), it->second.GetSize() + FRAME_HEADER_LENGTH);
		dwOffset += it->second.GetSize() + FRAME_HEADER_LENGTH;
		it++;
	}

	//unsynchronization
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

	//set tag header
	memcpy_s(header, EncLength + HEADER_LENGTH, "ID3", 3);
	header[3] = m_ver;
	header[4] = m_subver;
	header[5] = m_Flags;
	memcpy_s((header + HEADER_LENGTH), EncLength, tempData, EncLength);
	delete tempData;

	char TempPath[MAX_PATHLEN];
	char szTempFile[MAX_PATHLEN];

	if(EncLength > m_dwSize) {
		::pack_sint28(EncLength, (header + 6));
		strcpy_s(TempPath, MAX_PATHLEN, (LPCTSTR)GetFilePath(FileName));
		if(!GetTempFileName(TempPath, "wat", 0, szTempFile)) {
			dwWin32errorCode = GetLastError();
			DeleteFile(szTempFile);
			return dwWin32errorCode;
		}

		// body(music) data copy to temp file
		dwWin32errorCode = CopyBodyData(EncLength + HEADER_LENGTH, szTempFile);
		if(dwWin32errorCode != ERROR_SUCCESS) {
			DeleteFile(szTempFile);
			return dwWin32errorCode;
		}

		//open tempfile
		bTempFile = true;
		HFILE = CreateFile(szTempFile, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL, NULL);
		if(HFILE == INVALID_HANDLE_VALUE) {
			dwWin32errorCode = GetLastError();
			delete header;
			return dwWin32errorCode;
		}

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

	if(!WriteFile(HFILE, header, EncLength + HEADER_LENGTH, &result, NULL)){
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
		ExchangeTempFileToOriginalFile(szTempFile);
		DeleteFile(szTempFile);
	}

	return dwWin32errorCode;

}

__int32 CID3v2::DeleteTag(const char *filename)
{
	FileName =  filename;
	__int32 dwWin32errorCode = ERROR_SUCCESS;
	bool bTempFile = false;
	char szTempFile[MAX_PATHLEN];
	char TempPath[MAX_PATHLEN];
	// Create Temporary File
	GetTempPath(MAX_PATHLEN, TempPath);
	if(!GetTempFileName(TempPath, "wat", 0, szTempFile)) {
		dwWin32errorCode = GetLastError();
		DeleteFile(szTempFile);
		return dwWin32errorCode;
	}

	dwWin32errorCode = CopyBodyData(0, szTempFile);
	if(dwWin32errorCode != ERROR_SUCCESS)
		return dwWin32errorCode;

	dwWin32errorCode = ExchangeTempFileToOriginalFile(szTempFile);
	if(dwWin32errorCode != ERROR_SUCCESS)
		m_bHastag = true;
	else
		m_bHastag = false;
	DeleteFile(szTempFile);
	return dwWin32errorCode;
}

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

__int32 CID3v2::CopyBodyData(__int32 startbody, const char *sDstFileName)
{
	__int32 dwWin32errorCode = ERROR_SUCCESS;
	DWORD  result;
	HANDLE ORIGINALFILE, TEMPFILE;

	ORIGINALFILE = CreateFile(FileName, GENERIC_READ, FILE_SHARE_READ, 
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(ORIGINALFILE == INVALID_HANDLE_VALUE) {
		dwWin32errorCode = GetLastError();
		return dwWin32errorCode;
	}
	DWORD dwDataSize = GetFileSize(ORIGINALFILE, NULL);
	if (m_dwSize != 0) {
		dwDataSize -= m_dwSize + HEADER_LENGTH;
	}

	char *pRawData = new char[dwDataSize];
	if(!pRawData) {
		dwWin32errorCode = GetLastError();
		CloseHandle(ORIGINALFILE);
		return dwWin32errorCode;
	}
	// Create Temporary File
	if(SetFilePointer(ORIGINALFILE, m_dwSize != 0 ? m_dwSize + HEADER_LENGTH : 0, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER) {
		dwWin32errorCode = GetLastError();
		delete pRawData;
		CloseHandle(ORIGINALFILE);
		DeleteFile(sDstFileName);
		return dwWin32errorCode;
	}
	TEMPFILE = CreateFile(sDstFileName, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ, NULL,
		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(TEMPFILE == INVALID_HANDLE_VALUE) {
		dwWin32errorCode = GetLastError();
		delete pRawData;
		DeleteFile(sDstFileName);
		CloseHandle(ORIGINALFILE);
		return dwWin32errorCode;
	}
	if(SetFilePointer(TEMPFILE, startbody, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER) {
		dwWin32errorCode = GetLastError();
		delete pRawData;
		CloseHandle(ORIGINALFILE);
		CloseHandle(TEMPFILE);
		DeleteFile(sDstFileName);
		return dwWin32errorCode;
	}
	if(!ReadFile(ORIGINALFILE, pRawData, dwDataSize, &result, NULL) || dwDataSize != result) {
		dwWin32errorCode = GetLastError();
		CloseHandle(ORIGINALFILE);
		CloseHandle(TEMPFILE);
		delete pRawData;
		return dwWin32errorCode;
	}
	if(!WriteFile(TEMPFILE, pRawData, dwDataSize, &result, NULL) || dwDataSize != result) {
		dwWin32errorCode = GetLastError();
		CloseHandle(ORIGINALFILE);
		CloseHandle(TEMPFILE);
		delete pRawData;
		return dwWin32errorCode;
	}
	delete pRawData;
	CloseHandle(ORIGINALFILE);
	CloseHandle(TEMPFILE);
	return dwWin32errorCode;
}

__int32 CID3v2::ExchangeTempFileToOriginalFile(const char *sDestFileName)
{
	char szPreFile[MAX_PATHLEN];
	char TempPath[MAX_PATHLEN];
	__int32 dwWin32errorCode = ERROR_SUCCESS;

	GetTempPath(MAX_PATHLEN, TempPath);
	if(!GetTempFileName(TempPath, "wat", 0, szPreFile)) {
		dwWin32errorCode = GetLastError();
		return dwWin32errorCode;
	}
	DeleteFile(szPreFile);
	if(!CopyFileEx(FileName, szPreFile, NULL, NULL, false, COPY_FILE_RESTARTABLE)) {
		dwWin32errorCode = GetLastError();
		return dwWin32errorCode;
	}
	if(!CopyFileEx(sDestFileName, FileName, NULL, NULL, false, COPY_FILE_RESTARTABLE)) {
		dwWin32errorCode = GetLastError();
		MoveFile(szPreFile, FileName);
		return dwWin32errorCode;
	}
	DeleteFile(szPreFile);

	return dwWin32errorCode;
}

CString CID3v2::GetFilePath(CString &str)
{
	return str.Left(str.ReverseFind('\\'));
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
	memcpy_s(m_ID, ID3v2FrameIDLength + 1, obj.m_ID, ID3v2FrameIDLength);
	m_ID[ID3v2FrameIDLength] = '\0';
	m_dwSize = obj.m_dwSize;
	m_Encoding = obj.m_Encoding;
	m_wFlags = obj.m_wFlags;
	m_Version = obj.m_Version;
	m_Description = obj.m_Description;
	if(obj.m_sLanguage != NULL){
		m_sLanguage = new char[COMM_Lang_Length + 1];
		memcpy_s(m_sLanguage, COMM_Lang_Length + 1, obj.m_sLanguage, COMM_Lang_Length);
		m_sLanguage[COMM_Lang_Length] = '\0';
	} else
		m_sLanguage = NULL;
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
	m_sLanguage = NULL;
}

__int32 CID3v2Frame::GetFrame(unsigned char *pData, __int32 dwSize, unsigned __int8 version)
{
	Release();
	if(*pData == NULL)
		return 0;
	if(dwSize < 10)
		return 0;
	m_ID = new char[ID3v2FrameIDLength + 1];
	if(!m_ID)
		return 0;
	memcpy_s(m_ID, ID3v2FrameIDLength + 1, pData, ID3v2FrameIDLength);
	// for Padding
	m_ID[ID3v2FrameIDLength] = '\0';

	__int32 size;
	m_Version = version;
	if(m_Version == 0x03)
		size = ::GetLength32(pData + 4);
	else if(m_Version == 0x04)
		size = ::unpack_sint28(pData + 4);

	if((size + 10) > dwSize)
		return 0;

	m_dwSize = size;
	pData += 8;
	m_wFlags = Extract16(pData);
	pData += 2;


	if(m_ID[0] == 'W' && !strcmp(m_ID, "WXXX")) {
		m_Comment = GetEncodingString(pData, m_dwSize, FIELD_TEXT_ISO_8859_1);
	} else {
		memcpy_s(&m_Encoding, sizeof(unsigned __int8), pData, sizeof(m_Encoding));
		// L'\0' or '\0'
		__int32 dwTermLength;
		if(m_Encoding == FIELD_TEXT_UTF_16)
			dwTermLength = 2 * sizeof(WCHAR);
		else if(m_Encoding == FIELD_TEXT_UTF_16BE)
			dwTermLength = sizeof(L'\0');
		else
			dwTermLength = sizeof('\0');
		pData++;
		size = m_dwSize - 1;
		if(m_ID[0] == 'T' && strcmp(m_ID, "TXXX"))
			m_Comment = GetEncodingString(pData, size, m_Encoding);
		else if(!strcmp(m_ID, "TXXX")) {
			m_Description = GetEncodingString(pData, size, m_Encoding);
			size -= m_Description.GetLength() + dwTermLength;
			pData += m_Description.GetLength() + dwTermLength;
			m_Comment = GetEncodingString(pData, size, m_Encoding);
		} else if(!strcmp(m_ID, "WXXX")) {
			m_Description = GetEncodingString(pData, size, m_Encoding);
			size -= m_Description.GetLength() + dwTermLength;
			pData += m_Description.GetLength() + dwTermLength;
			m_Comment = GetEncodingString(pData, size, FIELD_TEXT_ISO_8859_1);
		} else if(!strcmp(m_ID, "COMM")) {
			m_sLanguage = new char[COMM_Lang_Length + 1];
			if(!m_sLanguage)
				return 0;
			memcpy_s(m_sLanguage, COMM_Lang_Length + 1, pData, COMM_Lang_Length);
			m_sLanguage[COMM_Lang_Length] = '\0';
			pData += COMM_Lang_Length;
			size -= COMM_Lang_Length;
			m_Description = GetEncodingString(pData, size, m_Encoding);
			size -= m_Description.GetLength() + dwTermLength;
			pData += m_Description.GetLength() + dwTermLength;
			m_Comment = GetEncodingString(pData, size, m_Encoding);
		}
	}
	return (m_dwSize + FRAME_HEADER_LENGTH);
}

char *CID3v2Frame::SetFrame()
{

	char *frame = new char[m_dwSize + FRAME_HEADER_LENGTH];

	memcpy_s(frame, m_dwSize + FRAME_HEADER_LENGTH, m_ID, ID3v2FrameIDLength);
	if(m_Version == 0x03)
		::SetLength32(m_dwSize, frame + 4);
	else if(m_Version == 0x04)
		::pack_sint28(m_dwSize, frame + 4);
	Compress16((unsigned char *)(frame + 8), m_wFlags);

	// L'\0' or '\0'
	__int32 dwTermLength;
	if(m_Encoding == FIELD_TEXT_UTF_16 || m_Encoding == FIELD_TEXT_UTF_16BE)
		dwTermLength = sizeof(L'\0');
	else
		dwTermLength = sizeof('\0');
	char *temp = frame + FRAME_HEADER_LENGTH;

	if(m_ID[0] == 'W' && !strcmp(m_ID, "WXXX")) {
		memcpy_s(temp, m_dwSize, (char *)SetEncodingString(m_Comment, m_Version, FIELD_TEXT_ISO_8859_1), m_dwSize);
	} else {
		temp[0] = m_Encoding;
		temp += sizeof(m_Encoding);
		__int32 size = m_dwSize - 1;
		if(m_ID[0] == 'T' && strcmp(m_ID, "TXXX"))
			memcpy_s(temp, m_dwSize, (char *)SetEncodingString(m_Comment, m_Version, m_Encoding), size);
		else if(!strcmp(m_ID, "TXXX")) {
			__int32 dwCurrentSize = GetEncodingLength(m_Description, m_Version, m_Encoding);
			memcpy_s(temp, m_dwSize, (char *)SetEncodingString(m_Description, m_Version, m_Encoding), dwCurrentSize);
			temp += dwCurrentSize;
			size -= dwCurrentSize;
			memcpy_s(temp, m_dwSize - dwCurrentSize, (char *)SetEncodingString(m_Comment, m_Version, m_Encoding), size);
		} else if(!strcmp(m_ID, "WXXX")) {
			__int32 dwCurrentSize = GetEncodingLength(m_Description, m_Version, m_Encoding);
			memcpy_s(temp, m_dwSize, (char *)SetEncodingString(m_Description, m_Version, m_Encoding), dwCurrentSize);
			temp += dwCurrentSize;
			size -= dwCurrentSize;
			memcpy_s(temp, m_dwSize - dwCurrentSize, (char *)SetEncodingString(m_Comment, m_Version, FIELD_TEXT_ISO_8859_1), size);
		} else if(!strcmp(m_ID, "COMM")) {
			memcpy_s(temp, m_dwSize, m_sLanguage, COMM_Lang_Length);
			temp += COMM_Lang_Length;
			size -= COMM_Lang_Length;
			__int32 dwCurrentSize = GetEncodingLength(m_Description, m_Version, m_Encoding);
			memcpy_s(temp, m_dwSize, (char *)SetEncodingString(m_Description, m_Version, m_Encoding), dwCurrentSize);
			temp += dwCurrentSize;
			size -= dwCurrentSize;
			memcpy_s(temp, m_dwSize - dwCurrentSize, (char *)SetEncodingString(m_Comment, m_Version, m_Encoding), size);
		}
	}

	return frame;
}

void CID3v2Frame::SetComment(CString description, CString str, char *sLanguage, unsigned __int8 Encoding, unsigned __int8 version)
{
	if((Encoding > FIELD_TEXT_MAX) || (version != 0x03 && version != 0x04))
		return;
	if(!strcmp(m_ID, "COMM") && sLanguage == NULL) 
		return;

	m_Encoding = Encoding;
	m_Comment = str;
	m_Description = description;
	m_Version = version;

	if(!strcmp(m_ID, "COMM")) {
		if(m_sLanguage != NULL) 
			delete m_sLanguage;
		m_sLanguage = new char[COMM_Lang_Length];
		memcpy_s(m_sLanguage, COMM_Lang_Length, sLanguage, COMM_Lang_Length);
		m_sLanguage[COMM_Lang_Length] = '\0';
	}

	__int32 size = 0;

	// L'\0' or '\0'
	__int32 dwTermLength;
	if(m_Encoding == FIELD_TEXT_UTF_16 || m_Encoding == FIELD_TEXT_UTF_16BE)
		dwTermLength = sizeof(L'\0');
	else
		dwTermLength = sizeof('\0');

	if(m_ID[0] == 'W' && !strcmp(m_ID, "WXXX")) {
		size = GetEncodingLength(str, version, FIELD_TEXT_ISO_8859_1);
	} else {
		size++;
		if(m_ID[0] == 'T' && strcmp(m_ID, "TXXX"))
			size += GetEncodingLength(str, version, Encoding);
		else if(!strcmp(m_ID, "TXXX")) {
			size += GetEncodingLength(description, version, Encoding);
			size += GetEncodingLength(str, version, Encoding);
		} else if(!strcmp(m_ID, "WXXX")) {
			size += GetEncodingLength(description, version, Encoding);
			size += GetEncodingLength(str, version, FIELD_TEXT_ISO_8859_1);
		} else if(!strcmp(m_ID, "COMM")) {
			size += COMM_Lang_Length;
			size += GetEncodingLength(description, version, Encoding);
			size += GetEncodingLength(str, version, Encoding);
		}
	}
	m_dwSize = size;

}

void CID3v2Frame::UTF16toUTF16BE(WCHAR *str, int len)
{
	for(int i = 0; i < len; i++)
		str[i] = (str[i] << 8) | (str[i] >> 8);
}

CString CID3v2Frame::GetEncodingString(unsigned char *pData, __int32 dwRemainSize, unsigned __int8 Encoding)
{
	if(m_Encoding > FIELD_TEXT_MAX)
		return "";

	CString sTempChar;

	switch(Encoding) {
		case FIELD_TEXT_ISO_8859_1:
		default: {
			__int32 size = strlen((char *)pData);
			if(dwRemainSize < size + 1)
				return "";
			char *tempchar = new char[size + 1];
			if(tempchar == NULL) 
				return "";
			strcpy_s(tempchar, size + 1, (char *)pData);
			tempchar[size] = '\0';
			sTempChar = tempchar;
			delete tempchar;
			break;
		}
		case FIELD_TEXT_UTF_16:	{
			char *tempchar = new char[dwRemainSize];
			if(tempchar == NULL)
				return "";

			memcpy_s(tempchar, dwRemainSize, pData, dwRemainSize);
			if(!(memcmp(pData, UTF16_BE, 2)))
				UTF16toUTF16BE((WCHAR *)(tempchar + 2), (dwRemainSize - 2) / 2);

			__int32 dwStrSize = (wcslen((WCHAR *)(tempchar + 2)) + 1) * sizeof(WCHAR);
			if (dwRemainSize - 2 < dwStrSize)
				return "";
	
			__int32 size = ::WideCharToMultiByte(CP_ACP, 0, 
				(const WCHAR *)(tempchar + 2), dwStrSize / 2, 0, 0, NULL, NULL);
			size ++;

			char *copychar = new char[size];
			if(!copychar)
				return "";

			::WideCharToMultiByte(CP_ACP, 0,
				(const WCHAR *)(tempchar + 2), dwStrSize / 2, copychar, size, NULL, NULL);

			tempchar[size - 1] = '\0';
			sTempChar = copychar;
			delete tempchar;
			delete copychar;
			break;
		}
		case FIELD_TEXT_UTF_16BE: {
			char *copychar = new char[dwRemainSize];
			if(copychar == NULL)
				return "";
			memcpy_s(copychar, dwRemainSize, pData, dwRemainSize);
			UTF16toUTF16BE((WCHAR *)copychar, dwRemainSize / 2);
			__int32 dwStrSize = (wcslen((WCHAR *)(copychar + 2)) + 1) * sizeof(WCHAR);
			if(dwRemainSize < dwStrSize)
				return "";

			__int32 size = ::WideCharToMultiByte(CP_ACP, 0, (const WCHAR *)copychar, dwStrSize / 2, 0, 0, NULL, NULL);
			size++;
			char *tempchar = new char[size];
			if(!tempchar)
				return "";
			::WideCharToMultiByte(CP_ACP, 0, (const WCHAR *)copychar, dwStrSize / 2, tempchar, size, NULL, NULL);
			tempchar[size - 1] = '\0';
			sTempChar = tempchar;
			delete tempchar;
			delete copychar;
			break;
		}
		case FIELD_TEXT_UTF_8: {
			__int32 dwStrSize = strlen((char *)pData) + 1;
			__int32 size = ::MultiByteToWideChar(CP_UTF8, 0, (char *)pData, dwStrSize, NULL, 0);
			size++;
			WCHAR *tempchar = new WCHAR[size];
			if(tempchar == NULL)
				return "";
			::MultiByteToWideChar(CP_UTF8, 0, (char *)pData, dwStrSize, tempchar, size - 1);
				tempchar[size - 1] = L'\0';

			size = ::WideCharToMultiByte(CP_ACP, 0, tempchar, -1, 0, 0, NULL, NULL);
			char *tempchar2 = new char[size];
			if(tempchar2 == NULL) {
				delete tempchar;
				return "";
			}
			::WideCharToMultiByte(CP_ACP, 0, tempchar, -1, tempchar2, size, NULL, NULL);
			sTempChar = tempchar2;
			delete tempchar2;
			delete tempchar;
			break;
		}
	}
	return sTempChar;
}

__int32 CID3v2Frame::GetEncodingLength(CString &str, unsigned __int8 version, unsigned __int8 Encoding)
{
	if((Encoding > FIELD_TEXT_MAX) || (version != 0x03 && version != 0x04)) 
		return 0;

	if(version == 0x03 && Encoding > FIELD_TEXT_UTF_16)
		return 0;


	__int32 size;
	switch(Encoding) {
		case FIELD_TEXT_ISO_8859_1:
		default: {
			size = str.GetLength() + 1;
			break;
		}
		case FIELD_TEXT_UTF_16:	{
			if(str == "")
				size = 2 * sizeof(WCHAR);
			else {
				size = ::MultiByteToWideChar(CP_ACP, 0, str, -1, 0, 0);
				size = (size  + 2) * sizeof(WCHAR);
			}
			break;
		}
		case FIELD_TEXT_UTF_16BE: {
			if(str == "")
				size = sizeof(WCHAR);
			else {
				size = ::MultiByteToWideChar(CP_ACP, 0, str, -1, 0, 0);
				size = (size  + 1) * sizeof(WCHAR);
			}
			break;
		}
		case FIELD_TEXT_UTF_8: {
			if(str == "")
				size = sizeof(unsigned char);
			else {
				size = ::MultiByteToWideChar(CP_ACP, 0, str, -1, 0, 0);
				size = size * sizeof(WCHAR);
				unsigned char *tempDataUTF16 = new unsigned char[size];
				if(tempDataUTF16 == NULL)
					return 0;
				::MultiByteToWideChar(CP_ACP, 0, str, -1, (WCHAR *)tempDataUTF16, size / sizeof(WCHAR));
				size = ::WideCharToMultiByte(CP_UTF8, 0, (WCHAR *)tempDataUTF16, -1, NULL, 0, NULL, NULL);
				size += 1;
			}
			break;
		}
	}
	return size;

}

unsigned char *CID3v2Frame::SetEncodingString(CString &str, unsigned __int8 version, unsigned __int8 Encoding)
{
	if((Encoding > FIELD_TEXT_MAX) || (version != 0x03 && version != 0x04)) 
		return NULL;

	if(version == 0x03 && Encoding > FIELD_TEXT_UTF_16)
		return NULL;

	unsigned char *tempchar;


	switch(m_Encoding) {
		case FIELD_TEXT_ISO_8859_1:
		default: {
			if(str == "") {
				tempchar = new unsigned char[1];
				tempchar[0] = '\0';
				break;
			}
			tempchar = new unsigned char[str.GetLength() + 1];
			if(tempchar == NULL)
				return NULL;
			memcpy_s((char *)tempchar, str.GetLength() + 1, (LPCTSTR)str, str.GetLength() + 1);
			break;
		}
		case FIELD_TEXT_UTF_16:	{
			__int32 size;
			if(str == "") 
				size = 2 * sizeof(WCHAR);
			else {
				size = ::MultiByteToWideChar(CP_ACP, 0, (LPCTSTR)str, -1, 0, 0);
				size = (size + 1) * sizeof(WCHAR);
			}
			tempchar = new unsigned char[size];
			if(tempchar == NULL)
				return NULL;
			memcpy_s(tempchar, size, UTF16_LE, 2);
			if(str == "")
				memcpy_s(tempchar + 2, size - 2, "\0\0", sizeof(WCHAR));
			else {
				::MultiByteToWideChar(CP_ACP, 0, (LPCTSTR)str, -1, (WCHAR *)(tempchar + 2), 
					(size - 2) / sizeof(WCHAR));
			}
			break;
		}
		case FIELD_TEXT_UTF_16BE: {
			__int32 size;
			if(str == "")
				size = sizeof(WCHAR);
			else {
				size = ::MultiByteToWideChar(CP_ACP, 0, str, -1, 0, 0);
				size = (size  + 1) * sizeof(WCHAR);
			}
			tempchar = new unsigned char[size];
			if(tempchar == NULL)
				return NULL;
			if(str == "")
				memcpy_s(tempchar, size, "\0\0", sizeof(WCHAR));
			else {
				::MultiByteToWideChar(CP_ACP, 0, (LPCTSTR)str, -1, (WCHAR *)tempchar, size / sizeof(WCHAR));
				UTF16toUTF16BE((WCHAR *)tempchar, m_dwSize / sizeof(WCHAR));
			}
			break;
		}
		case FIELD_TEXT_UTF_8: {
			unsigned char *tempDataUTF16;
			__int32 size;
			if(str == "")
				size = sizeof(unsigned char);
			else {
				__int32 tempsize = ::MultiByteToWideChar(CP_ACP, 0, (LPCTSTR)str, -1, 0, 0);
				tempsize = tempsize * sizeof(WCHAR);
				tempDataUTF16 = new unsigned char[tempsize];
				if(tempDataUTF16 == NULL)
					return NULL;
				::MultiByteToWideChar(CP_ACP, 0, (LPCTSTR)str, -1, (WCHAR *)tempDataUTF16, tempsize / sizeof(WCHAR));
				size = ::WideCharToMultiByte(CP_UTF8, 0, (WCHAR *)tempDataUTF16, -1, NULL, 0, NULL, NULL);
				size += sizeof(unsigned char);
			}
			tempchar = new unsigned char[size];
			if(tempchar == NULL)
				return NULL;
			if(str == "")
				tempchar[0] = '\0';
			else
				::WideCharToMultiByte(CP_UTF8, 0, (WCHAR *)tempDataUTF16, -1, (char *)tempchar, size, NULL, NULL);
			break;
		}
	}
	return tempchar;
}
