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
#include <asm/io.h>

//#include "gpio_i2c.h"
#include "video.h"
#include "nvp6124.h"
#include "coax_protocol.h"

extern int rev_id[4];
extern unsigned int nvp6124_slave_addr[4];
extern unsigned char ch_mode_status[16];
extern unsigned int nvp6124_cnt;
extern unsigned int vloss;

//#define _EQ_ADJ_COLOR_
volatile unsigned char stage_update[16];

unsigned char ANALOG_EQ_1080P_BYPASS[8]  = {0x13,0x03,0x53,0x73,0x73,0x73,0x73,0x73};
unsigned char ANALOG_EQ_1080P_LPF30M[8]  = {0x11,0x01,0x51,0x71,0x71,0x71,0x71,0x71}; 
unsigned char DIGITAL_EQ_1080P[8] = {0x00,0x00,0x00,0x00,0x8B,0x8F,0x8F,0x8F};
#ifdef _EQ_ADJ_COLOR_
unsigned char BRI_EQ_1080P[8]    = {0xF4,0xF4,0xF4,0xF4,0xF8,0xF8,0xF8,0xF8};
unsigned char CON_EQ_1080P[8]    = {0x90,0x90,0x90,0x90,0x90,0x90,0x80,0x80};
unsigned char SAT_EQ_1080P[8]    = {0x80,0x80,0x80,0x78,0x78,0x78,0x78,0x78};
unsigned char BRI_EQ_720P[9]    = {0xF4,0xF4,0xF4,0xF4,0xF8,0xF8,0xF8,0xF8,0xF8};
unsigned char CON_EQ_720P[9]    = {0x90,0x90,0x90,0x90,0x88,0x88,0x84,0x90,0x90};
unsigned char SAT_EQ_720P[9]    = {0x84,0x84,0x84,0x80,0x80,0x80,0x80,0x84,0x84};
#endif
unsigned char SHARP_EQ_1080P[8]  = {0x90,0x90,0x99,0x99,0x99,0x99,0x99,0x90};
unsigned char PEAK_EQ_1080P[8]   = {0x00,0x10,0x00,0x00,0x00,0x00,0x50,0x00};
unsigned char CTI_EQ_1080P[8]    = {0x0A,0x0A,0x0A,0x0A,0x0A,0x0A,0x0A,0x0A};
unsigned char C_LOCK_EQ_1080P[8] = {0x92,0x92,0x92,0x92,0x92,0xA2,0xA2,0xA2};
unsigned char UGAIN_EQ_1080P[8]  = {0x00,0x00,0x00,0x00,0x10,0x10,0x20,0x00};
unsigned char VGAIN_EQ_1080P[8]  = {0x00,0x00,0x00,0x00,0x10,0x10,0x20,0x00};


unsigned char SHARP_EQ_720P[9]   =  {0x90,0x90,0x99,0x99,0x99,0x99,0x99,0x90,0x90};
unsigned char PEAK_EQ_720P[9]    =  {0x00,0x20,0x10,0x10,0x00,0x00,0x40,0x20,0x20};
unsigned char CTI_EQ_720P[9]     =  {0x0A,0x0A,0x0A,0x0A,0x0A,0x0A,0x0A,0x0A,0x0A};
unsigned char C_LOCK_EQ_720P[9]  =  {0x92,0x92,0x92,0x92,0x92,0x92,0xA2,0x92,0xA2};
unsigned char UGAIN_EQ_720P[9]   =  {0x30,0x30,0x30,0x30,0x30,0x30,0x40,0x30,0x30};
unsigned char VGAIN_EQ_720P[9]   =  {0x30,0x30,0x30,0x30,0x30,0x30,0x40,0x30,0x30};
unsigned char ANALOG_EQ_720P[9]  =  {0x13,0x03,0x53,0x73,0x73,0x73,0x73,0x03,0x13};
unsigned char DIGITAL_EQ_720P[9] =  {0x00,0x00,0x00,0x00,0x88,0x8F,0x8F,0x00,0x00}; 

unsigned char eq_stage[16]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
#ifdef _EQ_ADJ_COLOR_
void nvp6124_brightness_eq(unsigned int ch,  unsigned int stage)
{
	I2CWriteByte(nvp6124_slave_addr[ch/4], 0xFF, 0x00);
    if(ch_mode_status[ch] == NVP6124_VI_720P_2530)
		I2CWriteByte(nvp6124_slave_addr[ch/4],(0x0c+(ch%4)),BRI_EQ_720P[stage]);
	else if(ch_mode_status[ch] == NVP6124_VI_1080P_2530)
		I2CWriteByte(nvp6124_slave_addr[ch/4],(0x0c+(ch%4)),BRI_EQ_1080P[stage]);
}

void nvp6124_contrast_eq(unsigned int ch,  unsigned int stage)
{
	I2CWriteByte(nvp6124_slave_addr[ch/4], 0xFF, 0x00);
    if(ch_mode_status[ch] == NVP6124_VI_720P_2530)
		I2CWriteByte(nvp6124_slave_addr[ch/4], (0x10+(ch%4)),CON_EQ_720P[stage]);
	else if(ch_mode_status[ch] == NVP6124_VI_1080P_2530)
		I2CWriteByte(nvp6124_slave_addr[ch/4], (0x10+(ch%4)),CON_EQ_1080P[stage]);
}

void nvp6124_saturation_eq(unsigned int ch,  unsigned int stage)
{
	I2CWriteByte(nvp6124_slave_addr[ch/4], 0xFF, 0x00);
    if(ch_mode_status[ch] == NVP6124_VI_720P_2530)
		I2CWriteByte(nvp6124_slave_addr[ch/4], (0x3C+(ch%4)),SAT_EQ_720P[stage]);
	else if(ch_mode_status[ch] == NVP6124_VI_1080P_2530)
		I2CWriteByte(nvp6124_slave_addr[ch/4], (0x3C+(ch%4)),SAT_EQ_1080P[stage]);
}
#endif
void nvp6124_c_filter_eq(unsigned int ch,  unsigned int stage)
{
	I2CWriteByte(nvp6124_slave_addr[ch/4], 0xFF, 0x00);
    if(ch_mode_status[ch] == NVP6124_VI_720P_2530)
		I2CWriteByte(nvp6124_slave_addr[ch/4], (0x21+4*(ch%4)),C_LOCK_EQ_720P[stage]);
	else if(ch_mode_status[ch] == NVP6124_VI_1080P_2530)
		I2CWriteByte(nvp6124_slave_addr[ch/4], (0x21+4*(ch%4)),C_LOCK_EQ_1080P[stage]);
}


void nvp6124_sharpness_eq(unsigned int ch,  unsigned int stage)
{
	I2CWriteByte(nvp6124_slave_addr[ch/4], 0xFF, 0x00);
    if(ch_mode_status[ch] == NVP6124_VI_720P_2530)
		I2CWriteByte(nvp6124_slave_addr[ch/4], (0x14+(ch%4)),SHARP_EQ_720P[stage]);
	else if(ch_mode_status[ch] == NVP6124_VI_1080P_2530)
		I2CWriteByte(nvp6124_slave_addr[ch/4], (0x14+(ch%4)),SHARP_EQ_1080P[stage]);
}

void nvp6124_peaking_eq(unsigned int ch,  unsigned int stage)
{
	I2CWriteByte(nvp6124_slave_addr[ch/4], 0xFF, 0x00);
    if(ch_mode_status[ch] == NVP6124_VI_720P_2530)
		I2CWriteByte(nvp6124_slave_addr[ch/4], (0x18+(ch%4)),PEAK_EQ_720P[stage]);
	else if(ch_mode_status[ch] == NVP6124_VI_1080P_2530)
		I2CWriteByte(nvp6124_slave_addr[ch/4], (0x18+(ch%4)),PEAK_EQ_1080P[stage]);
}

void nvp6124_ctigain_eq(unsigned int ch,  unsigned int stage)
{
	I2CWriteByte(nvp6124_slave_addr[ch/4], 0xFF, 0x00);
    if(ch_mode_status[ch] == NVP6124_VI_720P_2530)
		I2CWriteByte(nvp6124_slave_addr[ch/4], (0x38+(ch%4)),CTI_EQ_720P[stage]);
	else if(ch_mode_status[ch] == NVP6124_VI_1080P_2530)
		I2CWriteByte(nvp6124_slave_addr[ch/4], (0x38+(ch%4)),CTI_EQ_1080P[stage]);
}

void nvp6124_ugain_eq(unsigned int ch,  unsigned int stage)
{
	I2CWriteByte(nvp6124_slave_addr[ch/4], 0xFF, 0x00);
    if(ch_mode_status[ch] == NVP6124_VI_720P_2530)
		I2CWriteByte(nvp6124_slave_addr[ch/4], (0x44+(ch%4)),UGAIN_EQ_720P[stage]);
	else if(ch_mode_status[ch] == NVP6124_VI_1080P_2530)
		I2CWriteByte(nvp6124_slave_addr[ch/4], (0x44+(ch%4)),UGAIN_EQ_1080P[stage]);
}

void nvp6124_vgain_eq(unsigned int ch,  unsigned int stage)
{
	I2CWriteByte(nvp6124_slave_addr[ch/4], 0xFF, 0x00);
    if(ch_mode_status[ch] == NVP6124_VI_720P_2530)
		I2CWriteByte(nvp6124_slave_addr[ch/4], (0x48+(ch%4)),VGAIN_EQ_720P[stage]);
	else if(ch_mode_status[ch] == NVP6124_VI_1080P_2530)
		I2CWriteByte(nvp6124_slave_addr[ch/4], (0x48+(ch%4)),VGAIN_EQ_1080P[stage]);
}
unsigned int get_ceq_stage(unsigned char resol, unsigned int acc_gain)
{
	unsigned char c_eq = 0;

	if(resol == NVP6124_VI_1080P_2530)
	{
		if	   (acc_gain >= 0x000 && acc_gain < 0x052 )   c_eq = 1;
		else if(acc_gain >= 0x052 && acc_gain < 0x089 )   c_eq = 2;
		else if(acc_gain >= 0x089 && acc_gain < 0x113 )   c_eq = 3;
		else if(acc_gain >= 0x113 && acc_gain < 0x25F )   c_eq = 4;
		else if(acc_gain >= 0x25F && acc_gain < 0x700 )   c_eq = 5;
		else if(acc_gain >= 0x700 && acc_gain < 0x7FF )   c_eq = 6;
		else											  c_eq = 7;  
	}
	else if(resol == NVP6124_VI_720P_2530)
	{
		if	   (acc_gain >= 0x000 && acc_gain < 0x055 )  c_eq = 1;
		else if(acc_gain >= 0x055 && acc_gain < 0x082 )  c_eq = 2;
		else if(acc_gain >= 0x082 && acc_gain < 0x0D8 )  c_eq = 3;
		else if(acc_gain >= 0x0D8 && acc_gain < 0x18F )  c_eq = 4;
		else if(acc_gain >= 0x18F && acc_gain < 0x700 )  c_eq = 5;
		else if(acc_gain >= 0x700 && acc_gain < 0x7FF )  c_eq = 6;
		else											 c_eq = 7;  
	}

	return c_eq;
}

unsigned int get_yeq_stage(unsigned char resol, unsigned int y_minus_slp)
{
	unsigned char y_eq = 0;

	if(resol == NVP6124_VI_1080P_2530)
	{
		if     (y_minus_slp == 0x000)						    y_eq = 0;
		else if(y_minus_slp >  0x000 && y_minus_slp < 0x0E7)    y_eq = 1;
		else if(y_minus_slp >= 0x0E7 && y_minus_slp < 0x11A)    y_eq = 2;
		else if(y_minus_slp >= 0x11A && y_minus_slp < 0x151)    y_eq = 3;
		else if(y_minus_slp >= 0x151 && y_minus_slp < 0x181)    y_eq = 4;
		else if(y_minus_slp >= 0x181 && y_minus_slp < 0x200)    y_eq = 5;
		else													y_eq = 6;
	}
	else if(resol == NVP6124_VI_720P_2530)
	{
		if     (y_minus_slp == 0x000)                           y_eq = 0;
		else if(y_minus_slp >  0x000 && y_minus_slp < 0x104)    y_eq = 1;
		else if(y_minus_slp >= 0x104 && y_minus_slp < 0x125)    y_eq = 2;
		else if(y_minus_slp >= 0x125 && y_minus_slp < 0x14C)    y_eq = 3;
		else if(y_minus_slp >= 0x14C && y_minus_slp < 0x16F)    y_eq = 4;
		else if(y_minus_slp >= 0x16F && y_minus_slp < 0x185)    y_eq = 5;
		else													y_eq = 6;
	}

	return y_eq;

}

unsigned int is_bypass_mode(unsigned int agc_val, unsigned int y_ref2_sts)
{
	if(((agc_val < 0x20)  && (y_ref2_sts >= 0x176))			||
	   ((agc_val >= 0x20) && (y_ref2_sts >= 0x1B0)))          
				return 1;
	return 0;
	}
#define ACP_CLR_CNT			1
#define ACP_SET_CNT			(ACP_CLR_CNT+1)
#define ACP_READ_START_CNT	(ACP_SET_CNT+3)
#define RETRY_CNT			5
#define COMPARE_NUM			3
#define COUNT_NUM			4
#define LOOP_BUF_SIZE		20
unsigned char check_c_stage[16][LOOP_BUF_SIZE];
unsigned char check_y_stage[16][LOOP_BUF_SIZE];
unsigned char acp_val[16][LOOP_BUF_SIZE];
unsigned char acp_ptn[16][LOOP_BUF_SIZE];
unsigned char vidmode_back[16];
unsigned char video_on[16];
unsigned char eq_loop_cnt[16];
unsigned char loop_cnt[16];
unsigned char bypass_flag_retry[16];
unsigned char ystage_flag_retry[16];
unsigned char cstage_flag_retry[16];
unsigned char one_setting[16]={0,};
unsigned char bypass_flag[16]={0,};
unsigned char acp_isp_wr_en[16]={0,};
static void nvp6124_sw_reset(unsigned char ch)
{
	unsigned char tmp;
	I2CWriteByte(nvp6124_slave_addr[ch/4], 0xFF, 0x01);
	tmp = I2CReadByte( nvp6124_slave_addr[ch/4], 0x97) & 0x0F;
	tmp = tmp & (~(1<<(ch%4)));
	I2CWriteByte(nvp6124_slave_addr[ch/4], 0x97, tmp);
	I2CWriteByte(nvp6124_slave_addr[ch/4], 0x97, 0x0F);
	printk("software reset OK\n");
}
void lossfakechannel(unsigned char ch, unsigned char bypassflag)
{
	unsigned char tmp;
	I2CWriteByte(nvp6124_slave_addr[ch/4], 0xFF, 0x00);
	tmp = I2CReadByte(nvp6124_slave_addr[ch/4], 0x7A+(ch<2?0x00:0x01));
	if(ch%2 == 0)
		tmp = tmp & 0xF0;
	else
		tmp = tmp & 0x0F;
	if(bypassflag)
		tmp = tmp | (0x0F << (ch%2)*4);
	else
		tmp = tmp | (0x01 << (ch%2)*4);
	I2CWriteByte(nvp6124_slave_addr[ch/4], 0x7A+(ch<2?0x00:0x01),tmp);
}
void bubble_sort(unsigned char *buf ,unsigned char len)
{
	int i,j,tmp;
	for(i = 0; i < len; i++)
	{
		for(j = 0; j < len - 1 ; j++)
		{
			if(buf[j] > buf[j+1])
			{
				tmp = buf[j];
				buf[j] = buf[j+1];
				buf[j+1] = tmp;
			}
		}
	}
}
unsigned char calc_stage_gap(unsigned char *buf)
{
	unsigned char cnt,zero_cnt=0;
	unsigned char tmpbuf[LOOP_BUF_SIZE];
	memcpy(tmpbuf,buf,LOOP_BUF_SIZE);
	bubble_sort(tmpbuf, LOOP_BUF_SIZE);
	for(cnt = 0; cnt < LOOP_BUF_SIZE; cnt++)
	{
		if(tmpbuf[cnt] == 0) zero_cnt++;
	}
	return(abs(tmpbuf[LOOP_BUF_SIZE-1] - tmpbuf[zero_cnt]));
}
unsigned char calc_arr_mean(unsigned char *buf)
{
	unsigned char cnt,tmp=0,zero_cnt=0;
	unsigned char tmpbuf[LOOP_BUF_SIZE];
	memcpy(tmpbuf,buf,LOOP_BUF_SIZE);
	bubble_sort(tmpbuf, LOOP_BUF_SIZE);
	for(cnt = 0; cnt < LOOP_BUF_SIZE; cnt++)
	{
		if(tmpbuf[cnt] == 0) zero_cnt++;
		tmp = tmp + tmpbuf[cnt];
	}
	tmp = abs( tmp / (LOOP_BUF_SIZE-zero_cnt));
	for(cnt = 0; cnt <LOOP_BUF_SIZE; cnt++)
		printk("cnt[%d]  origin = %x   sort= %x\n",cnt, buf[cnt], tmpbuf[cnt]);
	printk("mean value ========================== %d\n",tmp);
	return tmp;
	}

unsigned char compare_arr(unsigned char cur_ptr, unsigned char *buf)
{
	unsigned char start_ptr;
	if(cur_ptr == 1)		start_ptr = LOOP_BUF_SIZE - 1;
	else if(cur_ptr == 0)	start_ptr = LOOP_BUF_SIZE - 2;
	else					start_ptr = cur_ptr - 2;
	if((buf[start_ptr] == buf[(start_ptr+1)%LOOP_BUF_SIZE])						&&
       (buf[(start_ptr+1)%LOOP_BUF_SIZE] == buf[(start_ptr+2)%LOOP_BUF_SIZE]))
		return buf[start_ptr];
	else
		return 0xFF;
}

unsigned char decide_noptn_stage(unsigned char agcval)
{
	unsigned char stage;
	if	   (agcval <= 0x18)	stage = 1;
	else if(agcval <= 0x1A)	stage = 2;
	else if(agcval <= 0x1C)	stage = 3;
	else if(agcval <= 0x1E)	stage = 4;
	else if(agcval <= 0x20)	stage = 5;
	else					stage = 6;
	return stage;
}
void init_eq_stage(unsigned char ch)
{
	unsigned char default_stage=0;
	if(ch_mode_status[ch] == NVP6124_VI_720P_2530)
	{
		I2CWriteByte(nvp6124_slave_addr[ch/4], 0xFF,0x05+ch%4);
		I2CWriteByte(nvp6124_slave_addr[ch/4], 0x58, ANALOG_EQ_720P[default_stage]);
		I2CWriteByte(nvp6124_slave_addr[ch/4], 0xFF,((ch%4)<2)?0x0A:0x0B);         
		I2CWriteByte(nvp6124_slave_addr[ch/4], (ch%2==0)?0x3B:0xBB, DIGITAL_EQ_720P[default_stage]);
	}
	else
	{
		I2CWriteByte(nvp6124_slave_addr[ch/4], 0xFF,0x05+ch%4);
		if(rev_id[ch/4] == 0x20)
			I2CWriteByte(nvp6124_slave_addr[ch/4], 0x58, ANALOG_EQ_1080P_BYPASS[default_stage]);
		else
			I2CWriteByte(nvp6124_slave_addr[ch/4], 0x58, ANALOG_EQ_1080P_LPF30M[default_stage]);
		I2CWriteByte(nvp6124_slave_addr[ch/4], 0xFF,((ch%4)<2)?0x0A:0x0B);         
		I2CWriteByte(nvp6124_slave_addr[ch/4], (ch%2==0)?0x3B:0xBB, DIGITAL_EQ_1080P[default_stage]);
	}
	memset(check_y_stage[ch],0x00,sizeof(check_y_stage[ch]));
}
void nvp6124_set_equalizer(void)
{
	unsigned char i,tmp_acp_val,tmp_acp_ptn,tmp_y_stage,tmp_c_stage;
	unsigned char ch;
	unsigned char vidmode[16];
	unsigned char agc_lock;
	unsigned char agc_val[16];
	unsigned int  acc_gain_sts[16];
	unsigned int  y_ref_status[16];
	unsigned int  y_ref2_status[16];
	unsigned char stage_gap;

	for(ch=0;ch<nvp6124_cnt*4;ch++)
	{
		I2CWriteByte(nvp6124_slave_addr[ch/4], 0xFF, 0x09);
		I2CWriteByte(nvp6124_slave_addr[ch/4], 0x61, ch%4);
		I2CWriteByte(nvp6124_slave_addr[ch/4], 0xFF, 0x00);
		agc_val[ch] = I2CReadByte(nvp6124_slave_addr[ch/4],0xF7);
		I2CWriteByte(nvp6124_slave_addr[ch/4], 0xFF, 0x00);
		vidmode[ch] = I2CReadByte(nvp6124_slave_addr[ch/4],0xD0+ch%4);

		agc_lock = I2CReadByte(nvp6124_slave_addr[ch/4],0xEC);

		I2CWriteByte(nvp6124_slave_addr[ch/4], 0xFF, 0x05 + ch%4);
		acc_gain_sts[ch] = I2CReadByte(nvp6124_slave_addr[ch/4],0xE2)&0x07;
		acc_gain_sts[ch] <<= 8;
		acc_gain_sts[ch] |= I2CReadByte(nvp6124_slave_addr[ch/4],0xE3);
		
		y_ref_status[ch] = I2CReadByte(nvp6124_slave_addr[ch/4],0xEA)&0x07;
		y_ref_status[ch] <<= 8;
		y_ref_status[ch] |= I2CReadByte(nvp6124_slave_addr[ch/4],0xEB);

		y_ref2_status[ch] = I2CReadByte(nvp6124_slave_addr[ch/4],0xE8)&0x07;
		y_ref2_status[ch] <<= 8;
		y_ref2_status[ch] |= I2CReadByte(nvp6124_slave_addr[ch/4],0xE9);

		if(vidmode[ch] >= 4)
		{
			if((((vloss>>ch)&0x01) == 0x00) && (((agc_lock>>(ch%4))&0x01) == 0x01))
				video_on[ch] = 1;
			else
				video_on[ch] = 0;
		}
		else
		{
			video_on[ch] = 0;
		}

		if((ch_mode_status[ch] == NVP6124_VI_1080P_2530) || (ch_mode_status[ch] == NVP6124_VI_720P_2530))
		{
			if(video_on[ch])
			{
				if(loop_cnt[ch] != 0xFF)
				{
					loop_cnt[ch]++;
					if(loop_cnt[ch] > 200) loop_cnt[ch] = ACP_READ_START_CNT;
					if(loop_cnt[ch] == ACP_CLR_CNT)	acp_reg_rx_clear(ch); 
					else							acp_each_setting(ch); 
					eq_loop_cnt[ch]++;
					eq_loop_cnt[ch] %= LOOP_BUF_SIZE;
					check_c_stage[ch][eq_loop_cnt[ch]] = get_ceq_stage(ch_mode_status[ch], acc_gain_sts[ch]);
					check_y_stage[ch][eq_loop_cnt[ch]] = get_yeq_stage(ch_mode_status[ch], y_ref_status[ch]);
					acp_val[ch][eq_loop_cnt[ch]] = read_acp_status(ch);
					acp_ptn[ch][eq_loop_cnt[ch]] = read_acp_pattern(ch);
				}

				if(ch_mode_status[ch] == NVP6124_VI_720P_2530)
				{
					if((loop_cnt[ch] != 0xFF) && (loop_cnt[ch] >= ACP_READ_START_CNT))
					{
						if(one_setting[ch] == 0)
					{
							bypass_flag[ch] = 0xFF;
							tmp_acp_val = compare_arr(eq_loop_cnt[ch],acp_val[ch]);
							tmp_acp_ptn = compare_arr(eq_loop_cnt[ch],acp_ptn[ch]);
							if(ch == 0)
						{
								printk("acp_val = %x   acp_ptn = %x\n", acp_val[ch][eq_loop_cnt[ch]], acp_ptn[ch][eq_loop_cnt[ch]]);
								printk("tmp_acp_val = %x   tmp_acp_ptn = %x\n", tmp_acp_val, tmp_acp_ptn);
							}
							if((tmp_acp_val == 0x55) && (tmp_acp_ptn == 0x01))
							{
								bypass_flag[ch] = 0;
								one_setting[ch] = 1;
						}
						else
						{
								bypass_flag_retry[ch]++;
								bypass_flag_retry[ch] %= 200;
								if(bypass_flag_retry[ch] == RETRY_CNT)
								{
									bypass_flag[ch] = 1;
									one_setting[ch] = 1;
								}
							}
						}

						if(bypass_flag[ch] == 1)
						{
							stage_update[ch] = 1;
						}
						else if(bypass_flag[ch] == 0)
						{
							if(ch == 0)
							{
								printk("loop_cnt = %d\n",loop_cnt[ch]);
								printk("check_y_stage = %d \n",check_y_stage[ch][eq_loop_cnt[ch]]);
								printk("check_c_stage = %d \n",check_c_stage[ch][eq_loop_cnt[ch]]);
							}
							if( loop_cnt[ch] == (ACP_READ_START_CNT+bypass_flag_retry[ch]))
						{
								init_eq_stage(ch); 
								if(ch == 0)
								{
									printk("720p init_eq_stage\n");
								}
							}
							else if(loop_cnt[ch] >= (ACP_READ_START_CNT+bypass_flag_retry[ch]+3))
							{
								tmp_y_stage = compare_arr(eq_loop_cnt[ch],check_y_stage[ch]);
								tmp_c_stage = compare_arr(eq_loop_cnt[ch],check_c_stage[ch]);
								if(ch == 0)
								{
									printk("tmp_y_stage = %d\n",tmp_y_stage);
									printk("tmp_c_stage = %d\n",tmp_c_stage);
						}
								if((tmp_c_stage != 0xFF) && (tmp_y_stage != 0xFF))
									stage_update[ch] = 1;
						else
						{
									if(tmp_y_stage == 0xFF)
									{
										ystage_flag_retry[ch]++;
										ystage_flag_retry[ch] %= 200;
						}
									if(tmp_c_stage == 0xFF)
						{
										cstage_flag_retry[ch]++;
										cstage_flag_retry[ch] %= 200;
						}
									if((ystage_flag_retry[ch]==RETRY_CNT)||(cstage_flag_retry[ch]==RETRY_CNT))	
									{
										stage_update[ch] = 1;
										check_y_stage[ch][eq_loop_cnt[ch]] = calc_arr_mean(check_y_stage[ch]);
					}
									else
									{
									stage_gap = abs(decide_noptn_stage(agc_val[ch]) - check_y_stage[ch][eq_loop_cnt[ch]]);
									printk("agc_val = %d y_stage = %d\n",decide_noptn_stage(agc_val[ch]),check_y_stage[ch][eq_loop_cnt[ch]]);
									if(stage_gap >= 3)
										printk("GAP IS TOO BIG\n");
									if((calc_stage_gap(check_y_stage[ch]) >= 2) || (stage_gap >= 3))
									{
										init_eq_stage(ch); 
										nvp6124_sw_reset(ch);
									}
									}
								}
							}
						}
					else
						{
						stage_update[ch] = 0;
						}
						if(bypass_flag[ch] == 1)
						{
							check_y_stage[ch][eq_loop_cnt[ch]] = 7;
							if(((check_c_stage[ch][eq_loop_cnt[ch]] == 5)   && y_ref2_status[ch] >= 0x1A0) ||
				   				(acc_gain_sts[ch] == 0x7FF && y_ref2_status[ch] >= 0x1B0))
				   				check_y_stage[ch][eq_loop_cnt[ch]] = 8;
						}
					}
				}
				else
				{
					printk("1080P CH%d  acp_val=%x  acp_ptn=%x\n",ch, acp_val[ch][eq_loop_cnt[ch]],acp_ptn[ch][eq_loop_cnt[ch]]);
					if((loop_cnt[ch] != 0xFF) && (loop_cnt[ch] >= ACP_READ_START_CNT))
					{
						if(one_setting[ch] == 0)
					{
							bypass_flag[ch] = 0xFF;
							tmp_acp_val = compare_arr(eq_loop_cnt[ch],acp_val[ch]);
							tmp_acp_ptn = compare_arr(eq_loop_cnt[ch],acp_ptn[ch]);
							if((tmp_acp_val == 0x55) && (tmp_acp_ptn == 0x01))
						{
								bypass_flag[ch] = 0;
								one_setting[ch] = 1;
						}
						else
						{
								bypass_flag_retry[ch]++;
								bypass_flag_retry[ch] %= 200;
								if(bypass_flag_retry[ch] == RETRY_CNT)
								{
									bypass_flag[ch] = 1;
									one_setting[ch] = 1;
									lossfakechannel(ch, bypass_flag[ch]);
						}
					}
						}

						if(bypass_flag[ch] == 1)
						{
						stage_update[ch] = 1;
						}
						else if(bypass_flag[ch] == 0)
						{
                            if(ch == 0)
	                        {
		                        printk("1080p loop_cnt = %d\n",loop_cnt[ch]);
		                        printk("1080p check_y_stage = %d \n",check_y_stage[ch][eq_loop_cnt[ch]]);
		                        printk("1080p check_c_stage = %d \n",check_c_stage[ch][eq_loop_cnt[ch]]);
	                        }
							if( loop_cnt[ch] == (ACP_READ_START_CNT+bypass_flag_retry[ch]))
							{
								init_eq_stage(ch); 
								if(ch == 0)
								{
									printk("1080p init_eq_stage\n");
								}
							}
							else if(loop_cnt[ch] >= (ACP_READ_START_CNT+bypass_flag_retry[ch]+3))
							{
								tmp_y_stage = compare_arr(eq_loop_cnt[ch],check_y_stage[ch]);
								if(tmp_y_stage != 0xFF)
								{
									stage_update[ch] = 1;
								}
					else
								{
									if(tmp_y_stage == 0xFF)
									{
										ystage_flag_retry[ch]++;
										ystage_flag_retry[ch] %= 200;
									}
									if(ystage_flag_retry[ch]==RETRY_CNT)
									{
										stage_update[ch] = 1;
										check_y_stage[ch][eq_loop_cnt[ch]] = calc_arr_mean(check_y_stage[ch]);
									}
									else
									{
									stage_gap = abs(decide_noptn_stage(agc_val[ch]) - check_y_stage[ch][eq_loop_cnt[ch]]);
									printk("1080p agc_val=%d y_stage=%d\n",decide_noptn_stage(agc_val[ch]),check_y_stage[ch][eq_loop_cnt[ch]]);
									if(stage_gap >= 3)
										printk("GAP IS TOO BIG\n");
									if((calc_stage_gap(check_y_stage[ch]) >= 2) || (stage_gap >= 3))
									{
										init_eq_stage(ch); 
										nvp6124_sw_reset(ch);
										}
									}
								}
							}
						}
					}
				}
			}
			else
			{
				if(vidmode_back[ch] >= 0x04) 
				{
					stage_update[ch] = 1;
				}
				else
					stage_update[ch] = 0;

				for(i=0;i<LOOP_BUF_SIZE;i++)
				{
					check_c_stage[ch][i]=0;
					check_y_stage[ch][i]=0;
					acp_val[ch][i] = 0;
					acp_ptn[ch][i] = 0;
				}
				eq_loop_cnt[ch]=0;
				check_y_stage[ch][eq_loop_cnt[ch]] = 1; 
				loop_cnt[ch] =0;
				bypass_flag_retry[ch]=0;
				ystage_flag_retry[ch]=0;
				cstage_flag_retry[ch]=0;
				one_setting[ch] = 0;
				acp_isp_wr_en[ch] = 0;
				lossfakechannel(ch, 0);
			}

			if(stage_update[ch])
			{
				stage_update[ch] = 0;
				acp_isp_wr_en[ch] = 1;
				eq_stage[ch] = check_y_stage[ch][eq_loop_cnt[ch]];
				if(video_on[ch]) loop_cnt[ch] = 0xFF;

				if(ch_mode_status[ch] == NVP6124_VI_720P_2530)
				{
					if(video_on[ch] == 0)
						init_eq_stage(ch);
					else
					{
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0xFF,0x05+ch%4);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0x58, ANALOG_EQ_720P[eq_stage[ch]]);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0xFF,((ch%4)<2)?0x0A:0x0B);         
					I2CWriteByte(nvp6124_slave_addr[ch/4], (ch%2==0)?0x3B:0xBB, DIGITAL_EQ_720P[eq_stage[ch]]);
					}
				}
				else
				{
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0xFF,0x05+ch%4);
					if(rev_id[ch/4] == 0x20)
						I2CWriteByte(nvp6124_slave_addr[ch/4], 0x58, ANALOG_EQ_1080P_BYPASS[eq_stage[ch]]);
					else
						I2CWriteByte(nvp6124_slave_addr[ch/4], 0x58, ANALOG_EQ_1080P_LPF30M[eq_stage[ch]]);
					I2CWriteByte(nvp6124_slave_addr[ch/4], 0xFF,((ch%4)<2)?0x0A:0x0B);         
					I2CWriteByte(nvp6124_slave_addr[ch/4], (ch%2==0)?0x3B:0xBB, DIGITAL_EQ_1080P[eq_stage[ch]]);
				}
				printk("CH[%d]-Stage update : eq_stage = %d\n",ch,eq_stage[ch]);
				if(eq_stage[ch] != 7)
				{
				#ifdef _EQ_ADJ_COLOR_
				nvp6124_brightness_eq(ch, eq_stage[ch]);
				nvp6124_contrast_eq(ch, eq_stage[ch]);
				nvp6124_saturation_eq(ch, eq_stage[ch]);
				#endif
				nvp6124_sharpness_eq(ch, eq_stage[ch]);
				nvp6124_peaking_eq(ch, eq_stage[ch]);
				nvp6124_ctigain_eq(ch, eq_stage[ch]);
				nvp6124_c_filter_eq(ch, eq_stage[ch]);
				nvp6124_ugain_eq(ch, eq_stage[ch]);
				nvp6124_vgain_eq(ch, eq_stage[ch]);
			}
			}
			vidmode_back[ch] = vidmode[ch];
		}
	}
}

