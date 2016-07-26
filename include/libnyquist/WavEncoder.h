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

#ifndef WAVE_ENCODER_H
#define WAVE_ENCODER_H

#include "Common.h"
#include "WavDecoder.h"
#include "RiffUtils.h"

namespace nqr
{

	// This is a naieve implementation of a resampling filter where a lerp is used as a bad low-pass.
	// It very far from the ideal case and should be used with caution (or not at all) on signals that matter.
	// It is included here to upsample 44.1k to 48k for the purposes of microphone input => Opus, where the the 
	// nominal frequencies of speech are particularly far from Nyquist.
	static inline void linear_resample(const double rate, const std::vector<float> & input, std::vector<float> & output, size_t samplesToProcess)
	{
		double virtualReadIndex = 0;
		double a, b, i, sample;
		uint32_t n = samplesToProcess - 1;
		while (n--)
		{
			uint32_t readIndex = static_cast<uint32_t>(virtualReadIndex);
			i = virtualReadIndex - readIndex;
			a = input[readIndex + 0];
			b = input[readIndex + 1];
			sample = (1.0 - i) * a + i * b; // linear interpolate
			output.push_back(sample);
			virtualReadIndex += rate;
		}
	}

	enum EncoderError
	{
		NoError,
		InsufficientSampleData,
		FileIOError,
		UnsupportedSamplerate,
		UnsupportedChannelConfiguration,
		UnsupportedBitdepth,
		UnsupportedChannelMix,
		BufferTooBig,
	};
    
	// A simplistic encoder that takes a blob of data, conforms it to the user's
	// EncoderParams preference, and writes to disk. Be warned, does not support resampling!
	// @todo support dithering, samplerate conversion, etc.
	struct WavEncoder
	{
		// Assume data adheres to EncoderParams, except for bit depth and fmt
		static int WriteFile(const EncoderParams p, const AudioData * d, const std::string & path);
	};

	struct OggOpusEncoder
	{
		static int WriteFile(const EncoderParams p, const AudioData * d, const std::string & path);
	};

} // end namespace nqr

#endif
