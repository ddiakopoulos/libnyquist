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

#ifndef IMA4_UTIL_H
#define IMA4_UTIL_H

#include "AudioDecoder.h"

namespace nqr
{

    struct ADPCMState
    {
        int frame_size;
        int firstDataBlockByte;
        int dataSize;
        int currentByte;
        const uint8_t * inBuffer;
    };

    static const int ima_index_table[16] =
    {
        -1, -1, -1, -1,  // +0 / +3 : - the step
        2, 4, 6, 8,      // +4 / +7 : + the step
        -1, -1, -1, -1,  // -0 / -3 : - the step
        2, 4, 6, 8,      // -4 / -7 : + the step
    };

    static inline int ima_clamp_index(int index)
    {
        if (index < 0) return 0;
        else if (index > 88) return 88;
        return index;
    }
    
    static inline int16_t ima_clamp_predict(int16_t predict)
    {
        if (predict < -32768) return -32768;
        else if (predict > 32767) return 32767;
        return predict;
    }
    
    static const int ima_step_table[89] =
    {
        7, 8, 9, 10, 11, 12, 13, 14, 16, 17, 19, 21, 23, 25, 28, 31, 34,
        37, 41, 45, 50, 55, 60, 66, 73, 80, 88, 97, 107, 118, 130, 143,
        157, 173, 190, 209, 230, 253, 279, 307, 337, 371, 408, 449, 494,
        544, 598, 658, 724, 796, 876, 963, 1060, 1166, 1282, 1411, 1552,
        1707, 1878, 2066, 2272, 2499, 2749, 3024, 3327, 3660, 4026,
        4428, 4871, 5358, 5894, 6484, 7132, 7845, 8630, 9493, 10442,
        11487, 12635, 13899, 15289, 16818, 18500, 20350, 22385, 24623,
        27086, 29794, 32767
    };

    // Decodes an IMA ADPCM nibble to a 16 bit pcm sample
    static inline int16_t decode_nibble(uint8_t nibble, int16_t & p, int & s)
    {
        // Compute a delta to add to the predictor value
        int diff = ima_step_table[s] >> 3;
        if (nibble & 4) diff += ima_step_table[s];
        if (nibble & 2) diff += ima_step_table[s] >> 1;
        if (nibble & 1) diff += ima_step_table[s] >> 2;

        // Sign
        if (nibble & 8) diff = -diff;
        
        // Add delta
        p += diff;

        s += ima_index_table[nibble];
        s = ima_clamp_index(s);
        
        return ima_clamp_predict(p);
    }
    
    void decode_ima_adpcm(ADPCMState & state, int16_t * outBuffer, uint32_t num_channels)
    {
        const uint8_t * data = state.inBuffer;
        
        // Loop over the interleaved channels
        for (int32_t ch = 0; ch < num_channels; ch++)
        {
            const int byteOffset = ch * 4;
            
            // Header Structure:
            // Byte0: packed low byte of the initial predictor
            // Byte1: packed high byte of the initial predictor
            // Byte2: initial step index
            // Byte3: Reserved empty value
            int16_t predictor = ((int16_t)data[byteOffset + 1] << 8) | data[byteOffset];
            int stepIndex = data[byteOffset + 2];
            
            uint8_t reserved = data[byteOffset + 3];
            if (reserved != 0) throw std::runtime_error("adpcm decode error");
            
            int byteIdx = num_channels * 4 + byteOffset; //the byte index of the first data word for this channel
            int idx = ch;
            
            // Decode nibbles of the remaining data
            while (byteIdx < state.frame_size)
            {
                for (int j = 0; j < 4; j++)
                {
                    outBuffer[idx] = decode_nibble(data[byteIdx] & 0xf, predictor, stepIndex); // low nibble
                    idx += num_channels;
                    outBuffer[idx] = decode_nibble(data[byteIdx] >> 4, predictor, stepIndex); // high nibble
                    idx += num_channels;
                    byteIdx++;
                }
                byteIdx += (num_channels - 1) << 2; // Jump to the next data word for the current channel
            }

        }

    }

} // end namespace nqr

#endif
