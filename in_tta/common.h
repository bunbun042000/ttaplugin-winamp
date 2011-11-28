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
#pragma once

#ifndef COMMON_H_
#define COMMON_H_

#include "stdafx.h"
#include "..\libtta-2.0-src\libtta.h"

static const double FRAME_TIME = 1.04489795918367346939;
static const __int32 BUFFER_LENGTH = 576;
static const __int32 BUFFER_SIZE = BUFFER_LENGTH * MAX_DEPTH * MAX_NCH;
static const __int32 MAX_PATHLEN = 512;
static const double SEEK_STEP = FRAME_TIME * 1000;
static const unsigned __int8 FIELD_TEXT_ISO_8859_1	= 0x00;
static const unsigned __int8 FIELD_TEXT_UTF_16		= 0x01;
static const unsigned __int8 FIELD_TEXT_UTF_16BE	= 0x02;
static const unsigned __int8 FIELD_TEXT_UTF_8		= 0x03;
static const unsigned __int8 FIELD_TEXT_MAX        = FIELD_TEXT_UTF_8;
static const unsigned __int8 UTF16_LE[] = {0xfe, 0xff};
static const unsigned __int8 UTF16_BE[] = {0xff, 0xfe};

CString GetEncodingString(const _TCHAR *string);
const char *SetEncodingString(CString &str, unsigned __int8 version = 0x04, unsigned __int8 Encoding = FIELD_TEXT_UTF_8);
const char *SetEncodingString(const _TCHAR *string, unsigned __int8 version = 0x04, unsigned __int8 Encoding = FIELD_TEXT_UTF_8);

#endif