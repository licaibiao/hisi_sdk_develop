/*********************************************************************
 * Project Name : NVP1918C
 *
 * Copyright 2011 by NEXTCHIP Co., Ltd.
 * All rights reserved.
 *
 *********************************************************************/
#ifndef __COAX_PROTOCOL_NVP6124_H__
#define __COAX_PROTOCOL_NVP6124_H__
#define PACKET_MODE	0x0B

#define AHD2_PEL_D0	0x20
#define AHD2_FHD_D0	0x10
#define AHD2_PEL_OUT	0x0C
#define AHD2_PEL_BAUD	0x02
#define AHD2_PEL_LINE	0x07
#define AHD2_PEL_SYNC	0x0D
#define AHD2_PEL_EVEN	0x2F
#define AHD2_FHD_BAUD	0x00
#define AHD2_FHD_LINE	0x03
#define AHD2_FHD_LINES	0x05
#define AHD2_FHD_BYTE	0x0A
#define AHD2_FHD_MODE	0x0B
#define AHD2_FHD_OUT	0x09
#define ACP_CLR			0x3A

#define ACP_CAM_STAT	0x55
#define ACP_REG_WR		0x60
#define ACP_REG_RD		0x61
#define ACP_MODE_ID		0x60

#define ACP_RX_D0		0x78
 
extern void nvp6124_pelco_coax_mode(unsigned char ch);
extern unsigned char nvp6124_pelco_command(unsigned char pel_ch, unsigned char command);
extern void init_acp(unsigned char ch);
extern void acp_each_setting(unsigned char ch);
extern unsigned char read_acp_status(unsigned char ch);
extern unsigned char read_acp_pattern(unsigned char ch);
extern void init_acp_camera_status(unsigned char ch);
extern void init_acp_reg_wr(unsigned char ch);
extern void init_acp_reg_rd(unsigned char ch);
extern void acp_reg_rx_clear(unsigned char ch);
extern void set_acp_reg_wr(unsigned char bank, unsigned char addr, unsigned char data);
extern void get_acp_reg_rd(unsigned char bank, unsigned char addr);
extern void acp_read(nvp6124_input_videofmt *pvideoacp, unsigned char ch);
extern void acp_sw_ver_transfer(unsigned char ch);
extern unsigned char acp_isp_read(unsigned char ch, unsigned int reg_addr);

#endif
