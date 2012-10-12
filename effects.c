#include "sackit_internal.h"

uint32_t sackit_pitchslide_linear(uint32_t freq, int16_t amt)
{
	uint32_t slidemul;
	
	if(amt == 0)
		return freq;
	
	if(amt < 0)
	{
		slidemul = (uint32_t)linear_slide_down_table[-amt];
	} else  {
		slidemul = (uint32_t)linear_slide_up_table[amt*2+1];
		slidemul <<= 16;
		slidemul += (uint32_t)linear_slide_up_table[amt*2];
	}
	
	uint32_t r = sackit_mul_fixed_16_int_32(slidemul, freq);
	
	//printf("slide %i\n", r);
	
	return r;
}

uint32_t sackit_pitchslide_linear_fine(uint32_t freq, int16_t amt)
{
	uint32_t slidemul;
	
	if(amt == 0)
		return freq;
	
	if(amt < 0)
	{
		slidemul = (uint32_t)fine_linear_slide_down_table[-amt];
	} else  {
		slidemul = (uint32_t)fine_linear_slide_up_table[amt*2+1];
		slidemul <<= 16;
		slidemul += (uint32_t)fine_linear_slide_up_table[amt*2];
	}
	
	return sackit_mul_fixed_16_int_32(slidemul, freq);
}

uint32_t sackit_pitchslide_amiga_fine(uint32_t freq, int16_t amt)
{
	if(amt == 0)
		return freq;
	
	uint32_t r = AMICLK/(AMICLK/((int32_t)freq) - ((int32_t)amt)*64);
	
	//printf("ami %i\n", r);
	
	return r;
}

void sackit_effect_volslide(sackit_playback_t *sackit, sackit_pchannel_t *pchn, int8_t amt)
{
	if(amt < 0)
	{
		pchn->achn->vol = (pchn->achn->vol < -amt
			? 0
			: pchn->achn->vol+amt);
	} else if(amt > 0) {
		pchn->achn->vol = (pchn->achn->vol+amt > 64
			? 64
			: pchn->achn->vol+amt);
	}
}

void sackit_effect_volslide_cv(sackit_playback_t *sackit, sackit_pchannel_t *pchn, int8_t amt)
{
	if(amt < 0)
	{
		pchn->cv = (pchn->cv < -amt
			? 0
			: pchn->cv+amt);
	} else if(amt > 0) {
		pchn->cv = (pchn->cv+amt > 64
			? 64
			: pchn->cv+amt);
	}
	pchn->achn->cv = pchn->cv;
}

void sackit_effect_volslide_gv(sackit_playback_t *sackit, sackit_pchannel_t *pchn, int8_t amt)
{
	if(amt < 0)
	{
		sackit->gv = (sackit->gv < -amt
			? 0
			: sackit->gv+amt);
	} else if(amt > 0) {
		sackit->gv = (sackit->gv+amt > 128
			? 128
			: sackit->gv+amt);
	}
}

void sackit_effect_pitchslide(sackit_playback_t *sackit, sackit_pchannel_t *pchn, int16_t amt)
{
	if(amt == 0 || pchn->achn->freq == 0)
		return;
	
	// TODO confirm this behaviour
	
	if(sackit->module->header.flags & IT_MOD_LINEAR)
	{
		pchn->achn->freq = sackit_pitchslide_linear(pchn->achn->freq, amt);
	} else {
		pchn->achn->freq = sackit_pitchslide_amiga_fine(pchn->achn->freq, amt*4);
	}
	
	//printf("%i %i\n", pchn->achn->freq, pchn->achn->flags);
}

void sackit_effect_pitchslide_fine(sackit_playback_t *sackit, sackit_pchannel_t *pchn, int16_t amt)
{
	if(amt == 0 || pchn->achn->freq == 0)
		return;
	
	// TODO confirm this behaviour
	
	if(sackit->module->header.flags & IT_MOD_LINEAR)
	{
		pchn->achn->freq = sackit_pitchslide_linear_fine(pchn->achn->freq, amt);
	} else {
		pchn->achn->freq = sackit_pitchslide_amiga_fine(pchn->achn->freq, amt);
	}
}

void sackit_effect_portaslide(sackit_playback_t *sackit, sackit_pchannel_t *pchn, int16_t amt)
{
	if(amt == 0 || (uint32_t)pchn->achn->freq == pchn->tfreq || pchn->achn->freq == 0)
		return;
	
	if((uint32_t)pchn->achn->freq < pchn->tfreq)
	{
		sackit_effect_pitchslide(sackit, pchn, amt);
		// TODO: confirm if > or >=
		if((uint32_t)pchn->achn->freq >= pchn->tfreq)
			pchn->nfreq = pchn->achn->freq = pchn->tfreq;
	} else {
		sackit_effect_pitchslide(sackit, pchn, -amt);
		// TODO: confirm if < or <=
		if((uint32_t)pchn->achn->freq <= pchn->tfreq)
			pchn->nfreq = pchn->achn->freq = pchn->tfreq;
	}
	
	//printf("%i\n", pchn->achn->freq);
}

void sackit_effect_vibrato(sackit_playback_t *sackit, sackit_pchannel_t *pchn)
{
	int32_t v;
	
	if(pchn->achn->ofreq == 0 || pchn->vib_speed == 0)
		return;
	
	//if(pchn->achn->vol == 0 || !(pchn->achn->flags & SACKIT_ACHN_PLAYING))
	if(!(pchn->achn->flags & SACKIT_ACHN_PLAYING))
		return;
	
	// vibrato starts IMMEDIATELY.
	pchn->vib_offs += pchn->vib_speed;
	
	uint8_t offs = (uint8_t)pchn->vib_offs;
	
	switch(pchn->vib_type&3)
	{
		case 0: // sine
			v = fine_sine_data[offs];
			break;
		case 1: // ramp down
			v = fine_ramp_down_data[offs];
			break;
		case 2: // square
			v = fine_square_wave[offs];
			break;
		case 3: // random - NOT EASILY TESTABLE
			// TODO!
			v = 0;
			break;
	}
	
	/*
	chan_sj.it:
	12464 8
	12532 14
	12554 16
	12600 20
	12554 16
	12532 14
	12464 8
	12375 0
	12341 -7
	12219 -14
	12197 -16
	12153 -20
	12197 -16
	12219 -14
	12341 -7
	12375 0
	*/
	
	v = v*pchn->vib_depth;
	int negdepth = (v < 0 ? 1 : 0);
	if(negdepth) v = ~v;
	v = (v+8)>>4;
	if(negdepth) v = -v;
	
	if(sackit->module->header.flags & IT_MOD_LINEAR)
	{
		if(v >= -15 && v <= 15)
		{
			pchn->achn->ofreq = sackit_pitchslide_linear_fine(pchn->achn->ofreq, v);
		} else {
			// compensating that i have no separate slide up/down function
			pchn->achn->ofreq = sackit_pitchslide_linear(pchn->achn->ofreq
				, (negdepth ? -((-v)>>2): v>>2));
		}
	} else {
		pchn->achn->ofreq = sackit_pitchslide_amiga_fine(pchn->achn->ofreq, v);
	}
	
	//printf("v %i %i\n", pchn->achn->ofreq, v);
}
