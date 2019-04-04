/*
Copyright (c) 2019, Dimitri Diakopoulos All rights reserved.

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

#include "AudioDevice.h"
#include <cmath>
#include <algorithm>
#include <thread>
#include <chrono>
#include <iostream>

static RingBufferT<float> buffer(BUFFER_LENGTH);
static RingBufferT<float> record_buffer(BUFFER_LENGTH / 2);

static int rt_callback(void * output_buffer, void * input_buffer, uint32_t num_bufferframes, double stream_time, RtAudioStreamStatus status, void * user_data)
{
    if (status) std::cerr << "[rtaudio] buffer over or underflow" << std::endl;

    // Playback
    if (buffer.getAvailableRead()) buffer.read((float*) output_buffer, BUFFER_LENGTH);
    else memset(output_buffer, 0, BUFFER_LENGTH * sizeof(float));

    // Recording
    if (record_buffer.getAvailableWrite()) record_buffer.write((float*) input_buffer, BUFFER_LENGTH / 2);

    return 0;
}

AudioDevice::AudioDevice(int numChannels, int sampleRate, int deviceId)
{
    rtaudio = std::unique_ptr<RtAudio>(new RtAudio);
    info.id = (deviceId != -1) ? deviceId : rtaudio->getDefaultOutputDevice();
    info.numChannels = numChannels;
    info.sampleRate = sampleRate;
    info.frameSize = FRAME_SIZE;
}
    
AudioDevice::~AudioDevice()
{
    if (rtaudio) 
    {
        rtaudio->stopStream();
        if (rtaudio->isStreamOpen())
        {
            rtaudio->closeStream();
        }
    }
}

bool AudioDevice::Open(const int deviceId)
{
    if (!rtaudio) throw std::runtime_error("rtaudio not created yet");

    RtAudio::StreamParameters outputParams;
    outputParams.deviceId = info.id;
    outputParams.nChannels = info.numChannels;
    outputParams.firstChannel = 0;

    RtAudio::StreamParameters inputParams;
    inputParams.deviceId = rtaudio->getDefaultInputDevice();
    inputParams.nChannels = 1;
    inputParams.firstChannel = 0;

    rtaudio->openStream(&outputParams, &inputParams, RTAUDIO_FLOAT32, info.sampleRate, &info.frameSize, &rt_callback, (void*) & buffer);

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
    uint32_t devices = tempDevice->getDeviceCount();

    std::cout << "[rtaudio] Found: " << devices << " device(s)\n";

    for (uint32_t i = 0; i < devices; ++i)
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
    
    while (writeCount < sizeInFrames)
    {
        bool status = buffer.write((data.data() + (writeCount * BUFFER_LENGTH)), BUFFER_LENGTH);
        if (status) writeCount++;
    }

    return true;
}

bool AudioDevice::Record(const uint32_t lengthInSamples, std::vector<float> & recordingBuffer)
{
    uint64_t recordedSamples = 0;

    // Allocate memory upfront (revisit this later...)
    recordingBuffer.resize(lengthInSamples + (BUFFER_LENGTH)); // + a little padding

    while (recordedSamples < lengthInSamples)
    {
        if (record_buffer.getAvailableRead())
        {
            if (record_buffer.read(recordingBuffer.data() + recordedSamples, BUFFER_LENGTH / 2))
            {
                recordedSamples += (BUFFER_LENGTH / 2);
            }
        }
    }

    return true;
}
