#ifndef HST3520D_CHN_ADAPT
#define HST3520D_CHN_ADAPT

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
#include <sys/select.h>
#include <sys/time.h>
#include <time.h>

#include "sample_comm.h"
#include "hstSdkStruct.h"

#ifdef __cplusplus
extern "C"{
#endif


typedef struct
{
    VIDEO_CHN_E enVQueChn;
    HI_S32      s32PhyChn;
    VENC_CHN    s32VencChn;
    VPSS_GRP    s32VpssGrp;
    VPSS_CHN    s32VpssChn;
    HI_S32      s32ViChn; //对应Vi设备号add by tmf 20181027
}VIDEO_CHN_CFG_S;

typedef struct
{
    AUDIO_CHN_E enAQueChn;
    HI_U32      u32PhyChn;
    AENC_CHN    s32AencChn;
    HI_S32      s32AiChn; //对应Ai设备号add by tmf 20181027
}AUDIO_CHN_CFG_S;

HI_S32 Hst3520d_Adapt_Probe_CodecType();
HI_S32 Hst3520d_Adapt_Probe_CodecNum();
HI_S32 Hst3520d_Adapt_Probe_VideoInputFMT();

HI_S32 Hst3520d_Adapt_Probe_Nvp6124FMT(nvp6124_input_videofmt *pstVideofmt);
HI_S32 Hst3520d_Adapt_Probe_Nvp6134FMT(nvp6124_input_videofmt *pstVideofmt);

HI_U32 Hst3520d_Adapt_Get_CodecNum();
HI_U32 Hst3520d_Adapt_Get_ChnNum();
HI_U32 Hst3520d_Adapt_Get_ChnFmt(HI_U32 u32PhyChn);

AD_TYPE_EN Hst3520d_Adapt_Get_CodecType();

HI_S32 Hst3520d_Adapt_Get_VideoChnCfg(VIDEO_CHN_E enVQueChn,
    VIDEO_CHN_CFG_S *pstVideoCfg);

HI_S32 Hst3520d_Adapt_Get_AudioChnCfg(AUDIO_CHN_E enVQueChn,
    AUDIO_CHN_CFG_S *pstAudioCfg);

#ifdef __cplusplus
}
#endif

#endif
