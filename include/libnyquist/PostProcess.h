#pragma comment(user, "license")

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
