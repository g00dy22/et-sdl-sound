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

#include "etalsa.hpp"
#include <iostream>
#include <cstdlib>
// #include <errno.h>
// #include <unistd.h>

EtALSA::EtALSA(dma_t * dma)
{
	this->dma = dma;
	init_done = false;
	dma_pos = 0;
	beginPainting_done = false;
}

EtALSA::~EtALSA()
{

}

qboolean EtALSA::init()
{
	if (init_done)
		return qfalse;

	std::cout << "ALSA audio driver initializing..." << std::endl;

	sndbits = getCvar("sndbits", "16", CVAR_ARCHIVE);
	sndspeed = getCvar("sndspeed", "0", CVAR_ARCHIVE);
	sndchannels = getCvar("sndchannels", "2", CVAR_ARCHIVE);
	alsadevice = getCvar("alsadevice", "default", CVAR_ARCHIVE);

	snd_pcm_stream_t stream = SND_PCM_STREAM_PLAYBACK;
	snd_pcm_hw_params_t *hwparams;

	snd_pcm_hw_params_alloca(&hwparams);

	if (snd_pcm_open(&pcm_handle, alsadevice->string, stream, 0) < 0) {
		std::cout << "Error opening PCM device " << alsadevice->string << std::endl;
		return qfalse;
	}

	std::cout << "PCM device is " << alsadevice->string << std::endl;

	if (snd_pcm_hw_params_any(pcm_handle, hwparams) < 0) {
		std::cout << "Can't configure this PCM device" << std::endl;
		return qfalse;
	}

	unsigned int rate = (unsigned int) sndspeed->value;
	channels = (unsigned int) sndchannels->value;
	unsigned int dma_size = 32768;
	snd_pcm_format_t format = SND_PCM_FORMAT_S16;
	int periods = 20;

	if (!rate)
		rate = 44100;

	if ((int) sndbits->value == 8)
		format = SND_PCM_FORMAT_U8;

	if (rate <= 22050)
		dma_size = 16384;

	unsigned int buffer_time = (unsigned int) ((float) dma_size / (((float) (sndbits->value / 8)) * (((float) rate) / 1000000.0f)) + 0.5f);

	int err;
	if ((err = snd_pcm_hw_params_set_rate_resample(pcm_handle, hwparams, 1)) < 0) {
		std::cout << "Unable to enable resampling: " << snd_strerror(err) << std::endl;
		return qfalse;
	}

	if ((err = snd_pcm_hw_params_set_access(pcm_handle, hwparams, SND_PCM_ACCESS_MMAP_INTERLEAVED)) < 0) {
		std::cout << "Interleaved mmap access is not available for playback: " << snd_strerror(err) << std::endl;
		return qfalse;
	}

	if ((err = snd_pcm_hw_params_set_format(pcm_handle, hwparams, format)) < 0) {
		std::cout << "Sample format " << (void *) format << " not available for playback: " << snd_strerror(err) << std::endl;
		return qfalse;
	}

	if ((err = snd_pcm_hw_params_set_channels(pcm_handle, hwparams, (int) sndchannels->value)) < 0) {
		std::cout << "Unable to get " << (int) sndchannels->value << " channels: " << snd_strerror(err) << std::endl;
		return qfalse;
	}

	if ((err = snd_pcm_hw_params_set_rate_near(pcm_handle, hwparams, &rate, 0)) < 0) {
		std::cout << "Rate " << rate << "Hz not available for playback: " << snd_strerror(err) << std::endl;
		return qfalse;
	}

	if ((err = snd_pcm_hw_params_set_periods(pcm_handle, hwparams, periods, 0)) < 0) {
		std::cout << "Unable to get " << periods << " periods for playback: " << snd_strerror(err) << std::endl;
		return qfalse;
	}

	int dir;
	snd_pcm_uframes_t buffer_size;
	if ((err = snd_pcm_hw_params_set_buffer_time_near(pcm_handle, hwparams, &buffer_time, &dir)) < 0) {
		std::cout << "Unable to set buffer time " << buffer_time << " for playback: " << snd_strerror(err) << std::endl;
		return qfalse;
	}

	if ((err = snd_pcm_hw_params_get_buffer_size(hwparams, &buffer_size)) < 0) {
		std::cout << "Unable to get buffer size for playback: " << snd_strerror(err) << std::endl;
		return qfalse;
	}

	if (buffer_size * channels != dma_size) {
		std::cout << "Playback buffer size " << buffer_size * channels << " does not match requested size " << dma_size << std::endl;
		dma_size = buffer_size * channels;
		//return qfalse;
	}

	if ((err = snd_pcm_hw_params_set_period_time_near(pcm_handle, hwparams, &buffer_time, &dir)) < 0) {
		std::cout << "Unable to set period time " << buffer_time << " for playback: " << snd_strerror(err) << std::endl;
		return qfalse;
	}

	if ((err = snd_pcm_hw_params_get_period_size(hwparams, &period_size, &dir)) < 0) {
		std::cout << "Unable to get period size for playback: " << snd_strerror(err) << std::endl;
		return qfalse;
	}

	int submission_chunk = (int) (period_size * channels);

	if (snd_pcm_hw_params(pcm_handle, hwparams) < 0) {
		std::cout << "Unable to set hw params for playback: " << snd_strerror(err) << std::endl;
		return qfalse;
	}

	dma->samplebits = (int) sndbits->value;
	dma->channels = (int) sndchannels->value;
	dma->samples = (int) dma_size;
	dma->submission_chunk = submission_chunk;
	dma->speed = rate;

	// lets just pretend that we are doing something
	err_buffer = (char *) malloc(dma_size);
	dma->buffer = (byte *) err_buffer;

	beginPainting();

	std::cout << "ALSA audio initialized." << std::endl;

	init_done = true;
	return qtrue;
}

void EtALSA::shutdown()
{
	std::cout << "Closing ALSA audio device..." << std::endl;

	free(err_buffer);
	init_done = false;

	std::cout << "ALSA audio device shut down." << std::endl;
}

int EtALSA::getDMAPos()
{
	beginPainting(); // we need correct offset

	return dma_pos;
}

void EtALSA::beginPainting()
{
	if (beginPainting_done)
		return;

	snd_pcm_state_t state = snd_pcm_state(pcm_handle);
	const snd_pcm_channel_area_t *my_areas;
	int err = 0;

	if (state == SND_PCM_STATE_XRUN) {
		if ((err = xrunRecover(-EPIPE)) < 0)
			std::cout << "XRUN recovery failed: " << snd_strerror(err) << std::endl;
	} else if (state == SND_PCM_STATE_SUSPENDED) {
		if ((err = xrunRecover(-ESTRPIPE)) < 0)
			std::cout << "SUSPEND recovery failed: " << snd_strerror(err) << std::endl;
	}

	if ((avail = snd_pcm_avail_update(pcm_handle)) < 0) {
		if ((err = xrunRecover(avail)) < 0)
			std::cout << "avail update failed: " << snd_strerror(err) << std::endl;
	}

	if ((snd_pcm_uframes_t) avail < period_size) {
		if (snd_pcm_start(pcm_handle) < 0)
			std::cout << "Start error: " << snd_strerror(err) << std::endl;
	}

	frames = period_size;
	if ((err = snd_pcm_mmap_begin(pcm_handle, &my_areas, &offset, &frames)) < 0) {
		if ((err = xrunRecover(err)) < 0) {
			std::cout << "MMAP begin avail error: " << snd_strerror(err) << std::endl;
			dma_pos = 0;
			dma->submission_chunk = 1;
			dma->buffer = (byte *) err_buffer;
			return;
		}
	}

	dma->buffer = (byte *) my_areas->addr;
	dma->submission_chunk = (int) (frames * channels);
	dma_pos = (int) (offset * channels);

	std::cout << "submission_chunk=" << dma->submission_chunk << ", dma_pos=" << dma_pos << " ,buffer=" << (void *) dma->buffer << std::endl;

	beginPainting_done = true;
}

void EtALSA::submit()
{
	commitres = snd_pcm_mmap_commit(pcm_handle, offset, frames);
	int err;

	if (commitres < 0 || (snd_pcm_uframes_t) commitres != frames) {
		if ((err = xrunRecover(commitres >= 0 ? -EPIPE : commitres)) < 0)
			std::cout << "MMAP commit error: " << snd_strerror(err) << std::endl;
	}

	std::cout << "commitres=" << commitres << std::endl;

	beginPainting_done = false;
}

cvar_t *EtALSA::getCvar(const char *var_name, const char *var_value, int flags)
{
	return Cvar_Get(var_name, var_value, flags);
}

int EtALSA::xrunRecover(int err)
{
	if (err == -EPIPE) {
		if ((err = snd_pcm_prepare(pcm_handle)) < 0)
			std::cout << "Can't recover from under-run, prepare failed: " << snd_strerror(err) << std::endl;
	} else if (err == -ESTRPIPE) {
		while ((err = snd_pcm_resume(pcm_handle)) == -EAGAIN)
			sleep(1);
		if (err < 0) {
			if ((err = snd_pcm_prepare(pcm_handle)) < 0)
				std::cout << "Can't recover from suspend, prepare failed: " << snd_strerror(err) << std::endl;
		}
	}

	return err;
}
