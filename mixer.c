#include "sackit_internal.h"

// IT211 mixer
#define MIXER_NAME sackit_playback_mixstuff_it211
#define MIXER_VER 211
#include "mixer_int.h"

// IT212 mixer: IT211 with an anticlick filter for note cuts (TODO!)
#define MIXER_NAME sackit_playback_mixstuff_it212
#define MIXER_VER 212
#define MIXER_ANTICLICK
#include "mixer_int.h"

