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

#ifndef DECODEFILE_H
#define DECODEFILE_H

#include "common.h"
#include "..\libtta++\libtta.h"

typedef struct {
	TTA_io_callback iocb;
	HANDLE handle;
} TTA_io_callback_wrapper;

void CALLBACK tta_callback(TTAuint32 rate, TTAuint32 fnum, TTAuint32 frames);
TTAint32 CALLBACK read_callback(_tag_TTA_io_callback *io, TTAuint8 *buffer, TTAuint32 size);
TTAint32 CALLBACK write_callback(_tag_TTA_io_callback *io, TTAuint8 *buffer, TTAuint32 size);
TTAint64 CALLBACK seek_callback(_tag_TTA_io_callback *io, TTAint64 offset);

class CDecodeFile
{
private:
	std::string				FileName;

	int						paused;
	long					seek_needed;
	double					decode_pos_ms;
	TTAuint64				pos;

	long					bitrate;			// kbps
	long					Filesize;			// total file size (in bytes)

	unsigned long			st_state;			// seek table status


	HANDLE					decoderFileHANDLE;

	tta::tta_decoder		*TTA;
	TTA_info				tta_info;
	__int64					signature;
	static const __int64	sig_number = 7792625911880894;
	CRITICAL_SECTION		CriticalSection;

public:
	TTA_io_callback_wrapper iocb_wrapper;

	CDecodeFile(void);
	CDecodeFile(CDecodeFile &s);
	~CDecodeFile(void);

	bool			isValid() {return sig_number == signature ? true : false;}

	int				SetFileName(const char *filename);
	const char	   *GetFileName(){return FileName.c_str();}
	int				GetSamples(BYTE *buffer, long buffersize, int *current_bitrate);

	int				GetPaused(){return paused;}
	void			SetPaused(int p){paused = p;}
	double			GetDecodePosMs(){return decode_pos_ms;}
	long double		SeekPosition(int *done);
	void			SetSeekNeeded(int sn){seek_needed = sn;}
	int				GetSeekNeeded(){return seek_needed;}
	int				GetSampleRate() {return tta_info.sps;}
	int				GetBitrate() {return (int) (bitrate);}
	__int32			GetNumberofChannel() {return tta_info.nch;}
	unsigned long	GetLengthbymsec() {return (unsigned long)(tta_info.samples / tta_info.sps * 1000);}
	int				GetDataLength() {return tta_info.samples;}
	TTAuint8		GetByteSize() {return TTAuint8(tta_info.bps/8);}
	unsigned long	GetOutputBPS() {return tta_info.bps;}
	void			SetOutputBPS(unsigned long bps);
	__int32			GetBitsperSample() {return tta_info.bps;}

};

class CDecodeFile_exception : public tta::tta_exception {
		tta_error err_code;

	public:
		CDecodeFile_exception(tta_error code) : tta::tta_exception(code) {}
		tta_error code() const { return err_code; }
	}; // class tta_exception
#endif
