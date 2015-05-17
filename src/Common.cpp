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
