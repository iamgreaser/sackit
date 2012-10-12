#include "sackit_internal.h"

void sackit_playback_mixstuff_it211(sackit_playback_t *sackit, int offs, int len)
{
	uint32_t tfreq = 44100; // TODO define this elsewhere
	
	int i,j;
	int offsend = offs+len;
	
	int16_t *buf = &(sackit->buf[offs]);
	int32_t *mixbuf = (int32_t *)&(sackit->mixbuf[offs]);
	
	// just a guess :)
	//uint32_t rampspd = (65536*488)/44100;
	uint32_t rampspd = 488;
	
	int32_t gvol = sackit->gv; // 7
	int32_t mvol = sackit->mv; // 7
	
	for(j = 0; j < len; j++)
		mixbuf[j] = 0;

	for(i = 0; i < sackit->achn_count; i++)
	{
		sackit_achannel_t *achn = &(sackit->achn[i]);
		
		if(achn->sample == NULL || achn->sample->data == NULL
			|| achn->offs >= achn->sample->length
			|| achn->offs < 0)
		{
			achn->flags &= ~(
				SACKIT_ACHN_RAMP
				|SACKIT_ACHN_MIXING
				|SACKIT_ACHN_PLAYING
				|SACKIT_ACHN_SUSTAIN);
		}
		
		uint32_t rampmul = tfreq;
		
		if(achn->flags & SACKIT_ACHN_RAMP)
		{
			achn->flags &= ~SACKIT_ACHN_RAMP;
			rampmul = 0;
			
			//printf("ramp %i %i %i\n", i, rampspd, (32768+rampspd-1)/rampspd);
		}
		
		if(achn->flags & SACKIT_ACHN_MIXING)
		{
			int32_t zoffs = achn->offs;
			int32_t zsuboffs = achn->suboffs;
			int32_t zfreq = achn->ofreq;
			
			zfreq = sackit_div_int_32_32_to_fixed_16(zfreq,tfreq);
			
			//printf("freq %i %i %i\n", zfreq, zoffs, zsuboffs);
			
			uint32_t zlpbeg = achn->sample->loop_begin;
			uint32_t zlpend = achn->sample->loop_end;
			uint32_t zlength = achn->sample->length;
			uint8_t zflg = achn->sample->flg;
			int16_t *zdata = achn->sample->data;
			
			if((achn->flags & SACKIT_ACHN_SUSTAIN)
				&& (zflg & IT_SMP_SUSLOOP))
			{
				zlpbeg = achn->sample->susloop_begin;
				zlpend = achn->sample->susloop_end;
				zflg |= IT_SMP_LOOP;
				zflg &= ~IT_SMP_LOOPBIDI;
				if(zflg & IT_SMP_SUSBIDI)
				{
					zflg |= IT_SMP_LOOPBIDI;
				} else {
					zflg &= ~IT_SMP_LOOPBIDI;
					achn->flags &= ~SACKIT_ACHN_REVERSE;
				}
			}
			
			// TODO: sanity check somewhere!
			if(zflg & IT_SMP_LOOP)
				zlength = zlpend;
			if(achn->flags & SACKIT_ACHN_REVERSE)
				zfreq = -zfreq;
			
			int32_t vol = 0x8000;
			vol = ((int32_t)achn->vol) // 6
				*((int32_t)achn->sv) // 6
				*((int32_t)achn->cv) // 6
				*gvol; // 7
			vol >>= 10;
			
			for(j = 0; j < len; j++)
			{
				// get sample value
				int32_t v0 = zdata[zoffs];
				int32_t v1 = zdata[(uint32_t)(zoffs+1) == zlength
					? (zflg & IT_SMP_LOOP
						? zlpbeg
						: 0)
					: (uint32_t)(zoffs+1)];
				int32_t v  = ((v0*((65535-zsuboffs)))>>16)
					+ ((v1*(zsuboffs))>>16);
				//int32_t v = v0 + (((v1-v0)*(zsuboffs>>1))>>15);
				
				if(rampmul < tfreq)
				{
					int32_t rampvol = (rampmul<<16)/tfreq;
					v = (v*rampvol)>>16;
					rampmul += rampspd;
				}
				
				v = ((v*vol)>>16);
				
				// mix
				mixbuf[j] -= v;
				
				// update
				zsuboffs += zfreq;
				zoffs += (((int32_t)zsuboffs)>>16);
				zsuboffs &= 0xFFFF;
				
				if(zoffs >= (int32_t)zlength)
				{
					// TODO: ping-pong/bidirectional loops
					// TODO? speed up for tiny loops?
					if(zflg & IT_SMP_LOOP)
					{
						if(zflg & IT_SMP_LOOPBIDI)
						{
							if(zfreq > 0)
							{
								zoffs = zlpend*2-1-zoffs;
								zfreq = -zfreq;
								zsuboffs = 0x10000-zsuboffs;
								achn->flags |= SACKIT_ACHN_REVERSE;
							} else {
								zoffs = zlpbeg*2-zoffs;
								zfreq = -zfreq;
								zsuboffs = 0x10000-zsuboffs;
								achn->flags &= ~SACKIT_ACHN_REVERSE;
							}
						} else {
							while(zoffs >= (int32_t)zlpend)
								zoffs += (zlpbeg-zlpend);
						}
					} else {
						achn->flags &= ~(
							 SACKIT_ACHN_MIXING
							|SACKIT_ACHN_PLAYING
							|SACKIT_ACHN_SUSTAIN);
						break;
					}
				}
			}
			
			achn->offs = zoffs;
			achn->suboffs = zsuboffs;
		} else if(achn->flags & SACKIT_ACHN_PLAYING) {
			// TODO: update offs/suboffs
		}
	}
	
	// stick into the buffer
	for(j = 0; j < len; j++)
	{
		int32_t bv = mixbuf[j];
		bv = (bv*mvol)>>7;
		if(bv < -32768) bv = -32768;
		else if(bv > 32767) bv = 32767;
		
		buf[j] = bv;
	}
}
