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

#include <fstream>
#include "WavEncoder.h"

using namespace nqr;

static inline void to_bytes(uint8_t value, char * arr)
{
    arr[0] = (value) & 0xFF;
}

static inline void to_bytes(uint16_t value, char * arr)
{
    arr[0] = (value) & 0xFF;
    arr[1] = (value >> 8) & 0xFF;
}

static inline void to_bytes(uint32_t value, char * arr)
{
    arr[0] = (value) & 0xFF;
    arr[1] = (value >> 8) & 0xFF;
    arr[2] = (value >> 16) & 0xFF;
    arr[3] = (value >> 24) & 0xFF;
}

////////////////////////////
//   Wave File Encoding   //
////////////////////////////

int WavEncoder::WriteFile(const EncoderParams p, const AudioData * d, const std::string & path)
{
    assert(d->samples.size() > 0);

    // Cast away const because we know what we are doing (Hopefully?)
    float * sampleData = const_cast<float *>(d->samples.data());
    size_t sampleDataSize = d->samples.size();
    
    std::vector<float> sampleDataOptionalMix;
    
    if (sampleDataSize <= 32)
    {
        return EncoderError::InsufficientSampleData;
    }
    
    if (d->channelCount < 1 || d->channelCount > 8)
    {
        return EncoderError::UnsupportedChannelConfiguration;
    }
    
    // Handle Channel Mixing --
    
    // Mono => Stereo
    if (d->channelCount == 1 && p.channelCount == 2)
    {
        sampleDataOptionalMix.resize(sampleDataSize * 2);
        MonoToStereo(sampleData, sampleDataOptionalMix.data(), sampleDataSize); // Mix
        
        // Re-point data
        sampleData = sampleDataOptionalMix.data();
        sampleDataSize = sampleDataOptionalMix.size();
    }
    
    // Stereo => Mono
    else if (d->channelCount == 2 && p.channelCount == 1)
    {
        sampleDataOptionalMix.resize(sampleDataSize / 2);
        StereoToMono(sampleData, sampleDataOptionalMix.data(), sampleDataSize); // Mix
        
        // Re-point data
        sampleData = sampleDataOptionalMix.data();
        sampleDataSize = sampleDataOptionalMix.size();
        
    }
    
    else if (d->channelCount == p.channelCount)
    {
        // No op
    }
    
    else
    {
        return EncoderError::UnsupportedChannelMix;
    }
    // -- End Channel Mixing
    
    auto maxFileSizeInBytes = std::numeric_limits<uint32_t>::max();
    auto samplesSizeInBytes = (sampleDataSize * GetFormatBitsPerSample(p.targetFormat)) / 8;
    
    // 64 arbitrary
    if ((samplesSizeInBytes - 64) >= maxFileSizeInBytes)
    {
        return EncoderError::BufferTooBig;
    }
    
    // Don't support PC64 or PCDBL
    if (GetFormatBitsPerSample(p.targetFormat) > 32)
    {
        return EncoderError::UnsupportedBitdepth;
    }
    
    std::ofstream fout(path.c_str(), std::ios::out | std::ios::binary);
    
    if (!fout.is_open())
    {
        return EncoderError::FileIOError;
    }
    
    char * chunkSizeBuff = new char[4];
    
    // Initial size (this is changed after we're done writing the file)
    to_bytes(uint32_t(36), chunkSizeBuff);

    // RIFF file header
    fout.write(GenerateChunkCodeChar('R', 'I', 'F', 'F'), 4);
    fout.write(chunkSizeBuff, 4);
    
    fout.write(GenerateChunkCodeChar('W', 'A', 'V', 'E'), 4);
    
    // Fmt header
    auto header = MakeWaveHeader(p, d->sampleRate);
    fout.write(reinterpret_cast<char*>(&header), sizeof(WaveChunkHeader));
    
    auto sourceBits = GetFormatBitsPerSample(d->sourceFormat);
    auto targetBits = GetFormatBitsPerSample(p.targetFormat);
    
    ////////////////////////////
    //@todo - channel mixing! //
    ////////////////////////////
    
    // Write out fact chunk
    if (p.targetFormat == PCM_FLT)
    {
        uint32_t four = 4;
        uint32_t dataSz = int(sampleDataSize / p.channelCount);
        fout.write(GenerateChunkCodeChar('f', 'a', 'c', 't'), 4);
        fout.write(reinterpret_cast<const char *>(&four), 4);
        fout.write(reinterpret_cast<const char *>(&dataSz), 4); // Number of samples (per channel)
    }
    
    // Data header
    fout.write(GenerateChunkCodeChar('d', 'a', 't', 'a'), 4);
    
    // + data chunk size
    to_bytes(uint32_t(samplesSizeInBytes), chunkSizeBuff);
    fout.write(chunkSizeBuff, 4);
    
    if (targetBits <= sourceBits && p.targetFormat != PCM_FLT)
    {
        // At most need this number of bytes in our copy
        std::vector<uint8_t> samplesCopy(samplesSizeInBytes);
        ConvertFromFloat32(samplesCopy.data(), sampleData, sampleDataSize, p.targetFormat, p.dither);
        fout.write(reinterpret_cast<const char*>(samplesCopy.data()), samplesSizeInBytes);
    }
    else
    {
        // Handle PCFLT. Coming in from AudioData ensures we start with 32 bits, so we're fine
        // since we've also rejected formats with more than 32 bits above.
        fout.write(reinterpret_cast<const char*>(sampleData), samplesSizeInBytes);
    }
    
    // Padding byte
    if (isOdd(samplesSizeInBytes))
    {
        const char * zero = "";
        fout.write(zero, 1);
    }
    
    // Find size
    long totalSize = fout.tellp();
    
    // Modify RIFF header
    fout.seekp(4);
    
    // Total size of the file, less 8 bytes for the RIFF header
    to_bytes(uint32_t(totalSize - 8), chunkSizeBuff);
    
    fout.write(chunkSizeBuff, 4);

    delete[] chunkSizeBuff;
    
    return EncoderError::NoError;
}

////////////////////////////
//   Opus File Encoding   //
////////////////////////////

#include "opus/opusfile/include/opusfile.h"
#include "ogg/ogg.h"

typedef std::pair<std::string, std::string> metadata_type;

class OggWriter
{
    void write_to_ostream(bool flush)
    {
        while (ogg_stream_pageout(&oss, &page))
        {
            ostream->write(reinterpret_cast<char*>(page.header), static_cast<std::streamsize>(page.header_len));
            ostream->write(reinterpret_cast<char*>(page.body), static_cast<std::streamsize>(page.body_len));
        }

        if (flush && ogg_stream_flush(&oss, &page))
        {
            ostream->write(reinterpret_cast<char*>(page.header), static_cast<std::streamsize>(page.header_len));
            ostream->write(reinterpret_cast<char*>(page.body), static_cast<std::streamsize>(page.body_len));
        }
    }

    std::vector<char> make_header(int channel_count, int preskip, long sample_rate, int gain)
    {
        std::vector<char> header;
 
        std::array<char, 9> streacount = {{ 0x0, 0x1, 0x1, 0x2, 0x2, 0x3, 0x4, 0x5, 0x5 }};
        std::array<char, 9> coupled_streacount = {{ 0x0, 0x0, 0x1, 0x1, 0x2, 0x2, 0x2, 0x2, 0x3 }};
    
        std::array<char, 1> chan_map_1 = {{ 0x0 }};
        std::array<char, 2> chan_map_2 = {{ 0x0, 0x1 }};
        std::array<char, 3> chan_map_3 = {{ 0x0, 0x2, 0x1 }};
        std::array<char, 4> chan_map_4 = {{ 0x0, 0x1, 0x2, 0x3 }};
        std::array<char, 5> chan_map_5 = {{ 0x0, 0x4, 0x1, 0x2, 0x3 }};
        std::array<char, 6> chan_map_6 = {{ 0x0, 0x4, 0x1, 0x2, 0x3, 0x5 }};
        std::array<char, 7> chan_map_7 = {{ 0x0, 0x4, 0x1, 0x2, 0x3, 0x5, 0x6 }};
        std::array<char, 8> chan_map_8 = {{ 0x0, 0x6, 0x1, 0x2, 0x3, 0x4, 0x5, 0x7 }};

        std::array<char, 9> _preamble = {{ 'O', 'p', 'u', 's', 'H', 'e', 'a', 'd', 0x1 }};
        std::array<char, 1> _channel_count;
        std::array<char, 2> _preskip;
        std::array<char, 4> _sample_rate;
        std::array<char, 2> _gain;
        std::array<char, 1> _channel_family = {{ 0x1 }};

        to_bytes(uint8_t(channel_count), _channel_count.data());
        to_bytes(uint16_t(preskip), _channel_count.data());
        to_bytes(uint32_t(sample_rate), _channel_count.data());
        to_bytes(uint16_t(gain), _channel_count.data());

        header.insert(header.end(), _preamble.cbegin(), _preamble.cend());
        header.insert(header.end(), _channel_count.cbegin(), _channel_count.cend());
        header.insert(header.end(), _preskip.cbegin(), _preskip.cend());
        header.insert(header.end(), _sample_rate.cbegin(), _sample_rate.cend());
        header.insert(header.end(), _gain.cbegin(), _gain.cend());
        header.insert(header.end(), _channel_family.cbegin(), _channel_family.cend());
        header.push_back(streacount[channel_count]);
        header.push_back(coupled_streacount[channel_count]);

        switch (channel_count)
        {
            case 1: header.insert(header.end(), chan_map_1.cbegin(), chan_map_1.cend()); break;
            case 2: header.insert(header.end(), chan_map_2.cbegin(), chan_map_2.cend()); break;
            case 3: header.insert(header.end(), chan_map_3.cbegin(), chan_map_3.cend()); break;
            case 4: header.insert(header.end(), chan_map_4.cbegin(), chan_map_4.cend()); break;
            case 5: header.insert(header.end(), chan_map_5.cbegin(), chan_map_5.cend()); break;
            case 6: header.insert(header.end(), chan_map_6.cbegin(), chan_map_6.cend()); break;
            case 7: header.insert(header.end(), chan_map_7.cbegin(), chan_map_7.cend()); break;
            case 8: header.insert(header.end(), chan_map_8.cbegin(), chan_map_8.cend()); break;
            default: throw std::runtime_error("couldn't map channel data");
        }
    
        return header;
    }
                        
    std::vector<char> make_tags(const std::string & vendor, const std::vector<metadata_type> & metadata)
    {
        std::vector<char> tags;

        std::array<char, 8> _preamble = {{ 'O', 'p', 'u', 's', 'T', 'a', 'g', 's' }};
        std::array<char, 4> _vendor_length;
        std::array<char, 4> _metadata_count;

        to_bytes(uint32_t(vendor.size()), _vendor_length.data());
        to_bytes(uint32_t(metadata.size()), _metadata_count.data());

        tags.insert(tags.end(), _preamble.cbegin()       , _preamble.cend());
        tags.insert(tags.end(), _vendor_length.cbegin()  , _vendor_length.cend());
        tags.insert(tags.end(), vendor.cbegin()          , vendor.cend());
        tags.insert(tags.end(), _metadata_count.cbegin() , _metadata_count.cend());

        // Process metadata.
        for (const auto & metadata_entry : metadata)
        {
            std::array<char, 4> _metadata_entry_length;
            std::string entry = metadata_entry.first + "=" + metadata_entry.second;
            to_bytes(uint32_t(entry.size()), _metadata_entry_length.data());
            tags.insert(tags.end(), _metadata_entry_length.cbegin(), _metadata_entry_length.cend());
            tags.insert(tags.end(), entry.cbegin() , entry.cend());
        }

        return tags;
    }

    ogg_int64_t        packet_number;
    ogg_int64_t        granule;
    ogg_page           page;
    ogg_stream_state   oss;

    int                        channel_count;
    long                       sample_rate;
    int                        bits_per_sample;
    std::ofstream *            ostream;
    std::string                description;
    std::vector<metadata_type> metadata;

public:

    OggWriter(int channel_count, long sample_rate, int bits_per_sample, std::ofstream & stream, const std::vector<metadata_type> & md)
    {
        channel_count   = channel_count;
        sample_rate     = sample_rate;
        bits_per_sample = bits_per_sample;
        ostream         = &stream;
        metadata        = md;

        int          oss_init_error;
        int          packet_write_error;
        ogg_packet   header_packet;
        ogg_packet   tags_packet;
        std::vector<char> header_vector;
        std::vector<char> tags_vector;
    
        description   = "Ogg";
        packet_number = 0;
        granule       = 0;

        // Validate parameters  
        if (channel_count < 1 && channel_count > 255) throw std::runtime_error("Channel count must be between 1 and 255.");

        // Initialize the Ogg stream.
        oss_init_error = ogg_stream_init(&oss, 12345);
        if (oss_init_error) throw std::runtime_error("Could not initialize the Ogg stream state.");

        // Initialize the header packet.
        header_vector			 = make_header(channel_count, 3840, sample_rate, 0);
        header_packet.packet     = reinterpret_cast<unsigned char*>(header_vector.data());
        header_packet.bytes      = header_vector.size();
        header_packet.b_o_s      = 1;
        header_packet.e_o_s      = 0;
        header_packet.granulepos = 0;
        header_packet.packetno   = packet_number++;

        // Initialize tags
        tags_vector			   = make_tags("libnyquist", metadata);
        tags_packet.packet     = reinterpret_cast<unsigned char*>(tags_vector.data());
        tags_packet.bytes      = tags_vector.size();
        tags_packet.b_o_s      = 0;
        tags_packet.e_o_s      = 0;
        tags_packet.granulepos = 0;
        tags_packet.packetno   = packet_number++;
    
        // Write header page
        packet_write_error = ogg_stream_packetin(&oss, &header_packet);
        if (packet_write_error) throw std::runtime_error("Could not write header packet to the Ogg stream.");
        write_to_ostream(true);
    
        // Write tags page
        packet_write_error = ogg_stream_packetin(&oss, &tags_packet);
        if (packet_write_error) throw std::runtime_error("Could not write tags packet to the Ogg stream.");
        write_to_ostream(true);
    }

    ~OggWriter()
    {
        write_to_ostream(true);
        ogg_stream_clear(&oss);
    }

    bool write(char * data, std::streamsize length, size_t sample_count, bool end);

};

// Opus only supports a 48k samplerate...
int OggOpusEncoder::WriteFile(const EncoderParams p, const AudioData * d, const std::string & path)
{
    assert(d->samples.size() > 0);

    OpusEncoder * enc;
    enc = opus_encoder_create(48000, 1, OPUS_APPLICATION_AUDIO, nullptr);

    // How big should this be? 
    std::vector<uint8_t> encodedBuffer(1024);

    auto encoded_size = opus_encode_float(enc, d->samples.data(), d->frameSize, encodedBuffer.data(), encodedBuffer.size());

    // Cast away const because we know what we are doing (Hopefully?)
    float * sampleData = const_cast<float *>(d->samples.data());
    const size_t sampleDataSize = d->samples.size();

    return EncoderError::NoError;
}
