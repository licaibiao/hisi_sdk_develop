#ifndef __COMMON_H__
#define __COMMON_H__

//#define HI_GPIO_I2C

#ifdef HI_GPIO_I2C
#include "gpio_i2c.h"
#else
//#include "hi_i2c.h"
#endif


#ifdef HI_GPIO_I2C
#define  I2CReadByte   gpio_i2c_read
#define  I2CWriteByte  gpio_i2c_write

#else
unsigned char __I2CReadByte8(unsigned char devaddress, unsigned char address);
void __I2CWriteByte8(unsigned char devaddress, unsigned char address, unsigned char data);

#define  I2CReadByte   __I2CReadByte8
#define  I2CWriteByte  __I2CWriteByte8

#endif

#if 0
#ifdef HI_I2C   // 2102/10/11 by lkf72840 把之前的 && 运算符 修改为 &  , 解决 I2C 下 9022 无输出的问题
unsigned char __I2CReadByte8(unsigned char devaddress, unsigned char address)
{
    return (HI_I2C_Read(devaddress, address, 1, 1) & 0xff);
}

void __I2CWriteByte8(unsigned char devaddress, unsigned char address, unsigned char data)
{
    HI_I2C_Write(devaddress, address, 1, data, 1);
	udelay(200);
}

#endif
#endif

// device address define
#define NVP6124_R0_ID 	0x84
#define NVP6114A_R0_ID 	0x85

#define NTSC		0x00
#define PAL			0x01

#define PELCO_CMD_RESET			0
#define PELCO_CMD_SET			1
#define PELCO_CMD_UP			2
#define PELCO_CMD_DOWN			3
#define PELCO_CMD_LEFT			4
#define PELCO_CMD_RIGHT			5
#define PELCO_CMD_OSD			6
#define PELCO_CMD_IRIS_OPEN		7
#define PELCO_CMD_IRIS_CLOSE	8
#define PELCO_CMD_FOCUS_NEAR	9
#define PELCO_CMD_FOCUS_FAR		10
#define PELCO_CMD_ZOOM_WIDE		11
#define PELCO_CMD_ZOOM_TELE		12
#define PELCO_CMD_SCAN_SR		13
#define PELCO_CMD_SCAN_ST		14
#define PELCO_CMD_PRESET1		15
#define PELCO_CMD_PRESET2		16
#define PELCO_CMD_PRESET3		17
#define PELCO_CMD_PTN1_SR		18
#define PELCO_CMD_PTN1_ST		19
#define PELCO_CMD_PTN2_SR		20
#define PELCO_CMD_PTN2_ST		21
#define PELCO_CMD_PTN3_SR       22
#define PELCO_CMD_PTN3_ST       23
#define PELCO_CMD_RUN           24

#define SET_ALL_CH          0xff

//FIXME HI3520 Register
#define VIU_CH_CTRL					0x08
#define VIU_ANC0_START				0x9c
#define VIU_ANC0_SIZE				0xa0
#define VIU_ANC1_START				0xa4
#define VIU_ANC1_SIZE				0xa8
#define VIU_BLANK_DATA_ADDR			0xac

#define IOC_VDEC_SET_VIDEO_MODE			0x07
#define IOC_VDEC_GET_INPUT_VIDEO_FMT	0x08  
#define IOC_VDEC_GET_VIDEO_LOSS     	0x09
#define IOC_VDEC_SET_SYNC		     	0x0A
#define IOC_VDEC_SET_EQUALIZER			0x0B
#define IOC_VDEC_GET_DRIVERVER			0x0C
#define IOC_VDEC_PTZ_ACP_READ			0x0D
#define IOC_VDEC_SET_BRIGHTNESS	    	0x0E
#define IOC_VDEC_SET_CONTRAST   		0x0F
#define IOC_VDEC_SET_HUE    			0x10
#define IOC_VDEC_SET_SATURATION  		0x11
#define IOC_VDEC_SET_SHARPNESS  		0x12
#define IOC_VDEC_SET_CHNMODE    		0x13
#define IOC_VDEC_SET_OUTPORTMODE  		0x14
#define IOC_VDEC_PTZ_CHANNEL_SEL		0x20
#define IOC_VDEC_PTZ_PELCO_INIT			0x21
#define IOC_VDEC_PTZ_PELCO_RESET		0x22
#define IOC_VDEC_PTZ_PELCO_SET			0x23
#define IOC_VDEC_PTZ_PELCO_UP			0x24
#define IOC_VDEC_PTZ_PELCO_DOWN			0x25
#define IOC_VDEC_PTZ_PELCO_LEFT			0x26
#define IOC_VDEC_PTZ_PELCO_RIGHT		0x27
#define IOC_VDEC_PTZ_PELCO_OSD			0x28
#define IOC_VDEC_PTZ_PELCO_IRIS_OPEN	0x29
#define IOC_VDEC_PTZ_PELCO_IRIS_CLOSE	0x2a
#define IOC_VDEC_PTZ_PELCO_FOCUS_NEAR	0x2b
#define IOC_VDEC_PTZ_PELCO_FOCUS_FAR	0x2c
#define IOC_VDEC_PTZ_PELCO_ZOOM_WIDE	0x2d
#define IOC_VDEC_PTZ_PELCO_ZOOM_TELE	0x2e
#define IOC_VDEC_ACP_WRITE              0x2f

#define IOC_VDEC_INIT_MOTION			0x40
#define IOC_VDEC_ENABLE_MOTION			0x41
#define IOC_VDEC_DISABLE_MOTION			0x42
#define IOC_VDEC_SET_MOTION_AREA		0x43
#define IOC_VDEC_GET_MOTION_INFO		0x44
#define IOC_VDEC_SET_MOTION_DISPLAY		0x45
#define IOC_VDEC_SET_MOTION_SENS		0x46

#define IOC_VDEC_ENABLE_LOW_RES			0x50
#define IOC_VDEC_DISABLE_LOW_RES		0x51

#define IOC_VDEC_ENABLE_BW				0x60
#define IOC_VDEC_DISABLE_BW				0x61
#define IOC_VDEC_READ_BALCK_COUNT		0x62
#define IOC_VDEC_READ_WHITE_COUNT		0x63
#define IOC_VDEC_4CH_VIDEO_SEQUENCE		0x64


#define IOC_AUDIO_SET_CHNNUM            0x80
#define IOC_AUDIO_SET_SAMPLE_RATE       0x81
#define IOC_AUDIO_SET_BITWIDTH          0x82
typedef struct _nvp6124_video_mode
{
    unsigned int chip;
    unsigned int mode;
	unsigned char vformat[16];
	unsigned char chmode[16];
}nvp6124_video_mode;

typedef struct _nvp6124_chn_mode
{
    unsigned char ch;
	unsigned char vformat;
	unsigned char chmode;
}nvp6124_chn_mode;

typedef struct _nvp6124_opt_mode
{
	unsigned char chipsel;
    unsigned char portsel;
	unsigned char portmode;
	unsigned char chid;
}nvp6124_opt_mode;

typedef struct _nvp6124_input_videofmt
{
    unsigned int inputvideofmt[16];
	unsigned int getvideofmt[16];
	unsigned int geteqstage[16];
	unsigned int getacpdata[16][8];
}nvp6124_input_videofmt;

typedef struct _nvp6124_video_adjust
{
    unsigned int ch;
	unsigned int value;
}nvp6124_video_adjust;

typedef struct _nvp6124_motion_area
{
    unsigned char ch;
    int m_info[12];
}nvp6124_motion_area;

typedef struct _nvp6124_audio_playback
{
    unsigned int chip;
    unsigned int ch;
}nvp6124_audio_playback;

typedef struct _nvp6124_audio_da_mute
{
    unsigned int chip;
}nvp6124_audio_da_mute;

typedef struct _nvp6124_audio_da_volume
{
    unsigned int chip;
    unsigned int volume;
}nvp6124_audio_da_volume;

typedef struct _nvp6124_audio_format
{
	unsigned char format;   /* 0:i2s; 1:dsp */
    unsigned char mode;   /* 0:slave 1:master*/
	unsigned char dspformat; /*0:dsp;1:ssp*/
    unsigned char clkdir;  /*0:inverted;1:non-inverted*/
	unsigned char chn_num; /*2,4,8,16*/
	unsigned char bitrate; /*0:256fs 1:384fs invalid for nvp6114 2:320fs*/
	unsigned char precision;/*0:16bit;1:8bit*/
	unsigned char samplerate;/*0:8kHZ;1:16kHZ; 2:32kHZ*/
} nvp6124_audio_format;


#define NVP6124_IOC_MAGIC            'n'

#define NVP6124_SET_AUDIO_PLAYBACK   		_IOW(NVP6124_IOC_MAGIC, 0x21, nvp6124_audio_playback) 
#define NVP6124_SET_AUDIO_DA_MUTE    		_IOW(NVP6124_IOC_MAGIC, 0x22, nvp6124_audio_da_mute)
#define NVP6124_SET_AUDIO_DA_VOLUME  		_IOW(NVP6124_IOC_MAGIC, 0x23, nvp6124_audio_da_volume)
/*set record format*/
#define NVP6124_SET_AUDIO_R_FORMAT     		_IOW(NVP6124_IOC_MAGIC, 0x24, nvp6124_audio_format)
/*set playback format*/
#define NVP6124_SET_AUDIO_PB_FORMAT     	_IOW(NVP6124_IOC_MAGIC, 0x25, nvp6124_audio_format)
			
#endif

