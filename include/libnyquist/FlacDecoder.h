#pragma comment(user, "license")

#ifndef FLAC_DECODER_H
#define FLAC_DECODER_H

#include "AudioDecoder.h"
#include <map>

namespace nqr
{
    
//@todo expose this in API
inline std::map<int, std::string> GetQualityTable()
{
     return {
             { 0, "0 (Fastest)" },
             { 1, "1" },
             { 2, "2" },
             { 3, "3" },
             { 4, "4" },
             { 5, "5 (Default)" },
             { 6, "6" },
             { 7, "7" },
             { 8, "8 (Highest)" },
         };
 }

struct FlacDecoder : public nqr::BaseDecoder
{
    FlacDecoder() {}
    virtual ~FlacDecoder() {}
    virtual int LoadFromPath(nqr::AudioData * data, const std::string & path) override;
    virtual int LoadFromBuffer(nqr::AudioData * data, const std::vector<uint8_t> & memory) override;
    virtual std::vector<std::string> GetSupportedFileExtensions() override;
};

} // end namespace nqr

#endif
