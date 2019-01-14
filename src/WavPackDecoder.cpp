/*
Copyright (c) 2019, Dimitri Diakopoulos All rights reserved.

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

#include "Decoders.h"
#include "wavpack.h"
#include <string.h>
#include <cstring>

using namespace nqr;

class WavPackInternal
{
    
public:
    
    WavPackInternal(AudioData * d, const std::string & path) : d(d)
    {
        char errorStr[128];
        context = WavpackOpenFileInput(path.c_str(), errorStr, OPEN_WVC | OPEN_NORMALIZE, 0);
        
        if (!context) throw std::runtime_error("Not a WavPack file");
        
        auto bitdepth = WavpackGetBitsPerSample(context);
        
        d->sampleRate = WavpackGetSampleRate(context);
        d->channelCount = WavpackGetNumChannels(context);
        d->lengthSeconds = double(getLengthInSeconds());
        d->frameSize = d->channelCount * bitdepth;
        
        //@todo support channel masks
        // WavpackGetChannelMask
        
        auto totalSamples = size_t(getTotalSamples());
        
        int mode = WavpackGetMode(context);
        bool isFloatingPoint = (MODE_FLOAT & mode);
        
        d->sourceFormat = MakeFormatForBits(bitdepth, isFloatingPoint, false);

        /// From the  WavPack docs:
        /// "... required memory at "buffer" is 4 * samples * num_channels bytes. The
        /// audio data is returned right-justified in 32-bit longs in the endian
        /// mode native to the executing processor."
        d->samples.resize(totalSamples * d->channelCount);
        
        if (!isFloatingPoint)
            internalBuffer.resize(totalSamples * d->channelCount);
        
        if (!readInternal(totalSamples))
            throw std::runtime_error("could not read any data");
        
        // Next, process internal buffer into the user-visible samples array
        if (!isFloatingPoint)
            ConvertToFloat32(d->samples.data(), internalBuffer.data(), totalSamples * d->channelCount, d->sourceFormat);
        
    }

    WavPackInternal(AudioData * d, const std::vector<uint8_t> & memory) : d(d), data(std::move(memory)), dataPos(0)
    {
        WavpackStreamReader64 reader = 
        {
            read_bytes,
            write_bytes,
            get_pos,
            set_pos_abs,
            set_pos_rel,
            push_back_byte,
            get_length,
            can_seek,
            truncate_here,
            close,
        };

        char errorStr[128];
        context = WavpackOpenFileInputEx64(&reader, this, nullptr, errorStr, OPEN_WVC | OPEN_NORMALIZE, 0);
        
        if (!context)
        {
            throw std::runtime_error("Not a WavPack file");
        }
        
        auto bitdepth = WavpackGetBitsPerSample(context);
        
        d->sampleRate = WavpackGetSampleRate(context);
        d->channelCount = WavpackGetNumChannels(context);
        d->lengthSeconds = double(getLengthInSeconds());
        d->frameSize = d->channelCount * bitdepth;
        
        //@todo support channel masks
        // WavpackGetChannelMask
        
        auto totalSamples = size_t(getTotalSamples());
        
        int mode = WavpackGetMode(context);
        bool isFloatingPoint = (MODE_FLOAT & mode);
        
        d->sourceFormat = MakeFormatForBits(bitdepth, isFloatingPoint, false);

        d->samples.resize(totalSamples * d->channelCount);
        
        if (!isFloatingPoint)
            internalBuffer.resize(totalSamples * d->channelCount);
        
        if (!readInternal(totalSamples))
            throw std::runtime_error("could not read any data");
        
        // Next, process internal buffer into the user-visible samples array
        if (!isFloatingPoint)
            ConvertToFloat32(d->samples.data(), internalBuffer.data(), totalSamples * d->channelCount, d->sourceFormat);
        
    }
    
    ~WavPackInternal()
    {
        WavpackCloseFile(context);
    }
    
    size_t readInternal(size_t requestedFrameCount, size_t frameOffset = 0)
    {
        size_t framesRemaining = requestedFrameCount;
        size_t totalFramesRead = 0;
        
        // The samples returned are handled differently based on the file's mode
        int mode = WavpackGetMode(context);
        
        while (0 < framesRemaining)
        {
            uint32_t framesRead = -1;
            
            if (MODE_FLOAT & mode)
            {
                // Since it's float, we can decode directly into our buffer as a huge blob
                framesRead = WavpackUnpackSamples(context, reinterpret_cast<int32_t*>(&d->samples.data()[0]), uint32_t(d->samples.size() / d->channelCount));
            }
            
            else if(MODE_LOSSLESS & mode)
            {
                // Lossless files will be handed off as integers
                framesRead = WavpackUnpackSamples(context, internalBuffer.data(), uint32_t(internalBuffer.size() / d->channelCount));
            }
            
            // EOF
            //if (framesRead == 0) break;
            
            totalFramesRead += framesRead;
            framesRemaining -= framesRead;
        }
        
        return totalFramesRead;
    }

    static int32_t read_bytes(void * id, void * data, int32_t byte_count) 
    {
        if (id != nullptr) 
        {
            WavPackInternal *decoder = (WavPackInternal *)id;
            auto readLength = std::min<size_t>(byte_count, decoder->data.size() - decoder->dataPos);
            if (readLength > 0) 
            {
                std::memcpy(data, decoder->data.data(), readLength);
                decoder->dataPos += readLength;
                return readLength;
            } 
            else return 0;
        } 
        return 0;
    }
    static int32_t write_bytes(void * id, void * data, int32_t byte_count)
    {
        if (id != nullptr) 
        {
            WavPackInternal *decoder = (WavPackInternal *)id;
            auto writeLength = std::min<size_t>(byte_count, decoder->data.size() - decoder->dataPos);
            if (writeLength > 0) 
            {
                std::memcpy(decoder->data.data(), data, writeLength);
                decoder->dataPos += writeLength;
                return writeLength;
            } 
            else return 0;
        } 
        return 0;
    }
    static int64_t get_pos(void *id) 
    {
        if (id != nullptr) 
        {
            WavPackInternal *decoder = (WavPackInternal *)id;
            return decoder->dataPos;
        }
        return 0;
    }
    static int set_pos_abs(void *id, int64_t pos) 
    {
        if (id != nullptr) 
        {
            WavPackInternal *decoder = (WavPackInternal *)id;
            size_t newPos = std::min<size_t>(pos, decoder->data.size());
            decoder->dataPos = newPos;
            return newPos;
        } 
        return 0;
    }
    static int set_pos_rel(void *id, int64_t delta, int mode) 
    {
        if (id != nullptr) 
        {
            WavPackInternal *decoder = (WavPackInternal *)id;
            size_t newPos = 0;
            if (mode == SEEK_SET) newPos = delta;
            else if (mode == SEEK_CUR) newPos = decoder->dataPos + delta;
            else if (mode == SEEK_END) newPos = decoder->data.size() + delta;
            newPos = std::min<size_t>(newPos, decoder->data.size());
            decoder->dataPos = newPos;
            return newPos;
        } 
        return 0;
    }
    static int push_back_byte(void *id, int c) 
    {
        if (id != nullptr) 
        {
            WavPackInternal *decoder = (WavPackInternal *)id;
            decoder->dataPos--;
            decoder->data[decoder->dataPos] = c;
            return 1;
        } 
        return 0;
    }
    static int64_t get_length(void *id) 
    {
        if (id != nullptr) 
        {
            WavPackInternal *decoder = (WavPackInternal *)id;
            return decoder->data.size();
        } 
        return 0;
    }
    static int can_seek(void *id) 
    {
        if (id != nullptr) return 1;
        return 0;
    }

    static int truncate_here(void *id) 
    {
        if (id != nullptr) 
        {
            WavPackInternal *decoder = (WavPackInternal *)id;
            decoder->data.resize(decoder->dataPos);
            return 1;
        } 
        return 0;
    }
    static int close(void *id) 
    {
        if (id != nullptr) return 1;
        return 0;
    }
    
private:
    
    NO_MOVE(WavPackInternal);
    
    //WavpackStreamReader streamReader; //@todo: streaming support
    
    WavpackContext * context; //@todo unique_ptr
    
    AudioData * d;
    std::vector<uint8_t> data;
    size_t dataPos;
    
    std::vector<int32_t> internalBuffer;
    
    inline int64_t getTotalSamples() const { return WavpackGetNumSamples(context); }
    inline int64_t getLengthInSeconds() const { return getTotalSamples() / WavpackGetSampleRate(context); }
    
};

//////////////////////
// Public Interface //
//////////////////////

void WavPackDecoder::LoadFromPath(AudioData * data, const std::string & path)
{
    WavPackInternal decoder(data, path);
}

void WavPackDecoder::LoadFromBuffer(AudioData * data, const std::vector<uint8_t> & memory)
{
    WavPackInternal decoder(data, memory);
}

std::vector<std::string> WavPackDecoder::GetSupportedFileExtensions()
{
    return {"wv"};
}
