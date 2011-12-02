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
#include "..\libtta++\libtta.h"

std::string GetEncodingString(const char *string)
{
	std::string sTempChar;

	__int32 dwStrSize = strlen(string) + 1;
	__int32 size = ::MultiByteToWideChar(CP_UTF8, 0, string, dwStrSize, NULL, 0);
	size++;
	wchar_t *tempchar = new wchar_t[size];
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

void UTF16toUTF16BE(wchar_t *str, int len)
{
	for(int i = 0; i < len; i++)
		str[i] = (str[i] << 8) | (str[i] >> 8);
}

const char *SetEncodingString(std::string &str, unsigned __int8 version, unsigned __int8 Encoding)
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
			tempchar = new char[str.length() + 1];
			if(tempchar == NULL)
				return NULL;
			memcpy_s((char *)tempchar, str.length() + 1, str.c_str(), str.length() + 1);
			break;
		}
		case FIELD_TEXT_UTF_16:	{
			__int32 size;
			if(str == "") 
				size = 2 * sizeof(wchar_t);
			else {
				size = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, 0, 0);
				size = (size + 1) * sizeof(wchar_t);
			}
			tempchar = new char[size];
			if(tempchar == NULL)
				return NULL;
			memcpy_s(tempchar, size, UTF16_LE, 2);
			if(str == "")
				memcpy_s(tempchar + 2, size - 2, "\0\0", sizeof(WCHAR));
			else {
				::MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, (wchar_t *)(tempchar + 2), 
					(size - 2) / sizeof(wchar_t));
			}
			break;
		}
		case FIELD_TEXT_UTF_16BE: {
			__int32 size;
			if(str == "")
				size = sizeof(wchar_t);
			else {
				size = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, 0, 0);
				size = (size  + 1) * sizeof(wchar_t);
			}
			tempchar = new char[size];
			if(tempchar == NULL) {
				return NULL;
			}
			if(str == "") {
				memcpy_s(tempchar, size, "\0\0", sizeof(wchar_t));
			} else {
				::MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, (wchar_t *)tempchar, size / sizeof(wchar_t));
				UTF16toUTF16BE((wchar_t *)tempchar, size / sizeof(wchar_t));
			}
			break;
		}
		case FIELD_TEXT_UTF_8: {
			unsigned char *tempDataUTF16;
			__int32 size;
			if(str == "")
				size = sizeof(unsigned char);
			else {
				__int32 tempsize = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, 0, 0);
				tempsize = tempsize * sizeof(wchar_t);
				tempDataUTF16 = new unsigned char[tempsize];
				if(tempDataUTF16 == NULL)
					return NULL;
				::MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, (wchar_t *)tempDataUTF16, tempsize / sizeof(wchar_t));
				size = ::WideCharToMultiByte(CP_UTF8, 0, (wchar_t *)tempDataUTF16, -1, NULL, 0, NULL, NULL);
				size += sizeof(unsigned char);
			}
			tempchar = new char[size];
			if(tempchar == NULL)
				return NULL;
			if(str == "")
				tempchar[0] = '\0';
			else
				::WideCharToMultiByte(CP_UTF8, 0, (wchar_t *)tempDataUTF16, -1, (char *)tempchar, size, NULL, NULL);
			break;
		}
	}
	return tempchar;
}

const char *SetEncodingString(const char *string, unsigned char version, unsigned char Encoding)
{
	std::string temp = string;
	return SetEncodingString(temp, version, Encoding);
}

const char *SetEncodingString(std::wstring &str, const char *locale, unsigned __int8 version, unsigned __int8 Encoding)
{
	return SetEncodingString(wcstostring(str.c_str(), locale), version, Encoding);
}

const char *SetEncodingString(const wchar_t *wstr, const char *locale, unsigned __int8 version, unsigned __int8 Encoding)
{
	std::string temp = wcstostring(wstr, locale);
	return SetEncodingString(temp, version, Encoding);
}

std::wstring GetWideString(const char *string)
{
	return mbstowstring(GetEncodingString(string));
}

std::wstring mbstowstring(const std::string &multibytestring, const char *locale)
{

	if ("" == multibytestring || NULL == locale) {
		return std::wstring(L"");
	} else {
		// do nothing
	}

	char *return_locale = setlocale(LC_ALL, locale);

	if (NULL == return_locale) {
		return std::wstring(L"");
	} else {
		// do nothing
	}

	size_t origsize = multibytestring.length() + 1;
	size_t demandSize = 0;
	errno_t err = mbstowcs_s(&demandSize, NULL, 0, multibytestring.c_str(), _TRUNCATE);

	if (0 != err || 0 == demandSize) {
		return std::wstring(L"");
	} else {
		//do nothing
	}

	wchar_t *return_widechar = new wchar_t[demandSize];

	size_t convertedChars = 0;
	mbstowcs_s(&convertedChars, return_widechar, demandSize, multibytestring.c_str(), _TRUNCATE);
	if (demandSize != convertedChars) {
		return std::wstring(L"");
	} else {
		// do nothing
	}

	return std::wstring(return_widechar);

}

std::wstring mbstowstring(const char *multibytestring, const char *locale)
{
	const std::string mbstring(multibytestring);
	return mbstowstring(mbstring, locale);
}

std::string wcstostring(const wchar_t *widestring, const char *locale)
{

	if (NULL == widestring || NULL == locale) {
		return std::string("");
	} else {
		// do nothing
	}

	char *return_locale = setlocale(LC_ALL, locale);

	if (NULL == return_locale) {
		return std::string("");
	} else {
		// do nothing
	}

	size_t origsize = wcslen(widestring) + 1;
	size_t demandSize = 0;
	errno_t err = wcstombs_s(&demandSize, NULL, 0, widestring, _TRUNCATE);

	if (0 != err || 0 == demandSize) {
		return std::string("");
	} else {
		//do nothing
	}

	char *returnchar = new char[demandSize];

	size_t convertedChars = 0;
	wcstombs_s(&convertedChars, returnchar, demandSize, widestring, _TRUNCATE);
	if (demandSize != convertedChars) {
		return std::string("");
	} else {
		// do nothing
	}

	return std::string(returnchar);
}