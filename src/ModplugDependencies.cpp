/*
Copyright (c) 2016, Dimitri Diakopoulos All rights reserved.

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
    #pragma warning (disable: 181 111 4267 4996 4244 4701 4702 4133 4100 4127 4206 4312 4505 4365 4005 4013 4334)
#endif

#ifdef __clang__
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wconversion"
    #pragma clang diagnostic ignored "-Wshadow"
    #pragma clang diagnostic ignored "-Wdeprecated-register"
    #define HAVE_SETENV
#endif

#ifndef MODPLUG_STATIC
#define MODPLUG_STATIC
#endif

#define HAVE_SINF

#include "libmodplug/src/load_669.cpp"
#include "libmodplug/src/load_abc.cpp"
#include "libmodplug/src/load_amf.cpp"
#include "libmodplug/src/load_ams.cpp"
#include "libmodplug/src/load_dbm.cpp"
#include "libmodplug/src/load_dmf.cpp"
#include "libmodplug/src/load_dsm.cpp"
#include "libmodplug/src/load_far.cpp"
#include "libmodplug/src/load_it.cpp"
#include "libmodplug/src/load_j2b.cpp"
#include "libmodplug/src/load_mdl.cpp"
#include "libmodplug/src/load_med.cpp"

#define none none_alt
#define MMFILE MMFILE_alt
#define mmfseek mmfseek_alt
#define mmftell mmftell_alt
#define mmreadUBYTES mmreadUBYTES_alt
#include "libmodplug/src/load_mid.cpp"
#include "libmodplug/src/load_mod.cpp"
#include "libmodplug/src/load_mt2.cpp"
#include "libmodplug/src/load_mtm.cpp"
#include "libmodplug/src/load_okt.cpp"
#undef MMFILE
#undef mmfseek
#undef mmftell
#undef mmreadUBYTES

#define MMFILE MMFILE_alt2
#define mmfseek mmfseek_alt2
#define mmftell mmftell_alt2
#define mmreadUBYTES mmreadUBYTES_alt2
#include "libmodplug/src/load_pat.cpp"
#include "libmodplug/src/load_psm.cpp"
#include "libmodplug/src/load_ptm.cpp"
#include "libmodplug/src/load_s3m.cpp"
#include "libmodplug/src/load_stm.cpp"
#include "libmodplug/src/load_ult.cpp"
#include "libmodplug/src/load_umx.cpp"
#include "libmodplug/src/load_wav.cpp"
#include "libmodplug/src/load_xm.cpp"

#include "libmodplug/src/mmcmp.cpp"
#include "libmodplug/src/modplug.cpp"
#include "libmodplug/src/sndfile.cpp"
#include "libmodplug/src/sndmix.cpp"
#include "libmodplug/src/fastmix.cpp"
#include "libmodplug/src/snd_dsp.cpp"
#include "libmodplug/src/snd_flt.cpp"
#include "libmodplug/src/snd_fx.cpp"

#ifdef __clang__
    #pragma clang diagnostic pop
#endif

#if (_MSC_VER)
    #pragma warning (pop)
#endif
