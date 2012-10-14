#include "sackit_internal.h"

void sackit_playback_mixstuff_it211(sackit_playback_t *sackit, int offs, int len)
{
	uint32_t tfreq = 44100; // TODO define this elsewhere
	
	int i,j;
	int offsend = offs+len;
	
	int16_t *buf = &(sackit->buf[offs]);
	int32_t *mixbuf = (int32_t *)&(sackit->mixbuf[offs]);
	
	// just a guess :)
	int32_t ramplen = tfreq/500+1;
	
	int32_t gvol = sackit->gv; // 7
	int32_t mvol = sackit->mv; // 7
	
	for(j = 0; j < len; j++)
		mixbuf[j] = 0;

	for(i = 0; i < sackit->achn_count; i++)
	{
		sackit_achannel_t *achn = &(sackit->achn[i]);
		
		if(achn->sample == NULL || achn->sample->data == NULL
			|| achn->offs >= (int32_t)achn->sample->length
			|| achn->offs < 0)
		{
			achn->flags &= ~(
				SACKIT_ACHN_RAMP
				|SACKIT_ACHN_MIXING
				|SACKIT_ACHN_PLAYING
				|SACKIT_ACHN_SUSTAIN);
		}
		
		if(achn->flags & SACKIT_ACHN_RAMP)
		{
			achn->flags &= ~SACKIT_ACHN_RAMP;
			//ramprem = rampspd;
			achn->lramp = 0;
			
			//printf("ramp %i %i %i\n", i, rampspd, (32768+rampspd-1)/rampspd);
			//printf("ramp %i %i %i\n", i, rampinc, ramprem);
		}
		
		if(achn->flags & SACKIT_ACHN_MIXING)
		{
			int32_t zoffs = achn->offs;
			int32_t zsuboffs = achn->suboffs;
			int32_t zfreq = achn->ofreq;
			int32_t zlramp = achn->lramp;
			
			zfreq = sackit_div_int_32_32_to_fixed_16(zfreq,tfreq);
			
			//printf("freq %i %i %i\n", zfreq, zoffs, zsuboffs);
			
			int32_t zlpbeg = achn->sample->loop_begin;
			int32_t zlpend = achn->sample->loop_end;
			int32_t zlength = achn->sample->length;
			uint8_t zflg = achn->sample->flg;
			int16_t *zdata = achn->sample->data;
			
			if((achn->flags & SACKIT_ACHN_SUSTAIN)
				&& (zflg & IT_SMP_SUSLOOP))
			{
				zlpbeg = achn->sample->susloop_begin;
				zlpend = achn->sample->susloop_end;
				zflg |= IT_SMP_LOOP;
				if(zflg & IT_SMP_SUSBIDI)
				{
					zflg |= IT_SMP_LOOPBIDI;
				} else {
					zflg &= ~IT_SMP_LOOPBIDI;
				}
			}
			
			if(!(zflg & IT_SMP_LOOPBIDI))
				achn->flags &= ~SACKIT_ACHN_REVERSE;
			
			// TODO: sanity check somewhere!
			if(zflg & IT_SMP_LOOP)
				zlength = zlpend;
			if(achn->flags & SACKIT_ACHN_REVERSE)
				zfreq = -zfreq;
			
			int32_t vol = 0x8000;
			/*vol = ((int32_t)achn->vol) // 6
				*((int32_t)achn->sv) // 6
				*((int32_t)achn->cv) // 6
				*gvol // 7
			;
			//vol += (1<<9);
			vol >>= 10;
			vol = (vol*mvol)>>7; // 7*/
			// TODO: sort the order / rounding out
			vol = (vol*((int32_t)achn->vol))>>6;
			vol = (vol*((int32_t)achn->sv))>>6;
			vol = (vol*((int32_t)achn->cv))>>6;
			vol = (vol*((int32_t)achn->evol.y))>>6;
			vol = (vol*((int32_t)achn->iv))>>6;
			vol = (vol*gvol)>>7;
			vol = (vol*mvol)>>7;
			//vol += 0x0080;
			//vol &= 0x7F00;
			
			achn->lramp = vol;
			
			int32_t rampmul = zlramp;
			int32_t ramprem = ramplen;
			int32_t rampdelta = (vol-zlramp);
			int negdepth = (rampdelta < 0);
			int32_t rampdelta_i = rampdelta;
			if(negdepth)
				rampdelta = -rampdelta;
			int32_t rampspd = (rampdelta+0x0080)&~0x00FF;
			
			rampspd = rampspd / (ramplen+1);
			
			rampspd >>= 2;
			rampspd <<= 2;
			
			if(negdepth)
			{
				rampspd = -rampspd;
				//rampspd -= 4;
			}
			
			/*
			if(rampdelta != 0)
				printf("%5i %04X / %5i %04X mod90 is %5i => %5i \n", vol, vol
					, rampdelta
					, rampdelta_i&0xFFFF
					, (rampdelta_i+(ramplen+1)*0x10000) % (ramplen+1)
					, rampspd);*/
			/*
			Ramp speeds:
			06BF NOT 16
			0B40 = 32
			0CC0 = 36
			0F00 = 40
			1200 = 48
			1800 = 68
			1E00 = 84
			1ED8 NOT 84 (it's 88!)
			27C0 = 112
			3000 = 136
			
			D000 = -136
			E800 NOT -68
			EE00 = -48
			F400 = -32
			F4C0 (?) -28
			FA00 (?) -16
			*/
			
			//printf("%i\n", rampspd);
			for(j = 0; j < len; j++)
			{
				// get sample value
				int32_t v0 = zdata[zoffs];
				int32_t v1 = ((zoffs+1) == zlength
					? (zflg & IT_SMP_LOOP
						? zdata[zlpbeg]
						: 0)
					: zdata[(zoffs+1)]);
				int32_t v  = ((v0*((65535-zsuboffs)))>>16)
					+ ((v1*(zsuboffs))>>16);
				//int32_t v = v0 + (((v1-v0)*(zsuboffs>>1))>>15);
				
				if(ramprem > 0)
				{
					v = (v*rampmul+0x8000)>>16;
					rampmul += rampspd;
					ramprem--;
				} else {
					v = ((v*vol+0x8000)>>16);
				}
				
				// mix
				mixbuf[j] += v;
				
				// update
				zsuboffs += zfreq;
				zoffs += (((int32_t)zsuboffs)>>16);
				zsuboffs &= 0xFFFF;
				
				if((zfreq < 0
					? zoffs < zlpbeg
					: zoffs >= (int32_t)zlength))
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
		int32_t bv = -mixbuf[j];
		//bv = (bv*mvol+64)>>7;
		if(bv < -32768) bv = -32768;
		else if(bv > 32767) bv = 32767;
		
		buf[j] = bv;
	}
}
