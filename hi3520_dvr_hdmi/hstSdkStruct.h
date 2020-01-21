#ifndef _HST_SDK_STRUCT_H
#define _HST_SDK_STRUCT_H

#include "hstChnConfig.h"
#include "hstSdkDefine.h"
#include "sample_comm.h"
#include <pthread.h>


#define	HST_SUCCEED	(0)
#define	HST_FAILURE	(-1)


//-------------------------------回调函数指针--------------------------------//

typedef int (*CallWriteVQueue)(HI_U32, void*); 
typedef int (*CallDestoryVQueue)(HI_U32); 

typedef int (*CallWriteAQueue)(HI_U32, void*); 
typedef int (*CallDestoryAQueue)(HI_U32); 

typedef int (*CallWriteSnapQueue)(HI_U32, void*);
typedef int (*CallDestorySnapQueue)(HI_U32);

//----------------------------------------------------------------------------//


#define HST_PHY_CHN_NUM  4

typedef enum
{
	VI_DEFAULT_4CHN_D1,
	VI_DEFAULT_4CHN_720P,
	VI_DEFAULT_4CHN_1080P,
}VI_DEFAULT_TYPE_E;//add by lianzihao 2016-07-21


typedef struct HST_VI_ATTR
{
	HI_U32      u32ViChnCnt;
	HI_S32      s32VpssGrpCnt;
	HI_S32		s32VpssChCnt;
	VPSS_CHN	VpssChnSDVo; 
	SAMPLE_VI_MODE_E    enViMode;
	VIDEO_NORM_E        enNorm;
	PIC_SIZE_E          enPicSize;
	PIXEL_FORMAT_E      enPixFmt;
	HI_U32              u32AlignWidth;
	COMPRESS_MODE_E     enCompFmt;
	
}VI_ATTR_S;

typedef struct HST_VO_ATTR
{
	VO_DEV      VoDev;
	VO_LAYER    VoLayer;
	HI_U32      u32WndNum;
	VPSS_CHN    VpssChnSDVo; 
	SAMPLE_VO_MODE_E    enVoMode;
	VO_PUB_ATTR_S       stVoPubAttr;
}VO_ATTR_S;

typedef struct HST_VO_FULLSCREEN_ATTR
{
	int temp;

}VO_FULLSCREEN_ATTR_S;

typedef struct HST_VENC_ATTR
{
	HI_S32  s32VpssGrpCnt;
	PAYLOAD_TYPE_E enPayLoad;
	PIC_SIZE_E enSize;
	HI_U32 u32Profile; /*0: baseline; 1:MP; 2:HP 3:svc-t */
	SAMPLE_RC_E enRcMode;
    HI_S32 siSetGop;
    HI_S32 siSetFps;
    HI_S32 siSetBitRate;
    /* 码率值(kbit/s)， 当enRcMode为VBR时为最大码率值；
                当enRcMode为CBR时为固定码率值；
                当enRcMode为VBR且码率值为0，则通过VBR码率等级与帧率计算码率值;
                当enRcMode为CBR且码率值为0，则通过使用最高码率等级与帧率计算码率值;*/
    HI_S32 uiIQp;
    HI_S32 uiPQp;
    HI_U32 u32VBRBitRateLevel;
    /*  VBR码率等级， 当enRcMode为VBR时有效。取值范围：0~5，数值越大，图像效果越差*/
    HI_U32 u32BitRateUpperLimit;/*码率上限值(kByte/s)*/
	HI_U16 usOsdSet;    /**OSD设置参数**/
}HST_VENC_ATTR_S;

typedef struct HST_VDEC_ATTR
{
	SIZE_S      stInputSize;  //需要解码视频的尺寸
	SIZE_S      stOuputSize;  //处理后输出视频的尺寸

	HI_S32      s32Framerate; //视频帧率
	
	HI_U32      u32VdecCnt;   //VDEC 通道数
	HI_U32      u32VpssGrpCnt;//VPSS 通道组数量
	HI_U32      u32VpssChnCnt;//VPSS 物理通道数

    VPSS_GRP    VpssGrp;      //VPSS 使用的通道组
    VPSS_CHN    VpssChn;      //VPSS 使用的通道号
	VO_DEV      VoDev;        //视频输出设备
	VO_LAYER    VoLayer;      //视频输出的视频层
	HI_U32      u32WndNum;    //输出显示的窗口数

	SAMPLE_VO_MODE_E  enVoMode;
	VO_PUB_ATTR_S     stVoPubAttr;

}VDEC_ATTR_S;


typedef struct HST_AIO_ATTR
{
	AUDIO_DEV AiDev;
	AI_CHN       AiChn;
	AUDIO_DEV AoDev;
	AO_CHN      AoChn;

	AIO_ATTR_S   stAioAttr;

	AUDIO_RESAMPLE_ATTR_S stAiReSampleAttr;
	AUDIO_RESAMPLE_ATTR_S stAoReSampleAttr;
}AI_AO_ATTR_S;


typedef struct HST_AIAENC_ATTR
{
	AUDIO_DEV   AiDev;
	AI_CHN	s32AiChn;
	HI_S64	s64EncStart; 			/* 编码的参考时间 */
	PAYLOAD_TYPE_E  enPayloadType;
    G726_BPS_E enG726bps;
		/**modified by fjw 20171016**/
	HI_U32 	u32PtNumPerFrm;/* 编码帧长 */
}AI_AENC_ATTR_S;

typedef struct HST_ADEC_AO_ATTR
{
	AUDIO_DEV AoDev;
	AO_CHN      AoChn;
	ADEC_CHN    AdecChn;
	AIO_ATTR_S  stAioAttr;
	PAYLOAD_TYPE_E enPayloadType;
}ADEC_AO_ATTR_S;


typedef struct HST_SNAP_ATTR
{
	HI_S32  s32VpssGrpCnt;
	PIC_SIZE_E enSize;
}SNAP_ATTR_S;


typedef enum
{
	CAMERA_SIGNAL_NULL,
	CAMERA_SIGNAL_CVBS,
	CAMERA_SIGNAL_AHD,
	CAMERA_SIGNAL_IPC,
}CAMERA_SIGNAL_STATUS_E;

typedef enum
{
	CAMERA_RESOLUTION_NULL=0,
	CAMERA_RESOLUTION_D1=1,
	CAMERA_RESOLUTION_720P=2,
	CAMERA_RESOLUTION_906P=3,
	CAMERA_RESOLUTION_1080P=4,
}CAMERA_RESOLUTION_STATUS_E;

typedef enum
{
	CAMERA_DETECT_NULL,
	CAMERA_DETECT_COVER,
	CAMERA_DETECT_NORMAL,
}CAMERA_DETECT_STATUS_E;


typedef struct HST_CAMERA_STATUS
{
	CAMERA_SIGNAL_STATUS_E 		  eSignalStat;
	CAMERA_RESOLUTION_STATUS_E    eResolutionStat;
	CAMERA_DETECT_STATUS_E		  eDetectStat;
}CAMERA_INPUT_STATUS_S;

#endif
