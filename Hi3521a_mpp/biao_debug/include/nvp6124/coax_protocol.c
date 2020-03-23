/**************************************************************************
	Header file include
**************************************************************************/
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
//#include "gpio_i2c.h"
#include "nvp6124.h"
#include "video.h"
#include "coax_protocol.h"

extern unsigned int nvp6124_cnt;
extern unsigned int nvp6124_mode;
extern unsigned char ch_mode_status[16];
extern unsigned int nvp6124_slave_addr[4];

/**************************************************************************
	Function prototype
**************************************************************************/
void nvp6124_pelco_coax_mode(unsigned char ch);
void nvp6124_pelco_shout(unsigned char pel_ch);
void init_acp(unsigned char ch);
void acp_each_setting(unsigned char ch);
unsigned char read_acp_status(unsigned char ch);
unsigned char read_acp_pattern(unsigned char ch);
void init_acp_camera_status(unsigned char ch);
void init_acp_reg_wr(unsigned char ch);
void init_acp_reg_rd(unsigned char ch);
void acp_reg_rx_clear(unsigned char ch);
void set_acp_reg_wr(unsigned char bank, unsigned char addr, unsigned char data);
void get_acp_reg_rd(unsigned char bank, unsigned char addr);
void acp_read(nvp6124_input_videofmt *pvideoacp, unsigned char ch);
unsigned char nvp6124_pelco_command(unsigned char pel_ch, unsigned char command);
void acp_sw_ver_transfer(unsigned char ch);
void acp_isp_write(unsigned char ch, unsigned int reg_addr, unsigned char reg_data);

extern int chip_id[4];

void init_acp(unsigned char ch)

	{
	I2CWriteByte(nvp6124_slave_addr[ch/4], 0xFF, 0x01);
		//Decoder TX Setting
		if(chip_id[0] == NVP6124_R0_ID)
		{
		I2CWriteByte(nvp6124_slave_addr[ch/4], 0xBD, 0xD0);
		I2CWriteByte(nvp6124_slave_addr[ch/4], 0xBE, 0xDD);
		I2CWriteByte(nvp6124_slave_addr[ch/4], 0xBF, 0x0D);
		}
		else if(chip_id[0] == NVP6114A_R0_ID)
		{
		I2CWriteByte(nvp6124_slave_addr[ch/4], 0xBC, 0xDD);
		I2CWriteByte(nvp6124_slave_addr[ch/4], 0xBD, 0xED);
	}

		acp_reg_rx_clear(ch);
}

void acp_each_setting(unsigned char ch)
{
	I2CWriteByte(nvp6124_slave_addr[ch/4], 0xFF, 0x03+((ch%4)/2));
	if(ch_mode_status[ch] == NVP6124_VI_720P_2530)
	{
		I2CWriteByte(nvp6124_slave_addr[ch/4], AHD2_FHD_BAUD+((ch%2)*0x80), 0x0E);
		I2CWriteByte(nvp6124_slave_addr[ch/4], AHD2_FHD_LINE+((ch%2)*0x80), nvp6124_mode%2? 0x0D:0x0E);
	}
	else if(ch_mode_status[ch] == NVP6124_VI_1080P_2530)
	{
		I2CWriteByte(nvp6124_slave_addr[ch/4], AHD2_FHD_BAUD+((ch%2)*0x80), 0x0E);
		I2CWriteByte(nvp6124_slave_addr[ch/4], AHD2_FHD_LINE+((ch%2)*0x80), 0x0E);
	}
	I2CWriteByte(nvp6124_slave_addr[ch/4], AHD2_PEL_SYNC+((ch%2)*0x80), 0x14);
	I2CWriteByte(nvp6124_slave_addr[ch/4], AHD2_PEL_SYNC+((ch%2)*0x80)+1, 0x00);  
	I2CWriteByte(nvp6124_slave_addr[ch/4], AHD2_FHD_LINES+((ch%2)*0x80), 0x07);
	I2CWriteByte(nvp6124_slave_addr[ch/4], AHD2_FHD_BYTE+((ch%2)*0x80), 0x03);
	I2CWriteByte(nvp6124_slave_addr[ch/4], AHD2_FHD_MODE+((ch%2)*0x80), 0x10);
	I2CWriteByte(nvp6124_slave_addr[ch/4], AHD2_PEL_EVEN+((ch%2)*0x80), 0x00);

	//Decoder RX Setting
	I2CWriteByte(nvp6124_slave_addr[ch/4], 0xFF, 0x05+(ch%4));
	I2CWriteByte(nvp6124_slave_addr[ch/4], 0x30, 0x00);
	I2CWriteByte(nvp6124_slave_addr[ch/4], 0x31, 0x01);
	I2CWriteByte(nvp6124_slave_addr[ch/4], 0x32, 0x64);
	I2CWriteByte(nvp6124_slave_addr[ch/4], 0x7C, 0x11);
	if(ch_mode_status[ch] == NVP6124_VI_720P_2530)
	{
		I2CWriteByte(nvp6124_slave_addr[ch/4], 0x7D, 0x80);
	}
	else if(ch_mode_status[ch] == NVP6124_VI_1080P_2530)
	{
		I2CWriteByte(nvp6124_slave_addr[ch/4], 0x7D, 0x80);
	}
	I2CWriteByte(nvp6124_slave_addr[ch/4], 0xFF, 0x03+((ch%4)/2));

	if(ch_mode_status[ch] == NVP6124_VI_720P_2530)
	{
		I2CWriteByte(nvp6124_slave_addr[ch/4], 0x62+((ch%2)*0x80), nvp6124_mode%2? 0x05:0x06);	// Camera TX DATA Check
		I2CWriteByte(nvp6124_slave_addr[ch/4], 0x68+((ch%2)*0x80), 0x40);	// RX size
	}
	else if(ch_mode_status[ch] == NVP6124_VI_1080P_2530)
	{
		I2CWriteByte(nvp6124_slave_addr[ch/4], 0x62+((ch%2)*0x80), 0x06);	// Camera TX DATA Check
		I2CWriteByte(nvp6124_slave_addr[ch/4], 0x68+((ch%2)*0x80), 0x70);	// RX size
	}

	I2CWriteByte(nvp6124_slave_addr[ch/4], 0x60+((ch%2)*0x80), 0x55);
	I2CWriteByte(nvp6124_slave_addr[ch/4], 0x63+((ch%2)*0x80), 0x01);
	I2CWriteByte(nvp6124_slave_addr[ch/4], 0x66+((ch%2)*0x80), 0x80);
	I2CWriteByte(nvp6124_slave_addr[ch/4], 0x67+((ch%2)*0x80), 0x01);
	printk("ACP DATA READ TEST!!!!CH%d  RESOL = %s\n\r",ch, ch_mode_status[ch]==NVP6124_VI_720P_2530?"720P":"1080P");
}

unsigned char read_acp_status(unsigned char ch)
{
	unsigned char val;

	I2CWriteByte(nvp6124_slave_addr[ch/4], 0xFF, 0x03+((ch%4)/2));
	val = I2CReadByte( nvp6124_slave_addr[ch/4], 0x78+((ch%2)*0x80));

	return val;
}

unsigned char read_acp_pattern(unsigned char ch)
{
	unsigned char val;

	I2CWriteByte(nvp6124_slave_addr[ch/4], 0xFF, 0x03+((ch%4)/2));
	val = I2CReadByte(nvp6124_slave_addr[ch/4],0x78+((ch%2)*0x80)+1);
	val = (val >> 2) & 0x03;

	return val;
}

void init_acp_camera_status(unsigned char ch)
{
	I2CWriteByte(nvp6124_slave_addr[ch/4], 0xFF, 0x03+((ch%4)/2));
	I2CWriteByte(nvp6124_slave_addr[ch/4], AHD2_FHD_LINES+((ch%2)*0x80), 0x07);		
	I2CWriteByte(nvp6124_slave_addr[ch/4], ACP_MODE_ID+((ch%2)*0x80), ACP_CAM_STAT);
	I2CWriteByte(nvp6124_slave_addr[ch/4], AHD2_FHD_D0+((ch%2)*0x80), ACP_RX_D0);
	I2CWriteByte(nvp6124_slave_addr[ch/4], AHD2_FHD_D0+((ch%2)*0x80)+1,ACP_RX_D0+1);
	I2CWriteByte(nvp6124_slave_addr[ch/4], AHD2_FHD_D0+((ch%2)*0x80)+2,ACP_RX_D0+2);
	I2CWriteByte(nvp6124_slave_addr[ch/4], AHD2_FHD_D0+((ch%2)*0x80)+3,ACP_RX_D0+3);
	I2CWriteByte(nvp6124_slave_addr[ch/4], AHD2_FHD_OUT+((ch%2)*0x80), 0x08);
	if((ch%4)==0)	msleep( 100 );
	printk("ACP_Camera_status_mode_set CH%d\n",ch);
}

void init_acp_reg_wr(unsigned char ch)
{
	I2CWriteByte(nvp6124_slave_addr[ch/4], 0xFF, 0x03+((ch%4)/2));
	I2CWriteByte(nvp6124_slave_addr[ch/4], AHD2_FHD_LINES+((ch%2)*0x80), 0x03);	
	I2CWriteByte(nvp6124_slave_addr[ch/4], ACP_MODE_ID+((ch%2)*0x80), ACP_REG_WR);
	printk("ACP_register_write_mode_set CH%d\n",ch);
}

void init_acp_reg_rd(unsigned char ch)
{
	I2CWriteByte(nvp6124_slave_addr[ch/4], 0xFF, 0x03+((ch%4)/2));
	I2CWriteByte(nvp6124_slave_addr[ch/4], AHD2_FHD_LINES+((ch%2)*0x80), 0x03);
	I2CWriteByte(nvp6124_slave_addr[ch/4], ACP_MODE_ID+((ch%2)*0x80), ACP_REG_RD);
	printk("ACP_register_read_mode_set CH%d\n",ch);
}

#define DRIVER_SW_VER_MAJOR     0x02
#define DRIVER_SW_VER_MINOR     0x00
void acp_sw_ver_transfer(unsigned char ch)
{
	acp_isp_write(ch, 0x8485, DRIVER_SW_VER_MAJOR);
	msleep(100);
	acp_isp_write(ch, 0x8486, DRIVER_SW_VER_MINOR);
	msleep(100);
	I2CWriteByte( nvp6124_slave_addr[ch/4], 0x09+((ch%2)*0x80), 0x00);
}
unsigned char acp_isp_read(unsigned char ch, unsigned int reg_addr)
{
	unsigned int data;
	unsigned char bank;
	unsigned char addr;
	bank = (reg_addr>>8)&0xFF;
	addr = reg_addr&0xFF;
	init_acp_reg_rd(ch);
	get_acp_reg_rd(bank, addr);
	I2CWriteByte(nvp6124_slave_addr[ch/4], 0xFF, 0x03+((ch%4)/2));
	data = I2CReadByte(nvp6124_slave_addr[ch/4],ACP_RX_D0+3+((ch%2)*0x80));
	I2CWriteByte( nvp6124_slave_addr[ch/4], AHD2_FHD_OUT+((ch%2)*0x80), 0x00);
	acp_reg_rx_clear(ch);
	return data;
}
void acp_isp_write(unsigned char ch, unsigned int reg_addr, unsigned char reg_data)
{
	I2CWriteByte( nvp6124_slave_addr[ch/4], 0xFF, 0x03+((ch%4)/2));
	I2CWriteByte( nvp6124_slave_addr[ch/4], AHD2_FHD_LINES+((ch%2)*0x80), 0x03);
	I2CWriteByte( nvp6124_slave_addr[ch/4], ACP_MODE_ID+((ch%2)*0x80), ACP_REG_WR);
	I2CWriteByte( nvp6124_slave_addr[ch/4], 0x10+((ch%2)*0x80), ACP_REG_WR);
	I2CWriteByte( nvp6124_slave_addr[ch/4], 0x11+((ch%2)*0x80), (reg_addr>>8)&0xFF);
	I2CWriteByte( nvp6124_slave_addr[ch/4], 0x12+((ch%2)*0x80), reg_addr&0xFF);
	I2CWriteByte( nvp6124_slave_addr[ch/4], 0x13+((ch%2)*0x80), reg_data);
	I2CWriteByte( nvp6124_slave_addr[ch/4], 0x09+((ch%2)*0x80), 0x08);
}
void set_acp_reg_wr(unsigned char bank, unsigned char addr, unsigned char data)
{
	unsigned char ch;

	for(ch=0;ch<nvp6124_cnt*4;ch++)
	{
		I2CWriteByte(nvp6124_slave_addr[ch/4], 0xFF, 0x03+((ch%4)/2));
		I2CWriteByte(nvp6124_slave_addr[ch/4], AHD2_FHD_D0, ACP_REG_WR);
		I2CWriteByte(nvp6124_slave_addr[ch/4], AHD2_FHD_D0+1, bank);
		I2CWriteByte(nvp6124_slave_addr[ch/4], AHD2_FHD_D0+2, addr);
		I2CWriteByte(nvp6124_slave_addr[ch/4], AHD2_FHD_D0+3, data);	
		I2CWriteByte(nvp6124_slave_addr[ch/4], AHD2_FHD_OUT, 0x08);
		if((ch%4)==0)	msleep( 100 );
		printk("set_ACP_register_write CH%d\n",ch);
	}
}

void get_acp_reg_rd(unsigned char bank, unsigned char addr)
{
	unsigned char ch;

	for(ch=0;ch<nvp6124_cnt*4;ch++)
	{
		I2CWriteByte(nvp6124_slave_addr[ch/4], 0xFF, 0x03+((ch%4)/2));
		I2CWriteByte(nvp6124_slave_addr[ch/4], AHD2_FHD_D0, ACP_REG_RD);
		I2CWriteByte(nvp6124_slave_addr[ch/4], AHD2_FHD_D0+1, bank);
		I2CWriteByte(nvp6124_slave_addr[ch/4], AHD2_FHD_D0+2, addr);
		I2CWriteByte(nvp6124_slave_addr[ch/4], AHD2_FHD_D0+3, 0x00);//Dummy
		I2CWriteByte(nvp6124_slave_addr[ch/4], AHD2_FHD_OUT, 0x08);
		if((ch%4)==0)	msleep( 100 );
		printk("get_register_read CH%d\n",ch);
	}
}


void acp_read(nvp6124_input_videofmt *pvideoacp, unsigned char ch)
{
	unsigned int buf[16];
	unsigned char val, i;
	//unsigned char ch;

	//for(ch=0;ch<nvp6124_cnt*4;ch++)
	{
	
		I2CWriteByte(nvp6124_slave_addr[ch/4], 0xFF, 0x00);
		printk("ACP_RX_DONE = 0x%x\n",(I2CReadByte(nvp6124_slave_addr[ch/4], 0xc7)>>(ch%4))&0x01);
		I2CWriteByte(nvp6124_slave_addr[ch/4], 0xFF, 0x03+((ch%4)/2));

		val = I2CReadByte(nvp6124_slave_addr[ch/4], ACP_RX_D0+((ch%2)*0x80));
		if(val == ACP_CAM_STAT)
		{
			for(i=0;i<8;i++)
			{
				buf[i] = I2CReadByte(nvp6124_slave_addr[ch/4],ACP_RX_D0+((ch%2)*0x80)+i);
				I2CWriteByte(nvp6124_slave_addr[ch/4], AHD2_FHD_D0+((ch%2)*0x80)+i, buf[i]); 
				pvideoacp->getacpdata[ch][i] = buf[i];
			}
			I2CWriteByte(nvp6124_slave_addr[ch/4], AHD2_FHD_OUT+((ch%2)*0x80), 0x08); 

			printk("ACP_DATA = [%02x %02x %02x %02x %02x %02x %02x %02x]\n", \
					buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]);
		}
		else if(val == ACP_REG_WR)
		{
			for(i=0;i<4;i++)
			{
				buf[i] = I2CReadByte(nvp6124_slave_addr[ch/4],ACP_RX_D0+((ch%2)*0x80)+i);
				pvideoacp->getacpdata[ch][i] = buf[i];
			}
			I2CWriteByte(nvp6124_slave_addr[ch/4], AHD2_FHD_OUT+((ch%2)*0x80), 0x00);

//			printk("ACP_Write = 0x%02x 0x%02x 0x%02x 0x%02x\n",buf[0], buf[1], buf[2]. buf[3]);
		}
		else if(val == ACP_REG_RD)
		{
			for(i=0;i<4;i++)
			{
				buf[i] = I2CReadByte(nvp6124_slave_addr[ch/4],ACP_RX_D0+((ch%2)*0x80)+i);
				pvideoacp->getacpdata[ch][i] = buf[i];
			}
			I2CWriteByte(nvp6124_slave_addr[ch/4], AHD2_FHD_OUT+((ch%2)*0x80), 0x00);

			printk("ACP_Read = 0x%02x 0x%02x 0x%02x 0x%02x\n",buf[0], buf[1], buf[2], buf[3]);
		}
		else
		{
			for(i=0;i<8;i++)
			{
				pvideoacp->getacpdata[ch][i] = 0x00;
			}
			I2CWriteByte(nvp6124_slave_addr[ch/4], AHD2_FHD_OUT+((ch%2)*0x80), 0x00);		
			printk("ACP_RX_Error!!!!\n");
		}
		acp_reg_rx_clear(ch);
	}
}

void acp_reg_rx_clear(unsigned char ch)
{
	I2CWriteByte(nvp6124_slave_addr[ch/4], 0xFF, 0x03+((ch%4)/2));
	I2CWriteByte(nvp6124_slave_addr[ch/4], ACP_CLR+((ch%2)*0x80), 0x01);
	msleep(10);
	I2CWriteByte(nvp6124_slave_addr[ch/4], ACP_CLR+((ch%2)*0x80), 0x00);
}


void nvp6124_pelco_coax_mode(unsigned char ch)
{
	I2CWriteByte(nvp6124_slave_addr[ch/4], 0xFF, 0x01);
	if(chip_id[0] == NVP6124_R0_ID)
	{
		I2CWriteByte(nvp6124_slave_addr[ch/4], 0xBD, 0xD0);
		I2CWriteByte(nvp6124_slave_addr[ch/4], 0xBE, 0xDD);
		I2CWriteByte(nvp6124_slave_addr[ch/4], 0xBF, 0x0D);
	}
	else if(chip_id[0] == NVP6114A_R0_ID)
	{
		I2CWriteByte(nvp6124_slave_addr[ch/4], 0xBC, 0xDD);
		I2CWriteByte(nvp6124_slave_addr[ch/4], 0xBD, 0xED);
	}

	I2CWriteByte(nvp6124_slave_addr[ch/4], 0xFF, 0x05+ch%4);
	
	if(ch_mode_status[ch] == NVP6124_VI_720P_2530 || ch_mode_status[ch] == NVP6124_VI_720P_5060)
	{
		I2CWriteByte(nvp6124_slave_addr[ch/4], 0x7C, 0x11);
	}
	else if(ch_mode_status[ch] == NVP6124_VI_1080P_2530)
	{
		I2CWriteByte(nvp6124_slave_addr[ch/4], 0x7C, 0x11);
	}
	else if(ch_mode_status[ch] == NVP6124_VI_SD)
	{
		I2CWriteByte(nvp6124_slave_addr[ch/4], 0x7C, 0x11);
	}
	else
		printk("nvp6124_pelco_coax_mode ch[%d] error\n", ch);

	if(ch_mode_status[ch] == NVP6124_VI_SD)
	{
		I2CWriteByte(nvp6124_slave_addr[ch/4], 0xFF, 0x03+((ch%4)/2));
		I2CWriteByte(nvp6124_slave_addr[ch/4], AHD2_PEL_BAUD+((ch%2)*0x80), nvp6124_mode%2? 0x1B:0x1B);
		I2CWriteByte(nvp6124_slave_addr[ch/4], AHD2_PEL_LINE+((ch%2)*0x80), nvp6124_mode%2? 0x0E:0x0E);
		I2CWriteByte(nvp6124_slave_addr[ch/4], PACKET_MODE+((ch%2)*0x80), 0x06);
		I2CWriteByte(nvp6124_slave_addr[ch/4], AHD2_PEL_SYNC+((ch%2)*0x80), nvp6124_mode%2? 0xa0:0xd4);
		I2CWriteByte(nvp6124_slave_addr[ch/4], AHD2_PEL_SYNC+1+((ch%2)*0x80), nvp6124_mode%2?0x05:0x05);
		I2CWriteByte(nvp6124_slave_addr[ch/4], AHD2_PEL_EVEN+((ch%2)*0x80), 0x01);
			
		printk("\r\n960H COAXIAL PROTOCOL IS SETTING....");
	}
	else if(ch_mode_status[ch] == NVP6124_VI_720P_2530 || ch_mode_status[ch] == NVP6124_VI_720P_5060)
	{
		I2CWriteByte(nvp6124_slave_addr[ch/4], 0xFF, 0x03+((ch%4)/2));
		I2CWriteByte(nvp6124_slave_addr[ch/4], AHD2_PEL_BAUD+((ch%2)*0x80), 0x0D);
		I2CWriteByte(nvp6124_slave_addr[ch/4], AHD2_PEL_LINE+((ch%2)*0x80), nvp6124_mode%2? 0x0d:0x0e);
		I2CWriteByte(nvp6124_slave_addr[ch/4], PACKET_MODE+((ch%2)*0x80), 0x06);
		I2CWriteByte(nvp6124_slave_addr[ch/4], AHD2_PEL_SYNC+((ch%2)*0x80), nvp6124_mode%2? 0x40:0x80);
		I2CWriteByte(nvp6124_slave_addr[ch/4], AHD2_PEL_SYNC+1+((ch%2)*0x80), nvp6124_mode%2?0x05:0x00);
		I2CWriteByte(nvp6124_slave_addr[ch/4], AHD2_PEL_EVEN+((ch%2)*0x80), 0x00);
		printk("\r\n720P COAXIAL PROTOCOL IS SETTING....");	
	}
	else if(ch_mode_status[ch] == NVP6124_VI_1080P_2530)
	{
		I2CWriteByte(nvp6124_slave_addr[ch/4], 0xFF, 0x03+((ch%4)/2));
		I2CWriteByte(nvp6124_slave_addr[ch/4], AHD2_FHD_BAUD+((ch%2)*0x80),0x0E);
		I2CWriteByte(nvp6124_slave_addr[ch/4], AHD2_FHD_LINE+((ch%2)*0x80),nvp6124_mode%2? 0x0E:0x0E);
		I2CWriteByte(nvp6124_slave_addr[ch/4], AHD2_PEL_SYNC+((ch%2)*0x80),nvp6124_mode%2? 0x14:0x14);
		I2CWriteByte(nvp6124_slave_addr[ch/4], AHD2_FHD_LINES+((ch%2)*0x80), 0x03);
		I2CWriteByte(nvp6124_slave_addr[ch/4], AHD2_FHD_BYTE+((ch%2)*0x80), 0x03);
		I2CWriteByte(nvp6124_slave_addr[ch/4], AHD2_FHD_MODE+((ch%2)*0x80), 0x10);
		I2CWriteByte(nvp6124_slave_addr[ch/4], ((ch%2)*0x80)+0x0E, 0x00);  //10.25
		I2CWriteByte(nvp6124_slave_addr[ch/4], AHD2_PEL_EVEN+((ch%2)*0x80), 0x00);
		printk("\r\n1080P COAXIAL PROTOCOL IS SETTING....");
	}
	else
	{
		printk("\r\nCOAXIAL MODE NOT RIGHT...\n");
	}
}

unsigned char nvp6124_pelco_command(unsigned char pel_ch, unsigned char command)
{
	int i;
	unsigned char str[4];

	nvp6124_pelco_coax_mode(pel_ch);
	
	msleep( 20 );	

	if(ch_mode_status[pel_ch] == NVP6124_VI_1080P_2530)
	{
		switch(command)
		{
			case  PELCO_CMD_RESET :
				str[0] = 0x00;str[1] = 0x00;str[2] = 0x00;str[3] = 0x00;
			break;
			case  PELCO_CMD_SET:
				str[0] = 0x02;str[1] = 0x00;str[2] = 0x00;str[3] = 0x00;
			break;
			case  PELCO_CMD_UP:			
				str[0] = 0x00;str[1] = 0x08;str[2] = 0x00;str[3] = 0x32;
			break;
			case  PELCO_CMD_DOWN:			
				str[0] = 0x00;str[1] = 0x10;str[2] = 0x00;str[3] = 0x32;
			break;
			case  PELCO_CMD_LEFT:			
				str[0] = 0x00;str[1] = 0x04;str[2] = 0x32;str[3] = 0x00;
			break;
			case  PELCO_CMD_RIGHT:			
				str[0] = 0x00;str[1] = 0x02;str[2] = 0x32;str[3] = 0x00;
			break;
			case  PELCO_CMD_OSD:			
				str[0] = 0x00;str[1] = 0x03;str[2] = 0x00;str[3] = 0x3F;
			break;
			case  PELCO_CMD_IRIS_OPEN:
				str[0] = 0x02;str[1] = 0x00;str[2] = 0x00;str[3] = 0x00;
			break;
			case  PELCO_CMD_IRIS_CLOSE:	
				str[0] = 0x04;str[1] = 0x00;str[2] = 0x00;str[3] = 0x00;
			break;
			case  PELCO_CMD_FOCUS_NEAR:	
				str[0] = 0x01;str[1] = 0x00;str[2] = 0x00;str[3] = 0x00;
			break;
			case  PELCO_CMD_FOCUS_FAR:		
				str[0] = 0x00;str[1] = 0x80;str[2] = 0x00;str[3] = 0x00;
			break;
			case  PELCO_CMD_ZOOM_WIDE:		
				str[0] = 0x00;str[1] = 0x40;str[2] = 0x00;str[3] = 0x00;
			break;
			case  PELCO_CMD_ZOOM_TELE:		
				str[0] = 0x00;str[1] = 0x20;str[2] = 0x00;str[3] = 0x00;
			break;

			default : return 0; //unexpected command
		}		
	}
	else if(ch_mode_status[pel_ch] == NVP6124_VI_720P_2530)
	{
		switch(command)
		{
			case  PELCO_CMD_RESET :
				str[0] = 0x00;str[1] = 0x00;str[2] = 0x00;str[3] = 0x00;
			break;
			case  PELCO_CMD_SET:
				str[0] = 0x40;str[1] = 0x00;str[2] = 0x00;str[3] = 0x00;
			break;
			case  PELCO_CMD_UP:			
				str[0] = 0x00;str[1] = 0x10;str[2] = 0x10;str[3] = 0x4C;
			break;
			case  PELCO_CMD_DOWN:				
				str[0] = 0x00;str[1] = 0x08;str[2] = 0x08;str[3] = 0x4C;
			break;
			case  PELCO_CMD_LEFT:				
				str[0] = 0x00;str[1] = 0x20;str[2] = 0x20;str[3] = 0x00;
			break;
			case  PELCO_CMD_RIGHT:				
				str[0] = 0x00;str[1] = 0x40;str[2] = 0x40;str[3] = 0x00;
			break;
			case  PELCO_CMD_OSD:				
				str[0] = 0x00;str[1] = 0xC0;str[2] = 0xC0;str[3] = 0xFA;
			break;
			case  PELCO_CMD_IRIS_OPEN:
				str[0] = 0x40;str[1] = 0x00;str[2] = 0x00;str[3] = 0x00;
			break;
			case  PELCO_CMD_IRIS_CLOSE:		
				str[0] = 0x20;str[1] = 0x00;str[2] = 0x00;str[3] = 0x00;
			break;
			case  PELCO_CMD_FOCUS_NEAR:		
				str[0] = 0x80;str[1] = 0x00;str[2] = 0x00;str[3] = 0x00;
			break;
			case  PELCO_CMD_FOCUS_FAR:				
				str[0] = 0x00;str[1] = 0x01;str[2] = 0x01;str[3] = 0x00;
			break;
			case  PELCO_CMD_ZOOM_WIDE:				
				str[0] = 0x00;str[1] = 0x02;str[2] = 0x02;str[3] = 0x00;
			break;
			case  PELCO_CMD_ZOOM_TELE:				
				str[0] = 0x00;str[1] = 0x04;str[2] = 0x04;str[3] = 0x00;
			break;
			case  PELCO_CMD_SCAN_SR:			
				str[0] = 0x00;str[1] = 0xE0;str[2] = 0xE0;str[3] = 0x46;
			break;
			case  PELCO_CMD_SCAN_ST:			
				str[0] = 0x00;str[1] = 0xE0;str[2] = 0xE0;str[3] = 0x00;
			break;
			case  PELCO_CMD_PRESET1:			
				str[0] = 0x00;str[1] = 0xE0;str[2] = 0xE0;str[3] = 0x80;
			break;
			case  PELCO_CMD_PRESET2:		
				str[0] = 0x00;str[1] = 0xE0;str[2] = 0xE0;str[3] = 0x40;
			break;
			case  PELCO_CMD_PRESET3:			
				str[0] = 0x00;str[1] = 0xE0;str[2] = 0xE0;str[3] = 0xC0;
			break;
			case  PELCO_CMD_PTN1_SR:			
				str[0] = 0x00;str[1] = 0xF8;str[2] = 0xF8;str[3] = 0x01;
			break;
			case  PELCO_CMD_PTN1_ST:			
				str[0] = 0x00;str[1] = 0x84;str[2] = 0x84;str[3] = 0x01;
			break;
			case  PELCO_CMD_PTN2_SR:			
				str[0] = 0x00;str[1] = 0xF8;str[2] = 0xF8;str[3] = 0x02;
			break;
			case  PELCO_CMD_PTN2_ST:			
				str[0] = 0x00;str[1] = 0x84;str[2] = 0x84;str[3] = 0x02;
			break;
			case  PELCO_CMD_PTN3_SR:			
				str[0] = 0x00;str[1] = 0xF8;str[2] = 0xF8;str[3] = 0x03;
			break;
			case  PELCO_CMD_PTN3_ST:			
				str[0] = 0x00;str[1] = 0x84;str[2] = 0x84;str[3] = 0x03;
			break;
			case  PELCO_CMD_RUN:				
				str[0] = 0x00;str[1] = 0xC4;str[2] = 0xC4;str[3] = 0x00;
			break;

			default : return 0; //unexpected command
		}		
	}
	else
	{
		switch(command)
		{
			case  PELCO_CMD_RESET :
				str[0] = 0x00;str[1] = 0x00;str[2] = 0x00;str[3] = 0x00;
			break;
			case  PELCO_CMD_SET:
				str[0] = 0x40;str[1] = 0x00;str[2] = 0x00;str[3] = 0x00;
			break;
			case  PELCO_CMD_UP:			
				str[0] = 0x00;str[1] = 0x10;str[2] = 0x00;str[3] = 0x4C;
			break;
			case  PELCO_CMD_DOWN:			
				str[0] = 0x00;str[1] = 0x08;str[2] = 0x00;str[3] = 0x4C;
			break;
			case  PELCO_CMD_LEFT:			
				str[0] = 0x00;str[1] = 0x20;str[2] = 0x4C;str[3] = 0x00;
			break;
			case  PELCO_CMD_RIGHT:			
				str[0] = 0x00;str[1] = 0x40;str[2] = 0x4C;str[3] = 0x00;
			break;
			case  PELCO_CMD_OSD:			
				str[0] = 0x00;str[1] = 0xC0;str[2] = 0x00;str[3] = 0xFA;
			break;
			case  PELCO_CMD_IRIS_OPEN:
				str[0] = 0x40;str[1] = 0x00;str[2] = 0x00;str[3] = 0x00;
			break;
			case  PELCO_CMD_IRIS_CLOSE:	
				str[0] = 0x20;str[1] = 0x00;str[2] = 0x00;str[3] = 0x00;
			break;
			case  PELCO_CMD_FOCUS_NEAR:	
				str[0] = 0x80;str[1] = 0x00;str[2] = 0x00;str[3] = 0x00;
			break;
			case  PELCO_CMD_FOCUS_FAR:		
				str[0] = 0x00;str[1] = 0x01;str[2] = 0x00;str[3] = 0x00;
			break;
			case  PELCO_CMD_ZOOM_WIDE:		
				str[0] = 0x00;str[1] = 0x02;str[2] = 0x00;str[3] = 0x00;
			break;
			case  PELCO_CMD_ZOOM_TELE:		
				str[0] = 0x00;str[1] = 0x04;str[2] = 0x00;str[3] = 0x00;
			break;
			case  PELCO_CMD_SCAN_SR:		
				str[0] = 0x00;str[1] = 0xE0;str[2] = 0x00;str[3] = 0x46;
			break;
			case  PELCO_CMD_SCAN_ST:		
				str[0] = 0x00;str[1] = 0xE0;str[2] = 0x00;str[3] = 0x00;
			break;
			case  PELCO_CMD_PRESET1:		
				str[0] = 0x00;str[1] = 0xE0;str[2] = 0x00;str[3] = 0x80;
			break;
			case  PELCO_CMD_PRESET2:		
				str[0] = 0x00;str[1] = 0xE0;str[2] = 0x00;str[3] = 0x40;
			break;
			case  PELCO_CMD_PRESET3:		
				str[0] = 0x00;str[1] = 0xE0;str[2] = 0x00;str[3] = 0xC0;
			break;
			case  PELCO_CMD_PTN1_SR:		
				str[0] = 0x00;str[1] = 0xF8;str[2] = 0x00;str[3] = 0x01;
			break;
			case  PELCO_CMD_PTN1_ST:		
				str[0] = 0x00;str[1] = 0x84;str[2] = 0x00;str[3] = 0x01;
			break;
			case  PELCO_CMD_PTN2_SR:		
				str[0] = 0x00;str[1] = 0xF8;str[2] = 0x00;str[3] = 0x02;
			break;
			case  PELCO_CMD_PTN2_ST:		
				str[0] = 0x00;str[1] = 0x84;str[2] = 0x00;str[3] = 0x02;
			break;
			case  PELCO_CMD_PTN3_SR:		
				str[0] = 0x00;str[1] = 0xF8;str[2] = 0x00;str[3] = 0x03;
			break;
			case  PELCO_CMD_PTN3_ST:		
				str[0] = 0x00;str[1] = 0x84;str[2] = 0x00;str[3] = 0x03;
			break;
			case  PELCO_CMD_RUN:			
				str[0] = 0x00;str[1] = 0xC4;str[2] = 0x00;str[3] = 0x00;
			break;

			default : return 0; 
		}
	}


	I2CWriteByte(nvp6124_slave_addr[pel_ch/4], 0xFF, 0x03+((pel_ch/2)%2));
		
	if(ch_mode_status[pel_ch] == NVP6124_VI_SD)
	{
		for(i=0;i<4;i++)
		{
			I2CWriteByte(nvp6124_slave_addr[pel_ch/4], AHD2_PEL_D0+i+(((pel_ch)%2)*0x80), 0x00);
		}

		nvp6124_pelco_shout(pel_ch);

		
		for(i=0;i<4;i++)
		{
			I2CWriteByte(nvp6124_slave_addr[pel_ch/4], AHD2_PEL_D0+i+(((pel_ch)%2)*0x80), str[i]);
		}		
		nvp6124_pelco_shout(pel_ch);
		printk("\r\nPelco protocl shout!");				
	}

	else if(ch_mode_status[pel_ch] == NVP6124_VI_720P_2530|| ch_mode_status[pel_ch] == NVP6124_VI_720P_5060)
	{
		for(i=0;i<4;i++)
		{
			I2CWriteByte(nvp6124_slave_addr[pel_ch/4], AHD2_PEL_D0+i+(((pel_ch)%2)*0x80), 0x00);
		}

		nvp6124_pelco_shout(pel_ch);

		
		for(i=0;i<4;i++)
		{
			I2CWriteByte(nvp6124_slave_addr[pel_ch/4], AHD2_PEL_D0+i+(((pel_ch)%2)*0x80), str[i]);
		}		
		nvp6124_pelco_shout(pel_ch);
		printk("\r\n720p Coaxial protocl shout!");				
	}
	
	else if(ch_mode_status[pel_ch] == NVP6124_VI_1080P_2530)
	{
		for(i=0;i<4;i++)
		{
			I2CWriteByte(nvp6124_slave_addr[pel_ch/4], AHD2_FHD_D0+i+(((pel_ch)%2)*0x80), 0x00);			
		}

		nvp6124_pelco_shout(pel_ch);
		
		for(i=0;i<4;i++)
		{
			I2CWriteByte(nvp6124_slave_addr[pel_ch/4], AHD2_FHD_D0+i+(((pel_ch)%2)*0x80), str[i]);
		}
		
		nvp6124_pelco_shout(pel_ch);
		printk("\r\n1080p Coaxial protocl shout!");		
	}
	return 1;
}

void nvp6124_pelco_shout(unsigned char pel_ch)
{
	I2CWriteByte(nvp6124_slave_addr[pel_ch/4], 0xFF, 0x03+((pel_ch/2)%2));
	if(ch_mode_status[pel_ch] == NVP6124_VI_SD)
	{
		I2CWriteByte(nvp6124_slave_addr[pel_ch/4], AHD2_PEL_OUT+(((pel_ch)%2)*0x80), 0x01);
		msleep( 30 );
		I2CWriteByte(nvp6124_slave_addr[pel_ch/4], AHD2_PEL_OUT+(((pel_ch)%2)*0x80), 0x00);
		msleep( 60 );
	}	
	else if(ch_mode_status[pel_ch] == NVP6124_VI_720P_2530|| ch_mode_status[pel_ch] == NVP6124_VI_720P_5060)
	{
		I2CWriteByte(nvp6124_slave_addr[pel_ch/4], AHD2_PEL_OUT+(((pel_ch)%2)*0x80), 0x01);
		msleep( 30 );
		I2CWriteByte(nvp6124_slave_addr[pel_ch/4], AHD2_PEL_OUT+(((pel_ch)%2)*0x80), 0x00);
		msleep( 60 );
	}
	else 
	{
		I2CWriteByte(nvp6124_slave_addr[pel_ch/4], AHD2_FHD_OUT+(((pel_ch)%2)*0x80), 0x08);
		msleep( 25 );
		I2CWriteByte(nvp6124_slave_addr[pel_ch/4], AHD2_FHD_OUT+(((pel_ch)%2)*0x80), 0x00);
		msleep( 25 );
	}
}

