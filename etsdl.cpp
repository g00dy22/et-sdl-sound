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

#include "etsdl.hpp"
#include <iostream>
#include <cstdlib>

#include <SDL/SDL_audio.h>

#ifdef __DLOPEN_SDL
#include <dlfcn.h>
#endif // __DLOPEN_SDL

EtSDL::EtSDL(dma_t *dma, void *callback, bool quake3)
{
	this->dma = dma;
	this->quake3 = quake3;
	dma_size = 0;
	dma_pos = 0;
	init_done = false;
	sdl_lib_loaded = false;
	
	this->callback_func = callback;
}

EtSDL::~EtSDL()
{

}

qboolean EtSDL::init()
{
	if (init_done)
		return qfalse;
	
	if (!loadSDLLib())
		return qfalse;
	
	SDL_AudioSpec desired, obtained;
	char drivername[128];
	
	std::cout << "SDL audio driver initializing..." << std::endl;
	
	sndbits = getCvar("sndbits", "16", CVAR_ARCHIVE);
	sndspeed = getCvar("sndspeed", "0", CVAR_ARCHIVE);
	sndchannels = getCvar("sndchannels", "2", CVAR_ARCHIVE);
	sdlsamplesmult = getCvar("sdlsamplesmult", "8", CVAR_ARCHIVE);
	
	if (!__SDL_WasInit(SDL_INIT_AUDIO)) {
		if (__SDL_Init(SDL_INIT_AUDIO) == -1) {
			std::cout << "SDL_Init(SDL_INIT_AUDIO) failed: " << __SDL_GetError() << std::endl;
			return qfalse;
		}
	}
	
	if (__SDL_AudioDriverName(drivername, sizeof(drivername)) == NULL) {
		/* SDL_Init() may pass, but it reports NULL here */
		std::cout << "SDL_AudioDriverName() = NULL" << std::endl;
		return qfalse;
	}
	
	std::cout << "SDL audio driver is \"" << drivername << "\"" << std::endl;
	
	memset(&desired, '\0', sizeof(desired));
	memset(&obtained, '\0', sizeof(obtained));
	
	desired.freq = (int) sndspeed->value;
	
	if (!desired.freq)
		desired.freq = 44100;
	
	if ((int) sndbits->value == 8)
		desired.format = AUDIO_U8;
	else
		desired.format = AUDIO_S16SYS;
	
	if (desired.freq <= 11025)
		desired.samples = 256;
	else if (desired.freq <= 22050)
		desired.samples = 512;
	else if (desired.freq <= 44100)
		desired.samples = 1024;
	else
		desired.samples = 2048;
	
	desired.channels = (int) sndchannels->value;
	desired.callback = (void (*) (void*, Uint8*, int)) callback_func;
	
	if (__SDL_OpenAudio(&desired, &obtained) == -1) {
		std::cout << "SDL_OpenAudio() failed: " << SDL_GetError() << std::endl;
		return qfalse;
	}

	int samples = obtained.samples * obtained.channels;

	if ((int) sdlsamplesmult->value >= 1)
		samples *= (int) sdlsamplesmult->value;
	
	// make it power of two
	if (samples & (samples - 1)) {
		int val = 1;
		while (val < samples)
			val <<= 1;
		samples = val;
	}
	
	dma->samplebits = obtained.format & 0xff; // first byte of format is bits
	dma->channels = obtained.channels;
	dma->samples = samples;
	dma->submission_chunk = 1;
	dma->speed = obtained.freq;
	
	dma_pos = 0;
	dma_size = (dma->samples * (dma->samplebits / 8));
	
	buffer = (byte *) malloc(dma_size);
	
	if (quake3)
		dma->q3buffer = buffer;
	else
		dma->buffer = buffer;
	
	__SDL_PauseAudio(0); // start callback
	
	std::cout << "SDL audio initialized." << std::endl;
	
	init_done = true;
	return qtrue;
}

void EtSDL::shutdown()
{
	std::cout << "Closing SDL audio device..." << std::endl;
	
	__SDL_PauseAudio(1);
	__SDL_CloseAudio();
	__SDL_QuitSubSystem(SDL_INIT_AUDIO);
	
	free(buffer);
	
	dma_pos = 0;
	dma_size = 0;
	init_done = false;
	
	std::cout << "SDL audio device shut down." << std::endl;
}

void EtSDL::callback(void *userdata, Uint8 *stream, int len)
{
	int pos = (dma_pos * (dma->samplebits / 8));
	
	if (pos > dma_size) {
		dma_pos = 0;
		pos = 0;
	}
	
	if (!init_done) {
		memset(stream, '\0', len);
		return;
	}
	
	int tobufend = dma_size - pos;
	int len1 = len;
	int len2 = 0;
	
	if (len1 > tobufend) {
		len1 = tobufend;
		len2 = len - len1;
	}
	
	memcpy(stream, buffer + pos, len1);
	
	if (len2 <= 0)
		dma_pos += (len1 / (dma->samplebits / 8));
	else {
		memcpy(stream + len1, buffer, len2);
		dma_pos = (len2 / (dma->samplebits / 8));
	}
	
	if (dma_pos >= dma_size)
		dma_pos = 0;
}

int EtSDL::getDMAPos()
{
	return dma_pos;
}

void EtSDL::beginPainting()
{
	__SDL_LockAudio();
}

void EtSDL::submit()
{
	__SDL_UnlockAudio();
}

cvar_t *EtSDL::getCvar(const char *var_name, const char *var_value, int flags)
{
	return Cvar_Get(var_name, var_value, flags);
}

bool EtSDL::loadSDLLib()
{
	if (sdl_lib_loaded)
		return true;
	
#ifndef __DLOPEN_SDL
	__SDL_Init = &SDL_Init;
	__SDL_WasInit = &SDL_WasInit;
	__SDL_GetError = &SDL_GetError;
	__SDL_QuitSubSystem = &SDL_QuitSubSystem;
	__SDL_OpenAudio = &SDL_OpenAudio;
	__SDL_AudioDriverName = &SDL_AudioDriverName;
	__SDL_PauseAudio = &SDL_PauseAudio;
	__SDL_LockAudio = &SDL_LockAudio;
	__SDL_UnlockAudio = &SDL_UnlockAudio;
	__SDL_CloseAudio = &SDL_CloseAudio;
#else
	const char *sdl_lib = getenv("ETSDL_SDL_LIB");
	
	if (sdl_lib == NULL)
		sdl_lib = "libSDL.so";
	
	void *sdl_lib_handle = dlopen(sdl_lib, RTLD_LAZY);
	
	if (!sdl_lib_handle) {
		std::cout << "Could not open " << sdl_lib << ": " << dlerror() << std::endl;
		std::cout << "Please check that ETSDL_SDL_LIB environment variable points to valid 32-bit SDL library." << std::endl;
		return false;
	}
	
#define DLSYM_FUNC(type, func) \
	if ((__##func = (type) dlsym(sdl_lib_handle, #func)) == NULL) { \
		std::cout << "Could not load " << #func << ": " << dlerror() << std::endl; \
		return false; \
	}
	
	DLSYM_FUNC(int (*) (Uint32), SDL_Init)
	DLSYM_FUNC(Uint32 (*) (Uint32), SDL_WasInit)
	DLSYM_FUNC(char *(*) (void), SDL_GetError)
	DLSYM_FUNC(void (*) (Uint32), SDL_QuitSubSystem)
	DLSYM_FUNC(int (*) (SDL_AudioSpec *, SDL_AudioSpec *), SDL_OpenAudio)
	DLSYM_FUNC(char *(*) (char *, int), SDL_AudioDriverName)
	DLSYM_FUNC(void (*) (int), SDL_PauseAudio)
	DLSYM_FUNC(void (*) (void), SDL_LockAudio)
	DLSYM_FUNC(void (*) (void), SDL_UnlockAudio)
	DLSYM_FUNC(void (*) (void), SDL_CloseAudio)
#endif // __DLOPEN_SDL
	
	sdl_lib_loaded = true;
	
	return true;
}
