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

#include "WavEncoder.h"
#include <fstream>

using namespace nqr;

inline void toBytes(int value, char * arr)
{
    arr[0] = (value) & 0xFF;
    arr[1] = (value >> 8) & 0xFF;
    arr[2] = (value >> 16) & 0xFF;
    arr[3] = (value >> 24) & 0xFF;
}

WavEncoder::WavEncoder()
{
    
}

WavEncoder::~WavEncoder()
{
    
}

int WavEncoder::WriteFile(const EncoderParams p, const AudioData * d, const std::string & path)
{
    if (d->samples.size() <= 32)
    {
        return EncoderError::InsufficientSampleData;
    }
    
    if (d->channelCount < 1 || d->channelCount > 8)
    {
        return EncoderError::UnsupportedChannelConfiguration;
    }
    
    auto maxFileSizeInBytes = std::numeric_limits<uint32_t>::max();
    auto samplesSizeInBytes = d->samples.size() * sizeof(float);
    
    // 64 arbitrary
    if ((samplesSizeInBytes - 64) >= maxFileSizeInBytes)
    {
        return EncoderError::BufferTooBig;
    }
    
    // No resampling
    if (d->sampleRate != p.sampleRate)
    {
        return EncoderError::UnsupportedSamplerate;
    }
    
    std::ofstream fout(path.c_str(), std::ios::out | std::ios::binary);
    
    if (!fout.is_open())
    {
        return EncoderError::FileIOError;
    }
    
    char * chunkSizeBuff = new char[4];
    
    // Initial size
    toBytes(36, chunkSizeBuff);

    // RIFF file header
    fout.write(GenerateChunkCodeChar('R', 'I', 'F', 'F'), 4);
    fout.write(chunkSizeBuff, 4);
    
    fout.write(GenerateChunkCodeChar('W', 'A', 'V', 'E'), 4);
    
    // Fmt header
    auto header = MakeWaveHeader(p);
    fout.write(reinterpret_cast<char*>(&header), sizeof(WaveChunkHeader));
    
    // Data header
    fout.write(GenerateChunkCodeChar('d', 'a', 't', 'a'), 4);
    
    // + data chunk size
    toBytes(int(samplesSizeInBytes), chunkSizeBuff);
    fout.write(chunkSizeBuff, 4);
    
    // Debugging -- assume IEEE_Float
    auto sourceBits = GetFormatBitsPerSample(d->sourceFormat);
    auto targetBits = GetFormatBitsPerSample(p.targetFormat);

    // Apply dithering
    if (sourceBits != targetBits && p.dither != DITHER_NONE)
    {
        
    }
    else
    {
        fout.write(reinterpret_cast<const char*>(d->samples.data()), samplesSizeInBytes);
    }
    
    // Find size
    long totalSize = fout.tellp();
    
    // Modify RIFF header
    fout.seekp(4);
    
    // Total size of the file, less 8 bytes for the RIFF header
    toBytes(int(totalSize - 8), chunkSizeBuff);
    
    fout.write(chunkSizeBuff, 4);

    delete[] chunkSizeBuff;
    
    return EncoderError::NoError;
}
