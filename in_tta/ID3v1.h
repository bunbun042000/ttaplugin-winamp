// ID3v1.h: CID3v1 クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ID3V1_H__F52065F6_B156_468F_8A35_DAD534E31131__INCLUDED_)
#define AFX_ID3V1_H__F52065F6_B156_468F_8A35_DAD534E31131__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

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

// length for ID3v1.1
#define ID3V1_TITLELENGTH	30
#define ID3V1_ARTISTLENGTH	30
#define ID3V1_ALBUMLENGTH	30
#define ID3V1_YEARLENGTH	4
#define ID3V1_COMMENTLENGTH	28

// ID3 ver1.1
struct v1tag
{
	char  id[3];
	char  title[ID3V1_TITLELENGTH];
	char  artist[ID3V1_ARTISTLENGTH];
	char  album[ID3V1_ALBUMLENGTH];
	char  year[ID3V1_YEARLENGTH];
	char  comment[ID3V1_COMMENTLENGTH];
	char  zero;
	unsigned char  track;
	unsigned char  genre;
};

class CID3v1  
{
public:
	CID3v1();
	virtual ~CID3v1();
	bool			ReadTag(HWND hMainWindow, const char *filename);
	bool			SaveTag(HWND hMainWindow);
	void			DeleteTag(HWND hMainWindow);

//	CString GetFileName(){return FileName;}
//	void    SetFileName(const char *filename);
	CString			GetTitle() {return Title.GetBufferSetLength(ID3V1_TITLELENGTH);}
	void			SetTitle(const char *title);
	CString			GetArtist(){return Artist.GetBufferSetLength(ID3V1_ARTISTLENGTH);}
	void			SetArtist(const char *artist);
	CString			GetAlbum() {return Album.GetBufferSetLength(ID3V1_ALBUMLENGTH);}
	void			SetAlbum(const char *album);
	CString			GetYear() {return Year.GetBufferSetLength(ID3V1_YEARLENGTH);}
	void			SetYear(const char *year);
	CString			GetComment() {return Comment.GetBufferSetLength(ID3V1_COMMENTLENGTH);}
	void			SetComment(const char *comment);
	unsigned char	GetTrack() {return Track;}
	void			SetTrack(const unsigned char track);
	unsigned char	GetGenre() {return Genre;}
	void			SetGenre(const unsigned char genre);

	bool    hasTag() {return has_tag;}
private:
	HANDLE	HFILE;
	CString	FileName;	// filename
	int		STATE; // filestate

	CString Title;
	CString Artist;
	CString Album;
	CString Year;
	CString Comment;
	unsigned char    Track;
	unsigned char    Genre;

	bool	has_tag;

	void  error(HWND hMainWindow, int error_no);
};

#endif // !defined(AFX_ID3V1_H__F52065F6_B156_468F_8A35_DAD534E31131__INCLUDED_)
