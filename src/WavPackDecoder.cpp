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

#include "WavPackDecoder.h"
#include "wavpack.h"

using namespace nqr;

class WavPackInternal
{
    
public:
    
    WavPackInternal(AudioData * d, const std::string path) : d(d)
    {
        char errorStr[128];
        context = WavpackOpenFileInput(path.c_str(), errorStr, OPEN_WVC | OPEN_NORMALIZE, 0);
        
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

        /* From the docs:
            "... required memory at "buffer" is 4 * samples * num_channels bytes. The
            audio data is returned right-justified in 32-bit longs in the endian
            mode native to the executing processor."
        */
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
        
        // int frameSize = d->channelCount * WavpackGetBitsPerSample(context);
        
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
            //if (framesRead == 0)
            //    break;
            
            totalFramesRead += framesRead;
            framesRemaining -= framesRead;
        }
        
        return totalFramesRead;
    }
    
private:
    
    NO_MOVE(WavPackInternal);
    
    //WavpackStreamReader streamReader; //@todo: streaming support
    
    WavpackContext * context; //@todo unique_ptr
    
    AudioData * d;
    
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
    throw LoadBufferNotImplEx();
}

std::vector<std::string> WavPackDecoder::GetSupportedFileExtensions()
{
    return {"wv"};
}
