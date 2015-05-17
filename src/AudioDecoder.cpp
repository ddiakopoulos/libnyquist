#pragma comment(user, "license")

#include "AudioDecoder.h"

#include "WavDecoder.h"
#include "WavPackDecoder.h"
#include "FlacDecoder.h"
#include "VorbisDecoder.h"
#include "OpusDecoder.h"
#include "CafDecoder.h"

using namespace nqr;

NyquistIO::NyquistIO()
{
    BuildDecoderTable();
}

NyquistIO::~NyquistIO()
{
    
}

int NyquistIO::Load(AudioData * data, const std::string & path)
{
    if (IsFileSupported(path))
    {
        if (decoderTable.size() > 0)
        {
            auto fileExtension = ParsePathForExtension(path);
            auto decoder = GetDecoderForExtension(fileExtension);
            
            try
            {
                return decoder->LoadFromPath(data, path);
            }
            catch (std::exception e)
            {
                std::cerr << "Caught fatal exception: " << e.what() << std::endl;
            }
            
        }
        return IOError::NoDecodersLoaded;
    }
    else
    {
        return IOError::ExtensionNotSupported;
    }

    // Should never be reached
    return IOError::UnknownError;
}

bool NyquistIO::IsFileSupported(const std::string path) const
{
    auto fileExtension = ParsePathForExtension(path);
    if (decoderTable.find(fileExtension) == decoderTable.end())
    {
        return false;
    }
    else
    {
        return true;
    }
}

std::string NyquistIO::ParsePathForExtension(const std::string & path) const
{
    if (path.find_last_of(".") != std::string::npos)
        return path.substr(path.find_last_of(".") + 1);
    
    return std::string("");
}

std::shared_ptr<BaseDecoder> NyquistIO::GetDecoderForExtension(const std::string ext)
{
    return decoderTable[ext];
}

void NyquistIO::BuildDecoderTable()
{
    AddDecoderToTable(std::make_shared<WavDecoder>());
    AddDecoderToTable(std::make_shared<WavPackDecoder>());
    AddDecoderToTable(std::make_shared<FlacDecoder>());
    AddDecoderToTable(std::make_shared<VorbisDecoder>());
    AddDecoderToTable(std::make_shared<OpusDecoder>());
    AddDecoderToTable(std::make_shared<CAFDecoder>());
}