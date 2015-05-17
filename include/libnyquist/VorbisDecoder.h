#pragma comment(user, "license")

#ifndef VORBIS_DECODER_H
#define VORBIS_DECODER_H

#include "AudioDecoder.h"
#include "libvorbis/include/vorbis/vorbisfile.h"

namespace nqr
{
    
struct VorbisDecoder : public nqr::BaseDecoder
{
    VorbisDecoder() {}
    virtual ~VorbisDecoder() {}
    virtual int LoadFromPath(nqr::AudioData * data, const std::string & path) override;
    virtual int LoadFromBuffer(nqr::AudioData * data, const std::vector<uint8_t> & memory) override;
    virtual std::vector<std::string> GetSupportedFileExtensions() override;
};
    
} // end namespace nqr
#endif