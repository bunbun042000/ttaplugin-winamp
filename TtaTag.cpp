// TtaTag.cpp: CTtaTag クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////

// $ LastChangedDate: $
#include "stdafx.h"
#include "in_tta.h"
#include "TtaTag.h"
#include "id3genre.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// 構築/消滅
//////////////////////////////////////////////////////////////////////

CTtaTag::CTtaTag()
{
	have_id3v1 = false;
	have_id3v2 = false;
	id3v2frames.clear();

}

CTtaTag::~CTtaTag()
{
	id3v2frames.clear();
}

int CTtaTag::ReadTag(const char *filename)
{
	unsigned long result;

	// File open
	::strncpy(FileName, filename, MAX_PATHLEN);

	HFILE = CreateFile(FileName, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (HFILE == INVALID_HANDLE_VALUE) {
		STATE = OPEN_ERROR;
		return -1;
	}

	FILESIZE = GetFileSize(HFILE, NULL);

	//Read ID3v1.1
	ReadID3v1tag();

	//Read ID3v2.3
	ReadID3v2tag();

	//Read TTA Header
	ReadTTAheader();

}

int CTtaTag::ReadID3v2tag()
{

	HANDLE hMap;
	unsigned char *buffer, *ptr;
	unsigned long result;
	int id3v2_size;

	if (!ReadFile(HFILE, &id3v2_header, sizeof(ID3v2_header), &result, NULL) ||
		result != sizeof(ID3v2_header) || memcmp(id3v2_header.id, "ID3", 3)) {
		SetFilePointer(ttainfo->HFILE, 0, NULL, FILE_BEGIN);
	}

	id3v2_size = unpack_sint28(id3v2_header.size) + 10;

	if ((id3v2_header.flags & ID3_UNSYNCHRONISATION_FLAG) ||
		(id3v2_header.flags & ID3_EXPERIMENTALTAG_FLAG) ||
		(id3v2_header.version < 3)) goto done;

	hMap = CreateFileMapping(HFILE, NULL, PAGE_READONLY, 0, id3v2_size, NULL);
	if (!hMap) goto done;

	buffer = (unsigned char *)MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, id3v2_size);
	if (!buffer) {
		CloseHandle(hMap);
		goto done;
	}

	ptr = buffer + 10;

	// skip extended header if present
	if (id3v2_header.flags & ID3_EXTENDEDHEADER_FLAG) {
		int offset = (int) unpack_sint32(ptr);
		ptr += offset;
	}

	// read id3v2 frames
	while (ptr - buffer < id3v2_size) {
		int data_size, frame_id;
		int size, comments = 0;
		char *data;

		// get frame header
		CopyMemory(&frame_header, ptr, sizeof(id3v2_frame));
		ptr += sizeof(id3v2_frame);
		data_size = unpack_sint32(frame_header.size);

		if (!*frame_header.id) break;

		if (!(frame_id = get_frame_id(frame_header.id))) {
			ptr += data_size;
			continue;
		}

		// skip unsupported frames
		if (frame_header.flags & FRAME_COMPRESSION_FLAG ||
			frame_header.flags & FRAME_ENCRYPTION_FLAG ||
			frame_header.flags & FRAME_UNSYNCHRONISATION_FLAG ||
			*ptr != FIELD_TEXT_ISO_8859_1) {
			ptr += data_size;
			continue;
		}

		ptr++; data_size--;

		switch (frame_id) {
		case TIT2:	data = ttainfo->id3v2.title;
					size = sizeof(ttainfo->id3v2.title) - 1; break;
		case TPE1:	data = ttainfo->id3v2.artist;
					size = sizeof(ttainfo->id3v2.artist) - 1; break;
		case TALB:	data = ttainfo->id3v2.album;
					size = sizeof(ttainfo->id3v2.album) - 1; break;
		case TRCK:	data = ttainfo->id3v2.track;
					size = sizeof(ttainfo->id3v2.track) - 1; break;
		case TYER:	data = ttainfo->id3v2.year;
					size = sizeof(ttainfo->id3v2.year) - 1; break;
		case TCON:	data = ttainfo->id3v2.genre;
					size = sizeof(ttainfo->id3v2.genre) - 1; break;
		case COMM:	if (comments++) goto next;
					data = ttainfo->id3v2.comment;
					size = sizeof(ttainfo->id3v2.comment) - 1;
					data_size -= 3; ptr += 3;
					// skip zero short description
					if (*ptr == 0) { data_size--; ptr++; }
					break;
		}
next:
		CopyMemory(data, ptr, (data_size <= size)? data_size:size);
		ptr += data_size;
	}

	UnmapViewOfFile((LPCVOID *) buffer);
	CloseHandle(hMap);

done:
	if (id3v2.flags & ID3_FOOTERPRESENT_FLAG) id3v2_size += 10;
	SetFilePointer(ttainfo->HFILE, id3v2_size, NULL, FILE_BEGIN);
	ttainfo->id3v2.size = id3v2_size;

	ttainfo->id3v2.id3has = 1;
}

int CTtaTag::ReadID3v1tag()
{
	SetFilePointer(HFILE, -(int) sizeof(ID3v1_tag), NULL, FILE_END);
	if (ReadFile(HFILE, &id3v1tag, sizeof(id3v1_tag), &result, NULL) &&
		result == sizeof(ID3v1_tag) && !memcmp(id3v1tag.id, "TAG", 3)) {
		have_id3v1 = true;
	}
	SetFilePointer(ttainfo->HFILE, 0, NULL, FILE_BEGIN);
	return (0);
}

