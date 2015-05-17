#pragma comment(user, "license")

#ifndef AUDIO_DECODER_H
#define AUDIO_DECODER_H

#include "Common.h"
#include <utility>
#include <map>

namespace nqr
{
    
// Individual decoder classes will throw std::exceptions for bad things,
// but NyquistIO implements this enum for high-level error notifications.
enum IOError
{
    NoError,
    NoDecodersLoaded,
    ExtensionNotSupported,
    LoadPathNotImplemented,
    LoadBufferNotImplemented,
    UnknownError
};

struct BaseDecoder
{
    virtual int LoadFromPath(nqr::AudioData * data, const std::string & path) = 0;
    virtual int LoadFromBuffer(nqr::AudioData * data, const std::vector<uint8_t> & memory) = 0;
    virtual std::vector<std::string> GetSupportedFileExtensions() = 0;
    //@todo implement streaming helper methods here
};

    typedef std::pair<std::string, std::shared_ptr<nqr::BaseDecoder>> DecoderPair;

class NyquistIO
{
public:
    
    NyquistIO();
    ~NyquistIO();
    
    int Load(AudioData * data, const std::string & path);
    
    bool IsFileSupported(const std::string path) const;
    
private:
    
    std::string ParsePathForExtension(const std::string & path) const;
    
    std::shared_ptr<nqr::BaseDecoder> GetDecoderForExtension(const std::string ext);
    
    void BuildDecoderTable();
    
    template<typename T>
    void AddDecoderToTable(std::shared_ptr<T> decoder)
    {
        auto supportedExtensions = decoder->GetSupportedFileExtensions();
        
        //@todo: basic sanity checking that the extension isn't already supported
        for (const auto ext : supportedExtensions)
        {
            decoderTable.insert(DecoderPair(ext, decoder));
        }
    }
    
    std::map<std::string, std::shared_ptr<BaseDecoder>> decoderTable;
    
    NO_MOVE(NyquistIO);
};

} // end namespace nqr

#endif