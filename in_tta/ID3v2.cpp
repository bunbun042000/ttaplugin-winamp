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
	
	return true;
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
			return true;
		}
		itp.first++;
		i++;
	}
	
	return true;
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
			return true;
		}
		itp.first++;
		i++;
	}
	
	return false;
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
	FileName = filename;
	v2header	header;
	has_tag = false;
	tag_length = 0;

	HFILE = CreateFile((LPCTSTR)FileName, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (HFILE == INVALID_HANDLE_VALUE) {
		STATE = OPEN_ERROR;
		return -1;
	}

	HANDLE hMap;
	unsigned char *buffer, *ptr;
	unsigned long result;

	if (!ReadFile(HFILE, &header, sizeof(v2header), &result, NULL) || result != sizeof(v2header))
	{
		CloseHandle(HFILE);
		return -1;
	}

	if(memcmp(header.id, "ID3", 3) != 0) {
		SetFilePointer(HFILE, 0, NULL, FILE_BEGIN);
		CloseHandle(HFILE);
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
		int offset = (int) 0;//ntohl((u_long)*ptr);
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
		data_size = (int) 0;//::ntohl((u_long)*(temp_frame.size));

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

int CID3v2::SaveTag()
{
	HANDLE hFile, hMap;
	v2header header;
	unsigned char *buffer, *ptr;
	unsigned char *tag_data, *tptr;
	DWORD new_size, id3v2_size;
	int indx, offset;
	DWORD result;
	BOOL copy_data = TRUE;
	BOOL safe_mode = FALSE;

	hFile = CreateFile(FileName, GENERIC_READ|GENERIC_WRITE,
		FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		return OPEN_ERROR;
	}

	if (!ReadFile(hFile, &header, sizeof(v2header), &result, NULL) ||
		result != (DWORD) sizeof(v2header)) {
		CloseHandle(hFile);
		return READ_ERROR;
	}

	if (!memcmp(header.id, "ID3", 3)) {
		id3v2_size = unpack_sint28(header.size) + 10;
		if (header.flags & ID3_FOOTERPRESENT_FLAG) id3v2_size += 10;
	} else {
		ZeroMemory(&header, sizeof(v2header));
		CopyMemory(header.id, "ID3", 3);
		id3v2_size = 0;
	}

	tag_data = (unsigned char *)HeapAlloc(heap, HEAP_ZERO_MEMORY,
		id3v2_size + sizeof(id3v2_data));
	tptr = tag_data + 10;

	if (!(header.flags & ID3_UNSYNCHRONISATION_FLAG) &&
		!(header.flags & ID3_EXPERIMENTALTAG_FLAG) &&
		(header.version >= ID3_VERSION) && id3v2_size) {

		hMap = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, id3v2_size, NULL);
		if (!hMap) goto done;

		buffer = (unsigned char *)MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, id3v2_size);
		if (!buffer) {
			CloseHandle(hMap);
			goto done;
		}

		ptr = buffer + 10;

		// copy extended header if present
		if ((header.flags & ID3_EXTENDEDHEADER_FLAG)) {
			int ext_size = (int) unpack_sint32(ptr);
			CopyMemory(tptr, ptr, ext_size);
			ptr += ext_size; tptr += ext_size;
		}
	} else copy_data = FALSE;

	// add updated id3v2 frames
//	add_text_frame("TIT2", &tptr, ttainfo->id3v2.title);
//	add_text_frame("TPE1", &tptr, ttainfo->id3v2.artist);
//	add_text_frame("TALB", &tptr, ttainfo->id3v2.album);
//	add_text_frame("TRCK", &tptr, ttainfo->id3v2.track);
//	add_text_frame("TYER", &tptr, ttainfo->id3v2.year);
//	add_text_frame("TCON", &tptr, ttainfo->id3v2.genre);
//	add_comm_frame("COMM", &tptr, ttainfo->id3v2.comment);

	if (!copy_data) goto save;

	// copy unchanged frames
	while ((unsigned long)abs(ptr - buffer) < id3v2_size) {
		int data_size, frame_size;
		int frame_id, comments = 0;
		frame frame_header;

		// get frame header
		CopyMemory(&frame_header, ptr, sizeof(frame));
		data_size = unpack_sint32(frame_header.size);
		frame_size = sizeof(frame) + data_size;

		if (!*frame_header.id) break;

		if ((frame_id = get_frame_id(frame_header.id)))
			if (frame_id != COMM || !comments++) {
				ptr += frame_size; continue;
			}

		// copy frame
		CopyMemory(tptr, ptr, frame_size);
		tptr += frame_size; ptr += frame_size;
	}

	// copy footer if present
	if (id3v2.flags & ID3_FOOTERPRESENT_FLAG) {
		CopyMemory(tptr, ptr, 10);
		tptr += 10; ptr += 10;
	}

save:
	if (copy_data) {
		UnmapViewOfFile((LPCVOID *) buffer);
		CloseHandle(hMap);
	}

	new_size = tptr - tag_data;

	// fill ID3v2 header
	id3v2.flags &= ~ID3_UNSYNCHRONISATION_FLAG;
	id3v2.flags &= ~ID3_EXPERIMENTALTAG_FLAG;
	id3v2.version = ID3_VERSION;

	// write data
	if (new_size <= id3v2_size) {
		pack_sint28(id3v2_size - 10, id3v2.size);
		CopyMemory(tag_data, &id3v2, sizeof(id3v2_tag));

		SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
		if (!WriteFile(hFile, tag_data, id3v2_size, &result, 0) ||
			result != id3v2_size) {
			CloseHandle(hFile);
			tta_error(WRITE_ERROR, ttainfo->filename);
			return;
		}
		goto done;
	}

	pack_sint28(new_size - 10, id3v2.size);
	CopyMemory(tag_data, &id3v2, sizeof(id3v2_tag));
	offset = (int) new_size - id3v2_size;

	hMap = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0,
		ttainfo->FILESIZE + offset, NULL);
	if (!hMap) goto done;

	buffer = (unsigned char *)MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0,
		ttainfo->FILESIZE + offset);
	if (!buffer) {
		CloseHandle(hMap);
		goto done;
	}

	if (safe_mode) pause();

	MoveMemory(buffer + ((int)id3v2_size + offset),
		buffer + id3v2_size, ttainfo->FILESIZE);
	CopyMemory(buffer, tag_data, new_size);

	if (safe_mode) FlushViewOfFile((LPCVOID *) buffer, 0);
	UnmapViewOfFile((LPCVOID *) buffer);
	CloseHandle(hMap);

	ttainfo->FILESIZE += offset;
	ttainfo->id3v2.size = new_size;

	if (safe_mode) {
		info.FILESIZE = ttainfo->FILESIZE;
		info.id3v2.size = ttainfo->id3v2.size;
		seek_needed = decode_pos_ms;
		unpause();
	}

done:
	CloseHandle(hFile);
	HeapFree(heap, 0, tag_data);

	ttainfo->id3v2.id3has = 1;
}

//static void del_id3v2_tag (tta_info *ttainfo) {
//	HANDLE hFile, hMap;
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
