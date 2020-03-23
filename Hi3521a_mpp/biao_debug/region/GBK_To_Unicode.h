/************************************************************
*Copyright (C),lcb0281at163.com lcb0281atgmail.com
*BlogAddr: caibiao-lee.blog.csdn.net
*FileName: GBK_To_Unicode.h
*Description:字体格式转换
*Date:     2020-02-03
*Author:   Caibiao Lee
*Version:  V1.0
*Others:
*History:
***********************************************************/
#ifndef _GBK_TO_UNICODE_H_
#define _GBK_TO_UNICODE_H_
#include <unistd.h> 
#include <dirent.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <signal.h>

#include <sys/types.h>
#include <termios.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h> 
#include <string.h>

extern  unsigned short zz_gbk2uni(unsigned char ch, unsigned char cl);
extern  int enc_unicode_to_utf8_one(unsigned long unic, unsigned char *pOutput,  
        int outSize);
#endif





