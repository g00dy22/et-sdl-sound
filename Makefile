# Makefile for et-sdl-sound

# _GCC_PATH=/usr/x86_64-pc-linux-gnu/gcc-bin/3.4.6/

LD = $(_GCC_PATH)g++
CPP = $(_GCC_PATH)g++
CC = $(_GCC_PATH)gcc

BUILD = build
RELEASE = release
OBJS = $(BUILD)/mainlib.o $(BUILD)/etsdl.o $(BUILD)/hooks.o
HEADERS = hooks.hpp etsdl.hpp etalsa.hpp
LIB = et-sdl-sound.so
SCRIPTS = et-sdl-sound wolf-sdl-sound wolfsp-sdl-sound quake3-sdl-sound
SCRIPT_IN = launcher-script.in

CFLAGS = -m32 -O2 -fomit-frame-pointer -Wall -fPIC -D__SDL -D__DLOPEN_SDL -D__DEFAULT_BACKEND=SDL
LDFLAGS = -m32 -shared

all: $(SCRIPTS)

$(LIB): $(BUILD) $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $(LIB)

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/mainlib.o: mainlib.cpp $(HEADERS)
	$(CPP) $(CFLAGS) -c mainlib.cpp -o $(BUILD)/mainlib.o

$(BUILD)/hooks.o: hooks.cpp $(HEADERS)
	$(CPP) $(CFLAGS) -c hooks.cpp -o $(BUILD)/hooks.o

$(BUILD)/etsdl.o: etsdl.cpp $(HEADERS)
	$(CPP) $(CFLAGS) -c etsdl.cpp -o $(BUILD)/etsdl.o

$(BUILD)/etalsa.o: etalsa.cpp $(HEADERS)
	$(CPP) $(CFLAGS) -c etalsa.cpp -o $(BUILD)/etalsa.o

embed-lib: embed-lib.c
	$(CC) embed-lib.c -Wall -o embed-lib

et-sdl-sound: $(LIB) $(SCRIPT_IN) embed-lib
	sed 's/--script-name--/et-sdl-sound/g;s/--game-bin--/et.x86/g;s/--game-dir--/enemy-territory/g' $(SCRIPT_IN) > et-sdl-sound
	./embed-lib et-sdl-sound et-sdl-sound --et-sdl-sound-lib-- $(LIB) wrap
	chmod a+x et-sdl-sound

wolf-sdl-sound: $(LIB) $(SCRIPT_IN) embed-lib
	sed 's/--script-name--/wolf-sdl-sound/g;s/--game-bin--/wolf.x86/g;s/--game-dir--/rtcw/g' $(SCRIPT_IN) > wolf-sdl-sound
	./embed-lib wolf-sdl-sound wolf-sdl-sound --et-sdl-sound-lib-- $(LIB) wrap
	chmod a+x wolf-sdl-sound

wolfsp-sdl-sound: $(LIB) $(SCRIPT_IN) embed-lib
	sed 's/--script-name--/wolfsp-sdl-sound/g;s/--game-bin--/wolfsp.x86/g;s/--game-dir--/rtcw/g' $(SCRIPT_IN) > wolfsp-sdl-sound
	./embed-lib wolfsp-sdl-sound wolfsp-sdl-sound --et-sdl-sound-lib-- $(LIB) wrap
	chmod a+x wolfsp-sdl-sound

quake3-sdl-sound: $(LIB) $(SCRIPT_IN) embed-lib
	sed 's/--script-name--/quake3-sdl-sound/g;s/--game-bin--/quake3.x86/g;s/--game-dir--/quake3/g' $(SCRIPT_IN) > quake3-sdl-sound
	./embed-lib quake3-sdl-sound quake3-sdl-sound --et-sdl-sound-lib-- $(LIB) wrap
	chmod a+x quake3-sdl-sound

release: $(SCRIPTS)
	mkdir -p $(RELEASE)/et-sdl-sound
	cp *.hpp *.cpp *.in *.c README CHANGELOG $(SCRIPTS) $(LIB) $(RELEASE)/et-sdl-sound/
	sed 's/^_GCC_PATH/\# _GCC_PATH/g' Makefile > $(RELEASE)/et-sdl-sound/Makefile
	
	cd $(RELEASE) && tar -czf et-sdl-sound.tar.gz et-sdl-sound
	
	cd $(RELEASE)/et-sdl-sound && gzip -f $(SCRIPTS)
	cp $(RELEASE)/et-sdl-sound/*.gz $(RELEASE)/
	
	rm -rf $(RELEASE)/et-sdl-sound

clean:
	rm -rf $(BUILD)

clean-all:
	rm -rf $(BUILD)
	rm -rf $(RELEASE)
	rm -f $(LIB) embed-lib $(SCRIPTS)
