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

#if (_MSC_VER)
    #pragma warning (push)
    #pragma warning (disable: 181 111 4267 4996 4244 4701 4702 4133 4100 4127 4206 4312 4505 4365 4005 4013 4334 4703)
#endif
        
#ifdef __clang__
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wconversion"
    #pragma clang diagnostic ignored "-Wshadow"
    #pragma clang diagnostic ignored "-Wdeprecated-register"
#endif

#ifdef _WIN32
    #ifndef WIN32
        #define WIN32
    #endif
#endif

#include "wavpack/src/bits.c"
#include "wavpack/src/extra1.c"
#define WavpackExtraInfo WavpackExtraInfo_alt
#define log2overhead log2overhead_alt
#define xtable xtable_alt
#include "wavpack/src/extra2.c"
#include "wavpack/src/float.c"
#include "wavpack/src/metadata.c"
#define decorr_stereo_pass decorr_stereo_pass_alt
#include "wavpack/src/pack.c"
#include "wavpack/src/tags.c"
#undef  decorr_stereo_pass
#define decorr_stereo_pass decorr_stereo_pass_alt_2
#include "wavpack/src/unpack.c"
#include "wavpack/src/unpack3.c"
#include "wavpack/src/words.c"
#include "wavpack/src/wputils.c"

#ifdef __clang__
    #pragma clang diagnostic pop
#endif

#if (_MSC_VER)
    #pragma warning (pop)
#endif
