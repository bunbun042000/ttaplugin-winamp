// in_tta.cpp : Defines the initialization routines for the DLL.
//
/* Description:	 TTA input plug-in for upper Winamp 2.91
 *               with MediaLibrary Extension version
 * Developed by: Alexander Djourik <ald@true-audio.com>
 *               Pavel Zhilin <pzh@true-audio.com>
 *               (MediaLibrary Extension Bunbun <bunbun042000@yahoo.co.jp> )
 *
 * Copyright (c) 2005 Alexander Djourik. All rights reserved.
 *
 */

/* This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Please see the file COPYING in this directory for full copyright
 * information.
 */

#include "stdafx.h"
#include "in_tta.h"

#include "../Winamp SDK/Winamp/in2.h"
#include "../Winamp SDK/Agave/Language/api_language.h"
#include "FileInfo.h"
#include "DecodeFile.h"
#include "MediaLibrary.h"
#include <Shlwapi.h>
#include <taglib/tag.h>
#include "AlbumArt.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//
//TODO: If this DLL is dynamically linked against the MFC DLLs,
//		any functions exported from this DLL which call into
//		MFC must have the AFX_MANAGE_STATE macro added at the
//		very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the 
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//


// Cin_ttaApp

BEGIN_MESSAGE_MAP(Cin_ttaApp, CWinApp)
END_MESSAGE_MAP()


// Cin_ttaApp construction

Cin_ttaApp::Cin_ttaApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// Cin_ttaApp initialization

BOOL Cin_ttaApp::InitInstance()
{
	CWinApp::InitInstance();

	return TRUE;
}
// The one and only Cin_ttaApp object

Cin_ttaApp theApp;

// For Support Transcoder input (2007/10/15)
CDecodeFile playing_ttafile;
CMediaLibrary m_Tag;

static long	vis_buffer[BUFFER_SIZE * MAX_NCH];	// vis buffer
static BYTE pcm_buffer[BUFFER_SIZE];

#define  PLUGIN_VERSION "3.2 extended beta8 $Rev: 83 $"
#define  PROJECT_URL "<http://www.sourceforge.net>"

static HANDLE decoder_handle = NULL;
static DWORD WINAPI __stdcall DecoderThread (void *p);
static volatile int killDecoderThread = 0;
HANDLE decoderFileHANDLE;

void config(HWND hwndParent);
void about(HWND hwndParent);
void init();
void quit();
void getfileinfo(const char *file, char *title, int *length_in_ms);
int  infodlg(const char *file, HWND hwndParent);
int  isourfile(const char *fn);
int  play(const char *fn);
void pause();
void unpause();
int  ispaused();
void stop();
int  getlength();
int  getoutputtime();
void setoutputtime(int time_in_ms);
void setvolume(int volume);
void setpan(int pan);
void eq_set(int on, char data[10], int preamp);

void SetPlayingTitle(const char *filename, char *title);

In_Module mod = {
	IN_VER,
	"TTA Audio Decoder v" PLUGIN_VERSION " (x86)",
	NULL,	// hMainWindow
	NULL,	// hDllInstance
	"TTA\0TTA Audio File (*.TTA)\0",
	1,	// is_seekable
	1,	// uses output
	config,
	about,
	init,
	quit,
	getfileinfo,
	infodlg,
	isourfile,
	play,
	pause,
	unpause,
	ispaused,
	stop,
	getlength,
	getoutputtime,
	setoutputtime,
	setvolume,
	setpan,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, // vis stuff
	NULL, NULL,	// dsp
	eq_set,
	NULL,	// setinfo
	NULL		// out_mod
};

typedef struct _buffer {
	long		data_length;
	BYTE	   *buffer;
} data_buf;

static data_buf remain_data;

static void tta_error(int error, char *filename)
{
	char message[1024];
	char *name = NULL;

	if (filename) {
		char *p = filename + lstrlen(filename);
		while (*p != '\\' && p >= filename) p--;
		if (*p == '\\') name = ++p; else name = filename;
	}

	switch (error) {
	case tta::TTA_OPEN_ERROR:	wsprintf(message, "Can't open file:\n%s", name); break;
	case tta::TTA_FORMAT_ERROR:	wsprintf(message, "Unknown TTA format version:\n%s", name); break;
//	case tta::TTA_FILE_ERROR:	wsprintf(message, "Not supported file format:\n%s", name); break;
	case tta::TTA_FILE_ERROR:	wsprintf(message, "File is corrupted:\n%s", name); break;
	case tta::TTA_READ_ERROR:	wsprintf(message, "Can't read from file:\n%s", name); break;
	case tta::TTA_WRITE_ERROR:	wsprintf(message, "Can't write to file:\n%s", name); break;
	case tta::TTA_MEMORY_ERROR:	wsprintf(message, "Insufficient memory available"); break;
//	case tta::TTA_THREAD_ERROR:	wsprintf(message, "Error killing thread"); break;
	default:			wsprintf(message, "Unknown TTA decoder error"); break;
	}

	MessageBox(mod.hMainWindow, message, "TTA Decoder Error",
		MB_OK|MB_ICONERROR|MB_SYSTEMMODAL);
}

static BOOL CALLBACK config_dialog(HWND dialog, UINT message,
	WPARAM wparam, LPARAM lparam)
{

	switch (message) {
	case WM_INITDIALOG:
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wparam)) {
		case IDOK:
		case IDCANCEL:
			EndDialog(dialog, wparam);
			return TRUE;
		}
		break;
	}
	return FALSE;
}
static BOOL CALLBACK about_dialog(HWND dialog, UINT message,
	WPARAM wparam, LPARAM lparam)
{
	switch (message) {
	case WM_INITDIALOG:
		SetDlgItemText(dialog, IDC_PLUGIN_VERSION,
			"TTA Audio Decoder version " TTA_VERSION "\n"
			"Winamp plug-in version " PLUGIN_VERSION "\n" PROJECT_URL);
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wparam)) {
		case IDOK:
		case IDCANCEL:
			EndDialog(dialog, wparam);
			return TRUE;
		}
		break;
	}
	return FALSE;
}

void config(HWND hwndParent) 
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	DialogBox(mod.hDllInstance, MAKEINTRESOURCE(IDD_CONFIG),
		hwndParent, config_dialog);
}

void about(HWND hwndParent)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	DialogBox(mod.hDllInstance, MAKEINTRESOURCE(IDD_ABOUT),
		hwndParent, about_dialog);
}


void init()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	decoderFileHANDLE = INVALID_HANDLE_VALUE;
	Wasabi_Init();
}

void quit()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	Wasabi_Quit();
}


void getfileinfo(const char *file, char *title, int *length_in_ms)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if (!file || !*file) { // currently playing file
//		*length_in_ms = playing_ttafile.GetLengthbymsec();
//		SetPlayingTitle(playing_ttafile.GetFileName(), title);  
	} else {
		SetPlayingTitle(file, title);
		TagLib::FileName fn(file);
		TagLib::TrueAudio::File f(fn);
		*length_in_ms = f.audioProperties()->length() * 1000;
	}
}

int infodlg(const char *filename, HWND parent)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

//	CFileInfo *infodlg = new CFileInfo(NULL, filename);
//	infodlg->DoModal();

//	delete infodlg;
	return 0;
}

int isourfile(const char *filename)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return 0;
} 

int play(const char *fn)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	int maxlatency;
	unsigned long decoder_thread_id;
	int return_number;

	return_number = playing_ttafile.SetFileName(fn);
	if(return_number) return return_number;

	maxlatency = mod.outMod->Open(playing_ttafile.GetSampleRate(), 
		playing_ttafile.GetNumberofChannel(), playing_ttafile.GetOutputBPS(), -1, -1);
	if (maxlatency < 0) {
//		ttaTag.CloseFile();
		return 1;
	}

	// setup information display
	mod.SetInfo(playing_ttafile.GetBitrate(), playing_ttafile.GetSampleRate() / 1000, playing_ttafile.GetNumberofChannel(), 1);

	// initialize vis stuff
	mod.SAVSAInit(maxlatency, playing_ttafile.GetSampleRate());
	mod.VSASetInfo(playing_ttafile.GetNumberofChannel(), playing_ttafile.GetSampleRate());

	// set the output plug-ins default volume
	mod.outMod->SetVolume(-666);

	killDecoderThread = 0;

	decoder_handle = CreateThread(NULL, 0, DecoderThread, NULL, 0, &decoder_thread_id);
	if (!decoder_handle) { stop(); return 1; }

	return 0;
}

void pause() 
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	playing_ttafile.SetPaused(1);
	mod.outMod->Pause(1);
}

void unpause() 
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	playing_ttafile.SetPaused(0);
	mod.outMod->Pause(0);
}

int ispaused()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return playing_ttafile.GetPaused(); 
}

void stop()
{

	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if (decoder_handle) {
		killDecoderThread = 1;
		if (WaitForSingleObject(decoder_handle, INFINITE) == WAIT_TIMEOUT) {
//			tta_error(THREAD_ERROR, NULL);
			TerminateThread(decoder_handle, 0);
		}
		CloseHandle(decoder_handle);
		decoder_handle = NULL;
	}

	mod.SetInfo(0, 0, 0, 1);
	mod.outMod->Close();
	mod.SAVSADeInit();

	if (decoderFileHANDLE != INVALID_HANDLE_VALUE)
		CloseHandle(decoderFileHANDLE);
}

int getlength()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return playing_ttafile.GetLengthbymsec();
}

int getoutputtime()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return (int)(playing_ttafile.GetDecodePosMs() 
		+ (mod.outMod->GetOutputTime() - mod.outMod->GetWrittenTime())); 
}

void setoutputtime(int time_in_ms)
{
		AFX_MANAGE_STATE(AfxGetStaticModuleState());
		playing_ttafile.SetSeekNeeded(time_in_ms); 
}

void setvolume(int volume)
{
		AFX_MANAGE_STATE(AfxGetStaticModuleState());
		mod.outMod->SetVolume(volume);
}

void setpan(int pan)
{
		AFX_MANAGE_STATE(AfxGetStaticModuleState());
		mod.outMod->SetPan(pan);
}
void eq_set(int on, char data[10], int preamp)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// do nothing.
}


static void do_vis(unsigned char *data, int count, int bps, long double position)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	int i, bsize = bps >> 3;

	// Winamp visuals may have problems accepting sample sizes larger than
	// 16 bits, so we reduce the sample size here if necessary.

	if (bps > 16) {
		for (i = 0; i < count; i++) {
			int t = *((unsigned char *)(data + i * bsize));
			vis_buffer[i] = t >> (bps-16);
		}
		data = (unsigned char *)vis_buffer;
	}

	mod.SAAddPCMData(data, playing_ttafile.GetNumberofChannel(), 16, (int)position);
	mod.VSAAddPCMData(data, playing_ttafile.GetNumberofChannel(), 16, (int)position);
}


DWORD WINAPI __stdcall DecoderThread (void *p) {
	int done = 0;
	int len;
	int bitrate = playing_ttafile.GetBitrate();

	while (!killDecoderThread) {
		if (playing_ttafile.GetSeekNeeded() != -1) {
			mod.outMod->Flush((int)playing_ttafile.SeekPosition(&done));
		}
		if (done) {
			mod.outMod->CanWrite();
			if (!mod.outMod->IsPlaying()) {
				PostMessage(mod.hMainWindow, WM_WA_MPEG_EOF, 0, 0);
				return 0;
			} else Sleep(10);
		} else if (mod.outMod->CanWrite() >= 
			((BUFFER_LENGTH * playing_ttafile.GetNumberofChannel() * 
			playing_ttafile.GetByteSize()) << (mod.dsp_isactive()? 1:0))) {
			if (!(len = playing_ttafile.GetSamples(pcm_buffer, BUFFER_LENGTH, &bitrate))) { 
				done = 1;
			} else {
 				do_vis(pcm_buffer, len, playing_ttafile.GetOutputBPS(), playing_ttafile.GetDecodePosMs());
				if (mod.dsp_isactive()) {
					len = mod.dsp_dosamples((short *)pcm_buffer, len, playing_ttafile.GetOutputBPS(),
						playing_ttafile.GetNumberofChannel(), playing_ttafile.GetSampleRate());
				}
				mod.outMod->Write((char *)pcm_buffer, len * playing_ttafile.GetNumberofChannel() * (playing_ttafile.GetOutputBPS() >> 3));
			}
			mod.SetInfo(bitrate, playing_ttafile.GetSampleRate() / 1000, playing_ttafile.GetNumberofChannel(), 1);
		} else {
			Sleep(20);
		}
	}

	return 0;
}

void SetPlayingTitle(const char *filename, char *title)
{
	if (filename != NULL) { 
		if (TagLib::TrueAudio::File::isReadable(filename)) {

			TagLib::FileName fn(filename);
			TagLib::TrueAudio::File File(fn);

			if (!(File.tag()->artist().isEmpty()) || !(File.tag()->title().isEmpty()) || !(File.tag()->album().isEmpty())) {
				if(!(File.tag()->artist().isEmpty()) || !(File.tag()->title().isEmpty())) {
					wsprintf(title, _T("%s - %s"), 
						File.tag()->artist(),
						File.tag()->title());
				} else if (!(File.tag()->artist().isEmpty()) || !(File.tag()->album().isEmpty())) {
					wsprintf(title, _T("%s - %s"), 
						File.tag()->artist(),
						File.tag()->album());
				} else if (!(File.tag()->artist().isEmpty())) {
					wsprintf(title, _T("%s"), 
						File.tag()->artist());
				} else if (!(File.tag()->title().isEmpty())) {
					wsprintf(title, _T("%s"), 
						File.tag()->title());
				}
			} else {
				_TCHAR p[MAX_PATHLEN];
				::GetFileTitle(filename, p, MAX_PATHLEN - 1);
				lstrcpyn(title, p, _tcschr(p, '.') - p);
			}
		} else {
			// do nothing.
		}
	} else {
		// do nothing
	}
}


extern "C"
{
	__declspec(dllexport) In_Module* __cdecl winampGetInModule2(void)
	{
		AFX_MANAGE_STATE(AfxGetStaticModuleState());
		return &mod;
	}

	__declspec(dllexport) int __cdecl
	winampGetExtendedFileInfo(const char *fn, const char *data, char *dest, size_t destlen)
	{
		AFX_MANAGE_STATE(AfxGetStaticModuleState());
		return m_Tag.GetExtendedFileInfo(fn, data, dest, destlen);
	}

	__declspec(dllexport) int winampUseUnifiedFileInfoDlg(const char * fn)
	{
  // this will be called when Winamp is requested to show a File Info dialog for the selected file(s)
  // and this will allow you to override or forceable ignore the handling of a file or format
  // e.g. this will allow streams/urls to be ignored
	if (!_tcsncicmp(fn, _T("file://"), 7)) {
		fn += 7;
	}
	if (PathIsURL(fn)) { 
		return 0;
	}
	return 1;
	}

//	__declspec(dllexport) HWND winampAddUnifiedFileInfoPane(int n, const char * filename, HWND parent, char *name, size_t namelen)
//	{
//	if (n == 0)
//		{
//    // If you don't want Winamp to write the information via the winampSetExtendedFileInfo(W) handler
//    // and instead you want to do this then set the following property on the parent HWND.
//    // Once set then this will be effective for all of the pages which have been added.
//    SetPropW(parent,L"INBUILT_NOWRITEINFO", (HANDLE)1);
//    lstrcpyn(name,L"ID3v1",namelen);
//    return WASABI_API_CREATEDIALOGPARAM(IDD_INFO_ID3V1, parent, id3v1_dlgproc, (LPARAM)filename);
//		}
//	if (n == 1)
//		{
//		lstrcpyn(name,L"ID3v2",namelen);
//		return WASABI_API_CREATEDIALOGPARAM(IDD_INFO_ID3V2, parent, id3v2_dlgproc, (LPARAM)filename);
//		}
//	return NULL;
//	}

	__declspec( dllexport ) int winampSetExtendedFileInfo(const char *fn, const char *data, const char *val)
	{
		AFX_MANAGE_STATE(AfxGetStaticModuleState());
		return m_Tag.SetExtendedFileInfo(fn, data, val);
	}

	__declspec(dllexport) int winampWriteExtendedFileInfo()
	{
		AFX_MANAGE_STATE(AfxGetStaticModuleState());
		return m_Tag.WriteExtendedFileInfo();
	}
	__declspec(dllexport) intptr_t winampGetExtendedRead_open(const char *filename, int *size, int *bps, int *nch, int *srate)
	{
		AFX_MANAGE_STATE(AfxGetStaticModuleState());
		CDecodeFile *transcoding_ttafile;
		transcoding_ttafile = new CDecodeFile;

		if (!transcoding_ttafile) return 0;
		
		transcoding_ttafile->SetFileName((char *)filename);
	
		*bps = transcoding_ttafile->GetBitsperSample();
		*nch = transcoding_ttafile->GetNumberofChannel();
		*srate = transcoding_ttafile->GetSampleRate();
		*size = transcoding_ttafile->GetDataLength() * (*bps / 8) * (*nch);
		transcoding_ttafile->SetOutputBPS(*bps);
		remain_data.data_length = 0;
		remain_data.buffer = NULL;
	
		return (intptr_t)transcoding_ttafile;
	}

	__declspec( dllexport ) intptr_t winampGetExtendedRead_getData(intptr_t handle, char *dest, int len, int *killswitch)
	{
		AFX_MANAGE_STATE(AfxGetStaticModuleState());
		CDecodeFile *dec = (CDecodeFile *)handle;
		BYTE *buf = new BYTE[BUFFER_SIZE];
		int used = 0;
		int n = 0;
		int bitrate;
		int current_decode_size = 0;
		
		if (!dec->GetLengthbyFrame()) return 0;

		if (remain_data.data_length != 0)
		{
			while (min(len - used, remain_data.data_length) != 0 && !*killswitch) 
			{
				n = min(len - used, remain_data.data_length);
				memcpy_s(dest + used, len - used, remain_data.buffer + used, n);
				used += n;
				remain_data.data_length -= n;
			}
			if(remain_data.data_length != 0)
			{
				return used;
			} else {
				delete [] remain_data.buffer;
				remain_data.buffer = NULL;
			}
		}

		while (used < len && !*killswitch)
		{
		/* do we need to decode more? */
			if (n >= current_decode_size) {
				current_decode_size = dec->GetSamples(buf, BUFFER_LENGTH, &bitrate)
					* dec->GetBitsperSample() / 8 
					* dec->GetNumberofChannel();
	 			if (current_decode_size <= 0) break; /* end of stream */
			}
      
	
			/* copy as much as we can back to winamp */
			n = min(len - used, current_decode_size);
			if (n != 0)
			{
				memcpy_s(dest + used, len - used, buf, n);
				used += n;
			}
		}

		if (n != 0 && n < current_decode_size)
		{
			remain_data.data_length = current_decode_size - n;
			remain_data.buffer = new BYTE[remain_data.data_length];
			memcpy_s(remain_data.buffer, remain_data.data_length, buf + n, remain_data.data_length);
		}

		delete [] buf;
		return (intptr_t)used;
	}

	/* return nonzero on success, zero on failure. */
	__declspec( dllexport ) int winampGetExtendedRead_setTime(intptr_t handle, int millisecs)
	{
		AFX_MANAGE_STATE(AfxGetStaticModuleState());
		int done = 0;
		CDecodeFile *dec = (CDecodeFile *)handle;
		dec->SetDecodePosMs(millisecs);
		dec->SeekPosition(&done);
		return 1;
	}

	__declspec( dllexport ) void winampGetExtendedRead_close(intptr_t handle)
	{
		AFX_MANAGE_STATE(AfxGetStaticModuleState());
		CDecodeFile *dec = (CDecodeFile *)handle;
		delete dec;
		if (remain_data.buffer != NULL)
		{
			delete [] remain_data.buffer;
			remain_data.buffer = NULL;
		}
	}
}
