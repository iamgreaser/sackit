#include "sackit_internal.h"

void sackit_nna_note_cut(sackit_playback_t *sackit, sackit_achannel_t *achn)
{
	if(achn == NULL)
		return;
	
	achn->flags &= ~(
		SACKIT_ACHN_MIXING
		|SACKIT_ACHN_PLAYING
		|SACKIT_ACHN_SUSTAIN);
}

void sackit_nna_note_off(sackit_playback_t *sackit, sackit_achannel_t *achn)
{
	if(achn == NULL)
		return;
	
	achn->flags &= ~SACKIT_ACHN_SUSTAIN;
	
	if(achn->instrument != NULL)
	{
		it_instrument_t *cins = achn->instrument;
		if(cins->evol.flg & IT_ENV_ON)
		{
			if(cins->evol.flg & IT_ENV_LOOP)
				achn->flags |= SACKIT_ACHN_FADEOUT;
		} else {
			achn->flags |= SACKIT_ACHN_FADEOUT;
		}
	}
}

void sackit_nna_note_fade(sackit_playback_t *sackit, sackit_achannel_t *achn)
{
	if(achn == NULL)
		return;
	
	achn->flags |= SACKIT_ACHN_FADEOUT;
}

/*
from ITTECH.TXT:

The player in Impulse Tracker 'allocates' channels to notes whenever they
are *PLAYED*. In sample mode, the allocation is simple:
               Virtual Channel (number) = 'Host' channel (number)

In instrument mode, the following procedure is used:

    Check if channel is already playing ---Yes--> set 'background' flag on.
                |                                 'Trigger' NNA. If NNA=cut,
                No                                then use this virtual
                |                                 channel.
                |                                          |
                |<------------------ else -----------------/
                |
                v
    Search and find the first non-active virtual channel.
                |
    Non-active channel found? ----Yes----> Use this for playback.
                |
                No
                |
                v
   Search through and find the channel of lowest volume that is in the     #
   'background' (ie. no longer controlled directly)                        #
                |                                                          #
   Background channel found? ----Yes----> Use this for playback.           #
                |                                                          #
                No                                                         #
                |                                                          #
                v                                                          #
   Return error - the note is *NOT* allocated a channel, and hence is not  #
   played.                                                                 #

   This is actually quite a simple process... just that it's another of
   those 'hassles' to have to write...

   ### Note: This is by far the simplest implementation of congestion
             resolution. IT 2.03 and above have a greatly enhanced
             method which more selectively removes the most insignificant
             channel. Obviously, there is no best way to do this - I
             encourage you to experiment and find new algorithms for
             yourself.
*/

void sackit_nna_allocate(sackit_playback_t *sackit, sackit_pchannel_t *pchn)
{
	int i;
	
	sackit_achannel_t *old_achn = NULL;
	// Check if playing
	if(pchn->achn != NULL)
	{
		old_achn = pchn->achn;
		
		if(old_achn->instrument == NULL || old_achn->instrument->nna == 0)
		{
			sackit_nna_note_cut(sackit, old_achn);
			return;
		}
		//if(!(old_achn->flags & SACKIT_ACHN_PLAYING))
		//	return;
		
		if(old_achn->instrument != NULL)
		{
			if(old_achn->instrument->nna == 2)
				sackit_nna_note_off(sackit, old_achn);
			if(old_achn->instrument->nna == 3)
				sackit_nna_note_fade(sackit, old_achn);
		}
		
		old_achn->flags |= SACKIT_ACHN_BACKGND;
		pchn->achn = NULL;
	}
	
	// Search and find the first non-active virtual channel.
	for(i = 0; i < sackit->achn_count; i++)
	{
		sackit_achannel_t *achn = &(sackit->achn[i]);
		if(!(achn->flags & SACKIT_ACHN_PLAYING))
		{
			if(achn->parent != NULL)
				achn->parent->achn = NULL;
			if(achn->prev != NULL)
				achn->prev->next = achn->next;
			if(achn->next != NULL)
				achn->next->prev = achn->prev;
			
			sackit_playback_reset_achn(achn);
			pchn->achn = achn;
			achn->parent = pchn;
			
			achn->prev = NULL;
			achn->next = old_achn;
			if(old_achn != NULL)
				old_achn->prev = achn;
			
			//printf("%i\n", i);
			return;
		}
	}
	
	// Search through and find the channel of lowest volume that is in the
	// 'background' (ie. no longer controlled directly)
	
	// TODO!
	fprintf(stderr, "TODO: find background channel\n");
	abort();
	for(i = 0; i < sackit->achn_count; i++)
	{
		sackit_achannel_t *achn = &(sackit->achn[i]);
	}
	
}
