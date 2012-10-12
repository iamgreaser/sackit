RM = rm

CFLAGS = -g -fPIC -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable
LDFLAGS = -g -fPIC

CFLAGS_COMPARE = -g `sdl-config --cflags`
LDFLAGS_COMPARE = -g -lm `sdl-config --libs`

INCLUDES = sackit_internal.h sackit.h

OBJS_MIXER = mixer_it211.o
OBJS = effects.o fixedmath.o $(OBJS_MIXER) objects.o playroutine.o playroutine_effects.o tables.o

all: libsackit.so sackit_compare

clean:
	$(RM) -f libsackit.so
	$(RM) -f sackit_compare
	$(RM) -f $(OBJS)

sackit_compare: libsackit.so app_compare.c
	gcc -o sackit_compare app_compare.c ./libsackit.so $(CFLAGS_COMPARE) $(LDFLAGS_COMPARE)

libsackit.so: $(OBJS)
	gcc -shared -o libsackit.so $(OBJS) $(LDFLAGS)

%.o: %.c $(INCLUDES)
	$(CC) -c -o $@ $(CFLAGS) $<
