#pragma comment(user, "license")

#ifndef OPUS_DECODER_H
#define OPUS_DECODER_H

#include "AudioDecoder.h"
#include "opus/opusfile/include/opusfile.h"

namespace nqr
{
    
// Opus is a general-purpose codec designed to replace Vorbis at some point. Primarily, it's a low
// delay format making it suitable for high-quality, real time streaming. It's not really
// an archival format or something designed for heavy DSP post-processing since
// it's fundamentally limited to encode/decode at 48khz.
// https://mf4.xiph.org/jenkins/view/opus/job/opusfile-unix/ws/doc/html/index.html
struct OpusDecoder : public nqr::BaseDecoder
{
    OpusDecoder() {}
    virtual ~OpusDecoder() {}
    virtual int LoadFromPath(nqr::AudioData * data, const std::string & path) override;
    virtual int LoadFromBuffer(nqr::AudioData * data, const std::vector<uint8_t> & memory) override;
    virtual std::vector<std::string> GetSupportedFileExtensions() override;
};
    
} // end namespace nqr

#endif
