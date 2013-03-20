#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>

#include <SDL.h>

#include "sackit.h"

SDL_Surface *screen = NULL;

uint32_t palette[4] = {
	0x00FFFFFF,
	0x00FF0000,
	0x0000FF00,
	0x000000FF,
};

volatile int sound_ready = 1;
volatile int16_t *sound_buf = NULL;
int16_t *sound_queue = NULL;
int sound_queue_pos = (int)(((unsigned int)-1)>>1);

void test_sdl_callback(void *userdata, Uint8 *stream, int len)
{
	int offs = 0;
	sackit_playback_t *sackit = (sackit_playback_t *)userdata;
	int16_t *outbuf = (int16_t *)stream;
	int16_t *nvbuf = (int16_t *)sound_buf;
	
	len /= 4;
	
	while(offs < len)
	{
		if(sound_queue_pos < 4096)
		{
			int xlen = 4096-sound_queue_pos;
			if(xlen > len-offs)
				xlen = len;
			
			memcpy(&stream[offs*4], &sound_queue[sound_queue_pos*2], xlen*4);
			sound_queue_pos += xlen;
			offs += xlen;
		} else {
			memcpy(sound_queue, nvbuf, 4096*4);
			sound_queue_pos = 0;
			sound_ready = 1;
		}
	}
}


int main(int argc, char *argv[])
{
	int x,y,i;
	
	it_module_t *module = sackit_module_load(argv[1]);
	
	if(module == NULL)
		return 1;
	
	SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_TIMER|SDL_INIT_NOPARACHUTE);
	
	SDL_WM_SetCaption("sackit IT player", NULL);
	screen = SDL_SetVideoMode(800, 600, 32, 0);
	
	// draw something
	uint32_t *pbuf = screen->pixels;
	int divpitch = screen->pitch/sizeof(uint32_t);
	for(y = 0; y < screen->h; y++)
		for(x = 0; x < screen->w; x++)
			pbuf[divpitch*y+x] = 0x00000000;
	
	sackit_playback_t *sackit = sackit_playback_new(module, 4096, 256, MIXER_IT212S);
	
	SDL_AudioSpec aspec;
	aspec.freq = 44100;
	aspec.format = AUDIO_S16SYS;
	aspec.channels = 2;
	aspec.samples = 4096;
	aspec.callback = test_sdl_callback;
	sound_buf = calloc(1,4096*4);
	sound_queue = calloc(1,4096*4);
	SDL_OpenAudio(&aspec, NULL);
	SDL_PauseAudio(0);
	
	int refoffs = 0;
	
	int play_a_sound = 1;
	
	int quitflag = 0;
	while(!quitflag)
	{
		SDL_Event ev;
		while(SDL_PollEvent(&ev))
		switch(ev.type)
		{
			case SDL_KEYDOWN:
				play_a_sound = 1;
				break;
			case SDL_QUIT:
				quitflag = 1;
				break;
		}
		
		if(play_a_sound && sound_ready)
		{
			//play_a_sound = 0;
			sackit_playback_update(sackit);
			
			// VISUALISE
			memset(screen->pixels, 0, screen->pitch*screen->h);
			for(x = 0; x < screen->w*2; x++)
			{
				int yb = sackit->buf[x];
				
				if((x&1) == 0)
				{
					y = 0;
					y = (y+0x8000)*screen->h/0x10000;
					y = screen->h-1-y;
					pbuf[divpitch*y+(x>>1)] = 0xFFFFFF;
				}
				
				y = yb;
				y = (y+0x8000)*screen->h/0x10000;
				y = screen->h-1-y;
				pbuf[divpitch*y+(x>>1)] |= ((x&1) ? 0x0000FF : 0xFF0000);
			}
			
			SDL_Flip(screen);
			
			int16_t *nvbuf = (int16_t *)sound_buf;
			memcpy(nvbuf, sackit->buf, 4096*4);
			sound_ready = 0;
		}
		
		SDL_Delay(10);
	}
	
	sackit_playback_free(sackit);
	sackit_module_free(module);

	free(sound_buf);
	free(sound_queue);

	// to help shut valgrind up
	SDL_Quit();
	
	return 0;
}
