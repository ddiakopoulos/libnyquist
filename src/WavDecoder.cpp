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

#include "WavDecoder.h"
#include "RiffUtils.h"
#include "IMA4Util.h"

using namespace nqr;

//////////////////////
// Public Interface //
//////////////////////

void WavDecoder::LoadFromPath(AudioData * data, const std::string & path)
{
    auto fileBuffer = nqr::ReadFile(path);
    return LoadFromBuffer(data, fileBuffer.buffer);
}

void WavDecoder::LoadFromBuffer(AudioData * data, const std::vector<uint8_t> & memory)
{
    //////////////////////
    // Read RIFF Header //
    //////////////////////
    
    //@todo swap methods for rifx
    
    RiffChunkHeader riffHeader = {};
    memcpy(&riffHeader, memory.data(), 12);
    
    // Files should be 2-byte aligned
    // @tofix: enforce this
    // bool usePaddingShort = ((riffHeader.file_size % sizeof(uint16_t)) == 1) ? true : false;
    
    // Check RIFF
    if (riffHeader.id_riff != GenerateChunkCode('R', 'I', 'F', 'F'))
    {
        // Check RIFX + FFIR
        if (riffHeader.id_riff == GenerateChunkCode('R', 'I', 'F', 'X') || riffHeader.id_riff == GenerateChunkCode('F', 'F', 'I', 'R'))
        {
            // We're not RIFF, and we don't match RIFX or FFIR either
            throw std::runtime_error("libnyquist doesn't support big endian files");
        }
        else
        {
            throw std::runtime_error("bad RIFF/RIFX/FFIR file header");
        }
    }
    
    if (riffHeader.id_wave != GenerateChunkCode('W', 'A', 'V', 'E')) throw std::runtime_error("bad WAVE header");
    
    auto expectedSize = (memory.size() - riffHeader.file_size);
    if (expectedSize != sizeof(uint32_t) * 2)
    {
        throw std::runtime_error("declared size of file less than file size"); //@todo warning instead of runtime_error
    }
    
    //////////////////////
    // Read WAVE Header //
    //////////////////////
    
    auto WaveChunkInfo = ScanForChunk(memory, GenerateChunkCode('f', 'm', 't', ' '));
    
    if (WaveChunkInfo.offset == 0) throw std::runtime_error("couldn't find fmt chunk");
    
    assert(WaveChunkInfo.size == 16 || WaveChunkInfo.size == 18 || WaveChunkInfo.size == 20 || WaveChunkInfo.size == 40);
    
    WaveChunkHeader wavHeader = {};
    memcpy(&wavHeader, memory.data() + WaveChunkInfo.offset, sizeof(WaveChunkHeader));
    
    if (wavHeader.chunk_size < 16)
        throw std::runtime_error("format chunk too small");
        
    //@todo validate wav header (sane sample rate, bit depth, etc)
    
    data->channelCount = wavHeader.channel_count;
    data->sampleRate = wavHeader.sample_rate;
    data->frameSize = wavHeader.frame_size;
    
    auto bit_depth = wavHeader.bit_depth;
    switch (bit_depth)
    {
        case 4: data->sourceFormat = PCMFormat::PCM_16; break; // for IMA ADPCM
        case 8: data->sourceFormat = PCMFormat::PCM_U8; break;
        case 16: data->sourceFormat = PCMFormat::PCM_16; break;
        case 24: data->sourceFormat = PCMFormat::PCM_24; break;
        case 32: data->sourceFormat = (wavHeader.format == WaveFormatCode::FORMAT_IEEE) ? PCMFormat::PCM_FLT : PCMFormat::PCM_32; break;
        case 64: data->sourceFormat = (wavHeader.format == WaveFormatCode::FORMAT_IEEE) ? PCMFormat::PCM_DBL : PCMFormat::PCM_64; break;
    }
    
    //std::cout << wavHeader << std::endl;
    
    bool scanForFact = false;
    bool grabExtensibleData = false;
    bool adpcmEncoded = false;
    
    if (wavHeader.format == WaveFormatCode::FORMAT_PCM)
    {
    }
    else if (wavHeader.format == WaveFormatCode::FORMAT_IEEE)
    {
        scanForFact = true;
    }
    else if (wavHeader.format == WaveFormatCode::FORMAT_IMA_ADPCM)
    {
        adpcmEncoded = true;
        scanForFact = true;
    }
    else if (wavHeader.format == WaveFormatCode::FORMAT_EXT)
    {
        // Used when (1) PCM data has more than 16 bits; (2) channels > 2; (3) bits/sample !== container size; (4) channel/speaker mapping specified
        //std::cout << "[format id] extended" << std::endl;
        scanForFact = true;
        grabExtensibleData = true;
    }
    else if (wavHeader.format ==  WaveFormatCode::FORMAT_UNKNOWN)
    {
        throw std::runtime_error("unknown wave format");
    }
    
    ////////////////////////////
    // Read Additional Chunks //
    ////////////////////////////
    
    FactChunk factChunk;
    if (scanForFact)
    {
        auto FactChunkInfo = ScanForChunk(memory, GenerateChunkCode('f', 'a', 'c', 't'));
        if (FactChunkInfo.size)
            memcpy(&factChunk, memory.data() + FactChunkInfo.offset, sizeof(FactChunk));
    }
    
    if (grabExtensibleData)
    {
        ExtensibleData extData = {};
        memcpy(&extData, memory.data() + WaveChunkInfo.offset + sizeof(WaveChunkHeader), sizeof(ExtensibleData));
        // extData can be compared against the multi-channel masks defined in the header
        // eg. extData.channel_mask == SPEAKER_5POINT1
    }
    
     //@todo smpl chunk could be useful
    
    /////////////////////
    // Read Bext Chunk //
    /////////////////////
    
    auto BextChunkInfo = ScanForChunk(memory, GenerateChunkCode('b', 'e', 'x', 't'));
    BextChunk bextChunk = {};
    
    if (BextChunkInfo.size)
    {
        memcpy(&bextChunk, memory.data() + BextChunkInfo.offset, sizeof(BextChunk));
    }
    
    /////////////////////
    // Read DATA Chunk //
    /////////////////////
    
    auto DataChunkInfo = ScanForChunk(memory, GenerateChunkCode('d', 'a', 't', 'a'));
    
    if (DataChunkInfo.offset == 0) 
        throw std::runtime_error("couldn't find data chunk");
    
    DataChunkInfo.offset += 2 * sizeof(uint32_t); // ignore the header and size fields

    if (adpcmEncoded)
    {
        ADPCMState s;
        s.frame_size = wavHeader.frame_size;
        s.firstDataBlockByte = 0;
        s.dataSize = DataChunkInfo.size;
        s.currentByte = 0;
        s.inBuffer = const_cast<uint8_t*>(memory.data() + DataChunkInfo.offset);
        
        size_t totalSamples = (factChunk.sample_length * wavHeader.channel_count); // Samples per channel times channel count
        std::vector<int16_t> adpcm_pcm16(totalSamples * 2, 0); // Each frame decodes into twice as many pcm samples
        
        uint32_t frameOffset = 0;
        uint32_t frameCount = DataChunkInfo.size / s.frame_size;

        for (int i = 0; i < frameCount; ++i)
        {
            decode_ima_adpcm(s, adpcm_pcm16.data() + frameOffset, wavHeader.channel_count);
            s.inBuffer += s.frame_size;
            frameOffset += (s.frame_size * 2) - (8 * wavHeader.channel_count);
        }
        
        data->lengthSeconds = ((float) totalSamples / (float) wavHeader.sample_rate) / wavHeader.channel_count;
        data->samples.resize(totalSamples);
        ConvertToFloat32(data->samples.data(), adpcm_pcm16.data(), totalSamples, data->sourceFormat);
    }
    else
    {
        data->lengthSeconds = ((float) DataChunkInfo.size / (float) wavHeader.sample_rate) / wavHeader.frame_size;
        size_t totalSamples = (DataChunkInfo.size / wavHeader.frame_size) * wavHeader.channel_count;
        data->samples.resize(totalSamples);
        ConvertToFloat32(data->samples.data(), memory.data() + DataChunkInfo.offset, totalSamples, data->sourceFormat);
    }
}

std::vector<std::string> WavDecoder::GetSupportedFileExtensions()
{
    return {"wav", "wave"};
}
