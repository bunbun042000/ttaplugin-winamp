
#include "stdafx.h"
#include <stdio.h>
#include "../Winamp SDK/Wasabi/api/service/api_service.h"
#include "../Winamp SDK/Agave/Config/api_config.h"
#include "../Winamp SDK/Wasabi/api/memmgr/api_memmgr.h"
#include "../Winamp SDK/Winamp/in2.h"
#include "../Winamp SDK/Winamp/wa_ipc.h"
#include "../Winamp SDK/Wasabi/api/service/waservicefactory.h"
#include "../Winamp SDK/Agave/AlbumArt/svc_albumArtProvider.h"
#include "AlbumArt.h"
#include <taglib/trueaudiofile.h>
#include <taglib/id3v2tag.h>
#include <taglib/attachedpictureframe.h>
#include <taglib/tag.h>
#include "MediaLibrary.h"
//#include <taglib/tstring.h>

static const int MIME_LENGTH = 64;
extern CMediaLibrary m_Tag;

class AlbumArtFactory : public waServiceFactory
{
public:
	FOURCC GetServiceType();
	const char *GetServiceName();
	GUID GetGUID();
	void *GetInterface(int global_lock);
	int SupportNonLockingInterface();
	int ReleaseInterface(void *ifc);
	const char *GetTestString();
	int ServiceNotify(int msg, int param1, int param2);
	
protected:
	RECVS_DISPATCH;
};

	extern In_Module mod; // TODO: change if you called yours something else

#define WASABI_API_MEMMGR memmgr

static api_config *AGAVE_API_CONFIG = 0;
static api_service *WASABI_API_SVC = 0;
static api_memmgr *WASABI_API_MEMMGR=0;

static AlbumArtFactory albumArtFactory;

void Wasabi_Init()
{
	WASABI_API_SVC = (api_service *)SendMessage(mod.hMainWindow, WM_WA_IPC, 0, IPC_GET_API_SERVICE);

	if (WASABI_API_SVC == 0 || WASABI_API_SVC == (api_service *) 1) {
		WASABI_API_SVC = 0;
		return;
	}

	WASABI_API_SVC->service_register(&albumArtFactory);

	waServiceFactory *sf = (waServiceFactory *)WASABI_API_SVC->service_getServiceByGuid(AgaveConfigGUID);
	if (sf)	
		AGAVE_API_CONFIG= (api_config *)sf->getInterface();
	sf = (waServiceFactory *)WASABI_API_SVC->service_getServiceByGuid(memMgrApiServiceGuid);
	if (sf)	
		WASABI_API_MEMMGR= (api_memmgr *)sf->getInterface();
}

void Wasabi_Quit()
{
	if (WASABI_API_SVC) {
		waServiceFactory *sf = (waServiceFactory *)WASABI_API_SVC->service_getServiceByGuid(AgaveConfigGUID);
		if (sf)	
			sf->releaseInterface(AGAVE_API_CONFIG);
		sf = (waServiceFactory *)WASABI_API_SVC->service_getServiceByGuid(memMgrApiServiceGuid);
		if (sf)	
			sf->releaseInterface(WASABI_API_MEMMGR);

		WASABI_API_SVC->service_deregister(&albumArtFactory);
	}
}

void *Wasabi_Malloc(size_t size_in_bytes)
{
	return WASABI_API_MEMMGR->sysMalloc(size_in_bytes);
}

void Wasabi_Free(void *memory_block)
{
	WASABI_API_MEMMGR->sysFree(memory_block);
}

class TTA_AlbumArtProvider : public svc_albumArtProvider
{
public:
	bool IsMine(const wchar_t *filename);
	int ProviderType();
	// implementation note: use WASABI_API_MEMMGR to alloc bits and mimetype, so that the recipient can free through that
	int GetAlbumArtData(const wchar_t *filename, const wchar_t *type, void **bits, size_t *len, wchar_t **mimeType);
	int SetAlbumArtData(const wchar_t *filename, const wchar_t *type, void *bits, size_t len, const wchar_t *mimeType);
	int DeleteAlbumArt(const wchar_t *filename, const wchar_t *type);
protected:
	RECVS_DISPATCH;
};

static const wchar_t *GetLastCharactercW(const wchar_t *string)
{
	if (!string || !*string)
		return string;

	return CharPrevW(string, string+lstrlenW(string));
}

static const wchar_t *scanstr_backW(const wchar_t *str, const wchar_t *toscan, const wchar_t *defval)
{
	const wchar_t *s = GetLastCharactercW(str);
	if (!str[0]) return defval;
	if (!toscan || !toscan[0]) return defval; 
	while (1)
	{
		const wchar_t *t = toscan;
		while (*t)
		{
			if (*t == *s) return s;
			t = CharNextW(t);
		}
		t = CharPrevW(str, s);
		if (t == s)
			return defval;
		s = t;
	}
}

static const wchar_t *extensionW(const wchar_t *fn)
{
	const wchar_t *end = scanstr_backW(fn, L"./\\", 0);
	if (!end)
		return (fn+lstrlenW(fn));

	if (*end == L'.')
		return end+1;

	return (fn+lstrlenW(fn));
}

bool TTA_AlbumArtProvider::IsMine(const wchar_t *filename)
{
	const wchar_t *extension = extensionW(filename);
	if (extension && *extension)
	{
	return (_wcsicmp (extension, L"tta") == 0) ? true : false;
	}
	return false;
}

int TTA_AlbumArtProvider::ProviderType()
{
	return ALBUMARTPROVIDER_TYPE_EMBEDDED;
}

int TTA_AlbumArtProvider::GetAlbumArtData(const wchar_t *filename, const wchar_t *type, void **bits, size_t *len, wchar_t **mime_type)
{
	return m_Tag.GetAlbumArtData(filename, type, bits, len, mime_type);
}

int TTA_AlbumArtProvider::SetAlbumArtData(const wchar_t *filename, const wchar_t *type, void *bits, size_t len, const wchar_t *mime_type)
{
	return ALBUMARTPROVIDER_READONLY; // read-noly
}

int TTA_AlbumArtProvider::DeleteAlbumArt(const wchar_t *filename, const wchar_t *type)
{
	return ALBUMARTPROVIDER_READONLY; // read-noly
}

#define CBCLASS TTA_AlbumArtProvider
START_DISPATCH;
CB(SVC_ALBUMARTPROVIDER_PROVIDERTYPE, ProviderType);
CB(SVC_ALBUMARTPROVIDER_GETALBUMARTDATA, GetAlbumArtData);
CB(SVC_ALBUMARTPROVIDER_ISMINE, IsMine);
CB(SVC_ALBUMARTPROVIDER_SETALBUMARTDATA, SetAlbumArtData);
CB(SVC_ALBUMARTPROVIDER_DELETEALBUMART, DeleteAlbumArt);
END_DISPATCH;
#undef CBCLASS

static TTA_AlbumArtProvider albumArtProvider;

// {bb653840-6dab-4867-9f42-A772E4068C81}
static const GUID TTA_albumartproviderGUID = 
{ 0xbb653840, 0x6dab, 0x4867, { 0x9f, 0x42, 0xa7, 0x72, 0xe4, 0x05, 0x8c, 0x81 } };


FOURCC AlbumArtFactory::GetServiceType()
{
	return svc_albumArtProvider::SERVICETYPE;
}

const char *AlbumArtFactory::GetServiceName()
{
	return "TTA Album Art Provider";
}

GUID AlbumArtFactory::GetGUID()
{
	return TTA_albumartproviderGUID;
}

void *AlbumArtFactory::GetInterface(int global_lock)
{
	return &albumArtProvider;
}

int AlbumArtFactory::SupportNonLockingInterface()
{
	return 1;
}

int AlbumArtFactory::ReleaseInterface(void *ifc)
{
	//WASABI_API_SVC->service_unlock(ifc);
	return 1;
}

const char *AlbumArtFactory::GetTestString()
{
	return 0;
}

int AlbumArtFactory::ServiceNotify(int msg, int param1, int param2)
{
	return 1;
}

#define CBCLASS AlbumArtFactory
START_DISPATCH;
CB(WASERVICEFACTORY_GETSERVICETYPE, GetServiceType)
CB(WASERVICEFACTORY_GETSERVICENAME, GetServiceName)
CB(WASERVICEFACTORY_GETGUID, GetGUID)
CB(WASERVICEFACTORY_GETINTERFACE, GetInterface)
CB(WASERVICEFACTORY_SUPPORTNONLOCKINGGETINTERFACE, SupportNonLockingInterface)
CB(WASERVICEFACTORY_RELEASEINTERFACE, ReleaseInterface)
CB(WASERVICEFACTORY_GETTESTSTRING, GetTestString)
CB(WASERVICEFACTORY_SERVICENOTIFY, ServiceNotify)
END_DISPATCH;
#undef CBCLASS
