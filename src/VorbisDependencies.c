#pragma comment(user, "license")

#if (_MSC_VER)
#pragma warning (push)
#pragma warning (disable: 181 111 4267 4996 4244 4701 4702 4133 4100 4127 4206 4312 4505 4365 4005 4013 4334)
#endif
        
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wconversion"
#pragma clang diagnostic ignored "-Wshadow"
#pragma clang diagnostic ignored "-Wdeprecated-register"
#endif

#include "libvorbis/include/vorbis/vorbisenc.h"
#include "libvorbis/include/vorbis/codec.h"
#include "libvorbis/include/vorbis/vorbisfile.h"

#include "libogg/src/bitwise.c"
#include "libogg/src/framing.c"
        
#include "libvorbis/src/analysis.c"
#include "libvorbis/src/bitrate.c"
#include "libvorbis/src/block.c"
#include "libvorbis/src/codebook.c"
#include "libvorbis/src/envelope.c"
#include "libvorbis/src/floor0.c"
#include "libvorbis/src/floor1.c"
#include "libvorbis/src/info.c"
#include "libvorbis/src/lpc.c"
#include "libvorbis/src/lsp.c"
#include "libvorbis/src/mapping0.c"
#include "libvorbis/src/psy.c"
#include "libvorbis/src/registry.c"
#include "libvorbis/src/res0.c"
#include "libvorbis/src/sharedbook.c"
#include "libvorbis/src/smallft.c"
#include "libvorbis/src/synthesis.c"
#include "libvorbis/src/vorbisenc.c"
#include "libvorbis/src/vorbisfile.c"
#include "libvorbis/src/window.c"
#include "libvorbis/src/mdct.c"

#ifdef __clang__
#pragma clang diagnostic pop
#endif
        
#if (_MSC_VER)
#pragma warning (pop)
#endif
