REV=$(shell sh -c 'git rev-parse --short @{0}')
CFLAGS=-pedantic -Wall -Os -s -Wall -Ires -DREV=\"$(REV)\"

all: renlauncher.exe

renlauncher.exe: main.c
	sed 's/__REV__/$(REV)/g' res/renlauncher.rc | i686-w64-mingw32-windres -o res/renlauncher.rc.o
	i686-w64-mingw32-gcc -nostdlib $(CFLAGS) -mwindows -o renlauncher.exe main.c res/renlauncher.rc.o -lcomctl32
	i686-w64-mingw32-strip -s renlauncher.exe

clean:
	rm -f renlauncher.exe res/renlauncher.o