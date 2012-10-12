#include "sackit_internal.h"

void sackit_update_effects(sackit_playback_t *sackit)
{
	int i;
	
	for(i = 0; i < 64; i++)
	{
		sackit_pchannel_t *pchn = &(sackit->pchn[i]);
		
		sackit_effect_volslide_cv(sackit, pchn, pchn->slide_vol_cv);
		sackit_effect_volslide_gv(sackit, pchn, pchn->slide_vol_gv);
		sackit_effect_volslide(sackit, pchn, pchn->slide_vol);
		
		// TODO: confirm order
		sackit_effect_pitchslide(sackit, pchn, pchn->slide_pitch);
		sackit_effect_portaslide(sackit, pchn, pchn->slide_porta);
		
		pchn->achn->ofreq = pchn->achn->freq;
		
		sackit_effect_vibrato(sackit, pchn);
		
		uint16_t arp = (pchn->arpeggio>>4)&15;
		if(arp != 0)
		{
			uint32_t arpmul = (uint32_t)pitch_table[(arp+60)*2+1];
			arpmul <<= 16;
			arpmul += (uint32_t)pitch_table[(arp+60)*2];
			pchn->achn->ofreq = sackit_mul_fixed_16_int_32(arpmul, pchn->achn->ofreq);
		}
		
		pchn->arpeggio = ((pchn->arpeggio<<4)&0xFFF)|((pchn->arpeggio>>8)&15);
		
		if(pchn->note_cut != 0)
		{
			pchn->note_cut--;
			if(pchn->note_cut == 0)
			{
				pchn->achn->flags &= ~(
					SACKIT_ACHN_MIXING
					|SACKIT_ACHN_PLAYING
					|SACKIT_ACHN_SUSTAIN
				);
			}
		}
		
		
		if(pchn->note_delay != 0)
		{
			pchn->note_delay--;
			if(pchn->note_delay == 0)
			{
				pchn->note_delay = sackit->max_tick;
				sackit_update_effects_chn(sackit, pchn
					,pchn->note_delay_note
					,pchn->note_delay_ins
					,pchn->note_delay_vol
					,0,0);
			}
		}
	}
}

void sackit_update_pattern(sackit_playback_t *sackit)
{
	int i;
	
	it_pattern_t *pat = sackit->module->patterns[sackit->current_pattern];
	int ptr = sackit->pat_ptr;
	uint8_t *data = pat->data;
	
	if(sackit->pat_row > sackit->process_row)
	{
		sackit->pat_row = 0;
		ptr = 0;
	}
	
	uint8_t note[64], ins[64], vol[64], eft[64], efp[64];
	
	for(i = 0; i < 64; i++)
	{
		note[i] = 253;
		ins[i] = 0;
		vol[i] = 255;
		eft[i] = 0;
		efp[i] = 0;
	}
	
	//printf("pat_row %i %i\n", sackit->pat_row, sackit->process_row);
	
	while(sackit->pat_row <= sackit->process_row)
	{
		while(data[ptr] != 0x00)
		{
			uint8_t cval = data[ptr++];
			uint8_t chn = ((cval-1)&0x7F);
			sackit_pchannel_t *pchn = &(sackit->pchn[chn]);
			
			if(cval&0x80)
				pchn->lmask = data[ptr++];
			
			if(pchn->lmask&0x01)
				pchn->ldata[0] = data[ptr++];
			if(pchn->lmask&0x02)
				pchn->ldata[1] = data[ptr++];
			if(pchn->lmask&0x04)
				pchn->ldata[2] = data[ptr++];
			if(pchn->lmask&0x08)
			{
				pchn->ldata[3] = data[ptr++];
				pchn->ldata[4] = data[ptr++];
			}
			
			if(sackit->pat_row == sackit->process_row)
			{
				if(pchn->lmask&0x11)
					note[chn] = pchn->ldata[0];
				if(pchn->lmask&0x22)
					ins[chn] = pchn->ldata[1];
				if(pchn->lmask&0x44)
					vol[chn] = pchn->ldata[2];
				if(pchn->lmask&0x88)
				{
					eft[chn] = pchn->ldata[3];
					efp[chn] = pchn->ldata[4];
				}
			}
		}
		ptr++;
		
		sackit->pat_row++;
	}
	
	for(i = 0; i < 64; i++)
		sackit_update_effects_chn(sackit, &(sackit->pchn[i]),
			note[i], ins[i], vol[i], eft[i], efp[i]);
	
	sackit->pat_ptr = ptr;
}

/*

once again, ITTECH.TXT:

       ┌─────────────────────────────────────────────────────────┐
       │ Set note volume to volume set for each channel          │
       │ Set note frequency to frequency set for each channel    │
       └────────────┬────────────────────────────────────────────┘
                    │
       ┌────────────┴────────────┐
       │ Decrease tick counter   │        Yes
       │  Is tick counter 0 ?    ├─────────────────────────┐
       └────────────┬────────────┘                         │
                    │                                      │
                No  │                ┌─────────────────────┴──────────────────┐
       ┌────────────┴────────────┐   │ Tick counter = Tick counter set        │
       │ Update effects for each │   │                  (the current 'speed') │
       │  channel as required.   │   │      Decrease Row counter.             │
       │                         │   │        Is row counter 0?               │
       └───┬─────────────────────┘   └────────────┬──────────┬────────────────┘
           │                                  No  │          │
           │                ┌─────────────────────┘          │ Yes
           │                │                                │
           │  ┌─────────────┴──────────────┐ ┌───────────────┴────────────────┐
           │  │ Call update-effects for    │ │  Row counter = 1               │
           │  │ each channel.              │ │                                │
           │  └──────────────┬─────────────┘ │ Increase ProcessRow            │
           │                 │               │ Is ProcessRow > NumberOfRows?  │
           ├─────────────────┘               └────────┬──────────────────┬────┘
           │                                      Yes │                  │ No
           │         ┌────────────────────────────────┴──────────────┐   │
           │         │  ProcessRow = BreakRow                        │   │
           │         │  BreakRow = 0                                 │   │
           │         │  Increase ProcessOrder                        │   │
           │         │  while Order[ProcessOrder] = 0xFEh,           │   │
           │         │                         increase ProcessOrder │   │
           │         │  if Order[ProcessOrder] = 0xFFh,              │   │
           │         │                         ProcessOrder = 0      │   │
           │         │  CurrentPattern = Order[ProcessOrder]         │   │
           │         └─────────────────────┬─────────────────────────┘   │
           │                               │                             │
           │                               ├─────────────────────────────┘
           │                               │
           │         ┌─────────────────────┴──────────────────────────┐
           │         │ CurrentRow = ProcessRow                        │
           │         │ Update Pattern Variables (includes jumping to  │
           │         │  the appropriate row if requried and getting   │
           │         │  the NumberOfRows for the pattern)             │
           │         └─────────────────────┬──────────────────────────┘
           │                               │
           ├───────────────────────────────┘
           │
       ┌───┴───────────────┐       Yes        ┌───────────────────────────────┐
       │ Instrument mode?  ├──────────────────┤ Update Envelopes as required  │
       └───┬───────────────┘                  │ Update fadeout as required    │
           │                                  │ Calculate final volume if req │
           │ No (Sample mode)                 │ Calculate final pan if req    │
           │                                  │ Process sample vibrato if req │
       ┌───┴─────────────────────────────────┐└───────────────┬───────────────┘
       │ Calculate final volume if required  │                │
       │ Calculate final pan if requried     │                │
       │ Process sample vibrato if required  │                │
       └───┬─────────────────────────────────┘                │
           │                                                  │
           │                                                  │
           └─────────────────────────┬────────────────────────┘
                                     │
                    ┌────────────────┴──────────────────┐
                    │ Output sound!!!                   │
                    └───────────────────────────────────┘
*/


void sackit_tick(sackit_playback_t *sackit)
{
	int i;
	/*
	printf("%i %i %i %i %i\n"
		,sackit->current_tick
		,sackit->max_tick
		,sackit->process_row
		,sackit->process_order
		,sackit->current_pattern);*/
	
	for(i = 0; i < sackit->achn_count; i++)
	{
		sackit_achannel_t *achn = &(sackit->achn[i]);
		// Set note volume to volume set for each channel
		// TODO!
		
		// Set note frequency to frequency set for each channel
		achn->ofreq = achn->freq;
	}
	
	// Decrease tick counter
	sackit->current_tick--;
	
	// Is tick counter 0 ?
	if(sackit->current_tick == 0)
	{
		// Yes
		// Tick counter = Tick counter set (the current 'speed')
		sackit->current_tick = sackit->max_tick;
		
		// Decrease Row counter.
		sackit->row_counter--;
		
		// Is row counter 0?
		if(sackit->row_counter == 0)
		{
			// Yes
			// Row counter = 1
			sackit->row_counter = 1;
			
			// Increase ProcessRow
			sackit->process_row++;
			
			// Is ProcessRow > NumberOfRows?
			if(sackit->process_row >= sackit->number_of_rows)
			{
				// Yes
				// ProcessRow = BreakRow
				sackit->process_row = sackit->break_row;
				
				// BreakRow = 0
				sackit->break_row = 0;
				
				// Increase ProcessOrder
				sackit->process_order++;
				
				// while Order[ProcessOrder] = 0xFEh,
				// increase ProcessOrder
				while(sackit->module->orders[sackit->process_order] == 0xFE)
					sackit->process_order++;
				
				// if Order[ProcessOrder] = 0xFFh,
				// ProcessOrder = 0
				if(sackit->module->orders[sackit->process_order] == 0xFF)
					sackit->process_order = 0;
				
				// NOT LISTED ON CHART: Repeat the "while" loop --GM
				while(sackit->module->orders[sackit->process_order] == 0xFE)
					sackit->process_order++;
				
				// TODO: handle the case where we get 0xFF again
				
				// CurrentPattern = Order[ProcessOrder]
				sackit->current_pattern = sackit->module->orders[sackit->process_order];
				sackit->number_of_rows = sackit->module->patterns[sackit->current_pattern]->rows; 
				
				// NOT LISTED ON CHART: Set pattern row
				sackit->pat_row = -1;
			}
			
			// CurrentRow = ProcessRow
			sackit->current_row = sackit->process_row;
			
			// Update Pattern Variables (includes jumping to
			// the appropriate row if requried and getting 
			// the NumberOfRows for the pattern)
			sackit_update_pattern(sackit);
		} else {
			// No
			// Call update-effects for each channel. 
			sackit_update_effects(sackit);
		}
	} else {
		// No
		// Update effects for each channel as required.
		sackit_update_effects(sackit);
	}
	
	// ----------------------------------------------------
	
	// Instrument mode?
	// TODO!
	if(0)
	{
		// Yes
		// Update Envelopes as required
		// TODO!
		
		// Update fadeout as required
		// TODO!
	}
	/*
	from ITTECH.TXT:
	
	Abbreviations:
		FV = Final Volume (Ranges from 0 to 128). In versions 1.04+, mixed output
		devices are reduced further to a range from 0 to 64 due to lack of
		memory.
		Vol = Volume at which note is to be played. (Ranges from 0 to 64)
		SV = Sample Volume (Ranges from 0 to 64)
		IV = Instrument Volume (Ranges from 0 to 128)
		CV = Channel Volume (Ranges from 0 to 64)
		GV = Global Volume (Ranges from 0 to 128)
		VEV = Volume Envelope Value (Ranges from 0 to 64)
	
	In Sample mode, the following calculation is done:
		FV = Vol * SV * CV * GV / 262144
	Note that 262144 = 2^18 - So bit shifting can be done.
	
	In Instrument mode the following procedure is used:
		1) Update volume envelope value. Check for loops / end of envelope.
		2) If end of volume envelope (ie. position >= 200 or VEV = 0FFh), then turn
			on note fade.
		3) If notefade is on, then NoteFadeComponent (NFC) = NFC - FadeOut
			; NFC should be initialised to 1024 when a note is played.
		4) FV = Vol * SV * IV * CV * GV * VEV * NFC / 2^41
	*/
	
	for(i = 0; i < 64; i++)
	{
		// Calculate final volume if required
		// TODO!
		
		// Calculate final pan if requried
		// FIXME: Lacking stereo wavewriter! Can only guess here!
		// TODO!
		
		// Process sample vibrato if required
		// TODO!
	}
	
	// ----------------------------------------------------
	
	// Output sound!!!
	// -- handled elsewhere
}

void sackit_playback_update(sackit_playback_t *sackit)
{
	int offs = 0;
	
	while(offs+sackit->buf_tick_rem <= sackit->buf_len)
	{
		if(sackit->buf_tick_rem != 0)
		{
			sackit_playback_mixstuff_it211(sackit, offs, sackit->buf_tick_rem);
			offs += sackit->buf_tick_rem;
		}
		
		sackit_tick(sackit);
		// TODO: set freq somewhere
		sackit->buf_tick_rem = (44100*10)/(sackit->tempo*4);
	}
	
	if(offs != (int)sackit->buf_len)
	{
		sackit_playback_mixstuff_it211(sackit, offs, sackit->buf_len-offs);
		sackit->buf_tick_rem -= sackit->buf_len-offs;
	}
	//printf("rem %i row %i\n", sackit->buf_tick_rem, sackit->process_row);
}
