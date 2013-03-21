RM = rm
RANLIB = ranlib

CFLAGS = -g -fPIC -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable
LDFLAGS = -g -fPIC

CFLAGS_COMPARE = -g `sdl-config --cflags`
LDFLAGS_COMPARE = -g -lm `sdl-config --libs`
CFLAGS_PLAY = -g `sdl-config --cflags`
LDFLAGS_PLAY = -g -lm `sdl-config --libs`
CFLAGS_CONVERT = -g
LDFLAGS_CONVERT = -g

INCLUDES = sackit_internal.h sackit.h

OBJS_MIXER = mixer.o
OBJS = effects.o fixedmath.o $(OBJS_MIXER) objects.o playroutine.o playroutine_effects.o playroutine_nna.o tables.o

LIBSACKIT_SO = libsackit.so
LIBSACKIT_A = libsackit.a
SACKIT_COMPARE = sackit_compare
SACKIT_PLAY = sackit_play
SACKIT_CONVERT = sackit_convert

all: $(LIBSACKIT_SO) $(LIBSACKIT_A) $(SACKIT_COMPARE) $(SACKIT_PLAY) $(SACKIT_CONVERT)

clean:
	$(RM) -f $(OBJS)

$(SACKIT_COMPARE): $(LIBSACKIT_A) app_compare.c
	$(CC) -o $(SACKIT_COMPARE) app_compare.c ./$(LIBSACKIT_A) $(CFLAGS_COMPARE) $(LDFLAGS_COMPARE)

$(SACKIT_PLAY): $(LIBSACKIT_A) app_play.c
	$(CC) -o $(SACKIT_PLAY) app_play.c ./$(LIBSACKIT_A) $(CFLAGS_PLAY) $(LDFLAGS_PLAY)

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
