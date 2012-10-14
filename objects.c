#include "sackit_internal.h"

it_module_t *sackit_module_new(void)
{
	int i;
	
	it_module_t *module = malloc(sizeof(it_module_t));
	
	for(i = 0; i < MAX_ORDERS; i++)
		module->orders[i] = 0xFF;
	for(i = 0; i < MAX_INSTRUMENTS; i++)
		module->instruments[i] = NULL;
	for(i = 0; i < MAX_SAMPLES; i++)
		module->samples[i] = NULL;
	for(i = 0; i < MAX_PATTERNS; i++)
		module->patterns[i] = NULL;
	
	return module;
}

void sackit_module_free(it_module_t *module)
{
	// TODO!
	
	free(module);
}

it_module_t *sackit_module_load(char *fname)
{
	int i,j;
	
	// open file
	FILE *fp = fopen(fname, "rb");
	if(fp == NULL)
	{
		perror("sackit_module_load");
		return NULL;
	}
	
	// create module
	it_module_t *module = sackit_module_new();
	
	// load header
	if(fread(&(module->header), sizeof(it_module_header_t), 1, fp) != 1)
	{
		fprintf(stderr, "sackit_module_load: could not read header\n");
		fclose(fp);
		sackit_module_free(module);
		return NULL;
	}
	
	// check magic
	if(memcmp(module->header.magic, "IMPM", 4))
	{
		fprintf(stderr, "sackit_module_load: invalid magic\n");
		fclose(fp);
		sackit_module_free(module);
		return NULL;
	}
	
	// sanity checks
	if(module->header.ordnum > MAX_ORDERS
		|| module->header.insnum > MAX_INSTRUMENTS
		|| module->header.smpnum > MAX_SAMPLES
		|| module->header.patnum > MAX_PATTERNS)
	{
		fprintf(stderr, "sackit_module_load: header limits exceeded\n");
		fclose(fp);
		sackit_module_free(module);
		return NULL;
	}
	
	module->header.song_name[25] = 0x00;
	printf("module name: \"%s\"\n", module->header.song_name);
	
	if(fread(module->orders, module->header.ordnum, 1, fp) != 1)
	{
		fprintf(stderr, "sackit_module_load: could not read orderlist\n");
		fclose(fp);
		sackit_module_free(module);
		return NULL;
	}
	
	static uint32_t offset_instruments[MAX_INSTRUMENTS];
	static uint32_t offset_samples[MAX_SAMPLES];
	static uint32_t offset_patterns[MAX_PATTERNS];
	
	if((module->header.insnum != 0 && fread(offset_instruments, module->header.insnum*4, 1, fp) != 1)
		|| (module->header.smpnum != 0 && fread(offset_samples, module->header.smpnum*4, 1, fp) != 1)
		|| (module->header.patnum != 0 && fread(offset_patterns, module->header.patnum*4, 1, fp) != 1))
	{
		fprintf(stderr, "sackit_module_load: could not read pointers from header\n");
		fclose(fp);
		sackit_module_free(module);
		return NULL;
	}
	
	// instruments
	for(i = 0; i < module->header.insnum; i++)
	{
		fseek(fp, offset_instruments[i], SEEK_SET);
		module->instruments[i] = malloc(sizeof(it_instrument_t));
		fread(module->instruments[i], sizeof(it_instrument_t), 1, fp);
	}
	
	// samples
	for(i = 0; i < module->header.smpnum; i++)
	{
		fseek(fp, offset_samples[i], SEEK_SET);
		it_sample_t *smp = malloc(sizeof(it_sample_t));
		module->samples[i] = smp;
		fread(smp, sizeof(it_sample_t)-sizeof(int16_t *), 1, fp);
		
		smp->data = NULL;
		if(smp->samplepointer != 0 && smp->length != 0 && (smp->flg & IT_SMP_EXISTS))
		{
			// NO WE ARE NOT SUPPORTING STEREO SAMPLES PISS OFF MODPLUG
			fseek(fp, smp->samplepointer, SEEK_SET);
			smp->data = malloc(smp->length*sizeof(int16_t));
			
			// check compression flag
			if(smp->flg & IT_SMP_COMPRESS)
			{
				// FIXME: YES WE *WILL* HAVE THIS!
				fprintf(stderr, "TODO: IT214-compressed samples\n");
				abort();
			} else {
				// load
				if(smp->flg & IT_SMP_16BIT)
				{
					fread(smp->data, smp->length*2, 1, fp);
				} else {
					for(j = 0; j < (int)smp->length; j++)
						smp->data[j] = (fgetc(fp))<<8;
				}
				
				// convert
				if(!(smp->cvt & 0x01))
				{
					// TODO!
				}
				
				// TODO: other conversion flags
			}
		}
	}
	
	// patterns
	for(i = 0; i < module->header.patnum; i++)
	{
		fseek(fp, offset_patterns[i], SEEK_SET);
		module->patterns[i] = malloc(sizeof(it_pattern_t));
		fread(module->patterns[i], sizeof(it_pattern_t)-65536, 1, fp);
		fread(8+(uint8_t *)module->patterns[i], module->patterns[i]->length, 1, fp);
	}
	
	fclose(fp);
	
	return module;
}

void sackit_playback_free(sackit_playback_t *sackit)
{
	// TODO!
	
	free(sackit);
}

void sackit_playback_reset_env(sackit_envelope_t *aenv, int8_t def)
{
	aenv->carry_idx = aenv->idx = 0;
	aenv->carry_x = aenv->x = 0;
	aenv->y = aenv->def = def;
	aenv->carry_flags = aenv->flags = 0;
}

void sackit_playback_reset_achn(sackit_achannel_t *achn)
{
	achn->freq = 0;
	achn->ofreq = 0;
	achn->offs = 0;
	achn->suboffs = 0;
	
	achn->flags = 0;
	
	achn->instrument = NULL;
	achn->sample = NULL;
	
	achn->sv = 0;
	achn->vol = 0;
	achn->fv = 0;
	achn->cv = 0;
	achn->iv = 128;
	
	achn->lramp = 0;
	
	achn->fadeout = 1024;
	
	achn->next = achn->prev = NULL;
	achn->parent = NULL;
	
	sackit_playback_reset_env(&(achn->evol), 64);
	sackit_playback_reset_env(&(achn->epan), 0);
	sackit_playback_reset_env(&(achn->epitch), 0);
}

void sackit_playback_reset_pchn(sackit_pchannel_t *pchn)
{
	pchn->achn = NULL;
	pchn->tfreq = 0;
	pchn->nfreq = 0;
	pchn->freq = 0;
	pchn->note = 253;
	pchn->lins = 0;
	
	pchn->cv = 64;
	pchn->vol = 0;
	
	pchn->slide_vol = 0;
	pchn->slide_vol_cv = 0;
	pchn->slide_vol_gv = 0;
	pchn->slide_pan = 0;
	pchn->slide_pitch = 0;
	pchn->slide_porta = 0;
	pchn->arpeggio = 0;
	pchn->note_cut = 0;
	pchn->note_delay = 0;
	pchn->vib_speed = 0;
	pchn->vib_depth = 0;
	pchn->vib_offs = 0;
	pchn->vib_type = 0;
	pchn->vib_lins = 0;
	
	pchn->eff_slide_vol = 0;
	pchn->eff_slide_vol_cv = 0;
	pchn->eff_slide_vol_gv = 0;
	pchn->eff_slide_pitch = 0;
	pchn->eff_slide_porta = 0;
	pchn->eff_sample_offs = 0;
	pchn->eff_misc = 0;
	pchn->eff_arpeggio = 0;
	pchn->eff_vibrato = 0;
	pchn->eff_tempo = 0;
	pchn->eff_slide_vol_veff = 0;
	
	pchn->instrument = NULL;
	pchn->sample = NULL;
}

void sackit_playback_reset(sackit_playback_t *sackit)
{
	int i;
	
	sackit->current_tick = 1;
	sackit->max_tick = sackit->module->header.is;
	sackit->row_counter = 1;
	
	sackit->current_row = 0xFFFE;
	sackit->process_row = 0xFFFE;
	sackit->break_row = 0;
	sackit->number_of_rows = 64;
	
	sackit->current_pattern = 0;
	sackit->process_order = -1;
	
	sackit->pat_ptr = 0;
	sackit->pat_row = 0;
	
	sackit->gv = sackit->module->header.gv;
	sackit->mv = sackit->module->header.mv;
	
	sackit->tempo = sackit->module->header.it;
	
	sackit->achn_count = 256; // TODO: set this somewhere else!
	sackit->buf_len = SACKIT_TEST_AU_BUFFER; // TODO: same deal!
	sackit->buf_tick_rem = 0;
	sackit->buf = malloc(sizeof(int16_t)*2*sackit->buf_len);
	sackit->mixbuf = malloc(sizeof(int32_t)*2*sackit->buf_len);
	
	for(i = 0; i < SACKIT_MAX_ACHANNEL; i++)
		sackit_playback_reset_achn(&(sackit->achn[i]));
	for(i = 0; i < 64; i++)
	{
		sackit_playback_reset_pchn(&(sackit->pchn[i]));
		
		/*sackit->pchn[i].achn = &(sackit->achn[i]);
		sackit->pchn[i].achn->parent = &(sackit->pchn[i]);*/
		
		sackit->pchn[i].cv = sackit->module->header.chnl_vol[i];
	}
}

sackit_playback_t *sackit_playback_new(it_module_t *module)
{
	// allocate
	sackit_playback_t *sackit = malloc(sizeof(sackit_playback_t));
	sackit->module = module;
	sackit_playback_reset(sackit);
	
	return sackit;
}
