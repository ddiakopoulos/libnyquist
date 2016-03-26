# Libnyquist

[![Build status](https://ci.appveyor.com/api/projects/status/2xeuyuxy618ndf4r?svg=true)](https://ci.appveyor.com/project/ddiakopoulos/libnyquist)

Libnyquist is a small C++11 library for reading sampled audio data from disk or memory. It's ideal to use as an audio asset frontend for games, audio sequencers, music players, and more.

The library steers away from patent or GPL license encumbered formats (such as MP3 and AAC). For portability, libnyquist does not link against platform-specific APIs like Windows Media Foundation or CoreAudio, and instead bundles the source code of reference decoders as an implementation detail. 

Libnyquist is meant to be statically linked, which is not the case with other popular libraries like libsndfile (which is licensed under the LGPL). Furthermore, the library is not concerned with supporting very rare encodings (for instance, A-law PCM or the SND format). 
 
While untested, there are no technical conditions that preclude compilation on other platforms with C++11 support (Android NDK r10e+, Linux, iOS, etc).

## Format Support

Regardless of input bit depth, the library hands over an interleaved float array, normalized between [-1.0,+1.0]. At present, the library does not provide resampling functionality. 

* Wave (+ IMA-ADPCM encoding)
* Ogg Vorbis
* Ogg Opus
* FLAC
* WavPack
* Musepack
* [MIDI files (with soundfonts)](midi-playback.md)
* libmodplug formats (669, amf, ams, dbm, dmf, dsm, far, it, j2b, mdl, med, mod, mt2, mtm, okt, pat, psm, ptm, s3m, stm, ult, umx, xm)

## Encoding
Simple but robust WAV format encoder now included. Extentions in the near future might include Ogg. 

## Supported Project Files
* Visual Studio 2013
* Visual Studio 2015
* XCode 6

## Known Issues & Bugs
* See the Github issue tracker. 

## License
Libyquist is released under the 2-Clause BSD license. All dependencies and codecs are under similar open licenses.
