#ifndef __SAMPLE_VOI_H__
#define __SAMPLE_VOI_H__

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


#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

int Hst3520D_Mpp_init(void);
int OneWindowsDisplayChannel(unsigned char u8ChNum);
int TwoWindowsDispalyCha_1_2(void);
int TwoWindowsDispalyCha_3_4(void) ;
int ThreeWindowsDispalyCha_1_2_3_mode01(void) ;
int ThreeWindowsDispalyCha_1_2_3_mode02(void) ;
int ThreeWindowsDispalyCha_1_2_3_mode03(void);
int FourWindowsDispalyCha_1_2_3_4(void) ;


#ifdef __cplusplus
}
#endif

#endif



