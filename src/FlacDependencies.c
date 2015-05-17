#pragma comment(user, "license")

#undef VERSION
#define VERSION "1.3.1"
    
#define FLAC__NO_DLL 1
    
#if (_MSC_VER)
#pragma warning (push)
#pragma warning (disable: 181 111 4267 4996 4244 4701 4702 4133 4100 4127 4206 4312 4505 4365 4005 4013 4334)
#ifndef _WIN32
#define _WIN32
#endif
#endif

#if defined(__APPLE__) && defined(__MACH__)
#define FLAC__SYS_DARWIN 1
#endif
    
#ifndef SIZE_MAX
#define SIZE_MAX (size_t) (-1)
#endif
    
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wconversion"
#pragma clang diagnostic ignored "-Wshadow"
#pragma clang diagnostic ignored "-Wdeprecated-register"
#endif
    
#if CPU_X86
#ifdef __i386__
#define FLAC__CPU_IA32 1
#endif
#ifdef __x86_64__
#define FLAC__CPU_X86_64 1
#endif
#define FLAC__HAS_X86INTRIN 1
#endif
    
// Ensure libflac can use non-standard <stdint> types
#undef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS 1
    
#if defined(__APPLE__) && defined(__MACH__)
#define flac_max(a,b) ((a) > (b) ? a : b)
#define flac_min(a,b) ((a) < (b) ? a : b)
#elif defined(_MSC_VER)
#include <stdlib.h>
#define flac_max(a,b) __max(a,b)
#define flac_min(a,b) __min(a,b)
#endif
    
#define HAVE_LROUND 1

#include "flac/all.h"

#if defined(_MSC_VER)
#include "flac/src/win_utf8_io.c"
#endif

#include "flac/src/bitmath.c"
#include "flac/src/bitreader.c"
#include "flac/src/bitwriter.c"
#include "flac/src/cpu.c"
#include "flac/src/crc.c"
#include "flac/src/fixed.c"
#include "flac/src/float.c"
#include "flac/src/format.c"
#include "flac/src/lpc.c"
#include "flac/src/md5.c"
#include "flac/src/memory.c"
#include "flac/src/stream_decoder.c"
#include "flac/src/window.c"

#undef VERSION

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#if (_MSC_VER)
#pragma warning (pop)
#endif