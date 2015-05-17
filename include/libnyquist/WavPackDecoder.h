#pragma comment(user, "license")

#ifndef WAVEPACK_DECODER_H
#define WAVEPACK_DECODER_H

#include "AudioDecoder.h"

namespace nqr
{
    
struct WavPackDecoder : public nqr::BaseDecoder
{
    WavPackDecoder() {};
    virtual ~WavPackDecoder() {};
    virtual int LoadFromPath(nqr::AudioData * data, const std::string & path) override;
    virtual int LoadFromBuffer(nqr::AudioData * data, const std::vector<uint8_t> & memory) override;
    virtual std::vector<std::string> GetSupportedFileExtensions() override;
};
    
} // end namespace nqr

#endif
