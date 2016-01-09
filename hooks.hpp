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

#ifndef HOOKS_H
#define HOOKS_H

#define __ETSDL_VERSION "r29"

// from Q3 sources
typedef enum {qfalse, qtrue} qboolean;
typedef unsigned char byte;

typedef struct {
	int	channels;
	int	samples;
	int	submission_chunk;
	int	samplebits;
	int	speed;
	byte	*q3buffer;
	byte	*buffer;
} dma_t;

typedef struct cvar_s {
	char	*name;
	char	*string;
	char	*resetString;
	char	*latchedString;
	int	flags;
	qboolean modified;
	int	modificationCount;
	float	value;
	int	integer;
	struct cvar_s *next;
	struct cvar_s *hashNext;
} cvar_t;

#define	CVAR_ARCHIVE         1
#define	CVAR_USERINFO        2
#define	CVAR_SERVERINFO      4
#define	CVAR_SYSTEMINFO      8
#define	CVAR_INIT           16
#define	CVAR_LATCH          32
#define	CVAR_ROM            64
#define	CVAR_USER_CREATED  128
#define	CVAR_TEMP          256
#define CVAR_CHEAT         512
#define CVAR_NORESTART    1024

enum AudioBackend {OSS, ALSA, SDL};

qboolean SNDDMA_Init(void);
int SNDDMA_GetDMAPos(void);
void SNDDMA_Shutdown(void);
void SNDDMA_BeginPainting(void);
void SNDDMA_Submit(void);

cvar_t *Cvar_Get(const char *var_name, const char *var_value, int flags);

void initHooks();
void writeAddr(void *addr, void *dest);
void writeJump(void *addr, void *dest);
void unprotectPage(void *addr);
void printMem(void *addr, int size);
unsigned int calculateProcCRC32();

#endif // HOOKS_H
