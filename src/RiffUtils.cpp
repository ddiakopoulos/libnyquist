#include "RiffUtils.h"

using namespace nqr;

ChunkHeaderInfo nqr::ScanForChunk(const std::vector<uint8_t> & fileData, uint32_t chunkMarker)
{
    // D[n] aligned to 16 bytes now
    const uint16_t * d = reinterpret_cast<const uint16_t *>(fileData.data());
    
    for (size_t i = 0; i < fileData.size() / sizeof(uint16_t); i++)
    {
        // This will be in machine endianess
        uint32_t m = Pack(Read16(d[i]), Read16(d[i+1]));
        
        if (m == chunkMarker)
        {
            uint32_t cSz = Pack(Read16(d[i+2]), Read16(d[i+3]));
            return {(uint32_t (i * sizeof(uint16_t))), cSz}; // return i in bytes to the start of the data
        }
        else continue;
    }
    return {0, 0};
};