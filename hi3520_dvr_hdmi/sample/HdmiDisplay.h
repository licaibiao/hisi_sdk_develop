#ifndef __HDMI_DISPLAY_H__
#define __HDMI_DISPLAY_H__

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<sys/types.h>
#include<termios.h>
#include<sys/types.h>   
#include<sys/stat.h>    
#include<fcntl.h>
#include<unistd.h>
#include<sys/ioctl.h>

unsigned int u32KeyValueStatus;

unsigned int read_thread_status;
unsigned int show_thread_status;
unsigned int IRkey_thread_status;


unsigned char g_u8LastShowMode;


pthread_t read_id;
pthread_t show_id;
pthread_t irkey_id;




#endif

