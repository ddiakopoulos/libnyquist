#pragma comment(user, "license")

#include "AudioDevice.h"
#include <cmath>
#include <algorithm>
#include <thread>
#include <chrono>

using namespace nqr;

static RingBufferT<float> buffer(BUFFER_LENGTH);

static int rt_callback(void * output_buffer, void * input_buffer, unsigned int num_bufferframes, double stream_time, RtAudioStreamStatus status, void * user_data)
{
	if (status) std::cerr << "[rtaudio] Buffer over or underflow" << std::endl;

	if (buffer.getAvailableRead()) 
	{
		buffer.read((float*) output_buffer, BUFFER_LENGTH);
	} 
	else
	{
		memset(output_buffer, 0, BUFFER_LENGTH * sizeof(float));
	}

	return 0;
}

bool AudioDevice::Open(const int deviceId)
{
	if (!rtaudio) throw std::runtime_error("rtaudio not created yet");

	RtAudio::StreamParameters parameters;
	parameters.deviceId = info.id;
	parameters.nChannels = info.numChannels;
	parameters.firstChannel = 0;

	rtaudio->openStream(&parameters, NULL, RTAUDIO_FLOAT32, info.sampleRate, &info.frameSize, &rt_callback, (void*) & buffer);

	if (rtaudio->isStreamOpen()) 
	{
		rtaudio->startStream();
		return true;
	}
	return false;
}

void AudioDevice::ListAudioDevices()
{
	std::unique_ptr<RtAudio> tempDevice(new RtAudio);

	RtAudio::DeviceInfo info;
	unsigned int devices = tempDevice->getDeviceCount();

	std::cout << "[rtaudio] Found: " << devices << " device(s)\n";

	for (unsigned int i = 0; i < devices; ++i)
	{
		info = tempDevice->getDeviceInfo(i);
		std::cout << "\tDevice: " << i << " - " << info.name << std::endl;
	}
	std::cout << std::endl;
}

bool AudioDevice::Play(const std::vector<float> & data)
{
	if (!rtaudio->isStreamOpen()) return false;
	
	// Each frame is the (size/2) cause interleaved channels! 
	int sizeInFrames = ((int) data.size()) / (BUFFER_LENGTH);
    
	int writeCount = 0;
    
	while(writeCount < sizeInFrames)
	{
		bool status = buffer.write((data.data() + (writeCount * BUFFER_LENGTH)), BUFFER_LENGTH);
		if (status) writeCount++;
	}

	return true;
}