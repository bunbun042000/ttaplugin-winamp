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

#include "common.h"
#include "..\libtta-2.0-src\libtta.h"

CString GetEncodingString(const _TCHAR *string)
{
	CString sTempChar;

	__int32 dwStrSize = strlen(string) + 1;
	__int32 size = ::MultiByteToWideChar(CP_UTF8, 0, string, dwStrSize, NULL, 0);
	size++;
	WCHAR *tempchar = new WCHAR[size];
	if(tempchar == NULL)
		return "";
	::MultiByteToWideChar(CP_UTF8, 0, string, dwStrSize, tempchar, size - 1);
	tempchar[size - 1] = L'\0';

	size = ::WideCharToMultiByte(CP_ACP, 0, tempchar, -1, 0, 0, NULL, NULL);
	char *tempchar2 = new char[size];
	if(tempchar2 == NULL) {
		delete tempchar;
		return "";
	}
	::WideCharToMultiByte(CP_ACP, 0, tempchar, -1, tempchar2, size, NULL, NULL);
	sTempChar = tempchar2;
	delete tempchar2;
	delete tempchar;

	return sTempChar;
}

void UTF16toUTF16BE(WCHAR *str, int len)
{
	for(int i = 0; i < len; i++)
		str[i] = (str[i] << 8) | (str[i] >> 8);
}

const char *SetEncodingString(CString &str, unsigned __int8 version, unsigned __int8 Encoding)
{
	if((Encoding > FIELD_TEXT_MAX) || (version != 0x03 && version != 0x04)) 
		return NULL;

	if(version == 0x03 && Encoding > FIELD_TEXT_UTF_16)
		return NULL;

	char *tempchar;


	switch(Encoding) {
		case FIELD_TEXT_ISO_8859_1:
		default: {
			if(str == "") {
				tempchar = new char[1];
				tempchar[0] = '\0';
				break;
			}
			tempchar = new char[str.GetLength() + 1];
			if(tempchar == NULL)
				return NULL;
			memcpy_s((char *)tempchar, str.GetLength() + 1, (LPCTSTR)str, str.GetLength() + 1);
			break;
		}
		case FIELD_TEXT_UTF_16:	{
			__int32 size;
			if(str == "") 
				size = 2 * sizeof(WCHAR);
			else {
				size = ::MultiByteToWideChar(CP_ACP, 0, (LPCTSTR)str, -1, 0, 0);
				size = (size + 1) * sizeof(WCHAR);
			}
			tempchar = new char[size];
			if(tempchar == NULL)
				return NULL;
			memcpy_s(tempchar, size, UTF16_LE, 2);
			if(str == "")
				memcpy_s(tempchar + 2, size - 2, "\0\0", sizeof(WCHAR));
			else {
				::MultiByteToWideChar(CP_ACP, 0, (LPCTSTR)str, -1, (WCHAR *)(tempchar + 2), 
					(size - 2) / sizeof(WCHAR));
			}
			break;
		}
		case FIELD_TEXT_UTF_16BE: {
			__int32 size;
			if(str == "")
				size = sizeof(WCHAR);
			else {
				size = ::MultiByteToWideChar(CP_ACP, 0, str, -1, 0, 0);
				size = (size  + 1) * sizeof(WCHAR);
			}
			tempchar = new char[size];
			if(tempchar == NULL) {
				return NULL;
			}
			if(str == "") {
				memcpy_s(tempchar, size, "\0\0", sizeof(WCHAR));
			} else {
				::MultiByteToWideChar(CP_ACP, 0, (LPCTSTR)str, -1, (WCHAR *)tempchar, size / sizeof(WCHAR));
				UTF16toUTF16BE((WCHAR *)tempchar, size / sizeof(WCHAR));
			}
			break;
		}
		case FIELD_TEXT_UTF_8: {
			unsigned char *tempDataUTF16;
			__int32 size;
			if(str == "")
				size = sizeof(unsigned char);
			else {
				__int32 tempsize = ::MultiByteToWideChar(CP_ACP, 0, (LPCTSTR)str, -1, 0, 0);
				tempsize = tempsize * sizeof(WCHAR);
				tempDataUTF16 = new unsigned char[tempsize];
				if(tempDataUTF16 == NULL)
					return NULL;
				::MultiByteToWideChar(CP_ACP, 0, (LPCTSTR)str, -1, (WCHAR *)tempDataUTF16, tempsize / sizeof(WCHAR));
				size = ::WideCharToMultiByte(CP_UTF8, 0, (WCHAR *)tempDataUTF16, -1, NULL, 0, NULL, NULL);
				size += sizeof(unsigned char);
			}
			tempchar = new char[size];
			if(tempchar == NULL)
				return NULL;
			if(str == "")
				tempchar[0] = '\0';
			else
				::WideCharToMultiByte(CP_UTF8, 0, (WCHAR *)tempDataUTF16, -1, (char *)tempchar, size, NULL, NULL);
			break;
		}
	}
	return tempchar;
}

const char *SetEncodingString(const _TCHAR *string, unsigned char version, unsigned char Encoding)
{
	CString temp = string;
	return SetEncodingString(temp, version, Encoding);
}