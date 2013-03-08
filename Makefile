RM = rm

CFLAGS = -g -fPIC -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable
LDFLAGS = -g -fPIC

CFLAGS_COMPARE = -g `sdl-config --cflags`
LDFLAGS_COMPARE = -g -lm `sdl-config --libs`
CFLAGS_PLAY = -g `sdl-config --cflags`
LDFLAGS_PLAY = -g -lm `sdl-config --libs`

INCLUDES = sackit_internal.h sackit.h

OBJS_MIXER = mixer.o
OBJS = effects.o fixedmath.o $(OBJS_MIXER) objects.o playroutine.o playroutine_effects.o playroutine_nna.o tables.o

LIBSACKIT_SO = libsackit.so
SACKIT_COMPARE = sackit_compare
SACKIT_PLAY = sackit_play

all: $(LIBSACKIT_SO) $(SACKIT_COMPARE) $(SACKIT_PLAY)

clean:
	$(RM) -f $(OBJS)

$(SACKIT_COMPARE): $(LIBSACKIT_SO) app_compare.c
	$(CC) -o $(SACKIT_COMPARE) app_compare.c ./$(LIBSACKIT_SO) $(CFLAGS_COMPARE) $(LDFLAGS_COMPARE)

$(SACKIT_PLAY): $(LIBSACKIT_SO) app_play.c
	$(CC) -o $(SACKIT_PLAY) app_play.c ./$(LIBSACKIT_SO) $(CFLAGS_PLAY) $(LDFLAGS_PLAY)

$(LIBSACKIT_SO): $(OBJS)
	$(CC) -shared -o $(LIBSACKIT_SO) $(OBJS) $(LDFLAGS)

mixer.o: mixer.c mixer_*.h $(INCLUDES)
	$(CC) -c -o mixer.o $(CFLAGS) mixer.c

%.o: %.c $(INCLUDES)
	$(CC) -c -o $@ $(CFLAGS) $<
