#include "sackit_internal.h"

void sackit_update_effects_chn(sackit_playback_t *sackit, sackit_pchannel_t *pchn,
	uint8_t note, uint8_t ins, uint8_t vol, uint8_t eft, uint8_t efp)
{
	//if(note != 253)
	//	printf("N %i %i\n", note, ins);
	
	pchn->slide_vol = 0;
	pchn->slide_vol_cv = 0;
	pchn->slide_vol_gv = 0;
	pchn->slide_pan = 0;
	pchn->slide_pitch = 0;
	pchn->slide_porta = 0;
	pchn->arpeggio = 0;
	pchn->vib_speed = 0;
	pchn->vib_depth = 0;
	
	pchn->note_cut = 0;
	pchn->note_delay = 0;
	pchn->note_delay_note = note;
	pchn->note_delay_ins = ins;
	pchn->note_delay_vol = vol;
	
	int16_t slide_vol_now = 0;
	int16_t slide_vol_cv_now = 0;
	int16_t slide_vol_gv_now = 0;
	int16_t slide_pan_now = 0;
	int16_t slide_pitch_now = 0;
	int16_t slide_pitch_fine_now = 0;
	
	int flag_slide_porta = 0;
	int flag_retrig = 0;
	int flag_vibrato = 0;
	int flag_done_instrument = 0;

	uint32_t new_sample_offset = 0;
	
	uint8_t el = efp&15;
	uint8_t eh = efp>>4;
	int vfp = 0;
	switch(eft)
	{
		case 0x01: // Axx - Set Speed (mislabelled as "Tempo" in ITTECH.TXT --GM)
			if(efp != 0x00)
			{
				sackit->max_tick = efp;
				sackit->current_tick = efp;
			}
			break;
		
		case 0x02: // Bxx - Jump to Order
			sackit->process_order = efp - 1;
			sackit->process_row = 0xFFFE; // indicates new pattern internally for IT...
			break;
		
		case 0x03: // Cxx - Break to Row
			sackit->break_row = efp;
			sackit->process_row = 0xFFFE;
			break;
		
		case 0x04: // Dxx - Volume slide
		case 0x0B: // Kxx - (vibrato + vol slide)
		case 0x0C: // Lxx - (porta to note + vol slide)
			// TODO: confirm behaviour
			if(efp == 0)
			{
				efp = pchn->eff_slide_vol;
				el = efp&15;
				eh = efp>>4;
			} else {
				pchn->eff_slide_vol = efp;
			}
			
			if(el == 0)
				pchn->slide_vol += eh;
			else if(eh == 0)
				pchn->slide_vol -= el;
			else if(el == 0xF)
				slide_vol_now += eh;
			else if(eh == 0xF)
				slide_vol_now -= el;
			
			if(efp == 0x0F || efp == 0xF0)
				slide_vol_now += eh-el;
			
			efp = eh = el = 0;
			break;
		
		case 0x05: // Exx - (pitch slide down)
		case 0x06: // Fxx - (pitch slide up)
			if(efp == 0)
			{
				efp = pchn->eff_slide_pitch;
			} else {
				pchn->eff_slide_pitch = efp;
			}
			
			// TODO: confirm behaviour
			if(efp <= 0xDF)
			{
				pchn->slide_pitch += (eft == 0x05 ? -1 : 1)*efp;
			} else if(efp <= 0xEF) {
				slide_pitch_fine_now += (eft == 0x05 ? -1 : 1)*(efp&15);
			} else {
				slide_pitch_now += (eft == 0x05 ? -1 : 1)*(efp&15);
			}
			break;
		
		case 0x0A: // Jxx - (arpeggio)
			if(efp == 0)
			{
				efp = pchn->eff_arpeggio;
			} else {
				pchn->eff_arpeggio = efp;
			}
			pchn->arpeggio = efp;
			break;
		
		case 0x0D: // Mxx - (channel volume)
			// TODO: confirm behaviour
			if(efp <= 64)
			{
				pchn->cv = efp;
				pchn->achn->cv = efp;
			}
			break;
		
		case 0x0E: // Nxx - (channel volume slide)
			// TODO: confirm behaviour
			if(efp == 0)
			{
				efp = pchn->eff_slide_vol_cv;
				el = efp&15;
				eh = efp>>4;
			} else {
				pchn->eff_slide_vol_cv = efp;
			}
			
			if(el == 0)
				pchn->slide_vol_cv += eh;
			else if(eh == 0)
				pchn->slide_vol_cv -= el;
			else if(el == 0xF)
				slide_vol_cv_now += eh;
			else if(eh == 0xF)
				slide_vol_cv_now -= el;
			
			efp = eh = el = 0;
			break;
		
		case 0x0F: // Oxx - (sample offset)
			// TODO: get out-of-range behaviour correct!
			if(efp == 0)
			{
				efp = pchn->eff_sample_offs;
			} else {
				pchn->eff_sample_offs = efp;
			}
			
			new_sample_offset = efp<<8;
			
			break;
		
		case 0x13: // Sxx - (miscellaneous)
			if(efp == 0)
			{
				efp = pchn->eff_misc;
				el = efp&15;
				eh = efp>>4;
			} else {
				pchn->eff_misc = efp;
			}
			switch(eh)
			{
				case 0xC: // SCx - (note cut)
					pchn->note_cut = (el == 0 ? 1 : el);
					break;
				case 0xD: // SDx - (note delay)
					pchn->note_delay = (el == 0 ? 1 : el);
					if(ins != 0)
						pchn->lins = ins;
					return; // cut this part!
					break;
				case 0xE: // SEx - (pattern delay)
					if(sackit->row_counter == 1)
					{
						sackit->row_counter = el+1;
					}
					break;
			}
			break;
		
		case 0x14: // Txx - (tempo)
			if(efp == 0)
			{
				efp = pchn->eff_tempo;
			} else {
				pchn->eff_tempo = efp;
			}
			
			if(efp < 0x10)
			{
				sackit->tempo -= efp;
				if(sackit->tempo < 32)
					sackit->tempo = 32;
			} else if(efp < 0x20) {
				sackit->tempo += efp-0x10;
				if(sackit->tempo > 255)
					sackit->tempo = 255;
			} else {
				sackit->tempo = efp;
			}
			break;
		
		case 0x16: // Vxx - (global volume)
			if(efp <= 0x80)
			{
				sackit->gv = efp;
			}
			break;
		
		case 0x17: // Wxx - (global volume slide)
			// TODO: confirm behaviour
			if(efp == 0)
			{
				efp = pchn->eff_slide_vol_gv;
				el = efp&15;
				eh = efp>>4;
			} else {
				pchn->eff_slide_vol_gv = efp;
			}
			
			if(el == 0)
				pchn->slide_vol_gv += eh;
			else if(eh == 0)
				pchn->slide_vol_gv -= el;
			else if(el == 0xF)
				slide_vol_gv_now += eh;
			else if(eh == 0xF)
				slide_vol_gv_now -= el;
			
			efp = eh = el = 0;
			break;
	}
	
	switch(eft)
	{
		case 0x07: // Gxx - (porta to note)
		case 0x0C: // Lxx - (porta to note + vol slide)
			if(efp == 0)
			{
				efp = (sackit->module->header.flags & IT_MOD_COMPGXX
					? pchn->eff_slide_porta
					: pchn->eff_slide_pitch);
			} else if(sackit->module->header.flags & IT_MOD_COMPGXX) {
				pchn->eff_slide_porta = efp;
			} else {
				pchn->eff_slide_pitch = efp;
			}
			
			pchn->slide_porta += efp;
			flag_slide_porta = 1;
			// TODO: confirm behaviour
			break;
		
		case 0x08: // Hxx - (vibrato)
		case 0x15: // Uxx - (fine vibrato)
		case 0x0B: // Kxx - (vibrato + vol slide)
			// TODO: check if x,y independent
			if((efp&0x0F) == 0)
				efp |= (pchn->eff_vibrato&0x0F);
			if((efp&0xF0) == 0)
				efp |= (pchn->eff_vibrato&0xF0);
			
			pchn->eff_vibrato = efp;
			
			pchn->vib_speed += (efp>>4)*4;
			pchn->vib_depth += (efp&15)*(eft == 0x15 ? 1 : 4);
			
			//if(!(sackit->module->header.flags & IT_MOD_OLDFX))
			flag_vibrato = 1;
			break;
	}
	
	if(vol <= 64)
	{
		// volume
		// (OPTIONAL: Feel free to emulate pre-voleffects stuff.
		//  (Turn the limit up to <= 127.))
		pchn->achn->vol = vol;
	} else if (vol <= 74) {
		// Ax
		if(vol == 65)
		{
			vfp = pchn->eff_slide_vol_veff;
		} else {
			pchn->eff_slide_vol_veff = vfp = ((int16_t)(vol-65));
		}
		slide_vol_now += vfp;
	} else if (vol <= 84) {
		// Bx
		if(vol == 75)
		{
			vfp = pchn->eff_slide_vol_veff;
		} else {
			pchn->eff_slide_vol_veff = vfp = ((int16_t)(vol-75));
		}
		slide_vol_now -= vfp;
	} else if (vol <= 94) {
		// Cx
		if(vol == 85)
		{
			vfp = pchn->eff_slide_vol_veff;
		} else {
			pchn->eff_slide_vol_veff = vfp = ((int16_t)(vol-85));
		}
		pchn->slide_vol += vfp;
	} else if (vol <= 104) {
		// Dx
		if(vol == 95)
		{
			vfp = pchn->eff_slide_vol_veff;
		} else {
			pchn->eff_slide_vol_veff = vfp = ((int16_t)(vol-95));
		}
		pchn->slide_vol -= vfp;
	} else if (vol <= 114) {
		// Ex
		if(vol == 105)
		{
			vfp = pchn->eff_slide_pitch;
		} else {
			pchn->eff_slide_pitch = vfp = ((int16_t)(vol-105))*4;
		}
		
		pchn->slide_pitch -= vfp;
	} else if (vol <= 124) {
		// Fx
		if(vol == 115)
		{
			vfp = pchn->eff_slide_pitch;
		} else {
			vfp = pchn->eff_slide_pitch = ((int16_t)(vol-115))*4;
		}
		
		pchn->slide_pitch += vfp;
	} else if (vol <= 127) {
		// DO NOTHING
	} else if (vol <= 192) {
		// panning
	} else if (vol <= 202) {
		// Gx
		
		if(vol == 193)
		{
			vfp = (sackit->module->header.flags & IT_MOD_COMPGXX
				? pchn->eff_slide_porta
				: pchn->eff_slide_pitch);
		} else if(sackit->module->header.flags & IT_MOD_COMPGXX) {
			pchn->eff_slide_porta = vfp = slide_table[vol-194];
		} else {
			pchn->eff_slide_pitch = vfp = slide_table[vol-194];
		}
		
		pchn->slide_porta += vfp;
		flag_slide_porta = 1;
	} else if (vol <= 212) {
		// Hx
	}
	
	if(ins != 0)
	{
		if(sackit->module->header.flags & IT_MOD_INSTR)
		{
			uint8_t xnote = (note <= 119 ? note : pchn->note);
			
			it_instrument_t *cins = sackit->module->instruments[ins-1];
			if(cins == NULL)
				cins = pchn->achn->instrument;
			else
				pchn->achn->instrument = cins;
			
			
			// TODO: confirm behaviour
			if(cins->notesample[xnote][1] != 0)
			{
				it_sample_t *csmp = sackit->module->samples[cins->notesample[xnote][1]-1];
				if(csmp != NULL)
					pchn->achn->sample = csmp;
			}
			
			if(note <= 119)
				note = cins->notesample[xnote][0];
			
			flag_done_instrument = 1;
			
			if(cins != NULL)
				pchn->achn->iv = pchn->achn->instrument->gbv;
		} else {
			pchn->achn->instrument = NULL;
			it_sample_t *csmp = sackit->module->samples[ins-1];
			if(csmp != NULL)
				pchn->achn->sample = csmp;
		}
		
		if(pchn->achn->sample != NULL)
		{
			if(vol > 64)
				pchn->achn->vol = pchn->achn->sample->vol;
			
			pchn->achn->sv = pchn->achn->sample->gvl;
		}
		
		if((/*(!(pchn->achn->flags & SACKIT_ACHN_PLAYING)) ||*/ pchn->lins != ins)
			&& note == 253
			&& pchn->note != 253)
		{
			note = pchn->note;
		}
		
		pchn->lins = ins;
	}
	
	if(note <= 119)
	{
		// actual note
		uint32_t nfreq = 
			((uint32_t)(pitch_table[note*2]))
			| (((uint32_t)(pitch_table[note*2+1]))<<16);
		
		//printf("N %i %i %i\n", note, ins, nfreq);
		if(pchn->achn->sample != NULL)
		{
			nfreq = sackit_mul_fixed_16_int_32(nfreq, pchn->achn->sample->c5speed);
			pchn->tfreq = nfreq;
			pchn->note = note;
			
			// TODO: compat Gxx
			if((!(pchn->achn->flags & SACKIT_ACHN_PLAYING)) || !flag_slide_porta)
			{
				pchn->nfreq = nfreq;
				flag_retrig = 1;
			}
		}
	} else if(note == 255) {
		// note off
		pchn->achn->flags &= ~SACKIT_ACHN_SUSTAIN;
		
		if(pchn->achn->instrument != NULL)
		{
			it_instrument_t *cins = pchn->achn->instrument;
			if(cins->evol.flg & IT_ENV_ON)
			{
				if(cins->evol.flg & IT_ENV_LOOP)
					pchn->achn->flags |= SACKIT_ACHN_FADEOUT;
			} else {
				pchn->achn->flags |= SACKIT_ACHN_FADEOUT;
			}
		}
	} else if(note == 254) {
		// note cut
		pchn->achn->flags &= ~(
			SACKIT_ACHN_MIXING
			|SACKIT_ACHN_PLAYING
			|SACKIT_ACHN_SUSTAIN);
	} else if(note != 253) {
		// note fade
		pchn->achn->flags |= SACKIT_ACHN_FADEOUT;
	}
	
	if(flag_retrig)
	{
		if(!flag_done_instrument)
		{
			// FIXME: this is messy! it shouldn't be duplicated twice!
			if(sackit->module->header.flags & IT_MOD_INSTR)
			{
				it_instrument_t *cins = sackit->module->instruments[pchn->lins-1];
				if(cins == NULL)
					cins = pchn->achn->instrument;
				else
					pchn->achn->instrument = cins;
				
				// TODO: confirm behaviour
				if(cins == NULL)
				{
					flag_retrig = 0;
				} else if(cins->notesample[pchn->note][1] != 0) {
					// FIXME: do i need to do something with the note HERE?
					it_sample_t *csmp = sackit->module->samples[cins->notesample[pchn->note][1]-1];
					if(csmp == NULL)
						csmp = pchn->achn->sample;
					else
						pchn->achn->sample = csmp;
					
					if(csmp == NULL)
						flag_retrig = 0;
				}
			} else {
				pchn->achn->instrument = NULL;
				it_sample_t *csmp = sackit->module->samples[pchn->lins-1];
				if(csmp == NULL)
					csmp = pchn->achn->sample;
				else
					pchn->achn->sample = csmp;
				
				if(csmp == NULL)
					flag_retrig = 0;
			}
		}
		
		if(flag_retrig)
		{
			pchn->achn->freq = pchn->nfreq;
			pchn->achn->offs = new_sample_offset;
			pchn->achn->suboffs = 0;
			pchn->achn->cv = pchn->cv;
			
			pchn->achn->flags |= (
				SACKIT_ACHN_MIXING
				|SACKIT_ACHN_PLAYING
				|SACKIT_ACHN_RAMP
				|SACKIT_ACHN_SUSTAIN);
			
			pchn->achn->evol.x = 0;
			pchn->achn->epan.x = 0;
			pchn->achn->epitch.x = 0;
			pchn->achn->evol.idx = 0;
			pchn->achn->epan.idx = 0;
			pchn->achn->epitch.idx = 0;
			
			pchn->achn->fadeout = 1024;
			
			pchn->achn->flags &= ~(
				SACKIT_ACHN_REVERSE
				|SACKIT_ACHN_FADEOUT);
		}
	}
	
	if(flag_vibrato && pchn->vib_lins != pchn->lins)
	{
		pchn->vib_lins = pchn->lins;
		pchn->vib_offs = 0;
	}
	
	// update slides & stuff
	sackit_effect_volslide_cv(sackit, pchn, slide_vol_cv_now);
	sackit_effect_volslide_gv(sackit, pchn, slide_vol_gv_now);
	sackit_effect_volslide(sackit, pchn, slide_vol_now);
	sackit_effect_pitchslide(sackit, pchn, slide_pitch_now);
	sackit_effect_pitchslide_fine(sackit, pchn, slide_pitch_fine_now);
	pchn->achn->ofreq = pchn->achn->freq;
	if(flag_vibrato)
	{
		if(sackit->module->header.flags & IT_MOD_OLDFX)
			sackit_effect_vibrato_nooffs(sackit, pchn);
		else
			sackit_effect_vibrato(sackit, pchn);
	}
}
