/*
 * tp2802.c
 */
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/fcntl.h>
#include <linux/mm.h>
#ifndef CONFIG_HISI_SNAPSHOT_BOOT
#include <linux/miscdevice.h>
#endif

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
#include <linux/poll.h>
#include <linux/kthread.h>
#include <linux/sched.h>

#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include "tp2823_def.h"
#include "tp2823.h"

#ifdef CONFIG_HISI_SNAPSHOT_BOOT
#include "himedia.h"
#define DEV_NAME "tp2823"
static struct himedia_device s_stTp2823Device;
#endif


static struct i2c_board_info hi_info =
{
    I2C_BOARD_INFO("tp2823", 0x88),
};

static struct i2c_client* tp2823_client;

#define MUXCTRL_BASE        0x200F0000
#define HW_REG(reg)         *((volatile unsigned int *)(reg))
#define MUXCTRL_REG(offset) IO_ADDRESS(MUXCTRL_BASE + (offset))


MODULE_AUTHOR("Jay Guillory <jayg@techpointinc.com>");
MODULE_DESCRIPTION("TechPoint TP2802 Linux Module");
MODULE_LICENSE("GPL");


enum
{
    TP2802C = 0x0200,
    TP2802D = 0x0201,
    TP2804 = 0x0400,
    TP2806 = 0x0401,
    TP2822 = 0x2200,
    TP2823 = 0x2300
};

//TP2823 audio
//both record and playback are master, 16ch,I2S mode.

//#define SAMPLE_16K  //if no define, default 8K
#define DATA_16BIT  //if no define, default 8BIT

//TP2802D EQ for short cable option
#define TP2802D_EQ_SHORT 0x0d
#define TP2802D_CGAIN_SHORT 0x74

//#define TP2802D_EQ_SHORT 0x01
//#define TP2802D_CGAIN_SHORT 0x70

#define BT1120_HEADER_8BIT   0x00 //reg0x02 bit3 0=BT1120,
#define BT656_HEADER_8BIT   0x08 //reg0x02 bit3 1=656,
#define SAV_HEADER          BT1120_HEADER_8BIT

static int mode = TP2802_720P25;
static int chips = 4;
static int output = MUX_1CH;
static int id[MAX_CHIPS];

#define TP2802A_I2C_ADDR 	0x88
#define TP2802B_I2C_ADDR 	0x8A
#define TP2802C_I2C_ADDR 	0x8C
#define TP2802D_I2C_ADDR 	0x8E



void tp28xx_byte_write(unsigned char chip_addr, unsigned char reg_addr, unsigned char value)
{
    int ret;
    unsigned char buf[2];
    struct i2c_client* client = tp2823_client;
    
    tp2823_client->addr = chip_addr;

    buf[0] = reg_addr;
    buf[1] = value;

    ret = i2c_master_send(client, buf, 2);
    //return ret;
}

unsigned char tp28xx_byte_read(unsigned char chip_addr, unsigned char reg_addr)
{
    int ret_data = 0xFF;
    int ret;
    struct i2c_client* client = tp2823_client;
    unsigned char buf[2];

    tp2823_client->addr = chip_addr;

    buf[0] = reg_addr;
    ret = i2c_master_recv(client, buf, 1);
    if (ret >= 0)
    {
        ret_data = buf[0];
    }
    return ret_data;
}


unsigned char tp2802_i2c_addr[] = { TP2802A_I2C_ADDR,
                                    TP2802B_I2C_ADDR,
                                    TP2802C_I2C_ADDR,
                                    TP2802D_I2C_ADDR
                                  };


#define TP2802_I2C_ADDR(chip_id)    (tp2802_i2c_addr[chip_id])

#define     SCAN_ENABLE     1
#define     SCAN_DISABLE    0
#define     MAX_COUNT       0xffff
typedef struct
{
    unsigned int			count[CHANNELS_PER_CHIP];
    unsigned int				  mode[CHANNELS_PER_CHIP];
    unsigned int               scan[CHANNELS_PER_CHIP];
    unsigned int				  gain[CHANNELS_PER_CHIP][4];
    //unsigned int               std[CHANNELS_PER_CHIP];
    unsigned int                 state[CHANNELS_PER_CHIP];
    unsigned int                 force[CHANNELS_PER_CHIP];
    unsigned char addr;
} tp2802wd_info;

static const unsigned char SYS_MODE[4] = {0x01, 0x02, 0x04, 0x08};
static const unsigned char SYS_AND[4] = {0xfe, 0xfd, 0xfb, 0xf7};
static const unsigned char CLK_MODE[4] = {0x01, 0x10, 0x01, 0x10};
static const unsigned char CLK_ADDR[4] = {0xfa, 0xfa, 0xfb, 0xfb};
static const unsigned char CLK_AND[4] = {0xf8, 0x8f, 0xf8, 0x8f};


static tp2802wd_info watchdog_info[MAX_CHIPS];
volatile static unsigned int watchdog_state = 0;
struct task_struct* task_watchdog_deamon = NULL;

static DEFINE_SPINLOCK(watchdog_lock);
#define WATCHDOG_EXIT    0
#define WATCHDOG_RUNNING 1
#define WDT              1
int  TP2802_watchdog_init(void);
void TP2802_watchdog_exit(void);
static void TP2822_PTZ_mode(unsigned char, unsigned char);



#if 0
unsigned char tp28xx_byte_write(unsigned char chip_addr,
										     unsigned char addr     , 
										     unsigned char data     ) 
{
#ifndef HI_FPGA
    #ifdef HI_GPIO_I2C
    gpio_i2c_write(chip_addr, addr, data);
    #else
    HI_I2C_Write(chip_addr, addr, 1, data, 1);
    udelay(1000);  //HI_I2C_Write¨¬??¨¬
    #endif
    
#else
    if (TW2865A_I2C_ADDR == chip_addr || TW2865B_I2C_ADDR == chip_addr)
    {
        gpio_i2c_write(chip_addr,addr,data);
    }
    else
    {
        gpio_i2c1_write(chip_addr,addr,data);
    }
#endif	

	return 0;
}


unsigned char tp28xx_byte_read(unsigned char chip_addr, unsigned char addr)
{	
#ifndef HI_FPGA
    #ifdef HI_GPIO_I2C
    return gpio_i2c_read(chip_addr, addr);
    #else
    return (HI_I2C_Read(chip_addr, addr, 1, 1) & 0xff);
    #endif
    
#else
    if (TW2865A_I2C_ADDR == chip_addr || TW2865B_I2C_ADDR == chip_addr)
    {
        return gpio_i2c_read(chip_addr,addr);
    }
    else
    {
        return gpio_i2c1_read(chip_addr,addr);
    }
#endif	
}

#endif

EXPORT_SYMBOL(tp28xx_byte_write);

EXPORT_SYMBOL(tp28xx_byte_read);

static void tp2802_write_table(unsigned char chip_addr,
                               unsigned char addr, unsigned char* tbl_ptr, unsigned char tbl_cnt)
{
    unsigned char i = 0;

    for (i = 0; i < tbl_cnt; i ++)
    {
        tp28xx_byte_write(chip_addr, (addr + i), *(tbl_ptr + i));
    }
}

#if 0

static void tp2802_read_table(unsigned char chip_addr,
                              unsigned char addr, unsigned char reg_num)
{
    unsigned char i = 0, temp = 0;

    for (i = 0; i < reg_num; i++ )
    {
        temp = tp28xx_byte_read(chip_addr, addr + i);
        printk("reg 0x%02x=0x%02x,", addr + i, temp);

        if (((i + 1) % 4) == 0)
        {
            printk("\n");
        }
    }
}
#endif


int tp2802_open(struct inode* inode, struct file* file)
{
    return SUCCESS;
}

int tp2802_close(struct inode* inode, struct file* file)
{
    return SUCCESS;
}

static void tp2802_set_work_mode_1080p25(unsigned chip_addr)
{
    // Start address 0x15, Size = 9B
    tp2802_write_table(chip_addr, 0x15, tbl_tp2802_1080p25_raster, 9);
}

static void tp2802_set_work_mode_1080p30(unsigned chip_addr)
{
    // Start address 0x15, Size = 9B
    tp2802_write_table(chip_addr, 0x15, tbl_tp2802_1080p30_raster, 9);
}

static void tp2802_set_work_mode_720p25(unsigned chip_addr)
{
    // Start address 0x15, Size = 9B
    tp2802_write_table(chip_addr, 0x15, tbl_tp2802_720p25_raster, 9);
}

static void tp2802_set_work_mode_720p30(unsigned chip_addr)
{
    // Start address 0x15, Size = 9B
    tp2802_write_table(chip_addr, 0x15, tbl_tp2802_720p30_raster, 9);
}

static void tp2802_set_work_mode_720p50(unsigned chip_addr)
{
    // Start address 0x15, Size = 9B
    tp2802_write_table(chip_addr, 0x15, tbl_tp2802_720p50_raster, 9);
}

static void tp2802_set_work_mode_720p60(unsigned chip_addr)
{
    // Start address 0x15, Size = 9B
    tp2802_write_table(chip_addr, 0x15, tbl_tp2802_720p60_raster, 9);
}

static int tp2823_audio_config_rmpos(unsigned i2c_addr, unsigned format, unsigned chn_num)
{
	int i = 0;

	//clear first
	for (i=0; i<20; i++)
	{
		tp28xx_byte_write(i2c_addr, i, 0);
	}

	switch(chn_num)
	{
		case 2:
			if (format)
			{
				tp28xx_byte_write(i2c_addr, 0x0, 1);
				tp28xx_byte_write(i2c_addr, 0x1, 2);
			}
			else
			{
				tp28xx_byte_write(i2c_addr, 0x0, 1);
				tp28xx_byte_write(i2c_addr, 0x8, 2);
			}
			
			break;
		case 4:
			if (format)
			{
				tp28xx_byte_write(i2c_addr, 0x0, 1);
				tp28xx_byte_write(i2c_addr, 0x1, 2);
				tp28xx_byte_write(i2c_addr, 0x2, 3);
				tp28xx_byte_write(i2c_addr, 0x3, 3);/**/
			}
			else
			{
				tp28xx_byte_write(i2c_addr, 0x0, 1);
				tp28xx_byte_write(i2c_addr, 0x1, 2);
				tp28xx_byte_write(i2c_addr, 0x8, 3);
				tp28xx_byte_write(i2c_addr, 0x9, 3);/**/
			}
			
			break;

		case 8:
			if (TP2802A_I2C_ADDR == i2c_addr)
			{
				if (format)
				{
					tp28xx_byte_write(i2c_addr, 0x0, 1);
					tp28xx_byte_write(i2c_addr, 0x1, 2);
					tp28xx_byte_write(i2c_addr, 0x2, 3);
					tp28xx_byte_write(i2c_addr, 0x3, 3);/**/
					tp28xx_byte_write(i2c_addr, 0x4, 5);
					tp28xx_byte_write(i2c_addr, 0x5, 6);
					tp28xx_byte_write(i2c_addr, 0x6, 7);
					tp28xx_byte_write(i2c_addr, 0x7, 3);/**/
				}
				else
				{
					tp28xx_byte_write(i2c_addr, 0x0, 1);
					tp28xx_byte_write(i2c_addr, 0x1, 2);
					tp28xx_byte_write(i2c_addr, 0x2, 3);
					tp28xx_byte_write(i2c_addr, 0x3, 3);/**/
					tp28xx_byte_write(i2c_addr, 0x8, 5);
					tp28xx_byte_write(i2c_addr, 0x9, 6);
					tp28xx_byte_write(i2c_addr, 0xa, 7);
					tp28xx_byte_write(i2c_addr, 0xb, 3);/**/
				}
			}
			else if (TP2802B_I2C_ADDR == i2c_addr)
			{
				if (format)
				{
					tp28xx_byte_write(i2c_addr, 0x0, 0);
					tp28xx_byte_write(i2c_addr, 0x1, 0);
					tp28xx_byte_write(i2c_addr, 0x2, 0);
					tp28xx_byte_write(i2c_addr, 0x3, 0);
					tp28xx_byte_write(i2c_addr, 0x4, 1);
					tp28xx_byte_write(i2c_addr, 0x5, 2);
					tp28xx_byte_write(i2c_addr, 0x6, 3);
					tp28xx_byte_write(i2c_addr, 0x7, 3);/**/
				}
				else
				{
					tp28xx_byte_write(i2c_addr, 0x0, 0);
					tp28xx_byte_write(i2c_addr, 0x1, 0);
					tp28xx_byte_write(i2c_addr, 0x2, 1);
					tp28xx_byte_write(i2c_addr, 0x3, 2);
					tp28xx_byte_write(i2c_addr, 0x8, 0);
					tp28xx_byte_write(i2c_addr, 0x9, 0);
					tp28xx_byte_write(i2c_addr, 0xa, 3);
					tp28xx_byte_write(i2c_addr, 0xb, 3);/**/
				}
			}
			
			
			break;

		case 16:
			if (TP2802A_I2C_ADDR == i2c_addr)
			{
				for (i=0; i<16; i++)
				{
					if ((i == 3) || (i == 7) || (i == 11) || (i == 15))
					{
						tp28xx_byte_write(i2c_addr, i, 3);/**/
					}
					else
					{
						tp28xx_byte_write(i2c_addr, i, i+1);
					}
					
				}
			}
			else if (TP2802B_I2C_ADDR == i2c_addr)
			{
				for (i=4; i<16; i++)
				{
					if (i == 7)
					{
						tp28xx_byte_write(i2c_addr, i, 3);/**/
					}
					else
					{
						tp28xx_byte_write(i2c_addr, i, i+1 -4);
					}					
				}
			}	
			else if	(TP2802C_I2C_ADDR == i2c_addr)
			{
				for (i=8; i<16; i++)
				{
					if (i == 11)
					{
						tp28xx_byte_write(i2c_addr, i, 3);/**/
					}
					else
					{
						tp28xx_byte_write(i2c_addr, i, i+1 - 8);
					}
					
				}
			}
			else
			{
				for (i=12; i<16; i++)
				{
					if (i == 15)
					{
						tp28xx_byte_write(i2c_addr, i, 3);/**/
					}
					else
					{
						tp28xx_byte_write(i2c_addr, i, i+1 - 12);
					}
					
				}
			}
			break;

		case 20:
			for (i=0; i<20; i++)
			{
				tp28xx_byte_write(i2c_addr, i, i+1);
			}
			break;

		default:
			for (i=0; i<20; i++)
			{
				tp28xx_byte_write(i2c_addr, i, i+1);
			}
			break;
	}

	mdelay(10);
    return 0;
}

static int tp2823_set_audio_rm_format(unsigned i2c_addr, tp2823_audio_format *pstFormat)
{
	unsigned char temp = 0;

	if (pstFormat->mode > 1)
	{
		return FAILURE;
	}

	if (pstFormat->format> 1)
	{
		return FAILURE;
	}

	if (pstFormat->bitrate> 1)
	{
		return FAILURE;
	}

	if (pstFormat->clkdir> 1)
	{
		return FAILURE;
	}

	if (pstFormat->precision > 1)
	{
		return FAILURE;
	}

	temp = tp28xx_byte_read(i2c_addr, 0x17);
	temp &= 0xf2;
	temp |= pstFormat->format;
	temp |= pstFormat->precision << 2;
	/*dsp*/
	//if (pstFormat->format)
	{
		temp |= 1 << 3;
	}
	tp28xx_byte_write(i2c_addr, 0x17, temp);

	temp = tp28xx_byte_read(i2c_addr, 0x18);
	temp &= 0xef;
	temp |= pstFormat->bitrate << 4;
	tp28xx_byte_write(i2c_addr, 0x18, temp);

	temp = tp28xx_byte_read(i2c_addr, 0x19);
	temp &= 0xdc;
	if (pstFormat->mode == 0)
	{
		/*slave mode*/
		temp |= 1 << 5;
	}
	else
	{
		/*master mode*/
		temp |= 0x3;
	}
	/*Notice:*/
	temp |= pstFormat->bitrate << 4;
	tp28xx_byte_write(i2c_addr, 0x19, temp);

	tp2823_audio_config_rmpos(i2c_addr, pstFormat->format, pstFormat->chn_num);

	return SUCCESS;
}

static int tp2823_set_audio_pb_format(unsigned i2c_addr, tp2823_audio_format *pstFormat)
{
	unsigned char temp = 0;
	
	if (pstFormat->mode > 1)
	{
		return FAILURE;
	}

	if (pstFormat->format> 1)
	{
		return FAILURE;
	}

	if (pstFormat->bitrate> 1)
	{
		return FAILURE;
	}

	if (pstFormat->clkdir> 1)
	{
		return FAILURE;
	}

	if (pstFormat->precision > 1)
	{
		return FAILURE;
	}

	temp = tp28xx_byte_read(i2c_addr, 0x1b);
	temp &= 0xb2;
	temp |= pstFormat->mode;
	temp |= pstFormat->format << 2;
	/*dsp*/
	if (pstFormat->format)
	{
		temp |= 1 << 3;
	}
	temp |= pstFormat->precision << 6;

	tp28xx_byte_write(i2c_addr, 0x1b, temp);

	return SUCCESS;
}

long tp2802_ioctl(struct file* file, unsigned int cmd, unsigned long arg)
{
    unsigned int __user* argp = (unsigned int __user*)arg;
    unsigned int i, j, i2c_addr, tmp, ret = 0;
    unsigned long flags;

    tp2802_register		   dev_register;
    tp2802_image_adjust    image_adjust;
    tp2802_work_mode       work_mode;
    tp2802_video_mode	   video_mode;
    tp2802_video_loss      video_loss;
    tp2802_PTZ_data        PTZ_data;
    tp2802_audio_playback  audio_playback ;
    tp2802_audio_da_volume audio_da_volume;

    switch (_IOC_NR(cmd))
    {

        case _IOC_NR(TP2802_READ_REG):
        {
            if (copy_from_user(&dev_register, argp, sizeof(tp2802_register)))
            { return FAILURE; }

            spin_lock_irqsave(&watchdog_lock, flags);

            i2c_addr = TP2802_I2C_ADDR(dev_register.chip);

            if (dev_register.reg_addr < 0x40)
            { tp2802_set_reg_page(i2c_addr, dev_register.ch); }

            dev_register.value = tp28xx_byte_read(i2c_addr, dev_register.reg_addr);

            spin_unlock_irqrestore(&watchdog_lock, flags);

            if (copy_to_user(argp, &dev_register, sizeof(tp2802_register)))
            { return FAILURE; }

            break;
        }

        case _IOC_NR(TP2802_WRITE_REG):
        {
            if (copy_from_user(&dev_register, argp, sizeof(tp2802_register)))
            { return FAILURE; }

            spin_lock_irqsave(&watchdog_lock, flags);

            i2c_addr = TP2802_I2C_ADDR(dev_register.chip);

            if (dev_register.reg_addr < 0x40)
            { tp2802_set_reg_page(i2c_addr, dev_register.ch); }

            tp28xx_byte_write(i2c_addr, dev_register.reg_addr, dev_register.value);

            spin_unlock_irqrestore(&watchdog_lock, flags);
            break;
        }

        case _IOC_NR(TP2802_SET_VIDEO_MODE):
        {
            if (copy_from_user(&work_mode, argp, sizeof(tp2802_work_mode)))
            { return FAILURE; }

            spin_lock_irqsave(&watchdog_lock, flags);

            i2c_addr = TP2802_I2C_ADDR(work_mode.chip);
            ret = tp2802_set_video_mode(i2c_addr, work_mode.mode, work_mode.ch);

            spin_unlock_irqrestore(&watchdog_lock, flags);

            if (!(ret))
            {
                return SUCCESS;
            }
            else
            {
                printk("Invalid mode:%d\n", work_mode.mode);
                return FAILURE;
            }

            break;
        }

        case _IOC_NR(TP2802_GET_VIDEO_MODE):
        {
            if (copy_from_user(&video_mode, argp, sizeof(tp2802_video_mode)))
            { return FAILURE; }

#if (WDT)
            video_mode.mode = watchdog_info[video_mode.chip].mode[video_mode.ch];

#else
            spin_lock_irqsave(&watchdog_lock, flags);

            i2c_addr = TP2802_I2C_ADDR(video_mode.chip);

            tp2802_set_reg_page(i2c_addr, video_mode.ch);

            tmp = tp28xx_byte_read(i2c_addr, 0x03);
            tmp &= 0x7; /* [2:0] - CVSTD */
            video_mode.mode = tmp;

            tmp = tp28xx_byte_read(i2c_addr, 0x01);
            tmp = (tmp & 0x2) >> 1; /* [2] - NINTL */
            video_mode.prog = tmp;

            spin_unlock_irqrestore(&watchdog_lock, flags);
#endif

            if (copy_to_user(argp, &video_mode, sizeof(tp2802_video_mode)))
            { return FAILURE; }

            break;
        }

        case _IOC_NR(TP2802_GET_VIDEO_LOSS):/* get video loss state */
        {
            if (copy_from_user(&video_loss, argp, sizeof(tp2802_video_loss)))
            { return FAILURE; }

#if (WDT)
            video_loss.is_lost = ( VIDEO_LOCKED == watchdog_info[video_loss.chip].state[video_loss.ch] ) ? 0 : 1;

#else
            spin_lock_irqsave(&watchdog_lock, flags);

            i2c_addr = TP2802_I2C_ADDR(video_loss.chip);

            tp2802_set_reg_page(i2c_addr, video_loss.ch);

            tmp = tp28xx_byte_read(i2c_addr, 0x01);
            tmp = (tmp & 0x80) >> 7;

            if (!tmp)
            {
                if (0x08 == tp28xx_byte_read(i2c_addr, 0x2f))
                {
                    tmp = tp28xx_byte_read(i2c_addr, 0x04);

                    if (tmp < 0x30) { tmp = 0; }
                    else { tmp = 1; }
                }

            }

            video_loss.is_lost = tmp;   /* [7] - VDLOSS */

            spin_unlock_irqrestore(&watchdog_lock, flags);
#endif

            if (copy_to_user(argp, &video_loss, sizeof(video_loss)))
            { return FAILURE; }

            break;
        }

        case _IOC_NR(TP2802_SET_IMAGE_ADJUST):
        {
            if (copy_from_user(&image_adjust, argp, sizeof(tp2802_image_adjust)))
            {
                return FAILURE;
            }

            spin_lock_irqsave(&watchdog_lock, flags);

            i2c_addr = TP2802_I2C_ADDR(image_adjust.chip);

            tp2802_set_reg_page(i2c_addr, image_adjust.ch);

            // Set Brightness
            tp28xx_byte_write(i2c_addr, BRIGHTNESS, image_adjust.brightness);

            // Set Contrast
            tp28xx_byte_write(i2c_addr, CONTRAST, image_adjust.contrast);

            // Set Saturation
            tp28xx_byte_write(i2c_addr, SATURATION, image_adjust.saturation);

            // Set Hue
            tp28xx_byte_write(i2c_addr, HUE, image_adjust.hue);

            // Set Sharpness
            tp28xx_byte_write(i2c_addr, SHARPNESS, (image_adjust.sharpness & 0x1F));

            spin_unlock_irqrestore(&watchdog_lock, flags);
            break;
        }

        case _IOC_NR(TP2802_GET_IMAGE_ADJUST):
        {
            if (copy_from_user(&image_adjust, argp, sizeof(tp2802_image_adjust)))
            { return FAILURE; }

            spin_lock_irqsave(&watchdog_lock, flags);

            i2c_addr = TP2802_I2C_ADDR(image_adjust.chip);

            tp2802_set_reg_page(i2c_addr, image_adjust.ch);

            // Get Brightness
            image_adjust.brightness = tp28xx_byte_read(i2c_addr, BRIGHTNESS);

            // Get Contrast
            image_adjust.brightness = tp28xx_byte_read(i2c_addr, CONTRAST);

            // Get Saturation
            image_adjust.brightness = tp28xx_byte_read(i2c_addr, SATURATION);

            // Get Hue
            image_adjust.brightness = tp28xx_byte_read(i2c_addr, HUE);

            // Get Sharpness
            image_adjust.brightness = tp28xx_byte_read(i2c_addr, SHARPNESS);

            spin_unlock_irqrestore(&watchdog_lock, flags);

            if (copy_to_user(argp, &image_adjust, sizeof(tp2802_image_adjust)))
            { return FAILURE; }

            break;
        }

        case _IOC_NR(TP2802_SET_PTZ_DATA):
        {
            if (copy_from_user(&PTZ_data, argp, sizeof(tp2802_PTZ_data)))
            {
                return FAILURE;
            }

            spin_lock_irqsave(&watchdog_lock, flags);

            i2c_addr = TP2802_I2C_ADDR(PTZ_data.chip);

            if (TP2822 ==  id[PTZ_data.chip] || TP2823 ==  id[PTZ_data.chip])
            {

                TP2822_PTZ_mode(i2c_addr, PTZ_data.ch);

                tmp = tp28xx_byte_read(i2c_addr, 0x40);
                tmp &= 0xef;
                tp28xx_byte_write(i2c_addr, 0x40, tmp); //data buffer bank0 switch for 2822

                // TX disable
                tp28xx_byte_write(i2c_addr, 0xBB, tp28xx_byte_read(i2c_addr, 0xBB) & ~(0x01 << (PTZ_data.ch)));

                //line1
                tp28xx_byte_write(i2c_addr, 0x56 + PTZ_data.ch * 10 , 0x02);
                tp28xx_byte_write(i2c_addr, 0x57 + PTZ_data.ch * 10 , PTZ_data.data[0]);
                tp28xx_byte_write(i2c_addr, 0x58 + PTZ_data.ch * 10 , PTZ_data.data[1]);
                tp28xx_byte_write(i2c_addr, 0x59 + PTZ_data.ch * 10 , PTZ_data.data[2]);
                tp28xx_byte_write(i2c_addr, 0x5A + PTZ_data.ch * 10 , PTZ_data.data[3]);
                //line2
                tp28xx_byte_write(i2c_addr, 0x5B + PTZ_data.ch * 10 , 0x02);
                tp28xx_byte_write(i2c_addr, 0x5C + PTZ_data.ch * 10 , PTZ_data.data[4]);
                tp28xx_byte_write(i2c_addr, 0x5D + PTZ_data.ch * 10 , PTZ_data.data[5]);
                tp28xx_byte_write(i2c_addr, 0x5E + PTZ_data.ch * 10 , PTZ_data.data[6]);
                tp28xx_byte_write(i2c_addr, 0x5F + PTZ_data.ch * 10 , PTZ_data.data[7]);

                // TX enable
                tp28xx_byte_write(i2c_addr, 0xBB, (0x01 << (PTZ_data.ch)));

            }
            else if (TP2802D == id[PTZ_data.chip] )
            {
                tp28xx_byte_write(i2c_addr, 0x40, 0x00); //bank switch for D


                tmp = tp28xx_byte_read(i2c_addr, 0xf5); //check TVI 1 or 2

                if ( (tmp >> PTZ_data.ch) & 0x01)
                {
                    tp28xx_byte_write(i2c_addr, 0x53, 0x33);
                    tp28xx_byte_write(i2c_addr, 0x54, 0xf0);

                }
                else
                {
                    tp28xx_byte_write(i2c_addr, 0x53, 0x19);
                    tp28xx_byte_write(i2c_addr, 0x54, 0x78);
                }

                // TX disable
                tp28xx_byte_write(i2c_addr, 0xBB, tp28xx_byte_read(i2c_addr, 0xBB) & ~(0x01 << (PTZ_data.ch)));

                //line1
                tp28xx_byte_write(i2c_addr, 0x56 + PTZ_data.ch * 10 , 0x02);
                tp28xx_byte_write(i2c_addr, 0x57 + PTZ_data.ch * 10 , PTZ_data.data[0]);
                tp28xx_byte_write(i2c_addr, 0x58 + PTZ_data.ch * 10 , PTZ_data.data[1]);
                tp28xx_byte_write(i2c_addr, 0x59 + PTZ_data.ch * 10 , PTZ_data.data[2]);
                tp28xx_byte_write(i2c_addr, 0x5A + PTZ_data.ch * 10 , PTZ_data.data[3]);
                //line2
                tp28xx_byte_write(i2c_addr, 0x5B + PTZ_data.ch * 10 , 0x02);
                tp28xx_byte_write(i2c_addr, 0x5C + PTZ_data.ch * 10 , PTZ_data.data[4]);
                tp28xx_byte_write(i2c_addr, 0x5D + PTZ_data.ch * 10 , PTZ_data.data[5]);
                tp28xx_byte_write(i2c_addr, 0x5E + PTZ_data.ch * 10 , PTZ_data.data[6]);
                tp28xx_byte_write(i2c_addr, 0x5F + PTZ_data.ch * 10 , PTZ_data.data[7]);

                // TX enable
                tp28xx_byte_write(i2c_addr, 0xBB, (0x01 << (PTZ_data.ch)));

            }
            else if (TP2802C == id[PTZ_data.chip] )
            {
                // line1
                tp28xx_byte_write(i2c_addr, 0x56 + PTZ_data.ch * 10 , 0x02);
                tp28xx_byte_write(i2c_addr, 0x57 + PTZ_data.ch * 10 , PTZ_data.data[0]);
                tp28xx_byte_write(i2c_addr, 0x58 + PTZ_data.ch * 10 , PTZ_data.data[1]);
                tp28xx_byte_write(i2c_addr, 0x59 + PTZ_data.ch * 10 , PTZ_data.data[2]);
                tp28xx_byte_write(i2c_addr, 0x5A + PTZ_data.ch * 10 , PTZ_data.data[3]);
                //line2
                tp28xx_byte_write(i2c_addr, 0x5B + PTZ_data.ch * 10 , 0x02);
                tp28xx_byte_write(i2c_addr, 0x5C + PTZ_data.ch * 10 , PTZ_data.data[4]);
                tp28xx_byte_write(i2c_addr, 0x5D + PTZ_data.ch * 10 , PTZ_data.data[5]);
                tp28xx_byte_write(i2c_addr, 0x5E + PTZ_data.ch * 10 , PTZ_data.data[6]);
                tp28xx_byte_write(i2c_addr, 0x5F + PTZ_data.ch * 10 , PTZ_data.data[7]);

                // TX enable
                tp28xx_byte_write(i2c_addr, 0x7e, 0x20 | (0x01 << (PTZ_data.ch)));
            }


            spin_unlock_irqrestore(&watchdog_lock, flags);
            break;
        }

        case _IOC_NR(TP2802_GET_PTZ_DATA):
        {
            if (copy_from_user(&PTZ_data, argp, sizeof(tp2802_PTZ_data)))
            {
                return FAILURE;
            }

            spin_lock_irqsave(&watchdog_lock, flags);

            i2c_addr = TP2802_I2C_ADDR(PTZ_data.chip);

            if (TP2822 == id[PTZ_data.chip] || TP2823 ==  id[PTZ_data.chip] )
            {
                tmp = tp28xx_byte_read(i2c_addr, 0x40);
                tmp &= 0xef;
                tp28xx_byte_write(i2c_addr, 0x40, tmp); //bank switch for 2822

            }
            else if (TP2802D == id[PTZ_data.chip] )
            {
                tp28xx_byte_write(i2c_addr, 0x40, 0x00); //bank switch for D
            }

            // line1

            PTZ_data.data[0] = tp28xx_byte_read(i2c_addr, 0x8C + PTZ_data.ch * 10 );
            PTZ_data.data[1] = tp28xx_byte_read(i2c_addr, 0x8D + PTZ_data.ch * 10 );
            PTZ_data.data[2] = tp28xx_byte_read(i2c_addr, 0x8E + PTZ_data.ch * 10 );
            PTZ_data.data[3] = tp28xx_byte_read(i2c_addr, 0x8F + PTZ_data.ch * 10 );

            //line2
            PTZ_data.data[4] = tp28xx_byte_read(i2c_addr, 0x91 + PTZ_data.ch * 10 );
            PTZ_data.data[5] = tp28xx_byte_read(i2c_addr, 0x92 + PTZ_data.ch * 10 );
            PTZ_data.data[6] = tp28xx_byte_read(i2c_addr, 0x93 + PTZ_data.ch * 10 );
            PTZ_data.data[7] = tp28xx_byte_read(i2c_addr, 0x94 + PTZ_data.ch * 10 );


            spin_unlock_irqrestore(&watchdog_lock, flags);
            break;
        }

        case _IOC_NR(TP2802_SET_SCAN_MODE):
        {
            if (copy_from_user(&work_mode, argp, sizeof(tp2802_work_mode)))
            { return FAILURE; }

            spin_lock_irqsave(&watchdog_lock, flags);

            if (work_mode.ch >= 0x04)
            {
                watchdog_info[work_mode.chip].scan[0] = work_mode.mode;
                watchdog_info[work_mode.chip].scan[1] = work_mode.mode;
                watchdog_info[work_mode.chip].scan[2] = work_mode.mode;
                watchdog_info[work_mode.chip].scan[3] = work_mode.mode;
            }
            else
            {
                watchdog_info[work_mode.chip].scan[work_mode.ch] = work_mode.mode;

            }

            spin_unlock_irqrestore(&watchdog_lock, flags);


            break;
        }

        case _IOC_NR(TP2802_DUMP_REG):
        {
            if (copy_from_user(&dev_register, argp, sizeof(tp2802_register)))
            { return FAILURE; }

            spin_lock_irqsave(&watchdog_lock, flags);

            i2c_addr = TP2802_I2C_ADDR(dev_register.chip);

            for (i = 0; i < 4; i++)
            {

                tp2802_set_reg_page(i2c_addr, i);

                for (j = 0; j < 0x40; j++)
                {
                    dev_register.value = tp28xx_byte_read(i2c_addr, j);
                    printk("%02x:%02x\n", j, dev_register.value );
                }
            }

            for (j = 0x40; j < 0x100; j++)
            {
                dev_register.value = tp28xx_byte_read(i2c_addr, j);
                printk("%02x:%02x\n", j, dev_register.value );
            }

            spin_unlock_irqrestore(&watchdog_lock, flags);

            if (copy_to_user(argp, &dev_register, sizeof(tp2802_register)))
            { return FAILURE; }

            break;
        }

        case _IOC_NR(TP2802_FORCE_DETECT):
        {
            if (copy_from_user(&work_mode, argp, sizeof(tp2802_work_mode)))
            { return FAILURE; }

            spin_lock_irqsave(&watchdog_lock, flags);

            if (work_mode.ch >= 0x04)
            {
                watchdog_info[work_mode.chip].force[0] = 1;
                watchdog_info[work_mode.chip].force[1] = 1;
                watchdog_info[work_mode.chip].force[2] = 1;
                watchdog_info[work_mode.chip].force[3] = 1;
            }
            else
            {
                watchdog_info[work_mode.chip].force[work_mode.ch] = 1;

            }

            spin_unlock_irqrestore(&watchdog_lock, flags);


            break;
        }

        case _IOC_NR(TP2802_SET_SAMPLE_RATE):
        {
            tp2802_audio_samplerate samplerate;

            if (copy_from_user(&samplerate, argp, sizeof(samplerate)))
            { return FAILURE; }

            spin_lock_irqsave(&watchdog_lock, flags);

            for (i = 0; i < chips; i ++)
            {
                i2c_addr = tp2802_i2c_addr[i];

                tp2802_set_reg_page(i2c_addr, 0x09); //audio page
                tmp = tp28xx_byte_read(i2c_addr, 0x18);
                tmp &= 0xf8;

                if (SAMPLE_RATE_16000 == samplerate)   { tmp |= 0x01; }

                tp28xx_byte_write(i2c_addr, 0x18, tmp);
                tp28xx_byte_write(i2c_addr, 0x3d, 0x01); //audio reset
            }

            spin_unlock_irqrestore(&watchdog_lock, flags);
            break;
        }

        case _IOC_NR(TP2802_SET_AUDIO_PLAYBACK):
        {

            if (copy_from_user(&audio_playback, argp, sizeof(tp2802_audio_playback)))
            { return FAILURE; }

            if (audio_playback.chn > 25 || audio_playback.chn < 1)
            { return FAILURE; };

            spin_lock_irqsave(&watchdog_lock, flags);

            i2c_addr = TP2802_I2C_ADDR(audio_playback.chip);

            tp2802_set_reg_page(i2c_addr, 0x09); //audio page

            tp28xx_byte_write(i2c_addr, 0x1a, audio_playback.chn);

            spin_unlock_irqrestore(&watchdog_lock, flags);

            break;
        }

        case _IOC_NR(TP2802_SET_AUDIO_DA_VOLUME):
        {
            if (copy_from_user(&audio_da_volume, argp, sizeof(tp2802_audio_da_volume)))
            { return FAILURE; }

            if (audio_da_volume.volume > 15 || audio_da_volume.volume < 0)
            { return FAILURE; };

            spin_lock_irqsave(&watchdog_lock, flags);

            i2c_addr = TP2802_I2C_ADDR(audio_da_volume.chip);

            tp2802_set_reg_page(i2c_addr, 0x09); //audio page

            tp28xx_byte_write(i2c_addr, 0x1f, audio_da_volume.volume);

            spin_unlock_irqrestore(&watchdog_lock, flags);

            break;
        }

        case _IOC_NR(TP2802_SET_AUDIO_DA_MUTE):
        {
            tp2802_audio_da_mute audio_da_mute;

            if (copy_from_user(&audio_da_mute, argp, sizeof(tp2802_audio_da_mute)))
            { return FAILURE; }

            if (audio_da_volume.chip > chips || audio_da_volume.chip < 0)
            { return FAILURE; };

            spin_lock_irqsave(&watchdog_lock, flags);

            i2c_addr = TP2802_I2C_ADDR(audio_da_mute.chip);

            tp2802_set_reg_page(i2c_addr, 0x09); //audio page

            tmp = tp28xx_byte_read(i2c_addr, 0x38);

            tmp &= 0xf0;

            if (audio_da_mute.flag)
            {
                tp28xx_byte_write(i2c_addr, 0x38, tmp);
            }
            else
            {
                tmp |= 0x08;
                tp28xx_byte_write(i2c_addr, 0x38, tmp);
            }

            spin_unlock_irqrestore(&watchdog_lock, flags);

            break;
        }

		case _IOC_NR(TP2802_SET_AUDIO_RM_FORMAT):
        {
            tp2823_audio_format audio_format;
			int i = 0;

            if (copy_from_user(&audio_format, argp, sizeof(tp2823_audio_format)))
            { return FAILURE; }

            if (audio_format.chip > chips || audio_format.chip < 0)
            { return FAILURE; };

            spin_lock_irqsave(&watchdog_lock, flags);

			for (i=0; i<chips; i++)
			{
				i2c_addr = TP2802_I2C_ADDR(i);

	            tp2802_set_reg_page(i2c_addr, 0x09); //audio page

	            ret = tp2823_set_audio_rm_format(i2c_addr, &audio_format);
			}            

            spin_unlock_irqrestore(&watchdog_lock, flags);

            break;
        }

		case _IOC_NR(TP2802_SET_AUDIO_PB_FORMAT):
        {
            tp2823_audio_format audio_format;

            if (copy_from_user(&audio_format, argp, sizeof(tp2823_audio_format)))
            { return FAILURE; }

            if (audio_format.chip > chips || audio_format.chip < 0)
            { return FAILURE; };

            spin_lock_irqsave(&watchdog_lock, flags);

            i2c_addr = TP2802_I2C_ADDR(audio_format.chip);

            tp2802_set_reg_page(i2c_addr, 0x09); //audio page

            ret = tp2823_set_audio_pb_format(i2c_addr, &audio_format);

            spin_unlock_irqrestore(&watchdog_lock, flags);

            break;
        }

        default:
        {
            printk("Invalid tp2802 ioctl cmd!\n");
            ret = -1;
            break;
        }
    }

    return ret;
}


int tp2802_set_video_mode(unsigned char addr, unsigned char mode, unsigned char ch)
{
    int err = 0;
    unsigned int tmp;

    // Set Page Register to the appropriate Channel
    tp2802_set_reg_page(addr, ch);
    tmp = tp28xx_byte_read(addr, 0x02);

    switch (mode)
    {
        case TP2802_1080P25:
            tp2802_set_work_mode_1080p25(addr);
            tmp &= 0xF8;
            tp28xx_byte_write(addr, 0x02, tmp);
            break;

        case TP2802_1080P30:
            tp2802_set_work_mode_1080p30(addr);
            tmp &= 0xF8;
            tp28xx_byte_write(addr, 0x02, tmp);
            break;

        case TP2802_720P25:
            tp2802_set_work_mode_720p25(addr);
            tmp &= 0xF8;
            tmp |= 0x02;
            tp28xx_byte_write(addr, 0x02, tmp);
            break;

        case TP2802_720P30:
            tp2802_set_work_mode_720p30(addr);
            tmp &= 0xF8;
            tmp |= 0x02;
            tp28xx_byte_write(addr, 0x02, tmp);
            break;

        case TP2802_720P50:
            tp2802_set_work_mode_720p50(addr);
            tmp &= 0xF8;
            tmp |= 0x02;
            tp28xx_byte_write(addr, 0x02, tmp);
            break;

        case TP2802_720P60:
            tp2802_set_work_mode_720p60(addr);
            tmp &= 0xF8;
            tmp |= 0x02;
            tp28xx_byte_write(addr, 0x02, tmp);
            break;

        case TP2802_720P30V2:
            tp2802_set_work_mode_720p60(addr);
            tmp &= 0xF8;
            tmp |= 0x02;
            tp28xx_byte_write(addr, 0x02, tmp);
            break;

        case TP2802_720P25V2:
            tp2802_set_work_mode_720p50(addr);
            tmp &= 0xF8;
            tmp |= 0x02;
            tp28xx_byte_write(addr, 0x02, tmp);
            break;

        default:
            err = -1;
            break;
    }


    return err;
}

void tp2802_set_reg_page(unsigned char addr, unsigned char ch)
{
    switch (ch)
    {
        case 0:
            tp28xx_byte_write(addr, 0x40, 0x00);
            break;  // VIN1 registers

        case 1:
            tp28xx_byte_write(addr, 0x40, 0x01);
            break;  // VIN2 registers

        case 2:
            tp28xx_byte_write(addr, 0x40, 0x02);
            break;  // VIN3 registers

        case 3:
            tp28xx_byte_write(addr, 0x40, 0x03);
            break;  // VIN4 registers

        case 4:
            tp28xx_byte_write(addr, 0x40, 0x04);
            break;  // Write All VIN1-4 regsiters

        case 9:
            tp28xx_byte_write(addr, 0x40, 0x40);
            break;  // Audio

        default:
            tp28xx_byte_write(addr, 0x40, 0x04);
            break;
    }
}
static void tp2802_manual_agc(unsigned char addr, unsigned char ch)
{
    unsigned int agc, tmp;

    tp28xx_byte_write(addr, 0x2F, 0x02);
    agc = tp28xx_byte_read(addr, 0x04);
    printk("AGC=0x%04x ch%02x\r\n", agc, ch);
    agc += tp28xx_byte_read(addr, 0x04);
    agc += tp28xx_byte_read(addr, 0x04);
    agc += tp28xx_byte_read(addr, 0x04);
    agc &= 0x3f0;
    agc >>= 1;

    if (agc > 0x1ff) { agc = 0x1ff; }

    printk("AGC=0x%04x ch%02x\r\n", agc, ch);
    tp28xx_byte_write(addr, 0x08, agc & 0xff);
    tmp = tp28xx_byte_read(addr, 0x06);
    tmp &= 0xf9;
    tmp |= (agc >> 7) & 0x02;
    tmp |= 0x04;
    tp28xx_byte_write(addr, 0x06, tmp);
}
static void tp282x_SYSCLK_V2(unsigned char addr, unsigned char ch)
{
    unsigned char tmp;
    tmp = tp28xx_byte_read(addr, 0xf5);
    tmp |= SYS_MODE[ch];
    tp28xx_byte_write(addr, 0xf5, tmp);

    if (MUX_2CH == output)
    {
        tp28xx_byte_write(addr, 0x35, 0x25);
    }
    else if (MUX_1CH == output)
    {
        tp28xx_byte_write(addr, 0x35, 0x25);
        tmp = tp28xx_byte_read(addr, CLK_ADDR[ch]);
        tmp &= CLK_AND[ch];
        tmp |= CLK_MODE[ch];
        tp28xx_byte_write(addr, CLK_ADDR[ch], tmp);
    }

}
static void tp282x_SYSCLK_V1(unsigned char addr, unsigned char ch)
{
    unsigned char tmp;
    tmp = tp28xx_byte_read(addr, 0xf5);
    tmp &= SYS_AND[ch];
    tp28xx_byte_write(addr, 0xf5, tmp);

    if (MUX_2CH == output)
    {
        tp28xx_byte_write(addr, 0x35, 0x45);
    }
    else if (MUX_1CH == output || DDR_2CH == output)
    {
        tp28xx_byte_write(addr, 0x35, 0x05);
        tmp = tp28xx_byte_read(addr, CLK_ADDR[ch]);
        tmp &= CLK_AND[ch];
        tp28xx_byte_write(addr, CLK_ADDR[ch], tmp);
    }

}
static void tp2802C_reset_default(unsigned char addr, unsigned char ch)
{
    tp28xx_byte_write(addr, 0x2F, 0x08);
    tp28xx_byte_write(addr, 0x07, 0x0d);
    tp28xx_byte_write(addr, 0x3a, 0x02);
}

static void tp2802D_reset_default(unsigned char addr, unsigned char ch)
{
    unsigned int tmp;
    tp28xx_byte_write(addr, 0x3A, 0x01);
    tp28xx_byte_write(addr, 0x0B, 0xC0);
    tp28xx_byte_write(addr, 0x07, 0xC0);
    tp28xx_byte_write(addr, 0x2e, 0x70);
    tp28xx_byte_write(addr, 0x39, 0x42);
    tp28xx_byte_write(addr, 0x09, 0x24);
    tmp = tp28xx_byte_read(addr, 0x06);   // soft reset and auto agc when cable is unplug
    tmp &= 0x7b;
    tp28xx_byte_write(addr, 0x06, tmp);

    tmp = tp28xx_byte_read(addr, 0xf5);
    tmp &= SYS_AND[ch];
    tp28xx_byte_write(addr, 0xf5, tmp);
    tmp = tp28xx_byte_read(addr, CLK_ADDR[ch]);
    tmp &= CLK_AND[ch];
    tp28xx_byte_write(addr, CLK_ADDR[ch], tmp);

}

static void tp282x_reset_default(unsigned char addr, unsigned char ch)
{
    unsigned int tmp;

    tmp = tp28xx_byte_read(addr, 0x06);
    tmp &= 0xfb;
    tp28xx_byte_write(addr, 0x06, tmp);
    tp28xx_byte_write(addr, 0x07, 0x40);
    //tp28xx_byte_write(addr, 0x35, 0x45);
    tp28xx_byte_write(addr, 0x2F, 0x08);
    tp282x_SYSCLK_V1(addr, ch);
}

static void TP28xx_reset_default(int chip, unsigned char addr, unsigned char ch)
{
    if (TP2823 == chip )
    {
        tp282x_reset_default(addr, ch);
    }
    else if (TP2822 == chip )
    {
        tp282x_reset_default(addr, ch);
    }
    else if (TP2802C == chip )
    {
        tp2802C_reset_default(addr, ch);
    }
    else if (TP2802D == chip )
    {
        tp2802D_reset_default(addr, ch);
    }
}
static void TP2822_PTZ_init(unsigned char addr)
{
    //TX total 34bit, valid load 32bit
    tp28xx_byte_write(addr, 0xC0, 0x00);
    tp28xx_byte_write(addr, 0xC1, 0x00);
    tp28xx_byte_write(addr, 0xC2, 0x0B);   //TX1 line1
    tp28xx_byte_write(addr, 0xC3, 0x0C);   //TX1 line2
    tp28xx_byte_write(addr, 0xC4, 0x00);   //TX1 line3
    tp28xx_byte_write(addr, 0xC5, 0x00);   //TX1 line4

    tp28xx_byte_write(addr, 0xC6, 0x19);
    tp28xx_byte_write(addr, 0xC7, 0x78);
    tp28xx_byte_write(addr, 0xC8, 0x21);

    // RX total 40bit, valid load 39bit
    tp28xx_byte_write(addr, 0xC9, 0x00);
    tp28xx_byte_write(addr, 0xCA, 0x00);
    tp28xx_byte_write(addr, 0xCB, 0x07);   //RX1 line1
    tp28xx_byte_write(addr, 0xCC, 0x08);   //RX1 line2
    tp28xx_byte_write(addr, 0xCD, 0x00);   //RX1 line3
    tp28xx_byte_write(addr, 0xCE, 0x00);   //RX1 line4

    tp28xx_byte_write(addr, 0xCF, 0x04);
    tp28xx_byte_write(addr, 0xD0, 0x00);
    tp28xx_byte_write(addr, 0xD1, 0x00);
    tp28xx_byte_write(addr, 0xD2, 0x60);
    tp28xx_byte_write(addr, 0xD3, 0x10);
    tp28xx_byte_write(addr, 0xD4, 0x06);
    tp28xx_byte_write(addr, 0xD5, 0xBE);
    tp28xx_byte_write(addr, 0xD6, 0x39);
    tp28xx_byte_write(addr, 0xD7, 0x27);   //


    tp28xx_byte_write(addr, 0xD8, 0x00);
    tp28xx_byte_write(addr, 0xD9, 0x00);
    tp28xx_byte_write(addr, 0xDA, 0x0B);   //TX2 line1
    tp28xx_byte_write(addr, 0xDB, 0x0C);   //TX2 line2
    tp28xx_byte_write(addr, 0xDC, 0x00);   //TX2 line3
    tp28xx_byte_write(addr, 0xDD, 0x00);   //TX2 line4

    tp28xx_byte_write(addr, 0xDE, 0x19);
    tp28xx_byte_write(addr, 0xDF, 0x78);
    tp28xx_byte_write(addr, 0xE0, 0x21);


    tp28xx_byte_write(addr, 0xE1, 0x00);
    tp28xx_byte_write(addr, 0xE2, 0x00);
    tp28xx_byte_write(addr, 0xE3, 0x07);   //RX2 line1
    tp28xx_byte_write(addr, 0xE4, 0x08);   //RX2 line2
    tp28xx_byte_write(addr, 0xE5, 0x00);   //RX2 line3
    tp28xx_byte_write(addr, 0xE6, 0x00);   //RX2 line4

    tp28xx_byte_write(addr, 0xE7, 0x04);
    tp28xx_byte_write(addr, 0xE8, 0x00);
    tp28xx_byte_write(addr, 0xE9, 0x00);
    tp28xx_byte_write(addr, 0xEA, 0x60);
    tp28xx_byte_write(addr, 0xEB, 0x10);
    tp28xx_byte_write(addr, 0xEC, 0x06);
    tp28xx_byte_write(addr, 0xED, 0xBE);
    tp28xx_byte_write(addr, 0xEE, 0x39);
    tp28xx_byte_write(addr, 0xEF, 0x27);   //
}
static void TP2823_NTSC_DataSet(unsigned char addr)
{
    if (MUX_2CH == output || DDR_2CH == output)
    {
        tp28xx_byte_write(addr, 0x02, 0xcf);  //BT656 header
    }
    else if (MUX_1CH == output)
    {
        tp28xx_byte_write(addr, 0x02, 0xc7 | SAV_HEADER); //BT1120/BT656 header
    }

    tp28xx_byte_write(addr, 0x15, 0x13);
    tp28xx_byte_write(addr, 0x16, 0x4e);
    tp28xx_byte_write(addr, 0x17, 0xbc);
    tp28xx_byte_write(addr, 0x18, 0x15);
    tp28xx_byte_write(addr, 0x19, 0xf0);
    tp28xx_byte_write(addr, 0x1a, 0x07);
    tp28xx_byte_write(addr, 0x1c, 0x09);
    tp28xx_byte_write(addr, 0x1d, 0x38);

    tp28xx_byte_write(addr, 0x0c, 0x53);
    tp28xx_byte_write(addr, 0x0d, 0x10);
    tp28xx_byte_write(addr, 0x20, 0xa0);
    tp28xx_byte_write(addr, 0x26, 0x12);
    tp28xx_byte_write(addr, 0x2d, 0x68);
    tp28xx_byte_write(addr, 0x2e, 0x5e);

    tp28xx_byte_write(addr, 0x30, 0x62);
    tp28xx_byte_write(addr, 0x31, 0xbb);
    tp28xx_byte_write(addr, 0x32, 0x96);
    tp28xx_byte_write(addr, 0x33, 0xc0);
    tp28xx_byte_write(addr, 0x35, 0x25);
    tp28xx_byte_write(addr, 0x39, 0x10);

}
static void TP2823_PAL_DataSet(unsigned char addr)
{
    if (MUX_2CH == output || DDR_2CH == output)
    {
        tp28xx_byte_write(addr, 0x02, 0xce);  //BT656 header
    }
    else if (MUX_1CH == output)
    {
        tp28xx_byte_write(addr, 0x02, 0xc6 | SAV_HEADER); //BT1120/BT656 header
    }

    tp28xx_byte_write(addr, 0x15, 0x13);
    tp28xx_byte_write(addr, 0x16, 0x5f);
    tp28xx_byte_write(addr, 0x17, 0xbc);
    tp28xx_byte_write(addr, 0x18, 0x18);
    tp28xx_byte_write(addr, 0x19, 0x20);
    tp28xx_byte_write(addr, 0x1a, 0x17);
    tp28xx_byte_write(addr, 0x1c, 0x09);
    tp28xx_byte_write(addr, 0x1d, 0x48);

    tp28xx_byte_write(addr, 0x0c, 0x53);
    tp28xx_byte_write(addr, 0x0d, 0x11);
    tp28xx_byte_write(addr, 0x20, 0xb0);
    tp28xx_byte_write(addr, 0x26, 0x02);
    tp28xx_byte_write(addr, 0x2d, 0x60);
    tp28xx_byte_write(addr, 0x2e, 0x5e);

    tp28xx_byte_write(addr, 0x30, 0x7a);
    tp28xx_byte_write(addr, 0x31, 0x4a);
    tp28xx_byte_write(addr, 0x32, 0x4d);
    tp28xx_byte_write(addr, 0x33, 0xf0);
    tp28xx_byte_write(addr, 0x35, 0x25);
    tp28xx_byte_write(addr, 0x39, 0x10);

}
static void TP2823_V1_DataSet(unsigned char addr)
{

    tp28xx_byte_write(addr, 0x0c, 0x43);
    tp28xx_byte_write(addr, 0x0d, 0x10);
    tp28xx_byte_write(addr, 0x20, 0x60);
    tp28xx_byte_write(addr, 0x26, 0x02);
    tp28xx_byte_write(addr, 0x2d, 0x30);
    tp28xx_byte_write(addr, 0x2e, 0x70);

    tp28xx_byte_write(addr, 0x30, 0x48);
    tp28xx_byte_write(addr, 0x31, 0xbb);
    tp28xx_byte_write(addr, 0x32, 0x2e);
    tp28xx_byte_write(addr, 0x33, 0x90);
    tp28xx_byte_write(addr, 0x39, 0x30);

}
static void TP2823_V2_DataSet(unsigned char addr)
{
    tp28xx_byte_write(addr, 0x0c, 0x53);
    tp28xx_byte_write(addr, 0x0d, 0x10);
    tp28xx_byte_write(addr, 0x20, 0x60);
    tp28xx_byte_write(addr, 0x26, 0x02);
    tp28xx_byte_write(addr, 0x2d, 0x30);
    tp28xx_byte_write(addr, 0x2e, 0x70);

    tp28xx_byte_write(addr, 0x30, 0x48);
    tp28xx_byte_write(addr, 0x31, 0xbb);
    tp28xx_byte_write(addr, 0x32, 0x2e);
    tp28xx_byte_write(addr, 0x33, 0x90);
    tp28xx_byte_write(addr, 0x39, 0x20);

}
static void TP2823_Audio_DataSet(unsigned char addr)
{

    unsigned int tmp;
    tmp = tp28xx_byte_read(addr, 0x40);
    tmp |= 0x40;
    tp28xx_byte_write(addr, 0x40, tmp);

    tp28xx_byte_write(addr, 0x00, 0x01); //channel
    tp28xx_byte_write(addr, 0x01, 0x02);
    tp28xx_byte_write(addr, 0x08, 0x03);
    tp28xx_byte_write(addr, 0x09, 0x04);

#ifdef DATA_16BIT
    tp28xx_byte_write(addr, 0x17, 0x00);
    tp28xx_byte_write(addr, 0x1B, 0x01);
#else
    tp28xx_byte_write(addr, 0x17, 0x04);
    tp28xx_byte_write(addr, 0x1B, 0x41);
#endif

#ifdef SAMPLE_16K
    tp28xx_byte_write(addr, 0x18, 0x01);
#else
    tp28xx_byte_write(addr, 0x18, 0x00);
#endif

    tp28xx_byte_write(addr, 0x19, 0x1F);
    tp28xx_byte_write(addr, 0x1A, 0x15);

    tp28xx_byte_write(addr, 0x37, 0x20);
    tp28xx_byte_write(addr, 0x38, 0x38);
    tp28xx_byte_write(addr, 0x3E, 0x06);
    tp28xx_byte_write(addr, 0x3d, 0x01);//audio reset

    tmp &= 0xBF;
    tp28xx_byte_write(addr, 0x40, tmp);

}
static void TP2822_PTZ_mode(unsigned char addr, unsigned char ch)
{
    unsigned int tmp;
    static const unsigned char PTZ_reg1[4] = {0xC6, 0xDE, 0xC6, 0xDE};
    static const unsigned char PTZ_reg2[4] = {0xC7, 0xDF, 0xC7, 0xDF};
    static const unsigned char PTZ_bank[4] = {0x00, 0x00, 0x10, 0x10};

    tmp = tp28xx_byte_read(addr, 0x40);
    tmp &= 0xef;
    tmp |= PTZ_bank[ch];
    tp28xx_byte_write(addr, 0x40, tmp); //reg bank1 switch for 2822
    tmp = tp28xx_byte_read(addr, 0xf5); //check TVI 1 or 2

    if ( (tmp >> ch) & 0x01)
    {
        tp28xx_byte_write(addr, PTZ_reg1[ch], 0x33);
        tp28xx_byte_write(addr, PTZ_reg2[ch], 0xf0);

    }
    else
    {
        tp28xx_byte_write(addr, PTZ_reg1[ch], 0x19);
        tp28xx_byte_write(addr, PTZ_reg2[ch], 0x78);
    }
}
static void TP282x_output(unsigned char addr)
{
    tp28xx_byte_write(addr, 0xF5, 0x00);
    tp28xx_byte_write(addr, 0xFA, 0x00);

    if (MUX_2CH == output)
    {
        //2CH-MUX output
        tp28xx_byte_write(addr, 0xF6, 0x10);
        tp28xx_byte_write(addr, 0xF7, 0x32);
        tp28xx_byte_write(addr, 0x40, 0x04);
        tp28xx_byte_write(addr, 0x35, 0x45);

    }
    else if (MUX_1CH == output)
    {
        //1CH-MUX output
        tp28xx_byte_write(addr, 0xF6, 0x00);
        tp28xx_byte_write(addr, 0xF7, 0x11);
        tp28xx_byte_write(addr, 0x40, 0x04);
        tp28xx_byte_write(addr, 0x35, 0x05);
    }
    else if (DDR_2CH == output)
    {
        tp28xx_byte_write(addr, 0x45, 0x54); //PLL 297M
        tp28xx_byte_write(addr, 0xF4, 0x60); //output clock 148.5M
        tp28xx_byte_write(addr, 0xF6, 0x18); //
        tp28xx_byte_write(addr, 0xF7, 0x32);
        tp28xx_byte_write(addr, 0x40, 0x04);
        tp28xx_byte_write(addr, 0x35, 0x05);
    }
}
static void tp2802_comm_init( int i)
{
    unsigned char addr;
    addr = tp2802_i2c_addr[i];
    tp2802_set_reg_page(addr, 4);

    if (TP2823 == id[i])
    {
        if (MUX_2CH == output || DDR_2CH == output)
        {
            tp28xx_byte_write(addr, 0x02, 0xca);  //BT656 header
        }
        else if (MUX_1CH == output)
        {
            tp28xx_byte_write(addr, 0x02, 0xc2 | SAV_HEADER); //BT1120/BT656 header
        }

        tp28xx_byte_write(addr, 0x0b, 0x60);
        tp28xx_byte_write(addr, 0x22, 0x34);
        tp28xx_byte_write(addr, 0x23, 0x44);
        tp28xx_byte_write(addr, 0x38, 0x40);
        tp28xx_byte_write(addr, 0x2F, 0x08);

        tp28xx_byte_write(addr, 0x30, 0x48);
        tp28xx_byte_write(addr, 0x31, 0xBB);
        tp28xx_byte_write(addr, 0x32, 0x2E);
        tp28xx_byte_write(addr, 0x33, 0x90);
        //tp28xx_byte_write(addr, 0x35, 0x45);
        tp28xx_byte_write(addr, 0x4D, 0x03);
        tp28xx_byte_write(addr, 0x4E, 0x33);


        //2CH-MUX output
        //tp28xx_byte_write(addr, 0xF5, 0x00);
        //tp28xx_byte_write(addr, 0xF6, 0x10);
        //tp28xx_byte_write(addr, 0xF7, 0x32);
        //tp28xx_byte_write(addr, 0xFA, 0x00);
        TP282x_output(addr);

        //channel ID
        tp28xx_byte_write(addr, 0x40, 0x00);
        tp28xx_byte_write(addr, 0x34, 0x10);
        tp28xx_byte_write(addr, 0x40, 0x01);
        tp28xx_byte_write(addr, 0x34, 0x11);
        tp28xx_byte_write(addr, 0x40, 0x02);
        tp28xx_byte_write(addr, 0x34, 0x10);
        tp28xx_byte_write(addr, 0x40, 0x03);
        tp28xx_byte_write(addr, 0x34, 0x11);

        //bank 1 for TX/RX 3,4
        tp28xx_byte_write(addr, 0x40, 0x10);
        TP2822_PTZ_init(addr);
        //bank 0 for TX/RX 1,2
        tp28xx_byte_write(addr, 0x40, 0x04);
        TP2822_PTZ_init(addr);
        tp28xx_byte_write(addr, 0x7E, 0x0F);   //TX channel enable
        tp28xx_byte_write(addr, 0xB9, 0x0F);   //RX channel enable

        TP2823_Audio_DataSet(addr);

    }
    else if (TP2822 == id[i])
    {
        if (MUX_2CH == output || DDR_2CH == output)
        {
            tp28xx_byte_write(addr, 0x02, 0xca);  //BT656 header
        }
        else if (MUX_1CH == output)
        {
            tp28xx_byte_write(addr, 0x02, 0xc2 | SAV_HEADER); //BT1120/BT656 header
        }

        //tp28xx_byte_write(addr, 0x07, 0xC0);
        //tp28xx_byte_write(addr, 0x0B, 0xC0);
        tp28xx_byte_write(addr, 0x22, 0x38);
        tp28xx_byte_write(addr, 0x2E, 0x60);

        tp28xx_byte_write(addr, 0x30, 0x48);
        tp28xx_byte_write(addr, 0x31, 0xBB);
        tp28xx_byte_write(addr, 0x32, 0x2E);
        tp28xx_byte_write(addr, 0x33, 0x90);

        //tp28xx_byte_write(addr, 0x35, 0x45);
        tp28xx_byte_write(addr, 0x39, 0x00);
        tp28xx_byte_write(addr, 0x3A, 0x01);
        tp28xx_byte_write(addr, 0x3D, 0x08);
        tp28xx_byte_write(addr, 0x4D, 0x03);
        tp28xx_byte_write(addr, 0x4E, 0x33);

        //2CH-MUX output
        //tp28xx_byte_write(addr, 0xF5, 0x00);
        //tp28xx_byte_write(addr, 0xF6, 0x10);
        //tp28xx_byte_write(addr, 0xF7, 0x32);
        //tp28xx_byte_write(addr, 0xFA, 0x00);
        TP282x_output(addr);

        //channel ID
        tp28xx_byte_write(addr, 0x40, 0x00);
        tp28xx_byte_write(addr, 0x34, 0x10);
        tp28xx_byte_write(addr, 0x40, 0x01);
        tp28xx_byte_write(addr, 0x34, 0x11);
        tp28xx_byte_write(addr, 0x40, 0x02);
        tp28xx_byte_write(addr, 0x34, 0x10);
        tp28xx_byte_write(addr, 0x40, 0x03);
        tp28xx_byte_write(addr, 0x34, 0x11);

        //bank 1 for TX/RX 3,4
        tp28xx_byte_write(addr, 0x40, 0x10);
        TP2822_PTZ_init(addr);
        //bank 0 for TX/RX 1,2
        tp28xx_byte_write(addr, 0x40, 0x00);
        TP2822_PTZ_init(addr);
        tp28xx_byte_write(addr, 0x7E, 0x0F);   //TX channel enable
        tp28xx_byte_write(addr, 0xB9, 0x0F);   //RX channel enable

        tp28xx_byte_write(addr, 0x40, 0x04); //all ch reset
        tp28xx_byte_write(addr, 0x3D, 0x00);

    }
    else if (TP2802D == id[i])
    {
        tp28xx_byte_write(addr, 0x02, 0xC0 | SAV_HEADER); //8 bit BT1120/BT656 mode
        tp28xx_byte_write(addr, 0x07, 0xC0);
        tp28xx_byte_write(addr, 0x0B, 0xC0);
        tp28xx_byte_write(addr, 0x2b, 0x4a);
        tp28xx_byte_write(addr, 0x2E, 0x60);

        tp28xx_byte_write(addr, 0x30, 0x48);
        tp28xx_byte_write(addr, 0x31, 0xBB);
        tp28xx_byte_write(addr, 0x32, 0x2E);
        tp28xx_byte_write(addr, 0x33, 0x90);

        tp28xx_byte_write(addr, 0x23, 0x50);
        tp28xx_byte_write(addr, 0x39, 0x42);
        tp28xx_byte_write(addr, 0x3A, 0x01);
        tp28xx_byte_write(addr, 0x4d, 0x0f);
        tp28xx_byte_write(addr, 0x4e, 0xff);


        //now TP2801A just support 2 lines, to disable line3&4, else IRQ is in trouble.
        tp28xx_byte_write(addr, 0x40, 0x01);
        tp28xx_byte_write(addr, 0x50, 0x00);
        tp28xx_byte_write(addr, 0x51, 0x00);
        tp28xx_byte_write(addr, 0x52, 0x00);
        tp28xx_byte_write(addr, 0x7F, 0x00);
        tp28xx_byte_write(addr, 0x80, 0x00);
        tp28xx_byte_write(addr, 0x81, 0x00);

        //0x50~0xb2 need bank switch
        tp28xx_byte_write(addr, 0x40, 0x00);
        //TX total 34bit, valid load 32bit
        tp28xx_byte_write(addr, 0x50, 0x00);
        tp28xx_byte_write(addr, 0x51, 0x0b);
        tp28xx_byte_write(addr, 0x52, 0x0c);
        tp28xx_byte_write(addr, 0x53, 0x19);
        tp28xx_byte_write(addr, 0x54, 0x78);
        tp28xx_byte_write(addr, 0x55, 0x21);
        tp28xx_byte_write(addr, 0x7e, 0x0f);   //

        // RX total 40bit, valid load 39bit
        tp28xx_byte_write(addr, 0x7F, 0x00);
        tp28xx_byte_write(addr, 0x80, 0x07);
        tp28xx_byte_write(addr, 0x81, 0x08);
        tp28xx_byte_write(addr, 0x82, 0x04);
        tp28xx_byte_write(addr, 0x83, 0x00);
        tp28xx_byte_write(addr, 0x84, 0x00);
        tp28xx_byte_write(addr, 0x85, 0x60);
        tp28xx_byte_write(addr, 0x86, 0x10);
        tp28xx_byte_write(addr, 0x87, 0x06);
        tp28xx_byte_write(addr, 0x88, 0xBE);
        tp28xx_byte_write(addr, 0x89, 0x39);
        tp28xx_byte_write(addr, 0x8A, 0x27);   //
        tp28xx_byte_write(addr, 0xB9, 0x0F);   //RX channel enable

    }
    else if (TP2802C == id[i])
    {
        //Init
        tp28xx_byte_write(addr, 0x06, 0xC0); //8bit BT1120 mode only
        tp28xx_byte_write(addr, 0x07, 0x0D);
        tp28xx_byte_write(addr, 0x08, 0xE0);
        tp28xx_byte_write(addr, 0x26, 0x12);
        tp28xx_byte_write(addr, 0x2A, 0x44);
        tp28xx_byte_write(addr, 0x2C, 0x0A);
        tp28xx_byte_write(addr, 0x2E, 0x60);
        tp28xx_byte_write(addr, 0x2F, 0x08);

        tp28xx_byte_write(addr, 0x30, 0x48);
        tp28xx_byte_write(addr, 0x31, 0xBB);
        tp28xx_byte_write(addr, 0x32, 0x2E);
        tp28xx_byte_write(addr, 0x33, 0x90);

        tp28xx_byte_write(addr, 0x38, 0xC0);
        tp28xx_byte_write(addr, 0x39, 0xCA);

        tp28xx_byte_write(addr, 0x3A, 0x02);
        tp28xx_byte_write(addr, 0x3D, 0x20);
        tp28xx_byte_write(addr, 0x3E, 0x02);

        // PLLs, Start address 0x42, Size = 4B
        tp2802_write_table(addr, 0x42, tbl_tp2802_common_pll, 4);

        // Rx Common, Start address 0x7E, Size = 13B
        tp2802_write_table(addr, 0x7E, tbl_tp2802_common_rx, 13);

        // IRQ Common, Start address 0xB3, Size = 6B
        //tp2802_write_table(addr, 0xB3, tbl_tp2802_common_irq, 6);

        // Output Enables, Start address 0x4B, Size = 11B
        //tp2802_write_table(addr, 0x4B, tbl_tp2802_common_oe, 11);
        tp2802_write_table(addr, 0x4B, tbl_tp2802_common_oe, 4);

        //TX total 34bit, valid load 32bit
        tp28xx_byte_write(addr, 0x50, 0x00);
        tp28xx_byte_write(addr, 0x51, 0x0b);
        tp28xx_byte_write(addr, 0x52, 0x0c);
        tp28xx_byte_write(addr, 0x53, 0x19);
        tp28xx_byte_write(addr, 0x54, 0x78);
        tp28xx_byte_write(addr, 0x55, 0x21);
        //tp28xx_byte_write(addr, 0x56, 0x02); //should be 0 when disable sending
        //tp28xx_byte_write(addr, 0x7e, 0x2f);
    }
}

#ifdef CONFIG_HISI_SNAPSHOT_BOOT
static int tp2823_freeze(struct himedia_device* pdev)
{
    printk(KERN_ALERT "%s  %d\n", __FUNCTION__, __LINE__);
    return 0;
}

static int tp2823_restore(struct himedia_device* pdev)
{  
    printk(KERN_ALERT "%s  %d\n", __FUNCTION__, __LINE__);
    return 0;
}
#endif

static int i2c_client_init(void)
{
    struct i2c_adapter* i2c_adap;

    // use i2c0 
    i2c_adap = i2c_get_adapter(0);
    tp2823_client = i2c_new_device(i2c_adap, &hi_info);
    i2c_put_adapter(i2c_adap);

    return 0;
}

static void i2c_client_exit(void)
{
    i2c_unregister_device(tp2823_client);
}


static struct file_operations tp2823_fops =
{
    .owner      = THIS_MODULE,
    .unlocked_ioctl  = tp2802_ioctl,
    .open       = tp2802_open,
    .release    = tp2802_close
};

#ifdef CONFIG_HISI_SNAPSHOT_BOOT
struct himedia_ops stTp823DrvOps =
{
    .pm_freeze = tp2823_freeze,
    .pm_restore  = tp2823_restore
};
#else
static struct miscdevice tp2823_dev =
{
    .minor		= MISC_DYNAMIC_MINOR,
    .name		= "tp2823dev",
    .fops  		= &tp2823_fops,
};
#endif


module_param(mode, uint, S_IRUGO);
module_param(chips, uint, S_IRUGO);
module_param(output, uint, S_IRUGO);

static int __init tp2802_module_init(void)
{
    int ret = 0, i = 0, val = 0;
    unsigned char i2c_addr;
    /*
    	// 1st check the module parameters
    	if ((mode < TP2802_1080P25) || (mode > TP2802_720P60))
    	{
    		printk("TP2802 module param 'mode' Invalid!\n");
    		return FAILURE;
    	}
    */
    printk("TP2802 driver version %d.%d.%d loaded\n",
           (TP2802_VERSION_CODE >> 16) & 0xff,
           (TP2802_VERSION_CODE >>  8) & 0xff,
           TP2802_VERSION_CODE & 0xff);

    if (chips <= 0 || chips > 4)
    {
        printk("TP2802 module param 'chips' invalid value:%d\n", chips);
        return FAILURE;
    }

    
#ifdef CONFIG_HISI_SNAPSHOT_BOOT
        snprintf(s_stTp2823Device.devfs_name, sizeof(s_stTp2823Device.devfs_name), DEV_NAME);
        s_stTp2823Device.minor  = HIMEDIA_DYNAMIC_MINOR;
        s_stTp2823Device.fops   = &tp2823_fops;
        s_stTp2823Device.drvops = &stTp823DrvOps;
        s_stTp2823Device.owner  = THIS_MODULE;
    
        ret = himedia_register(&s_stTp2823Device);
        if (ret)
        {
            printk(0, "could not register tp2802_dev device");
            return -1;
        }
#else
        /* register misc device*/
        ret = misc_register(&tp2823_dev);
        if (ret)
        {
            printk("could not register tp2802_dev device");
            return -1;
        }
#endif   
    
    i2c_client_init();

    /* initize each tp2802*/
    for (i = 0; i < chips; i ++)
    {

        i2c_addr = tp2802_i2c_addr[i];
        val = tp28xx_byte_read(i2c_addr, 0xfe);

        if (0x28 == val)
        { printk("Detected TP28xx \n"); }
        else
        { printk("Invalid chip %2x\n", val); }

        id[i] = tp28xx_byte_read(i2c_addr, 0xff);
        id[i] <<= 8;
        id[i] += tp28xx_byte_read(i2c_addr, 0xfd);

        printk("Detected ID&revision %04x\n", id[i]);

        tp2802_comm_init(i);

        tp2802_set_video_mode(i2c_addr, mode, 4);
        //PLL reset
        val = tp28xx_byte_read(i2c_addr, 0x44);
        tp28xx_byte_write(i2c_addr, 0x44, val | 0x40);
        msleep(10);
        tp28xx_byte_write(i2c_addr, 0x44, val);

        //ADC reset
        tp28xx_byte_write(i2c_addr, 0x40, 0x04); //write 4 channels
        tp28xx_byte_write(i2c_addr, 0x3B, 0x33);
        tp28xx_byte_write(i2c_addr, 0x3B, 0x03);

        //soft reset
        val = tp28xx_byte_read(i2c_addr, 0x06);
        tp28xx_byte_write(i2c_addr, 0x06, 0x80 | val);

    }

    /* Redirect the VOU1120 port to Bt1120 input port */
    /*
        for (i = 0; i < 19; i++)
        {
            val = HW_REG( MUXCTRL_REG(0x0EC + (i * 4)) );
            HW_REG( MUXCTRL_REG(0x0EC + (i * 4)) ) = val & 0xFFFFFFF8;
            ret = HW_REG( MUXCTRL_REG(0x0EC + (i * 4)) );
        }
    */

#if (WDT)
    ret = TP2802_watchdog_init();

    if (ret)
    {
        #ifdef CONFIG_HISI_SNAPSHOT_BOOT        
        misc_deregister(&tp2823_dev);
        #endif
        
        printk("ERROR: could not create watchdog\n");
        return ret;
    }

#endif

    printk("TP2823 Driver Init Successful!\n");

    return SUCCESS;
}

static void __exit tp2802_module_exit(void)
{
#if (WDT)
    TP2802_watchdog_exit();
#endif   

#ifdef CONFIG_HISI_SNAPSHOT_BOOT
    himedia_unregister(&s_stTp2823Device);
#else
    misc_deregister(&tp2823_dev);
#endif	
    i2c_client_exit();
}

/******************************************************************************
 *
 * TP2802_watchdog_deamon()

 *
 ******************************************************************************/
static int TP2802_watchdog_deamon(void* data)
{
    unsigned long flags;
    int iChip, i = 0;
    struct sched_param param = { .sched_priority = 99 };

    tp2802wd_info* wdi;

    struct timeval start, end;
    int interval;
    unsigned int status, cvstd, gain, agc, tmp;
    const unsigned char UV_offset[16] = {0x00, 0x00, 0x04, 0x04, 0x08, 0x08, 0x0c, 0x0c, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10};
    //const unsigned char UV_offset[16]={0x00,0x00,0x0b,0x0b,0x16,0x16,0x21,0x21,0x21,0x21,0x21,0x21,0x21,0x21,0x21,0x21};
    //const unsigned char AGC_table[16]={0x50,0x50,0x60,0x60,0x70,0x80,0x90,0xa0,0xb0,0xc0,0xd0,0xd0,0xd0,0xd0,0xd0,0xd0};
    const unsigned char TP2802D_CGAIN[16]   = {0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 0x60, 0x50, 0x40, 0x38, 0x30};
    const unsigned char TP2802D_CGAINMAX[16] = {0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x4a, 0x44, 0x44, 0x44, 0x44, 0x44};
    const unsigned char TP2823_CGAINMAX[16] = {0x4a, 0x47, 0x46, 0x45, 0x44, 0x44, 0x43, 0x43, 0x42, 0x42, 0x42, 0x42, 0x40, 0x40, 0x40, 0x40};

    printk("TP2802_watchdog_deamon: start!\n");

    sched_setscheduler(current, SCHED_FIFO, &param);
    current->flags |= PF_NOFREEZE;

    set_current_state(TASK_INTERRUPTIBLE);

    while (watchdog_state != WATCHDOG_EXIT)
    {
        spin_lock_irqsave(&watchdog_lock, flags);

        do_gettimeofday(&start);

        for (iChip = 0; iChip < chips; iChip++)
        {

            wdi = &watchdog_info[iChip];

            for (i = 0; i < CHANNELS_PER_CHIP; i++) //scan four inputs:
            {
                if (SCAN_DISABLE == wdi->scan[i]) { continue; }

                tp2802_set_reg_page(wdi->addr, i);

                status = tp28xx_byte_read(wdi->addr, 0x01);

                if (0x08 == tp28xx_byte_read(wdi->addr, 0x2f))
                {
                    tmp = tp28xx_byte_read(wdi->addr, 0x04);

                    if (tmp >= 0x30) { status |= 0x80; }
                }

                //state machine for video checking
                if (status & FLAG_LOSS)
                {

                    if (VIDEO_UNPLUG == wdi->state[i]) //still no video
                    {
                        if (wdi->count[i] < MAX_COUNT) { wdi->count[i]++; }

                        continue;
                    }
                    else //switch to no video
                    {
                        wdi->state[i] = VIDEO_UNPLUG;
                        wdi->count[i] = 0;
                        wdi->mode[i] = INVALID_FORMAT;
                        TP28xx_reset_default(id[iChip], wdi->addr, i);
                        tp2802_set_video_mode(wdi->addr, TP2802_720P25, i);
                        printk("video loss ch%02x addr%2x\r\n", i, wdi->addr );
                    }

                }
                else
                {
                    if ( FLAG_LOCKED == (status & FLAG_LOCKED) ) //video locked
                    {
                        if (VIDEO_LOCKED == wdi->state[i])
                        {
                            if (wdi->count[i] < MAX_COUNT) { wdi->count[i]++; }
                        }
                        else if (VIDEO_UNPLUG == wdi->state[i])
                        {
                            wdi->state[i] = VIDEO_IN;
                            wdi->count[i] = 0;
                            printk("video in ch%02x addr%2x\r\n", i, wdi->addr);
                        }
                        else
                        {
                            wdi->state[i] = VIDEO_LOCKED;
                            wdi->count[i] = 0;
                            printk("video locked ch%02x addr%2x\r\n", i, wdi->addr);
                        }
                    }
                    else  //video in but unlocked
                    {
                        if (VIDEO_UNPLUG == wdi->state[i])
                        {
                            wdi->state[i] = VIDEO_IN;
                            wdi->count[i] = 0;
                            printk("video in ch%02x addr%2x\r\n", i, wdi->addr);
                        }
                        else if (VIDEO_LOCKED == wdi->state[i])
                        {
                            wdi->state[i] = VIDEO_UNLOCK;
                            wdi->count[i] = 0;
                            printk("video unstable ch%02x addr%2x\r\n", i, wdi->addr);
                        }
                        else
                        {
                            if (wdi->count[i] < MAX_COUNT) { wdi->count[i]++; }

                            if (VIDEO_UNLOCK == wdi->state[i] && wdi->count[i] > 2)
                            {
                                wdi->state[i] = VIDEO_IN;
                                wdi->count[i] = 0;
                                TP28xx_reset_default(id[iChip], wdi->addr, i);
                                printk("video unlocked ch%02x addr%2x\r\n", i, wdi->addr);
                            }
                        }
                    }

                    if ( wdi->force[i] ) //manual reset for V1/2 switching
                    {

                        wdi->state[i] = VIDEO_UNPLUG;
                        wdi->count[i] = 0;
                        wdi->mode[i] = INVALID_FORMAT;
                        TP28xx_reset_default(id[iChip], wdi->addr, i);
                        tp2802_set_video_mode(wdi->addr, TP2802_720P25, i);
                        printk("video reset ch%02x addr%2x\r\n", i, wdi->addr );
                        wdi->force[i] = 0;
                    }
                }
                //printk("video state %2x detected ch%02x count %4x\r\n", wdi->state[i], i, wdi->count[i] );
                if ( VIDEO_IN == wdi->state[i])
                {
                    //tp28xx_SYSCLK_V1(wdi->addr, i);
                    //tp28xx_byte_write(wdi->addr, 0x35, 0x45);
                    cvstd = tp28xx_byte_read(wdi->addr, 0x03);
                    printk("video format %2x detected ch%02x addr%2x\r\n", cvstd, i, wdi->addr );

                    cvstd &= 0x0f;

                    if ( TP2802_SD == (cvstd & 0x07) )
                    {
                        if (TP2823 == id[iChip])
                        {
                            if (wdi->count[i] & 0x01)
                            {
                                wdi-> mode[i] = TP2802_PAL;
                                TP2823_PAL_DataSet(wdi->addr);

                                if (0x02 == i) { tp28xx_byte_write(wdi->addr, 0x0d, 0x1); }

                                printk("set PAL format ch%02x addr%2x\r\n", i, wdi->addr );
                            }
                            else
                            {
                                wdi-> mode[i] = TP2802_NTSC;
                                TP2823_NTSC_DataSet(wdi->addr);

                                if (0x02 == i) { tp28xx_byte_write(wdi->addr, 0x0d, 0x0); }

                                printk("set NTSC format ch%02x addr%2x\r\n", i, wdi->addr );
                            }

                            tp282x_SYSCLK_V2(wdi->addr, i);
                        }
                    }
                    else if (TP2802_720P30V2 == cvstd || TP2802_720P25V2 == cvstd)
                    {

                        if (TP2822 == id[iChip] || TP2823 == id[iChip])
                        {
                            wdi-> mode[i] = cvstd;
                            tp2802_set_video_mode(wdi->addr, cvstd, i);
                            tp282x_SYSCLK_V2(wdi->addr, i);

                            //tp28xx_byte_write(wdi->addr, 0x35, 0x25);
                            if (TP2823 == id[iChip]) { TP2823_V2_DataSet(wdi->addr); }

                        }
                        else
                        {
                            wdi-> mode[i] = cvstd & 0x07;
                            tp2802_set_video_mode(wdi->addr, wdi-> mode[i], i);
                        }

                    }
                    else if ((cvstd & 0x07) < 6 )
                    {
                        //wdi-> std[i] = cvstd;
                        wdi-> mode[i] = cvstd & 0x07;
                        tp2802_set_video_mode(wdi->addr, wdi-> mode[i], i);

                        if (TP2822 == id[iChip] || TP2823 == id[iChip])
                        {

                            tp282x_SYSCLK_V1(wdi->addr, i);

                            //tp28xx_byte_write(wdi->addr, 0x35, 0x45);
                            if (TP2823 == id[iChip]) { TP2823_V1_DataSet(wdi->addr); }

                        }
                    }
                }

#define EQ_COUNT 10

                if ( VIDEO_LOCKED == wdi->state[i]) //check signal lock
                {
                    if (0 == wdi->count[i])
                    {

                        if (TP2802C == id[iChip] )
                        {

                            tp28xx_byte_write(wdi->addr, 0x07, 0x0d);

                        }
                        else if (TP2802D == id[iChip] )
                        {

                            if ( (TP2802_720P30 == wdi-> mode[i]) || (TP2802_720P25 == wdi-> mode[i]) )
                            {
                                if ( 0x08 & tp28xx_byte_read(wdi->addr, 0x03))
                                {
                                    tmp = tp28xx_byte_read(wdi->addr, 0xf5);
                                    tmp |= SYS_MODE[i];
                                    tp28xx_byte_write(wdi->addr, 0xf5, tmp);

                                    tmp = tp28xx_byte_read(wdi->addr, CLK_ADDR[i]);
                                    tmp &= CLK_AND[i];
                                    tmp |= CLK_MODE[i];
                                    tp28xx_byte_write(wdi->addr, CLK_ADDR[i], tmp);

                                    printk("720P V2 Detected ch%02x addr%2x\r\n", i, wdi->addr);

                                    if (TP2802_720P30 == wdi-> mode[i]) //to speed the switching
                                    {
                                        wdi-> mode[i] = TP2802_720P30V2;
                                        tp2802_set_video_mode(wdi->addr, TP2802_720P60, i);
                                    }
                                    else if (TP2802_720P25 == wdi-> mode[i])
                                    {
                                        wdi-> mode[i] = TP2802_720P25V2;
                                        tp2802_set_video_mode(wdi->addr, TP2802_720P50, i);
                                    }
                                }
                            }
                        }
                    }
                    else if (1 == wdi->count[i])
                    {
                        if (TP2802C == id[iChip])
                        {
                            tp28xx_byte_write(wdi->addr, 0x2F, 0x02);
                            agc = tp28xx_byte_read(wdi->addr, 0x04);
                            printk("AGC=0x%04x ch%02x\r\n", agc, i);
                            agc += tp28xx_byte_read(wdi->addr, 0x04);
                            agc += tp28xx_byte_read(wdi->addr, 0x04);
                            agc += tp28xx_byte_read(wdi->addr, 0x04);
                            agc &= 0x3f0;
                            agc >>= 1;
                            agc += 0x80;

                            if (agc > 0x1c0) { agc = 0x1c0; }

                            tp28xx_byte_write(wdi->addr, 0x08, agc & 0xff);
                            printk("AGC=0x%04x ch%02x\r\n", agc, i);
                            tp28xx_byte_write(wdi->addr, 0x07, 0x0f);
                        }

                    }
                    else if ( wdi->count[i] < EQ_COUNT - 3)
                    {

                    }
                    else if ( wdi->count[i] < EQ_COUNT)
                    {
                        //wdi->count[i]++;
                        gain = tp28xx_byte_read(wdi->addr, 0x03);
                        wdi->gain[i][EQ_COUNT - wdi->count[i]] = gain & 0xf0;
                        printk("Egain=0x%02x ch%02x\r\n", gain, i);

                    }
                    else if ( wdi->count[i] == EQ_COUNT )
                    {
                        wdi->count[i]--;
                        wdi->gain[i][3] = wdi->gain[i][2];
                        wdi->gain[i][2] = wdi->gain[i][1];
                        wdi->gain[i][1] = wdi->gain[i][0];

                        gain = tp28xx_byte_read(wdi->addr, 0x03);
                        wdi->gain[i][0] = gain & 0xf0;

                        if (abs(wdi->gain[i][3] - wdi->gain[i][0]) < 0x20 && abs(wdi->gain[i][2] - wdi->gain[i][0]) < 0x20 && abs(wdi->gain[i][1] - wdi->gain[i][0]) < 0x20 )
                        {

                            wdi->count[i]++;

                            if (TP2822 == id[iChip])
                            {
                                tmp = tp28xx_byte_read(wdi->addr, 0x03);
                                printk("result Egain=0x%02x ch%02x\r\n", tmp, i);
                                tmp &= 0xf0;
                                tp28xx_byte_write(wdi->addr, 0x07, tmp >> 2); // manual mode
                                tmp >>= 4;
                                tp28xx_byte_write(wdi->addr, 0x2b, TP2823_CGAINMAX[tmp]);
                            }
                            else if (TP2802C == id[iChip] )
                            {
                                gain = wdi->gain[i][0];
                                gain += wdi->gain[i][1];
                                gain += wdi->gain[i][2];
                                gain += wdi->gain[i][3];
                                gain >>= 2;
                                gain &= 0xf0;

                                if (gain > 0x70) { gain = 0x70; }

                                printk("result Egain=0x%02x ch%02x\r\n", gain, i);

                                if (gain < 0x20) { tp28xx_byte_write(wdi->addr, 0x3a, 0x0a); }

                                tp28xx_byte_write(wdi->addr, 0x07, gain | 0x0b);
                                tp28xx_byte_write(wdi->addr, 0x13, UV_offset[gain >> 4]); //color offset adjust

                            }
                            else if (TP2802D == id[iChip] )
                            {

                                tmp = tp28xx_byte_read(wdi->addr, 0x03);
                                printk("result Egain=0x%02x ch%02x\r\n", tmp, i);

                                tmp &= 0xf0;
                                tmp >>= 4;
                                tp28xx_byte_write(wdi->addr, 0x2e, TP2802D_CGAIN[tmp]);
                                tp28xx_byte_write(wdi->addr, 0x2b, TP2802D_CGAINMAX[tmp]);

                                if (tmp < 0x02)
                                {
                                    tp28xx_byte_write(wdi->addr, 0x3A, TP2802D_EQ_SHORT);  // for short cable
                                    tp28xx_byte_write(wdi->addr, 0x2e, TP2802D_CGAIN_SHORT);
                                }

                                if (tmp > 0x03 && ((TP2802_720P30V2 == wdi-> mode[i]) || (TP2802_720P25V2 == wdi-> mode[i])) )
                                {
                                    tp28xx_byte_write(wdi->addr, 0x3A, 0x09);  // long cable
                                    tp28xx_byte_write(wdi->addr, 0x07, 0x40);  // for long cable
                                    tp28xx_byte_write(wdi->addr, 0x09, 0x20);  // for long cable
                                    tmp = tp28xx_byte_read(wdi->addr, 0x06);
                                    tmp |= 0x80;
                                    tp28xx_byte_write(wdi->addr, 0x06, tmp);
                                }
                            }
                            else if (TP2823 == id[iChip])
                            {
                                if ((TP2802_PAL == wdi-> mode[i]) || (TP2802_NTSC == wdi-> mode[i]))
                                {
                                    tp28xx_byte_write(wdi->addr, 0x2b, 0x70);
                                }
                                else
                                {
                                    tmp = tp28xx_byte_read(wdi->addr, 0x03);
                                    printk("result Egain=0x%02x ch%02x\r\n", tmp, i);
                                    tmp >>= 4;
                                    tp28xx_byte_write(wdi->addr, 0x2b, TP2823_CGAINMAX[tmp]);
                                }

                            }

                        }
                    }

                    else if (wdi->count[i] == EQ_COUNT + 1)
                    {
                        if (TP2822 == id[iChip] || TP2823 == id[iChip])
                        {
                            tp2802_manual_agc(wdi->addr, i);

                        }
                        else if ( TP2802D == id[iChip])
                        {
                            if ( (TP2802_720P30V2 != wdi-> mode[i]) && (TP2802_720P25V2 != wdi-> mode[i]) )
                            {
                                tp2802_manual_agc(wdi->addr, i);
                            }
                        }

                    }
                    else if (wdi->count[i] == EQ_COUNT + 5)
                    {
                        if ( TP2802D == id[iChip])
                        {
                            if ( (TP2802_720P30V2 == wdi-> mode[i]) || (TP2802_720P25V2 == wdi-> mode[i]) )
                            {
                                tp2802_manual_agc(wdi->addr, i);
                            }
                        }

                    }

                }
            }
        }

        do_gettimeofday(&end);

        interval = 1000000 * (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec);

        //printk("WDT elapsed time %d.%dms\n", interval/1000, interval%1000);
        //printk("cx25930_watchdog_deamon: running!\n");
        spin_unlock_irqrestore(&watchdog_lock, flags);

        /* sleep 1 seconds */
        schedule_timeout_interruptible(msecs_to_jiffies(500) + 1);
    }

    set_current_state(TASK_RUNNING);

    printk("TP2802_watchdog_deamon: exit!\n");

    return 0;

}


/******************************************************************************
 *
 * cx25930_watchdog_init()

 *
 ******************************************************************************/
int __init TP2802_watchdog_init(void)
{
    struct task_struct* p_dog;
    int i, j;

    watchdog_state = WATCHDOG_RUNNING;
    memset(&watchdog_info, 0, sizeof(watchdog_info));

    for (i = 0; i < MAX_CHIPS; i++)
    {
        watchdog_info[i].addr = tp2802_i2c_addr[i];

        for (j = 0; j < CHANNELS_PER_CHIP; j++)
        {
            watchdog_info[i].count[j] = 0;
            //watchdog_info[i].locked[j] = 0;
            //watchdog_info[i].loss[j] = 1;
            watchdog_info[i].mode[j] = INVALID_FORMAT;
            watchdog_info[i].scan[j] = SCAN_ENABLE;
            watchdog_info[i].state[j] = VIDEO_UNPLUG;
            //watchdog_info[i].std[j] = INVALID_FORMAT;
        }
    }

    p_dog = kthread_create(TP2802_watchdog_deamon, NULL, "WatchDog");

    if ( IS_ERR(p_dog) < 0)
    {
        printk("TP2802_watchdog_init: create watchdog_deamon failed!\n");
        return -1;
    }

    wake_up_process(p_dog);

    task_watchdog_deamon = p_dog;

    printk("TP2802_watchdog_init: done!\n");

    return 0;
}

/******************************************************************************
 *
 * cx25930_watchdog_exit()

 *
 ******************************************************************************/
void __exit TP2802_watchdog_exit(void)
{
    struct task_struct* p_dog = task_watchdog_deamon;
    watchdog_state = WATCHDOG_EXIT;

    if ( p_dog == NULL )
    { return; }

    wake_up_process(p_dog);

    kthread_stop(p_dog);

    yield();

    task_watchdog_deamon = NULL;

    printk("TP2802_watchdog_exit: done!\n");
}
module_init(tp2802_module_init);
module_exit(tp2802_module_exit);

