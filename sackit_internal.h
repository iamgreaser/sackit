#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>

#include "sackit.h"

// effects.c
uint32_t sackit_pitchslide_linear(uint32_t freq, int16_t amt);
uint32_t sackit_pitchslide_linear_fine(uint32_t freq, int16_t amt);
uint32_t sackit_pitchslide_amiga_fine(uint32_t freq, int16_t amt);
void sackit_effect_volslide(sackit_playback_t *sackit, sackit_pchannel_t *pchn, int8_t amt);
void sackit_effect_volslide_cv(sackit_playback_t *sackit, sackit_pchannel_t *pchn, int8_t amt);
void sackit_effect_volslide_gv(sackit_playback_t *sackit, sackit_pchannel_t *pchn, int8_t amt);
void sackit_effect_pitchslide(sackit_playback_t *sackit, sackit_pchannel_t *pchn, int16_t amt);
void sackit_effect_pitchslide_fine(sackit_playback_t *sackit, sackit_pchannel_t *pchn, int16_t amt);
void sackit_effect_portaslide(sackit_playback_t *sackit, sackit_pchannel_t *pchn, int16_t amt);
void sackit_effect_vibrato_nooffs(sackit_playback_t *sackit, sackit_pchannel_t *pchn);
void sackit_effect_vibrato(sackit_playback_t *sackit, sackit_pchannel_t *pchn);

// fixedmath.c
uint32_t sackit_mul_fixed_16_int_32(uint32_t a, uint32_t b);
uint32_t sackit_div_int_32_32_to_fixed_16(uint32_t a, uint32_t b);

// mixer_*.c
void sackit_playback_mixstuff_it211(sackit_playback_t *sackit, int offs, int len);

// objects.c
void sackit_playback_reset_achn(sackit_achannel_t *achn);
void sackit_playback_reset_pchn(sackit_pchannel_t *pchn);

// playroutine.c
void sackit_update_effects_chn(sackit_playback_t *sackit, sackit_pchannel_t *pchn,
	uint8_t note, uint8_t ins, uint8_t vol, uint8_t eft, uint8_t efp);
void sackit_update_effects(sackit_playback_t *sackit);
void sackit_update_pattern(sackit_playback_t *sackit);
void sackit_nna_allocate(sackit_playback_t *sackit, sackit_pchannel_t *pchn);
void sackit_nna_note_off(sackit_playback_t *sackit, sackit_achannel_t *achn);
void sackit_nna_note_cut(sackit_playback_t *sackit, sackit_achannel_t *achn);
void sackit_nna_note_fade(sackit_playback_t *sackit, sackit_achannel_t *achn);
void sackit_tick(sackit_playback_t *sackit);

// tables.c
extern int8_t fine_sine_data[];
extern int8_t fine_ramp_down_data[];
extern int8_t fine_square_wave[];
extern uint16_t pitch_table[];
extern uint16_t fine_linear_slide_up_table[];
extern uint16_t linear_slide_up_table[];
extern uint16_t fine_linear_slide_down_table[];
extern uint16_t linear_slide_down_table[];
extern uint8_t slide_table[];

