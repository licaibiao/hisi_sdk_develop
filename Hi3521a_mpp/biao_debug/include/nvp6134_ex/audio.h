/********************************************************************************
*
*  Copyright (C) 2016 	NEXTCHIP Inc. All rights reserved.
*  Module		: The decoder's audio header file
*  Description	: Audio i/o
*  Author		:
*  Date         :
*  Version		: Version 1.0
*
********************************************************************************
*  History      :
*
*
********************************************************************************/
#ifndef _AUDIO_H_
#define _AUDIO_H_

/********************************************************************
 *  define and enum
 ********************************************************************/
#define AIG_DEF   0x08
#define AOG_DEF   0x08

/********************************************************************
 *  structure
 ********************************************************************/

/********************************************************************
 *  external api
 ********************************************************************/
extern void audio_init(unsigned char recmaster, unsigned char pbmaster, unsigned char ch_num, unsigned char samplerate, unsigned char bits);
extern int nvp6134_audio_set_format(unsigned char type, nvp6134_audio_format format);
#endif	// End of _AUDIO_H_

/********************************************************************
 *  End of file
 ********************************************************************/

