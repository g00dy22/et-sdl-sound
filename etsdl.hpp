/*
et-sdl-sound - SDL sound system for Enemy Territory, RTCW and Q3
Copyright (C) 2007-2008  Pyry Haulos

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef ETSDL_H
#define ETSDL_H

#include <SDL/SDL.h>

#include "hooks.hpp"

class EtSDL {
public:
	EtSDL(dma_t *dma, void *callback, bool quake3);
	~EtSDL();
	
	qboolean init();
	void shutdown();
	
	int getDMAPos();
	void beginPainting();
	void submit();
	
	void callback(void *userdata, Uint8 *stream, int len);

protected:
	cvar_t *getCvar(const char *var_name, const char *var_value, int flags);
	bool loadSDLLib();
	
	// SDL.h
	int (*__SDL_Init) (Uint32 flags);
	Uint32 (*__SDL_WasInit) (Uint32 flags);
	char *(*__SDL_GetError) (void);
	void (*__SDL_QuitSubSystem) (Uint32 flags);
	
	// SDL_audio.h
	int (*__SDL_OpenAudio) (SDL_AudioSpec *desired, SDL_AudioSpec *obtained);
	char *(*__SDL_AudioDriverName) (char *namebuf, int maxlen);
	void (*__SDL_PauseAudio) (int pause_on);
	void (*__SDL_LockAudio) (void);
	void (*__SDL_UnlockAudio) (void);
	void (*__SDL_CloseAudio) (void);
	
	
	cvar_t *sndbits;
	cvar_t *sndspeed;
	cvar_t *sndchannels;
	cvar_t *sdlsamplesmult;
	
	dma_t *dma;
	byte *buffer;
	
	int dma_pos;
	int dma_size;
	
	bool quake3;
	
	bool init_done;
	bool sdl_lib_loaded;
	
	void *callback_func;
};

#endif // ETSDL_H
