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

#ifndef POSTPROCESS_H
#define POSTPROCESS_H

#include <vector>

namespace nqr
{

template <typename T>
inline void DeinterleaveStereo(T * c1, T * c2, T const * src, size_t count)
{
	auto src_end = src + count;
	while (src != src_end)
	{
		*c1 = src[0];
		*c2 = src[1];
		c1++;
		c2++;
		src += 2;
	}
}

template<typename T>
void InterleaveArbitrary(const T * src, T * dest, size_t numFramesPerChannel, size_t numChannels, size_t numCopyFrames)
{
    for (size_t ch = 0; ch < numChannels; ch++)
    {
        size_t x = ch;
        const T * srcChannel = &src[ch * numFramesPerChannel];
        for(size_t i = 0; i < numCopyFrames; i++)
        {
            dest[x] = srcChannel[i];
            x += numChannels;
        }
    }
}

template<typename T>
void DeinterleaveArbitrary(const T * src, T * dest, size_t numFramesPerChannel, size_t numChannels, size_t numCopyFrames)
{
    for(size_t ch = 0; ch < numChannels; ch++)
    {
        size_t x = ch;
        T *destChannel = &dest[ch * numFramesPerChannel];
        for (size_t i = 0; i < numCopyFrames; i++)
        {
            destChannel[i] = (T) src[x];
            x += numChannels;
        }
    }
}

inline void TrimSilenceInterleaved(std::vector<float> & buffer, float v, bool fromFront, bool fromEnd)
{
    //@todo implement me
}
    
} // end namespace nqr

#endif
