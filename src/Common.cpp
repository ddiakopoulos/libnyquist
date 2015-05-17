#include "Common.h"

using namespace nqr;

NyquistFileBuffer nqr::ReadFile(std::string pathToFile)
{
	std::cout << "[Debug] Open: " << pathToFile << std::endl;
    
    FILE * audioFile = fopen(pathToFile.c_str(), "rb");
    
    if (!audioFile)
    {
        throw std::runtime_error("file not found");
    }
    
    fseek(audioFile, 0, SEEK_END);
    size_t lengthInBytes = ftell(audioFile);
    fseek(audioFile, 0, SEEK_SET);
    
    // Allocate temporary buffer
    std::vector<uint8_t> fileBuffer(lengthInBytes);
    
    size_t elementsRead = fread(fileBuffer.data(), 1, lengthInBytes, audioFile);
    
    if (elementsRead == 0 || fileBuffer.size() < 64)
    {
        throw std::runtime_error("error reading file or file too small");
    }
   
	NyquistFileBuffer data = {std::move(fileBuffer), elementsRead};

	fclose(audioFile);

	// Copy out to user 
	return data;
}

// Src data is aligned to PCMFormat
// @todo normalize?
void nqr::ConvertToFloat32(float * dst, const uint8_t * src, const size_t N, PCMFormat f)
{
    assert(f != PCM_END);
    
    if (f == PCM_U8)
    {
        const uint8_t * dataPtr = reinterpret_cast<const uint8_t *>(src);
        for (size_t i = 0; i < N; ++i)
            dst[i] = uint8_to_float32(dataPtr[i]);
    }
    else if (f == PCM_S8)
    {
        const int8_t * dataPtr = reinterpret_cast<const int8_t *>(src);
        for (size_t i = 0; i < N; ++i)
            dst[i] = int8_to_float32(dataPtr[i]);
    }
    else if (f == PCM_16)
    {
        const int16_t * dataPtr = reinterpret_cast<const int16_t *>(src);
        for (size_t i = 0; i < N; ++i)
            dst[i] = int16_to_float32(Read16(dataPtr[i]));
    }
    else if (f == PCM_24)
    {
        const uint8_t * dataPtr = reinterpret_cast<const uint8_t *>(src);
        size_t c = 0;
        for (size_t i = 0; i < N; ++i)
        {
            int32_t sample = Pack(dataPtr[c], dataPtr[c+1], dataPtr[c+2]);
            dst[i] = int24_to_float32(sample); // Packed types don't need addtional endian helpers
            c += 3;
        }
    }
    else if (f == PCM_32)
    {
        const int32_t * dataPtr = reinterpret_cast<const int32_t *>(src);
        for (size_t i = 0; i < N; ++i)
            dst[i] = int32_to_float32(Read32(dataPtr[i]));
    }
    
    //@todo add int64 format
    
    else if (f == PCM_FLT)
    {
        const float * dataPtr = reinterpret_cast<const float *>(src);
        for (size_t i = 0; i < N; ++i)
            dst[i] = (float) Read32(dataPtr[i]);
    }
    else if (f == PCM_DBL)
    {
        const double * dataPtr = reinterpret_cast<const double *>(src);
        for (size_t i = 0; i < N; ++i)
            dst[i] = (float) Read64(dataPtr[i]);
    }
}

// Src data is always aligned to 4 bytes (WavPack, primarily)
void nqr::ConvertToFloat32(float * dst, const int32_t * src, const size_t N, PCMFormat f)
{
    assert(f != PCM_END);
    
    if (f == PCM_16)
    {
        for (size_t i = 0; i < N; ++i)
            dst[i] = int16_to_float32(Read32(src[i]));
    }
    else if (f == PCM_24)
    {
        const uint8_t * dataPtr = reinterpret_cast<const uint8_t *>(src);
        size_t c = 0;
        for (size_t i = 0; i < N; ++i)
        {
            int32_t sample = Pack(dataPtr[c], dataPtr[c+1], dataPtr[c+2]);
            dst[i] = int24_to_float32(sample);
            c += 4; // +4 for next 4 byte boundary
        }
    }
    else if (f == PCM_32)
    {
        for (size_t i = 0; i < N; ++i)
            dst[i] = int32_to_float32(Read32(src[i]));
    }
}
