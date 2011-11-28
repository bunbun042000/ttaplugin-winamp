#pragma once

#include "..\libtta-2.0-src\libtta.h"
#include "common.h"
//#include "..\include\ttadec.h"


//////////////////////// TTA hybrid filter ////////////////////////////


typedef struct {
	tta::TTA_io_callback iocb;
	HANDLE handle;
} TTA_io_callback_wrapper;

void CALLBACK tta_callback(TTAuint32 rate, TTAuint32 fnum, TTAuint32 frames);
TTAint32 CALLBACK read_callback(tta::_tag_TTA_io_callback *io, TTAuint8 *buffer, TTAuint32 size);
TTAint32 CALLBACK write_callback(tta::_tag_TTA_io_callback *io, TTAuint8 *buffer, TTAuint32 size);
TTAint64 CALLBACK seek_callback(tta::_tag_TTA_io_callback *io, TTAint64 offset);

class CDecodeFile
{
private:
	CString			FileName;

	int	            paused;
	long            seek_needed;
	double          decode_pos_ms;
	long            seek_skip;

	long            bitrate;    // kbps
	long            Filesize;   // total file size (in bytes)

	unsigned long	data_pos;			// currently playing frame index
	unsigned long	out_bps;			// output bps value

	unsigned long	st_state;			// seek table status


	HANDLE decoderFileHANDLE;

	tta::tta_decoder *TTA;

public:
	TTA_io_callback_wrapper iocb_wrapper;

	CDecodeFile(void);
	CDecodeFile(CDecodeFile &s);
	~CDecodeFile(void);

	int				SetFileName(const char *filename);
	const char	   *GetFileName(){return (LPCTSTR)FileName;}
	int				Play();
	int				Stop();
	int				GetSamples(BYTE *buffer, long count, int *current_bitrate);

	int				GetPaused(){return paused;}
	void			SetPaused(int p){paused = p;}
	long double		GetDecodePosMs(){return decode_pos_ms;}
	void		 	SetDecodePosMs(int d_pos_ms)
	{
		decode_pos_ms = d_pos_ms;
		TTA->set_position(decode_pos_ms);
	}
	long double		SeekPosition(int *done);
	void			SetSeekNeeded(int sn){seek_needed = sn;}
	int				GetSeekNeeded(){return seek_needed;}
	int				GetSampleRate() {return TTA->info.sps;}
	int				GetBitrate() {return (int) (bitrate);}
	__int32			GetNumberofChannel() {return TTA->info.nch;}
	unsigned long	GetLengthbymsec() {return (unsigned long)(TTA->info.samples / TTA->info.sps * 1000);}
	int				GetDataLength() {return TTA->info.samples;}
	unsigned long	GetDataPos(){return data_pos;}
	unsigned __int8	GetByteSize() {return unsigned __int8(TTA->info.depth);}
	unsigned long	GetOutputBPS() {return out_bps;}
	void			SetOutputBPS(unsigned long bps){out_bps = bps;}
	__int32			GetBitsperSample() {return TTA->info.bps;}
	int				GetLengthbyFrame() {return (int) (FRAME_TIME * TTA->info.flen);}

};
