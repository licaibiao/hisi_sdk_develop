#ifndef _AUDIO_H_
#define _AUDIO_H_

extern void audio_init(unsigned char recmaster, unsigned char pbmaster, unsigned char ch_num, unsigned char samplerate, unsigned char bits);
extern int nvp6124_audio_set_format(unsigned char type, nvp6124_audio_format format);
#endif
