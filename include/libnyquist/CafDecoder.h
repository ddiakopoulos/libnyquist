#pragma comment(user, "license")

#ifndef CAF_DECODER_H
#define CAF_DECODER_H

#include "AudioDecoder.h"

namespace nqr
{
    
struct CAFDecoder : public nqr::BaseDecoder
{
    CAFDecoder() {};
    virtual ~CAFDecoder() {};
    virtual int LoadFromPath(nqr::AudioData * data, const std::string & path) override;
    virtual int LoadFromBuffer(nqr::AudioData * data, const std::vector<uint8_t> & memory) override;
    virtual std::vector<std::string> GetSupportedFileExtensions() override;
};

} // end namespace nqr

#endif