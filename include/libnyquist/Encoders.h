/*
copyright (c) 2019, dimitri diakopoulos all rights reserved.

redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

this software is provided by the copyright holders and contributors "as is"
and any express or implied warranties, including, but not limited to, the
implied warranties of merchantability and fitness for a particular purpose are
disclaimed. in no event shall the copyright holder or contributors be liable
for any direct, indirect, incidental, special, exemplary, or consequential
damages (including, but not limited to, procurement of substitute goods or
services; loss of use, data, or profits; or business interruption) however
caused and on any theory of liability, whether in contract, strict liability,
or tort (including negligence or otherwise) arising in any way out of the use
of this software, even if advised of the possibility of such damage.
*/

#ifndef nyquist_encoders_h
#define nyquist_encoders_h

#include "common.h"

namespace nqr
{
    // a simplistic encoder that takes a buffer of audio, conforms it to the user's
    // encoderparams preference, and writes to disk. be warned, does not support resampling!
    // @todo support dithering, samplerate conversion, etc.
    int encode_wav_to_disk(const encoderparams p, const audiodata * d, const std::string & path);

    // assume data adheres to encoderparams, except for bit depth and fmt which are re-formatted
    // to satisfy the ogg/opus spec.
    int encode_opus_to_disk(const encoderparams p, const audiodata * d, const std::string & path);

} // end namespace nqr

#endif // end nyquist_encoders_h
