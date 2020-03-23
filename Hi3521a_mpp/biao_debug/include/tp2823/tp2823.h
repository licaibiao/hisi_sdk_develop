#ifndef __TP2802_H__
#define __TP2802_H__

//#include <linux/ioctl.h>

#define TP2802_VERSION_CODE KERNEL_VERSION(0, 3, 4)
enum{
TP2802_1080P25 =	    0x03,
TP2802_1080P30 =	    0x02,
TP2802_720P25  =	    0x05,
TP2802_720P30  =    	0x04,
TP2802_720P50  =	    0x01,
TP2802_720P60  =    	0x00,
TP2802_SD      =        0x06,
INVALID_FORMAT =		0x07,
TP2802_720P25V2=	    0x0D,
TP2802_720P30V2=		0x0C,
TP2802_PAL	   =        0x08,
TP2802_NTSC	   =    	0x09
};
enum{
    VIDEO_UNPLUG,
    VIDEO_IN,
    VIDEO_LOCKED,
    VIDEO_UNLOCK
};
enum{
    MUX_1CH,    //148.5M mode
    MUX_2CH,    //148.5M mode
    DDR_2CH,    //297M mode, TP2822/23 feature
    DDR_4CH     //future support
};


#define FLAG_LOSS       0x80
#define FLAG_LOCKED     0x68

#define CHANNELS_PER_CHIP 	4
#define MAX_CHIPS 	4
#define SUCCESS				0
#define FAILURE				-1

#define BRIGHTNESS			0x10
#define CONTRAST			0x11
#define SATURATION			0x12
#define HUE					0X13
#define SHARPNESS			0X14


typedef struct _tp2802_register
{
    unsigned char chip;
    unsigned char ch;
    unsigned int reg_addr;
    unsigned int value;
} tp2802_register;

typedef struct _tp2802_work_mode
{
    unsigned char chip;
    unsigned char ch;
    unsigned char mode;
} tp2802_work_mode;

typedef struct _tp2802_video_mode
{
    unsigned char chip;
    unsigned char ch;
    unsigned char mode;
    unsigned char prog;
} tp2802_video_mode;

typedef struct _tp2802_video_loss
{
    unsigned char chip;
    unsigned char ch;
    unsigned char is_lost;
} tp2802_video_loss;

typedef struct _tp2802_image_adjust
{
    unsigned char chip;
    unsigned char ch;
	unsigned int hue;
	unsigned int contrast;
	unsigned int brightness;
	unsigned int saturation;
	unsigned int sharpness;
} tp2802_image_adjust;

typedef struct _tp2802_PTZ_data
{
    unsigned char chip;
    unsigned char ch;
    unsigned char data[10];
} tp2802_PTZ_data;

typedef enum _tp2802_audio_samplerate
{
	SAMPLE_RATE_8000,
	SAMPLE_RATE_16000,
} tp2802_audio_samplerate;

typedef struct _tp2802_audio_playback
{
    unsigned int chip;
    unsigned int chn;
} tp2802_audio_playback;

typedef struct _tp2802_audio_da_volume
{
    unsigned int chip;
    unsigned int volume;
} tp2802_audio_da_volume;

typedef struct _tp2802_audio_da_mute
{
    unsigned int chip;
    unsigned int flag;
} tp2802_audio_da_mute;

typedef struct _tp2823_audio_format
{
	unsigned int chip;
	unsigned int chn_num;
	unsigned int format;   /* 0:i2s; 1:dsp */
    unsigned int mode;   /* 0:slave 1:master*/
    unsigned int clkdir;  /*0:inverted;1:non-inverted*/
	unsigned int bitrate; /*0:256fs 1:320fs*/
	unsigned int precision;/*0:16bit;1:8bit*/
} tp2823_audio_format;

// IOCTL Definitions
#define TP2802_IOC_MAGIC            'v'

#define TP2802_READ_REG				_IOWR(TP2802_IOC_MAGIC, 1, tp2802_register)
#define TP2802_WRITE_REG			_IOW(TP2802_IOC_MAGIC, 2, tp2802_register)
#define TP2802_SET_VIDEO_MODE		_IOW(TP2802_IOC_MAGIC, 3, tp2802_work_mode)
#define TP2802_GET_VIDEO_MODE	    _IOWR(TP2802_IOC_MAGIC, 4, tp2802_video_mode)
#define TP2802_GET_VIDEO_LOSS	    _IOWR(TP2802_IOC_MAGIC, 5, tp2802_video_loss)
#define TP2802_SET_IMAGE_ADJUST	    _IOW(TP2802_IOC_MAGIC, 6, tp2802_image_adjust)
#define TP2802_GET_IMAGE_ADJUST	    _IOWR(TP2802_IOC_MAGIC, 7, tp2802_image_adjust)
#define TP2802_SET_PTZ_DATA 	    _IOW(TP2802_IOC_MAGIC, 8, tp2802_PTZ_data)
#define TP2802_GET_PTZ_DATA 	    _IOWR(TP2802_IOC_MAGIC, 9, tp2802_PTZ_data)
#define TP2802_SET_SCAN_MODE 	    _IOW(TP2802_IOC_MAGIC, 10, tp2802_work_mode)
#define TP2802_DUMP_REG     	    _IOW(TP2802_IOC_MAGIC, 11, tp2802_register)
#define TP2802_FORCE_DETECT    	    _IOW(TP2802_IOC_MAGIC, 12, tp2802_work_mode)
#define TP2802_SET_SAMPLE_RATE      _IOW(TP2802_IOC_MAGIC, 13, tp2802_audio_samplerate)
#define TP2802_SET_AUDIO_PLAYBACK   _IOW(TP2802_IOC_MAGIC, 14, tp2802_audio_playback)
#define TP2802_SET_AUDIO_DA_VOLUME  _IOW(TP2802_IOC_MAGIC, 15, tp2802_audio_da_volume)
#define TP2802_SET_AUDIO_DA_MUTE    _IOW(TP2802_IOC_MAGIC, 16, tp2802_audio_da_mute)
#define TP2802_SET_AUDIO_RM_FORMAT  _IOW(TP2802_IOC_MAGIC, 17, tp2823_audio_format)
#define TP2802_SET_AUDIO_PB_FORMAT  _IOW(TP2802_IOC_MAGIC, 18, tp2823_audio_format)

// Function prototypes
int tp2802_set_video_mode(unsigned char addr, unsigned char mode, unsigned char ch);
void tp2802_set_reg_page(unsigned char addr, unsigned char ch);

#endif


