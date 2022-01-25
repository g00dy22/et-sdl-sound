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

#include "hooks.hpp"
#include <iostream>
#include <fstream>
#include <sys/mman.h>
#include <cstdlib>
#include <limits.h>

// avoid readlink function error
#include <stdlib.h>
#include <unistd.h>

#ifdef __SDL
#include <SDL/SDL.h>
#include "etsdl.hpp"
#endif

#ifdef __ALSA
#include "etalsa.hpp"
#endif

#ifdef __SDL
static EtSDL *etsdl;

static void sdl_audio_callback(void *userdata, Uint8 *stream, int len)
{
	etsdl->callback(userdata, stream, len);
}
#endif

#ifdef __ALSA
static EtALSA *etalsa;
#endif

AudioBackend backend = __DEFAULT_BACKEND;
dma_t *dma;
cvar_t *(*_Cvar_Get)(const char *var_name, const char *var_value, int flags);

void initHooks()
{
	// determine correct version by looking at /proc/self/exe
	unsigned int CRC32 = calculateProcCRC32();
	const char *version = "UNKNOWN";
	bool quake3 = false;
	
	switch (CRC32)
	{
	case 0xdc49bc09:
		version = "Q3 1.31";
		quake3 = true;
		
		writeJump((void *) 0x0813b4f0, (void *) SNDDMA_Init);
		writeJump((void *) 0x0813bb3c, (void *) SNDDMA_GetDMAPos);
		
		writeAddr((void *) 0x0807d6a9, (void *) SNDDMA_Shutdown);
		writeAddr((void *) 0x0807bc6d, (void *) SNDDMA_BeginPainting);
		writeAddr((void *) 0x0807cedb, (void *) SNDDMA_BeginPainting);
		writeAddr((void *) 0x0807bca3, (void *) SNDDMA_Submit);
		writeAddr((void *) 0x0807cee9, (void *) SNDDMA_Submit);
		
		_Cvar_Get = (cvar_t* (*) (const char *, const char *, int)) 0x0806f254;
		dma = (dma_t *) 0x0878c2c4;
		break;

	case 0x10f74d19:
		version = "Q3 1.32";
		quake3 = true;
		
		writeJump((void *) 0x080bdae8, (void *) SNDDMA_Init);
		writeJump((void *) 0x080be134, (void *) SNDDMA_GetDMAPos);
		
		writeAddr((void *) 0x080ab1c1, (void *) SNDDMA_Shutdown);
		writeAddr((void *) 0x080a9785, (void *) SNDDMA_BeginPainting);
		writeAddr((void *) 0x080aa9f3, (void *) SNDDMA_BeginPainting);
		writeAddr((void *) 0x080a97bc, (void *) SNDDMA_Submit);
		writeAddr((void *) 0x080aaa01, (void *) SNDDMA_Submit);
		
		_Cvar_Get = (cvar_t* (*) (const char *, const char *, int)) 0x08098a4c;
		dma = (dma_t *) 0x08844864;
		break;

	case 0xe5782e44:
		version = "Q3 1.32b";
		quake3 = true;
		
		writeJump((void *) 0x080bd99c, (void *) SNDDMA_Init);
		writeJump((void *) 0x080bdfe8, (void *) SNDDMA_GetDMAPos);
		
		writeAddr((void *) 0x080aafa5, (void *) SNDDMA_Shutdown);
		writeAddr((void *) 0x080a9569, (void *) SNDDMA_BeginPainting);
		writeAddr((void *) 0x080aa7d7, (void *) SNDDMA_BeginPainting);
		writeAddr((void *) 0x080a959f, (void *) SNDDMA_Submit);
		writeAddr((void *) 0x080aa7e5, (void *) SNDDMA_Submit);
		
		_Cvar_Get = (cvar_t* (*) (const char *, const char *, int)) 0x08098830;
		dma = (dma_t *) 0x088446a4;
		break;

	case 0x2f3661cf:
		version = "Q3 1.32c";
		quake3 = true;
		
		writeJump((void *) 0x080bd5c4, (void *) SNDDMA_Init);
		writeJump((void *) 0x080bdc10, (void *) SNDDMA_GetDMAPos);
		
		writeAddr((void *) 0x080bdca1, (void *) SNDDMA_Shutdown);
		writeAddr((void *) 0x080a91ed, (void *) SNDDMA_BeginPainting);
		writeAddr((void *) 0x080aa477, (void *) SNDDMA_BeginPainting);
		writeAddr((void *) 0x080a9223, (void *) SNDDMA_Submit);
		writeAddr((void *) 0x080aa485, (void *) SNDDMA_Submit);
		
		_Cvar_Get = (cvar_t* (*) (const char *, const char *, int)) 0x080985bc;
		dma = (dma_t *) 0x08846104;
		break;
	
	case 0x2f47b0da:
		version = "Wolf 1.41";
		
		writeJump((void *) 0x080d2118, (void *) SNDDMA_Init);
		writeJump((void *) 0x080d26cc, (void *) SNDDMA_GetDMAPos);
		
		writeAddr((void *) 0x080bd92b, (void *) SNDDMA_Shutdown);
		writeAddr((void *) 0x080bbc53, (void *) SNDDMA_BeginPainting);
		writeAddr((void *) 0x080bc055, (void *) SNDDMA_BeginPainting);
		writeAddr((void *) 0x080bbc89, (void *) SNDDMA_Submit);
		writeAddr((void *) 0x080bc063, (void *) SNDDMA_Submit);
		
		_Cvar_Get = (cvar_t* (*) (const char *, const char *, int)) 0x080a8fc4;
		dma = (dma_t *) 0x08ba35a4;
		break;
	
	case 0xd5676d8f:
		version = "Wolf 1.41-MP";
		
		writeJump((void *) 0x080d48dc, (void *) SNDDMA_Init);
		writeJump((void *) 0x080d4ed8, (void *) SNDDMA_GetDMAPos);
		
		writeAddr((void *) 0x080c0e5f, (void *) SNDDMA_Shutdown);
		writeAddr((void *) 0x080bf742, (void *) SNDDMA_BeginPainting);
		writeAddr((void *) 0x080bfb08, (void *) SNDDMA_BeginPainting);
		writeAddr((void *) 0x080bf778, (void *) SNDDMA_Submit);
		writeAddr((void *) 0x080bfb16, (void *) SNDDMA_Submit);
		
		_Cvar_Get = (cvar_t* (*) (const char *, const char *, int)) 0x080aca18;
		dma = (dma_t *) 0x08aedc04;
		break;
	
	case 0x6ab49f82:
		version = "ET 2.60b";
		
		writeJump((void *) 0x08188250, (void *) SNDDMA_Init);
		writeJump((void *) 0x08188840, (void *) SNDDMA_GetDMAPos);
		writeJump((void *) 0x081888d0, (void *) SNDDMA_Shutdown);
		writeJump((void *) 0x081888f0, (void *) SNDDMA_BeginPainting);
		writeJump((void *) 0x081888e0, (void *) SNDDMA_Submit);
		
		_Cvar_Get = (cvar_t* (*) (const char *, const char *, int)) 0x08073bb0;
		dma = (dma_t *) 0x0926d3a4;
		break;
	
	case 0x3b18a889:
		version = "ET 2.60";
		
		writeJump((void *) 0x0817d6c0, (void *) SNDDMA_Init);
		writeJump((void *) 0x0817dce0, (void *) SNDDMA_GetDMAPos);
		writeJump((void *) 0x0817dd70, (void *) SNDDMA_Shutdown);
		writeJump((void *) 0x0817dd90, (void *) SNDDMA_BeginPainting);
		writeJump((void *) 0x0817dd80, (void *) SNDDMA_Submit);
		
		_Cvar_Get = (cvar_t* (*) (const char *, const char *, int)) 0x0806fcb0;
		dma = (dma_t *) 0x0925c2e4;
		break;
	
	case 0x3d59a703:
		version = "ET 2.56";
		
		writeJump((void *) 0x080e5c54, (void *) SNDDMA_Init);
		writeJump((void *) 0x080e6250, (void *) SNDDMA_GetDMAPos);
		
		writeAddr((void *) 0x080d0947, (void *) SNDDMA_Shutdown);
		writeAddr((void *) 0x080ceaeb, (void *) SNDDMA_BeginPainting);
		writeAddr((void *) 0x080cf009, (void *) SNDDMA_BeginPainting);
		writeAddr((void *) 0x080ceb21, (void *) SNDDMA_Submit);
		writeAddr((void *) 0x080cf017, (void *) SNDDMA_Submit);
		
		_Cvar_Get = (cvar_t* (*) (const char *, const char *, int)) 0x080ba5d8;
		dma = (dma_t *) 0x09180184;
		break;
	
	case 0x21e60afb:
		version = "ET 2.55";
		
		writeJump((void *) 0x080e6064, (void *) SNDDMA_Init);
		writeJump((void *) 0x080e6660, (void *) SNDDMA_GetDMAPos);
		
		writeAddr((void *) 0x080d0da7, (void *) SNDDMA_Shutdown);
		writeAddr((void *) 0x080cef4b, (void *) SNDDMA_BeginPainting);
		writeAddr((void *) 0x080cf469, (void *) SNDDMA_BeginPainting);
		writeAddr((void *) 0x080cef81, (void *) SNDDMA_Submit);
		writeAddr((void *) 0x080cf477, (void *) SNDDMA_Submit);
		
		_Cvar_Get = (cvar_t* (*) (const char *, const char *, int)) 0x080bab60;
		dma = (dma_t *) 0x0917fba4;
		break;
	
	default:
		std::cout << "You are not running a recognized version of Enemy Territory or RTCW (CRC32 = " << (void *) CRC32  << ")" << std::endl;
		return; // we don't need to exit( 1 )
	}
	
	std::cout << "Found " << version << " (CRC32 = " << (void *) CRC32 << ")" << std::endl;
	
	if (backend == OSS) {
		std::cout << "Using default OSS backend." << std::endl;
		return;
	}
#ifdef __ALSA
	else if (backend == ALSA) {
		std::cout << "Using ALSA backend." << std::endl;
		etalsa = new EtALSA(dma);
	}
#endif
#ifdef __SDL
	else if (backend == SDL) {
		std::cout << "Using SDL backend." << std::endl;
		etsdl = new EtSDL(dma, (void *) sdl_audio_callback, quake3);
	}
#endif
	else {
		std::cout << "Requested backend is not available, using OSS." << std::endl;
		return;
	}
	
	std::cout << "et-sdl-sound-" << __ETSDL_VERSION << " (" << __DATE__ << " " << __TIME__ << ", " << __VERSION__ << ") loaded." << std::endl;
}

void writeAddr(void *addr, void *dest)
{
	unprotectPage(addr);
	
	*((void **) addr) = (void *) (((unsigned long) dest) - ((unsigned long) addr + 4));
	
	printMem((void *) (((unsigned long) addr) - 1), 5);
}

void writeJump(void *addr, void *dest)
{
	unprotectPage(addr);
	
	*((unsigned char *) addr) = 0xE9;
	
	writeAddr((void *) (((unsigned long) addr) + 1), dest);
}

void unprotectPage(void *addr)
{
	mprotect((void*) (((unsigned long) addr) & 0xfffff000), 4096, PROT_READ | PROT_WRITE | PROT_EXEC);
}

void printMem(void *addr, int size)
{
	std::cout << addr << ": ";
	for (int i = 0; i < size; i++)
		printf("%02x ", ((unsigned char*) addr)[i]);
	std::cout << std::endl;
}

qboolean SNDDMA_Init(void)
{
#ifdef __DEBUG
	std::cout << "SNDDMA_Init()" << std::endl;
#endif

#ifdef __ALSA
	if (backend == ALSA)
		return etalsa->init();
#endif
#ifdef __SDL
	if (backend == SDL)
		return etsdl->init();
#endif

	return qfalse;
}

int SNDDMA_GetDMAPos(void)
{
#ifdef __DEBUG
	std::cout << "SNDDMA_GetDMAPos()" << std::endl;
#endif

#ifdef __ALSA
	if (backend == ALSA)
		return etalsa->getDMAPos();
#endif

#ifdef __SDL
	if (backend == SDL)
		return etsdl->getDMAPos();
#endif

	return 0;
}

void SNDDMA_Shutdown(void)
{
#ifdef __DEBUG
	std::cout << "SNDDMA_Shutdown()" << std::endl;
#endif

#ifdef __ALSA
	if (backend == ALSA)
		etalsa->shutdown();
#endif

#ifdef __SDL
	if ( backend == SDL )
		etsdl->shutdown();
#endif
}

void SNDDMA_BeginPainting(void)
{
#ifdef __DEBUG
	std::cout << "SNDDMA_BeginPainting()" << std::endl;
#endif

#ifdef __ALSA
	if (backend == ALSA)
		etalsa->beginPainting();
#endif

#ifdef __SDL
	if (backend == SDL)
		etsdl->beginPainting();
#endif
}

void SNDDMA_Submit(void)
{
#ifdef __DEBUG
	std::cout << "SNDDMA_Submit()" << std::endl;
#endif

#ifdef __ALSA
	if (backend == ALSA)
		etalsa->submit();
#endif

#ifdef __SDL
	if (backend == SDL)
		etsdl->submit();
#endif
}

cvar_t *Cvar_Get(const char *var_name, const char *var_value, int flags)
{
	return _Cvar_Get(var_name, var_value, flags);
}

unsigned int calculateProcCRC32()
{
	// find actual file
	char filename[PATH_MAX];
	ssize_t len;
	
	if ((len = readlink("/proc/self/exe", filename, sizeof(filename) - 1)) < 1) {
		std::cout << "Can't find actual binary." << std::endl;
		return 0x00000000;
	}
	filename[len] = '\0';
	
	// read file
	std::ifstream exe;
	exe.open(filename);
	
	if (!exe.is_open() | !exe.good()) {
		std::cout << "Can't open " << filename << std::endl;
		return 0x00000000;
	}
	
	exe.seekg(0, std::ios_base::end);
	unsigned int fsize = exe.tellg();
	exe.seekg(0, std::ios_base::beg);
	
	unsigned char *file = new unsigned char[fsize];
	
	exe.read((char*) file, fsize);
	exe.close();
	
	std::cout << "Read " << filename << " (" << fsize << " bytes)" << std::endl;
	
	// generate crc_table
	unsigned int crc_table[256];
	
	register unsigned long crc;
	unsigned long poly = 0xEDB88320L;
	
	for (int i = 0; i < 256; i++) {
		crc = i;
		for (int j = 8; j > 0; j--) {
			if (crc & 1)
				crc = (crc >> 1) ^ poly;
			else
				crc >>= 1;
		}
		crc_table[i] = crc;
	}
	
	// calculate crc32
	unsigned char *block = file; // we need file to delete[] it
	crc = 0xFFFFFFFF;
	for (unsigned int i = 0; i < fsize; i++)
		crc = ((crc >> 8) & 0x00FFFFFF) ^ crc_table[(crc ^ *block++) & 0xFF];
	
	delete[] file;
	
	return (unsigned int) (crc ^ 0xFFFFFFFF);
}
