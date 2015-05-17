#pragma comment(user, "license")

#ifndef LIBNYQUIST_COMMON_H
#define LIBNYQUIST_COMMON_H

#include <vector>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>
#include <stdint.h>
#include <cassert>
#include <type_traits>
#include <numeric>
#include "PostProcess.h"

namespace nqr
{
    
/////////////////
// Util Macros //
/////////////////

#define F_ROUND(x) ((x) > 0 ? (x) + 0.5f : (x) - 0.5f)
#define D_ROUND(x) ((x) > 0 ? (x) + 0.5  : (x) - 0.5)

#define NO_COPY(C) C(const C &) = delete; C & operator = (const C &) = delete
#define NO_MOVE(C) NO_COPY(C); C(C &&) = delete; C & operator = (const C &&) = delete

///////////////////////
// Endian Operations //
///////////////////////

#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
	#define CPU_X86 1
#endif

#if defined(__arm__) || defined(_M_ARM)
	#define CPU_ARM 1
#endif

#if defined(CPU_X86) && defined(CPU_ARM)
	#error CPU_X86 and CPU_ARM both defined.
#endif

#if !defined(ARCH_CPU_BIG_ENDIAN) && !defined(ARCH_CPU_LITTLE_ENDIAN)
	#if CPU_X86 || CPU_ARM || (defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
		#define ARCH_CPU_LITTLE_ENDIAN
	#elif defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
		#define ARCH_CPU_BIG_ENDIAN
	#else
		#error ARCH_CPU_BIG_ENDIAN or ARCH_CPU_LITTLE_ENDIAN should be defined.
	#endif
#endif

#if defined(ARCH_CPU_BIG_ENDIAN) && defined(ARCH_CPU_LITTLE_ENDIAN)
	#error ARCH_CPU_BIG_ENDIAN and ARCH_CPU_LITTLE_ENDIAN both defined.
#endif


    
static inline uint16_t Swap16(uint16_t value)
{
    return (uint16_t)((value >> 8) | (value << 8));
}

static inline uint32_t Swap24(uint32_t value)
{
    return (((value & 0x00ff0000) >> 16) |
            ((value & 0x0000ff00))       |
            ((value & 0x000000ff) << 16)) & 0x00FFFFFF;
}

static inline uint32_t Swap32(uint32_t value)
{
    return (((value & 0x000000ff) << 24) |
            ((value & 0x0000ff00) <<  8) |
            ((value & 0x00ff0000) >>  8) |
            ((value & 0xff000000) >> 24));
}

static inline uint64_t Swap64(uint64_t value)
{
    return (((value & 0x00000000000000ffLL) << 56) |
            ((value & 0x000000000000ff00LL) << 40) |
            ((value & 0x0000000000ff0000LL) << 24) |
            ((value & 0x00000000ff000000LL) << 8)  |
            ((value & 0x000000ff00000000LL) >> 8)  |
            ((value & 0x0000ff0000000000LL) >> 24) |
            ((value & 0x00ff000000000000LL) >> 40) |
            ((value & 0xff00000000000000LL) >> 56));
}

#ifdef ARCH_CPU_LITTLE_ENDIAN
    #define Read16(n) (n)
    #define Read24(n) (n)
    #define Read32(n) (n)
    #define Read64(n) (n)
    #define Write16(n) (n)
    #define Write24(n) (n)
    #define Write32(n) (n)
    #define Write64(n) (n)
#else
    #define Read16(n) Swap16(n)
    #define Read24(n) Swap24(n)
    #define Read32(n) Swap32(n)
    #define Read64(n) Swap64(n)
    #define Write16(n) Swap16(n)
    #define Write24(n) Swap24(n)
    #define Write32(n) Swap32(n)
    #define Write64(n) Swap64(n)
#endif

inline uint64_t Pack(uint32_t a, uint32_t b)
{
    uint64_t tmp =  (uint64_t) b << 32 | (uint64_t) a;
#ifdef ARCH_CPU_LITTLE_ENDIAN
    return tmp;
#else
    return Swap64(tmp);
#endif
}

inline uint32_t Pack(uint16_t a, uint16_t b)
{
    uint32_t tmp =  (uint32_t) b << 16 | (uint32_t) a;
#ifdef ARCH_CPU_LITTLE_ENDIAN
    return tmp;
#else
    return Swap32(tmp);
#endif
}

inline uint16_t Pack(uint8_t a, uint8_t b)
{
    uint16_t tmp =  (uint16_t) b << 8 | (uint16_t) a;
#ifdef ARCH_CPU_LITTLE_ENDIAN
    return tmp;
#else
    return Swap16(tmp);
#endif
}

// http://www.dsprelated.com/showthread/comp.dsp/136689-1.php
inline int32_t Pack(uint8_t a, uint8_t b, uint8_t c)
{
    // uint32_t tmp = ((c & 0x80) ? (0xFF << 24) : 0x00 << 24) | (c << 16) | (b << 8) | (a << 0); // alternate method
    int32_t x = (c << 16) | (b << 8) | (a << 0);
    auto sign_extended = (x) | (!!((x) & 0x800000) * 0xff000000);
    #ifdef ARCH_CPU_LITTLE_ENDIAN
        return sign_extended;
    #else
        Swap32(sign_extended);
    #endif
}

//////////////////////////
// Conversion Utilities //
//////////////////////////

// Signed maxes, defined for readabilty/convenience
#define NQR_INT16_MAX 32768.f
#define NQR_INT24_MAX 8388608.f
#define NQR_INT32_MAX 2147483648.f

static const float NQR_BYTE_2_FLT = 1.0f / 127.0f;

#define int8_to_float32(s)  ( (float) (s) * NQR_BYTE_2_FLT)
#define uint8_to_float32(s) ( ((float) (s) - 128) * NQR_BYTE_2_FLT)
#define int16_to_float32(s) ( (float) (s) / NQR_INT16_MAX)
#define int24_to_float32(s) ( (float) (s) / NQR_INT24_MAX)
#define int32_to_float32(s) ( (float) (s) / NQR_INT32_MAX)

//@todo add 12, 20 for flac
enum PCMFormat
{
	PCM_U8,
	PCM_S8,
	PCM_16,
	PCM_24,
	PCM_32,
	PCM_64,
	PCM_FLT,
	PCM_DBL,
    PCM_END
};

// Src data is aligned to PCMFormat
// @todo normalize?
void ConvertToFloat32(float * dst, const uint8_t * src, const size_t N, PCMFormat f);

// Src data is always aligned to 4 bytes (WavPack, primarily)
void ConvertToFloat32(float * dst, const int32_t * src, const size_t N, PCMFormat f);

//////////////////////////
// User Data + File Ops //
//////////////////////////

struct AudioData
{
	int channelCount;
	int sampleRate;
    int bitDepth;
	double lengthSeconds;
	size_t frameSize; // channels * bits per sample
	std::vector<float> samples;
    //@todo: add field: channel layout
    //@todo: add field: lossy / lossless
    //@todo: audio data loaded (for metadata only)
    //@todo: bitrate (if applicable)
    //@todo: original sample rate (if applicable)
};

struct StreamableAudioData : public AudioData
{
    //@todo: add field: is this format streamable?
    //@todo: hold file handle
};

struct NyquistFileBuffer
{
	std::vector<uint8_t> buffer;
	size_t size;
};

NyquistFileBuffer ReadFile(std::string pathToFile);

} // end namespace nqr

#endif
