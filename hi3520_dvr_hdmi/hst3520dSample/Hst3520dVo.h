#ifndef HST3520D_VO_SAMPLE
#define HST3520D_VO_SAMPLE

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

int Hst3520D_Sample_GetVoAttr(VO_ATTR_S * pstVoAttr);
int Hst3520D_Sample_SetVoAttr(VO_ATTR_S * pstVoAttr);

int Hst3520D_Sample_VoModeGet(SAMPLE_VO_MODE_E *penVoMode,unsigned int u32WndNum);
int Hst3520D_Sample_DefaultCVBSVoAttr();
int Hst3520D_Sample_StartCVBSVo();
int Hst3520D_Sample_StopCVBSVo();


int Hst3520D_Sample_GetHdmiVoAttr(VO_ATTR_S * pstVoAttr);
int Hst3520D_Sample_SetHdmiVoAttr(VO_ATTR_S * pstVoAttr);

int Hst3520D_Sample_VoHdmiModeGet(SAMPLE_VO_MODE_E *penVoMode,unsigned int u32WndNum);
int Hst3520D_Sample_DefaultHdmiVoAttr();
int Hst3520D_Sample_StartHdmiVo();
int Hst3520D_Sample_StopHdmiVo();


#ifdef __cplusplus
}
#endif

#endif
