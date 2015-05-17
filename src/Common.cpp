#include "Common.h"

using namespace nqr;

ChunkHeaderInfo nqr::ScanForChunk(const std::vector<uint8_t> & fileData, uint32_t chunkMarker)
{
    // D[n] aligned to 16 bytes now
    const uint16_t * d = reinterpret_cast<const uint16_t *>(fileData.data());
    
    for (size_t i = 0; i < fileData.size() / sizeof(uint16_t); i++)
    {
        // This will be in machine endianess
        uint32_t m = Pack(Read16(d[i]), Read16(d[i+1]));
        
        if (m == chunkMarker)
        {
            uint32_t cSz = Pack(Read16(d[i+2]), Read16(d[i+3]));
            return {(uint32_t (i * sizeof(uint16_t))), cSz}; // return i in bytes to the start of the data
        }
        else continue;
    }
    return {0, 0};
};

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
