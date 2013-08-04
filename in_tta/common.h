/*
The ttaplugin-winamp project.
Copyright (C) 2005-2013 Yamagata Fumihiro

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

#ifndef COMMON_H_
#define COMMON_H_

#include "..\libtta++\libtta.h"
#include <string>

static const __int32 BUFFER_LENGTH = 576;
static const __int32 BUFFER_SIZE = BUFFER_LENGTH * MAX_DEPTH * MAX_NCH;
static const __int32 MAX_PATHLEN = 512;
static const unsigned __int8 FIELD_TEXT_ISO_8859_1	= 0x00;
static const unsigned __int8 FIELD_TEXT_UTF_16		= 0x01;
static const unsigned __int8 FIELD_TEXT_UTF_16BE	= 0x02;
static const unsigned __int8 FIELD_TEXT_UTF_8		= 0x03;
static const unsigned __int8 FIELD_TEXT_MAX        = FIELD_TEXT_UTF_8;
static const unsigned __int8 UTF16_LE[] = {0xfe, 0xff};
static const unsigned __int8 UTF16_BE[] = {0xff, 0xfe};

std::string GetEncodingString(const char *string);
const char *SetEncodingString(std::string &str, unsigned __int8 version = 0x04, unsigned __int8 Encoding = FIELD_TEXT_UTF_8);
const char *SetEncodingString(const char *string, unsigned __int8 version = 0x04, unsigned __int8 Encoding = FIELD_TEXT_UTF_8);

#endif