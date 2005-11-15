// ID3v2.cpp: CID3v2 クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "in_tta.h"
#include "ID3v2.h"
#include <winsock2.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// 構築/消滅
//////////////////////////////////////////////////////////////////////

CID3v2::CID3v2()
{
	has_tag = false;
	m_frames.clear();

}

CID3v2::~CID3v2()
{
	m_frames.clear();

}

bool CID3v2::AddComment(const char *name, const char *value)
{
	CString _name(name);
	_name.MakeUpper();
	m_frames.insert(pair<CString,CString>(_name,CString(value)));
	
	return TRUE;
}

bool CID3v2::DelComment(const char *name, int index)
{
	//nameのなかからdwIndexの値を取得
    pair<multimap<CString,CString>::iterator, multimap<CString,CString>::iterator> itp = m_frames.equal_range(CString(name));
	
	int i = 0;
	while(itp.first != itp.second)
	{
		if(i == index)
		{
			m_frames.erase(itp.first);
			return TRUE;
		}
		itp.first++;
		i++;
	}
	
	return FALSE;
}

bool CID3v2::GetComment(const char *name,int index,CString &strValue)
{
	strValue = "";
	//nameのなかからdwIndexの値を取得
    pair<multimap<CString,CString>::iterator,multimap<CString,CString>::iterator> itp = m_frames.equal_range(CString(name));
	
	int i = 0;
	while(itp.first != itp.second)
	{
		if(i == index)
		{
			strValue = (itp.first)->second;
			return TRUE;
		}
		itp.first++;
		i++;
	}
	
	return FALSE;
}

void CID3v2::GetCommentNames(CStringArray &strArray)
{
	//nameリストを返す
	multimap<CString,CString>::iterator it = m_frames.begin();
	
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

int CID3v2::ReadTag(const char *filename)
{
	::strncpy(FileName, filename, MAX_PATHLEN);

	HFILE = CreateFile(FileName, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (HFILE == INVALID_HANDLE_VALUE) {
		STATE = OPEN_ERROR;
		return -1;
	}

	HANDLE hMap;
	unsigned char *buffer, *ptr;
	unsigned long result;

	if (!ReadFile(HFILE, &header, sizeof(v2header), &result, NULL) ||
		result != sizeof(v2header) || memcmp(header.id, "ID3", 3)) {
		SetFilePointer(HFILE, 0, NULL, FILE_BEGIN);
		return -1;
	}

	tag_length = unpack_sint28(header.size) + 10; // size + headersize(10byte)

	if ((header.flags & ID3_UNSYNCHRONISATION_FLAG) ||
		(header.flags & ID3_EXPERIMENTALTAG_FLAG) ||
		(header.version < 3)) goto done;

	hMap = CreateFileMapping(HFILE, NULL, PAGE_READONLY, 0, tag_length, NULL);
	if (!hMap) goto done;

	buffer = (unsigned char *)MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, tag_length);
	if (!buffer) {
		CloseHandle(hMap);
		goto done;
	}

	ptr = buffer + 10;

	// skip extended header if present
	if (header.flags & ID3_EXTENDEDHEADER_FLAG) {
		int offset = (int) ntohl((u_long)*ptr);
		ptr += offset;
	}

	// read id3v2 frames
	while (ptr - buffer < tag_length) {
		int data_size;
		int comments = 0;
		frame temp_frame;
		char *data;

		// get frame header
		CopyMemory(&temp_frame, ptr, sizeof(frame));
		ptr += sizeof(frame);
		data_size = (int) ::ntohl((u_long)*(temp_frame.size));

		if (!*temp_frame.id) break;

		// skip unsupported frames
		// ToDo: support UTF-16/UTF-8 ?
		if (temp_frame.flags & FRAME_COMPRESSION_FLAG ||
			temp_frame.flags & FRAME_ENCRYPTION_FLAG ||
			temp_frame.flags & FRAME_UNSYNCHRONISATION_FLAG ||
			*ptr != FIELD_TEXT_ISO_8859_1) {
			ptr += data_size;
			continue;
		}

		ptr++; data_size--;
		// Comment 
		if (memcmp(temp_frame.id, "COMM", 4)) {
			ptr += 5; data_size -= 5;
		}

		CopyMemory(data, ptr, data_size);
		AddComment(temp_frame.id, data);
		ptr += data_size;
	}

	UnmapViewOfFile((LPCVOID *) buffer);
	CloseHandle(hMap);

done:
	if (header.flags & ID3_FOOTERPRESENT_FLAG) tag_length += 10;
	SetFilePointer(HFILE, tag_length, NULL, FILE_BEGIN);

	has_tag = false; // for debug
	CloseHandle(HFILE);
	return 0;
}
