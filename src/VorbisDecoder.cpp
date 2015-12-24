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

#include "VorbisDecoder.h"
#include "libvorbis/include/vorbis/vorbisfile.h"

using namespace nqr;

//@todo: implement decoding from memory (c.f. http://stackoverflow.com/questions/13437422/libvorbis-audio-decode-from-memory-in-c)
class VorbisDecoderInternal
{
    
public:
    
    VorbisDecoderInternal(AudioData * d, std::string filepath) : d(d)
    {
        fileHandle = new OggVorbis_File();
        
        /* @todo proper steaming support + classes
         const ov_callbacks callbacks =
         {
             .read_func = s_readCallback,
             .seek_func = s_seekCallback,
             .tell_func = s_tellCallback,
             .close_func = nullptr
         };
         */
        
        FILE * f = fopen(filepath.c_str(), "rb");
        
        if (!f)
            throw std::runtime_error("Can't open file");
        
        if (auto r = ov_test_callbacks(f, fileHandle, nullptr, 0, OV_CALLBACKS_DEFAULT) != 0)
        {
            std::cerr << errorAsString(r) << std::endl;
            throw std::runtime_error("File is not a valid ogg vorbis file");
        }
        
        if (auto r = ov_test_open(fileHandle) != 0)
        {
            std::cerr << errorAsString(r) << std::endl;
            throw std::runtime_error("ov_test_open failed");
        }
        
        // Don't need to fclose() after an open -- vorbis does this internally
        
        vorbis_info *ovInfo = ov_info(fileHandle, -1);
        
        if (ovInfo == nullptr)
        {
            throw std::runtime_error("Reading metadata failed");
        }
        
        if (auto r = ov_streams(fileHandle) != 1)
        {
            std::cerr << errorAsString(r) << std::endl;
            throw std::runtime_error( "Unsupported: file contains multiple bitstreams");
        }
        
        d->sampleRate = int(ovInfo->rate);
        d->channelCount = ovInfo->channels;
        d->sourceFormat = MakeFormatForBits(32, true, false);
        d->lengthSeconds = double(getLengthInSeconds());
        d->frameSize = ovInfo->channels * GetFormatBitsPerSample(d->sourceFormat);
        
        // Samples in a single channel
        auto totalSamples = size_t(getTotalSamples());
        
        d->samples.resize(totalSamples * d->channelCount);
        
        if (!readInternal(totalSamples))
            throw std::runtime_error("could not read any data");
    }
    
    ~VorbisDecoderInternal()
    {
        ov_clear(fileHandle);
    }
    
    size_t readInternal(size_t requestedFrameCount, size_t frameOffset = 0)
    {
        
        //@todo support offset
        
        float **buffer = nullptr;
        size_t framesRemaining = requestedFrameCount;
        size_t totalFramesRead = 0;
        int bitstream = 0;
        
        while(0 < framesRemaining)
        {
            int64_t framesRead = ov_read_float(fileHandle, &buffer, std::min(2048, (int) framesRemaining), &bitstream);
            
            // EOF
            if(!framesRead)
                break;
            
            if (framesRead < 0)
            {
                // Probably OV_HOLE, OV_EBADLINK, OV_EINVAL. Log warning here.
                continue;
            }
            
            for (int i = 0; i < framesRead; ++i)
            {
                for(int ch = 0; ch < d->channelCount; ch++)
                {
                    d->samples[totalFramesRead] = buffer[ch][i];
                    totalFramesRead++;
                }
            }
            
        }
        
        return totalFramesRead;
    }
    
    std::string errorAsString(int ovErrorCode)
    {
        switch(ovErrorCode)
        {
            case OV_FALSE: return "OV_FALSE";
            case OV_EOF: return "OV_EOF";
            case OV_HOLE: return "OV_HOLE";
            case OV_EREAD: return "OV_EREAD";
            case OV_EFAULT: return "OV_EFAULT";
            case OV_EIMPL: return "OV_EIMPL";
            case OV_EINVAL: return "OV_EINVAL";
            case OV_ENOTVORBIS: return "OV_ENOTVORBIS";
            case OV_EBADHEADER: return "OV_EBADHEADER";
            case OV_EVERSION: return "OV_EVERSION";
            case OV_ENOTAUDIO: return "OV_ENOTAUDIO";
            case OV_EBADPACKET: return "OV_EBADPACKET";
            case OV_EBADLINK: return "OV_EBADLINK";
            case OV_ENOSEEK: return "OV_ENOSEEK";
            default: return "OV_UNKNOWN_ERROR";
        }
    }
    
    //////////////////////
    // vorbis callbacks //
    //////////////////////
    
    //@todo: implement streaming support
    
private:
    
    NO_COPY(VorbisDecoderInternal);
    
    OggVorbis_File * fileHandle;
    AudioData * d;
    
    inline int64_t getTotalSamples() const { return int64_t(ov_pcm_total(const_cast<OggVorbis_File *>(fileHandle), -1)); }
    inline int64_t getLengthInSeconds() const { return int64_t(ov_time_total(const_cast<OggVorbis_File *>(fileHandle), -1)); }
    inline int64_t getCurrentSample() const { return int64_t(ov_pcm_tell(const_cast<OggVorbis_File *>(fileHandle))); }
    
};

//////////////////////
// Public Interface //
//////////////////////

void VorbisDecoder::LoadFromPath(AudioData * data, const std::string & path)
{
    VorbisDecoderInternal decoder(data, path);
}

void VorbisDecoder::LoadFromBuffer(AudioData * data, const std::vector<uint8_t> & memory)
{
    throw LoadBufferNotImplEx();
}

std::vector<std::string> VorbisDecoder::GetSupportedFileExtensions()
{
    return {"ogg"};
}
