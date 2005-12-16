// in_tta.cpp : Defines the initialization routines for the DLL.
//
/* Description:	 TTA input plug-in for upper Winamp 2.91
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
//#include "Config.h"


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



#define  PLUGIN_VERSION "3.2 extended Beta3"
#define  PROJECT_URL "<http://www.sourceforge.net>"

static int paused = 0;
static unsigned int seek_needed = -1;
static unsigned int decode_pos_ms = 0;

static BYTE isobuffer[ISO_BUFFER_SIZE + 4];
static BYTE pcm_buffer[BUFFER_SIZE];	// PCM buffer
static decoder tta[2*MAX_NCH];			// decoder state
static long	cache[MAX_NCH];				// decoder cache
static long vis_buffer[BUFFER_SIZE*MAX_NCH];	// vis buffer

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
int __cdecl ispaused() { return paused; }
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

HANDLE heap;

int player_init();
int set_position(unsigned long pos);
int get_samples(BYTE *buffer, long count);
void pause() { paused = 1; mod.outMod->Pause(1); }
void unpause() { paused = 0; mod.outMod->Pause(0); }

void __cdecl init () {
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

	if (seek_table) {
		HeapFree(heap, 0, seek_table);
		seek_table = NULL;
	}
}

void __cdecl show_bitrate (int bitrate) {
	mod.SetInfo(bitrate, ttaTag.GetSampleRate()/1000, ttaTag.GetNumberofChannel(), 1);
}

int __cdecl play (char *filename) {
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

int  __cdecl getlength () { return ttaTag.GetLengthbymsec(); }
int  __cdecl getoutputtime () { return decode_pos_ms + (mod.outMod->GetOutputTime() - mod.outMod->GetWrittenTime()); }
void __cdecl setoutputtime (int time_in_ms) { seek_needed = time_in_ms; }
void __cdecl setvolume (int volume) { mod.outMod->SetVolume(volume); }
void __cdecl setpan (int pan) { mod.outMod->SetPan(pan); }
void __cdecl eq_set (int on, char data[10], int preamp) {}


void __cdecl getfileinfo (char *filename, char *title, int *length_in_ms) {
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

static DWORD WINAPI __stdcall DecoderThread (void *p) {
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
	__declspec(dllexport) In_Module* __cdecl winampGetInModule2(void)
	{
		return &mod;
	}

	__declspec(dllexport) int __cdecl
	winampGetExtendedFileInfo(extendedFileInfoStruct ExtendedFileInfo)
	{
		return m_Tag.GetExtendedFileInfo(mod.hMainWindow, &ExtendedFileInfo);
	}
}


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

	seek_pos = ttaTag.id3v2.GetTagLength() + seek_table[data_pos = pos];
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
						*p++ = (ttaTag.GetByteSize() == 1)? (BYTE)(*r + 0x80): (BYTE)*r;
						if (ttaTag.GetByteSize() > 1) *p++ = (BYTE)(*r >> 8);
						if (ttaTag.GetByteSize() > 2) *p++ = (BYTE)(*r >> 16);
					}
				}
				*p++ = (ttaTag.GetByteSize() == 1)? (BYTE)(*prev + 0x80): (BYTE)*prev;
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
