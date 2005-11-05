/*
 * id3tag.h
 *
 * Description:	ID3 functions prototypes
 *
 */

#ifndef ID3TAG_H_INCLUDED
#define ID3TAG_H_INCLUDED

#define MAX_LINE 4096
#define ID3_VERSION 3

/* ID3 common headers set */

#define TIT2	1
#define TPE1	2
#define TALB	3
#define TRCK	4
#define TYER	5
#define TCON	6
#define COMM	7

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

typedef struct {
	unsigned char  id[3];
	char  title[30];
	char  artist[30];
	char  album[30];
	char  year[4];
	char  comment[28];
	char  zero;
	char  track;
	char  genre;
} id3v1_tag;

typedef struct {
	char  id[3];
	short version;
	char  flags;
	char  size[4];
} id3v2_tag;

typedef struct {
	char  id[4];
	unsigned char  size[4];
	short flags;
} id3v2_frame;

typedef struct {
	char  name[31];
	char  title[31];
	char  artist[31];
	char  album[31];
	char  comment[31];
	char  year[5];
	char  track;
	int  genre;
	char  id3has;
} id3v1_data;

typedef struct {
	char  name[MAX_PATH];
	char  title[MAX_LINE];
	char  artist[MAX_LINE];
	char  album[MAX_LINE];
	char  comment[MAX_LINE];
	char  year[5];
	char  track[3];
	char  genre[256];
	char  id3has;
	long  size;
} id3v2_data;

#endif // ID3TAG_H_INCLUDED