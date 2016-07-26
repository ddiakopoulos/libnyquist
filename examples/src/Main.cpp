// Note to Visual Studio / Windows users: you must set the working directory manually on the project file
// to $(ProjectDir)../../../ since these settings are not saved directly in project. The loader
// will be unable to find the example assets unless the proper working directory is set.

#if defined(_MSC_VER)
#pragma comment(lib, "dsound.lib")
#endif

#include "libnyquist/AudioDevice.h"
#include "libnyquist/AudioDecoder.h"
#include "libnyquist/WavEncoder.h"
#include "libnyquist/PostProcess.h"

#include <thread>

using namespace nqr;

int main(int argc, const char **argv) try
{
	AudioDevice::ListAudioDevices();

	int desiredSampleRate = 44100;
	AudioDevice myDevice(2, desiredSampleRate);
	myDevice.Open(myDevice.info.id);

	std::shared_ptr<AudioData> fileData = std::make_shared<AudioData>();

	NyquistIO loader;

	if (argc > 1)
	{
		std::string cli_arg = std::string(argv[1]);
		loader.Load(fileData.get(), cli_arg);
	}
	else
	{
		// Circular libnyquist testing
		//loader.Load(fileData.get(), "encoded.opus");

		// 1-channel wave
		//loader.Load(fileData.get(), "test_data/1ch/44100/8/test.wav");
		//loader.Load(fileData.get(), "test_data/1ch/44100/16/test.wav");
		//loader.Load(fileData.get(), "test_data/1ch/44100/24/test.wav");
		//loader.Load(fileData.get(), "test_data/1ch/44100/32/test.wav");
		//loader.Load(fileData.get(), "test_data/1ch/44100/64/test.wav");

		// 2-channel wave
		//loader.Load(fileData.get(), "test_data/2ch/44100/8/test.wav");
		//loader.Load(fileData.get(), "test_data/2ch/44100/16/test.wav");
		//loader.Load(fileData.get(), "test_data/2ch/44100/24/test.wav");
		//loader.Load(fileData.get(), "test_data/2ch/44100/32/test.wav");
		//loader.Load(fileData.get(), "test_data/2ch/44100/64/test.wav");

		//loader.Load(fileData.get(), "test_data/ad_hoc/TestBeat_44_16_mono-ima4-reaper.wav");
		//loader.Load(fileData.get(), "test_data/ad_hoc/TestBeat_44_16_stereo-ima4-reaper.wav");

		// Multi-channel wave
		//loader.Load(fileData.get(), "test_data/ad_hoc/6_channel_44k_16b.wav");

		// 1 + 2 channel ogg
		//loader.Load(fileData.get(), "test_data/ad_hoc/LR_Stereo.ogg");
		//loader.Load(fileData.get(), "test_data/ad_hoc/TestLaugh_44k.ogg");
		//loader.Load(fileData.get(), "test_data/ad_hoc/TestBeat.ogg");
		//loader.Load(fileData.get(), "test_data/ad_hoc/TestBeatMono.ogg");
		//loader.Load(fileData.get(), "test_data/ad_hoc/BlockWoosh_Stereo.ogg");

		// 1 + 2 channel flac
		//loader.Load(fileData.get(), "test_data/ad_hoc/KittyPurr8_Stereo_Dithered.flac");
		//loader.Load(fileData.get(), "test_data/ad_hoc/KittyPurr16_Stereo.flac");
		//loader.Load(fileData.get(), "test_data/ad_hoc/KittyPurr16_Mono.flac");
		//loader.Load(fileData.get(), "test_data/ad_hoc/KittyPurr24_Stereo.flac");

		// Single-channel opus
		//loader.Load(fileData.get(), "test_data/ad_hoc/detodos.opus"); // "Firefox: From All, To All"

		// 1 + 2 channel wavepack
		//loader.Load(fileData.get(), "test_data/ad_hoc/TestBeat_Float32.wv");
		//loader.Load(fileData.get(), "test_data/ad_hoc/TestBeat_Float32_Mono.wv");
		//loader.Load(fileData.get(), "test_data/ad_hoc/TestBeat_Int16.wv");
		//loader.Load(fileData.get(), "test_data/ad_hoc/TestBeat_Int24.wv");
		//loader.Load(fileData.get(), "test_data/ad_hoc/TestBeat_Int32.wv");
		//loader.Load(fileData.get(), "test_data/ad_hoc/TestBeat_Int24_Mono.wv");

		// 1 + 2 channel musepack
		//loader.Load(fileData.get(), "test_data/ad_hoc/44_16_stereo.mpc");
		//loader.Load(fileData.get(), "test_data/ad_hoc/44_16_mono.mpc");
	}

	// Test Recording Capabilities of AudioDevice
	fileData->samples.reserve(44100 * 2);
	fileData->channelCount = 1;
	fileData->frameSize = 32;
	fileData->lengthSeconds = 2.0;
	fileData->sampleRate = 44100;
	std::cout << "Starting recording for two seconds..." << std::endl;
	myDevice.Record(fileData->sampleRate * fileData->lengthSeconds, fileData->samples);

	// Libnyquist does not (currently) perform sample rate conversion - not exactly true, anymore. See below.
	if (fileData->sampleRate != desiredSampleRate)
	{
		std::cout << "[Warning - Sample Rate Mismatch] - file is sampled at " << fileData->sampleRate << " and output is " << desiredSampleRate << std::endl;
	}

	// Resample
	std::vector<float> outputBuffer;
	outputBuffer.reserve(fileData->samples.size());
	linear_resample(44100.0 / 48000.0, fileData->samples, outputBuffer, fileData->samples.size());

	std::cout << "Input Samples: " << fileData->samples.size() << std::endl;
	std::cout << "Output Samples: " << outputBuffer.size() << std::endl;

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

	fileData->samples = outputBuffer;
	int encoderStatus = OggOpusEncoder::WriteFile({ 1, PCM_FLT, DITHER_NONE }, fileData.get(), "encoded.opus");
	std::cout << "Encoder Status: " << encoderStatus << std::endl;

	return EXIT_SUCCESS;
}
catch (const UnsupportedExtensionEx & e)
{
	std::cerr << "Caught: " << e.what() << std::endl;
}
catch (const LoadPathNotImplEx & e)
{
	std::cerr << "Caught: " << e.what() << std::endl;
}
catch (const LoadBufferNotImplEx & e)
{
	std::cerr << "Caught: " << e.what() << std::endl;
}
catch (const std::exception & e)
{
	std::cerr << "Caught: " << e.what() << std::endl;
}