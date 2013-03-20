#include "sackit_internal.h"

// IT211 mixer
#define MIXER_NAME sackit_playback_mixstuff_it211
#define MIXER_VER 211
#include "mixer_int.h"
#undef MIXER_NAME

#define MIXER_NAME sackit_playback_mixstuff_it211s
#define MIXER_STEREO
#include "mixer_int.h"
#undef MIXER_STEREO
#undef MIXER_NAME
#undef MIXER_VER

// IT211 mixer, interpolated
#define MIXER_INTERP_LINEAR
#define MIXER_NAME sackit_playback_mixstuff_it211l
#define MIXER_VER 211
#include "mixer_int.h"
#undef MIXER_NAME

#define MIXER_NAME sackit_playback_mixstuff_it211ls
#define MIXER_STEREO
#include "mixer_int.h"
#undef MIXER_STEREO
#undef MIXER_NAME
#undef MIXER_VER
#undef MIXER_INTERP_LINEAR

// IT212 mixer: IT211 with an anticlick filter for note cuts
#define MIXER_NAME sackit_playback_mixstuff_it212
#define MIXER_VER 212
#define MIXER_ANTICLICK
#include "mixer_int.h"
#undef MIXER_NAME

#define MIXER_NAME sackit_playback_mixstuff_it212s
#define MIXER_STEREO
#include "mixer_int.h"
#undef MIXER_STEREO
#undef MIXER_NAME
#undef MIXER_ANTICLICK
#undef MIXER_VER

// IT212 mixer: IT211 with an anticlick filter for note cuts, interpolated
#define MIXER_NAME sackit_playback_mixstuff_it212l
#define MIXER_VER 212
#define MIXER_ANTICLICK
#define MIXER_INTERP_LINEAR
#include "mixer_int.h"
#undef MIXER_NAME

#define MIXER_NAME sackit_playback_mixstuff_it212ls
#define MIXER_STEREO
#include "mixer_int.h"
#undef MIXER_STEREO
#undef MIXER_NAME
#undef MIXER_ANTICLICK
#undef MIXER_VER
#undef MIXER_INTERP_LINEAR

