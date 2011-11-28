/*
 * tta.cpp
 *
 * Description: TTA simple console frontend
 * Copyright (c) 2010 Aleksander Djuric. All rights reserved.
 * Distributed under the GNU Lesser General Public License (LGPL).
 * The complete text of the license can be found in the COPYING
 * file included in the distribution.
 *
 */

#include <stdio.h>
#include <locale.h>
#include "../libtta.h"
#include "tta.h"

using namespace std;
using namespace tta;

//////////////////////// Constants and definitions //////////////////////////
/////////////////////////////////////////////////////////////////////////////

#define PROJECT_URL "http://www.true-audio.com/"

#define RIFF_SIGN (0x46464952)
#define WAVE_SIGN (0x45564157)
#define fmt_SIGN  (0x20746D66)
#define data_SIGN (0x61746164)

#define WAVE_FORMAT_PCM 1
#define WAVE_FORMAT_EXTENSIBLE 0xFFFE 
#define PCM_BUFFER_SIZE 5184

typedef struct {
	TTAuint32 chunk_id;
	TTAuint32 chunk_size;
	TTAuint32 format;
	TTAuint32 subchunk_id;
	TTAuint32 subchunk_size;
	TTAuint16 audio_format;
	TTAuint16 num_channels;
	TTAuint32 sample_rate;
	TTAuint32 byte_rate;
	TTAuint16 block_align;
	TTAuint16 bits_per_sample;
} WAVE_hdr;

typedef struct {
	TTAuint32 subchunk_id;
	TTAuint32 subchunk_size;
} WAVE_subchunk_hdr;

typedef struct {
	TTAuint32 f1;
	TTAuint16 f2;
	TTAuint16 f3;
	TTAuint8 f4[8];
} WAVE_subformat;

typedef struct {
	TTAuint16 cb_size;
	TTAuint16 valid_bits;
	TTAuint32 ch_mask;
	WAVE_subformat est;
} WAVE_ext_hdr;

typedef struct {
	TTA_io_callback iocb;
	HANDLE handle;
} TTA_io_callback_wrapper;

///////////////////////// Translate TTA error code //////////////////////////
/////////////////////////////////////////////////////////////////////////////

void tta_strerror(TTA_CODEC_STATUS e) {
	switch(e) {
		case TTA_OPEN_ERROR: fprintf(stderr, "Error:\tcan't open file\n"); break;
		case TTA_FORMAT_ERROR: fprintf(stderr, "Error:\tnot compatible file format\n"); break;
		case TTA_FILE_ERROR: fprintf(stderr, "Error:\tfile is corrupted\n"); break;
		case TTA_READ_ERROR: fprintf(stderr, "Error:\tcan't read from input file\n"); break;
		case TTA_WRITE_ERROR: fprintf(stderr, "Error:\tcan't write to output file\n"); break;
		case TTA_MEMORY_ERROR: fprintf(stderr, "Error:\tinsufficient memory available\n"); break;
		case TTA_SEEK_ERROR: fprintf(stderr, "Error:\tfile seek error\n"); break;
		case TTA_NO_ERROR:
		default: fprintf(stderr, "No known errors found\n"); break;
	}
} // tta_strerror

////////////////// Displays the proper usage for TTA.exe ////////////////////
/////////////////////////////////////////////////////////////////////////////

void usage() {
	fprintf(stderr, "Usage:\ttta [mode] input_file output_file\n\n");

	fprintf(stderr, "Modes:\n");
	fprintf(stderr, "\t-e\tencode file\n");
	fprintf(stderr, "\t-d\tdecode file\n\n");

	fprintf(stderr, "when file is '-', use standard input/output.\n\n");
	fprintf(stderr, "Project site: %s\n", PROJECT_URL);
} // usage

/////////////////////////////// WAV headers /////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

TTAuint32 read_wav_hdr(HANDLE infile, WAVE_hdr *wave_hdr) {
	WAVE_subchunk_hdr subchunk_hdr;
	TTAuint32 result;
	TTAuint32 def_subchunk_size = 16;

	// Read WAVE header
	if (!tta_read(infile, wave_hdr, sizeof(WAVE_hdr), result) || !result)
		throw tta_exception(TTA_READ_ERROR);

	if (wave_hdr->audio_format == WAVE_FORMAT_EXTENSIBLE) {
		WAVE_ext_hdr wave_hdr_ex;

		if (!tta_read(infile, &wave_hdr_ex, sizeof(WAVE_ext_hdr), result) || !result)
			throw tta_exception(TTA_READ_ERROR);

		def_subchunk_size += sizeof(WAVE_ext_hdr);
		wave_hdr->audio_format = wave_hdr_ex.est.f1;
	}

	// Skip extra format bytes
	if (wave_hdr->subchunk_size > def_subchunk_size) {
		TTAuint32 extra_len = wave_hdr->subchunk_size - def_subchunk_size;

		if (tta_seek(infile, extra_len) < 0 && errno != 0)
			throw tta_exception(TTA_READ_ERROR);
	}

	// Skip unsupported chunks
	while (1) {
		TTAuint8 chunk_id[5];

		if (!tta_read(infile, &subchunk_hdr, sizeof(WAVE_subchunk_hdr), result) || !result)
			throw tta_exception(TTA_READ_ERROR);

		if (subchunk_hdr.subchunk_id == data_SIGN) break;
		if (tta_seek(infile, subchunk_hdr.subchunk_size) < 0 && errno != 0)
			throw tta_exception(TTA_READ_ERROR);

		tta_memcpy(chunk_id, &subchunk_hdr.subchunk_id, 4);
		chunk_id[4] = 0;
	}

	return subchunk_hdr.subchunk_size;
} // read_wav_hdr

void write_wav_hdr(HANDLE outfile, WAVE_hdr *wave_hdr, TTAuint32 data_size) {
	TTAuint32 result;
	WAVE_subchunk_hdr subchunk_hdr;

	subchunk_hdr.subchunk_id = data_SIGN;
	subchunk_hdr.subchunk_size = data_size;

	// Write WAVE header
	if (!tta_write(outfile, wave_hdr, sizeof(WAVE_hdr), result) ||
		result != sizeof(WAVE_hdr)) throw tta_exception(TTA_WRITE_ERROR);

	// Write Subchunk header
	if (!tta_write(outfile, &subchunk_hdr, sizeof(WAVE_subchunk_hdr), result) ||
		result != sizeof(WAVE_subchunk_hdr)) throw tta_exception(TTA_WRITE_ERROR);
} // write_wav_hdr

/////////////////////////////// Callbacks ///////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void CALLBACK tta_callback(TTAuint32 rate, TTAuint32 fnum, TTAuint32 frames) {
	TTAuint32 pcnt = (TTAuint32)(fnum * 100. / frames);
	if (!(pcnt % 10))
		fprintf(stderr, "\rProgress: %02d%%", pcnt);
} // tta_callback

TTAint32 CALLBACK read_callback(TTA_io_callback *io, TTAuint8 *buffer, TTAuint32 size) {
	TTA_io_callback_wrapper *iocb = (TTA_io_callback_wrapper *)io; 
	TTAint32 result;
	if (tta_read(iocb->handle, buffer, size, result))
		return result;
	return 0;
} // read_callback

TTAint32 CALLBACK write_callback(TTA_io_callback *io, TTAuint8 *buffer, TTAuint32 size) {
	TTA_io_callback_wrapper *iocb = (TTA_io_callback_wrapper *)io; 
	TTAint32 result;
	if (tta_write(iocb->handle, buffer, size, result))
		return result;
	return 0;
} // write_callback

TTAint64 CALLBACK seek_callback(TTA_io_callback *io, TTAint64 offset) {
	TTA_io_callback_wrapper *iocb = (TTA_io_callback_wrapper *)io; 
	return tta_seek(iocb->handle, offset);
} // seek_callback

//////////////////////////////// Compress ///////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

TTAuint32 compress(HANDLE infile, HANDLE outfile) {
	TTAuint32 data_size;
	WAVE_hdr wave_hdr;
	tta_encoder *TTA;
	TTA_io_callback_wrapper io;

	io.iocb.write = &write_callback;
	io.iocb.seek = &seek_callback;
	io.iocb.read = NULL;
	io.handle = outfile;

	try {
		data_size = read_wav_hdr(infile, &wave_hdr);
	} catch (tta_exception ex) {
		tta_strerror(ex.code());
		return -1;
	}

	TTAuint32 smp_size = (wave_hdr.num_channels * ((wave_hdr.bits_per_sample + 7) / 8));

	try {
		TTA = new tta_encoder((TTA_io_callback *) &io);
		TTA->encoder_init(wave_hdr.num_channels, wave_hdr.bits_per_sample,
			wave_hdr.sample_rate, data_size / smp_size, 0);
	} catch (tta_exception ex) {
		tta_strerror(ex.code());
		return -1;
	}

	// allocate memory for PCM buffer
	TTAuint8 *buffer = (TTAuint8 *) malloc(PCM_BUFFER_SIZE * smp_size);
	if (buffer == NULL) {
		tta_strerror(TTA_MEMORY_ERROR);
		delete TTA;
		return -1;
	}

	try {
		while (data_size > 0) {
			TTAuint32 len;

			if (!tta_read(infile, buffer, PCM_BUFFER_SIZE * smp_size, len) || !len)
				throw tta_exception(TTA_READ_ERROR);
			if (len) {
				TTA->encode_stream(buffer, len, tta_callback);
			} else break;

			data_size -= len;
		}

		TTA->encoder_finalize();
	} catch (tta_exception ex) {
		tta_strerror(ex.code());
		delete TTA;
		delete [] buffer;
		return -1;
	}

	delete TTA;
	delete [] buffer;

	return 0;
} // compress

/////////////////////////////// Decompress //////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

TTAuint32 decompress(HANDLE infile, HANDLE outfile) {
	WAVE_hdr wave_hdr;
	tta_decoder *TTA;
	TTA_io_callback_wrapper io;

	io.iocb.read = &read_callback;
	io.iocb.seek = &seek_callback;
	io.iocb.write = NULL;
	io.handle = infile;

	try {
		TTA = new tta_decoder((TTA_io_callback *) &io);
		TTA->decoder_init();
	} catch (tta_exception ex) {
		tta_strerror(ex.code());
		return -1;
	}	

	TTAuint32 smp_size = TTA->info.nch * ((TTA->info.bps + 7) / 8);

	// allocate memory for PCM buffer
	TTAuint8 *buffer = (TTAuint8 *) malloc(PCM_BUFFER_SIZE * smp_size);

	if (buffer == NULL) {
		tta_strerror(TTA_MEMORY_ERROR);
		delete TTA;
		return -1;
	}

	TTAuint32 data_size = TTA->info.samples * smp_size;

	// Fill in WAV header
	tta_memclear(&wave_hdr, sizeof (wave_hdr));
	wave_hdr.chunk_id = RIFF_SIGN;
	wave_hdr.chunk_size = data_size + 36;
	wave_hdr.format = WAVE_SIGN;
	wave_hdr.subchunk_id = fmt_SIGN;
	wave_hdr.subchunk_size = 16;
	wave_hdr.audio_format = 1;
	wave_hdr.num_channels = (TTAuint16) TTA->info.nch;
	wave_hdr.sample_rate = TTA->info.sps;
	wave_hdr.bits_per_sample = TTA->info.bps;
	wave_hdr.byte_rate = TTA->info.sps * smp_size;
	wave_hdr.block_align = (TTAuint16) smp_size;

	// Write WAVE header
	try {
		write_wav_hdr(outfile, &wave_hdr, data_size);
	} catch (tta_exception ex) {
		tta_strerror(ex.code());
		delete TTA;
		delete [] buffer;
		return -1;
	}

	try {
		while (1) {
			TTAuint32 result;
			TTAuint32 len = TTA->decode_stream(buffer, PCM_BUFFER_SIZE, tta_callback);
			if (len) tta_write(outfile, buffer, len * smp_size, result);
			else break;
		}
	} catch (tta_exception ex) {
		tta_strerror(ex.code());
		delete TTA;
		delete [] buffer;
		return -1;
	}

	delete TTA;
	delete [] buffer;

	return 0;
} // decompress

//////////////////////////// The main function //////////////////////////////
/////////////////////////////////////////////////////////////////////////////

int
#ifdef __GNUC__
main(int argc, char **argv)
#else
__cdecl wmain(int argc, wchar_t **argv)
#endif
{
	HANDLE infile = INVALID_HANDLE_VALUE;
	HANDLE outfile = INVALID_HANDLE_VALUE;
	TTAuint32 result = -1;

	setlocale(LC_ALL, LOCALE);

	fprintf(stderr, "TTA1 lossless audio encoder/decoder.\n\n");

	if (argc != 4 ||
		argv[1][0] == '\0' ||
		argv[1][0] != '-' ||(
		argv[1][1] != 'e' &&
		argv[1][1] != 'd')) {
			usage();
			goto done;
	}

	if (argv[2][0] == '-') infile = STDIN_FILENO;
	else infile = tta_open_read(argv[2]);

	if (infile == INVALID_HANDLE_VALUE) {
		tta_strerror(TTA_OPEN_ERROR);
		goto done;
	}

	if (argv[3][0] == '-') outfile = STDOUT_FILENO;
	else outfile = tta_open_write(argv[3]);

	if (outfile == INVALID_HANDLE_VALUE) {
		tta_strerror(TTA_OPEN_ERROR);
		goto done;
	}

	// process
	if (argv[1][1] == 'e') {
		tta_print(stderr, T("Encoding: \"%s\" to \"%s\"\n"), argv[2], argv[3]);
		result = compress(infile, outfile);
	} else if (argv[1][1] == 'd') {
		tta_print(stderr, T("Decoding: \"%s\" to \"%s\"\n"), argv[2], argv[3]);
		result = decompress(infile, outfile);
	}

	if (infile != STDIN_FILENO) tta_close(infile);
	if (outfile != STDOUT_FILENO) {
		tta_close(outfile);
		if (result < 0)
			tta_unlink(argv[3]);
	}

done:
	fprintf(stderr, "\n");
	return result;
}

/* eof */
