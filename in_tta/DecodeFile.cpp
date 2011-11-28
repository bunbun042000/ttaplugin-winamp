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

#include "stdafx.h"
#include "DecodeFile.h"
#include "..\libtta-2.0-src\libtta.h"

TTAint32 CALLBACK read_callback(tta::TTA_io_callback *io, TTAuint8 *buffer, TTAuint32 size) {
	TTA_io_callback_wrapper *iocb = (TTA_io_callback_wrapper *)io; 
	TTAint32 result;

	result = ::ReadFile(iocb->handle, buffer, size, (LPDWORD)&result,NULL);

	if (result) {
		return result;
	}
	return 0;
} // read_callback

TTAint32 CALLBACK write_callback(tta::TTA_io_callback *io, TTAuint8 *buffer, TTAuint32 size) {
	TTA_io_callback_wrapper *iocb = (TTA_io_callback_wrapper *)io; 
	TTAint32 result;

	result = ::WriteFile(iocb->handle, buffer, size, (LPDWORD)&result, NULL);

	if (result){
		return result;
	}
	return 0;
} // write_callback

TTAint64 CALLBACK seek_callback(tta::TTA_io_callback *io, TTAint64 offset) {
	TTA_io_callback_wrapper *iocb = (TTA_io_callback_wrapper *)io; 
	return ::SetFilePointer(iocb->handle, (LONG)offset, NULL, FILE_BEGIN);
} // seek_callback


CDecodeFile::CDecodeFile(void)
{
	paused = 0;
	seek_needed = -1;
	decode_pos_ms = 0;
	seek_skip = 0;
	bitrate = 0;
	Filesize = 0;
	st_state = 0;

	decoderFileHANDLE = INVALID_HANDLE_VALUE;
	iocb_wrapper.handle = INVALID_HANDLE_VALUE;
	iocb_wrapper.iocb.read = NULL;
	iocb_wrapper.iocb.seek = NULL;
	iocb_wrapper.iocb.write = NULL;
	TTA = NULL;

}

CDecodeFile::CDecodeFile(CDecodeFile &s)
{
	FileName = s.FileName;

	paused = s.paused;
	seek_needed = s.seek_needed;
	decode_pos_ms = s.decode_pos_ms;
	bitrate = s.bitrate;
	Filesize = s.Filesize;
	seek_skip = s.seek_skip;

	data_pos = s.data_pos;
	out_bps = s.out_bps;

	st_state = s.st_state;

	decoderFileHANDLE = INVALID_HANDLE_VALUE;
	iocb_wrapper.handle = INVALID_HANDLE_VALUE;
	iocb_wrapper.iocb.read = NULL;
	iocb_wrapper.iocb.seek = NULL;
	iocb_wrapper.iocb.write = NULL;

	TTA = NULL;
}

CDecodeFile::~CDecodeFile(void)
{
	delete TTA;

	decoderFileHANDLE = INVALID_HANDLE_VALUE;
	iocb_wrapper.handle = INVALID_HANDLE_VALUE;
	iocb_wrapper.iocb.read = NULL;
	iocb_wrapper.iocb.seek = NULL;
	iocb_wrapper.iocb.write = NULL;
}

int CDecodeFile::SetFileName(const char *filename)
{
	// check for required data presented
	if (!filename) {
		return -1;
	}

	FileName = filename;
	decoderFileHANDLE = ::CreateFile(FileName, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (decoderFileHANDLE == INVALID_HANDLE_VALUE || decoderFileHANDLE == NULL) {
		return -1;
	}

	Filesize = ::GetFileSize(decoderFileHANDLE, NULL);

	iocb_wrapper.handle = decoderFileHANDLE;
	iocb_wrapper.iocb.read = &read_callback;
	iocb_wrapper.iocb.seek = &seek_callback;
	TTA = new tta::tta_decoder((tta::TTA_io_callback *) &iocb_wrapper);
	TTA->decoder_init();

	paused = 0;
	decode_pos_ms = 0;
	seek_needed = -1;
	out_bps = (TTA->info.bps > MAX_BPS)? MAX_BPS : TTA->info.bps;
	bitrate = (long)((Filesize - TTA->info.offset) / (TTA->info.frames * TTA->info.depth) * TTA->info.bps / 1000);

	if (TTA->seek_allowed){
		st_state = 1;
	} else {
		st_state = 0;
	}

	return 0;
}



long double CDecodeFile::SeekPosition(int *done)
{

	if (seek_needed >= (long)GetLengthbymsec()) {
		decode_pos_ms = (double)(GetLengthbymsec());
		*done = 1;
	} else {
		data_pos = (unsigned long)(seek_needed / SEEK_STEP);
		decode_pos_ms = data_pos * SEEK_STEP;
		seek_skip = (long)((seek_needed - (data_pos * SEEK_STEP)) / 1000. * TTA->info.sps + 0.5);
		seek_needed = -1;
	}
	TTA->set_position(decode_pos_ms / 1000.);

	return decode_pos_ms;
}

int CDecodeFile::GetSamples(BYTE *buffer, long count, int *current_bitrate)
{
	BYTE *temp = new BYTE[count * TTA->info.depth * TTA->info.nch];
	int skip_len = 0;
	int len = 0;

	while (seek_skip > count) {
		len = TTA->decode_stream(temp, count);

		if (len == 0) {
			seek_skip = 0;
			return 0;
		}
		
		skip_len += len;
		seek_skip -= len;
	}
	len = TTA->decode_stream(temp, count);
	if (len == 0) {
		seek_skip = 0;
		return 0;
	} else {
		skip_len += len;
		len -= seek_skip;
		memcpy_s(buffer, count * TTA->info.nch * TTA->info.depth,
			temp + seek_skip * TTA->info.nch * TTA->info.depth, 
			len * TTA->info.nch * TTA->info.depth);
		seek_skip = 0;
		decode_pos_ms += (skip_len * 1000.) / TTA->info.sps;
	}

	*current_bitrate = TTA->info.rate;
	delete [] temp;
	return len;

}
