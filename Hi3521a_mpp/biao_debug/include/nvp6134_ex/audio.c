/********************************************************************************
*
*  Copyright (C) 2016 	NEXTCHIP Inc. All rights reserved.
*  Module		: The decoder's audio module
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
#include <linux/string.h>
#include <linux/delay.h>

//#include "common.h"
#include "nvp6134.h"
#include "audio.h"

#define I2C_ADDR (nvp6134_iic_addr[0])

/*******************************************************************************
 * extern variable
 *******************************************************************************/
extern int 				chip_id[4];				/* Chip ID */
extern int 				rev_id[4];				/* Reversion ID */
extern unsigned int 	nvp6134_cnt;			/* Chip count */
extern unsigned int 	nvp6134_iic_addr[4];	/* Slave address of Chip */

/*******************************************************************************
 * internal variable
 *******************************************************************************/


/*******************************************************************************
 *
 *
 *
 *  Internal Functions
 *
 *
 *
 *******************************************************************************/



/*******************************************************************************
 *
 *
 *
 *  External Functions
 *
 *
 *
 *******************************************************************************/
/*******************************************************************************
*	Description		: initialize audio
*	Argurments		: recmaster(0[slave],1[master];), pbmaster(), ch_num(audio channel number)
*					  samplerate(sample rate), bits(bits)
*	Return value	: void
*	Modify			:
*	warning			:
*
* 	 param:
*		- xxmaster:0[slave],1[master];
*		- ch_num: audio channel number
*		- samplerate: 0[8k], 1[16k]
*		- bits: 0[16bits], 1[8bits]
*
*******************************************************************************/
void audio_init(unsigned char recmaster, unsigned char pbmaster, unsigned char ch_num, unsigned char samplerate, unsigned char bits)
{
	int i;
	for(i=0;i<nvp6134_cnt;i++)
	{
		gpio_i2c_write(nvp6134_iic_addr[i], 0xff, 0x01);
		gpio_i2c_write(nvp6134_iic_addr[i], 0x94, 0x50);  //audio system clk = 74.25MHz
		gpio_i2c_write(nvp6134_iic_addr[i], 0x3a, 0x03);  //audio dac gain x1
		gpio_i2c_write(nvp6134_iic_addr[i], 0x3b, 0x30);  //audio afe gain x1
		gpio_i2c_write(nvp6134_iic_addr[i], 0x48, 0x10);  //audio sampling selection
	 	if(chip_id[i] == NVP6134B_R0_ID && rev_id[i]!=NVP6134B_REV_ID) 
			gpio_i2c_write(nvp6134_iic_addr[i], 0x31, 0x32); // NVP6134C 
		else
			gpio_i2c_write(nvp6134_iic_addr[i], 0x31, 0x82); // NVP6134&NVP6134B 
		
		if(i == 0)
		{
	 		if(chip_id[i] == NVP6134B_R0_ID && rev_id[i]!=NVP6134B_REV_ID) 
				gpio_i2c_write(nvp6134_iic_addr[i], 0x06, 0x3a); //Set chip0 NVP6134C to first stage
			else
				gpio_i2c_write(nvp6134_iic_addr[i], 0x06, 0x1a); // NVP6134&NVP6134B
			
			if(recmaster == 1)
			{
				gpio_i2c_write(nvp6134_iic_addr[i], 0x07, 0x80|(samplerate<<3)|(bits<<2));	//i2s rec: master
				gpio_i2c_write(nvp6134_iic_addr[i], 0x39, 0x00);
			}
			else
			{
				gpio_i2c_write(nvp6134_iic_addr[i], 0x07, 0x00|(samplerate<<3)|(bits<<2));	//i2s rec: slave
				gpio_i2c_write(nvp6134_iic_addr[i], 0x39, 0x80);
			}
			if(pbmaster == 1)
			{
				gpio_i2c_write(nvp6134_iic_addr[i], 0x13, 0x80|(samplerate<<3)|(bits<<2));	// PB I2C 8k 16bit : master
				gpio_i2c_write(nvp6134_iic_addr[i], 0xd5, 0x00);  
			}
			else
			{
				gpio_i2c_write(nvp6134_iic_addr[i], 0x13, 0x00|(samplerate<<3)|(bits<<2));	// PB I2C 8k 16bit : slave
				gpio_i2c_write(nvp6134_iic_addr[i], 0xd5, 0x01); 
			}
			
			if(8 == ch_num)
			{
  	 			if(chip_id[i] == NVP6134B_R0_ID && rev_id[i]!=NVP6134B_REV_ID) 
  					gpio_i2c_write(nvp6134_iic_addr[i], 0x06, 0x3a); // Set chip0 NVP6134C to first stage 
	  			else
		  			gpio_i2c_write(nvp6134_iic_addr[i], 0x06, 0x1b); // NVP6134&NVP6134B
 
				gpio_i2c_write(nvp6134_iic_addr[i], 0x08, 0x02);
				gpio_i2c_write(nvp6134_iic_addr[i], 0x0f, 0x54);    //set I2S right sequence
				gpio_i2c_write(nvp6134_iic_addr[i], 0x10, 0x76);
			}
			else if(4 == ch_num)
			{
				if(chip_id[i] == NVP6134B_R0_ID && rev_id[i]!=NVP6134B_REV_ID) 
					gpio_i2c_write(nvp6134_iic_addr[i], 0x06, 0x3b); // Set chip0 NVP6134C to single mode
				else
					gpio_i2c_write(nvp6134_iic_addr[i], 0x06, 0x1b); // NVP6134&NVP6134B

				gpio_i2c_write(nvp6134_iic_addr[i], 0x08, 0x01);
				gpio_i2c_write(nvp6134_iic_addr[i], 0x0f, 0x32);   //set I2S right sequence
			}
			
			gpio_i2c_write(nvp6134_iic_addr[i], 0x23, 0x10);  // Audio playback out
			//gpio_i2c_write(nvp6134_iic_addr[i], 0x23, 0x18);  // Audio mix out | for AD loopback test
		}
		else
		{
 	 		if(chip_id[i] == NVP6134B_R0_ID && rev_id[i]!=NVP6134B_REV_ID) 
 	 		{
 	 			if(8 == ch_num)
					gpio_i2c_write(nvp6134_iic_addr[i], 0x06, 0x39); // Set chip1 NVP6134C to last stage
				else if(16 == ch_num)
				{
					if(i==1 || i==2)
						gpio_i2c_write(nvp6134_iic_addr[i], 0x06, 0x3A); // Set chipx NVP6134C to middle stage
					else
						gpio_i2c_write(nvp6134_iic_addr[i], 0x06, 0x39); // Set chip3 NVP6134C to last stage
				}
 	 		}
			else
				gpio_i2c_write(nvp6134_iic_addr[i], 0x06, 0x19); // NVP6134&NVP6134B

			gpio_i2c_write(nvp6134_iic_addr[i], 0x07, 0x80|(samplerate<<3)|(bits<<2));	//Rec I2C 16K 16bit : master
			gpio_i2c_write(nvp6134_iic_addr[i], 0x39, 0x00);
			gpio_i2c_write(nvp6134_iic_addr[i], 0x13, 0x00|(samplerate<<3)|(bits<<2));	// PB I2C 8k 16bit : slave
			gpio_i2c_write(nvp6134_iic_addr[i], 0x23, 0x10);  // Audio playback out
			gpio_i2c_write(nvp6134_iic_addr[i], 0xd5, 0x01);
		}	
		gpio_i2c_write(nvp6134_iic_addr[i], 0x01, AIG_DEF);  // ch1 Audio input gain init
		gpio_i2c_write(nvp6134_iic_addr[i], 0x02, AIG_DEF);
		gpio_i2c_write(nvp6134_iic_addr[i], 0x03, AIG_DEF);
		gpio_i2c_write(nvp6134_iic_addr[i], 0x04, AIG_DEF);
		gpio_i2c_write(nvp6134_iic_addr[i], 0x05, AIG_DEF); 
		gpio_i2c_write(nvp6134_iic_addr[i], 0x40, AIG_DEF);  
		gpio_i2c_write(nvp6134_iic_addr[i], 0x41, AIG_DEF);
		gpio_i2c_write(nvp6134_iic_addr[i], 0x42, AIG_DEF);
		gpio_i2c_write(nvp6134_iic_addr[i], 0x43, AIG_DEF);
		gpio_i2c_write(nvp6134_iic_addr[i], 0x22, AOG_DEF);  
		
		gpio_i2c_write(nvp6134_iic_addr[i], 0x24, 0x14); //set mic_1's data to i2s_sp left channel
		gpio_i2c_write(nvp6134_iic_addr[i], 0x25, 0x15); //set mic_2's data to i2s_sp right channel

		gpio_i2c_write(nvp6134_iic_addr[i], 0x44, 0x11);  //audio LPF to reduce noise
		//gpio_i2c_write(nvp6134_iic_addr[i], 0x39, 0x00);
		gpio_i2c_write(nvp6134_iic_addr[i], 0x94, 0x50);
		gpio_i2c_write(nvp6134_iic_addr[i], 0x49, 0x88);  //audio live mute 
		gpio_i2c_write(nvp6134_iic_addr[i], 0x38, 0x18);
		msleep(20);
		gpio_i2c_write(nvp6134_iic_addr[i], 0x38, 0x08);
		
		printk("nvp6134 audio init,CH:%d REC:%s,PB:%s\n", ch_num, recmaster==1?"MASTER":"SLAVE", pbmaster==1?"MASTER":"SLAVE");
	}
}

int nvp6124_audio_adjust_r_seq(unsigned char dec, unsigned char chn_num)
{
	switch (chn_num)
	{
		case 2:
			gpio_i2c_write(dec, 0x0f, 0x01);    //set I2S right sequence
			break;

		case 4:
			gpio_i2c_write(dec, 0x0f, 0x32);    //set I2S right sequence
			break;

		case 8:
			gpio_i2c_write(dec, 0x0f, 0x54);    //set I2S right sequence
			gpio_i2c_write(dec, 0x10, 0x76);
			break;

		case 16:
			gpio_i2c_write(dec, 0x0f, 0x98);    //set I2S right sequence
			gpio_i2c_write(dec, 0x10, 0xba);
			break;

		default:

			break;
	}

	return 0;
}
/*
type:
0:record
1:playback
*/
int nvp6134_audio_set_format(unsigned char type, nvp6134_audio_format format)
{
	unsigned char cformat = 0;
	unsigned char chn_num = 0;
	unsigned char temp = 0;

	if ((format.chn_num != 2) && (format.chn_num != 4) && (format.chn_num != 8) && (format.chn_num != 16))
	{
		return -1;
	}
	
	if ((format.mode != 0) && (format.mode != 1))
	{
		return -1;
	}

	if ((format.clkdir != 0) && (format.clkdir != 1))
	{
		return -1;
	}

	if ((format.bitrate != 0) && (format.bitrate != 2))
	{
		return -1;
	}

	if ((format.samplerate != 0) && (format.samplerate != 1) && (format.samplerate != 2))
	{
		return -1;
	}

	if ((format.precision != 0) && (format.precision != 1))
	{
		return -1;
	}

	if ((format.dspformat != 0) && (format.dspformat != 1))
	{
		return -1;
	}

	if ((format.format != 0) && (format.format != 1))
	{
		return -1;
	}

	gpio_i2c_write(I2C_ADDR, 0xff, 0x01);

	temp = gpio_i2c_read(I2C_ADDR, 0x0);
	temp &= ~0x1;

	if (format.mode == 1)
	{
		gpio_i2c_write(I2C_ADDR, 0x39, 0x1);
	}
	else
	{
		gpio_i2c_write(I2C_ADDR, 0x39, 0x81);
	}

	/*32kHz or other*/
	if (format.samplerate == 2)
	{
		temp |= 0x1;
	}
	else
	{
		temp |= 0x0;
	}
	gpio_i2c_write(I2C_ADDR, 0x0, temp);
	
	if (type == 0)
	{
		/*set record channel number*/
		chn_num = gpio_i2c_read(I2C_ADDR, 0x8);
		chn_num &= 0xFC;
		
		switch (format.chn_num)
		{
			case 2:
				chn_num |= 0x0;
				break;

			case 4:
				chn_num |= 0x1;
				break;

			case 8:
				chn_num |= 0x2;
				break;

			case 16:
				chn_num |= 0x3;
				break;

			default:
				chn_num |= 0x3;
				break;
		}

		gpio_i2c_write(I2C_ADDR, 0x8, chn_num);

		nvp6124_audio_adjust_r_seq(I2C_ADDR, format.chn_num);
		//printk("0x8 Register:%x\n", gpio_i2c_read(I2C_ADDR, 0x8));
#if 1
		cformat = gpio_i2c_read(I2C_ADDR, 0x07);
		cformat &= ~0xbf;
#else
		cformat = 0;
#endif
		/*set master or slave mode*/
		cformat |= format.mode << 7;
#if 0
		/*clk polarity*/
		//cformat |= format.clkdir << 6;
		if ((format.mode == 0) && (format.format != 0))
		{
			cformat |= 1 << 6;
		}
		else
		{
			cformat |= 0 << 6;
		}
#endif
		/* bitrate*/
		cformat |= format.bitrate << 4;

		/*samplerate*/
		cformat |= format.samplerate << 3;

		/*bit width*/
		cformat |= format.precision << 2;

		/*dsp or ssp*/
		cformat |= format.dspformat << 1;

		/*i2s or dsp*/
		cformat |= format.format;
	
		gpio_i2c_write(I2C_ADDR, 0x07, cformat);
		//printk("0x7 Register:%x\n", gpio_i2c_read(I2C_ADDR, 0x7));
		//gpio_i2c_write(I2C_ADDR, 0x13, cformat);
		//printk("0x13 Register:%x\n", gpio_i2c_read(I2C_ADDR, 0x13));
	}
	else if (type == 1)
	{
#if 0
		cformat = gpio_i2c_read(I2C_ADDR, 0x13);
		cformat &= ~0xbf;
#else
		cformat = 0;
#endif
		/*set master or slave mode*/
		cformat |= format.mode << 7;

		/*clk polarity*/
		if ((format.format == 0))
		{
			cformat |= 0 << 6;
		}
		else
		{
			cformat |= 1 << 6;
		}

		/* bitrate*/
		cformat |= format.bitrate << 4;

		/*samplerate*/
		cformat |= format.samplerate << 3;

		/*bit width*/
		cformat |= format.precision << 2;

		/*dsp or ssp*/
		cformat |= format.dspformat << 1;

		/*i2s or dsp*/
		cformat |= format.format;
		
		gpio_i2c_write(I2C_ADDR, 0x13, cformat);
		//printk("0x13 Register:%x\n", gpio_i2c_read(I2C_ADDR, 0x13));

		temp = gpio_i2c_read(I2C_ADDR, 0xD5);
		temp &= ~0x1;
		/*slave*/
		if (format.mode == 0)
		{
			temp |= 0x1;
		}
		/*master*/
		else
		{
			temp |= 0x0;
		}
		gpio_i2c_write(I2C_ADDR, 0xD5, temp);
	}	

	return 0;
}
/*******************************************************************************
*	End of file
*******************************************************************************/
