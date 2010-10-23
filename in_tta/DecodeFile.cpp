#include "StdAfx.h"
#include "DecodeFile.h"

///////////////////////////////////////////////////////////////////////
// Description:	 TTAv1 lossless audio decoder.                       //
// Copyright (c) 1999-2004 Alexander Djourik. All rights reserved.   //
///////////////////////////////////////////////////////////////////////

///////// Filter Settings //////////
static long flt_set [4][2] = {
	{10,1}, {9,1}, {10,1}, {12,0}
};


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


///////////////////////// bit operations //////////////////////////////


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


CDecodeFile::CDecodeFile(void)
{
	paused = 0;
	seek_needed = -1;
	decode_pos_ms = 0;
	seek_skip = 0;

	isobuffer = new BYTE[ISO_BUFFER_SIZE + 4];
//	pcm_buffer = new BYTE[BUFFER_SIZE];
	tta = new decoder[2*MAX_NCH];
	cache = new long[MAX_NCH];

	heap = GetProcessHeap();
	decoderFileHANDLE = INVALID_HANDLE_VALUE;

}

CDecodeFile::CDecodeFile(CDecodeFile &s)
{
	FileName = s.FileName;
	ttaTag   = s.ttaTag;

	paused = s.paused;
	seek_needed = s.seek_needed;
	decode_pos_ms = s.decode_pos_ms;

	isobuffer = new BYTE[ISO_BUFFER_SIZE + 4];
//	pcm_buffer = new BYTE[BUFFER_SIZE];
	tta = new decoder[2*MAX_NCH];
	cache = new long[MAX_NCH];

	memcpy_s(isobuffer, ISO_BUFFER_SIZE + 4, s.isobuffer, (ISO_BUFFER_SIZE + 4));
//	memcpy_s(pcm_buffer, BUFFER_SIZE, s.pcm_buffer, BUFFER_SIZE);
	memcpy_s(tta, 2 * MAX_NCH, s.tta, 2 * MAX_NCH);
	memcpy_s(cache, MAX_NCH, s.cache, MAX_NCH);

	fframes = s.fframes;
	framelen = s.framelen;
	lastlen = s.lastlen;
	data_pos = s.data_pos;
	data_cur = s.data_cur;
	data_float = s.data_float;
	maxvalue = s.maxvalue;
	out_bps = s.out_bps;

	heap = GetProcessHeap();
	st_state = s.st_state;
	seek_table = s.seek_table;

	frame_crc32 = s.frame_crc32;
	bit_count = s.bit_count;
	bit_cache = s.bit_cache;
	bitpos = isobuffer + (s.bitpos - s.isobuffer);

	decoderFileHANDLE = INVALID_HANDLE_VALUE;
}

CDecodeFile::~CDecodeFile(void)
{
	delete [] isobuffer;
//	delete [] pcm_buffer;
	delete [] tta;
	delete [] cache;
	if (seek_table) {
		HeapFree(heap, 0, seek_table);
		seek_table = NULL;
	}
	decoderFileHANDLE = INVALID_HANDLE_VALUE;
}

int CDecodeFile::SetFileName(const char *filename)
{
	FileName = filename;
	// check for required data presented

	// open TTA file
	if (!ttaTag.ReadTag(FileName)) {
	//	if (info.STATE != FORMAT_ERROR)
	//		tta_error (info.STATE, filename);
	return 1;
	}
	decoderFileHANDLE = CreateFile(FileName, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (decoderFileHANDLE == INVALID_HANDLE_VALUE || decoderFileHANDLE == NULL) {
		return -1;
	}
	SetFilePointer(decoderFileHANDLE, ttaTag.id3v2.GetTagLength() + sizeof(TTA_header), NULL, FILE_BEGIN);

	if (player_init () < 0) {
//		tta_error (info.STATE, filename);
		return 1;
	}
	paused = 0;
	decode_pos_ms = 0;
	seek_needed = -1;
	return 0;
}

int CDecodeFile::player_init(void)
{
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
	if (!ReadFile(decoderFileHANDLE, seek_table, st_size, &result, NULL) ||
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


void  CDecodeFile::init_buffer_read(void)
{
    frame_crc32 = 0xFFFFFFFFUL;
    bit_count = bit_cache = 0;
    bitpos = (isobuffer + ISO_BUFFER_SIZE);
}

long double CDecodeFile::SeekPosition(int *done)
{
	if (seek_needed != -1){
		if (seek_needed >= (long)ttaTag.GetLengthbymsec()) {
			decode_pos_ms = ttaTag.GetLengthbymsec();
			*done = 1;
		} else {
			data_pos = (unsigned long)(seek_needed / SEEK_STEP);
			decode_pos_ms = seek_needed;
			seek_skip = (long)((seek_needed - (data_pos * SEEK_STEP)) / 1000. * ttaTag.GetSampleRate() + 0.5);
			seek_needed = -1;
			data_cur = -1;
		}
		set_position(data_pos);
	}
	return decode_pos_ms;
}

int CDecodeFile::GetSamples(BYTE *buffer, long count, int *current_bitrate)
{
	BYTE *temp = new BYTE[count*MAX_BSIZE*MAX_NCH];
	int skip_len = 0;
	int len;

	while (seek_skip > count) {
		if (!(len = get_decoded_data(temp, count, current_bitrate))) {
			seek_skip = 0;
//			done = 1; return 0;
			return 0;
		}
		skip_len += len;
		seek_skip -= len;
	}
	if (!(len = get_decoded_data(temp, count, current_bitrate))) {
		seek_skip = 0;
//		done = 1;
		return 0;
	} else {
		skip_len += len;
		len -= seek_skip;
		memcpy_s(buffer, count * ttaTag.GetNumberofChannel() * ttaTag.GetByteSize(),
			temp + seek_skip * ttaTag.GetNumberofChannel() * ttaTag.GetByteSize(), 
			len *ttaTag.GetNumberofChannel() * ttaTag.GetByteSize());
		seek_skip = 0;
		decode_pos_ms += (skip_len * 1000.L) / ttaTag.GetSampleRate();
	}
	delete [] temp;
	return len;

}

int CDecodeFile::get_decoded_data(BYTE *buffer, long count, int *current_bitrate) {
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
			if (framelen && done_buffer_read(current_bitrate)) {
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

int CDecodeFile::set_position (long double pos) {
	unsigned long seek_pos;

	if (pos >= fframes) return 0;
	if (!st_state) {
//		info.STATE = FILE_ERROR;
		return -1;
	}

	seek_pos = (unsigned long)(ttaTag.id3v2.GetTagLength()) + seek_table[data_pos = (unsigned long)pos];
	SetFilePointer(decoderFileHANDLE, seek_pos, NULL, FILE_BEGIN);

	data_cur = 0;
	framelen = 0;

	// init bit reader
	init_buffer_read();

	return 0;
}

void CDecodeFile::decoder_init(decoder *tta, long nch, long byte_size) {
    long *fset = flt_set[byte_size - 1];
    long i;

    for (i = 0; i < (nch << data_float); i++) {
		filter_init(&tta[i].fst, fset[0], fset[1]);
		rice_init(&tta[i].rice, 10, 10);
		tta[i].last = 0;
    }
}

void CDecodeFile::seek_table_init (unsigned long *seek_table,
	unsigned long len, unsigned long data_offset) {
	unsigned long *st, frame_len;

	for (st = seek_table; st < (seek_table + len); st++) {
		frame_len = *st; *st = data_offset;
		data_offset += frame_len;
	}
}

void CDecodeFile::rice_init(adapt *rice, BYTE k0, BYTE k1) {
    rice->k0 = k0;
    rice->k1 = k1;
    rice->sum0 = shift_16[k0];
    rice->sum1 = shift_16[k1];
}

void CDecodeFile::filter_init(fltst *fs, long shift, long mode) {
	ZeroMemory (fs, sizeof(fltst));
	fs->shift = shift;
	fs->round = 1 << (shift - 1);
	fs->mutex = mode;
}
