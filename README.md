# Libnyquist
Libnyquist is a small C++11 library for reading sampled audio data from disk or memory. It's ideal to use as an audio asset loading frontend for games, audio sequencers, music players, and more.

The library steers away from patent-encumbered formats and formats with non-BSD licensed reference implementations (such as MP3 and AAC, respectively). 

Libnyquist is meant to be statically linked, which is not the case with other popular libraries like libsndfile (LGPL). Furthermore, the library is not concerned with legacy format support (for instance, A-law PCM encoding or SND files). 
 
While untested, there are no technical reasons that preclude compilation on other platforms with C++11 support (Android 4.4+, Linux, iOS, etc).

## Format Support

Regardless of input bit depth, the library hands the user an interleaved float array, normalized between [-1.0,+1.0]. At present, the library does not include resampling functionality. 

* Wave
* Ogg Vorbis
* Ogg Opus
* FLAC
* WavPack
* Core Audio Format (Apple Lossless / AIFF)

## Platform Support
* Windows & Visual Studio 2013+
* OSX & XCode 5+

## Building

## Known Issues
* Ogg and Opus have conflicting mdct files. Their sources were modified such that they compile cleanly together in the same project.
* Streaming is not supported for any file formats.

## Encoding
WIP

## License
Libyquist is released under the 3-Clause BSD license. All dependencies are under similar/BSD-compatible licenses.
