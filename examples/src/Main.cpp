#if defined(_MSC_VER)
#pragma comment(lib, "dsound.lib")
#endif

#include "libnyquist/AudioDevice.h"
#include "libnyquist/AudioDecoder.h"
#include "libnyquist/WavEncoder.h"
#include "libnyquist/PostProcess.h"

#include <thread>

using namespace nqr;

int main()
{
	AudioDevice::ListAudioDevices();

	int desiredSampleRate = 44100;
	AudioDevice myDevice(2, desiredSampleRate);
	myDevice.Open(myDevice.info.id);
	
	AudioData * fileData = new AudioData();
	
	NyquistIO loader;
    
    WavEncoder encoder;
	
	try
	{
        // Circular libnyquist testing!
        //auto result = loader.Load(fileData, "encoded.wav");
        
		//auto result = loader.Load(fileData, "test_data/1ch/44100/8/test.wav");
		//auto result = loader.Load(fileData, "test_data/1ch/44100/16/test.wav");
		//auto result = loader.Load(fileData, "test_data/1ch/44100/24/test.wav");
		//auto result = loader.Load(fileData, "test_data/1ch/44100/32/test.wav");
		//auto result = loader.Load(fileData, "test_data/1ch/44100/64/test.wav");
		
		//auto result = loader.Load(fileData, "test_data/2ch/44100/8/test.wav");
		//auto result = loader.Load(fileData, "test_data/2ch/44100/16/test.wav");
		//auto result = loader.Load(fileData, "test_data/2ch/44100/24/test.wav");
		//auto result = loader.Load(fileData, "test_data/2ch/44100/32/test.wav");
		//auto result = loader.Load(fileData, "test_data/2ch/44100/64/test.wav");
		
		//auto result = loader.Load(fileData, "test_data/ad_hoc/6_channel_44k_16b.wav");
		
		//auto result = loader.Load(fileData, "test_data/ad_hoc/LR_Stereo.ogg");
		//auto result = loader.Load(fileData, "test_data/ad_hoc/TestLaugh_44k.ogg");
		//auto result = loader.Load(fileData, "test_data/ad_hoc/TestBeat.ogg");
		//auto result = loader.Load(fileData, "test_data/ad_hoc/TestBeatMono.ogg");
		//auto result = loader.Load(fileData, "test_data/ad_hoc/BlockWoosh_Stereo.ogg");
		
		//auto result = loader.Load(fileData, "test_data/ad_hoc/KittyPurr8_Stereo_Dithered.flac");
		//auto result = loader.Load(fileData, "test_data/ad_hoc/KittyPurr16_Stereo.flac");
		//auto result = loader.Load(fileData, "test_data/ad_hoc/KittyPurr16_Mono.flac");
		//auto result = loader.Load(fileData, "test_data/ad_hoc/KittyPurr24_Stereo.flac");
		
		//auto result = loader.Load(fileData, "test_data/ad_hoc/detodos.opus"); // "Firefox: From All, To All"
		
		//auto result = loader.Load(fileData, "test_data/ad_hoc/TestBeat_Float32.wv");
		//auto result = loader.Load(fileData, "test_data/ad_hoc/TestBeat_Float32_Mono.wv");
		//auto result = loader.Load(fileData, "test_data/ad_hoc/TestBeat_Int16.wv");
		//auto result = loader.Load(fileData, "test_data/ad_hoc/TestBeat_Int24.wv");
		//auto result = loader.Load(fileData, "test_data/ad_hoc/TestBeat_Int32.wv");
		//auto result = loader.Load(fileData, "test_data/ad_hoc/TestBeat_Int24_Mono.wv");
		
        //auto result = loader.Load(fileData, "test_data/ad_hoc/44_16_stereo.mpc");
        //auto result = loader.Load(fileData, "test_data/ad_hoc/44_16_mono.mpc");
        
        //Block-split-stereo-ima4-reaper.wav
        //auto result = loader.Load(fileData, "test_data/ad_hoc/TestBeat_44_16_mono-ima4-reaper.wav");
         loader.Load(fileData, "test_data/ad_hoc/TestBeat_44_16_stereo-ima4-reaper.wav");
	}
    catch(const UnsupportedExtensionEx & e)
    {
        std::cerr << "Caught: " << e.what() << std::endl;
    }
    catch(const LoadPathNotImplEx & e)
    {
       std::cerr << "Caught: " << e.what() << std::endl;
    }
    catch(const LoadBufferNotImplEx & e)
    {
        std::cerr << "Caught: " << e.what() << std::endl;
    }
	catch (const std::exception & e)
	{
		std::cerr << "Caught: " << e.what() << std::endl;
		std::exit(1);
	}

	// Libnyquist does not do sample rate conversion
	if (fileData->sampleRate != desiredSampleRate)
	{
		std::cout << "[Warning - Sample Rate Mismatch] - file is sampled at " << fileData->sampleRate << " and output is " << desiredSampleRate << std::endl;
	}
	
	// Convert mono to stereo for testing playback
	if (fileData->channelCount == 1)
	{
        std::cout << "Playing MONO for: " << fileData->lengthSeconds << " seconds..." << std::endl;
		std::vector<float> stereoCopy(fileData->samples.size() * 2);
        MonoToStereo(fileData->samples.data(), stereoCopy.data(), fileData->samples.size());
		myDevice.Play(stereoCopy);
	}
	else
	{
		std::cout << "Playing for: " << fileData->lengthSeconds << " seconds..." << std::endl;
		myDevice.Play(fileData->samples);
	}
    
    // Test wav file encoder
    int encoderStatus = encoder.WriteFile({2, PCM_16, DITHER_NONE}, fileData, "encoded.wav");
    std::cout << "Encoder Status: " << encoderStatus << std::endl;
    
	return 0;
}