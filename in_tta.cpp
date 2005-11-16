/*
 * in_tta.cpp
 *
 * Description:	 TTA input plug-in for upper Winamp 2.91
 *               MediaLibrary Extension version
 * Developed by: Alexander Djourik <ald@true-audio.com>
 *               Pavel Zhilin <pzh@true-audio.com>
 *               (MediaLibrary Extension Fumihiro YAMAGATA <f_Yamagata@hotmail.com> )
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


#include "stdafx.h"
#include "in_tta.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


BEGIN_MESSAGE_MAP(CIn_ttaApp, CWinApp)
	//{{AFX_MSG_MAP(CIn_ttaApp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CIn_ttaApp �̍\�z

CIn_ttaApp::CIn_ttaApp()
{
}

/////////////////////////////////////////////////////////////////////////////
// �B��� CIn_ttaApp �I�u�W�F�N�g

CIn_ttaApp theApp;

//#include <windows.h>
#include <mmreg.h>
//#include <time.h>

#include "in2.h"
#include "id3genre.h"
#include "resource.h"
#include "id3tag.h"
#include "crc32.h"
#include "ttadec.h"
#include "wa_ipc.h"
#include "TagInfo.h"
#include "TtaTag.h"
//#include "ID3v1.h"


#define  PLUGIN_VERSION "3.2 (Media Library Extension)"
#define  PROJECT_URL "<http://www.sourceforge.net>"

static int paused = 0;
static unsigned int seek_needed = -1;
static unsigned int decode_pos_ms = 0;

static BYTE isobuffer[ISO_BUFFER_SIZE + 4];
static BYTE pcm_buffer[BUFFER_SIZE];	// PCM buffer
static decoder tta[2*MAX_NCH];			// decoder state
static long	cache[MAX_NCH];				// decoder cache
static long vis_buffer[BUFFER_SIZE*MAX_NCH];	// vis buffer

//static tta_info info;			// currently playing file info
//static tta_info dlgInfo;
CTagInfo m_Tag;
CTtaTag  ttaTag;
CTtaTag  dlgTag;

unsigned long fframes;			// number of frames in file
unsigned long framelen;			// the frame length in samples
unsigned long lastlen;			// the length of the last frame in samples
unsigned long data_pos;			// currently playing frame index
unsigned long data_cur;			// the playing position in frame
unsigned long data_float;		// data type flag
long maxvalue;			// output data max value
unsigned long out_bps;			// output bps value

unsigned long *seek_table;		// the playing position table
unsigned long st_state;			//seek table status

unsigned long frame_crc32;
unsigned long bit_count;
unsigned long bit_cache;
unsigned char *bitpos;

static HANDLE decoder_handle = NULL;
static DWORD WINAPI DecoderThread (void *p);
static volatile int killDecoderThread = 0;

void config(HWND parent);
void init();
void about(HWND parent);
void quit () {}
void getfileinfo(char *filename, char *title, int *length_in_ms);
int infodlg(char *filename, HWND parent);
void pause();
void unpause();
int ispaused() { return paused; }
int isourfile(char *filename) { return 0; } 
int play(char *filename);
void stop();
int  getlength();
int  getoutputtime();
void setoutputtime(int time_in_ms);
void setvolume(int volume);
void setpan(int pan);
void eq_set(int on, char data[10], int preamp);

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

HANDLE heap;

int player_init();
int set_position(unsigned long pos);
int get_samples(BYTE *buffer, long count);
void pause() { paused = 1; mod.outMod->Pause(1); }
void unpause() { paused = 0; mod.outMod->Pause(0); }

void init () {
	heap = GetProcessHeap();
	ttaTag.Flush();
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

void about (HWND parent) {
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

void config (HWND parent) {
	DialogBox(mod.hDllInstance, MAKEINTRESOURCE(IDD_CONFIG),
		parent, config_dialog);
}

static int fill_id3_data (HWND dialog, int id3version) {
	int indx;

	// set text limits
	if (id3version == 1) {
		bool state = dlgTag.id3v1.hasTag();

		SetDlgItemText(dialog, IDC_ID3_EDITOR, "ID3v1");
		EnableWindow(GetDlgItem(dialog, IDC_ID3_EDITOR), state);
		EnableWindow(GetDlgItem(dialog, IDC_ID3_DELETE), state);

		if (!state && dlgTag.id3v2.hasTag()) {
			SendDlgItemMessage(dialog, IDC_ID3_TITLE,	EM_SETSEL, 30, -1);
			SendDlgItemMessage(dialog, IDC_ID3_TITLE,	EM_REPLACESEL, FALSE, (LPARAM)"");
			SendDlgItemMessage(dialog, IDC_ID3_TITLE,	EM_SETSEL, 0, 0);
			SendDlgItemMessage(dialog, IDC_ID3_ARTIST,	EM_SETSEL, 30, -1);
			SendDlgItemMessage(dialog, IDC_ID3_ARTIST,	EM_REPLACESEL, FALSE, (LPARAM)"");
			SendDlgItemMessage(dialog, IDC_ID3_ARTIST,	EM_SETSEL, 0, 0);
			SendDlgItemMessage(dialog, IDC_ID3_ALBUM,	EM_SETSEL, 30, -1);
			SendDlgItemMessage(dialog, IDC_ID3_ALBUM,	EM_REPLACESEL, FALSE, (LPARAM)"");
			SendDlgItemMessage(dialog, IDC_ID3_ALBUM,	EM_SETSEL, 0, 0);
			SendDlgItemMessage(dialog, IDC_ID3_COMMENT,	EM_SETSEL, 28, -1);
			SendDlgItemMessage(dialog, IDC_ID3_COMMENT,	EM_REPLACESEL, FALSE, (LPARAM)"");
			SendDlgItemMessage(dialog, IDC_ID3_COMMENT,	EM_SETSEL, 0, 0);
		}

		SendDlgItemMessage(dialog, IDC_ID3_TITLE,	EM_SETLIMITTEXT, 30, 0);
		SendDlgItemMessage(dialog, IDC_ID3_ARTIST,	EM_SETLIMITTEXT, 30, 0);
		SendDlgItemMessage(dialog, IDC_ID3_ALBUM,	EM_SETLIMITTEXT, 30, 0);
		SendDlgItemMessage(dialog, IDC_ID3_YEAR,	EM_SETLIMITTEXT,  4, 0);
		SendDlgItemMessage(dialog, IDC_ID3_TRACK,	EM_SETLIMITTEXT,  3, 0);
		SendDlgItemMessage(dialog, IDC_ID3_COMMENT,	EM_SETLIMITTEXT, 28, 0);
	} else {
		bool state = dlgTag.id3v2.hasTag();

		SetDlgItemText(dialog, IDC_ID3_EDITOR, "ID3v2");
		EnableWindow(GetDlgItem(dialog, IDC_ID3_EDITOR), state);
		EnableWindow(GetDlgItem(dialog, IDC_ID3_DELETE), state);
		SendDlgItemMessage(dialog, IDC_ID3_SWITCH,	BM_SETCHECK, BST_CHECKED, 0);

		SendDlgItemMessage(dialog, IDC_ID3_TITLE,	EM_SETLIMITTEXT, MAX_LINE, 0);
		SendDlgItemMessage(dialog, IDC_ID3_ARTIST,	EM_SETLIMITTEXT, MAX_LINE, 0);
		SendDlgItemMessage(dialog, IDC_ID3_ALBUM,	EM_SETLIMITTEXT, MAX_LINE, 0);
		SendDlgItemMessage(dialog, IDC_ID3_YEAR,	EM_SETLIMITTEXT,  4, 0);
		SendDlgItemMessage(dialog, IDC_ID3_TRACK,	EM_SETLIMITTEXT,  3, 0);
		SendDlgItemMessage(dialog, IDC_ID3_COMMENT,	EM_SETLIMITTEXT, MAX_LINE, 0);
	}

	if (id3version == 1 && dlgTag.id3v1.hasTag()) {
		SetDlgItemText(dialog, IDC_ID3_TITLE, dlgTag.id3v1.GetTitle());
		SetDlgItemText(dialog, IDC_ID3_ARTIST, dlgTag.id3v1.GetArtist());
		SetDlgItemText(dialog, IDC_ID3_ALBUM, dlgTag.id3v1.GetAlbum());
		SetDlgItemText(dialog, IDC_ID3_YEAR, dlgTag.id3v1.GetYear());
		SetDlgItemText(dialog, IDC_ID3_COMMENT, dlgTag.id3v1.GetComment());
		if (dlgTag.id3v1.GetTrack() > 0)
			SetDlgItemInt(dialog, IDC_ID3_TRACK, dlgTag.id3v1.GetTrack(), FALSE);
		else  {
			SendDlgItemMessage(dialog, IDC_ID3_TRACK,	EM_SETSEL, 0, -1);
			SendDlgItemMessage(dialog, IDC_ID3_TRACK,	EM_REPLACESEL, FALSE, (LPARAM)"");
		}
		if ((unsigned char)dlgTag.id3v1.GetGenre() >= 0 && dlgTag.id3v1.GetGenre() != 0xFF) {
			if ((unsigned int) dlgTag.id3v1.GetGenre() > GENRES - 1) dlgTag.id3v1.SetGenre(12); // Other
			indx = SendDlgItemMessage(dialog, IDC_ID3_GENRE, CB_FINDSTRINGEXACT,
				-1, (LPARAM) genre[(unsigned char)dlgTag.id3v1.GetGenre()]);
			SendDlgItemMessage(dialog, IDC_ID3_GENRE, CB_SETCURSEL, indx, 0);
		} else SendDlgItemMessage(dialog, IDC_ID3_GENRE, CB_SETCURSEL, -1, 0);
		return 0;
	} else if (id3version == 2 && dlgTag.id3v2.hasTag()) {
		SetDlgItemText(dialog, IDC_ID3_TITLE, dlgTag.id3v2.GetTitle());
		SetDlgItemText(dialog, IDC_ID3_ARTIST, dlgTag.id3v2.GetArtist());
		SetDlgItemText(dialog, IDC_ID3_ALBUM, dlgTag.id3v2.GetAlbum());
//		SetDlgItemText(dialog, IDC_ID3_YEAR, dlgInfo.id3v2.year);
//		SetDlgItemText(dialog, IDC_ID3_COMMENT, dlgInfo.id3v2.comment);
//		SetDlgItemText(dialog, IDC_ID3_TRACK, dlgInfo.id3v2.track);
//		if (*dlgInfo.id3v2.genre && dlgInfo.id3v2.genre[0] != 0x20) {
//			for (indx = 0, genre_indx = 12; indx < GENRES; indx++) {
//				if (lstrcmp(dlgInfo.id3v2.genre, genre[indx]) == 0) {
//					genre_indx = indx;
//					break;
//				}
//			}
//			indx = SendDlgItemMessage(dialog, IDC_ID3_GENRE, CB_FINDSTRINGEXACT,
//				-1, (LPARAM) genre[genre_indx]);
//			SendDlgItemMessage(dialog, IDC_ID3_GENRE, CB_SETCURSEL, indx, 0);
//		}
		return 0;
	}

	return 1;
}

static void update_id3_data (HWND dialog, int id3version) {
	char itemtext[64];
	char tempchar[MAX_MUSICTEXT];
	int indx;

	if (id3version == 1) {

		// title, artist, album, year, comment, track
		GetDlgItemText(dialog, IDC_ID3_TITLE, tempchar,	sizeof(tempchar));
		dlgTag.id3v1.SetTitle(tempchar);
		GetDlgItemText(dialog, IDC_ID3_ARTIST, tempchar, sizeof(tempchar));
		dlgTag.id3v1.SetArtist(tempchar);
		GetDlgItemText(dialog, IDC_ID3_ALBUM, tempchar,	sizeof(tempchar));
		dlgTag.id3v1.SetAlbum(tempchar);
		GetDlgItemText(dialog, IDC_ID3_YEAR, tempchar, sizeof(tempchar));
		dlgTag.id3v1.SetYear(tempchar);
		GetDlgItemText(dialog, IDC_ID3_COMMENT, tempchar, sizeof(tempchar));
		dlgTag.id3v1.SetComment(tempchar);
		dlgTag.id3v1.SetTrack(GetDlgItemInt(dialog, IDC_ID3_TRACK, 0, FALSE));

		// genre
		GetDlgItemText(dialog, IDC_ID3_GENRE, itemtext, sizeof(itemtext));
		for (indx = 0, dlgTag.id3v1.SetGenre((char)0xFF); indx < GENRES; indx++) {
			if (!*itemtext && itemtext[0] == 0x20) break;
			if (lstrcmp(itemtext, genre[indx]) == 0) {
				dlgTag.id3v1.SetGenre(indx);
				break;
			}
		}

		dlgTag.id3v1.SaveTag(mod.hMainWindow);
		EnableWindow(GetDlgItem(dialog, IDC_ID3_EDITOR), TRUE);

		fill_id3_data(dialog,1);
		return;
//	} else {
//
//		// title, artist, album, year, comment, track
//		GetDlgItemText(dialog, IDC_ID3_TITLE, dlgInfo.id3v2.title,
//			sizeof(dlgInfo.id3v2.title));
//		GetDlgItemText(dialog, IDC_ID3_ARTIST, dlgInfo.id3v2.artist,
//			sizeof(dlgInfo.id3v2.artist));
//		GetDlgItemText(dialog, IDC_ID3_ALBUM, dlgInfo.id3v2.album,
//			sizeof(dlgInfo.id3v2.album));
//		GetDlgItemText(dialog, IDC_ID3_YEAR, dlgInfo.id3v2.year,
//			sizeof(dlgInfo.id3v2.year));
//		GetDlgItemText(dialog, IDC_ID3_COMMENT, dlgInfo.id3v2.comment,
//			sizeof(dlgInfo.id3v2.comment));
//		GetDlgItemText(dialog, IDC_ID3_TRACK, dlgInfo.id3v2.track,
//			sizeof(dlgInfo.id3v2.track));
//		GetDlgItemText(dialog, IDC_ID3_GENRE, dlgInfo.id3v2.genre,
//			sizeof(dlgInfo.id3v2.genre));
//		if (*dlgInfo.id3v2.genre == 0x20) *dlgInfo.id3v2.genre = 0;
//
//		save_id3v2_tag(&dlgInfo);
//		EnableWindow(GetDlgItem(dialog, IDC_ID3_EDITOR), TRUE);
	}
}

static BOOL CALLBACK info_dialog (HWND dialog, UINT message,
	WPARAM wparam, LPARAM lparam) {
	char itemtext[64];
	int indx;

	switch (message) {
		HICON hicon;

	case WM_INITDIALOG:
		SetWindowText(dialog, (char *) lparam);

		// Set new icon
		hicon = LoadIcon (mod.hDllInstance, MAKEINTRESOURCE(IDI_INFO_ICON));
		SendMessage (dialog, WM_SETICON, (WPARAM)ICON_SMALL, (LPARAM)hicon);

		// initialize genre combobox
		for (indx = 0; indx < GENRES; indx++)
			SendDlgItemMessage(dialog, IDC_ID3_GENRE, CB_ADDSTRING, 0, (LPARAM) genre[indx]);
		indx = SendDlgItemMessage(dialog, IDC_ID3_GENRE, CB_ADDSTRING, 0, (LPARAM) " ");

		// load data
		SetDlgItemText(dialog, IDC_FILE_LOCATION, dlgTag.GetFileName());
		SetDlgItemInt(dialog, IDC_TTA_LEVEL, TTA_LEVEL, FALSE);
		SetDlgItemInt(dialog, IDC_TTA_BPS, dlgTag.GetBitsperSample(), FALSE);
		SetDlgItemInt(dialog, IDC_TTA_SAMPLERATE, dlgTag.GetSampleRate(), FALSE);
		SetDlgItemInt(dialog, IDC_TTA_CHANNELS, dlgTag.GetNumberofChannel(), FALSE);

		SetDlgItemInt(dialog, IDC_TTA_FILESIZE, dlgTag.GetFileSize(), FALSE);
		sprintf(itemtext, "%.2f", dlgTag.GetCompressRate());
		SetDlgItemText(dialog, IDC_TTA_COMPRESSION, itemtext);

		if (dlgTag.id3v2.hasTag()) fill_id3_data(dialog, 2);
		else fill_id3_data(dialog, 1); 

		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wparam)) {
			int id3version;

		case IDC_ID3_SWITCH:
			if (IsDlgButtonChecked(dialog, IDC_ID3_SWITCH) == BST_CHECKED)
				 fill_id3_data(dialog, 2);
			else fill_id3_data(dialog, 1);
			return TRUE;
		case IDC_ID3_DELETE:
			if (IsDlgButtonChecked(dialog, IDC_ID3_SWITCH) == BST_CHECKED)
//				 del_id3v2_tag(&dlgInfo);
				int i;
			else dlgTag.id3v1.DeleteTag(mod.hMainWindow);
			EnableWindow(GetDlgItem(dialog, IDC_ID3_EDITOR), FALSE);
			EnableWindow(GetDlgItem(dialog, IDC_ID3_DELETE), FALSE);
			return TRUE;
		case IDOK:
			id3version = (IsDlgButtonChecked(dialog, IDC_ID3_SWITCH) == BST_CHECKED)? 2:1;
			update_id3_data(dialog, id3version);
			EndDialog(dialog, wparam);
			return TRUE;
		case IDCANCEL:
			EndDialog(dialog, wparam);
			return TRUE;
		}
		break;
	}
	return FALSE;
}

int infodlg (char *filename, HWND parent) {
	char *p, *fn, *caption;

	// check for required data presented
//	if (!filename || !*filename) {
//		if (!ttaTag.GetFileName() || !*ttaTag.GetFileName()) return 1;
//		fn = ttaTag.GetFileName();
//	} else fn = filename;


	if (!dlgTag.ReadTag(parent, filename)) {
		return 1;
	}

//	if (dlgInfo.HFILE != INVALID_HANDLE_VALUE)
//		CloseHandle(dlgInfo.HFILE);

	fn = filename;
	p = fn + lstrlen(fn);
	while (*p != '\\' && p >= fn) p--;
	if (*p == '\\') caption = ++p;
	else caption = fn;
//	::GetFileTitle(filename, caption, MAX_PATHLEN - 1);

	DialogBoxParam(mod.hDllInstance, MAKEINTRESOURCE(IDD_INFO),
		parent, info_dialog, (LPARAM) caption);

	return 0;
}


void stop () {

	if (decoder_handle) {
		killDecoderThread = 1;
		if (WaitForSingleObject(decoder_handle, INFINITE) == WAIT_TIMEOUT) {
			tta_error(THREAD_ERROR, NULL);
			TerminateThread(decoder_handle, 0);
		}
		CloseHandle(decoder_handle);
		decoder_handle = NULL;
	}

//	if (info.HFILE != INVALID_HANDLE_VALUE) {
//		CloseHandle(info.HFILE);
//		info.HFILE = INVALID_HANDLE_VALUE;
//	}

	mod.outMod->Close();
	mod.SAVSADeInit();

	if (seek_table) {
		HeapFree(heap, 0, seek_table);
		seek_table = NULL;
	}
}

void show_bitrate (int bitrate) {
	mod.SetInfo(bitrate, ttaTag.GetSampleRate()/1000, ttaTag.GetNumberofChannel(), 1);
}

int play (char *filename) {
	int maxlatency;
	unsigned long decoder_thread_id;

	// check for required data presented
	if (!filename || !*filename) return -1;

	// open TTA file
	if (!ttaTag.ReadTag(mod.hMainWindow, filename)) {
	//	if (info.STATE != FORMAT_ERROR)
	//		tta_error (info.STATE, filename);
	return 1;
	}

	if (player_init () < 0) {
//		tta_error (info.STATE, filename);
		return 1;
	}

	paused = 0;
	decode_pos_ms = 0;
	seek_needed = -1;
	mod.is_seekable = st_state;

	maxlatency = mod.outMod->Open(ttaTag.GetSampleRate(), ttaTag.GetNumberofChannel(), out_bps, -1, -1);
	if (maxlatency < 0) {
		ttaTag.CloseFile();
		return 1;
	}

	// setup information display
	show_bitrate(ttaTag.GetBitrate());

	// initialize vis stuff
	mod.SAVSAInit(maxlatency, ttaTag.GetSampleRate());
	mod.VSASetInfo(ttaTag.GetNumberofChannel(), ttaTag.GetSampleRate());

	// set the output plug-ins default volume
	mod.outMod->SetVolume(-666);

	killDecoderThread = 0;

	decoder_handle = CreateThread(NULL, 0, DecoderThread, NULL, 0, &decoder_thread_id);
	if (!decoder_handle) { stop(); return 1; }

	return 0;
}

int  getlength () { return ttaTag.GetLengthbymsec(); }
int  getoutputtime () { return decode_pos_ms + (mod.outMod->GetOutputTime() - mod.outMod->GetWrittenTime()); }
void setoutputtime (int time_in_ms) { seek_needed = time_in_ms; }
void setvolume (int volume) { mod.outMod->SetVolume(volume); }
void setpan (int pan) { mod.outMod->SetPan(pan); }
void eq_set (int on, char data[10], int preamp) {}


void getfileinfo (char *filename, char *title, int *length_in_ms) {
	if (!filename || !*filename) { // currently playing file
		*length_in_ms = ttaTag.GetLengthbymsec();
		ttaTag.SetPlayTitle(title);
	} else {
		ttaTag.ReadTag(mod.hMainWindow, filename);
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

	mod.SAAddPCMData(data, ttaTag.GetNumberofChannel(), 16, position);
	mod.VSAAddPCMData(data, ttaTag.GetNumberofChannel(), 16, position);
}

static DWORD WINAPI DecoderThread (void *p) {
	int done = 0;
	int len;

	while (!killDecoderThread) {
		if (seek_needed != -1) {
			if (seek_needed >= ttaTag.GetLengthbymsec()) {
				decode_pos_ms = ttaTag.GetLengthbymsec();
				mod.outMod->Flush(decode_pos_ms);
				done = 1;
			} else {
				data_pos = seek_needed / SEEK_STEP;
				decode_pos_ms = data_pos * SEEK_STEP;
				seek_needed = -1;
				mod.outMod->Flush(decode_pos_ms);
				data_cur = -1;
			}
			set_position (data_pos);
		}
		if (done) {
			mod.outMod->CanWrite();
			if (!mod.outMod->IsPlaying()) {
				PostMessage(mod.hMainWindow, WM_WA_MPEG_EOF, 0, 0);
				return 0;
			} else Sleep(10);
		} else if (mod.outMod->CanWrite() >= 
			((576 * ttaTag.GetNumberofChannel() * ttaTag.GetByteSize()) << (mod.dsp_isactive()? 1:0))) {
			if (!(len = get_samples(pcm_buffer, 576))) done = 1;
			else {
				decode_pos_ms += (len * 1000) / ttaTag.GetSampleRate();
				do_vis(pcm_buffer, len, out_bps, decode_pos_ms);
				if (mod.dsp_isactive())
					len = mod.dsp_dosamples((short *)pcm_buffer, len, out_bps,
						ttaTag.GetNumberofChannel(), ttaTag.GetSampleRate());
				mod.outMod->Write((char *)pcm_buffer, len * ttaTag.GetNumberofChannel() * (out_bps >> 3));
			}
		} else Sleep(20);
	}

	return 0;
}


extern "C"
{
	__declspec(dllexport) In_Module *winampGetInModule2(void)
	{
		return &mod;
	}

	__declspec(dllexport) int
	winampGetExtendedFileInfo(extendedFileInfoStruct ExtendedFileInfo)
	{
		return m_Tag.GetExtendedFileInfo(mod.hMainWindow, &ExtendedFileInfo);
	}
}

//BOOL WINAPI _DllMainCRTStartup (HANDLE hInst, ULONG ul_reason_for_call,
//	LPVOID lpReserved) { return TRUE; }

///////////////////////////////////////////////////////////////////////
// Description:	 TTAv1 lossless audio decoder.                       //
// Copyright (c) 1999-2004 Alexander Djourik. All rights reserved.   //
///////////////////////////////////////////////////////////////////////

///////////////////// constants and definitions ///////////////////////

#define PREDICTOR1(x, k)	((long)((((__int64)x << k) - x) >> k))
#define DEC(x)			(((x)&1)?(++(x)>>1):(-(x)>>1))
#define SHR8(x)			((((x)>0)?((x)+0x80):((x)-0x80)) >> 8); 

#define SWAP16(x) (\
(((x)&(1<< 0))?(1<<15):0) | \
(((x)&(1<< 1))?(1<<14):0) | \
(((x)&(1<< 2))?(1<<13):0) | \
(((x)&(1<< 3))?(1<<12):0) | \
(((x)&(1<< 4))?(1<<11):0) | \
(((x)&(1<< 5))?(1<<10):0) | \
(((x)&(1<< 6))?(1<< 9):0) | \
(((x)&(1<< 7))?(1<< 8):0) | \
(((x)&(1<< 8))?(1<< 7):0) | \
(((x)&(1<< 9))?(1<< 6):0) | \
(((x)&(1<<10))?(1<< 5):0) | \
(((x)&(1<<11))?(1<< 4):0) | \
(((x)&(1<<12))?(1<< 3):0) | \
(((x)&(1<<13))?(1<< 2):0) | \
(((x)&(1<<14))?(1<< 1):0) | \
(((x)&(1<<15))?(1<< 0):0))

const unsigned long bit_mask[] = {
    0x00000000, 0x00000001, 0x00000003, 0x00000007,
    0x0000000f, 0x0000001f, 0x0000003f, 0x0000007f,
    0x000000ff, 0x000001ff, 0x000003ff, 0x000007ff,
    0x00000fff, 0x00001fff, 0x00003fff, 0x00007fff,
    0x0000ffff, 0x0001ffff, 0x0003ffff, 0x0007ffff,
    0x000fffff, 0x001fffff, 0x003fffff, 0x007fffff,
    0x00ffffff, 0x01ffffff, 0x03ffffff, 0x07ffffff,
    0x0fffffff, 0x1fffffff, 0x3fffffff, 0x7fffffff,
    0xffffffff
};

const unsigned long bit_shift[] = {
    0x00000001, 0x00000002, 0x00000004, 0x00000008,
    0x00000010, 0x00000020, 0x00000040, 0x00000080,
    0x00000100, 0x00000200, 0x00000400, 0x00000800,
    0x00001000, 0x00002000, 0x00004000, 0x00008000,
    0x00010000, 0x00020000, 0x00040000, 0x00080000,
    0x00100000, 0x00200000, 0x00400000, 0x00800000,
    0x01000000, 0x02000000, 0x04000000, 0x08000000,
    0x10000000, 0x20000000, 0x40000000, 0x80000000,
    0x80000000, 0x80000000, 0x80000000, 0x80000000,
    0x80000000, 0x80000000, 0x80000000, 0x80000000
};

const unsigned long *shift_16 = bit_shift + 4;

///////////////////////// bit operations //////////////////////////////

void  init_buffer_read() {
    frame_crc32 = 0xFFFFFFFFUL;
    bit_count = bit_cache = 0;
    bitpos = (isobuffer + ISO_BUFFER_SIZE);
}

__inline void get_binary(unsigned long *value, unsigned long bits) {
    while (bit_count < bits) {
		if (bitpos == (isobuffer + ISO_BUFFER_SIZE)) {
			DWORD result;
			if (!ReadFile(ttaTag.GetHFILE(), isobuffer, ISO_BUFFER_SIZE,
				&result, NULL) || !result) {
//				info.STATE = READ_ERROR;
				return;
			}
			bitpos = isobuffer;
		}

		UPDATE_CRC32(*bitpos, frame_crc32);
		bit_cache |= *bitpos << bit_count;
		bit_count += 8;
		bitpos++;
    }

    *value = bit_cache & bit_mask[bits];
    bit_cache >>= bits;
    bit_count -= bits;
    bit_cache &= bit_mask[bit_count];
}

__inline void get_unary(unsigned long *value) {
    *value = 0;

    while (!(bit_cache ^ bit_mask[bit_count])) {
		if (bitpos == (isobuffer + ISO_BUFFER_SIZE)) {
			DWORD result;
			if (!ReadFile(ttaTag.GetHFILE(), isobuffer, ISO_BUFFER_SIZE,
				&result, NULL) || !result) {
//				info.STATE = READ_ERROR;
				return;
			}
			bitpos = isobuffer;
		}

		*value += bit_count;
		bit_cache = *bitpos++;
		UPDATE_CRC32(bit_cache, frame_crc32);
		bit_count = 8;
    }

    while (bit_cache & 1) {
		(*value)++;
		bit_cache >>= 1;
		bit_count--;
    }

    bit_cache >>= 1;
    bit_count--;
}

__inline int done_buffer_read() {
    unsigned long crc32, rbytes;
	DWORD result;

    frame_crc32 ^= 0xFFFFFFFFUL;

    rbytes = (isobuffer + ISO_BUFFER_SIZE) - bitpos;
    if (rbytes < sizeof(long)) {
		*(long *)isobuffer = *(long *)bitpos;
		if (!ReadFile(ttaTag.GetHFILE(), isobuffer + rbytes,
			ISO_BUFFER_SIZE - rbytes, &result, NULL) || !result) {
//			info.STATE = READ_ERROR;
			return 0;
		}
		bitpos = isobuffer;
    }

	crc32 = *(long *)bitpos;
    bitpos += sizeof(long);
    result = (crc32 != frame_crc32);

    bit_cache = bit_count = 0;
    frame_crc32 = 0xFFFFFFFFUL;

    // calculate dynamic bitrate
    if (data_pos < fframes) {
		rbytes = seek_table[data_pos] -
			seek_table[data_pos - 1];
		show_bitrate((rbytes << 3) /
			(long)(1000 * FRAME_TIME));
    }

    return result;
}

//////////////////////// TTA hybrid filter ////////////////////////////

///////// Filter Settings //////////
static long flt_set [4][2] = {
	{10,1}, {9,1}, {10,1}, {12,0}
};

__inline void memshl (long *pA, long *pB) {
	*pA++ = *pB++;
	*pA++ = *pB++;
	*pA++ = *pB++;
	*pA++ = *pB++;
	*pA++ = *pB++;
	*pA++ = *pB++;
	*pA++ = *pB++;
	*pA   = *pB;
}

__inline void hybrid_filter (fltst *fs, long *in) {
	long *pA = fs->dl;
	long *pB = fs->qm;
	long *pM = fs->dx;
	long sum = fs->round;

	if (!fs->error) {
		sum += *pA++ * *pB, pB++;
		sum += *pA++ * *pB, pB++;
		sum += *pA++ * *pB, pB++;
		sum += *pA++ * *pB, pB++;
		sum += *pA++ * *pB, pB++;
		sum += *pA++ * *pB, pB++;
		sum += *pA++ * *pB, pB++;
		sum += *pA++ * *pB, pB++; pM += 8;
	} else if (fs->error < 0) {
		sum += *pA++ * (*pB -= *pM++), pB++;
		sum += *pA++ * (*pB -= *pM++), pB++;
		sum += *pA++ * (*pB -= *pM++), pB++;
		sum += *pA++ * (*pB -= *pM++), pB++;
		sum += *pA++ * (*pB -= *pM++), pB++;
		sum += *pA++ * (*pB -= *pM++), pB++;
		sum += *pA++ * (*pB -= *pM++), pB++;
		sum += *pA++ * (*pB -= *pM++), pB++;
	} else {
		sum += *pA++ * (*pB += *pM++), pB++;
		sum += *pA++ * (*pB += *pM++), pB++;
		sum += *pA++ * (*pB += *pM++), pB++;
		sum += *pA++ * (*pB += *pM++), pB++;
		sum += *pA++ * (*pB += *pM++), pB++;
		sum += *pA++ * (*pB += *pM++), pB++;
		sum += *pA++ * (*pB += *pM++), pB++;
		sum += *pA++ * (*pB += *pM++), pB++;
	}

	*(pM-0) = ((*(pA-1) >> 30) | 1) << 2;
	*(pM-1) = ((*(pA-2) >> 30) | 1) << 1;
	*(pM-2) = ((*(pA-3) >> 30) | 1) << 1;
	*(pM-3) = ((*(pA-4) >> 30) | 1);

	fs->error = *in;
	*in += (sum >> fs->shift);
	*pA = *in;

	if (fs->mutex) {
		*(pA-1) = *(pA-0) - *(pA-1);
		*(pA-2) = *(pA-1) - *(pA-2);
		*(pA-3) = *(pA-2) - *(pA-3);
	}

	memshl (fs->dl, fs->dl + 1);
	memshl (fs->dx, fs->dx + 1);
}

static void filter_init (fltst *fs, long shift, long mode) {
	ZeroMemory (fs, sizeof(fltst));
	fs->shift = shift;
	fs->round = 1 << (shift - 1);
	fs->mutex = mode;
}

//////////////////////// decoder functions ////////////////////////////

static void rice_init(adapt *rice, BYTE k0, BYTE k1) {
    rice->k0 = k0;
    rice->k1 = k1;
    rice->sum0 = shift_16[k0];
    rice->sum1 = shift_16[k1];
}

void decoder_init(decoder *tta, long nch, long byte_size) {
    long *fset = flt_set[byte_size - 1];
    long i;

    for (i = 0; i < (nch << data_float); i++) {
		filter_init(&tta[i].fst, fset[0], fset[1]);
		rice_init(&tta[i].rice, 10, 10);
		tta[i].last = 0;
    }
}

static void seek_table_init (unsigned long *seek_table,
	unsigned long len, unsigned long data_offset) {
	unsigned long *st, frame_len;

	for (st = seek_table; st < (seek_table + len); st++) {
		frame_len = *st; *st = data_offset;
		data_offset += frame_len;
	}
}

int set_position (unsigned long pos) {
	unsigned long seek_pos;

	if (pos >= fframes) return 0;
	if (!st_state) {
//		info.STATE = FILE_ERROR;
		return -1;
	}

	seek_pos = ttaTag.id3v2.TagLength() + seek_table[data_pos = pos];
	SetFilePointer(ttaTag.GetHFILE(), seek_pos, NULL, FILE_BEGIN);

	data_cur = 0;
	framelen = 0;

	// init bit reader
	init_buffer_read();

	return 0;
}

int player_init () {
	unsigned long checksum;
	unsigned long data_offset;
	unsigned long st_size;
	DWORD result;

	framelen = 0;
	data_pos = 0;
	data_cur = 0;

	lastlen = ttaTag.GetDataLength() % ttaTag.GetLengthbyFrame();
	fframes = ttaTag.GetDataLength() / ttaTag.GetLengthbyFrame() + (lastlen ? 1:0);

	st_size = (fframes + 1) * sizeof(long);
	seek_table = (unsigned long *)HeapAlloc(heap, HEAP_ZERO_MEMORY, st_size);
	if (!seek_table) {
//		info.STATE = MEMORY_ERROR;
		return -1;
	}

	// read seek table
	if (!ReadFile(ttaTag.GetHFILE(), seek_table, st_size, &result, NULL) ||
		result != st_size) {
//		info.STATE = READ_ERROR;
		return -1;
	}

	checksum = crc32((unsigned char *) seek_table, fframes * sizeof(long));
	st_state = (checksum == seek_table[fframes]);
	data_offset = sizeof(tta_hdr) + st_size;

	// init seek table
	seek_table_init(seek_table, fframes, data_offset);

	// init bit reader
	init_buffer_read();

	data_float = (ttaTag.GetFormat() == WAVE_FORMAT_IEEE_FLOAT);
	out_bps = (ttaTag.GetBitsperSample() > OUT_BPS)? OUT_BPS : ttaTag.GetBitsperSample();

	if (data_float)
		maxvalue = 1UL << (OUT_BPS - 1);
	else maxvalue = (1UL << ttaTag.GetBitsperSample()) - 1;

	return 0;
}

int get_samples (BYTE *buffer, long count) {
	unsigned long k, depth, unary, binary;
	long buffer_size = count * ttaTag.GetByteSize() * ttaTag.GetNumberofChannel();
	BYTE *p = buffer;
	decoder *dec = tta;
	long *prev = (long *)cache;
	long value;
	int res, flag;

	for (res = flag = 0; p < buffer + buffer_size;) {
		fltst *fst = &dec->fst;
		adapt *rice = &dec->rice;
		long  *last = &dec->last;

		if (data_cur == framelen) {
			if (data_pos == fframes) break;
			if (framelen && done_buffer_read()) {
				if (set_position(data_pos) < 0)
					return -1;
				if (res) break;
			}

			if (data_pos == fframes - 1 && lastlen)
				framelen = lastlen;
			else framelen = ttaTag.GetLengthbyFrame();

			decoder_init(tta, ttaTag.GetNumberofChannel(), ttaTag.GetByteSize());
			data_pos++; data_cur = 0;
		}

		// decode Rice unsigned
		get_unary(&unary);

		switch (unary) {
		case 0: depth = 0; k = rice->k0; break;
		default:
				depth = 1; k = rice->k1;
				unary--;
		}

		if (k) {
			get_binary(&binary, k);
			value = (unary << k) + binary;
		} else value = unary;

		switch (depth) {
		case 1: 
			rice->sum1 += value - (rice->sum1 >> 4);
			if (rice->k1 > 0 && rice->sum1 < shift_16[rice->k1])
				rice->k1--;
			else if (rice->sum1 > shift_16[rice->k1 + 1])
				rice->k1++;
			value += bit_shift[rice->k0];
		default:
			rice->sum0 += value - (rice->sum0 >> 4);
			if (rice->k0 > 0 && rice->sum0 < shift_16[rice->k0])
				rice->k0--;
			else if (rice->sum0 > shift_16[rice->k0 + 1])
			rice->k0++;
		}

		value = DEC(value);

		// decompress stage 1: adaptive hybrid filter
		hybrid_filter(fst, &value);

		// decompress stage 2: fixed order 1 prediction
		switch (ttaTag.GetByteSize()) {
		case 1: value += PREDICTOR1(*last, 4); break;	// bps 8
		case 2: value += PREDICTOR1(*last, 5); break;	// bps 16
		case 3: value += PREDICTOR1(*last, 5); break;	// bps 24
		case 4: value += *last; break;		// bps 32
		} *last = value;

		// check for errors
		if (!data_float && abs(value) > maxvalue) {
			unsigned long tail =
				buffer_size / (ttaTag.GetByteSize() * ttaTag.GetNumberofChannel()) - res;
			ZeroMemory(buffer, buffer_size);
			data_cur += tail; res += tail;
			break;
		}

		// combine TTA_FLOAT data
		if (data_float && flag) {
			unsigned long negative = value & 0x80000000;
			unsigned long data_lo = abs(value) - 1;
			unsigned long data_hi = (*prev || data_lo)? (*prev + 0x3F80):0;
			unsigned long t = (data_hi << 16) | SWAP16(data_lo);
			long exponent = (t & 0x7F800000) >> 23;
			long fraction = (t & 0x007FFFFF), value;
			float fvalue = 0.0;

			if (exponent) {
				exponent -= 0x7F;

				if (fraction)
					fvalue = (fraction | 0x800000) / (float) 0x800000;
				if (negative) fvalue *= -1;

				if (exponent > 0) fvalue *= (1 << exponent); else
				if (exponent < 0) fvalue /= (1 << abs(exponent));
			}

			// check for errors
			if (fvalue > 1.0) {
				unsigned long tail =
					buffer_size / (ttaTag.GetByteSize() * ttaTag.GetNumberofChannel()) - res;
				ZeroMemory(buffer, buffer_size);
				data_cur += tail; res += tail;
				break;
			}

			value = (long)(maxvalue * fvalue);
			if (ttaTag.GetByteSize() == 4) SHR8(value);

			*p++ = (BYTE) value;
			if (ttaTag.GetByteSize() > 1) *p++ = (BYTE)(value >> 8);
			if (ttaTag.GetByteSize() > 2) *p++ = (BYTE)(value >> 16);

			flag = 0;
		} else flag = 1;

		if (dec < tta + (ttaTag.GetNumberofChannel() << data_float) - 1) {
			if (!data_float) *prev++ = value;
			else *prev = value;
			dec++;
		} else {
			if (!data_float) {
				*prev = value;
				if (ttaTag.GetNumberofChannel() > 1) {
					long *r = prev - 1;
					for (*prev += *r/2; r >= cache; r--)
						*r = *(r + 1) - *r;
					for (r = cache; r < prev; r++) {
						*p++ = (ttaTag.GetByteSize() == 1)? (*r + 0x80): *r;
						if (ttaTag.GetByteSize() > 1) *p++ = (BYTE)(*r >> 8);
						if (ttaTag.GetByteSize() > 2) *p++ = (BYTE)(*r >> 16);
					}
				}
				*p++ = (ttaTag.GetByteSize() == 1)? (*prev + 0x80): *prev;
				if (ttaTag.GetByteSize() > 1) *p++ = (BYTE)(*prev >> 8);
				if (ttaTag.GetByteSize() > 2) *p++ = (BYTE)(*prev >> 16);
				prev = cache;
			}
			data_cur++; res++;
			dec = tta;
		}
	}

	return res;
}

/***********************************************************************
 * ID3 tags manipulation routines
 *
 * Provides read/write access to ID3v1 tags v1.1
 * Provides read/write access to ID3v2 tags v2.3.x and above
 * Supported ID3v2 frames: Title, Artist, Album, Track, Year,
 *                         Genre, Comment.
 *
 **********************************************************************/

static void pack_sint28 (unsigned int value, char *ptr) {
	ptr[0] = (value >> 21) & 0x7f;
	ptr[1] = (value >> 14) & 0x7f;
	ptr[2] = (value >>  7) & 0x7f;
	ptr[3] = (value & 0x7f);
}

static unsigned int unpack_sint28 (const char *ptr) {
	unsigned int value = 0;

	if (ptr[0] & 0x80) return 0;

	value =  value       | (ptr[0] & 0x7f);
	value = (value << 7) | (ptr[1] & 0x7f);
	value = (value << 7) | (ptr[2] & 0x7f);
	value = (value << 7) | (ptr[3] & 0x7f);

	return value;
}

static void pack_sint32 (unsigned int value, unsigned char *ptr) {
	ptr[0] = (value >> 24) & 0xff;
	ptr[1] = (value >> 16) & 0xff;
	ptr[2] = (value >>  8) & 0xff;
	ptr[3] = (value & 0xff);
}

static unsigned int unpack_sint32 (const unsigned char *ptr) {
	unsigned int value = 0;

	if (ptr[0] & 0x80) return 0;

	value = (value << 8) | ptr[0];
	value = (value << 8) | ptr[1];
	value = (value << 8) | ptr[2];
	value = (value << 8) | ptr[3];

	return value;
}

static int get_frame_id (const char *id) {
	if (!memcmp(id, "TIT2", 4)) return TIT2;	// Title
	if (!memcmp(id, "TPE1", 4)) return TPE1;	// Artist
	if (!memcmp(id, "TALB", 4)) return TALB;	// Album
	if (!memcmp(id, "TRCK", 4)) return TRCK;	// Track
	if (!memcmp(id, "TYER", 4)) return TYER;	// Year
	if (!memcmp(id, "TCON", 4)) return TCON;	// Genre
	if (!memcmp(id, "COMM", 4)) return COMM;	// Comment
	return 0;
}

//void get_id3v1_tag (tta_info *ttainfo) {
//	id3v1_tag id3v1;
//	unsigned long result;
//
//	SetFilePointer(ttainfo->HFILE, -(int) sizeof(id3v1_tag), NULL, FILE_END);
//	if (ReadFile(ttainfo->HFILE, &id3v1, sizeof(id3v1_tag), &result, NULL) &&
//		result == sizeof(id3v1_tag) && !memcmp(id3v1.id, "TAG", 3)) {
//		CopyMemory(ttainfo->id3v1.title, id3v1.title, 30);
//		CopyMemory(ttainfo->id3v1.artist, id3v1.artist, 30);
//		CopyMemory(ttainfo->id3v1.album, id3v1.album, 30);
//		CopyMemory(ttainfo->id3v1.year, id3v1.year, 4);
//		CopyMemory(ttainfo->id3v1.comment, id3v1.comment, 28);
//		ttainfo->id3v1.track = id3v1.track;
//		ttainfo->id3v1.genre = id3v1.genre;
//		ttainfo->id3v1.id3has = 1;
//	}
//	SetFilePointer(ttainfo->HFILE, 0, NULL, FILE_BEGIN);
//}

static char *unwrap (char *str, int length) {
	char *ptr = str, *e = str + length;
	for (;ptr < e; ptr++) {
		if (*ptr == '\r' && *(ptr + 1) == '\n') {
			MoveMemory(ptr, ptr+1, e - ptr);
			if (*ptr == '\n') *ptr = ' ';
		}
	}
	return str;
}

//static void save_id3v1_tag (tta_info *ttainfo) {
//	HANDLE hFile;
//	id3v1_tag tag;
//	int offset;
//	unsigned long result;
//
//	ZeroMemory(&tag, sizeof(id3v1_tag));
//	CopyMemory(tag.id, "TAG", 3);
//
//	CopyMemory(tag.title, ttainfo->id3v1.title, sizeof(tag.title));
//	CopyMemory(tag.artist, ttainfo->id3v1.artist, sizeof(tag.artist));
//	CopyMemory(tag.album, ttainfo->id3v1.album, sizeof(tag.album));
//	CopyMemory(tag.year, ttainfo->id3v1.year, sizeof(tag.year));
//	CopyMemory(tag.comment, unwrap(ttainfo->id3v1.comment,
//		sizeof(tag.comment)), sizeof(tag.comment));
//	tag.track = ttainfo->id3v1.track;
//	tag.genre = ttainfo->id3v1.genre;
//
//	// update ID3V1 tag
//	hFile = CreateFile(ttainfo->filename, GENERIC_READ|GENERIC_WRITE,
//		FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
//	if (hFile == INVALID_HANDLE_VALUE) {
//		tta_error(OPEN_ERROR, ttainfo->filename);
//		return;
//	}
//
//	offset = (ttainfo->id3v1.id3has)? -(int) sizeof(id3v1_tag):0;
//	SetFilePointer(hFile, offset, NULL, FILE_END);
//	if (!WriteFile(hFile, &tag, sizeof(id3v1_tag), &result, 0) ||
//		result != sizeof(id3v1_tag)) {
//		CloseHandle(hFile);
//		tta_error(WRITE_ERROR, ttainfo->filename);
//		return;
//	}
//	CloseHandle(hFile);
//
//	ttainfo->id3v1.id3has = 1;
//}

//static void del_id3v1_tag (tta_info *ttainfo) {
//	HANDLE hFile;
//
//	if (!ttainfo->id3v1.id3has) return;
//
//	// delete ID3V1 tag
//	hFile = CreateFile(ttainfo->filename, GENERIC_READ|GENERIC_WRITE,
//		FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
//	if (hFile == INVALID_HANDLE_VALUE) {
//		tta_error(OPEN_ERROR, ttainfo->filename);
//		return;
//	}
//
//	SetFilePointer(hFile, -(int) sizeof(id3v1_tag), NULL, FILE_END);
//	SetEndOfFile(hFile);
//	CloseHandle(hFile);
//
//	ttainfo->id3v1.id3has = 0;
//}

void get_id3v2_tag (tta_info *ttainfo) {
	HANDLE hMap;
	id3v2_tag id3v2;
	id3v2_frame frame_header;
	unsigned char *buffer, *ptr;
	unsigned long result;
	int id3v2_size;

	if (!ReadFile(ttainfo->HFILE, &id3v2, sizeof(id3v2_tag), &result, NULL) ||
		result != sizeof(id3v2_tag) || memcmp(id3v2.id, "ID3", 3)) {
		SetFilePointer(ttainfo->HFILE, 0, NULL, FILE_BEGIN);
		return;
	}

	id3v2_size = unpack_sint28(id3v2.size) + 10;

	if ((id3v2.flags & ID3_UNSYNCHRONISATION_FLAG) ||
		(id3v2.flags & ID3_EXPERIMENTALTAG_FLAG) ||
		(id3v2.version < 3)) goto done;

	hMap = CreateFileMapping(ttainfo->HFILE, NULL, PAGE_READONLY, 0, id3v2_size, NULL);
	if (!hMap) goto done;

	buffer = (unsigned char *)MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, id3v2_size);
	if (!buffer) {
		CloseHandle(hMap);
		goto done;
	}

	ptr = buffer + 10;

	// skip extended header if present
	if (id3v2.flags & ID3_EXTENDEDHEADER_FLAG) {
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

static void add_text_frame (char *id, unsigned char **dest, char *src) {
	id3v2_frame frame_header;

	if (*src) {
		int size = lstrlen(src) + 1;
		CopyMemory(frame_header.id, id, 4);
		frame_header.flags = 0;
		pack_sint32(size, frame_header.size);
		CopyMemory(*dest, &frame_header, sizeof(id3v2_frame));
		*dest += sizeof(id3v2_frame);
		CopyMemory(*dest + 1, src, size); *dest += size;
	}
}

static void add_comm_frame (char *id, unsigned char **dest, char *src) {
	id3v2_frame frame_header;

	if (*src) {
		int size = lstrlen(src) + 1;
		CopyMemory(frame_header.id, id, 4);
		frame_header.flags = 0;
		pack_sint32(size + 4, frame_header.size);
		CopyMemory(*dest, &frame_header, sizeof(id3v2_frame));
		// skip language, add zero short description
		*dest += sizeof(id3v2_frame) + 4;
		CopyMemory(*dest + 1, src, size); *dest += size;
	}
}

//static void save_id3v2_tag (tta_info *ttainfo) {
//	HANDLE hFile, hMap;
//	id3v2_tag id3v2;
//	unsigned char *buffer, *ptr;
//	unsigned char *tag_data, *tptr;
//	DWORD new_size, id3v2_size;
//	int indx, offset;
//	DWORD result;
//	BOOL copy_data = TRUE;
//	BOOL safe_mode = FALSE;
//
//	if (!memcmp(ttainfo->filename, info.filename,
//		lstrlen(ttainfo->filename))) safe_mode = TRUE;
//
//	hFile = CreateFile(ttainfo->filename, GENERIC_READ|GENERIC_WRITE,
//		FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
//	if (hFile == INVALID_HANDLE_VALUE) {
//		tta_error(OPEN_ERROR, ttainfo->filename);
//		return;
//	}
//
//	if (!ReadFile(hFile, &id3v2, sizeof(id3v2_tag), &result, NULL) ||
//		result != (DWORD) sizeof(id3v2_tag)) {
//		tta_error(READ_ERROR, ttainfo->filename);
//		CloseHandle(hFile);
//		return;
//	}
//
//	if (!memcmp(id3v2.id, "ID3", 3)) {
//		id3v2_size = unpack_sint28(id3v2.size) + 10;
//		if (id3v2.flags & ID3_FOOTERPRESENT_FLAG) id3v2_size += 10;
//	} else {
//		ZeroMemory(&id3v2, sizeof(id3v2_tag));
//		CopyMemory(id3v2.id, "ID3", 3);
//		id3v2_size = 0;
//	}
//
//	tag_data = (unsigned char *)HeapAlloc(heap, HEAP_ZERO_MEMORY,
//		id3v2_size + sizeof(id3v2_data));
//	tptr = tag_data + 10;
//
//	if (!(id3v2.flags & ID3_UNSYNCHRONISATION_FLAG) &&
//		!(id3v2.flags & ID3_EXPERIMENTALTAG_FLAG) &&
//		(id3v2.version >= ID3_VERSION) && id3v2_size) {
//
//		hMap = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, id3v2_size, NULL);
//		if (!hMap) goto done;
//
//		buffer = (unsigned char *)MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, id3v2_size);
//		if (!buffer) {
//			CloseHandle(hMap);
//			goto done;
//		}
//
//		ptr = buffer + 10;
//
//		// copy extended header if present
//		if ((id3v2.flags & ID3_EXTENDEDHEADER_FLAG)) {
//			int ext_size = (int) unpack_sint32(ptr);
//			CopyMemory(tptr, ptr, ext_size);
//			ptr += ext_size; tptr += ext_size;
//		}
//	} else copy_data = FALSE;
//
//	// add updated id3v2 frames
//	add_text_frame("TIT2", &tptr, ttainfo->id3v2.title);
//	add_text_frame("TPE1", &tptr, ttainfo->id3v2.artist);
//	add_text_frame("TALB", &tptr, ttainfo->id3v2.album);
//	add_text_frame("TRCK", &tptr, ttainfo->id3v2.track);
//	add_text_frame("TYER", &tptr, ttainfo->id3v2.year);
//	add_text_frame("TCON", &tptr, ttainfo->id3v2.genre);
//	add_comm_frame("COMM", &tptr, ttainfo->id3v2.comment);
//
//	if (!copy_data) goto save;
//
//	// copy unchanged frames
//	while ((unsigned long)abs(ptr - buffer) < id3v2_size) {
//		int data_size, frame_size;
//		int frame_id, comments = 0;
//		id3v2_frame frame_header;
//
//		// get frame header
//		CopyMemory(&frame_header, ptr, sizeof(id3v2_frame));
//		data_size = unpack_sint32(frame_header.size);
//		frame_size = sizeof(id3v2_frame) + data_size;
//
//		if (!*frame_header.id) break;
//
//		if ((frame_id = get_frame_id(frame_header.id)))
//			if (frame_id != COMM || !comments++) {
//				ptr += frame_size; continue;
//			}
//
//		// copy frame
//		CopyMemory(tptr, ptr, frame_size);
//		tptr += frame_size; ptr += frame_size;
//	}
//
//	// copy footer if present
//	if (id3v2.flags & ID3_FOOTERPRESENT_FLAG) {
//		CopyMemory(tptr, ptr, 10);
//		tptr += 10; ptr += 10;
//	}
//
//save:
//	if (copy_data) {
//		UnmapViewOfFile((LPCVOID *) buffer);
//		CloseHandle(hMap);
//	}
//
//	new_size = tptr - tag_data;
//
//	// fill ID3v2 header
//	id3v2.flags &= ~ID3_UNSYNCHRONISATION_FLAG;
//	id3v2.flags &= ~ID3_EXPERIMENTALTAG_FLAG;
//	id3v2.version = ID3_VERSION;
//
//	// write data
//	if (new_size <= id3v2_size) {
//		pack_sint28(id3v2_size - 10, id3v2.size);
//		CopyMemory(tag_data, &id3v2, sizeof(id3v2_tag));
//
//		SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
//		if (!WriteFile(hFile, tag_data, id3v2_size, &result, 0) ||
//			result != id3v2_size) {
//			CloseHandle(hFile);
//			tta_error(WRITE_ERROR, ttainfo->filename);
//			return;
//		}
//		goto done;
//	}
//
//	pack_sint28(new_size - 10, id3v2.size);
//	CopyMemory(tag_data, &id3v2, sizeof(id3v2_tag));
//	offset = (int) new_size - id3v2_size;
//
//	hMap = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0,
//		ttainfo->FILESIZE + offset, NULL);
//	if (!hMap) goto done;
//
//	buffer = (unsigned char *)MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0,
//		ttainfo->FILESIZE + offset);
//	if (!buffer) {
//		CloseHandle(hMap);
//		goto done;
//	}
//
//	if (safe_mode) pause();
//
//	MoveMemory(buffer + ((int)id3v2_size + offset),
//		buffer + id3v2_size, ttainfo->FILESIZE);
//	CopyMemory(buffer, tag_data, new_size);
//
//	if (safe_mode) FlushViewOfFile((LPCVOID *) buffer, 0);
//	UnmapViewOfFile((LPCVOID *) buffer);
//	CloseHandle(hMap);
//
//	ttainfo->FILESIZE += offset;
//	ttainfo->id3v2.size = new_size;
//
//	if (safe_mode) {
//		info.FILESIZE = ttainfo->FILESIZE;
//		info.id3v2.size = ttainfo->id3v2.size;
//		seek_needed = decode_pos_ms;
//		unpause();
//	}
//
//done:
//	CloseHandle(hFile);
//	HeapFree(heap, 0, tag_data);
//
//	ttainfo->id3v2.id3has = 1;
//}

//static void del_id3v2_tag (tta_info *ttainfo) {
//	HANDLE hFile, hMap;
//	unsigned char *buffer;
//	int indx, result;
//	BOOL safe_mode = FALSE;
//
//	if (!ttainfo->id3v2.id3has) return;
//
//	if (!memcmp(ttainfo->filename, info.filename,
//		lstrlen(ttainfo->filename))) safe_mode = TRUE;
//
//	hFile = CreateFile(ttainfo->filename, GENERIC_READ|GENERIC_WRITE,
//		FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
//	if (hFile == INVALID_HANDLE_VALUE) {
//		tta_error(OPEN_ERROR, ttainfo->filename);
//		return;
//	}
//
//	hMap = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0, 0, NULL);
//	if (!hMap) {
//		CloseHandle(hFile);
//		CloseHandle(hMap);
//		return;
//	}
//
//	buffer = (unsigned char *)MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
//	if (!buffer) {
//		CloseHandle(hFile);
//		CloseHandle(hMap);
//		return;
//	}
//
//	if (safe_mode) pause();
//
//	MoveMemory(buffer, buffer + ttainfo->id3v2.size,
//		ttainfo->FILESIZE - ttainfo->id3v2.size);
//
//	if (safe_mode) FlushViewOfFile((LPCVOID *) buffer, 0);
//	UnmapViewOfFile((LPCVOID *) buffer);
//	CloseHandle(hMap);
//
//	SetFilePointer(hFile, -(int) ttainfo->id3v2.size, NULL, FILE_END);
//	SetEndOfFile(hFile);
//	CloseHandle(hFile);
//
//	ttainfo->FILESIZE -= ttainfo->id3v2.size;
//	ttainfo->id3v2.size = 0;
//
//	if (safe_mode) {
//		info.FILESIZE = ttainfo->FILESIZE;
//		info.id3v2.size = ttainfo->id3v2.size;
//		seek_needed = decode_pos_ms;
//		unpause();
//	}
//
//	ttainfo->id3v2.id3has = 0;
//}

/* eof */

