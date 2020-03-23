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

static unsigned int viu_virt;

extern unsigned int nvp6124_mode;
//extern unsigned int nvp6124_slave_addr[4];

void nvp6124_hi3520_viu_init(void)
{
	printk("nvp6124_hi3520_viu_init...\n");
	//viu_virt = ioremap(0x10100000, 0x100000);
    viu_virt = IO_ADDRESS(0x10100000);
	printk("viu_virt  = %x\n", viu_virt);
}

void hi3520_init_blank_data(unsigned int ch)
{
	nvp6124_hi3520_viu_init();

	if(viu_virt == 0)
	{
		printk("hi3520_init_blank_data Error\n");
		return;
	}
	
	*(unsigned int *)(viu_virt + VIU_CH_CTRL) |= 0x3000;
	*(unsigned int *)(viu_virt + VIU_ANC0_START) = 0x01000000;
	*(unsigned int *)(viu_virt + VIU_ANC0_SIZE) = 0x20;
	*(unsigned int *)(viu_virt + VIU_ANC1_START) = 0x01000020;
	*(unsigned int *)(viu_virt + VIU_ANC1_SIZE) = 0x20;
}

void nvp6124_motion_init(void)
{
	printk("nvp6124_motion_init...\n");
}


/*
 * Motion Area Map
 * Motion Block 16(H) * 12(V)
 * x -> not used
 * o -> 1 : Motion 0 : No Motion
 *
 *  m_area.m_info[x(0~11)] = xxxx oooo xxxx oooo xxxx oooo xxxx oooo -> 32Bit
 */
					 
nvp6124_motion_area nvp6124_get_motion_info(unsigned int ch)
{
	//int i;
	nvp6124_motion_area m_area;
    
    memset(&m_area, 0,sizeof(nvp6124_motion_area));

	if(viu_virt == 0)
	{
		printk("nvp6124_get_motion_info Error\n");
		return m_area;
	}

	m_area.m_info[0] = (*(unsigned int *)(viu_virt + VIU_BLANK_DATA_ADDR + 0x00)) & 0x0f0f0f0f;
	m_area.m_info[1] = (*(unsigned int *)(viu_virt + VIU_BLANK_DATA_ADDR + 0x04)) & 0x0f0f0f0f;
	m_area.m_info[2] = (*(unsigned int *)(viu_virt + VIU_BLANK_DATA_ADDR + 0x08)) & 0x0f0f0f0f;
	m_area.m_info[3] = (*(unsigned int *)(viu_virt + VIU_BLANK_DATA_ADDR + 0x0c)) & 0x0f0f0f0f;
	m_area.m_info[4] = (*(unsigned int *)(viu_virt + VIU_BLANK_DATA_ADDR + 0x10)) & 0x0f0f0f0f;
	m_area.m_info[5] = (*(unsigned int *)(viu_virt + VIU_BLANK_DATA_ADDR + 0x14)) & 0x0f0f0f0f;
	m_area.m_info[6] = (*(unsigned int *)(viu_virt + VIU_BLANK_DATA_ADDR + 0x18)) & 0x0f0f0f0f;
	m_area.m_info[7] = (*(unsigned int *)(viu_virt + VIU_BLANK_DATA_ADDR + 0x1c)) & 0x0f0f0f0f;
	m_area.m_info[8] = (*(unsigned int *)(viu_virt + VIU_BLANK_DATA_ADDR + 0x20)) & 0x0f0f0f0f;
	m_area.m_info[9] = (*(unsigned int *)(viu_virt + VIU_BLANK_DATA_ADDR + 0x24)) & 0x0f0f0f0f;
	m_area.m_info[10] = (*(unsigned int *)(viu_virt + VIU_BLANK_DATA_ADDR + 0x28)) & 0x0f0f0f0f;
	m_area.m_info[11] = (*(unsigned int *)(viu_virt + VIU_BLANK_DATA_ADDR + 0x2c)) & 0x0f0f0f0f;

	printk("---------------------------\n");
	printk("%#x\n", m_area.m_info[0]);
	printk("%#x\n", m_area.m_info[1]);
	printk("%#x\n", m_area.m_info[2]);
	printk("%#x\n", m_area.m_info[3]);
	printk("%#x\n", m_area.m_info[4]);
	printk("%#x\n", m_area.m_info[5]);
	printk("%#x\n", m_area.m_info[6]);
	printk("%#x\n", m_area.m_info[7]);
	printk("%#x\n", m_area.m_info[8]);
	printk("%#x\n", m_area.m_info[9]);
	printk("%#x\n", m_area.m_info[10]);
	printk("%#x\n", m_area.m_info[11]);

	return m_area;
}

void nvp6124_motion_display(unsigned int ch, unsigned int on)
{
	
}

void nvp6124_motion_sensitivity(unsigned int sens[16])
{
}

void nvp6124_motion_area_mask(unsigned int sens[16])
{
}




