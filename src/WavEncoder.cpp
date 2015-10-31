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

int WavEncoder::WriteFile(const EncoderParams p, const AudioData * d, const std::string & path)
{
    // Cast away const because we know what we are doing (Hopefully?)
    float * sampleData = const_cast<float *>(d->samples.data());
    size_t sampleDataSize = d->samples.size();
    
    std::vector<float> sampleDataOptionalMix;
    
    if (sampleDataSize <= 32)
    {
        return EncoderError::InsufficientSampleData;
    }
    
    if (d->channelCount < 1 || d->channelCount > 8)
    {
        return EncoderError::UnsupportedChannelConfiguration;
    }
    
    // Handle Channel Mixing --
    
    // Mono => Stereo
    if (d->channelCount == 1 && p.channelCount == 2)
    {
        sampleDataOptionalMix.resize(sampleDataSize * 2);
        MonoToStereo(sampleData, sampleDataOptionalMix.data(), sampleDataSize); // Mix
        
        // Re-point data
        sampleData = sampleDataOptionalMix.data();
        sampleDataSize = sampleDataOptionalMix.size();
    }
    
    // Stereo => Mono
    else if (d->channelCount == 2 && p.channelCount == 1)
    {
        sampleDataOptionalMix.resize(sampleDataSize / 2);
        StereoToMono(sampleData, sampleDataOptionalMix.data(), sampleDataSize); // Mix
        
        // Re-point data
        sampleData = sampleDataOptionalMix.data();
        sampleDataSize = sampleDataOptionalMix.size();
        
    }
    
    else if (d->channelCount == p.channelCount)
    {
        // No op
    }
    
    else
    {
        return EncoderError::UnsupportedChannelMix;
    }
    // -- End Channel Mixing
    
    auto maxFileSizeInBytes = std::numeric_limits<uint32_t>::max();
    auto samplesSizeInBytes = (sampleDataSize * GetFormatBitsPerSample(p.targetFormat)) / 8;
    
    // 64 arbitrary
    if ((samplesSizeInBytes - 64) >= maxFileSizeInBytes)
    {
        return EncoderError::BufferTooBig;
    }
    
    // Don't support PCM_64 or PCM_DBL
    if (GetFormatBitsPerSample(p.targetFormat) > 32)
    {
        return EncoderError::UnsupportedBitdepth;
    }
    
    std::ofstream fout(path.c_str(), std::ios::out | std::ios::binary);
    
    if (!fout.is_open())
    {
        return EncoderError::FileIOError;
    }
    
    char * chunkSizeBuff = new char[4];
    
    // Initial size (this is changed after we're done writing the file)
    toBytes(36, chunkSizeBuff);

    // RIFF file header
    fout.write(GenerateChunkCodeChar('R', 'I', 'F', 'F'), 4);
    fout.write(chunkSizeBuff, 4);
    
    fout.write(GenerateChunkCodeChar('W', 'A', 'V', 'E'), 4);
    
    // Fmt header
    auto header = MakeWaveHeader(p, d->sampleRate);
    fout.write(reinterpret_cast<char*>(&header), sizeof(WaveChunkHeader));
    
    auto sourceBits = GetFormatBitsPerSample(d->sourceFormat);
    auto targetBits = GetFormatBitsPerSample(p.targetFormat);
    
    ////////////////////////////
    //@todo - channel mixing! //
    ////////////////////////////
    
    // Write out fact chunk
    if (p.targetFormat == PCM_FLT)
    {
        uint32_t four = 4;
        uint32_t dataSz = int(sampleDataSize / p.channelCount);
        fout.write(GenerateChunkCodeChar('f', 'a', 'c', 't'), 4);
        fout.write(reinterpret_cast<const char *>(&four), 4);
        fout.write(reinterpret_cast<const char *>(&dataSz), 4); // Number of samples (per channel)
    }
    
    // Data header
    fout.write(GenerateChunkCodeChar('d', 'a', 't', 'a'), 4);
    
    // + data chunk size
    toBytes(int(samplesSizeInBytes), chunkSizeBuff);
    fout.write(chunkSizeBuff, 4);
    
    if (targetBits <= sourceBits && p.targetFormat != PCM_FLT)
    {
        // At most need this number of bytes in our copy
        std::vector<uint8_t> samplesCopy(samplesSizeInBytes);
        ConvertFromFloat32(samplesCopy.data(), sampleData, sampleDataSize, p.targetFormat, p.dither);
        fout.write(reinterpret_cast<const char*>(samplesCopy.data()), samplesSizeInBytes);
    }
    else
    {
        // Handle PCM_FLT. Coming in from AudioData ensures we start with 32 bits, so we're fine
        // since we've also rejected formats with more than 32 bits above.
        fout.write(reinterpret_cast<const char*>(sampleData), samplesSizeInBytes);
    }
    
    // Padding byte
    if (isOdd(samplesSizeInBytes))
    {
        const char * zero = "";
        fout.write(zero, 1);
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
