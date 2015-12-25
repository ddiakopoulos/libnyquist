/*
Copyright (c) 2015, Dimitri Diakopoulos All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "AudioDecoder.h"

#include "WavDecoder.h"
#include "WavPackDecoder.h"
#include "FlacDecoder.h"
#include "VorbisDecoder.h"
#include "OpusDecoder.h"
#include "CafDecoder.h"
#include "MusepackDecoder.h"

using namespace nqr;

NyquistIO::NyquistIO()
{
    BuildDecoderTable();
}

NyquistIO::~NyquistIO() { }

void NyquistIO::Load(AudioData * data, const std::string & path)
{
    if (IsFileSupported(path))
    {
        if (decoderTable.size())
        {
            auto fileExtension = ParsePathForExtension(path);
            auto decoder = GetDecoderForExtension(fileExtension);
            
            try
            {
                decoder->LoadFromPath(data, path);
            }
            catch (const std::exception & e)
            {
                std::cerr << "Caught internal exception: " << e.what() << std::endl;
            }
            
        }
        else throw std::runtime_error("No available decoders.");
    }
    else
    {
        throw UnsupportedExtensionEx();
    }
}

void NyquistIO::Load(AudioData * data, std::string extension, const std::vector<uint8_t> & buffer)
{
    if (decoderTable.find(extension) == decoderTable.end())
    {
        throw UnsupportedExtensionEx();
    }

    if (decoderTable.size())
    {
        auto decoder = GetDecoderForExtension(extension);  
        try
        {
            decoder->LoadFromBuffer(data, buffer);
        }
        catch (const std::exception & e)
        {
            std::cerr << "Caught internal exception: " << e.what() << std::endl;
        }
    } 
    else 
    {
        throw std::runtime_error("No available decoders.");
    }

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
    if (decoderTable.size())
    {
        return decoderTable[ext];
    }
    else throw std::runtime_error("No available decoders.");
    return nullptr;
}

void NyquistIO::AddDecoderToTable(std::shared_ptr<nqr::BaseDecoder> decoder)
{
    auto supportedExtensions = decoder->GetSupportedFileExtensions();
    
    for (const auto ext : supportedExtensions)
    {
        if (decoderTable.count(ext) >= 1) throw std::runtime_error("decoder already exists for extension.");
        decoderTable.insert(DecoderPair(ext, decoder));
    }
}

void NyquistIO::BuildDecoderTable()
{
    AddDecoderToTable(std::make_shared<WavDecoder>());
    AddDecoderToTable(std::make_shared<WavPackDecoder>());
    AddDecoderToTable(std::make_shared<FlacDecoder>());
    AddDecoderToTable(std::make_shared<VorbisDecoder>());
    AddDecoderToTable(std::make_shared<OpusDecoder>());
    AddDecoderToTable(std::make_shared<CAFDecoder>());
    AddDecoderToTable(std::make_shared<MusepackDecoder>());
}