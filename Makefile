RM = rm
RANLIB = ranlib

APPSUFFIX =

CFLAGS = -g -fPIC -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable
LDFLAGS = -g -fPIC -lm

SDL_CFLAGS = `sdl-config --cflags`
SDL_LDFLAGS = `sdl-config --libs`
CFLAGS_COMPARE = -g $(SDL_CFLAGS)
LDFLAGS_COMPARE = -g -lm $(SDL_LDFLAGS)
CFLAGS_PLAY = -g $(SDL_CFLAGS)
LDFLAGS_PLAY = -g -lm $(SDL_LDFLAGS)
CFLAGS_SFX = -g $(SDL_CFLAGS)
LDFLAGS_SFX = -g -lm $(SDL_LDFLAGS)
CFLAGS_CONVERT = -g
LDFLAGS_CONVERT = -g -lm

INCLUDES = sackit_internal.h sackit.h

OBJS_MIXER = mixer.o
OBJS = effects.o fixedmath.o $(OBJS_MIXER) objects.o playroutine.o playroutine_effects.o playroutine_nna.o tables.o

LIBSACKIT_SO = libsackit.so
LIBSACKIT_A = libsackit.a
SACKIT_COMPARE = sackit_compare$(APPSUFFIX)
SACKIT_PLAY = sackit_play$(APPSUFFIX)
SACKIT_SFX = sackit_sfx$(APPSUFFIX)
SACKIT_CONVERT = sackit_convert$(APPSUFFIX)

all: $(LIBSACKIT_SO) $(LIBSACKIT_A) $(SACKIT_COMPARE) $(SACKIT_PLAY) $(SACKIT_SFX) $(SACKIT_CONVERT)

clean:
	$(RM) -f $(OBJS)

$(SACKIT_COMPARE): $(LIBSACKIT_A) app_compare.c
	$(CC) -o $(SACKIT_COMPARE) app_compare.c ./$(LIBSACKIT_A) $(CFLAGS_COMPARE) $(LDFLAGS_COMPARE)

$(SACKIT_PLAY): $(LIBSACKIT_A) app_play.c
	$(CC) -o $(SACKIT_PLAY) app_play.c ./$(LIBSACKIT_A) $(CFLAGS_PLAY) $(LDFLAGS_PLAY)

$(SACKIT_SFX): $(LIBSACKIT_A) app_sfx.c
	$(CC) -o $(SACKIT_SFX) app_sfx.c ./$(LIBSACKIT_A) $(CFLAGS_SFX) $(LDFLAGS_SFX)

$(SACKIT_CONVERT): $(LIBSACKIT_A) app_convert.c
	$(CC) -o $(SACKIT_CONVERT) app_convert.c ./$(LIBSACKIT_A) $(CFLAGS_CONVERT) $(LDFLAGS_CONVERT)

$(LIBSACKIT_SO): $(OBJS)
	$(CC) -shared -o $(LIBSACKIT_SO) $(OBJS) $(LDFLAGS)

$(LIBSACKIT_A): $(OBJS)
	$(AR) rf $(LIBSACKIT_A) $(OBJS) 
	$(RANLIB) $(LIBSACKIT_A)

mixer.o: mixer.c mixer_*.h $(INCLUDES)
	$(CC) -c -o mixer.o $(CFLAGS) mixer.c

%.o: %.c $(INCLUDES)
	$(CC) -c -o $@ $(CFLAGS) $<
