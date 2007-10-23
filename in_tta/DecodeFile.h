#pragma once

#include "TtaTag.h"
#include "ttadec.h"
#include "crc32.h"


//////////////////////// TTA hybrid filter ////////////////////////////


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


class CDecodeFile
{
public:
	CDecodeFile(void);
	CDecodeFile(CDecodeFile &s);
	~CDecodeFile(void);

	int				SetFileName(char *filename);
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
		set_position(decode_pos_ms);
	}
	long double		SeekPosition(int *done);
	void			SetSeekNeeded(int sn){seek_needed = sn;}
	int				GetSeekNeeded(){return seek_needed;}
	int				GetSampleRate() {return ttaTag.GetSampleRate();}
	int				GetBitrate() {return ttaTag.GetBitrate();}
	__int32			GetNumberofChannel() {return ttaTag.GetNumberofChannel();}
	unsigned long	GetLengthbymsec() {return ttaTag.GetLengthbymsec();}
	int				GetDataLength() {return ttaTag.GetDataLength();}
	void			SetPlayTitle(char *title){ttaTag.SetPlayTitle(title);}
	unsigned long	GetDataPos(){return data_pos;}
	unsigned __int8	GetByteSize() {return ttaTag.GetByteSize();}
	unsigned long	GetOutputBPS() {return out_bps;}
	void			SetOutputBPS(unsigned long bps){out_bps = bps;}
	unsigned long	GetSeekTableState(){return st_state;}
	__int32			GetBitsperSample() {return ttaTag.GetBitsperSample();}
	int				GetLengthbyFrame() {return ttaTag.GetLengthbyFrame();}


private:
	CString			FileName;
	CTtaTag			ttaTag;

	int				paused;
	long			seek_needed;
	long double		decode_pos_ms;
	long			seek_skip;

	BYTE		   *isobuffer;
	decoder		   *tta;		// decoder state
	long		   *cache;		// decoder cache

	unsigned long	fframes;			// number of frames in file
	unsigned long	framelen;			// the frame length in samples
	unsigned long	lastlen;			// the length of the last frame in samples
	unsigned long	data_pos;			// currently playing frame index
	unsigned long	data_cur;			// the playing position in frame
	unsigned long	data_float;			// data type flag
	long			maxvalue;			// output data max value
	unsigned long	out_bps;			// output bps value

	unsigned long  *seek_table;			// the playing position table
	unsigned long	st_state;			// seek table status

	unsigned long	frame_crc32;
	unsigned long	bit_count;
	unsigned long	bit_cache;
	unsigned char  *bitpos;

	HANDLE heap;
	HANDLE decoderFileHANDLE;


	int		player_init();
	void	init_buffer_read();
	int		set_position(long double pos);
	void	decoder_init(decoder *tta, long nch, long byte_size);
	void	seek_table_init(unsigned long *seek_table,	unsigned long len, unsigned long data_offset);	
	void	rice_init(adapt *rice, BYTE k0, BYTE k1);
	void	filter_init(fltst *fs, long shift, long mode);
	int		get_decoded_data(BYTE *buffer, long count, int *current_bitrate);

	__inline int done_buffer_read(int *current_bitrate) {
		unsigned long crc32, rbytes;
		DWORD result;
		
		frame_crc32 ^= 0xFFFFFFFFUL;
		
		rbytes = (isobuffer + ISO_BUFFER_SIZE) - bitpos;
		if (rbytes < sizeof(long)) {
			*(long *)isobuffer = *(long *)bitpos;
			if (!ReadFile(decoderFileHANDLE, isobuffer + rbytes,
				ISO_BUFFER_SIZE - rbytes, &result, NULL) || !result) {
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
			rbytes = seek_table[data_pos] - seek_table[data_pos - 1];
			*current_bitrate = (rbytes << 3) / (long)(1000 * FRAME_TIME);
		}
		return result;
	}

	__inline void get_unary(unsigned long *value) {
		*value = 0;
		
		while (!(bit_cache ^ bit_mask[bit_count])) {
			if (bitpos == (isobuffer + ISO_BUFFER_SIZE)) {
				DWORD result;
				if (!ReadFile(decoderFileHANDLE, isobuffer, ISO_BUFFER_SIZE,
					&result, NULL) || !result) {
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

	__inline void get_binary(unsigned long *value, unsigned long bits) {
		while (bit_count < bits) {
			if (bitpos == (isobuffer + ISO_BUFFER_SIZE)) {
				DWORD result;
				if (!ReadFile(decoderFileHANDLE, isobuffer, ISO_BUFFER_SIZE,
					&result, NULL) || !result) {
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

};