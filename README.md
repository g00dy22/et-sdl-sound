et-sdl-sound - SDL audio support hack for Enemy Territory, RTCW and Q3
--------------------------------------------------------------------------------

### About:
        et-sdl-sound provides SDL-based replacement for deprecated OSS-based
        sound systems of Enemy Territory, Return to Castle Wolfenstein and Quake
        III Arena. To put it short, et-sdl-sound is a working ALSA support hack
        for ET, RTCW and Q3 (and all mods for those binaries).

        Since modifying binary directly makes game unplayable, this is
        accomplished via replacing standard sound system functions in run-time
        by forcing dynamic linker to load an additional dynamic library to the
        process (so-called LD_PRELOAD trick).

### Support:
        et-sdl-sound currently supports following binaries:

        CRC32           description
        0x6ab49f82      ET 2.60b (et.x86)
        0x3b18a889      ET 2.60 (et.x86)
        0x3d59a703      ET 2.56 (et.x86)
        0x21e60afb      ET 2.55 (et.x86)
        0xc6aebd79      Wolf 1.41 (wolfsp.x86)
        0xd5676d8f      Wolf 1.41-MP (wolf.x86)
        0xdc49bc09      Q3 1.31 (quake3.x86)
        0x10f74d19      Q3 1.32 (quake3.x86)
        0xe5782e44      Q3 1.32b (quake3.x86)
        0x2f3661cf      Q3 1.32c (quake3.x86)

### Installation:
        et-sdl-sound script contains everything you need to launch Enemy Territory
        with SDL audio support. The fastest way to install the script is to
        execute following command line:

        wget -q -O - http://nullkey.ath.cx/~stuff/et-sdl-sound/et-sdl-sound.gz | gzip -d > et-sdl-sound && chmod a+x et-sdl-sound

        Now you can start Enemy Territory with SDL sound support by running
        ./et-sdl-sound

        There are also specific launcher scripts for
        RTCW SP (wolfsp-sdl-sound.gz), RTCW MP (wolf-sdl-sound.gz) and
        Quake 3 (quake3-sdl-sound.gz).


        Of course the old method is still available; extract et-sdl-sound.tar.gz,
        copy et-sdl-sound.so to somewhere safe and create the following script:

        #!/bin/bash
        export ETSDL_SDL_LIB="libSDL.so"
        export SDL_AUDIODRIVER="alsa"
        cd <game installation directory>
        LD_PRELOAD="/<path to>/et-sdl-sound.so" ./<game>.x86 $*

### Authors:
        Pyry Haulos <pyry.haulos@gmail.com>
        Kevin Vacit <kevin_vacit@linuxmail.org>
        Escor @ nixcoders.org
