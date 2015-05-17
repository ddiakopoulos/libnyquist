#pragma comment(user, "license")

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
        
        d->sampleRate = WavpackGetSampleRate(context);
        d->channelCount = WavpackGetNumChannels(context);
        d->bitDepth = WavpackGetBitsPerSample(context);
        d->lengthSeconds = double(getLengthInSeconds());
        d->frameSize = d->channelCount * d->bitDepth;
        
        //@todo support channel masks
        // WavpackGetChannelMask
        
        auto totalSamples = size_t(getTotalSamples());
        
        PCMFormat internalFmt = PCMFormat::PCM_END;
        
        int mode = WavpackGetMode(context);
        int isFloatingPoint = (MODE_FLOAT & mode);
        
        switch (d->bitDepth)
        {
            case 16:
                internalFmt = PCMFormat::PCM_16;
                break;
            case 24:
                internalFmt = PCMFormat::PCM_24;
                break;
            case 32:
                internalFmt = isFloatingPoint ? PCMFormat::PCM_FLT : PCMFormat::PCM_32;
                break;
            default:
                throw std::runtime_error("unsupported WavPack bit depth");
                break;
        }
        
        /* From the docs:
            "... required memory at "buffer" is 4 * samples * num_channels bytes. The
            audio data is returned right-justified in 32-bit longs in the endian
            mode native to the executing processor."
        */
        d->samples.resize(totalSamples * d->channelCount);
        
        if (!isFloatingPoint)
            internalBuffer.resize(totalSamples * d->channelCount);
        
        auto r = readInternal(totalSamples);
        
        // Next, process internal buffer into the user-visible samples array
        if (!isFloatingPoint)
            ConvertToFloat32(d->samples.data(), internalBuffer.data(), totalSamples * d->channelCount, internalFmt);
        
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
            
            std::cout << framesRead << std::endl;
            
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

int WavPackDecoder::LoadFromPath(AudioData * data, const std::string & path)
{
    WavPackInternal decoder(data, path);
    return IOError::NoError;
}

int WavPackDecoder::LoadFromBuffer(AudioData * data, const std::vector<uint8_t> & memory)
{
    return IOError::LoadBufferNotImplemented;
}

std::vector<std::string> WavPackDecoder::GetSupportedFileExtensions()
{
    return {"wv"};
}
