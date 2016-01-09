#ifndef ETALSA_H
#define ETALSA_H

#include "hooks.hpp"
#include <alsa/asoundlib.h>

// NOTE this does not yet work
class EtALSA {
public:
	EtALSA(dma_t * dma);
	~EtALSA();

	qboolean init();
	void shutdown();

	int getDMAPos();
	void beginPainting();
	void submit();

protected:
	 cvar_t * getCvar(const char *var_name, const char *var_value, int flags);
	int xrunRecover(int err);

	cvar_t *sndbits;
	cvar_t *sndspeed;
	cvar_t *sndchannels;
	cvar_t *alsadevice;

	dma_t *dma;

	snd_pcm_t *pcm_handle;

	bool init_done;
	bool beginPainting_done;
	int dma_pos;
	unsigned int channels;

	snd_pcm_uframes_t offset, frames, period_size;
	snd_pcm_sframes_t avail, commitres;

	char *err_buffer;
};

#endif // ETALSA_H
