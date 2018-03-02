/*
 Copyright (c) 2016, Dimitri Diakopoulos All rights reserved.
 
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

#include "ModplugDecoder.h"

using namespace nqr;

#ifndef MODPLUG_STATIC
#define MODPLUG_STATIC
#endif

#include "libmodplug/src/modplug.h"

class ModplugInternal
{
    
public:
    
    ModplugInternal(AudioData * d, std::vector<uint8_t> & fileData) : d(d)
    {
        ModPlug_Settings mps;
        ModPlug_GetSettings(&mps);
        mps.mChannels = 2;
        mps.mBits = 32;
        mps.mFrequency = 44100;
        mps.mResamplingMode = MODPLUG_RESAMPLE_FIR; //_LINEAR, _SPLINE
        mps.mStereoSeparation = 128;
        mps.mMaxMixChannels = 64;
        mps.mLoopCount = 0; // forever: -1;
        mps.mFlags = MODPLUG_ENABLE_OVERSAMPLING; // _NOISE_REDUCTION, _REVERB, _MEGABASS, _SURROUND
        ModPlug_SetSettings(&mps);
        
        mpf = ModPlug_Load((const void*)fileData.data(), fileData.size());
        if (!mpf)
        {
            throw std::runtime_error("could not load module");
        }
        
        d->sampleRate = 44100;
        d->channelCount = 2;
        d->sourceFormat = MakeFormatForBits(32, true, false);

        auto len_ms = ModPlug_GetLength(mpf);
        d->lengthSeconds = (double) len_ms / 1000.0;
        
        auto totalSamples = (44100LL * len_ms) / 1000;
        d->samples.resize(totalSamples * d->channelCount);

        auto readInternal = [&]()
        {
            const float invf = 1 / (float)0x7fffffff;
        	float *ptr = d->samples.data();
        	float *end = d->samples.data() + d->samples.size();

        	while( ptr < end ) {
	            int res = ModPlug_Read( mpf, (void*)ptr, (end - ptr) * sizeof(float) );
	            int samples_read = res / (sizeof(float) * 2);

	            if( totalSamples < samples_read ) {
	            	samples_read = totalSamples;
	            }

	            for( int i = 0; i < samples_read; ++i ) {
	                *ptr++ = *((int*)ptr) * invf;
	                *ptr++ = *((int*)ptr) * invf;
	            }

	            totalSamples -= samples_read;
        	}

            return ptr >= end;
        };

        if (!readInternal())
            throw std::runtime_error("could not read any data");

        ModPlug_Unload(mpf);
    }
    
private:

    ModPlugFile* mpf;
    
    NO_MOVE(ModplugInternal);
    
    AudioData * d;
};


//////////////////////
// Public Interface //
//////////////////////

void ModplugDecoder::LoadFromPath(AudioData * data, const std::string & path)
{
    auto fileBuffer = nqr::ReadFile(path);
    ModplugInternal decoder(data, fileBuffer.buffer);
}

void ModplugDecoder::LoadFromBuffer(AudioData * data, std::vector<uint8_t> & memory)
{
    ModplugInternal decoder(data, memory);
}

std::vector<std::string> ModplugDecoder::GetSupportedFileExtensions()
{
    return {"pat","mid", "mod","s3m","xm","it","669","amf","ams","dbm","dmf","dsm","far","mdl","med","mtm","okt","ptm","stm","ult","umx","mt2","psm"};
}

