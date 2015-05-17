#pragma comment(user, "license")

#include "Common.h"

/*
 
 AudioSampleBuffer can be backed by a memory store such as standard vector,
 
 a file, such as NyquistFileBuffer, or some platform specific DMA backed
 
 thingamajig, or a memmaped file.
 
 */


template <typename Buffer>

struct AudioSampleBuffer

{
    
    // RAII object to temporarily map a data buffer into R/O memory
    
    struct Data
    
    {
        
        Data(AudioSampleBuffer&); // map data into CPU memory
        
        ~Data(); // unmap data
        
        
        
        void *data();
        
        size_t size();
        
    };
    
    
    // RAII object to temporarily map a data buffer into R/W memory
    
    struct MutableData
    
    {
        
        MutableData(AudioSampleBuffer&); // map data into CPU memory
        
        ~MutableData(); // unmap data after copying contents back to backing store
        
        
        
        void *data();
        
        size_t size();
        
    };
    
    
    
    AudioSampleBuffer(std::function<size_t(size_t, Buffer)> resize,
                      
                      std::function<void()> dealloc)
    
    : resize(resize), dealloc(dealloc)
    
    {
        
    }
    
    
    
    ~AudioSampleBuffer()
    
    {
        
        if (dealloc)
            
            dealloc();
        
    }
    
    
    
    size_t resize(size_t sz)
    
    {
        
        if (resize)
            
            return resize(sz, &buffer);
        
        
        return buffer.size();
        
    }
    
    
    
private:
    
    AudioSampleBuffer() = 0;
    
    
    
    Buffer buffer;
    
    std::function<size_t(size_t, Buffer*)> _resize;
    
    std::function<void()> _dealloc;
    
};



struct AudioData

{
    
    AudioData() : channelCount(0), sampleRate(0), bitDepth(0), lengthSeconds(0), frameSize(0) { }
    
    
    
    AudioData(const AudioData& rhs)
    
    : channelCount(rhs.channelCount), sampleRate(rhs.sampleRate), bitDepth(rhs.bitDepth)
    
    , lengthInSeconds(rhs.lengthInSeconds), frameSize(rhs.frameSize), samples(rhs.samples) { }
    
    
    
    int channelCount; // unsigned?
    
    int sampleRate;   // ditto?
    
    int bitDepth;     // ditto?
    
    double lengthSeconds;
    
    
    
    // since it does make sense to set this (does it?) it should probably be a function not a data value
    
    size_t frameSize; // channels * bits per sample
    
    
    AudioSampleBuffer samples;
    
    //@todo: add field: streamable
    
    //@todo: add field: channel layout
    
    //@todo: add field: lossy / lossless
    
    
    
    void mixToChannels(int channelCount);
    
    void resample(int sampleRate);
    
    void setBitDepth(int bitDepth);
    
    void setLength(double lengthInSeconds);
    
};



// in general nice to pass string ref



void LoadFile(AudioData * dataResult, const std::string & path)

{
    
    string extension = path.substr(path.rfind('.') + 1, path.length());
    
    if (!extension.length())
        
        return;
    
    
    
    if (extension == "ogg")
        
    {
        
        VorbisDecoder::LoadFile(dataResult, path);
        
    }
    
    else if (extension == "wav")
        
    {
        
        VorbisDecoder::LoadFile(dataResult, path);
        
    }
    
    else
        
    {
        
        // etc.
        
    }
    
}


void LoadFile(AudioData * dataResult, const std::string & path, bool conformToData)

{
    
    if (!conformToData)
        
    {
        
        LoadFile(dataResult, path);
        
    }
    
    else
        
    {
        
        AudioData * conform = new dataResult(dataResult); // add copy constructor
        
        LoadFile(dataResult, path);
        
        
        if (conform->channelCount && (conform->channelCount != dataResult->channelCount))
            
            dataResult->mixToChannels(conform->channelCount);
        
        
        
        if (conform->sampleRate && (conform->sampleRate != dataResult->sampleRate))
            
            dataResult->resample(conform->sampleRate);
        
        
        
        if (conform->bitDepth && (conform->bitDepth != dataResult->bitDepth))
            
            dataResult->setBitDepth(conform->bitDepth);
        
        
        
        if (conform->lengthInSeconds > 0 && (conform->lengthSeconds != dataResult->lengthSeconds))
            
            dataResult->setLength(conform->lengthSeconds);
        
    }
    
}