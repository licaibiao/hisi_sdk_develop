#ifndef HST3520D_VI_SAMPLE
#define HST3520D_VI_SAMPLE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <math.h>
#include <unistd.h>

#include "hstSdkStruct.h"

#ifdef __cplusplus
extern "C"{
#endif

int Hst3520D_Sample_GetViAttr(VI_ATTR_S *pstViAttr);
int Hst3520D_Sample_SetViAttr(VI_ATTR_S *pstViAttr);

int Hst3520D_Sample_DefaultD1ViAttr();
int Hst3520D_Sample_Default720PViAttr();
int Hst3520D_Sample_Default1080PViAttr();
int Hst3520D_Sample_Default_ViGroupAttr(unsigned char u8vigroup,PIC_SIZE_E enPicSize,unsigned char *pu8ChnCnt);

HI_S32 VI_WISDOM_NVP6124_CfgByGroup(unsigned char u8Group,VIDEO_NORM_E enVideoMode,SAMPLE_VI_6134_MODE_E enViMode);
HI_S32 VI_WISDOM_NVP6134_CfgByGroup(unsigned char u8Group,VIDEO_NORM_E enVideoMode,SAMPLE_VI_6134_MODE_E enViMode);
int Hst3520_VbSet_ByGroup(VB_CONF_S *pstVbConf);
int Hst3520D_Sample_Start_GroupVi(unsigned char u8vigroup);

int Hst3520D_Sample_StartVi();
int Hst3520D_Sample_StopVi();

int Hst3520D_Sample_GetVideoState(unsigned int *pu32VideoState);
int Hst3520D_Sample_GetCurVencSzie(VIDEO_CHN_E enVeChnNo);
//int Hst3520D_Sample_SelfAdaptationViAttr();
int Hst3520D_Sample_SelfAdaptationViAttr(CAMERA_INPUT_STATUS_S *pastCameraStatus,CAMERA_RESOLUTION_STATUS_E	* paeGroupStat);
#ifdef __cplusplus
}
#endif

#endif
