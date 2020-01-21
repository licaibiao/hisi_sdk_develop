#ifndef HST_SDK_AL
#define HST_SDK_AL

#include "hstSdkStruct.h"
#include "hstChnConfig.h"

#ifdef __cplusplus
extern "C"{
#endif

//------------------------------------- VO API BEGIN------------------------------------//
//int HstSdkAL_GetVoAttr(VO_ATTR_S *pstVoAttr);
//int HstSdkAL_SetVoAttr(VO_ATTR_S *pstVoAttr);
//int HstSdkAL_DefaultVoAttr();
//int HstSdkAL_StartVo();
int HstSdkAL_StopVo();
//------------------------------------- VO API END-------------------------------------//


//------------------------------------- VI API BEGIN------------------------------------//
int HstSdkAL_GetViAttr(VI_ATTR_S *pstViAttr);
int HstSdkAL_SetViAttr(VI_ATTR_S *pstViAttr);
int HstSdkAL_DefaultViAttr(VI_DEFAULT_TYPE_E enType);
//int HstSdkAL_StartVi();
int HstSdkAL_StopVi();
//------------------------------------- VI API END-------------------------------------//


//------------------------------------- VENC API BEGIN------------------------------------//
int HstSdkAL_RegWriteVQueueFunc(VIDEO_CHN_E enVeChnNo, CallWriteVQueue pWrite);
int HstSdkAL_RegDestoryVQueueFunc(VIDEO_CHN_E enVeChnNo, CallDestoryVQueue pDestory);

int HstSdkAL_GetVencAttr(VIDEO_CHN_E enVeChnNo, HST_VENC_ATTR_S *pstVencAttr);
int HstSdkAL_SetVencAttr(VIDEO_CHN_E enVeChnNo, HST_VENC_ATTR_S *pstVencAttr);

int HstSdkAL_DefaultVencAttr();

int HstSdkAL_StartVenc(VIDEO_CHN_E enVeChnNo);
int HstSdkAL_BeingVenc(VIDEO_CHN_E enVeChnNo);
int HstSdkAL_StopVenc(VIDEO_CHN_E enVeChnNo);
//------------------------------------- VENC API END------------------------------------//


//------------------------------------- VDEC API BEGIN------------------------------------//
int HstSdkAL_SetVdecAttr(VIDEO_CHN_E enVdChnNo, VDEC_ATTR_S *pstVdecAttr);
int HstSdkAL_GetVdecAttr(VIDEO_CHN_E enVdChnNo, VDEC_ATTR_S *pstVdecAttr);

int HstSdkAL_SetVdecFrameRate(VIDEO_CHN_E enVdChnNo, HI_S32 s32FrmRate);

int HstSdkAL_StartVdec(VIDEO_CHN_E enVdChnNo);
int HstSdkAL_BeingVdec(VIDEO_CHN_E enVdChnNo, VDEC_STREAM_S stStream);
int HstSdkAL_StopVdec(VIDEO_CHN_E enVdChnNo);

int HstSdkAL_DefaultVdecAttr();
int HstSdkAL_TestVdec(VIDEO_CHN_E enVdChnNo, char *pStreamFileName, pthread_t VdecThread);
int HstSdkAL_VdecEnd2PrevMode(VIDEO_CHN_E enVdChnNo);

//------------------------------------- VDEC API END------------------------------------//



//------------------------------------- AIO API BEGIN------------------------------------//

int HstSdkAL_GetAioAttr(AI_AO_ATTR_S *pstAioAttr);
int HstSdkAL_SetAioAttr(AI_AO_ATTR_S *pstAioAttr);

int HstSdkAL_DefaultAioAttr();
int HstSdkAL_StartAio();
int HstSdkAL_StopAio();

//------------------------------------- AIO API END------------------------------------//


//------------------------------------- AENC API BEGIN------------------------------------//
int HstSdkAL_RegDestoryAQueueFunc(AUDIO_CHN_E enAeChn, CallDestoryAQueue pDestory);
int HstSdkAL_RegWriteAQueueFunc(AUDIO_CHN_E enAeChn, CallWriteAQueue pWrite);

int HstSdkAL_SetAencAttr(AUDIO_CHN_E enAeChn, AI_AENC_ATTR_S *pstAencAttr);
int HstSdkAL_GetAencAttr(AUDIO_CHN_E enAeChn, AI_AENC_ATTR_S *pstAencAttr);

int HstSdkAL_StartAenc(AUDIO_CHN_E enAeChn);
int HstSdkAL_BeingAenc(AUDIO_CHN_E enAeChn);
int HstSdkAL_StopAenc(AUDIO_CHN_E enAeChn);
//------------------------------------- AENC API END------------------------------------//


//------------------------------------- ADEC API BEGIN------------------------------------//
int HstSdkAL_GetAdecAoAttr(ADEC_AO_ATTR_S *pstAdecAoAttr);
int HstSdkAL_SetAdecAoAttr(ADEC_AO_ATTR_S *pstAdecAoAttr);
int HstSdkAL_DefaultAdecAoAttr();

int HstSdkAL_StartAdecAo();
int HstSdkAL_BeingAdecAo(AUDIO_STREAM_S stAudioStream);
int HstSdkAL_StopAdecAo();
//------------------------------------- ADEC API END------------------------------------//


//------------------------------------- SNAP API BEGIN------------------------------------//

int HstSdkAL_RegWriteSnapQueue(VIDEO_CHN_E enSnapChn, CallWriteSnapQueue pWrite);
int HstSdkAL_RegDestorySnapQueue(VIDEO_CHN_E enSnapChn, CallDestorySnapQueue pDestory);

int HstSdkAL_SetSnapAttr(VIDEO_CHN_E enSnapChn, SNAP_ATTR_S *pstSnapAttr);
int HstSdkAL_GetSnapAttr(VIDEO_CHN_E enSnapChn, SNAP_ATTR_S *pstSnapAttr);

int HstSdkAL_DefaultSnapAttr();

int HstSdkAL_StartSnap(VIDEO_CHN_E enSnapChn);
int HstSdkAL_BeingSnap(VIDEO_CHN_E enSnapChn);
int HstSdkAL_StopSnap(VIDEO_CHN_E enSnapChn);

//------------------------------------- SNAP API END------------------------------------//

int HstSdkAL_GetVideoState(unsigned int *pu32VideoState);
//int HstSdkAL_SelfAdaptationViAttr();
int HstSdkAL_SelfAdaptationViAttr(CAMERA_INPUT_STATUS_S *pastCameraStatus,CAMERA_RESOLUTION_STATUS_E	* paeGroupStat);

/********* 获取机器的硬件通道数 2018-06-23 *********/
unsigned int HstSdkAL_GetHwChnNum();
int HstSdkAL_GetCurVencSzie(VIDEO_CHN_E enVeChnNo);
    //注册当前设备版本
int HstSdkAL_RegDevVer(DEV_VERSION_BT_E enVer);
    //判断当前设备类型
//unsigned int HstSdkAL_CheckDevType(DEV_VERSION_BT_E enVer);

#ifdef __cplusplus
}
#endif

#endif
