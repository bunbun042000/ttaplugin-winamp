/*
 * ttaenc.h
 *
 * Description: TTA general portability definitions
 * Copyright (c) 2010 Aleksander Djuric. All rights reserved.
 * Distributed under the GNU Lesser General Public License (LGPL).
 * The complete text of the license can be found in the COPYING
 * file included in the distribution.
 *
 */

#ifndef _TTAENC_H
#define _TTAENC_H

#ifdef CARIBBEAN
#define ALLOW_OS_CODE 1

#include "../../../rmdef/rmdef.h"
#include "../../../rmlibcw/include/rmlibcw.h"
#include "../../../rmcore/include/rmcore.h"
#endif

#ifdef __GNUC__
#ifdef CARIBBEAN
typedef RMfile (HANDLE);
typedef RMint32 (TTAint32);
typedef RMint64 (TTAint64);
typedef RMuint8 (TTAuint8);
typedef RMuint16 (TTAuint16);
typedef RMuint32 (TTAuint32);
#else // GNUC
typedef int (HANDLE);
typedef int32_t (TTAint32);
typedef int64_t (TTAint64);
typedef uint8_t (TTAuint8);
typedef uint16_t (TTAuint16);
typedef uint32_t (TTAuint32);
#endif
#else // MSVC
typedef __int32 (TTAint32);
typedef __int64 (TTAint64);
typedef unsigned __int8 (TTAuint8);
typedef unsigned __int16 (TTAuint16);
typedef unsigned __int32 (TTAuint32);
#endif

#ifdef __GNUC__
#define LOCALE ""
#define T(x) x
#define tta_print fprintf
#ifdef CARIBBEAN
#define INVALID_HANDLE_VALUE (NULL)
#define tta_open_read(__name) RMOpenFile(__name,RM_FILE_OPEN_READ)
#define tta_open_write(__name) RMOpenFile(__name,RM_FILE_OPEN_WRITE)
#define tta_close(__handle) (RMCloseFile(__handle)==RM_OK?(0):(-1))
#define tta_unlink(__name) (RMFileUnlink(__name)==RM_OK?(0):(-1))
#define tta_read(__handle,__buffer,__size,__result) (RMReadFile(__handle,__buffer,__size,&(__result))==RM_OK?(1):(0))
#define tta_write(__handle,__buffer,__size,__result) (RMWriteFile(__handle,__buffer,__size,&(__result))==RM_OK?(1):(0))
#define tta_seek(__handle,__offset) (RMSeekFile(__handle,__offset,RM_FILE_SEEK_START)==RM_OK?(0):(-1))
#define tta_memclear(__dest,__length) RMMemset(__dest,0,__length)
#define tta_memcpy(__dest,__source,__length) RMMemcpy(__dest,__source,__length)
#define tta_malloc RMMalloc
#else // GNUC
#define INVALID_HANDLE_VALUE (-1)
#define tta_open_read(__name) open(__name,O_RDONLY|O_NONBLOCK)
#define tta_open_write(__name) open(__name,O_WRONLY|O_TRUNC|O_CREAT)
#define tta_close(__handle) close(__handle)
#define tta_unlink(__name) unlink(__name)
#define tta_read(__handle,__buffer,__size,__result) (__result=read(__handle,__buffer,__size))
#define tta_write(__handle,__buffer,__size,__result) (__result=write(__handle,__buffer,__size))
#define tta_seek(__handle,__offset) lseek64(__handle,__offset,SEEK_SET)
#define tta_memclear(__dest,__length) memset(__dest,0,__length)
#define tta_memcpy(__dest,__source,__length) memcpy(__dest,__source,__length)
#define tta_malloc malloc
#endif
#else // MSVC
#define LOCALE ".OCP"
#define STDIN_FILENO GetStdHandle(STD_INPUT_HANDLE)
#define STDOUT_FILENO GetStdHandle(STD_OUTPUT_HANDLE)
#define T(x) L ## x
#define tta_print fwprintf
#define tta_open_read(__name) CreateFileW(__name,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL)
#define tta_open_write(__name) CreateFileW(__name,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL|FILE_FLAG_SEQUENTIAL_SCAN,NULL)
#define tta_close(__handle) (CloseHandle(__handle)==TRUE?(0):(-1))
#define tta_unlink(__name) (DeleteFile(__name)==TRUE?(0):(-1))
#define tta_read(__handle,__buffer,__size,__result) ReadFile(__handle,__buffer,__size,(LPDWORD)&(__result),NULL)
#define tta_write(__handle,__buffer,__size,__result) WriteFile(__handle,__buffer,__size,(LPDWORD)&(__result),NULL)
#define tta_seek(__handle,__offset) SetFilePointer(__handle,(LONG)__offset,(PLONG)&__offset+1,FILE_BEGIN)
#define tta_memclear(__dest,__length) ZeroMemory(__dest,__length)
#define tta_memcpy(__dest,__source,__length) CopyMemory(__dest,__source,__length)
#define tta_malloc malloc
#endif

#endif // _TTAENC_H
