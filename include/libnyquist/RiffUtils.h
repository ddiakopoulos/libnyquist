#pragma comment(user, "license")

#ifndef RIFF_UTILS_H
#define RIFF_UTILS_H

#include "Common.h"

namespace nqr
{
    
/////////////////////
// Chunk utilities //
/////////////////////

struct ChunkHeaderInfo
{
    uint32_t offset;			// Byte offset into chunk
    uint32_t size;				// Size of the chunk in bytes
};

inline uint32_t GenerateChunkCode(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
{
    #ifdef ARCH_CPU_LITTLE_ENDIAN
        return ((uint32_t) ((a) | ((b) << 8) | ((c) << 16) | (((uint32_t) (d)) << 24)));
    #else
        return ((uint32_t) ((((uint32_t) (a)) << 24) | ((b) << 16) | ((c) << 8) | (d)));
    #endif
}

ChunkHeaderInfo ScanForChunk(const std::vector<uint8_t> & fileData, uint32_t chunkMarker);

} // end namespace nqr

#endif
