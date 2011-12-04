// in_tta.cpp : Defines the initialization routines for the DLL.
//
/* Description:	 TTA input plug-in for upper Winamp 2.91
 *               with MediaLibrary Extension version
 * Developed by: Alexander Djourik <ald@true-audio.com>
 *               Pavel Zhilin <pzh@true-audio.com>
 *               (MediaLibrary Extension Yamagata Fumihiro <bunbun042000@gmail.com> )
 *
 * Copyright (c) 2005 Alexander Djourik. All rights reserved.
 *
 */

/*
The ttaplugin-winamp project.
Copyright (C) 2005-2011 Yamagata Fumihiro

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

#include <Shlwapi.h>
#include <stdlib.h>

#include <Winamp/in2.h>
#include <Agave/Language/api_language.h>

#include <taglib/tag.h>
#include <taglib/trueaudiofile.h>
#include <taglib/tstring.h>

#include "AlbumArt.h"
#include "DecodeFile.h"
#include "MediaLibrary.h"

#include "resource.h"

#define  PLUGIN_VERSION "3.2 extended Beta13"
#define  PROJECT_URL "<git://github.com/bunbun042000/ttaplugin-winamp.git>"
#define  LIBTTA_VERSION "based on libtta++2.1"
#define  ORIGINAL_CREADIT01 "Plugin is written by Alexander Djourik, Pavel Zhilin and Anton Gorbunov.\n"
#define  ORIGINAL_CREADIT02 "Copyright (c) 2003 Alexander Djourik.\n"
#define  ORIGINAL_CREADIT03 "All rights reserved.\n"
#define  CREADIT01 "Modified by Yamagata Fumihiro, 2005-2011\n"
#define  CREADIT02 "Copyright (C)2005-2011 Yamagata Fumihiro.\n"

// For Support Transcoder input (2007/10/15)
CDecodeFile playing_ttafile;
CMediaLibrary m_Tag;

static long	vis_buffer[BUFFER_SIZE * MAX_NCH];	// vis buffer
static BYTE pcm_buffer[BUFFER_SIZE];

static HANDLE decoder_handle = NULL;
static DWORD WINAPI __stdcall DecoderThread (void *p);
static volatile int killDecoderThread = 0;

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

static data_buf remain_data; // for transcoding (buffer for data that is decoded but is not copied)

static void tta_error(int error, const char *filename)
{
	char message[1024];
	std::string tempname(filename);
	std::string name;

	if (!tempname.empty()) {
		name = tempname.substr(tempname.find_last_of('\\' + 1));
	} else {
		name = "";
	}
//	if (filename) {
//		char *p = filename + lstrlen(filename);
//		while (*p != '\\' && p >= filename) p--;
//		if (*p == '\\') name = ++p; else name = filename;
//	}

	switch (error) {
		case TTA_OPEN_ERROR:	
			wsprintf(message, "Can't open file:\n%s", name.c_str());
			break;
		case TTA_FORMAT_ERROR:
			wsprintf(message, "Unknown TTA format version:\n%s", name.c_str());
			break;
		case TTA_NOT_SUPPORTED:
			wsprintf(message, "Not supported file format:\n%s", name.c_str());
			break;
		case TTA_FILE_ERROR:
			wsprintf(message, "File is corrupted:\n%s", name.c_str());
			break;
		case TTA_READ_ERROR:
			wsprintf(message, "Can't read from file:\n%s", name.c_str());
			break;
		case TTA_WRITE_ERROR:
			wsprintf(message, "Can't write to file:\n%s", name.c_str());
			break;
		case TTA_MEMORY_ERROR:	
			wsprintf(message, "Insufficient memory available");
			break;
		case TTA_SEEK_ERROR:
			wsprintf(message, "file seek error");
			break;
		case TTA_PASSWORD_ERROR:
			wsprintf(message, "password protected file");
			break;
		default:
			wsprintf(message, "Unknown TTA decoder error");
			break;
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
			"Winamp plug-in version " PLUGIN_VERSION "\n" 
			LIBTTA_VERSION "\n" PROJECT_URL);
		SetDlgItemText(dialog, IDC_PLUGIN_CREADIT,
			ORIGINAL_CREADIT01 ORIGINAL_CREADIT02 ORIGINAL_CREADIT03
			CREADIT01 CREADIT02);
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
	DialogBox(mod.hDllInstance, MAKEINTRESOURCE(IDD_CONFIG),
		hwndParent, config_dialog);
}

void about(HWND hwndParent)
{
	DialogBox(mod.hDllInstance, MAKEINTRESOURCE(IDD_ABOUT),
		hwndParent, about_dialog);
}


void init()
{
	Wasabi_Init();
}

void quit()
{
	Wasabi_Quit();
}


void getfileinfo(const char *file, char *title, int *length_in_ms)
{
	if (!file || !*file) { 
		// invalid filename
	} else {
		SetPlayingTitle(file, title);
		TagLib::FileName fn(file);
		TagLib::TrueAudio::File f(fn);
		if (f.isValid() == true) {
			*length_in_ms = f.audioProperties()->length() * 1000;
		} else {
			// cannot get fileinfo
		}
	}
}

int infodlg(const char *filename, HWND parent)
{
	return 0;
}

int isourfile(const char *filename)
{
	return 0;
} 

int play(const char *fn)
{
	int maxlatency;
	unsigned long decoder_thread_id;
	int return_number;

	return_number = playing_ttafile.SetFileName(fn);
	if(return_number) {
		return return_number;
	} else {
		// do nothing
	}

	maxlatency = mod.outMod->Open(playing_ttafile.GetSampleRate(), 
		playing_ttafile.GetNumberofChannel(), playing_ttafile.GetOutputBPS(), -1, -1);
	if (maxlatency < 0) {
		return 1;
	} else {
		//do nothing
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
	playing_ttafile.SetPaused(1);
	mod.outMod->Pause(1);
}

void unpause() 
{
	playing_ttafile.SetPaused(0);
	mod.outMod->Pause(0);
}

int ispaused()
{
	return playing_ttafile.GetPaused(); 
}

void stop()
{
	if (INVALID_HANDLE_VALUE != decoder_handle) {
		killDecoderThread = 1;
		if (WaitForSingleObject(decoder_handle, INFINITE) == WAIT_TIMEOUT) {
			TerminateThread(decoder_handle, 0);
		}
		CloseHandle(decoder_handle);
		decoder_handle = INVALID_HANDLE_VALUE;
	}

	mod.SetInfo(0, 0, 0, 1);
	mod.outMod->Close();
	mod.SAVSADeInit();

	if (INVALID_HANDLE_VALUE != decoder_handle)
		CloseHandle(decoder_handle);
}

int getlength()
{
	return playing_ttafile.GetLengthbymsec();
}

int getoutputtime()
{
	return (int)(playing_ttafile.GetDecodePosMs() 
		+ (mod.outMod->GetOutputTime() - mod.outMod->GetWrittenTime())); 
}

void setoutputtime(int time_in_ms)
{
	playing_ttafile.SetSeekNeeded(time_in_ms); 
}

void setvolume(int volume)
{
	mod.outMod->SetVolume(volume);
}

void setpan(int pan)
{
	mod.outMod->SetPan(pan);
}
void eq_set(int on, char data[10], int preamp)
{
	// do nothing.
}


static void do_vis(unsigned char *data, int count, int bps, long double position)
{

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
	int decoder_err;

	while (!killDecoderThread) {
		if (playing_ttafile.GetSeekNeeded() != -1) {
			mod.outMod->Flush((int)playing_ttafile.SeekPosition(&done));
		} else {
			// do nothing
		}

		if (done) {
			mod.outMod->CanWrite();
			if (!mod.outMod->IsPlaying()) {
				PostMessage(mod.hMainWindow, WM_WA_MPEG_EOF, 0, 0);
				return 0;
			} else {
				Sleep(10);
			}
		} else if (mod.outMod->CanWrite() >= 
			((BUFFER_LENGTH * playing_ttafile.GetNumberofChannel() * 
			playing_ttafile.GetByteSize()) << (mod.dsp_isactive()? 1:0))) {
				decoder_err = playing_ttafile.GetSamples(&len, pcm_buffer, BUFFER_SIZE, &bitrate);
			if (decoder_err != TTA_NO_ERROR) {
				tta_error(decoder_err, playing_ttafile.GetFileName());
				done = 1;
			} else if (len == 0) {
				done = 1;
			} else {
 				do_vis(pcm_buffer, len, playing_ttafile.GetOutputBPS(), playing_ttafile.GetDecodePosMs());
				if (mod.dsp_isactive()) {
					len = mod.dsp_dosamples((short *)pcm_buffer, len, playing_ttafile.GetOutputBPS(),
						playing_ttafile.GetNumberofChannel(), playing_ttafile.GetSampleRate());
				} else {
					// do nothing
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
			if (File.isValid() == false) {
				// cannot get file info
			} else if (!(File.tag()->artist().isEmpty()) || !(File.tag()->title().isEmpty()) || !(File.tag()->album().isEmpty())) {
				if(!(File.tag()->artist().isEmpty()) || !(File.tag()->title().isEmpty())) {
					wsprintf(title, "%s - %s", 
						File.tag()->artist(),
						File.tag()->title());
				} else if (!(File.tag()->artist().isEmpty()) || !(File.tag()->album().isEmpty())) {
					wsprintf(title, "%s - %s", 
						File.tag()->artist(),
						File.tag()->album());
				} else if (!(File.tag()->artist().isEmpty())) {
					wsprintf(title, "%s", 
						File.tag()->artist());
				} else if (!(File.tag()->title().isEmpty())) {
					wsprintf(title, "%s", 
						File.tag()->title());
				}
			} else {
				char p[MAX_PATHLEN];
				::GetFileTitle(filename, p, MAX_PATHLEN - 1);
				lstrcpyn(title, p, strchr(p, '.') - p);
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
		return &mod;
	}

	__declspec(dllexport) int __cdecl
	winampGetExtendedFileInfo(const char *fn, const char *data, char *dest, size_t destlen)
	{
		return m_Tag.GetExtendedFileInfo(fn, data, dest, destlen);
	}

	__declspec(dllexport) int __cdecl winampUseUnifiedFileInfoDlg(const char * fn)
	{
  // this will be called when Winamp is requested to show a File Info dialog for the selected file(s)
  // and this will allow you to override or forceable ignore the handling of a file or format
  // e.g. this will allow streams/urls to be ignored
	if (!_strnicmp(fn, "file://", 7)) {
		fn += 7;
	}
	if (PathIsURLA(fn)) { 
		return 0;
	}
	return 1;
	}


	__declspec( dllexport ) int __cdecl winampSetExtendedFileInfo(const char *fn, const char *data, const char *val)
	{
		return m_Tag.SetExtendedFileInfo(fn, data, val);
	}

	__declspec(dllexport) int __cdecl winampWriteExtendedFileInfo()
	{
		return m_Tag.WriteExtendedFileInfo();
	}

	__declspec(dllexport) intptr_t __cdecl winampGetExtendedRead_open(const char *filename, int *size, int *bps, int *nch, int *srate)
	{
		
		CDecodeFile *dec = new CDecodeFile;
		dec->SetFileName(filename);
	
		*bps = dec->GetBitsperSample();
		*nch = dec->GetNumberofChannel();
		*srate = dec->GetSampleRate();
		*size = dec->GetDataLength() * (*bps / 8) * (*nch);
		remain_data.data_length = 0;
		remain_data.buffer = NULL;
	
		return (intptr_t)dec;
	}

	__declspec( dllexport ) intptr_t __cdecl winampGetExtendedRead_getData(intptr_t handle, char *dest, int len, int *killswitch)
	{
		CDecodeFile *dec = (CDecodeFile *)handle;
		unsigned char *buf = new unsigned char[BUFFER_SIZE];
		int dest_used = 0;
		int n = 0;
		int bitrate;
		int32_t decoded_bytes = 0;
		int decoder_err;

		// restore remain (not copied) data
		if (remain_data.data_length != 0) {
			while (min(len - dest_used, remain_data.data_length) > 0 && !*killswitch) {
				n = min(len - dest_used, remain_data.data_length);
				memcpy_s(dest + dest_used, len - dest_used, remain_data.buffer + dest_used, n);
				dest_used += n;
				remain_data.data_length -= n;
			}

			if(remain_data.data_length != 0) {
				delete [] buf;
				delete [] remain_data.buffer;
				remain_data.buffer = NULL;
				return (intptr_t)dest_used;
			} else {
				delete [] remain_data.buffer;
				remain_data.buffer = NULL;
			}
		}

		while (dest_used < len && !*killswitch) {
		// do we need to decode more?
			if (n >= decoded_bytes) {
				 decoder_err = dec->GetSamples(&decoded_bytes, buf, BUFFER_SIZE, &bitrate);
				 if (TTA_NO_ERROR != decoder_err) {
					 tta_error(decoder_err, dec->GetFileName());
					 dest_used = -1;
					 break;
				 } else if (0 == decoded_bytes) {
					 break; // end of stream
				 } else {
					 decoded_bytes = decoded_bytes * dec->GetBitsperSample() / 8 * dec->GetNumberofChannel();
					 n = min(len - dest_used, decoded_bytes);
					 if (n > 0) {
						 memcpy_s(dest + dest_used, len - dest_used, buf, n);
						 dest_used += n;
					 } else {
						 // do nothing
					 }
				 }
			} else {
				// do nothing
			}
		}
      	
		// copy as much as we can back to winamp
		if (n > 0 && n < decoded_bytes)	{
			remain_data.data_length = decoded_bytes - n;
			remain_data.buffer = new BYTE[remain_data.data_length];
			memcpy_s(remain_data.buffer, remain_data.data_length, buf + n, remain_data.data_length);
		}

		delete [] buf;
		return (intptr_t)dest_used;
	}

	// return nonzero on success, zero on failure
	__declspec( dllexport ) int __cdecl winampGetExtendedRead_setTime(intptr_t handle, int millisecs)
	{
		int done = 0;
		CDecodeFile *dec = (CDecodeFile *)handle;
		dec->SetDecodePosMs(millisecs);
		dec->SeekPosition(&done);
		return 1;
	}

	__declspec( dllexport ) void __cdecl winampGetExtendedRead_close(intptr_t handle)
	{
		if (remain_data.buffer != NULL) {
			delete [] remain_data.buffer;
			remain_data.buffer = NULL;
		} else {
			// nothing to do
		}
		CDecodeFile *dec = (CDecodeFile *)handle;
		delete dec;
	}
}
