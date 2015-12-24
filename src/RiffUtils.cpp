/*
Copyright (c) 2015, Dimitri Diakopoulos All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

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

WaveChunkHeader nqr::MakeWaveHeader(const EncoderParams param, const int sampleRate)
{
    WaveChunkHeader header;
    
    int bitdepth = GetFormatBitsPerSample(param.targetFormat);
    
    header.fmt_id = GenerateChunkCode('f', 'm', 't', ' ');
    header.chunk_size = 16;
    header.format = (param.targetFormat <= PCMFormat::PCM_32) ? WaveFormatCode::FORMAT_PCM : WaveFormatCode::FORMAT_IEEE;
    header.channel_count = param.channelCount;
    header.sample_rate = sampleRate;
    header.data_rate = sampleRate * param.channelCount * (bitdepth / 8);
    header.frame_size = param.channelCount * (bitdepth / 8);
    header.bit_depth = bitdepth;
    
    return header;
}
