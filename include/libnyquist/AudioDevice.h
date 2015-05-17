#pragma comment(user, "license")

#ifndef AUDIO_DEVICE_H
#define AUDIO_DEVICE_H

// This file implements a simple sound file player based on RtAudio for testing / example purposes.

#include "Common.h"
#include "RingBuffer.h"

#include "rtaudio/RtAudio.h"

#include <iostream>
#include <memory>

namespace nqr
{
    
const unsigned int FRAME_SIZE = 512;
const int CHANNELS = 2;
const int BUFFER_LENGTH = FRAME_SIZE * CHANNELS;

struct DeviceInfo
{
    int id;
    int numChannels;
    int sampleRate;
    unsigned int frameSize;
    bool isPlaying = false;
};

class AudioDevice
{
    
    NO_MOVE(AudioDevice);
    
    std::unique_ptr<RtAudio> rtaudio;
    
public:
    
    DeviceInfo info;
    
    static void ListAudioDevices();
    
    AudioDevice(int numChannels, int sampleRate, int deviceId = -1)
    {
        rtaudio = std::unique_ptr<RtAudio>(new RtAudio);
        info.id = deviceId != -1 ? deviceId : rtaudio->getDefaultOutputDevice();
        info.numChannels = numChannels;
        info.sampleRate = sampleRate;
        info.frameSize = FRAME_SIZE;
    }
    
    ~AudioDevice()
    {
        if (rtaudio) 
        {
            rtaudio->stopStream();
            if (rtaudio->isStreamOpen())
                rtaudio->closeStream();
        }
    }
    
    bool Open(const int deviceId);
    
    bool Play(const std::vector<float> & data);
    
};
    
} // end namespace nqr

#endif
