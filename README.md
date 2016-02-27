# Libnyquist

[![Build status](https://ci.appveyor.com/api/projects/status/2xeuyuxy618ndf4r?svg=true)](https://ci.appveyor.com/project/ddiakopoulos/libnyquist)

Libnyquist is a small C++11 library for reading sampled audio data from disk or memory. It's ideal to use as an audio asset frontend for games, audio sequencers, music players, and more.

The library steers away from patent or GPL license encumbered formats (such as MP3 and AAC). For portability, libnyquist does not link against platform-specific APIs like Windows Media Foundation or CoreAudio, and instead bundles the source code of reference decoders as an implementation detail. 

Libnyquist is meant to be statically linked, which is not the case with other popular libraries like libsndfile (LGPL). Furthermore, the library is not concerned with legacy format support (for instance, A-law PCM encoding or SND files). 
 
While untested, there are no technical conditions that preclude compilation on other platforms with C++11 support (Android NDK r10e+, Linux, iOS, etc).

## Format Support

Regardless of input bit depth, the library hands over an interleaved float array, normalized between [-1.0,+1.0]. At present, the library does not provide resampling functionality. 

* Wave (+ IMA-ADPCM encoding)
* Ogg Vorbis
* Ogg Opus
* FLAC
* WavPack
* Musepack
* Multichannel modules (669, amf, ams, dbm, dmf, dsm, far, it, j2b, mdl, med, mod, mt2, mtm, okt, pat, psm, ptm, s3m, stm, ult, umx, xm)
* MIDI files [(SoundFonts required (*))](#about-midi-files-and-soundfonts)

## Encoding
Simple but robust WAV format encoder now included. Extentions in the near future might include Ogg. 

## Supported Project Files
* Visual Studio 2013
* Visual Studio 2015
* XCode 6

## Known Issues & Bugs
* See the Github issue tracker. 

## About MIDI files and SoundFonts
A SoundFont `.sf2` file is a file that contains many samples of different instruments playing different notes, enabling Libnyquist to render finally each instrument sound in the MIDI song. SoundFonts are available on the web, but the quality and size varies a lot, and they may have licensing issues (because of the copyright of the different samples), they may be designed for different styles of music (classical, jazz, pop...), and they may target different sets of instruments as well. As a general rule, most MIDI files are targeted to use the 128 sounds of [General MIDI standard (GM)](http://en.wikipedia.org/wiki/General_MIDI), so use a SoundFont that does conform to this standard. You probably want to try the [Fluid R3 GM (141 MB uncompressed)](http://google.com/search?q=Fluid+R3+GM+soundfont) as a starter pack, which is a very good general purpose SoundFont (and fully MIT licensed).

SoundFont files are large and often compressed in `.zip`, `.tar.gz` or `.sfArk` formats. `.sfArk` files can be unpacked with [sfark tool](http://www.melodymachine.com/sfark.htm). For now, libnyquist supports `.pat` (GUS) soundfonts only, which can be converted from `.sf2` files with the [unsf tool](http://alsa.opensrc.org/Unsf).

Back to the library, the compiler directive `PAT_CONFIG_FILE` tells libnyquist what soundfont file is hardcoded by default (defaults to `pat/timidity.cfg`, but might be `FluidR3_GM2/FluidR3_GM2-2.cfg`, or any other valid path during compilation). Additionally, there is a runtime check for the environment variable `MMPAT_PATH_TO_CFG`, which contains the path to the config file. For more info please check [this source file](third_party\libmodplug\src\load_pat.cpp)

## License
Libyquist is released under the 2-Clause BSD license. All dependencies and codecs are under similar open licenses.
