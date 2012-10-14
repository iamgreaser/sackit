RM = rm

CFLAGS = -g -fPIC -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable
LDFLAGS = -g -fPIC

CFLAGS_COMPARE = -g `sdl-config --cflags`
LDFLAGS_COMPARE = -g -lm `sdl-config --libs`

INCLUDES = sackit_internal.h sackit.h

OBJS_MIXER = mixer_it211.o
OBJS = effects.o fixedmath.o $(OBJS_MIXER) objects.o playroutine.o playroutine_effects.o tables.o

LIBSACKIT_SO = libsackit.so
SACKIT_COMPARE = sackit_compare

all: $(LIBSACKIT_SO) $(SACKIT_COMPARE)

clean:
	$(RM) -f $(OBJS)

$(SACKIT_COMPARE): libsackit.so app_compare.c
	$(CC) -o $(SACKIT_COMPARE) app_compare.c ./$(LIBSACKIT_SO) $(CFLAGS_COMPARE) $(LDFLAGS_COMPARE)

$(LIBSACKIT_SO): $(OBJS)
	$(CC) -shared -o $(LIBSACKIT_SO) $(OBJS) $(LDFLAGS)

%.o: %.c $(INCLUDES)
	$(CC) -c -o $@ $(CFLAGS) $<
