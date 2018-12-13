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

#include "Mp3Decoder.h"

using namespace nqr;

#include "mpc/mpcdec.h"
#include "mpc/reader.h"
#include "musepack/libmpcdec/decoder.h"
#include "musepack/libmpcdec/internal.h"

class Mp3Internal
{
 
    NO_MOVE(Mp3Internal);
    AudioData * d;

public:
    
    Mp3Internal(AudioData * d, const std::vector<uint8_t> & fileData) : d(d)
    {
        /*
        d->sampleRate = (int) streamInfo.sample_freq;
        d->channelCount = streamInfo.channels;
        d->sourceFormat = MakeFormatForBits(32, true, false);
        d->lengthSeconds = (double) mpc_streaminfo_get_length(&streamInfo);
        
        auto totalSamples = size_t(mpc_streaminfo_get_length_samples(&streamInfo));
        d->samples.reserve(totalSamples * d->channelCount + (MPC_DECODER_BUFFER_LENGTH / sizeof(MPC_SAMPLE_FORMAT))); // demux writes in chunks
        d->samples.resize(totalSamples * d->channelCount);
        
        if (!read_implementation())
        {
            throw std::runtime_error("could not read any data");
        }
        */
    }
    
    size_t read_implementation()
    {
        return 0;
    }

    ~Mp3Internal() {}
};

//////////////////////
// Public Interface //
//////////////////////

void Mp3Decoder::LoadFromPath(AudioData * data, const std::string & path)
{
    auto fileBuffer = nqr::ReadFile(path);
    Mp3Internal decoder(data, fileBuffer.buffer);
}

void Mp3Decoder::LoadFromBuffer(AudioData * data, const std::vector<uint8_t> & memory)
{
    Mp3Internal decoder(data, memory);
}

std::vector<std::string> Mp3Decoder::GetSupportedFileExtensions()
{
    return {"mp3"};
}
