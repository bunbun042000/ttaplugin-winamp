// in_tta.cpp : Defines the initialization routines for the DLL.
//
// $LastChangedDate: 2007-10-17 23:35:11 +0900 (水, 17 10 2007) $
/* Description:	 TTA input plug-in for upper Winamp 2.91
 *               MediaLibrary Extension version
 * Developed by: Alexander Djourik <ald@true-audio.com>
 *               Pavel Zhilin <pzh@true-audio.com>
 *               (MediaLibrary Extension Bunbun <bunbun042000@yahoo.co.jp> )
 *
 * Copyright (c) 2005 Alexander Djourik. All rights reserved.
 *
 */

/*
 * This library is free software; you can redistribute it and/or
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

#define	STRICT


#include "stdafx.h"
#include "in_tta.h"
#include <windows.h>
#include <mmreg.h>
#include <process.h>

#include "in2.h"
#include "id3genre.h"
#include "resource.h"
#include "crc32.h"
#include "wa_ipc.h"
#include "TagInfo.h"
#include "TtaTag.h"
#include "FileInfo.h"
#include "DecodeFile.h"
#include "ttadec.h"


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

static long	vis_buffer[BUFFER_SIZE*MAX_NCH];	// vis buffer

#define  PLUGIN_VERSION "3.2 extended $Rev$"
#define  PROJECT_URL "<http://www.sourceforge.net>"


CTagInfo m_Tag;
CTtaTag  dlgTag;


static HANDLE decoder_handle = NULL;
static DWORD WINAPI __stdcall DecoderThread (void *p);
static volatile int killDecoderThread = 0;

void __cdecl config(HWND parent);
void __cdecl init();
void __cdecl about(HWND parent);
void __cdecl quit () {}
void __cdecl getfileinfo(char *filename, char *title, int *length_in_ms);
int __cdecl infodlg(char *filename, HWND parent);
void __cdecl pause();
void __cdecl unpause();
int __cdecl ispaused() { return playing_ttafile.GetPaused(); }
int __cdecl isourfile(char *filename) { return 0; } 
int __cdecl play(char *filename);
void __cdecl stop();
int  __cdecl getlength();
int  __cdecl getoutputtime();
void __cdecl setoutputtime(int time_in_ms);
void __cdecl setvolume(int volume);
void __cdecl setpan(int pan);
void __cdecl eq_set(int on, char data[10], int preamp);


In_Module mod = {
	IN_VER,
	"TTA Audio Decoder v" PLUGIN_VERSION " (x86)",
	0,	// hMainWindow
	0,	// hDllInstance
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
	0,0,0,0,0,0,0,0,0, // vis stuff
	0,0,	// dsp
	eq_set,
	NULL,	// setinfo
	0		// out_mod
};

HANDLE decoderFileHANDLE;


void pause() { playing_ttafile.SetPaused(1); mod.outMod->Pause(1); }
void unpause() { playing_ttafile.SetPaused(0); mod.outMod->Pause(0); }

void __cdecl init () {
	decoderFileHANDLE = INVALID_HANDLE_VALUE;
}

static void tta_error (int error, char *filename) {
	char message[1024];
	char *name = NULL;

	if (filename) {
		char *p = filename + lstrlen(filename);
		while (*p != '\\' && p >= filename) p--;
		if (*p == '\\') name = ++p; else name = filename;
	}

	switch (error) {
	case OPEN_ERROR:	wsprintf(message, "Can't open file:\n%s", name); break;
	case FORMAT_ERROR:	wsprintf(message, "Unknown TTA format version:\n%s", name); break;
	case PLAYER_ERROR:	wsprintf(message, "Not supported file format:\n%s", name); break;
	case FILE_ERROR:	wsprintf(message, "File is corrupted:\n%s", name); break;
	case READ_ERROR:	wsprintf(message, "Can't read from file:\n%s", name); break;
	case WRITE_ERROR:	wsprintf(message, "Can't write to file:\n%s", name); break;
	case MEMORY_ERROR:	wsprintf(message, "Insufficient memory available"); break;
	case THREAD_ERROR:	wsprintf(message, "Error killing thread"); break;
	default:			wsprintf(message, "Unknown TTA decoder error"); break;
	}

	MessageBox(mod.hMainWindow, message, "TTA Decoder Error",
		MB_OK|MB_ICONERROR|MB_SYSTEMMODAL);
}

static BOOL CALLBACK about_dialog (HWND dialog, UINT message,
	WPARAM wparam, LPARAM lparam) {
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

void __cdecl about (HWND parent) {
	DialogBox(mod.hDllInstance, MAKEINTRESOURCE(IDD_ABOUT),
		parent, about_dialog);
}

static BOOL CALLBACK config_dialog (HWND dialog, UINT message,
	WPARAM wparam, LPARAM lparam) {

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

void __cdecl config (HWND parent) {
	DialogBox(mod.hDllInstance, MAKEINTRESOURCE(IDD_CONFIG),
		parent, config_dialog);
}

int __cdecl infodlg (char *filename, HWND parent) {
	char *p, *fn, *caption;

	fn = filename;
	p = fn + lstrlen(fn);
	while (*p != '\\' && p >= fn) p--;
	if (*p == '\\') caption = ++p;
	else caption = fn;

//	AfxSetResourceHandle(mod.hDllInstance);

	CFileInfo *infodlg = new CFileInfo(NULL, filename);
	infodlg->DoModal();

	delete infodlg;

	return 0;
}


void __cdecl stop () {

	if (decoder_handle) {
		killDecoderThread = 1;
		if (WaitForSingleObject(decoder_handle, INFINITE) == WAIT_TIMEOUT) {
			tta_error(THREAD_ERROR, NULL);
			TerminateThread(decoder_handle, 0);
		}
		CloseHandle(decoder_handle);
		decoder_handle = NULL;
	}


	mod.outMod->Close();
	mod.SAVSADeInit();

	if (decoderFileHANDLE != INVALID_HANDLE_VALUE)
		CloseHandle(decoderFileHANDLE);
}

void __cdecl show_bitrate(CDecodeFile dec) {
	mod.SetInfo(dec.GetBitrate(), dec.GetSampleRate()/1000, dec.GetNumberofChannel(), 1);
}

int __cdecl play (char *filename) {
	int maxlatency;
	unsigned long decoder_thread_id;
	int return_number;

	return_number = playing_ttafile.SetFileName(filename);
	if(!return_number) return return_number;

	mod.is_seekable = playing_ttafile.GetSeekTableState();

	maxlatency = mod.outMod->Open(playing_ttafile.GetSampleRate(), 
		playing_ttafile.GetNumberofChannel(), playing_ttafile.GetOutputBPS(), -1, -1);
	if (maxlatency < 0) {
//		ttaTag.CloseFile();
		return 1;
	}

	// setup information display
	show_bitrate(playing_ttafile);

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

int  __cdecl getlength () { return playing_ttafile.GetLengthbymsec(); }
int  __cdecl getoutputtime () { return playing_ttafile.GetDecodePosMs() + (mod.outMod->GetOutputTime() - mod.outMod->GetWrittenTime()); }
void __cdecl setoutputtime (int time_in_ms) {playing_ttafile.SetSeekNeeded(time_in_ms); }
void __cdecl setvolume (int volume) { mod.outMod->SetVolume(volume); }
void __cdecl setpan (int pan) { mod.outMod->SetPan(pan); }
void __cdecl eq_set (int on, char data[10], int preamp) {}


void __cdecl getfileinfo (char *filename, char *title, int *length_in_ms) {
	if (!playing_ttafile.GetFileName() || !*filename) { // currently playing file
		*length_in_ms = playing_ttafile.GetLengthbymsec();
		playing_ttafile.SetPlayTitle(title);
	} else {
		CTtaTag ttaTag;
		ttaTag.ReadTag(filename);
		*length_in_ms = ttaTag.GetLengthbymsec();
		ttaTag.SetPlayTitle(title);
	}
}

static void do_vis(unsigned char *data, int count, int bps, int position) {
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

	mod.SAAddPCMData(data, playing_ttafile.GetNumberofChannel(), 16, position);
	mod.VSAAddPCMData(data, playing_ttafile.GetNumberofChannel(), 16, position);
}


DWORD WINAPI __stdcall DecoderThread (void *p) {
	int done = 0;
	int len;

	BYTE pcm_buffer[BUFFER_SIZE];

	while (!killDecoderThread) {
		if (playing_ttafile.GetSeekNeeded() != -1) {
			mod.outMod->Flush(playing_ttafile.SeekPosition(&done));
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
			if (!(len = playing_ttafile.GetSamples(pcm_buffer, BUFFER_LENGTH))) done = 1;
			else {
				do_vis(pcm_buffer, len, playing_ttafile.GetOutputBPS(), playing_ttafile.GetDecodePosMs());
				if (mod.dsp_isactive())
					len = mod.dsp_dosamples((short *)pcm_buffer, len, playing_ttafile.GetOutputBPS(),
						playing_ttafile.GetNumberofChannel(), playing_ttafile.GetSampleRate());
				mod.outMod->Write((char *)pcm_buffer, len * playing_ttafile.GetNumberofChannel() * (playing_ttafile.GetOutputBPS() >> 3));
			}
			show_bitrate(playing_ttafile);
		} else Sleep(20);
	}

	return 0;
}

extern "C"
{
	__declspec(dllexport) In_Module* __cdecl winampGetInModule2(void)
	{
		return &mod;
	}

	__declspec(dllexport) int __cdecl
	winampGetExtendedFileInfo(extendedFileInfoStruct ExtendedFileInfo)
	{
		return m_Tag.GetExtendedFileInfo(mod.hMainWindow, &ExtendedFileInfo);
	}

//	__declspec( dllexport ) intptr_t winampGetExtendedRead_open(const char *filename, int *size, int *bps, int *nch, int *srate)
//	{
//		CDecodeFile *transcoding_ttafile;
//		transcoding_ttafile = new CDecodeFile;
//
//		if (!transcoding_ttafile) return 0;
//		
//		transcoding_ttafile->SetFileName((char *)filename);
//		transcoding_ttafile->SetOutputBPS(*bps);
//	
//		*bps = transcoding_ttafile->GetBitsperSample();
//		*nch = transcoding_ttafile->GetNumberofChannel();
//		*srate = transcoding_ttafile->GetSampleRate();
//		*size = transcoding_ttafile->GetLengthbyFrame() * (*bps / 8) * (*nch);
//		
//		return (intptr_t)transcoding_ttafile;
//	}

//	__declspec( dllexport ) intptr_t winampGetExtendedRead_getData(intptr_t handle, char *dest, int len, int *killswitch)
//	{
//		CDecodeFile *dec = (CDecodeFile *)handle;
//		int used = 0;
//		int n;
////
//	//	if (e->file_info.eof && e->len == e->used) return 0;
////
//		while (used < len && !*killswitch)
//		{
		/* do we need to decode more? */
//			if (e->used >= e->len) {
//			e->used = 0;
//			e->len = get_samples((BYTE *)&e->sample_buffer, BUFFER_LENGTH);
//      
//			if (e->len <= 0) break; /* end of stream */
//			}

			/* copy as much as we can back to winamp */
//			n = min(len - used, e->len - e->used);
//			if (n)
//			{
//				memcpy(dest + used, e->sample_buffer + e->used, n);
//				e->used += n;
//				used += n;
//			}
//		}
//		return used;
//	}

	/* return nonzero on success, zero on failure. */
//	__declspec( dllexport ) int winampGetExtendedRead_setTime(intptr_t handle, int millisecs)
//	{
//		CDecodeFile *dec = (CDecodeFile *)handle;
//		dec->SetDecodePosMs(millisecs);
//		dec- = 0;
//		e->used = 0;
//		return 1;
//	}

//	__declspec( dllexport ) void winampGetExtendedRead_close(intptr_t handle)
//	{
//		CDecodeFile *dec = (CDecodeFile *)handle;
///		FLAC_plugin__decoder_finish(e->decoder);
//		FLAC_plugin__decoder_delete(e->decoder);
//		delete dec;
//	}
}
