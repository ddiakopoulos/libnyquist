#pragma comment(user, "license")

#include "CafDecoder.h"

using namespace nqr;

//////////////////////
// Public Interface //
//////////////////////

int CAFDecoder::LoadFromPath(AudioData * data, const std::string & path)
{
    return IOError::LoadPathNotImplemented;
}

int CAFDecoder::LoadFromBuffer(AudioData * data, const std::vector<uint8_t> & memory)
{
    return IOError::LoadBufferNotImplemented;
}

std::vector<std::string> CAFDecoder::GetSupportedFileExtensions()
{
    return {};
}