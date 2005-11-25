// TagInfo.h: CTagInfo クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TAGINFO_H__997DC726_50DB_46B4_A156_DB5E92EC2BE8__INCLUDED_)
#define AFX_TAGINFO_H__997DC726_50DB_46B4_A156_DB5E92EC2BE8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define	MAX_MUSICTEXT	512

//#include <windows.h>
#include <mmreg.h>
#include <stdlib.h>
#include "wa_ipc.h"
#include "ttadec.h"
#include "in_tta.h"
#include "TtaTag.h"

extern char *genre[];

struct TagInfo
{
	unsigned long	Length;
	char			FileName[MAX_PATHLEN];
	char			Title[MAX_MUSICTEXT];
	char			Artist[MAX_MUSICTEXT];
	char			Comment[MAX_MUSICTEXT];
	char			Album[MAX_MUSICTEXT];
	char			Year[MAX_MUSICTEXT];
	char			Genre[MAX_MUSICTEXT];
	char			Track[MAX_MUSICTEXT];
	char			Composer[MAX_MUSICTEXT];
	char			OrgArtist[MAX_MUSICTEXT];
	char			Copyright[MAX_MUSICTEXT];
	char			Encoder[MAX_MUSICTEXT];
};

class CTagInfo  
{
public:
	CTagInfo();
	virtual ~CTagInfo();
	int  GetExtendedFileInfo(HWND hMainWindow, extendedFileInfoStruct *ExtendedFileInfo);
	
private:
	CRITICAL_SECTION	CriticalSection;
	CTtaTag				ttatag;
	TagInfo				Cache;
	DWORD				GetTagTime;

	void FlushCache(void);
	bool GetTagInfo(HWND hMainWindow);

};

#endif // !defined(AFX_TAGINFO_H__997DC726_50DB_46B4_A156_DB5E92EC2BE8__INCLUDED_)
