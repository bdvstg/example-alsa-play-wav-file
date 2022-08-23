/*
 * # apt install libasound-dev
 * # gcc wavplay.c -I <path-to-asoundlib.h> -lasound
 */
 
#include <stdio.h>
#include <asoundlib.h>
 
static char *device = "default";            /* playback device */

typedef struct wav_header_ {
    char str_riff[4]; // "RIFF"
    int wav_size; // (file size) - 8
    char str_wave[4]; // "WAVE"
    
    // Format Header
    char str_fmt[4]; // "fmt "
    int fmt_chunk_size; // Should be 16 for PCM
    short audio_format; // Should be 1 for PCM. 3 for IEEE Float
    short channels;
    int sample_rate; // ex: 8000, 44100, 48000
    int byte_rate; // Number of bytes per second. sample_rate * channels * Bytes Per Sample
    short sample_alignment; // channels * Bytes Per Sample
    short bit_depth; // bits per sample, ex: 8, 16, 24
    
    // Data
    char str_data[4]; // "data"
    int data_bytes; // (file size) - 44
} wav_header; // ensure (sizeof(wav_header) == 44)
_Static_assert((sizeof(wav_header)==44), "sizeof wav_header must be 44");


int main(const int argc, const char *argv[])
{
    int err;
    unsigned int i;
    snd_pcm_t *handle;
    snd_pcm_sframes_t frames;
	unsigned char *buffer;
	wav_header info;

	FILE *f = fopen(argv[1],"rb");
	if (f == NULL)
		printf("Can not open file %s\n", argv[1]);
	fread(&info, 44, 1, f);

	printf("riff = %.4s\n", info.str_riff);
	printf("wav size = %d\n", info.wav_size);
	printf("wave = %.4s\n", info.str_wave);
	printf("fmt = %.4s\n", info.str_fmt);
	printf("fmt chunk size = %d\n", info.fmt_chunk_size);
	printf("audio format = %d\n", info.audio_format);
	printf("channels = %d\n", info.channels);
	printf("sample rate = %d\n", info.sample_rate);
	printf("byte rate = %d\n", info.byte_rate);
	printf("sample alignment = %d\n", info.sample_alignment);
	printf("bit depth = %d\n", info.bit_depth);
	printf("data = %.4s\n", info.str_data);
	printf("data bytes = %d\n", info.data_bytes);


	buffer = (unsigned char*)malloc(info.data_bytes);
	long readin = fread(buffer, 1, info.data_bytes, f);
	printf("read in = %ld\n", readin);
	fclose(f);
 
 
    if ((err = snd_pcm_open(&handle, device, SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
        printf("Playback open error: %s\n", snd_strerror(err));
        exit(EXIT_FAILURE);
    }
    
    snd_pcm_format_t format = SND_PCM_FORMAT_U8;
    snd_pcm_access_t access = SND_PCM_ACCESS_RW_INTERLEAVED;

	if (info.bit_depth == 8)
		format = SND_PCM_FORMAT_U8;
	else if(info.bit_depth == 16)
		format = SND_PCM_FORMAT_S16_LE;
    else if(info.bit_depth == 24)
		format = SND_PCM_FORMAT_S24_LE;
    else if(info.bit_depth == 32)
		format = SND_PCM_FORMAT_S32_LE;
	else {
		printf("error: bit depth not supported\n");
		exit(1);
	}

    int channels = info.channels; // mono, required PCM channels
    int rate = info.sample_rate; // 44.1k, required sample rate in Hz 
    int resample = 1; // allow alsa-lib resample stream
    unsigned int latency = 500000; // latency: required overall latency in us
    err = snd_pcm_set_params(handle,
        format, access, channels, rate, resample, latency);
    if(err < 0)
    {   /* 0.5sec */
        printf("Playback open error: %s\n", snd_strerror(err));
        exit(EXIT_FAILURE);
    }
	
	long n = info.data_bytes / info.sample_alignment;
 
    for (i = 0; i < 16; i++)
    {
        frames = snd_pcm_writei(handle, buffer, n);
        if (frames < 0)
            frames = snd_pcm_recover(handle, frames, 0);
        if (frames < 0)
        {
            printf("snd_pcm_writei failed: %s\n", snd_strerror(frames));
            break;
        }
        if (frames > 0 && frames < n)
            printf("Short write (expected %li, wrote %li)\n", n, frames);
    }
 
    /* pass the remaining samples, otherwise they're dropped in close */
    err = snd_pcm_drain(handle);
    if (err < 0)
        printf("snd_pcm_drain failed: %s\n", snd_strerror(err));
    snd_pcm_close(handle);
    return 0;
}
