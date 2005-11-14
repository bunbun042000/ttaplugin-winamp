// ID3v1.h: CID3v1 クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ID3V1_H__F52065F6_B156_468F_8A35_DAD534E31131__INCLUDED_)
#define AFX_ID3V1_H__F52065F6_B156_468F_8A35_DAD534E31131__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// ID3 ver1.1
struct v1tag
{
	char  id[3];
	char  title[30];
	char  artist[30];
	char  album[30];
	char  year[4];
	char  comment[28];
	char  zero;
	char  track;
	char  genre;
};

class CID3v1  
{
public:
	CID3v1();
	virtual ~CID3v1();
	int     ReadTag(const char *filename);
	bool    SaveTag();
	void    DeleteTag();

//	CString GetFileName(){return FileName;}
//	void    SetFileName(const char *filename);
	CString GetTitle() {return tag.title;}
	void    SetTitle(const char *title);
	CString GetArtist(){return tag.artist;}
	void    SetArtist(const char *artist);
	CString GetAlbum() {return tag.album;}
	void    SetAlbum(const char *album);
	CString GetYear() {return tag.year;}
	void    SetYear(const char *year);
	CString GetComment() {return tag.comment;}
	void    SetComment(const char *comment);
	CString GetTrack() {return tag.track;}
	void    SetTrack(const char track);
	CString GetGenre() {return tag.genre;}
	void    SetGenre(const char genre);

	bool    hastag() {return has_tag;}
private:
	HANDLE	HFILE;
	char	FileName[MAX_PATHLEN];	// filename
	int		STATE; // filestate

	v1tag	tag;
	bool	has_tag;

	void  error(int error_no, const char *FileName);
};

#endif // !defined(AFX_ID3V1_H__F52065F6_B156_468F_8A35_DAD534E31131__INCLUDED_)
