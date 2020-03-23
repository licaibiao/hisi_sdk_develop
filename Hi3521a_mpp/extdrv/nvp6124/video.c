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
#include "nvp6124_reg.h"
#include "coax_protocol.h"

/*nvp6124 1080P 色彩推荐配置*/
#define BRI_CENTER_VAL_NTSC 0xF4
#define BRI_CENTER_VAL_PAL  0xF4
#define CON_CENTER_VAL_NTSC 0x90
#define CON_CENTER_VAL_PAL  0x90
#define SAT_CENTER_VAL_NTSC 0x80
#define SAT_CENTER_VAL_PAL  0x80                       
#define HUE_CENTER_VAL_NTSC 0x00
#define HUE_CENTER_VAL_PAL  0x00


/*nvp6124 720P 色彩推荐配置*/
#define BRI_CENTER_VAL_NTSC_720P 0x08
#define BRI_CENTER_VAL_PAL_720P  0x08
#define CON_CENTER_VAL_NTSC_720P 0x88
#define CON_CENTER_VAL_PAL_720P  0x88
#define SAT_CENTER_VAL_NTSC_720P 0x90
#define SAT_CENTER_VAL_PAL_720P  0x90                       
#define HUE_CENTER_VAL_NTSC_720P 0xFD
#define HUE_CENTER_VAL_PAL_720P  0x00
                           
/*nvp6124 960H 色彩推荐配置*/
#define BRI_CENTER_VAL_NTSC_960H 0xFF
#define BRI_CENTER_VAL_PAL_960H  0x00
#define CON_CENTER_VAL_NTSC_960H 0x96  //960H contrast 0x92->0x96
#define CON_CENTER_VAL_PAL_960H  0x90
#define SAT_CENTER_VAL_NTSC_960H 0x88
#define SAT_CENTER_VAL_PAL_960H  0x88                       
#define HUE_CENTER_VAL_NTSC_960H 0x01
#define HUE_CENTER_VAL_PAL_960H  0x00

unsigned int nvp6124_con_tbl[2]  = {CON_CENTER_VAL_NTSC, CON_CENTER_VAL_PAL};
unsigned int nvp6124_hue_tbl[2]  = {HUE_CENTER_VAL_NTSC, HUE_CENTER_VAL_PAL};
unsigned int nvp6124_sat_tbl[2]  = {SAT_CENTER_VAL_NTSC, SAT_CENTER_VAL_PAL};
unsigned int nvp6124_bri_tbl[2]  = {BRI_CENTER_VAL_NTSC, BRI_CENTER_VAL_PAL};

unsigned int nvp6124_con_tbl_720P[2]  = {CON_CENTER_VAL_NTSC_720P, CON_CENTER_VAL_PAL_720P};
unsigned int nvp6124_hue_tbl_720P[2]  = {HUE_CENTER_VAL_NTSC_720P, HUE_CENTER_VAL_PAL_720P};
unsigned int nvp6124_sat_tbl_720P[2]  = {SAT_CENTER_VAL_NTSC_720P, SAT_CENTER_VAL_PAL_720P};
unsigned int nvp6124_bri_tbl_720P[2]  = {BRI_CENTER_VAL_NTSC_720P, BRI_CENTER_VAL_PAL_720P};


unsigned int nvp6124_con_tbl_960H[2]  = {CON_CENTER_VAL_NTSC_960H, CON_CENTER_VAL_PAL_960H};
unsigned int nvp6124_hue_tbl_960H[2]  = {HUE_CENTER_VAL_NTSC_960H, HUE_CENTER_VAL_PAL_960H};
unsigned int nvp6124_sat_tbl_960H[2]  = {SAT_CENTER_VAL_NTSC_960H, SAT_CENTER_VAL_PAL_960H};
unsigned int nvp6124_bri_tbl_960H[2]  = {BRI_CENTER_VAL_NTSC_960H, BRI_CENTER_VAL_PAL_960H};

unsigned char nvp6124_motion_sens_tbl[8]= {0xe0,0xc8,0xa0,0x98,0x78,0x68,0x50,0x48};
unsigned char ch_mode_status[16]={0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};
unsigned char ch_mode_status_tmp[16]={0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};

extern unsigned int nvp6124_cnt;
extern int chip_id[4];
extern unsigned int nvp6124_mode;
extern int g_soc_chiptype;
extern void audio_init(unsigned char dec, unsigned char ch_num, unsigned char samplerate, unsigned char bits);


void nvp6124_ntsc_common_init(void);
void nvp6124_pal_common_init(void);
void nvp6124_write_table(unsigned char chip_addr, unsigned char addr, unsigned char *tbl_ptr, unsigned char tbl_cnt);


extern unsigned int nvp6124_mode;
extern unsigned int nvp6124_slave_addr[4];
void NVP6124_AfeReset(unsigned char ch)
{
	int i = 0;       
	if(ch == 0xFF)
	{
		for(i=0;i<nvp6124_cnt;i++)
		{
			I2CWriteByte(nvp6124_slave_addr[i], 0xFF, 0x00);
			I2CWriteByte(nvp6124_slave_addr[i], 0x02, I2CReadByte(nvp6124_slave_addr[i], 0x02)|0x0F);
			I2CWriteByte(nvp6124_slave_addr[i], 0x02, I2CReadByte(nvp6124_slave_addr[i], 0x02)&0xF0);
		}
		printk("NVP6124_AfeReset ALL Channel done\n");
	}
	else
	{
		I2CWriteByte(nvp6124_slave_addr[ch/4], 0xFF, 0x00);
		I2CWriteByte(nvp6124_slave_addr[ch/4], 0x02, I2CReadByte(nvp6124_slave_addr[ch/4], 0x02)|(0x01<<(ch%4)));
		I2CWriteByte(nvp6124_slave_addr[ch/4], 0x02, I2CReadByte(nvp6124_slave_addr[ch/4], 0x02)&0xF0);
		printk("NVP6124_AfeReset ch[%d] done\n", ch);
	}	
	
}
EXPORT_SYMBOL(NVP6124_AfeReset);
void nvp6124_datareverse(void)
{
/*
BANK1 0xD2[5:2],每个bit控制一个bt656的数据顺序，1为反序，0为正序。
*/
	int i = 0;                               
	for(i=0;i<nvp6124_cnt;i++)
	{
		I2CWriteByte(nvp6124_slave_addr[i], 0xFF, 0x01);
		I2CWriteByte(nvp6124_slave_addr[i], 0xD2, 0x3C); 
	}
	printk("nvp6124 data reversed\n");
}

unsigned char chipon297MHz[4]={0};
void nvp6124_set_clockmode(unsigned char chip, unsigned char is297MHz)
{
    //int i = 0;                               
	//for(i=0;i<nvp6124_cnt;i++)
		I2CWriteByte(nvp6124_slave_addr[chip], 0xFF, 0x01);
		I2CWriteByte(nvp6124_slave_addr[chip], 0x80, 0x40);
		if(is297MHz == 1)
		{
			I2CWriteByte(nvp6124_slave_addr[chip], 0x82, 0x12); 
			chipon297MHz[chip] = 1;
		}
		else
		{
			I2CWriteByte(nvp6124_slave_addr[chip], 0x82, 0x14); 
			chipon297MHz[chip] = 0;
		}
		I2CWriteByte(nvp6124_slave_addr[chip], 0x83, 0x2C); 
		I2CWriteByte(nvp6124_slave_addr[chip], 0x80, 0x61); 
		I2CWriteByte(nvp6124_slave_addr[chip], 0x80, 0x60); 
		 
		printk("chip %d nvp6124_set_clockmode %d\n", chip, is297MHz);
		
}

void nvp6124_system_init(void)
{
    int i = 0;                               
	for(i=0;i<nvp6124_cnt;i++)
	{
		I2CWriteByte(nvp6124_slave_addr[i], 0xFF, 0x01); 
		I2CWriteByte(nvp6124_slave_addr[i], 0x82, 0x14); 
		I2CWriteByte(nvp6124_slave_addr[i], 0x83, 0x2C); 
		I2CWriteByte(nvp6124_slave_addr[i], 0x3e, 0x10); 
		I2CWriteByte(nvp6124_slave_addr[i], 0x80, 0x60); 
		I2CWriteByte(nvp6124_slave_addr[i], 0x80, 0x61); 
		msleep(100); 
		I2CWriteByte(nvp6124_slave_addr[i], 0x80, 0x40); 
		I2CWriteByte(nvp6124_slave_addr[i], 0x81, 0x02); 
		I2CWriteByte(nvp6124_slave_addr[i], 0x97, 0x00); 
		msleep(10); 
		I2CWriteByte(nvp6124_slave_addr[i], 0x80, 0x60); 
		I2CWriteByte(nvp6124_slave_addr[i], 0x81, 0x00); 
		msleep(10); 
		I2CWriteByte(nvp6124_slave_addr[i], 0x97, 0x0F); 
		I2CWriteByte(nvp6124_slave_addr[i], 0x38, 0x18); 
		I2CWriteByte(nvp6124_slave_addr[i], 0x38, 0x08); 
		msleep(10); 
		printk("~~~~~~~~~~~~~~~~20150227-2 nvp6124_system_init~~~~~~~~~~~~~~~~\n");
		msleep(100);
		I2CWriteByte(nvp6124_slave_addr[i], 0xCA, 0xFF);
		chipon297MHz[i] = 0;
	}
	if(chip_id[0] == NVP6124_R0_ID)
		nvp6124_outport_1mux_chseq();
	else if(chip_id[0] == NVP6114A_R0_ID)
		nvp6114a_outport_1mux_chseq();
}

void software_reset(void)
{
	  int i = 0;                               
	for(i=0;i<nvp6124_cnt;i++)
	{
	    I2CWriteByte(nvp6124_slave_addr[i], 0xFF, 0x01); 
	    I2CWriteByte(nvp6124_slave_addr[i], 0x80, 0x40); 
	    I2CWriteByte(nvp6124_slave_addr[i], 0x81, 0x02);
	    I2CWriteByte(nvp6124_slave_addr[i], 0x80, 0x61);
	    I2CWriteByte(nvp6124_slave_addr[i], 0x81, 0x00);
	    msleep(100); 
	    I2CWriteByte(nvp6124_slave_addr[i], 0x80, 0x60); 
	    I2CWriteByte(nvp6124_slave_addr[i], 0x81, 0x00);
	    msleep(100); 
	}
	printk("\n\r nvp6124 software reset!!!");
}           

void nvp6124_ntsc_common_init(void)
{
	int i = 0;

	for(i=0;i<nvp6124_cnt;i++)
	{
		I2CWriteByte(nvp6124_slave_addr[i], 0xFF, 0x00);
        nvp6124_write_table(nvp6124_slave_addr[i],0x00,NVP6124_B0_30P_Buf,254);
		I2CWriteByte(nvp6124_slave_addr[i], 0xFF, 0x01);
        nvp6124_write_table(nvp6124_slave_addr[i],0x00,NVP6124_B1_30P_Buf,254);
		I2CWriteByte(nvp6124_slave_addr[i], 0xFF, 0x02);
        nvp6124_write_table(nvp6124_slave_addr[i],0x00,NVP6124_B2_30P_Buf,254);
		I2CWriteByte(nvp6124_slave_addr[i], 0xFF, 0x03);
        nvp6124_write_table(nvp6124_slave_addr[i],0x00,NVP6124_B3_30P_Buf,254);
		I2CWriteByte(nvp6124_slave_addr[i], 0xFF, 0x04);
        nvp6124_write_table(nvp6124_slave_addr[i],0x00,NVP6124_B4_30P_Buf,254);
		//I2CWriteByte(nvp6124_slave_addr[i], 0xFF, 0x05);
        //nvp6124_write_table(nvp6124_slave_addr[i],0x00,NVP6124_B5_30P_Buf,254);
		//I2CWriteByte(nvp6124_slave_addr[i], 0xFF, 0x06);
        //nvp6124_write_table(nvp6124_slave_addr[i],0x00,NVP6124_B6_30P_Buf,254);
		//I2CWriteByte(nvp6124_slave_addr[i], 0xFF, 0x07);
        //nvp6124_write_table(nvp6124_slave_addr[i],0x00,NVP6124_B7_30P_Buf,254);
		//I2CWriteByte(nvp6124_slave_addr[i], 0xFF, 0x08);
        //nvp6124_write_table(nvp6124_slave_addr[i],0x00,NVP6124_B8_30P_Buf,254);
		I2CWriteByte(nvp6124_slave_addr[i], 0xFF, 0x09);
        nvp6124_write_table(nvp6124_slave_addr[i],0x00,NVP6124_B9_30P_Buf,254);
		I2CWriteByte(nvp6124_slave_addr[i], 0xFF, 0x0A);
        nvp6124_write_table(nvp6124_slave_addr[i],0x00,NVP6124_BA_30P_Buf,254);
		I2CWriteByte(nvp6124_slave_addr[i], 0xFF, 0x0B);
        nvp6124_write_table(nvp6124_slave_addr[i],0x00,NVP6124_BB_30P_Buf,254);
	}

	nvp6124_system_init();
}

void nvp6124_pal_common_init(void)
{
	int i = 0;

	for(i=0;i<nvp6124_cnt;i++)
	{
		I2CWriteByte(nvp6124_slave_addr[i], 0xFF, 0x00);
        nvp6124_write_table(nvp6124_slave_addr[i],0x00,NVP6124_B0_25P_Buf,254);
		I2CWriteByte(nvp6124_slave_addr[i], 0xFF, 0x01);
        nvp6124_write_table(nvp6124_slave_addr[i],0x00,NVP6124_B1_25P_Buf,254);
		I2CWriteByte(nvp6124_slave_addr[i], 0xFF, 0x02);
        nvp6124_write_table(nvp6124_slave_addr[i],0x00,NVP6124_B2_25P_Buf,254);
		I2CWriteByte(nvp6124_slave_addr[i], 0xFF, 0x03);
        nvp6124_write_table(nvp6124_slave_addr[i],0x00,NVP6124_B3_25P_Buf,254);
		I2CWriteByte(nvp6124_slave_addr[i], 0xFF, 0x04);
        nvp6124_write_table(nvp6124_slave_addr[i],0x00,NVP6124_B4_25P_Buf,254);
		//I2CWriteByte(nvp6124_slave_addr[i], 0xFF, 0x05);
        //nvp6124_write_table(nvp6124_slave_addr[i],0x00,NVP6124_B5_25P_Buf,254);
		//I2CWriteByte(nvp6124_slave_addr[i], 0xFF, 0x06);
        //nvp6124_write_table(nvp6124_slave_addr[i],0x00,NVP6124_B6_25P_Buf,254);
		//I2CWriteByte(nvp6124_slave_addr[i], 0xFF, 0x07);
        //nvp6124_write_table(nvp6124_slave_addr[i],0x00,NVP6124_B7_25P_Buf,254);
		//I2CWriteByte(nvp6124_slave_addr[i], 0xFF, 0x08);
        //nvp6124_write_table(nvp6124_slave_addr[i],0x00,NVP6124_B8_25P_Buf,254);
		I2CWriteByte(nvp6124_slave_addr[i], 0xFF, 0x09);
        nvp6124_write_table(nvp6124_slave_addr[i],0x00,NVP6124_B9_25P_Buf,254);
		I2CWriteByte(nvp6124_slave_addr[i], 0xFF, 0x0A);
        nvp6124_write_table(nvp6124_slave_addr[i],0x00,NVP6124_BA_25P_Buf,254);
		I2CWriteByte(nvp6124_slave_addr[i], 0xFF, 0x0B);
        nvp6124_write_table(nvp6124_slave_addr[i],0x00,NVP6124_BB_25P_Buf,254);
	}

	nvp6124_system_init();
}
void mpp2clk(unsigned char clktype)
{
	int i=0;
	for(i=0;i<nvp6124_cnt;i++)
	{
		I2CWriteByte(nvp6124_slave_addr[i], 0xFF, 0x01);
		I2CWriteByte(nvp6124_slave_addr[i], 0xD4, 0x0F);  //mpp1,2,3,4 port clock func enables
		I2CWriteByte(nvp6124_slave_addr[i], 0xB4, 0x66);  //clock&delay setting
		I2CWriteByte(nvp6124_slave_addr[i], 0xB5, 0x66);
		I2CWriteByte(nvp6124_slave_addr[i], 0xB6, 0x66);  
		I2CWriteByte(nvp6124_slave_addr[i], 0xB7, 0x66);
	}
}
/*视频输入制式检测函数*/
void video_fmt_det(nvp6124_input_videofmt *pvideofmt)
{
	int i;
	unsigned char tmp;
	for(i=0; i<nvp6124_cnt*4; i++)
	{
		I2CWriteByte(nvp6124_slave_addr[i/4], 0xFF, 0x00);
		pvideofmt->getvideofmt[i] = I2CReadByte(nvp6124_slave_addr[i/4], 0xD0+i%4);
		if(pvideofmt->getvideofmt[i] == 0x40 || pvideofmt->getvideofmt[i] == 0x10)
		{
			I2CWriteByte(nvp6124_slave_addr[i/4], 0xFF, 0x00);
			I2CWriteByte(nvp6124_slave_addr[i/4], 0x23+4*(i%4), 0x41);
			I2CWriteByte(nvp6124_slave_addr[i/4], 0xFF, 0x05+i%4);
			I2CWriteByte(nvp6124_slave_addr[i/4], 0x47, 0xee);
		}
		else
		{
			I2CWriteByte(nvp6124_slave_addr[i/4], 0xFF, 0x00);
			I2CWriteByte(nvp6124_slave_addr[i/4], 0x23+4*(i%4), 0x43);
			I2CWriteByte(nvp6124_slave_addr[i/4], 0xFF, 0x05+i%4);
			I2CWriteByte(nvp6124_slave_addr[i/4], 0x47, 0x04);
		}
		
		if(0x01 == pvideofmt->getvideofmt[i] || 0x02 == pvideofmt->getvideofmt[i])
		{
			I2CWriteByte(nvp6124_slave_addr[i/4], 0xFF, 0x00);
			tmp = I2CReadByte(nvp6124_slave_addr[i/4], 0xE8+i%4);
			if(0x6A == tmp || 0x6E == tmp)
			{
				I2CWriteByte(nvp6124_slave_addr[i/4], 0x21+4*(i%4), nvp6124_mode%2==PAL?0x22:0xA2);
				printk("CH[%d]nextchip comet input\n\n", i);
			}
			else
			{
				I2CWriteByte(nvp6124_slave_addr[i/4], 0x21+4*(i%4), nvp6124_mode%2==PAL?0x02:0x82);
			}
		}		
	}
}

unsigned int nvp6124_getvideoloss(void)
{
    unsigned int vloss=0, i;
    //unsigned int  ch;
	//static unsigned int vlossbak=0xFFFF;
	
	for(i=0;i<nvp6124_cnt;i++)
	{
		I2CWriteByte(nvp6124_slave_addr[i], 0xFF, 0x00);
		vloss|=(I2CReadByte(nvp6124_slave_addr[i], 0xB8)&0x0F)<<(4*i);
	}
	#if 0
	if(vlossbak != vloss)
	{
		for(ch=0;ch<nvp6124_cnt*4;ch++)
		{
			if(((vloss>>ch)&0x01)==0x00 && ((vlossbak>>ch)&0x01)==0x01)
				NVP6124_AfeReset(ch);
		}
		vlossbak = vloss;
	}
	#endif
	return vloss;
}

/*
nvp6124 has 4 BT656 output ports.
nvp6114a only has 2, so ch_seq[2]&ch_seq[3] are invalid in nvp6114a.
*/
unsigned char ch_seq[4]={2,1,0,3};
void nvp6124_outport_1mux_chseq(void)
{
	int i = 0;   
	for(i=0;i<nvp6124_cnt;i++)
	{
		I2CWriteByte(nvp6124_slave_addr[i], 0xFF, 0x01);
		I2CWriteByte(nvp6124_slave_addr[i], 0xC0, ch_seq[0]<<4|ch_seq[0]);
		I2CWriteByte(nvp6124_slave_addr[i], 0xC1, ch_seq[0]<<4|ch_seq[0]);
		I2CWriteByte(nvp6124_slave_addr[i], 0xC2, ch_seq[1]<<4|ch_seq[1]);
		I2CWriteByte(nvp6124_slave_addr[i], 0xC3, ch_seq[1]<<4|ch_seq[1]);
		I2CWriteByte(nvp6124_slave_addr[i], 0xC4, ch_seq[2]<<4|ch_seq[2]);
		I2CWriteByte(nvp6124_slave_addr[i], 0xC5, ch_seq[2]<<4|ch_seq[2]);
		I2CWriteByte(nvp6124_slave_addr[i], 0xC6, ch_seq[3]<<4|ch_seq[3]);
		I2CWriteByte(nvp6124_slave_addr[i], 0xC7, ch_seq[3]<<4|ch_seq[3]);
		I2CWriteByte(nvp6124_slave_addr[i], 0xC8, 0x00);
		I2CWriteByte(nvp6124_slave_addr[i], 0xC9, 0x00);
		I2CWriteByte(nvp6124_slave_addr[i], 0xCA, 0xFF);
	}
	printk("nvp6124_outport_1mux_chseq\n");
}

/*
vformat:0->NTSC, 1->PAL
portx_mode: 
高4bit选择port接口[7:4]->0~3:port0~3; 
低4bit选择vi模式[3:0]-> (NVP6124_VI_SD,NVP6124_VI_720P_2530, NVP6124_VI_1080P_2530)
*/
void nvp6124_outport_2mux(unsigned char vformat, unsigned char port1_mode, unsigned char port2_mode )
{
	int ch, i, tmp=0;
	unsigned char p1_num,p2_num,port1_vimode,port2_vimode;
	nvp6124_video_mode vmode;

	p1_num = port1_mode>>0x04;
	p2_num = port2_mode>>0x04;
	port1_vimode = port1_mode&0x0F;
	port2_vimode = port2_mode&0x0F;
	for(ch=0;ch<nvp6124_cnt*4;ch++)
  	{
  		vmode.vformat[0] = vformat;
		if(ch%4 < 2)
			vmode.chmode[ch] = port1_vimode;
		else
			vmode.chmode[ch] = port2_vimode;
	}
	nvp6124_each_mode_setting(&vmode);
	for(i=0;i<nvp6124_cnt;i++)
	{
		I2CWriteByte(nvp6124_slave_addr[i], 0xFF, 0x01);
		I2CWriteByte(nvp6124_slave_addr[i], 0xC0+p1_num*2, 0x10);    
		I2CWriteByte(nvp6124_slave_addr[i], 0xC1+p1_num*2, 0x10);
		I2CWriteByte(nvp6124_slave_addr[i], 0xC0+p2_num*2, 0x32);    
		I2CWriteByte(nvp6124_slave_addr[i], 0xC1+p2_num*2, 0x32);
		tmp = (port2_vimode==NVP6124_VI_1080P_2530?1:2)<<(p2_num*4)|((port1_vimode==NVP6124_VI_1080P_2530?1:2)<<(p1_num*4));
		I2CWriteByte(nvp6124_slave_addr[i], 0xC8, tmp&0xFF);         
		I2CWriteByte(nvp6124_slave_addr[i], 0xC9, (tmp>>8)&0xFF);    

		
		if(port1_vimode == NVP6124_VI_SD)
			I2CWriteByte(nvp6124_slave_addr[i], 0xCC+p1_num, 0x46);    //时钟频率设置
		else
			I2CWriteByte(nvp6124_slave_addr[i], 0xCC+p1_num, 0x66);
		if(port2_vimode == NVP6124_VI_SD)
			I2CWriteByte(nvp6124_slave_addr[i], 0xCC+p2_num, 0x46);    //时钟频率设置
		else
			I2CWriteByte(nvp6124_slave_addr[i], 0xCC+p2_num, 0x66);
  	}
	printk("nvp6124_outport_2mux setting\n");
}

/*
vformat:0->NTSC, 1->PAL
port1_mode: 
高4bit选择port接口[7:4]->0~3:port0~3; 
低4bit选择vi模式[3:0]-> (NVP6124_VI_SD,NVP6124_VI_720P_2530)
*/
void nvp6124_outport_4mux(unsigned char vformat, unsigned char port1_mode )
{
	int ch, i, tmp=0;
	unsigned char p1_num,port1_vimode;
	nvp6124_video_mode vmode;

	p1_num = port1_mode>>0x04;
	port1_vimode = port1_mode&0x0F;
	for(ch=0;ch<nvp6124_cnt*4;ch++)
  	{
  		vmode.vformat[0] = vformat;
		vmode.chmode[ch] = port1_vimode;
	}
	nvp6124_each_mode_setting(&vmode);
	for(i=0;i<nvp6124_cnt;i++)
	{
		I2CWriteByte(nvp6124_slave_addr[i], 0xFF, 0x00);
		I2CWriteByte(nvp6124_slave_addr[i], 0x55, 0x10);
		I2CWriteByte(nvp6124_slave_addr[i], 0x56, 0x32);  //reset channel id
		I2CWriteByte(nvp6124_slave_addr[i], 0xFF, 0x01);
		I2CWriteByte(nvp6124_slave_addr[i], 0xC0+p1_num*2, 0x10);    
		I2CWriteByte(nvp6124_slave_addr[i], 0xC1+p1_num*2, 0x32);
		tmp = ((NVP6124_VI_720P_2530?3:8)<<(p1_num*4));
		I2CWriteByte(nvp6124_slave_addr[i], 0xC8, tmp&0xFF);         
		I2CWriteByte(nvp6124_slave_addr[i], 0xC9, (tmp>>8)&0xFF);    

		
		I2CWriteByte(nvp6124_slave_addr[i], 0xCC+p1_num, 0x66);    //时钟频率设置
  	}
	printk("nvp6124_outport_4mux setting\n");
}

void nvp6114a_outport_1mux_chseq(void)
{
	int i = 0;    
	ch_seq[0] = 1;   // port A outputs channel0's video data
	ch_seq[1] = 0;   // port B outputs channel1's video data
	ch_seq[2] = 0xFF;
	ch_seq[3] = 0xFF;
	for(i=0;i<nvp6124_cnt;i++)
	{
		I2CWriteByte(nvp6124_slave_addr[i], 0xFF, 0x01);
		//I2CWriteByte(nvp6124_slave_addr[i], 0xC0, ch_seq[0]<<4|ch_seq[0]);
		//I2CWriteByte(nvp6124_slave_addr[i], 0xC1, ch_seq[0]<<4|ch_seq[0]);
		I2CWriteByte(nvp6124_slave_addr[i], 0xC2, ch_seq[0]<<4|ch_seq[0]);
		I2CWriteByte(nvp6124_slave_addr[i], 0xC3, ch_seq[0]<<4|ch_seq[0]);
		I2CWriteByte(nvp6124_slave_addr[i], 0xC4, ch_seq[1]<<4|ch_seq[1]);
		I2CWriteByte(nvp6124_slave_addr[i], 0xC5, ch_seq[1]<<4|ch_seq[1]);
		I2CWriteByte(nvp6124_slave_addr[i], 0xC6, ch_seq[1]<<4|ch_seq[1]);
		I2CWriteByte(nvp6124_slave_addr[i], 0xC7, ch_seq[1]<<4|ch_seq[1]);
		I2CWriteByte(nvp6124_slave_addr[i], 0xC8, 0x00);
		I2CWriteByte(nvp6124_slave_addr[i], 0xC9, 0x00);
		I2CWriteByte(nvp6124_slave_addr[i], 0xCA, 0xFF);
	}
	printk("nvp6114a_outport_1mux_chseq\n");
}

/*
vformat:0->NTSC, 1->PAL
portx_mode: 
高4bit选择port接口[7:4]->0~1:port0~1; 
低4bit选择vi模式[3:0]-> (NVP6124_VI_SD,NVP6124_VI_720P_2530, NVP6124_VI_1080P_2530)
*/
void nvp6114a_outport_2mux(unsigned char vformat, unsigned char port1_mode, unsigned char port2_mode )
{
	int ch, i;
	unsigned char tmp=0;
	unsigned char p1_num,p2_num,port1_vimode,port2_vimode;
	nvp6124_video_mode vmode;

	p1_num = port1_mode>>0x04;
	p2_num = port2_mode>>0x04;
	port1_vimode = port1_mode&0x0F;
	port2_vimode = port2_mode&0x0F;
	for(ch=0;ch<nvp6124_cnt*4;ch++)
  	{
  		vmode.vformat[0] = vformat;
		if(ch%4 < 2)
			vmode.chmode[ch] = port1_vimode;
		else
			vmode.chmode[ch] = port2_vimode;
	}
	nvp6124_each_mode_setting(&vmode);
	for(i=0;i<nvp6124_cnt;i++)
	{
		I2CWriteByte(nvp6124_slave_addr[i], 0xFF, 0x01);
		I2CWriteByte(nvp6124_slave_addr[i], 0xC2+p1_num*2, 0x10);    
		I2CWriteByte(nvp6124_slave_addr[i], 0xC3+p1_num*2, 0x10);
		I2CWriteByte(nvp6124_slave_addr[i], 0xC2+p2_num*2, 0x32);    
		I2CWriteByte(nvp6124_slave_addr[i], 0xC3+p2_num*2, 0x32);
		I2CWriteByte(nvp6124_slave_addr[i], 0xC6, I2CReadByte(nvp6124_slave_addr[i], 0xC4));
		I2CWriteByte(nvp6124_slave_addr[i], 0xC7, I2CReadByte(nvp6124_slave_addr[i], 0xC5));
		tmp = ((port1_vimode==NVP6124_VI_1080P_2530?1:2)<<4);
		I2CWriteByte(nvp6124_slave_addr[i], 0xC8, tmp);         
		tmp = ((port2_vimode==NVP6124_VI_1080P_2530?1:2)<<4)|((port2_vimode==NVP6124_VI_1080P_2530?1:2));
		I2CWriteByte(nvp6124_slave_addr[i], 0xC9, tmp);    

		
		if(port1_vimode == NVP6124_VI_SD)
			I2CWriteByte(nvp6124_slave_addr[i], 0xCD+p1_num*2, 0x46);    //时钟频率设置
		else
			I2CWriteByte(nvp6124_slave_addr[i], 0xCD+p1_num*2, 0x66);
		if(port2_vimode == NVP6124_VI_SD)
			I2CWriteByte(nvp6124_slave_addr[i], 0xCD+p2_num*2, 0x46);    //时钟频率设置
		else
			I2CWriteByte(nvp6124_slave_addr[i], 0xCD+p2_num*2, 0x66);
  	}
	printk("nvp6114A_outport_2mux setting\n");
}


/*
vformat:0->NTSC, 1->PAL
port1_mode: 
高4bit选择port接口[7:4]->0~1:port0~1; 
低4bit选择vi模式[3:0]-> (NVP6124_VI_SD,NVP6124_VI_720P_2530)
*/
void nvp6114a_outport_4mux(unsigned char vformat, unsigned char port1_mode)
{
	int ch, i;
	unsigned char tmp=0;
	unsigned char p1_num,port1_vimode;
	nvp6124_video_mode vmode;

	p1_num = port1_mode>>0x04;
	port1_vimode = port1_mode&0x0F;
	for(ch=0;ch<nvp6124_cnt*4;ch++)
  	{
  		vmode.vformat[0] = vformat;
		vmode.chmode[ch] = port1_vimode;
	}
	nvp6124_each_mode_setting(&vmode);
	for(i=0;i<nvp6124_cnt;i++)
	{
		I2CWriteByte(nvp6124_slave_addr[i], 0xFF, 0x00);
		I2CWriteByte(nvp6124_slave_addr[i], 0x55, 0x10);
		I2CWriteByte(nvp6124_slave_addr[i], 0x56, 0x32);
		I2CWriteByte(nvp6124_slave_addr[i], 0xFF, 0x01);
		I2CWriteByte(nvp6124_slave_addr[i], 0xC2+p1_num*2, 0x10);    
		I2CWriteByte(nvp6124_slave_addr[i], 0xC3+p1_num*2, 0x32);
		
		I2CWriteByte(nvp6124_slave_addr[i], 0xC6, I2CReadByte(nvp6124_slave_addr[i], 0xC4));
		I2CWriteByte(nvp6124_slave_addr[i], 0xC7, I2CReadByte(nvp6124_slave_addr[i], 0xC5));
		tmp = ((port1_vimode==NVP6124_VI_720P_2530?3:8)<<4)|((port1_vimode==NVP6124_VI_720P_2530?3:8));
		if(p1_num == 0)
		{
			I2CWriteByte(nvp6124_slave_addr[i], 0xC8, tmp);
		}
		else
		{
			I2CWriteByte(nvp6124_slave_addr[i], 0xC9, tmp); 
		}		
		I2CWriteByte(nvp6124_slave_addr[i], 0xCD+p1_num*2, 0x66);    //时钟频率设置
  	}
	printk("nvp6114A_outport_4mux setting\n");
}

/*
chip:chip select[0,1,2,3];
portsel: port select[0,1];
portmode: port mode select[1mux,2mux,4mux]
chid:  channel id, 1mux[0,1,2,3], 2mux[0,1], 4mux[0]
*/
void nvp6114a_set_portmode(unsigned char chip, unsigned char portsel, unsigned char portmode, unsigned char chid)
{
	//unsigned char ch, i, tmp=0;
	unsigned char chipaddr = nvp6124_slave_addr[chip];
	if(portsel>1)
		printk("nvp6114a_set_portmode portsel[%d] error!!!\n", portsel);
	if(portmode == NVP6124_OUTMODE_2MUX_FHD)
	{
		if(chipon297MHz[chip] == 0)
			nvp6124_set_clockmode(chip, 1);
	}
	else
	{
		if(chipon297MHz[chip] == 1)
			nvp6124_set_clockmode(chip, 0);
	}
	switch(portmode)
	{
		case NVP6124_OUTMODE_1MUX_SD:
			I2CWriteByte(chipaddr, 0xFF, 0x00);
			I2CWriteByte(chipaddr, 0x56, 0x10);
			I2CWriteByte(chipaddr, 0xFF, 0x01);
			I2CWriteByte(chipaddr, 0xC2+portsel*2, (chid<<4)|chid);    
			I2CWriteByte(chipaddr, 0xC3+portsel*2, (chid<<4)|chid);
			I2CWriteByte(chipaddr, 0xC6, I2CReadByte(chipaddr, 0xC4));
			I2CWriteByte(chipaddr, 0xC7, I2CReadByte(chipaddr, 0xC5));
			I2CWriteByte(chipaddr, 0xC8+portsel, 0x00);
			I2CWriteByte(chipaddr, 0xCD+(portsel<<1), 0x36);			
		break;
		case NVP6124_OUTMODE_1MUX_HD:
			I2CWriteByte(chipaddr, 0xFF, 0x00);
			I2CWriteByte(chipaddr, 0x56, 0x10);
			I2CWriteByte(chipaddr, 0xFF, 0x01);
			I2CWriteByte(chipaddr, 0xC2+portsel*2, (chid<<4)|chid);    
			I2CWriteByte(chipaddr, 0xC3+portsel*2, (chid<<4)|chid);
			I2CWriteByte(chipaddr, 0xC6, I2CReadByte(chipaddr, 0xC4));
			I2CWriteByte(chipaddr, 0xC7, I2CReadByte(chipaddr, 0xC5));
			I2CWriteByte(chipaddr, 0xC8+portsel, 0x00);
			I2CWriteByte(chipaddr, 0xCD+(portsel<<1), 0x46);   
		break;
		case NVP6124_OUTMODE_1MUX_HD5060:
		case NVP6124_OUTMODE_1MUX_FHD:	
			I2CWriteByte(chipaddr, 0xFF, 0x00);
			I2CWriteByte(chipaddr, 0x56, 0x10);
			I2CWriteByte(chipaddr, 0xFF, 0x01);
			I2CWriteByte(chipaddr, 0xC2+portsel*2, (chid<<4)|chid);    
			I2CWriteByte(chipaddr, 0xC3+portsel*2, (chid<<4)|chid);
			I2CWriteByte(chipaddr, 0xC6, I2CReadByte(chipaddr, 0xC4));
			I2CWriteByte(chipaddr, 0xC7, I2CReadByte(chipaddr, 0xC5));
			I2CWriteByte(chipaddr, 0xC8+portsel, 0x00);
			I2CWriteByte(chipaddr, 0xCD+(portsel<<1), 0x66);
			break;
		case NVP6124_OUTMODE_2MUX_SD:
			I2CWriteByte(chipaddr, 0xFF, 0x00);
			I2CWriteByte(chipaddr, 0x56, 0x10);
			I2CWriteByte(chipaddr, 0xFF, 0x01);
			I2CWriteByte(chipaddr, 0xC2+portsel*2, chid==0?0x10:0x32);    
			I2CWriteByte(chipaddr, 0xC3+portsel*2, chid==0?0x10:0x32);
			I2CWriteByte(chipaddr, 0xC6, I2CReadByte(chipaddr, 0xC4));
			I2CWriteByte(chipaddr, 0xC7, I2CReadByte(chipaddr, 0xC5));
			I2CWriteByte(chipaddr, 0xC8+portsel, 0x22);
			I2CWriteByte(chipaddr, 0xCD+(portsel<<1), 0x46);
			break;
		case NVP6124_OUTMODE_2MUX_HD_X:
		case NVP6124_OUTMODE_2MUX_FHD_X:	
			I2CWriteByte(chipaddr, 0xFF, 0x00);
			I2CWriteByte(chipaddr, 0x56, 0x10);
			I2CWriteByte(chipaddr, 0xFF, 0x01);
			I2CWriteByte(chipaddr, 0xC2+portsel*2, chid==0?0x10:0x32);    
			I2CWriteByte(chipaddr, 0xC3+portsel*2, chid==0?0x10:0x32);
			I2CWriteByte(chipaddr, 0xC6, I2CReadByte(chipaddr, 0xC4));
			I2CWriteByte(chipaddr, 0xC7, I2CReadByte(chipaddr, 0xC5));
			I2CWriteByte(chipaddr, 0xC8+portsel, 0x11);
			I2CWriteByte(chipaddr, 0xCD+(portsel<<1), 0x66);
			break;
		case NVP6124_OUTMODE_2MUX_HD:
		//case NVP6124_OUTMODE_2MUX_FHD:	
			I2CWriteByte(chipaddr, 0xFF, 0x00);
			I2CWriteByte(chipaddr, 0x56, 0x10);
			I2CWriteByte(chipaddr, 0xFF, 0x01);
			I2CWriteByte(chipaddr, 0xC2+portsel*2, chid==0?0x10:0x32);    
			I2CWriteByte(chipaddr, 0xC3+portsel*2, chid==0?0x10:0x32);
			I2CWriteByte(chipaddr, 0xC6, I2CReadByte(chipaddr, 0xC4));
			I2CWriteByte(chipaddr, 0xC7, I2CReadByte(chipaddr, 0xC5));
			I2CWriteByte(chipaddr, 0xC8+portsel, 0x22);
			I2CWriteByte(chipaddr, 0xCD+(portsel<<1), 0x66);
			break;
		case NVP6124_OUTMODE_4MUX_SD:
		case NVP6124_OUTMODE_4MUX_HD:	
			I2CWriteByte(chipaddr, 0xFF, 0x00);
			I2CWriteByte(chipaddr, 0x56, 0x32);
			I2CWriteByte(chipaddr, 0xFF, 0x01);
			I2CWriteByte(chipaddr, 0xC2+portsel*2, 0x10);    
			I2CWriteByte(chipaddr, 0xC3+portsel*2, 0x32);
			I2CWriteByte(chipaddr, 0xC6, I2CReadByte(chipaddr, 0xC4));
			I2CWriteByte(chipaddr, 0xC7, I2CReadByte(chipaddr, 0xC5));
			I2CWriteByte(chipaddr, 0xC8+portsel, 0x88);
			I2CWriteByte(chipaddr, 0xCD+(portsel<<1), 0x66);  
			break;
		case NVP6124_OUTMODE_4MUX_HD_X:
			I2CWriteByte(chipaddr, 0xFF, 0x00);
			I2CWriteByte(chipaddr, 0x56, 0x32);
			I2CWriteByte(chipaddr, 0xFF, 0x01);
			I2CWriteByte(chipaddr, 0xC2+portsel*2, 0x10);    
			I2CWriteByte(chipaddr, 0xC3+portsel*2, 0x32);
			I2CWriteByte(chipaddr, 0xC6, I2CReadByte(chipaddr, 0xC4));
			I2CWriteByte(chipaddr, 0xC7, I2CReadByte(chipaddr, 0xC5));			
			I2CWriteByte(chipaddr, 0xC8+portsel, 0x33);
			I2CWriteByte(chipaddr, 0xCD+(portsel<<1), 0x66);
			break;
		case NVP6124_OUTMODE_2MUX_FHD:	
			I2CWriteByte(chipaddr, 0xFF, 0x00);
			I2CWriteByte(chipaddr, 0x56, 0x10);
			I2CWriteByte(chipaddr, 0xFF, 0x01);
			I2CWriteByte(chipaddr, 0x88+chid*2, 0x88);
			I2CWriteByte(chipaddr, 0x89+chid*2, 0x88);
			I2CWriteByte(chipaddr, 0x8C+chid*2, 0x42);
			I2CWriteByte(chipaddr, 0x8D+chid*2, 0x42);
			I2CWriteByte(chipaddr, 0xC2+portsel*2, chid==0?0x10:0x32);    
			I2CWriteByte(chipaddr, 0xC3+portsel*2, chid==0?0x10:0x32);
			I2CWriteByte(chipaddr, 0xC6, I2CReadByte(chipaddr, 0xC4));
			I2CWriteByte(chipaddr, 0xC7, I2CReadByte(chipaddr, 0xC5));
			I2CWriteByte(chipaddr, 0xC8+portsel, 0x22);
			I2CWriteByte(chipaddr, 0xCD+(portsel<<1), 0x46);
			break;	
		default:
			printk("portmode %d not supported yet\n", portmode);
			break;		
  	}
	printk("nvp6114a_set_portmode portsel %d portmode %d setting\n", portsel, portmode);
}


void nvp6124_each_mode_setting(nvp6124_video_mode *pvmode )
{  
	int i;
	unsigned char tmp;
	unsigned char ch, chmode[16];
	unsigned char pn_value_sd_nt_comet[4] = {0x4D,0x0E,0x88,0x6C};
	unsigned char pn_value_720p_30[4] = 	{0xEE,0x00,0xE5,0x4E};
	unsigned char pn_value_720p_60[4] = 	{0x2C,0xF9,0xC5,0x52};
	unsigned char pn_value_fhd_nt[4] = 		{0x2C,0xF0,0xCA,0x52};
	unsigned char pn_value_sd_pal_comet[4] = {0x75,0x35,0xB4,0x6C};
	unsigned char pn_value_720p_25[4] = 	{0x46,0x08,0x10,0x4F};
	unsigned char pn_value_720p_50[4] = 	{0x2C,0xE7,0xCF,0x52};
	unsigned char pn_value_fhd_pal[4] = 	{0xC8,0x7D,0xC3,0x52};
	unsigned char vformat = pvmode->vformat[0];
	
  	for(ch=0;ch<(nvp6124_cnt*4);ch++)
  	{
		chmode[ch] = pvmode->chmode[ch];
		//printk("\nchmode[%d] == %d \n", ch,pvmode->chmode[ch]);
  	}
	for(i=0;i<nvp6124_cnt;i++)
  	{
		I2CWriteByte(nvp6124_slave_addr[i], 0xFF, 0);
		I2CWriteByte(nvp6124_slave_addr[i], 0x80, 0x0f);
		I2CWriteByte(nvp6124_slave_addr[i], 0xFF, 1);
		I2CWriteByte(nvp6124_slave_addr[i], 0x93, 0x80);
  	}
	
	for(ch=0;ch<(nvp6124_cnt*4);ch++)
	{
		if(chmode[ch] < NVP6124_VI_BUTT) 
		{
			switch(chmode[ch])
			{
				case NVP6124_VI_SD:
				case NVP6124_VI_1920H:	
				case NVP6124_VI_720H:	
				case NVP6124_VI_1280H:	
				case NVP6124_VI_1440H:	
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0xFF, 0x00);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x08+ch%4, vformat==PAL?0xDD:0xA0);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x0c+ch%4, nvp6124_bri_tbl_960H[vformat%2]);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x10+ch%4, nvp6124_con_tbl_960H[vformat%2]);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x14+ch%4, vformat==PAL?0x80:0x80);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x18+ch%4, vformat==PAL?0x18:0x18);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x21+4*(ch%4), vformat==PAL?0x02:0x82);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x23+4*(ch%4), 0x43);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x30+ch%4, vformat==PAL?0x12:0x11);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x3c+ch%4, nvp6124_sat_tbl_960H[vformat%2]);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x40+ch%4, nvp6124_hue_tbl_960H[vformat%2]);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x44+ch%4, vformat==PAL?0x00:0x00);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x48+ch%4, vformat==PAL?0x00:0x00);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x4c+ch%4, vformat==PAL?0x04:0x00);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x50+ch%4, vformat==PAL?0x04:0x00);
					if(chmode[ch]==NVP6124_VI_SD)
						I2CWriteByte(nvp6124_slave_addr[ch/4], 0x58+ch%4, vformat==PAL?0x80:0x90);
					else if(chmode[ch]==NVP6124_VI_720H)
						I2CWriteByte(nvp6124_slave_addr[ch/4], 0x58+ch%4, vformat==PAL?0xB0:0x40);
					else if(chmode[ch]==NVP6124_VI_1280H)
						I2CWriteByte(nvp6124_slave_addr[ch/4], 0x58+ch%4, vformat==PAL?0x80:0x90);
					else if(chmode[ch]==NVP6124_VI_1440H)
						I2CWriteByte(nvp6124_slave_addr[ch/4], 0x58+ch%4, vformat==PAL?0x90:0xA0);
					else
						I2CWriteByte(nvp6124_slave_addr[ch/4], 0x58+ch%4, vformat==PAL?0x3B:0x4B);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x5c+ch%4, vformat==PAL?0x1e:0x1e);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x64+ch%4, vformat==PAL?0x0d:0x08);
					if(chmode[ch]==NVP6124_VI_SD || chmode[ch]==NVP6124_VI_720H)
					{
		 				I2CWriteByte(nvp6124_slave_addr[ch/4], 0x81+ch%4, vformat==PAL?0x00:0x00);	
		 				I2CWriteByte(nvp6124_slave_addr[ch/4], 0x85+ch%4, vformat==PAL?0x11:0x11);
						if(chmode[ch]==NVP6124_VI_720H)
						{
		 					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x89+ch%4, vformat==PAL?0x11:0x11);
		 					I2CWriteByte(nvp6124_slave_addr[ch/4], ch%4+0x8E, vformat==PAL?0x20:0x00);
						}
						else
						{
		 				I2CWriteByte(nvp6124_slave_addr[ch/4], 0x89+ch%4, vformat==PAL?0x10:0x00);
		 				I2CWriteByte(nvp6124_slave_addr[ch/4], ch%4+0x8E, vformat==PAL?0x08:0x07);
						}	
					}
					else if(chmode[ch]==NVP6124_VI_1280H)
					{
		 				I2CWriteByte(nvp6124_slave_addr[ch/4], 0x81+ch%4, vformat==PAL?0x20:0x20);	
		 				I2CWriteByte(nvp6124_slave_addr[ch/4], 0x85+ch%4, vformat==PAL?0x00:0x00);
		 				I2CWriteByte(nvp6124_slave_addr[ch/4], 0x89+ch%4, vformat==PAL?0x10:0x10);
		 				I2CWriteByte(nvp6124_slave_addr[ch/4], ch%4+0x8E, vformat==PAL?0x17:0x17);
					}
					else if(chmode[ch]==NVP6124_VI_1440H)
					{
		 				I2CWriteByte(nvp6124_slave_addr[ch/4], 0x81+ch%4, vformat==PAL?0x30:0x30);	
		 				I2CWriteByte(nvp6124_slave_addr[ch/4], 0x85+ch%4, vformat==PAL?0x00:0x00);
		 				I2CWriteByte(nvp6124_slave_addr[ch/4], 0x89+ch%4, vformat==PAL?0x10:0x10);
		 				I2CWriteByte(nvp6124_slave_addr[ch/4], ch%4+0x8E, vformat==PAL?0x0B:0x0B);
					}
					else
					{
		 				I2CWriteByte(nvp6124_slave_addr[ch/4], 0x81+ch%4, vformat==PAL?0x40:0x40);	
		 				I2CWriteByte(nvp6124_slave_addr[ch/4], 0x85+ch%4, vformat==PAL?0x00:0x00);
		 				I2CWriteByte(nvp6124_slave_addr[ch/4], 0x89+ch%4, vformat==PAL?0x00:0x00);
		 				I2CWriteByte(nvp6124_slave_addr[ch/4], ch%4+0x8E, vformat==PAL?0x10:0x10);
					}	
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x93+ch%4, 0x01);   //hzoom on
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x98+ch%4, vformat==PAL?0x05:0x04); //hzoom ration
	 				I2CWriteByte(nvp6124_slave_addr[ch/4], 0xa0+ch%4, vformat==PAL?0x00:0x10);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0xa4+ch%4, vformat==PAL?0x00:0x01);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0xFF, 0x01);
					if(chmode[ch]==NVP6124_VI_SD || chmode[ch]==NVP6124_VI_720H)
					{
						I2CWriteByte(nvp6124_slave_addr[ch/4], 0x88+ch%4, vformat==PAL?0x7e:0x7e);
						I2CWriteByte(nvp6124_slave_addr[ch/4], 0x8c+ch%4, vformat==PAL?0x26:0x26);
						I2CWriteByte(nvp6124_slave_addr[ch/4], 0xd7, I2CReadByte( nvp6124_slave_addr[ch/4], 0xd7)&(~(1<<(ch%4))));
					}
					else if(chmode[ch]==NVP6124_VI_1280H)
					{
						I2CWriteByte(nvp6124_slave_addr[ch/4], 0x88+ch%4, vformat==PAL?0x7e:0x7e);
						I2CWriteByte(nvp6124_slave_addr[ch/4], 0x8c+ch%4, vformat==PAL?0x56:0x56);
						I2CWriteByte(nvp6124_slave_addr[ch/4], 0xd7,I2CReadByte( nvp6124_slave_addr[ch/4], 0xd7)|(1<<(ch%4)));
					}
					else if(chmode[ch]==NVP6124_VI_1440H)
					{
						I2CWriteByte(nvp6124_slave_addr[ch/4], 0x88+ch%4, vformat==PAL?0x7e:0x7e);
						I2CWriteByte(nvp6124_slave_addr[ch/4], 0x8c+ch%4, vformat==PAL?0x56:0x56);
						I2CWriteByte(nvp6124_slave_addr[ch/4], 0xd7,I2CReadByte( nvp6124_slave_addr[ch/4], 0xd7)|(1<<(ch%4)));
					}
					else
					{
						I2CWriteByte(nvp6124_slave_addr[ch/4], 0x88+ch%4, vformat==PAL?0x46:0x46);
						I2CWriteByte(nvp6124_slave_addr[ch/4], 0x8c+ch%4, vformat==PAL?0x47:0x47);
						I2CWriteByte(nvp6124_slave_addr[ch/4], 0xd7,I2CReadByte( nvp6124_slave_addr[ch/4], 0xd7)|(1<<(ch%4)));
					}	
					if(chip_id[0] == NVP6124_R0_ID)
						I2CWriteByte(nvp6124_slave_addr[ch/4], 0xcc+ch_seq[ch%4], 0x36);
					else if(chip_id[0] == NVP6114A_R0_ID && ch_seq[ch]!=0xFF)
						I2CWriteByte(nvp6124_slave_addr[ch/4], 0xcd+(ch_seq[ch%4]%2)*2, 0x36);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0xFF, 0x02);  
					tmp = I2CReadByte( nvp6124_slave_addr[ch/4], 0x16+(ch%4)/2);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x16+(ch%4)/2, (tmp&(ch%2==0?0xF0:0x0F))|(0x00<<((ch%2)*4)));
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0xFF, 0x05+ch%4);
					nvp6124_write_table( nvp6124_slave_addr[ch/4], 0x00, vformat==PAL?NVP6124_B5678_PAL_Buf:NVP6124_B5678_NTSC_Buf, 254 );
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x06,0x40);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x1B,0x08);
                    I2CWriteByte(nvp6124_slave_addr[ch/4], 0x20,0x88);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x25,vformat==PAL?0xCA:0xDA);
					if(chmode[ch]==NVP6124_VI_1280H || chmode[ch]==NVP6124_VI_1440H)
					{
						I2CWriteByte(nvp6124_slave_addr[ch/4], 0x62, vformat==PAL?0x20:0x20);
						I2CWriteByte(nvp6124_slave_addr[ch/4], 0x64, vformat==PAL?0x0D:0x0D);
					}
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0xFF, 0x09);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x40+(ch%4), 0x60);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x44, I2CReadByte( nvp6124_slave_addr[ch/4], 0x44)&(~(1<<(ch%4))));
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x50+4*(ch%4),vformat==PAL?pn_value_sd_pal_comet[0]:pn_value_sd_nt_comet[0]);	//ch%41 960H	
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x51+4*(ch%4),vformat==PAL?pn_value_sd_pal_comet[1]:pn_value_sd_nt_comet[1]);		
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x52+4*(ch%4),vformat==PAL?pn_value_sd_pal_comet[2]:pn_value_sd_nt_comet[2]);		
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x53+4*(ch%4),vformat==PAL?pn_value_sd_pal_comet[3]:pn_value_sd_nt_comet[3]);
					printk("ch %d setted to %d %s\n", ch, chmode[ch], vformat==PAL?"PAL":"NTSC");
				break;	
				case NVP6124_VI_720P_2530:
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0xFF, 0x00);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x08+ch%4,vformat==PAL?0x60:0x60);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x0c+ch%4,nvp6124_bri_tbl_720P[vformat%2]);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x10+ch%4,nvp6124_con_tbl_720P[vformat%2]);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x14+ch%4,vformat==PAL?0x90:0x90);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x18+ch%4,vformat==PAL?0x30:0x30);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x21+4*(ch%4), 0x92);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x22+4*(ch%4), 0x0A);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x23+4*(ch%4), 0x43);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x30+ch%4,vformat==PAL?0x12:0x12);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x3c+ch%4,nvp6124_sat_tbl_720P[vformat%2]);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x40+ch%4,nvp6124_hue_tbl_720P[vformat%2]);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x44+ch%4,vformat==PAL?0x18:0x18);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x48+ch%4,vformat==PAL?0x18:0x18);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x4c+ch%4,vformat==PAL?0x04:0x04);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x50+ch%4,vformat==PAL?0x04:0x04);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x58+ch%4,vformat==PAL?0x80:0x90);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x5c+ch%4,vformat==PAL?0x9e:0x9e);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x64+ch%4,vformat==PAL?0xb1:0xb2);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x81+ch%4,vformat==PAL?0x07:0x06);	
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x85+ch%4,vformat==PAL?0x00:0x00);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x89+ch%4,vformat==PAL?0x10:0x10);
					I2CWriteByte(nvp6124_slave_addr[ch/4], ch%4+0x8E,vformat==PAL?0x0d:0x0d);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x93+ch%4,0x00);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x98+ch%4,vformat==PAL?0x07:0x04);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0xa0+ch%4,vformat==PAL?0x00:0x00);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0xa4+ch%4,vformat==PAL?0x00:0x01);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0xFF,0x01);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x88+ch%4,vformat==PAL?0x5C:0x5C);	
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x8c+ch%4,vformat==PAL?0x40:0x40);
					if(chip_id[0] == NVP6124_R0_ID)
						I2CWriteByte(nvp6124_slave_addr[ch/4], 0xcc+ch_seq[ch%4], 0x46);
					else if(chip_id[0] == NVP6114A_R0_ID && ch_seq[ch]!=0xFF)
						I2CWriteByte(nvp6124_slave_addr[ch/4], 0xcd+(ch_seq[ch%4]%2)*2, 0x46);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0xd7,I2CReadByte( nvp6124_slave_addr[ch/4], 0xd7)|(1<<(ch%4)));
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0xFF, 0x02);  
					tmp = I2CReadByte( nvp6124_slave_addr[ch/4], 0x16+(ch%4)/2);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x16+(ch%4)/2, (tmp&(ch%2==0?0xF0:0x0F))|(0x05<<((ch%2)*4)));
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0xFF, 0x05+ch%4);
					nvp6124_write_table( nvp6124_slave_addr[ch/4], 0x00, vformat==PAL?NVP6124_B5678_25P_Buf:NVP6124_B5678_30P_Buf,254 );
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x01,0x0D); 
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x06,0x40); 
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x2B,0x78); 
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x59,0x00);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x58,0x13);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0xC0,0x16);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0xD8,0x0C);    
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0xD9,0x0E); 
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0xDA,0x12);    
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0xDB,0x14); 
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0xDC,0x1C);    
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0xDD,0x2C);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0xDE,0x34);
					
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0xFF, 0x09);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x40+(ch%4),0x00);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x44, I2CReadByte( nvp6124_slave_addr[ch/4], 0x44)|(1<<(ch%4)));
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x50+4*(ch%4),vformat==PAL?pn_value_720p_25[0]:pn_value_720p_30[0]);	//ch%41 960H	
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x51+4*(ch%4),vformat==PAL?pn_value_720p_25[1]:pn_value_720p_30[1]);		
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x52+4*(ch%4),vformat==PAL?pn_value_720p_25[2]:pn_value_720p_30[2]);		
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x53+4*(ch%4),vformat==PAL?pn_value_720p_25[3]:pn_value_720p_30[3]);
					printk("ch %d setted to 720P %s\n", ch, vformat==PAL?"PAL":"NTSC");
				break;
				case NVP6124_VI_720P_5060:
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0xFF, 0x00);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x08+ch%4,vformat==PAL?0x60:0x60);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x0c+ch%4,nvp6124_bri_tbl_720P[vformat%2]);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x10+ch%4,nvp6124_con_tbl_720P[vformat%2]);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x14+ch%4,vformat==PAL?0x90:0x90);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x18+ch%4,vformat==PAL?0x00:0x00);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x21+4*(ch%4), 0x92);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x22+4*(ch%4), 0x0A);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x23+4*(ch%4), vformat==PAL?0x43:0x43);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x30+ch%4,vformat==PAL?0x12:0x12);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x3c+ch%4,nvp6124_sat_tbl_720P[vformat%2]);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x40+ch%4,nvp6124_hue_tbl_720P[vformat%2]);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x44+ch%4,vformat==PAL?0x18:0x18);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x48+ch%4,vformat==PAL?0x18:0x18);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x4c+ch%4,vformat==PAL?0x04:0x04);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x50+ch%4,vformat==PAL?0x04:0x04);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x58+ch%4,vformat==PAL?0xc0:0xb0);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x5c+ch%4,vformat==PAL?0x9e:0x9e);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x64+ch%4,vformat==PAL?0xb1:0xb2);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x81+ch%4,vformat==PAL?0x05:0x04);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x85+ch%4,vformat==PAL?0x00:0x00);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x89+ch%4,vformat==PAL?0x10:0x10);
					I2CWriteByte(nvp6124_slave_addr[ch/4], ch%4+0x8E,vformat==PAL?0x0b:0x09);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x93+ch%4,0x00);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x98+ch%4,vformat==PAL?0x07:0x04);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0xa0+ch%4,vformat==PAL?0x00:0x00);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0xa4+ch%4,vformat==PAL?0x00:0x01);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0xFF, 0x01);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x88+ch%4,vformat==PAL?0x4d:0x4d);	
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x8c+ch%4,vformat==PAL?0x84:0x84);
					if(chip_id[0] == NVP6124_R0_ID)
						I2CWriteByte(nvp6124_slave_addr[ch/4], 0xcc+ch_seq[ch%4], 0x66);
					else if(chip_id[0] == NVP6114A_R0_ID && ch_seq[ch]!=0xFF)
						I2CWriteByte(nvp6124_slave_addr[ch/4], 0xcd+(ch_seq[ch%4]%2)*2, 0x66);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0xd7, I2CReadByte( nvp6124_slave_addr[ch/4], 0xd7)|(1<<(ch%4)));
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0xFF, 0x02); 
					tmp = I2CReadByte( nvp6124_slave_addr[ch/4], 0x16+(ch%4)/2);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x16+(ch%4)/2, (tmp&(ch%2==0?0xF0:0x0F))|(0x05<<((ch%2)*4)));
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0xFF, 0x05+ch%4);
					nvp6124_write_table( nvp6124_slave_addr[ch/4], 0x00, vformat==PAL?NVP6124_B5678_25P_Buf:NVP6124_B5678_30P_Buf,254 );
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x01,0x0C);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x06,0x40);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x2B,0x78); 
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x59,0x01);  
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x24,vformat==PAL?0x2A:0x1A);  
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x50,vformat==PAL?0x84:0x86);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0xBB,vformat==PAL?0x00:0xE4);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0xFF,0x09);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x40+(ch%4), 0x00);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x44, I2CReadByte( nvp6124_slave_addr[ch/4], 0x44)|(1<<(ch%4)));
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x50+4*(ch%4),vformat==PAL?pn_value_720p_50[0]:pn_value_720p_60[0]);	//ch%41 960H	
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x51+4*(ch%4),vformat==PAL?pn_value_720p_50[1]:pn_value_720p_60[1]);		
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x52+4*(ch%4),vformat==PAL?pn_value_720p_50[2]:pn_value_720p_60[2]);		
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x53+4*(ch%4),vformat==PAL?pn_value_720p_50[3]:pn_value_720p_60[3]);
					printk("ch %d setted to 720P@RT %s\n", ch, vformat==PAL?"PAL":"NTSC");
				break;		
				case NVP6124_VI_1080P_2530:
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0xFF, 0x00);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x08+ch%4,vformat==PAL?0x60:0x60);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x0c+ch%4,nvp6124_bri_tbl[vformat%2]);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x10+ch%4,nvp6124_con_tbl[vformat%2]);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x14+ch%4,vformat==PAL?0x90:0x90);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x18+ch%4,vformat==PAL?0x00:0x00);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x21+4*(ch%4), 0x92);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x22+4*(ch%4), 0x0A);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x23+4*(ch%4), vformat==PAL?0x43:0x43);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x30+ch%4,vformat==PAL?0x12:0x12);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x3c+ch%4,nvp6124_sat_tbl[vformat%2]);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x40+ch%4,nvp6124_hue_tbl[vformat%2]);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x44+ch%4,vformat==PAL?0x18:0x18);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x48+ch%4,vformat==PAL?0x18:0x18);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x4c+ch%4,vformat==PAL?0x00:0x00);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x50+ch%4,vformat==PAL?0x00:0x00);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x58+ch%4,vformat==PAL?0x6a:0x49);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x5c+ch%4,vformat==PAL?0x9e:0x9e);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x64+ch%4,vformat==PAL?0xbf:0x8d);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x81+ch%4,vformat==PAL?0x03:0x02);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x85+ch%4,vformat==PAL?0x00:0x00);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x89+ch%4,vformat==PAL?0x10:0x10);
					I2CWriteByte(nvp6124_slave_addr[ch/4], ch%4+0x8E,vformat==PAL?0x0a:0x09);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x93+ch%4,0x00);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x98+ch%4,vformat==PAL?0x07:0x04);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0xa0+ch%4,vformat==PAL?0x00:0x00);	
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0xa4+ch%4,vformat==PAL?0x00:0x01);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0xFF, 0x01);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x88+ch%4,vformat==PAL?0x4c:0x4c);	
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x8c+ch%4,vformat==PAL?0x84:0x84);							 		
					if(chip_id[0] == NVP6124_R0_ID)
						I2CWriteByte(nvp6124_slave_addr[ch/4], 0xcc+ch_seq[ch%4], 0x66);
					else if(chip_id[0] == NVP6114A_R0_ID && ch_seq[ch]!=0xFF)
						I2CWriteByte(nvp6124_slave_addr[ch/4], 0xcd+(ch_seq[ch%4]%2)*2, 0x66);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0xd7, I2CReadByte( nvp6124_slave_addr[ch/4], 0xd7)|(1<<(ch%4)));
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0xFF, 0x02);  
					tmp = I2CReadByte( nvp6124_slave_addr[ch/4], 0x16+(ch%4)/2);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x16+(ch%4)/2, (tmp&(ch%2==0?0xF0:0x0F))|(0x05<<((ch%2)*4)));
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0xFF, 0x05+ch%4);
					nvp6124_write_table( nvp6124_slave_addr[ch/4], 0x00, vformat==PAL?NVP6124_B5678_25P_Buf:NVP6124_B5678_30P_Buf,254 );
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x01,0x0C);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x06,0x40);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x2A,0x72);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x2B,0xA8); 
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x58,0x13);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x59,0x01);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0xD8,0x10);    
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0xD9,0x1F); 
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0xDA,0x2B);    
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0xDB,0x7F); 
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0xDC,0xFF);    
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0xDD,0xFF);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0xDE,0xFF);
					
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x24,vformat==PAL?0x2A:0x1A);  
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x50,vformat==PAL?0x84:0x86);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0xBB,vformat==PAL?0x00:0xE4);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0xFF, 0x09);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x40+(ch%4), 0x00);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x44,I2CReadByte( nvp6124_slave_addr[ch/4], 0x44)|(1<<(ch%4)));
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x50+4*(ch%4),vformat==PAL?pn_value_fhd_pal[0]:pn_value_fhd_nt[0]);		
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x51+4*(ch%4),vformat==PAL?pn_value_fhd_pal[1]:pn_value_fhd_nt[1]);		
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x52+4*(ch%4),vformat==PAL?pn_value_fhd_pal[2]:pn_value_fhd_nt[2]);		
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x53+4*(ch%4),vformat==PAL?pn_value_fhd_pal[3]:pn_value_fhd_nt[3]);
					printk("ch %d setted to 1080P %s\n", ch, vformat==PAL?"PAL":"NTSC");
				break;
				default:
					printk("ch%d wrong mode detected!!!\n", ch);
					break;
			}			
			ch_mode_status[ch] = chmode[ch];
			init_acp(ch);
			acp_each_setting(ch);
		}	
	}
}


/*
chip:chip select[0,1,2,3];
portsel: port select[0,1,2,3];
portmode: port mode select[1mux,2mux,4mux]
chid:  channel id, 1mux[0,1,2,3], 2mux[0,1], 4mux[0]
*/
void nvp6124_set_portmode(unsigned char chip, unsigned char portsel, unsigned char portmode, unsigned char chid)
{
	unsigned char tmp=0;
    //unsigned char ch, i;
	unsigned char chipaddr = nvp6124_slave_addr[chip];
	if(portmode == NVP6124_OUTMODE_2MUX_FHD || portmode == NVP6124_OUTMODE_4MUX_HD)
	{
		if(chipon297MHz[chip] == 0)
			nvp6124_set_clockmode(chip, 1);
	}
	else
	{
		if(chipon297MHz[chip] == 1)
			nvp6124_set_clockmode(chip, 0);
	}	
	switch(portmode)
	{
		case NVP6124_OUTMODE_1MUX_SD:
			I2CWriteByte(chipaddr, 0xFF, 0x00);
			I2CWriteByte(chipaddr, 0x56, 0x10);
			I2CWriteByte(chipaddr, 0xFF, 0x01);
			I2CWriteByte(chipaddr, 0xC0+portsel*2, (chid<<4)|chid);    
			I2CWriteByte(chipaddr, 0xC1+portsel*2, (chid<<4)|chid);
			tmp = I2CReadByte(chipaddr, 0xC8+portsel/2) & (portsel%2?0x0F:0xF0);
			I2CWriteByte(chipaddr, 0xC8+portsel/2, tmp);         
			tmp = I2CReadByte(chipaddr, 0xCA);
			tmp |= ((0x01<<(portsel+4)) | (0x01<<(portsel)));
			I2CWriteByte(chipaddr, 0xCA, tmp);    //打开端口[3:0]和时钟[7:4],和硬件相关。
			I2CWriteByte(chipaddr, 0xCC+portsel, 0x36);    //时钟频率设置
		break;
		case NVP6124_OUTMODE_1MUX_HD:
			I2CWriteByte(chipaddr, 0xFF, 0x00);
			I2CWriteByte(chipaddr, 0x56, 0x10);
			I2CWriteByte(chipaddr, 0xFF, 0x01);
			I2CWriteByte(chipaddr, 0xC0+portsel*2, (chid<<4)|chid);    
			I2CWriteByte(chipaddr, 0xC1+portsel*2, (chid<<4)|chid);
			tmp = I2CReadByte(chipaddr, 0xC8+portsel/2) & (portsel%2?0x0F:0xF0);
			I2CWriteByte(chipaddr, 0xC8+portsel/2, tmp);         
			tmp = I2CReadByte(chipaddr, 0xCA);
			tmp |= ((0x01<<(portsel+4)) | (0x01<<(portsel)));
			I2CWriteByte(chipaddr, 0xCA, tmp);    
			I2CWriteByte(chipaddr, 0xCC+portsel, 0x46);    
		break;
		case NVP6124_OUTMODE_1MUX_HD5060:
		case NVP6124_OUTMODE_1MUX_FHD:	
			I2CWriteByte(chipaddr, 0xFF, 0x00);
			I2CWriteByte(chipaddr, 0x56, 0x10);
			I2CWriteByte(chipaddr, 0xFF, 0x01);
			I2CWriteByte(chipaddr, 0xC0+portsel*2, (chid<<4)|chid);    
			I2CWriteByte(chipaddr, 0xC1+portsel*2, (chid<<4)|chid);
			tmp = I2CReadByte(chipaddr, 0xC8+portsel/2) & (portsel%2?0x0F:0xF0);
			I2CWriteByte(chipaddr, 0xC8+portsel/2, tmp);         
			tmp = I2CReadByte(chipaddr, 0xCA);
			tmp |= ((0x01<<(portsel+4)) | (0x01<<(portsel)));
			I2CWriteByte(chipaddr, 0xCA, tmp);    
			I2CWriteByte(chipaddr, 0xCC+portsel, 0x66); 
			break;
		case NVP6124_OUTMODE_2MUX_SD:
			I2CWriteByte(chipaddr, 0xFF, 0x00);
			I2CWriteByte(chipaddr, 0x56, 0x10);
			I2CWriteByte(chipaddr, 0xFF, 0x01);
			I2CWriteByte(chipaddr, 0xC0+portsel*2, chid==0?0x10:0x32);    
			I2CWriteByte(chipaddr, 0xC1+portsel*2, chid==0?0x10:0x32);
			tmp = I2CReadByte(chipaddr, 0xC8+portsel/2) & (portsel%2?0x0F:0xF0);
			tmp |= (portsel%2?0x20:0x02);
			I2CWriteByte(chipaddr, 0xC8+portsel/2, tmp);         
			tmp = I2CReadByte(chipaddr, 0xCA);
			tmp |= ((0x01<<(portsel+4)) | (0x01<<(portsel)));
			I2CWriteByte(chipaddr, 0xCA, tmp);    
			I2CWriteByte(chipaddr, 0xCC+portsel, 0x46); 
			break;
		case NVP6124_OUTMODE_2MUX_HD_X:
			I2CWriteByte(chipaddr, 0xFF, 0x00);
			I2CWriteByte(chipaddr, 0x56, 0x10);
			I2CWriteByte(chipaddr, 0xFF, 0x01);
			I2CWriteByte(chipaddr, 0xC0+portsel*2, chid==0?0x10:0x32);    
			I2CWriteByte(chipaddr, 0xC1+portsel*2, chid==0?0x10:0x32);
			tmp = I2CReadByte(chipaddr, 0xC8+portsel/2) & (portsel%2?0x0F:0xF0);
			tmp |= (portsel%2?0x10:0x01);
			I2CWriteByte(chipaddr, 0xC8+portsel/2, tmp);         
			tmp = I2CReadByte(chipaddr, 0xCA);
			tmp |= ((0x01<<(portsel+4)) | (0x01<<(portsel)));
			I2CWriteByte(chipaddr, 0xCA, tmp);    
			I2CWriteByte(chipaddr, 0xCC+portsel, 0x46); 
			break;
		case NVP6124_OUTMODE_2MUX_HD:
		//case NVP6124_OUTMODE_2MUX_FHD:	
			I2CWriteByte(chipaddr, 0xFF, 0x00);
			I2CWriteByte(chipaddr, 0x56, 0x10);
			I2CWriteByte(chipaddr, 0xFF, 0x01);
			I2CWriteByte(chipaddr, 0xC0+portsel*2, chid==0?0x10:0x32);    
			I2CWriteByte(chipaddr, 0xC1+portsel*2, chid==0?0x10:0x32);
			tmp = I2CReadByte(chipaddr, 0xC8+portsel/2) & (portsel%2?0x0F:0xF0);
			tmp |= (portsel%2?0x20:0x02);
			I2CWriteByte(chipaddr, 0xC8+portsel/2, tmp);         
			tmp = I2CReadByte(chipaddr, 0xCA);
			tmp |= ((0x01<<(portsel+4)) | (0x01<<(portsel)));
			I2CWriteByte(chipaddr, 0xCA, tmp);    
			I2CWriteByte(chipaddr, 0xCC+portsel, 0x66); 
			break;
		case NVP6124_OUTMODE_2MUX_FHD_X:
			I2CWriteByte(chipaddr, 0xFF, 0x00);
			I2CWriteByte(chipaddr, 0x56, 0x10);
			I2CWriteByte(chipaddr, 0xFF, 0x01);
			I2CWriteByte(chipaddr, 0xC0+portsel*2, chid==0?0x10:0x32);    
			I2CWriteByte(chipaddr, 0xC1+portsel*2, chid==0?0x10:0x32);
			tmp = I2CReadByte(chipaddr, 0xC8+portsel/2) & (portsel%2?0x0F:0xF0);
			tmp |= (portsel%2?0x10:0x01);
			I2CWriteByte(chipaddr, 0xC8+portsel/2, tmp);         
			tmp = I2CReadByte(chipaddr, 0xCA);
			tmp |= ((0x01<<(portsel+4)) | (0x01<<(portsel)));
			I2CWriteByte(chipaddr, 0xCA, tmp);    
			I2CWriteByte(chipaddr, 0xCC+portsel, 0x66); 
			break;
		case NVP6124_OUTMODE_4MUX_SD:
        case NVP6124_OUTMODE_4MUX_HD:    
			I2CWriteByte(chipaddr, 0xFF, 0x00);
			I2CWriteByte(chipaddr, 0x56, 0x32);
			I2CWriteByte(chipaddr, 0xFF, 0x01);
			I2CWriteByte(chipaddr, 0xC0+portsel*2, 0x10);    
			I2CWriteByte(chipaddr, 0xC1+portsel*2, 0x32);
			tmp = I2CReadByte(chipaddr, 0xC8+portsel/2) & (portsel%2?0x0F:0xF0);
			tmp |= (portsel%2?0x80:0x08);
			I2CWriteByte(chipaddr, 0xC8+portsel/2, tmp);         
			tmp = I2CReadByte(chipaddr, 0xCA);
			tmp |= ((0x01<<(portsel+4)) | (0x01<<(portsel)));
			I2CWriteByte(chipaddr, 0xCA, tmp);    
			I2CWriteByte(chipaddr, 0xCC+portsel, 0x66); 
			break;
		case NVP6124_OUTMODE_4MUX_HD_X:
			I2CWriteByte(chipaddr, 0xFF, 0x00);
			I2CWriteByte(chipaddr, 0x56, 0x32);
			I2CWriteByte(chipaddr, 0xFF, 0x01);
			I2CWriteByte(chipaddr, 0xC0+portsel*2, 0x10);    
			I2CWriteByte(chipaddr, 0xC1+portsel*2, 0x32);
			tmp = I2CReadByte(chipaddr, 0xC8+portsel/2) & (portsel%2?0x0F:0xF0);
			tmp |= (portsel%2?0x30:0x03);
			I2CWriteByte(chipaddr, 0xC8+portsel/2, tmp);         
			tmp = I2CReadByte(chipaddr, 0xCA);
			tmp |= ((0x01<<(portsel+4)) | (0x01<<(portsel)));
			I2CWriteByte(chipaddr, 0xCA, tmp);    
			I2CWriteByte(chipaddr, 0xCC+portsel, 0x66); 
			break;
		case NVP6124_OUTMODE_2MUX_FHD:	
			I2CWriteByte(chipaddr, 0xFF, 0x00);
			I2CWriteByte(chipaddr, 0x56, 0x10);
			I2CWriteByte(chipaddr, 0xFF, 0x01);
			I2CWriteByte(chipaddr, 0x88+chid*2, 0x88);
			I2CWriteByte(chipaddr, 0x89+chid*2, 0x88);
			I2CWriteByte(chipaddr, 0x8C+chid*2, 0x42);
			I2CWriteByte(chipaddr, 0x8D+chid*2, 0x42);
			I2CWriteByte(chipaddr, 0xC0+portsel*2, chid==0?0x10:0x32);    
			I2CWriteByte(chipaddr, 0xC1+portsel*2, chid==0?0x10:0x32);
			tmp = I2CReadByte(chipaddr, 0xC8+portsel/2) & (portsel%2?0x0F:0xF0);
			tmp |= (portsel%2?0x20:0x02);
			I2CWriteByte(chipaddr, 0xC8+portsel/2, tmp);         
			tmp = I2CReadByte(chipaddr, 0xCA);
			tmp |= ((0x01<<(portsel+4)) | (0x01<<(portsel)));
			I2CWriteByte(chipaddr, 0xCA, tmp);    
			I2CWriteByte(chipaddr, 0xCC+portsel, 0x46); 
            printk("NVP6124_OUTMODE_2MUX_FHD \n");
			break;	
		default:
			printk("portmode %d not supported yet\n", portmode);
			break;	
  	}
	printk("nvp6124_set_portmode portsel %d portmode %d setting\n", portsel, portmode);
}


int nvp6124_set_chnmode(unsigned char ch, unsigned char chnmode)
{
	nvp6124_video_mode vmode;
	int i;
	if((ch >= nvp6124_cnt*4) || chnmode >= NVP6124_VI_BUTT)
	{
		printk("channel/mode is out of range\n");
		return -1;
	}
	for(i=0;i<nvp6124_cnt*4;i++)
	{
		vmode.vformat[0] = nvp6124_mode%2;
		if(ch == i)
			vmode.chmode[i] = chnmode;
		else
			vmode.chmode[i] = NVP6124_VI_BUTT;
	}
	nvp6124_each_mode_setting(&vmode);
	return 0;
}

	
	

void nvp6124_video_set_contrast(unsigned int ch, unsigned int value, unsigned int v_format)
{	
	I2CWriteByte(nvp6124_slave_addr[ch/4], 0xFF, 0x00);
	if(value >= 100)
	{
		if(ch_mode_status[ch] == NVP6124_VI_1080P_2530)
			I2CWriteByte(nvp6124_slave_addr[ch/4], (0x10+(ch%4)),(nvp6124_con_tbl[v_format]+value-100));
		else if(ch_mode_status[ch] == NVP6124_VI_720P_2530 || ch_mode_status[ch] == NVP6124_VI_720P_5060)
			I2CWriteByte(nvp6124_slave_addr[ch/4], (0x10+(ch%4)),(nvp6124_con_tbl_720P[v_format]+value-100));
		else 
			I2CWriteByte(nvp6124_slave_addr[ch/4], (0x10+(ch%4)),(nvp6124_con_tbl_960H[v_format]+value-100));
	}
	else
	{
		if(ch_mode_status[ch] == NVP6124_VI_1080P_2530)
			I2CWriteByte(nvp6124_slave_addr[ch/4], (0x10+(ch%4)),(nvp6124_con_tbl[v_format]+(0xff-(98-value))));
		else if(ch_mode_status[ch] == NVP6124_VI_720P_2530 || ch_mode_status[ch] == NVP6124_VI_720P_5060)
			I2CWriteByte(nvp6124_slave_addr[ch/4], (0x10+(ch%4)),(nvp6124_con_tbl_720P[v_format]+(0xff-(98-value))));
		else
			I2CWriteByte(nvp6124_slave_addr[ch/4], (0x10+(ch%4)),(nvp6124_con_tbl_960H[v_format]+(0xff-(98-value))));
	}
}

void nvp6124_video_set_brightness(unsigned int ch, unsigned int value, unsigned int v_format)
{
	I2CWriteByte(nvp6124_slave_addr[ch/4], 0xFF, 0x00);
	if(value >= 100)
	{
		if(ch_mode_status[ch] == NVP6124_VI_1080P_2530)
			I2CWriteByte(nvp6124_slave_addr[ch/4], (0x0c+(ch%4)),(nvp6124_bri_tbl[v_format]+value-100));
		else if(ch_mode_status[ch] == NVP6124_VI_720P_2530 || ch_mode_status[ch] == NVP6124_VI_720P_5060)
			I2CWriteByte(nvp6124_slave_addr[ch/4], (0x0c+(ch%4)),(nvp6124_bri_tbl_720P[v_format]+value-100));
		else
			I2CWriteByte(nvp6124_slave_addr[ch/4], (0x0c+(ch%4)),(nvp6124_bri_tbl_960H[v_format]+value-100));
	}	
	else
	{
		if(ch_mode_status[ch] == NVP6124_VI_1080P_2530)
			I2CWriteByte(nvp6124_slave_addr[ch/4], (0x0c+(ch%4)),(nvp6124_bri_tbl[v_format]+(0xff-(98-value))));
		else if(ch_mode_status[ch] == NVP6124_VI_720P_2530 || ch_mode_status[ch] == NVP6124_VI_720P_5060)
			I2CWriteByte(nvp6124_slave_addr[ch/4], (0x0c+(ch%4)),(nvp6124_bri_tbl_720P[v_format]+(0xff-(98-value))));
		else
			I2CWriteByte(nvp6124_slave_addr[ch/4], (0x0c+(ch%4)),(nvp6124_bri_tbl_960H[v_format]+(0xff-(98-value))));
	}	
}

void nvp6124_video_set_saturation(unsigned int ch, unsigned int value, unsigned int v_format)
{
	I2CWriteByte(nvp6124_slave_addr[ch/4], 0xFF, 0x00);
	if(ch_mode_status[ch] == NVP6124_VI_1080P_2530)
		I2CWriteByte(nvp6124_slave_addr[ch/4], (0x3c+(ch%4)),(nvp6124_sat_tbl[v_format]+value-100));
	else if(ch_mode_status[ch] == NVP6124_VI_720P_2530 || ch_mode_status[ch] == NVP6124_VI_720P_5060)
		I2CWriteByte(nvp6124_slave_addr[ch/4], (0x3c+(ch%4)),(nvp6124_sat_tbl_720P[v_format]+value-100));
	else
		I2CWriteByte(nvp6124_slave_addr[ch/4], (0x3c+(ch%4)),(nvp6124_sat_tbl_960H[v_format]+value-100));
}

void nvp6124_video_set_hue(unsigned int ch, unsigned int value, unsigned int v_format)
{
	I2CWriteByte(nvp6124_slave_addr[ch/4], 0xFF, 0x00);
	if(ch_mode_status[ch] == NVP6124_VI_1080P_2530)
		I2CWriteByte(nvp6124_slave_addr[ch/4], (0x40+(ch%4)), (nvp6124_hue_tbl[v_format]+value-100));
	else if(ch_mode_status[ch] == NVP6124_VI_720P_2530 || ch_mode_status[ch] == NVP6124_VI_720P_5060)
		I2CWriteByte(nvp6124_slave_addr[ch/4], (0x40+(ch%4)), (nvp6124_hue_tbl_720P[v_format]+value-100));
	else
		I2CWriteByte(nvp6124_slave_addr[ch/4], (0x40+(ch%4)), (nvp6124_hue_tbl_960H[v_format]+value-100));
}

void nvp6124_video_set_sharpness(unsigned int ch, unsigned int value)
{
	I2CWriteByte(nvp6124_slave_addr[ch/4], 0xFF, 0x00);
	I2CWriteByte(nvp6124_slave_addr[ch/4], (0x14+(ch%4)), (0x90+value-100));
}

void nvp6124_write_table(unsigned char chip_addr, unsigned char addr, unsigned char *tbl_ptr, unsigned char tbl_cnt)
{
	unsigned char i = 0;
	
	for(i = 0; i < tbl_cnt; i ++)
	{
		I2CWriteByte(chip_addr, (addr + i), *(tbl_ptr + i));
	}
}

