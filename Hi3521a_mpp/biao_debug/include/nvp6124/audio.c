#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/fcntl.h>
#include <linux/mm.h>
#include <linux/miscdevice.h>
#include <linux/proc_fs.h>

#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/system.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/string.h>
#include <linux/list.h>
#include <asm/delay.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/proc_fs.h>
#include <linux/poll.h>
#include <asm/bitops.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <linux/moduleparam.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include "nvp6124.h"

extern unsigned int nvp6124_cnt;
extern unsigned int nvp6124_slave_addr[4];

#define I2C_ADDR (0x60)

void audio_read_reg(unsigned char dec)
{
	unsigned char reg_0x6;
	unsigned char reg_0x7;
	unsigned char reg_0x8;
	unsigned char reg_0xf;
	unsigned char reg_0x10;
	unsigned char reg_0x13;
	unsigned char reg_0x23;
	unsigned char reg_0x24;
	unsigned char reg_0x25;

	/*R_SEQ_x*/
	unsigned char reg_0x9;
	unsigned char reg_0xa;
	unsigned char reg_0xb;
	unsigned char reg_0xc;
	unsigned char reg_0xd;
	unsigned char reg_0xe;
	unsigned char reg_0x11;
	unsigned char reg_0x12;
	unsigned char reg_0x3c;
	unsigned char reg_0x3d;

	reg_0x6 = I2CReadByte(dec, 0x06);
	reg_0x7 = I2CReadByte(dec, 0x07);
	reg_0x8 = I2CReadByte(dec, 0x08);
	reg_0xf = I2CReadByte(dec, 0x0f);
	reg_0x10 = I2CReadByte(dec, 0x10);
	reg_0x13 = I2CReadByte(dec, 0x13);
	reg_0x23 = I2CReadByte(dec, 0x23);
	reg_0x24 = I2CReadByte(dec, 0x24);
	reg_0x25 = I2CReadByte(dec, 0x25);
	
	reg_0x9 = I2CReadByte(dec, 0x09);
	reg_0xa = I2CReadByte(dec, 0x0a);
	reg_0xb = I2CReadByte(dec, 0x0b);
	reg_0xc = I2CReadByte(dec, 0x0c);
	reg_0xd = I2CReadByte(dec, 0x0d);
	reg_0xe = I2CReadByte(dec, 0x0e);
	reg_0xf = I2CReadByte(dec, 0x0f);
	reg_0x10 = I2CReadByte(dec, 0x10);
	reg_0x11 = I2CReadByte(dec, 0x11);
	reg_0x12 = I2CReadByte(dec, 0x12);
	reg_0x3c = I2CReadByte(dec, 0x3c);
	reg_0x3d = I2CReadByte(dec, 0x3d);

	printk("addr:%x, value:%x\n", 0x0, I2CReadByte(dec, 0x00));
	printk("addr:%x, value:%x\n", 0x6, reg_0x6);
	printk("addr:%x, value:%x\n", 0x7, reg_0x7);
	printk("addr:%x, value:%x\n", 0x8, reg_0x8);
	printk("addr:%x, value:%x\n", 0xf, reg_0xf);
	printk("addr:%x, value:%x\n", 0x10, reg_0x10);
	printk("addr:%x, value:%x\n", 0x13, reg_0x13);
	printk("addr:%x, value:%x\n", 0x23, reg_0x23);
	printk("addr:%x, value:%x\n", 0x24, reg_0x24);
	printk("addr:%x, value:%x\n", 0x25, reg_0x25);

	printk("R_SEQ_X\n");
	printk("addr:%x, value:%x\n", 0x9, reg_0x9);
	printk("addr:%x, value:%x\n", 0xa, reg_0xa);
	printk("addr:%x, value:%x\n", 0xb, reg_0xb);
	printk("addr:%x, value:%x\n", 0xc, reg_0xc);
	printk("addr:%x, value:%x\n", 0xd, reg_0xd);
	printk("addr:%x, value:%x\n", 0xe, reg_0xe);
	printk("addr:%x, value:%x\n", 0xf, reg_0xf);
	printk("addr:%x, value:%x\n", 0x10, reg_0x10);
	printk("addr:%x, value:%x\n", 0x11, reg_0x11);
	printk("addr:%x, value:%x\n", 0x12, reg_0x12);
	printk("addr:%x, value:%x\n", 0x3c, reg_0x3c);
	printk("addr:%x, value:%x\n", 0x3d, reg_0x3d);
	printk("addr:%x, value:%x\n", 0xd5, I2CReadByte(dec, 0xd5));
}

/*
param:
dec[4]= {0x60,0x62,0x64,0x66};
ch_num: audio channel number
samplerate: 0[8k], 1[16k]
bits: 0[16bits], 1[8bits]
*/
void audio_init(unsigned char recmaster, unsigned char pbmaster, unsigned char ch_num, unsigned char samplerate, unsigned char bits)
{
	int i;
	
	
	for(i=0;i<nvp6124_cnt;i++)
	{
		I2CWriteByte(nvp6124_slave_addr[i], 0xff, 0x01);
		if(i == 0)
		{
			I2CWriteByte(nvp6124_slave_addr[i], 0x06, 0x1a);
			if(recmaster == 1)
			{
				I2CWriteByte(nvp6124_slave_addr[i], 0x07, 0x80|(samplerate<<3)|(bits<<2));	//i2s rec: master
				I2CWriteByte(nvp6124_slave_addr[i], 0x39, 0x01);
			}
			else
			{
				I2CWriteByte(nvp6124_slave_addr[i], 0x07, 0x00|(samplerate<<3)|(bits<<2));	//i2s rec: slave
				I2CWriteByte(nvp6124_slave_addr[i], 0x39, 0x81);
			}
			if(pbmaster == 1)
			{
				I2CWriteByte(nvp6124_slave_addr[i], 0x13, 0x80|(samplerate<<3)|(bits<<2));	// PB I2C 8k 16bit : master
				I2CWriteByte(nvp6124_slave_addr[i], 0xd5, 0x00);  
			}
			else
			{
				I2CWriteByte(nvp6124_slave_addr[i], 0x13, 0x00|(samplerate<<3)|(bits<<2));	// PB I2C 8k 16bit : slave
				I2CWriteByte(nvp6124_slave_addr[i], 0xd5, 0x01); 
			}
			if(8 == ch_num)
			{
				I2CWriteByte(nvp6124_slave_addr[i], 0x06, 0x1b);
				I2CWriteByte(nvp6124_slave_addr[i], 0x08, 0x02);
				I2CWriteByte(nvp6124_slave_addr[i], 0x0f, 0x54);    //set I2S right sequence
				I2CWriteByte(nvp6124_slave_addr[i], 0x10, 0x76);
			}
			else if(4 == ch_num)
			{
				I2CWriteByte(nvp6124_slave_addr[i], 0x06, 0x1b);
				I2CWriteByte(nvp6124_slave_addr[i], 0x08, 0x01);
				I2CWriteByte(nvp6124_slave_addr[i], 0x0f, 0x32);   //set I2S right sequence
			}

	//		I2CWriteByte(dec, 0x13, 0x00|(samplerate<<3)|(bits<<2));	// PB I2C 8k 16bit : slave		
			I2CWriteByte(nvp6124_slave_addr[i], 0x23, 0x10);  // Audio playback out
	//		I2CWriteByte(dec, 0x23, 0x18);  // Audio mix out


		}
		else
		{
			I2CWriteByte(nvp6124_slave_addr[i], 0x06, 0x19);	
			I2CWriteByte(nvp6124_slave_addr[i], 0x07, 0x00|(samplerate<<3)|(bits<<2));	//Rec I2C 16K 16bit : slave
			I2CWriteByte(nvp6124_slave_addr[i], 0x13, 0x00|(samplerate<<3)|(bits<<2));	// PB I2C 8k 16bit : slave
		}	
		I2CWriteByte(nvp6124_slave_addr[i], 0x01, 0x0f);  // ch1 Audio input gain init
		I2CWriteByte(nvp6124_slave_addr[i], 0x02, 0x0f);
		I2CWriteByte(nvp6124_slave_addr[i], 0x03, 0x0f);
		I2CWriteByte(nvp6124_slave_addr[i], 0x04, 0x0f);
		I2CWriteByte(nvp6124_slave_addr[i], 0x05, 0x0f); //mic gain
		I2CWriteByte(nvp6124_slave_addr[i], 0x40, 0x0f);  //ch5 
		I2CWriteByte(nvp6124_slave_addr[i], 0x41, 0x0f);
		I2CWriteByte(nvp6124_slave_addr[i], 0x42, 0x0f);
		I2CWriteByte(nvp6124_slave_addr[i], 0x43, 0x0f);
		I2CWriteByte(nvp6124_slave_addr[i], 0x22, 0x03);  //aogain
		
		I2CWriteByte(nvp6124_slave_addr[i], 0x24, 0x14); //set mic_1's data to i2s_sp left channel
		I2CWriteByte(nvp6124_slave_addr[i], 0x25, 0x15); //set mic_2's data to i2s_sp right channel
		printk("nvp6124 audio init,REC:%s,PB:%s\n", recmaster==1?"MASTER":"SLAVE", pbmaster==1?"MASTER":"SLAVE");
	}
		
	
}

int nvp6124_audio_adjust_r_seq(unsigned char dec, unsigned char chn_num)
{
	switch (chn_num)
	{
		case 2:
			I2CWriteByte(dec, 0x0f, 0x01);    //set I2S right sequence
			break;

		case 4:
			I2CWriteByte(dec, 0x0f, 0x32);    //set I2S right sequence
			break;

		case 8:
			I2CWriteByte(dec, 0x0f, 0x54);    //set I2S right sequence
			I2CWriteByte(dec, 0x10, 0x76);
			break;

		case 16:
			I2CWriteByte(dec, 0x0f, 0x98);    //set I2S right sequence
			I2CWriteByte(dec, 0x10, 0xba);
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
int nvp6124_audio_set_format(unsigned char type, nvp6124_audio_format format)
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

	I2CWriteByte(I2C_ADDR, 0xff, 0x01);

	temp = I2CReadByte(I2C_ADDR, 0x0);
	temp &= ~0x1;

	if (format.mode == 1)
	{
		I2CWriteByte(I2C_ADDR, 0x39, 0x1);
	}
	else
	{
		I2CWriteByte(I2C_ADDR, 0x39, 0x81);
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
	I2CWriteByte(I2C_ADDR, 0x0, temp);
	
	if (type == 0)
	{
		/*set record channel number*/
		chn_num = I2CReadByte(I2C_ADDR, 0x8);
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

		I2CWriteByte(I2C_ADDR, 0x8, chn_num);

		nvp6124_audio_adjust_r_seq(I2C_ADDR, format.chn_num);
		//printk("0x8 Register:%x\n", I2CReadByte(I2C_ADDR, 0x8));
#if 1
		cformat = I2CReadByte(I2C_ADDR, 0x07);
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
	
		I2CWriteByte(I2C_ADDR, 0x07, cformat);
		//printk("0x7 Register:%x\n", I2CReadByte(I2C_ADDR, 0x7));
		//I2CWriteByte(I2C_ADDR, 0x13, cformat);
		//printk("0x13 Register:%x\n", I2CReadByte(I2C_ADDR, 0x13));
	}
	else if (type == 1)
	{
#if 0
		cformat = I2CReadByte(I2C_ADDR, 0x13);
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
		
		I2CWriteByte(I2C_ADDR, 0x13, cformat);
		//printk("0x13 Register:%x\n", I2CReadByte(I2C_ADDR, 0x13));

		temp = I2CReadByte(I2C_ADDR, 0xD5);
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
		I2CWriteByte(I2C_ADDR, 0xD5, temp);
	}	

	return 0;
}
