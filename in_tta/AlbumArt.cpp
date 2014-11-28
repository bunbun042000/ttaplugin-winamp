/*
The ttaplugin-winamp project.
Copyright (C) 2005-2013 Yamagata Fumihiro

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <stdio.h>
#include <Wasabi/api/service/api_service.h>
#include <Agave/Config/api_config.h>
#include <Wasabi/api/memmgr/api_memmgr.h>
#include <Winamp/in2.h>
#include <Winamp/wa_ipc.h>
#include <Wasabi/api/service/waservicefactory.h>
#include <Agave/AlbumArt/svc_albumArtProvider.h>
#include "AlbumArt.h"
#include <taglib/trueaudiofile.h>
#include <taglib/id3v2tag.h>
#include <taglib/attachedpictureframe.h>
#include <taglib/tag.h>
#include "common.h"

static const int MIME_LENGTH = 64;

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
	TTA_AlbumArtProvider();
	virtual ~TTA_AlbumArtProvider();
	bool IsMine(const wchar_t *filename);
	int ProviderType();
	// implementation note: use WASABI_API_MEMMGR to alloc bits and mimetype, so that the recipient can free through that
	int GetAlbumArtData(const wchar_t *filename, const wchar_t *type, void **bits, size_t *len, wchar_t **mimeType);
	int SetAlbumArtData(const wchar_t *filename, const wchar_t *type, void *bits, size_t len, const wchar_t *mimeType);
	int DeleteAlbumArt(const wchar_t *filename, const wchar_t *type);
protected:
	RECVS_DISPATCH;
	CRITICAL_SECTION	CriticalSection;
};

TTA_AlbumArtProvider::TTA_AlbumArtProvider() : svc_albumArtProvider()
{
	::InitializeCriticalSection(&CriticalSection);

}

TTA_AlbumArtProvider::~TTA_AlbumArtProvider()
{
	::DeleteCriticalSection(&CriticalSection);

}

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
	else
	{
		// Do nothing
	}
	return false;
}

int TTA_AlbumArtProvider::ProviderType()
{
	return ALBUMARTPROVIDER_TYPE_EMBEDDED;
}

int TTA_AlbumArtProvider::GetAlbumArtData(const wchar_t *filename, const wchar_t *type, void **bits, size_t *len, wchar_t **mime_type)
{
	bool FindTag = false;
	size_t tag_size = 0;
    int retval = ALBUMARTPROVIDER_FAILURE;
	TagLib::String mimeType;
	char demandFile[MAX_PATHLEN];

	::EnterCriticalSection(&CriticalSection);

	if(!filename || !*filename || _wcsicmp (type, L"cover")) {
		::LeaveCriticalSection(&CriticalSection);
        return retval;
	} else {
		// do nothing
	}

	if(!bits || !len || !mime_type) {
		::LeaveCriticalSection(&CriticalSection);
		return retval;
	} else {
		// do nothing
	}

	size_t origsize = wcslen(filename) + 1;
	int ret = WideCharToMultiByte(CP_ACP, 0, filename, origsize, demandFile, MAX_PATHLEN - 1, NULL, NULL);

	if (ret == 0)
	{
		::LeaveCriticalSection(&CriticalSection);
		return retval;
	}
	else
	{
		// Do nothing
	}

	if (true != TagLib::File::isReadable(demandFile))
	{
		::LeaveCriticalSection(&CriticalSection);
		return retval;
	}
	else
	{
		// Do nothing
	}

	TagLib::TrueAudio::File TagFile(demandFile, false);
	if (true != TagFile.isValid()) {
		::LeaveCriticalSection(&CriticalSection);
		return retval;
	} else {
		// do nothing
	}

	// read Album Art
	TagLib::ByteVector AlbumArt = 
		TagFile.ID3v2Tag()->albumArt(TagLib::ID3v2::AttachedPictureFrame::FrontCover, mimeType);

	if(AlbumArt != TagLib::ByteVector::null) {
		*len = AlbumArt.size();
		*bits = (char *)Wasabi_Malloc(*len);
		if (NULL == *bits) {
			::LeaveCriticalSection(&CriticalSection);
			return retval;
		} else {
			// do nothing
		}

		errno_t err = memcpy_s(*bits, AlbumArt.size(), AlbumArt.data(), AlbumArt.size());
		if (err) {
			::LeaveCriticalSection(&CriticalSection);
			return retval;
		} else {
			// do nothing
		}

		retval = ALBUMARTPROVIDER_SUCCESS;

		size_t string_len;
		TagLib::String extension = mimeType.substr(mimeType.find("/") + 1);
		*mime_type = (wchar_t *)Wasabi_Malloc(extension.size() * 2 + 2);
		if (NULL == *mime_type) {
			if (NULL != *bits) {
				Wasabi_Free(*bits);
			} else {
				// do nothing
			}
			::LeaveCriticalSection(&CriticalSection);
			return retval;
		} else {
			// do nothing
		}

		mbstowcs_s(&string_len, *mime_type, extension.size() + 1, extension.toCString(), _TRUNCATE);

		if (retval) {
			if (NULL != *bits) {
				Wasabi_Free(*bits);
			} else {
				// do nothing
			}
			if (NULL != *mime_type) {
				Wasabi_Free(*bits);
			} else {
				// do nothing
			}
		}
	} else {
		// do nothing
	}

	::LeaveCriticalSection(&CriticalSection);
    return retval;
}

int TTA_AlbumArtProvider::SetAlbumArtData(const wchar_t *filename, const wchar_t *type, void *bits, size_t len, const wchar_t *mime_type)
{

	bool FindTag = false;
    int retval = ALBUMARTPROVIDER_FAILURE;
	TagLib::String mimeType(L"");
	char demandFile[MAX_PATHLEN];
	int size = 0;
	TagLib::ID3v2::AttachedPictureFrame::Type artType = TagLib::ID3v2::AttachedPictureFrame::Other;

	::EnterCriticalSection(&CriticalSection);

	if(!filename || !*filename) {
		::LeaveCriticalSection(&CriticalSection);
        return retval;
	}

	size_t origsize = wcslen(filename) + 1;
	size_t convertedChars = 0;
	int ret = WideCharToMultiByte(CP_ACP, 0, filename, origsize, demandFile, MAX_PATHLEN - 1, NULL, NULL);

	if(!ret) {
		::LeaveCriticalSection(&CriticalSection);
		return retval;
	} else {
		// do nothing
	}

	// If target file cannot access
	if (TagLib::File::isWritable(demandFile) == false) {
		::LeaveCriticalSection(&CriticalSection);
		return retval;
	} else {
		// do nothing
	}

	TagLib::ByteVector AlbumArt;

	if (!bits) {
		//delete AlbumArt
		AlbumArt.setData(NULL, 0);

	} else if(bits || !len || !mime_type) {
		::LeaveCriticalSection(&CriticalSection);
		return retval;
	} else {
		mimeType = L"image/";
		mimeType += mime_type;
		size = len;
		artType = TagLib::ID3v2::AttachedPictureFrame::FrontCover;
		AlbumArt.setData((const char *)bits, (TagLib::uint)size);
	}

	TagLib::TrueAudio::File TagFile(demandFile);
	if (!TagFile.isValid()) {
		::LeaveCriticalSection(&CriticalSection);
		return retval;
	} else {
		// do nothing
	}

	retval = ALBUMARTPROVIDER_SUCCESS;

	TagFile.ID3v2Tag()->setAlbumArt(AlbumArt, artType, mimeType);
	TagFile.save();

	::LeaveCriticalSection(&CriticalSection);

	return retval;

}

int TTA_AlbumArtProvider::DeleteAlbumArt(const wchar_t *filename, const wchar_t *type)
{
	return SetAlbumArtData(filename, type, NULL, 0, L"image/jpeg");
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
