#!/bin/sh
make -f Makefile APPSUFFIX=.exe SDL_CFLAGS=-Iwinlibs/ -Iwinlibs/SDL/ SDL_LDFLAGS=-Lwinlibs/ -lmingw32 -lSDLmain -lSDL CC=i686-pc-mingw32-gcc RANLIB=i686-pc-mingw32-ranlib LIBSACKIT_SO=libsackit.dll LIBSACKIT_A=libsackit-w32.a

