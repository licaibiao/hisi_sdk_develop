/******************************************************************************
  A simple program of Hisilicon HI3521 video input and output implementation.
  Copyright (C), 2012-2020, Hisilicon Tech. Co., Ltd.
 ******************************************************************************
    Modification:  2012-7 Created
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "sample_comm.h"

#define HDMI_SUPPORT

#define VPSS_BSTR_CHN     		0
#define VPSS_LSTR_CHN     		1

typedef struct hiSAMPLE_SPOT_THRD_ARG_S 
{
    VO_DEV VoDev;
    HI_S32 s32ViChnCnt;
    HI_BOOL bToExit;
}SAMPLE_SPOT_THRD_ARG_S;

/******************************************************************************
* function : show usage
******************************************************************************/
typedef struct hiSAMPLE_VICHN_D1_STATS_S
{
    /* 0 means this chn is not capturing D1, else means it is capturing D1.
       No matter who requests, it should add 1. */
    HI_U32 u32ReqCnt;
    pthread_mutex_t stThrdLock;
}SAMPLE_VICHN_D1_STATS_S;


static VIDEO_NORM_E s_enNorm = VIDEO_ENCODING_MODE_NTSC;
static HI_U32 s_u32D1Height = 0; 
static HI_U32 s_u32ViFrmRate = 0; 
static pthread_t s_stSdSpotThrd;
static SAMPLE_SPOT_THRD_ARG_S s_stSpotThrdArg;
static SAMPLE_VICHN_D1_STATS_S s_astViCapD1Status[VIU_MAX_CHN_NUM];
static SIZE_S s_stD1Size;
static SIZE_S s_st960X540Size;
static SIZE_S s_stCifSize;
static SIZE_S s_st2CifSize;


/******************************************************************************
* function : show usage
******************************************************************************/
void SAMPLE_IT_Usage(char *sPrgNm)
{
    printf("Usage : %s <index>\n", sPrgNm);
    printf("index:\n");
    printf("\t 0) VI:8*1080p/8*960540 MixCap + Venc,H264:8*1080p@10fps, JPEG:8*1080p@2fps  + VO:HD0(HDMI + VGA)preview.\n");    
    return;
}

void SAMPLE_IT_GlobalVarInit()
{   
    HI_S32 i;
    
    s_u32ViFrmRate = (VIDEO_ENCODING_MODE_PAL == s_enNorm) ? 25 : 30;
    s_u32D1Height = (VIDEO_ENCODING_MODE_PAL == s_enNorm) ? 576 : 480;
    s_stD1Size.u32Width = D1_WIDTH;
    s_stD1Size.u32Height = s_u32D1Height;
    s_st2CifSize.u32Width = D1_WIDTH / 2;
    s_st2CifSize.u32Height = s_u32D1Height;
    s_stCifSize.u32Width = D1_WIDTH / 2;
    s_stCifSize.u32Height = s_u32D1Height / 2;
    s_st960X540Size.u32Width = 960;
    s_st960X540Size.u32Height = 540;
    for (i = 0; i < VIU_MAX_CHN_NUM; ++i)
    {
        s_astViCapD1Status[i].u32ReqCnt = 0;
        pthread_mutex_init(&s_astViCapD1Status[i].stThrdLock, NULL);
    }
}

/******************************************************************************
* function : to process abnormal case                                         
******************************************************************************/
void SAMPLE_IT_HandleSig(HI_S32 signo)
{
    if (SIGINT == signo || SIGTSTP == signo)
    {
        SAMPLE_COMM_SYS_Exit();
        printf("\033[0;31mprogram termination abnormally!\033[0;39m\n");
    }
    exit(-1);
}

HI_S32 SAMPLE_IT_StartHD(VO_DEV VoDev,  VO_PUB_ATTR_S *pstVoPubAttr, 
    SAMPLE_VO_MODE_E enVoMode, VIDEO_NORM_E enVideoNorm, HI_U32 u32SrcFrmRate)
{
    VO_VIDEO_LAYER_ATTR_S stLayerAttr;       
    VO_LAYER VoLayer = SAMPLE_VO_LAYER_VHD0;
    HI_S32 s32Ret;

    memset(&(stLayerAttr), 0 , sizeof(VO_VIDEO_LAYER_ATTR_S));
    s32Ret = SAMPLE_COMM_VO_GetWH(pstVoPubAttr->enIntfSync, \
    &stLayerAttr.stImageSize.u32Width, \
    &stLayerAttr.stImageSize.u32Height, \
    &stLayerAttr.u32DispFrmRt);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start SAMPLE_COMM_VO_GetWH failed!\n");
        goto END_StartHD_0;
    }

    stLayerAttr.enPixFormat = SAMPLE_PIXEL_FORMAT;
    stLayerAttr.stDispRect.s32X       = 0;
    stLayerAttr.stDispRect.s32Y       = 0;
    stLayerAttr.stDispRect.u32Width   = stLayerAttr.stImageSize.u32Width;
    stLayerAttr.stDispRect.u32Height  = stLayerAttr.stImageSize.u32Height; 
   
    
    s32Ret = SAMPLE_COMM_VO_StartDev(VoDev, pstVoPubAttr);
    if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_StartDevLayer failed!\n");
		goto END_StartHD_0;
	}
    
    s32Ret = SAMPLE_COMM_VO_StartLayer(VoLayer, &stLayerAttr);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_StartDevLayer failed!\n");
		goto END_StartHD_1;
	}

    s32Ret = SAMPLE_COMM_VO_StartChn(VoLayer, enVoMode);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_StartChn failed!\n");
		goto END_StartHD_2;
	}

	/* if it's displayed on HDMI, we should start HDMI */

#ifdef HDMI_SUPPORT
        /* if it's displayed on HDMI, we should start HDMI */
        if (pstVoPubAttr->enIntfType & VO_INTF_HDMI)
        {
            if (HI_SUCCESS != SAMPLE_COMM_VO_HdmiStart(pstVoPubAttr->enIntfSync))
            {
                SAMPLE_PRT("Start SAMPLE_COMM_VO_HdmiStart failed!\n");
                goto END_StartHD_3;
            }
        }
        return HI_SUCCESS;
#endif
	
  
    
END_StartHD_3:
    SAMPLE_COMM_VO_StopChn(VoDev, enVoMode);
END_StartHD_2:
    SAMPLE_COMM_VO_StopLayer(VoLayer);
END_StartHD_1:
    SAMPLE_COMM_VO_StopDev(VoLayer);
END_StartHD_0:
    return s32Ret;
}

HI_S32 SAMPLE_IT_StopHD(VO_DEV VoDev, const VO_PUB_ATTR_S *pstVoPubAttr, SAMPLE_VO_MODE_E enVoMode)
{ 
    VO_LAYER VoLayer = SAMPLE_VO_LAYER_VHD0;
    if (pstVoPubAttr->enIntfType & VO_INTF_HDMI)
    {
        SAMPLE_COMM_VO_HdmiStop();
    }
    SAMPLE_COMM_VO_StopChn(VoDev, enVoMode);

    SAMPLE_COMM_VO_StopLayer(VoLayer);

    SAMPLE_COMM_VO_StopDev(VoLayer);
    return HI_SUCCESS;
}

/******************************************************************************
* function :  vodev sd spot process. 
******************************************************************************/
void *SAMPLE_IT_SdSpotProc(void *pData)
{
    SAMPLE_SPOT_THRD_ARG_S *pstThrdArg;
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;
    HI_S32 i = 0;
    HI_S32 s32Ret;
    VI_CHN_ATTR_S stViChnOldAttr;
    VI_CHN_ATTR_S stViChnD1Attr;   
    
    pstThrdArg = (SAMPLE_SPOT_THRD_ARG_S *)pData;

    stSrcChn.enModId = HI_ID_VIU;
    stSrcChn.s32DevId = 0;
    
    stDestChn.enModId = HI_ID_VOU;
    stDestChn.s32DevId = pstThrdArg->VoDev;
    stDestChn.s32ChnId = 0;

    while (!pstThrdArg->bToExit)
    {
        stSrcChn.s32ChnId = i;

        pthread_mutex_lock(&s_astViCapD1Status[i].stThrdLock);
        if (0 == s_astViCapD1Status[i].u32ReqCnt)
        {
            /* If this chn is not capturing D1 now, change it to capture D1 */
            s32Ret = HI_MPI_VI_GetChnAttr(i, &stViChnOldAttr);
            memcpy(&stViChnD1Attr, &stViChnOldAttr, sizeof(stViChnD1Attr));
            stViChnD1Attr.stDestSize.u32Width = D1_WIDTH;
            stViChnD1Attr.s32SrcFrameRate = s_u32ViFrmRate;
            stViChnD1Attr.s32DstFrameRate = s_u32ViFrmRate;
            s32Ret = HI_MPI_VI_SetChnAttr(i, &stViChnD1Attr);
        }
        ++s_astViCapD1Status[i].u32ReqCnt;
        pthread_mutex_unlock(&s_astViCapD1Status[i].stThrdLock);

        s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("failed with %#x!\n", s32Ret);
            return NULL;
        }

        sleep(2);

        pthread_mutex_lock(&s_astViCapD1Status[i].stThrdLock);
        if (1 == s_astViCapD1Status[i].u32ReqCnt)
        {
            s32Ret = HI_MPI_VI_SetChnAttr(i, &stViChnOldAttr);
        }
        --s_astViCapD1Status[i].u32ReqCnt;
        pthread_mutex_unlock(&s_astViCapD1Status[i].stThrdLock);

        s32Ret = HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("failed with %#x!\n", s32Ret);
            return NULL;
        }
        i = (i + 1) % pstThrdArg->s32ViChnCnt;
    }
    return 0;
}

/******************************************************************************
* function :  start vodev sd to spot. 
******************************************************************************/
HI_S32 SAMPLE_IT_StartSdSpot(VO_DEV VoDev, VIDEO_NORM_E enVideoNorm, HI_S32 s32ViChnCnt)
{
    VO_PUB_ATTR_S stVoPubAttr;
    HI_S32 s32Ret;
    VO_VIDEO_LAYER_ATTR_S stLayerAttr;     
    VO_LAYER VoLayer = SAMPLE_VO_LAYER_VSD0;
    SAMPLE_VO_MODE_E enVoMode = VO_MODE_1MUX;

  
    stVoPubAttr.enIntfSync = (VIDEO_ENCODING_MODE_NTSC == enVideoNorm) ? VO_OUTPUT_NTSC : VO_OUTPUT_PAL;
    stVoPubAttr.enIntfType = VO_INTF_CVBS;
    stVoPubAttr.u32BgColor = 0x000000ff; 
    
    memset(&(stLayerAttr), 0 , sizeof(VO_VIDEO_LAYER_ATTR_S));
    s32Ret = SAMPLE_COMM_VO_GetWH(stVoPubAttr.enIntfSync, \
    &stLayerAttr.stImageSize.u32Width, \
    &stLayerAttr.stImageSize.u32Height, \
    &stLayerAttr.u32DispFrmRt);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start SAMPLE_COMM_VO_GetWH failed!\n");
        goto END_StartSD_0;
    }

    stLayerAttr.enPixFormat = SAMPLE_PIXEL_FORMAT;
    stLayerAttr.stDispRect.s32X       = 0;
    stLayerAttr.stDispRect.s32Y       = 0;
    stLayerAttr.stDispRect.u32Width   = stLayerAttr.stImageSize.u32Width;
    stLayerAttr.stDispRect.u32Height  = stLayerAttr.stImageSize.u32Height; 


    s32Ret = SAMPLE_COMM_VO_StartDev(VoDev, &stVoPubAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start SAMPLE_COMM_VO_StartDevLayer failed!\n");
        goto END_StartSD_0;
    }

    s32Ret = SAMPLE_COMM_VO_StartLayer(VoLayer, &stLayerAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start SAMPLE_COMM_VO_StartDevLayer failed!\n");
        goto END_StartSD_1;
    }

    s32Ret = SAMPLE_COMM_VO_StartChn(VoLayer, enVoMode);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start SAMPLE_COMM_VO_StartChn failed!\n");
        goto END_StartSD_2;
    }
    
    s_stSpotThrdArg.VoDev = VoDev;
    s_stSpotThrdArg.s32ViChnCnt = s32ViChnCnt;
    s_stSpotThrdArg.bToExit = HI_FALSE;
    s32Ret = pthread_create(&s_stSdSpotThrd, NULL, SAMPLE_IT_SdSpotProc, (HI_VOID*)&s_stSpotThrdArg);
    if (0 != s32Ret)
    {
        return HI_FAILURE;
    }
    return HI_SUCCESS;


END_StartSD_2:
    SAMPLE_COMM_VO_StopLayer(VoLayer);
END_StartSD_1:
    SAMPLE_COMM_VO_StopDev(VoLayer);
END_StartSD_0:
    return s32Ret;
}

/******************************************************************************
* funciton : stop get venc stream process.
******************************************************************************/
HI_S32 SAMPLE_IT_StopSdSpot(VO_DEV VoDev)
{

    VO_LAYER VoLayer = SAMPLE_VO_LAYER_VSD0;
    SAMPLE_VO_MODE_E enVoMode = VO_MODE_1MUX;
    if (HI_TRUE != s_stSpotThrdArg.bToExit)
    {
        s_stSpotThrdArg.bToExit = HI_TRUE;
        pthread_join(s_stSdSpotThrd, 0);
    }
    
    SAMPLE_COMM_VO_StopChn(VoLayer, enVoMode);

    SAMPLE_COMM_VO_StopLayer(VoLayer);

    SAMPLE_COMM_VO_StopDev(VoLayer);
    
    return HI_SUCCESS;
}

/******************************************************************************
* function :  VI:8*1080p(10fps)/8cif(10fps); VO:HD0(HDMI 1080P)video preview. 
******************************************************************************/
HI_S32 SAMPLE_IT_10fps1080p_30fps960X540(HI_VOID)
{  
    SAMPLE_VI_MODE_E enViMode = SAMPLE_VI_MODE_8_1080P;
    HI_U32 s32ViChnCnt = 8;
    HI_S32 s32VpssGrpCnt = 16;
    VB_CONF_S stVbConf;
    VPSS_GRP VpssGrp;
    VPSS_GRP_ATTR_S stGrpAttr;
	VPSS_CHN VpssChn_VoHD0 = VPSS_CHN0;
    VPSS_CHN_ATTR_S stVpssChnAttr;
    VPSS_CHN_MODE_S stVpssChnMode,stVpssChnMode1;
    VO_DEV VoDev;
    VO_CHN VoChn;   
    VO_PUB_ATTR_S stVoPubAttr; 
    SAMPLE_VO_MODE_E enVoMode;
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;
    HI_S32 i;
    HI_S32 j;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32BlkSize;   
    HI_U32 u32WndNum;    
    VENC_CHN VencChn;    
    PAYLOAD_TYPE_E enPayLoad[3]= {PT_H264, PT_JPEG,PT_H264};
    HI_U32 u32Profile = 1; /*0: baseline; 1:MP; 2:HP 3:svc-t */
    SIZE_S stsize;
    VPSS_SIZER_INFO_S stVpssSize;

    /******************************************
     step  1: VB and system init.
    ******************************************/
    memset(&stVbConf,0,sizeof(VB_CONF_S));
    stVbConf.u32MaxPoolCnt = 64;

    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(s_enNorm,\
                PIC_HD1080, SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH, COMPRESS_MODE_NONE);
    stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[0].u32BlkCnt = 80;
    
    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(s_enNorm,\
                PIC_HD720, SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH, COMPRESS_MODE_NONE);
    stVbConf.astCommPool[1].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[1].u32BlkCnt = 30;
    
    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(s_enNorm,\
                PIC_CIF, SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH, COMPRESS_MODE_NONE);
    stVbConf.astCommPool[2].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[2].u32BlkCnt = 10;

    s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("system init failed with %d!\n", s32Ret);
        goto END_16_MixCap_0;
    }

    /******************************************
     step 2: start vo HD0(HDMI) 
    ******************************************/
	printf("start vo HD0.\n");
	VoDev = SAMPLE_VO_DEV_DHD0;
	u32WndNum = 8;
	enVoMode = VO_MODE_9MUX;
    if(VIDEO_ENCODING_MODE_PAL == s_enNorm)
    {
        stVoPubAttr.enIntfSync = VO_OUTPUT_1080P50;
    }
    else
    {
        stVoPubAttr.enIntfSync = VO_OUTPUT_1080P60;
    }
    stVoPubAttr.enIntfType = VO_INTF_HDMI | VO_INTF_VGA;
    stVoPubAttr.u32BgColor = 0x000000ff;   
    s32Ret = SAMPLE_IT_StartHD(VoDev, &stVoPubAttr, enVoMode, s_enNorm, s_u32ViFrmRate);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start VO failed!\n");
        goto END_16_MixCap_1;
    }
    
    /******************************************
     step 3: start vpss,  bind it to venc and vo
    ******************************************/    
    stGrpAttr.u32MaxW = HD_WIDTH;
    stGrpAttr.u32MaxH = HD_HEIGHT;
   
    stGrpAttr.bIeEn = HI_TRUE;
    stGrpAttr.bNrEn = HI_TRUE;
    stGrpAttr.bHistEn = HI_FALSE;
    stGrpAttr.bEsEn = HI_FALSE;
    
   
    stGrpAttr.enDieMode = VPSS_DIE_MODE_NODIE;
    stGrpAttr.enPixFmt = SAMPLE_PIXEL_FORMAT;


    memset(&stVpssChnAttr, 0,sizeof(VPSS_CHN_ATTR_S));
    stVpssChnAttr.bSpEn = HI_TRUE;
    stVpssChnAttr.bBorderEn = HI_TRUE;  
   
    stVpssChnAttr.stBorder.u32TopWidth = 2;
    stVpssChnAttr.stBorder.u32BottomWidth = 2;
    stVpssChnAttr.stBorder.u32LeftWidth =2;
    stVpssChnAttr.stBorder.u32RightWidth =2;
    stVpssChnAttr.stBorder.u32Color = 0xff00;

 
    stVpssChnMode.enChnMode = VPSS_CHN_MODE_USER;
    stVpssChnMode.enCompressMode = COMPRESS_MODE_NONE;
    stVpssChnMode.enPixelFormat = SAMPLE_PIXEL_FORMAT;
    stVpssChnMode.stFrameRate.s32DstFrmRate = -1;
    stVpssChnMode.stFrameRate.s32SrcFrmRate = -1;
    stVpssChnMode.u32Width = 1920;
    stVpssChnMode.u32Height = 1080; 
    memset(&stVpssChnMode.stAspectRatio,0, sizeof(ASPECT_RATIO_S)); 


    stVpssChnMode1.enChnMode = VPSS_CHN_MODE_USER;
    stVpssChnMode1.enCompressMode = COMPRESS_MODE_NONE;
    stVpssChnMode1.enPixelFormat = SAMPLE_PIXEL_FORMAT;
    stVpssChnMode1.stFrameRate.s32DstFrmRate = -1;
    stVpssChnMode1.stFrameRate.s32SrcFrmRate = -1;
    stVpssChnMode1.u32Width = 360;
    stVpssChnMode1.u32Height = 240;    
    memset(&stVpssChnMode1.stAspectRatio,0, sizeof(ASPECT_RATIO_S));
    stVpssSize.bSizer =  HI_TRUE;
    
    stVpssSize.stSize.u32Width = 1920;
    stVpssSize.stSize.u32Height = 1080;
    for (i = 0; i < s32VpssGrpCnt; i++)
    {
        VpssGrp = i;
        s32Ret = HI_MPI_VPSS_CreateGrp(VpssGrp, &stGrpAttr);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("HI_MPI_VPSS_CreateGrp(%d) failed!\n",i);
            goto END_16_MixCap_2;
        }

    
        VencChn = i;
        if (VpssGrp < 8)
        {
            /* 0-7vpss grp for venc 8X1080p@10fps */
            s32Ret |= HI_MPI_VPSS_SetGrpSizer(VpssGrp, &stVpssSize);
           
            s32Ret |= HI_MPI_VPSS_SetChnAttr(VpssGrp, VPSS_CHN0, &stVpssChnAttr);
            s32Ret |= HI_MPI_VPSS_SetChnMode( VpssGrp, VPSS_CHN0, &stVpssChnMode);
            s32Ret |= HI_MPI_VPSS_EnableChn(VpssGrp, VPSS_CHN0);

            s32Ret |= HI_MPI_VPSS_SetChnAttr(VpssGrp, VPSS_CHN1, &stVpssChnAttr);
            s32Ret |= HI_MPI_VPSS_SetChnMode( VpssGrp, VPSS_CHN1, &stVpssChnMode1);
            s32Ret |= HI_MPI_VPSS_EnableChn(VpssGrp, VPSS_CHN1);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_PRT("Start Vpss failed!\n");
                goto END_16_MixCap_2;
            }

            s32Ret |= SAMPLE_COMM_VENC_Start(VencChn,enPayLoad[0],\
                                           s_enNorm, PIC_HD1080,SAMPLE_RC_CBR, u32Profile);            
           
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_PRT("Start Venc failed!\n");
                goto END_16_MixCap_2;
            }
            s32Ret = SAMPLE_COMM_VENC_BindVpss(VencChn, VpssGrp, VPSS_CHN0);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_PRT("Start Venc failed!\n");
                goto END_16_MixCap_2;
            }


             s32Ret |= SAMPLE_COMM_VENC_Start(VencChn+8,enPayLoad[0],\
                       s_enNorm, PIC_CIF,SAMPLE_RC_CBR, u32Profile);            
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_PRT("Start Venc failed!\n");
                goto END_16_MixCap_2;
            }
            s32Ret = SAMPLE_COMM_VENC_BindVpss(VencChn+8, VpssGrp, VPSS_CHN1);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_PRT("Start Venc failed!\n");
                goto END_16_MixCap_2;
            }


            s32Ret = SAMPLE_COMM_SYS_GetPicSize(s_enNorm, PIC_HD1080, &stsize);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_PRT("Get picture size failed!\n");
                goto END_16_MixCap_2;
            }
           
            s32Ret = SAMPLE_COMM_VENC_SnapStart(VencChn+16,&stsize);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_PRT("Start Venc failed!\n");
                goto END_16_MixCap_2;
            }   

            s32Ret = HI_MPI_VENC_StartRecvPic(VencChn+16);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_PRT("HI_MPI_VENC_StartRecvPic faild with%#x!\n", s32Ret);
                return HI_FAILURE;
            }

            s32Ret = SAMPLE_COMM_VENC_BindVpss(VencChn+16, VpssGrp, VPSS_CHN0);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_PRT("Start Venc failed!\n");
                goto END_16_MixCap_2;
            }

           
            
        }
        else
        {
            /* 8-15 vpss grp for vo HD  */           
               
            s32Ret |= HI_MPI_VPSS_SetChnAttr(VpssGrp, VPSS_CHN0, &stVpssChnAttr);
           
            s32Ret |= HI_MPI_VPSS_EnableChn(VpssGrp, VPSS_CHN0);
            /* open pre-scale */
            s32Ret |= SAMPLE_COMM_EnableVpssPreScale(VpssGrp, s_st960X540Size);
            if(HI_SUCCESS != s32Ret)
            {
                SAMPLE_PRT("HI_MPI_VPSS_SetPreScale failed!\n");
                goto END_16_MixCap_2;
            }          
            
            VoChn = VpssGrp - 8;
            s32Ret = SAMPLE_COMM_VO_BindVpss(SAMPLE_VO_LAYER_VHD0, VoChn, VpssGrp, VPSS_CHN0);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_PRT("Start VO failed!\n");
                goto END_16_MixCap_2;
            }
        }
        
        s32Ret = HI_MPI_VPSS_StartGrp(VpssGrp);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("HI_MPI_VPSS_StartGrp failed!\n");
            goto END_16_MixCap_2;
        }
    }

    /******************************************
     step 4: bind vpss to vi
    ******************************************/
    stSrcChn.enModId = HI_ID_VIU;
    stSrcChn.s32DevId = 0;
    stDestChn.enModId = HI_ID_VPSS;
    stDestChn.s32ChnId = VPSS_CHN0;
    for (j=0; j<s32ViChnCnt; j++)
    {    
        stSrcChn.s32ChnId = 2*j;
        stDestChn.s32DevId = j;
        s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("failed with %#x!\n", s32Ret);
            goto END_16_MixCap_3;
        }
        
        stDestChn.s32DevId = j + 8;
        s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("failed with %#x!\n", s32Ret);
            goto END_16_MixCap_3;
        }
    }

    /******************************************
     step 5: start a thread to get venc stream 
    ******************************************/
    s32Ret = SAMPLE_COMM_VENC_StartGetStream(16);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Venc failed!\n");
        goto END_16_MixCap_3;
    }

    /******************************************
    step 5: start a thread to get venc stream 
    ******************************************/
    s32Ret = SAMPLE_COMM_VENC_StartGetJPEG(8);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Venc failed!\n");
        goto END_16_MixCap_3;
    }

    /******************************************
     step 6: start vi dev & chn
    ******************************************/
    s32Ret = SAMPLE_COMM_VI_MixCap_Start(enViMode, s_enNorm);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("start vi failed!\n");
        goto END_16_MixCap_4;
    }
    
#if 0
    /******************************************
     step 7: start vo sd to spot
    ******************************************/
    //s32Ret = SAMPLE_IT_StartSdSpot(SAMPLE_VO_DEV_DSD0, s_enNorm, s32ViChnCnt);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start VoDev %d to spot failed!\n", SAMPLE_VO_DEV_DSD0);
        goto END_16_MixCap_4;
    }
#endif

    /******************************************
     step 8: HD0 switch mode 
    ******************************************/
    printf("\nplease press any key,exit.\n");
    getchar();

    /******************************************
     step 9: exit process
    ******************************************/
END_16_MixCap_4:
    SAMPLE_COMM_VI_Stop(enViMode);
    //SAMPLE_IT_StopSdSpot(SAMPLE_VO_DEV_DSD0);
END_16_MixCap_3:
    SAMPLE_COMM_VENC_StopGetStream();
    SAMPLE_COMM_VENC_StopGetJPEG();
    SAMPLE_COMM_VI_UnBindVpss(enViMode);

    VpssGrp = 0;

    for(i = 0;i<8;i++)
    {
        stSrcChn.enModId = HI_ID_VIU;
        stSrcChn.s32DevId = 0;
        stSrcChn.s32ChnId = 2*i;

        stDestChn.enModId = HI_ID_VPSS;
        stDestChn.s32DevId = VpssGrp+8;
        stDestChn.s32ChnId = 0;

        s32Ret = HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("failed with %#x!\n", s32Ret);
            return HI_FAILURE;
        }
        VpssGrp ++;
    }
    
END_16_MixCap_2:
    for (i=0; i<8; i++)
    {
        VpssGrp = i;       
        VencChn = i;
        SAMPLE_COMM_VENC_UnBindVpss(VencChn, VpssGrp, VPSS_CHN0); 
        SAMPLE_COMM_VENC_Stop(VencChn);
        SAMPLE_COMM_VENC_UnBindVpss(VencChn+8, VpssGrp, VPSS_CHN1);
        SAMPLE_COMM_VENC_Stop(VencChn+8);
        SAMPLE_COMM_VENC_UnBindVpss(VencChn+16, VpssGrp, VPSS_CHN0);
        SAMPLE_COMM_VENC_Stop(VencChn+16);
    }    
    SAMPLE_COMM_VPSS_Stop(s32VpssGrpCnt, VPSS_MAX_CHN_NUM);
END_16_MixCap_1:
    VoDev = SAMPLE_VO_DEV_DHD0;
    u32WndNum = 8;
    enVoMode = VO_MODE_9MUX;
    for(i=0;i<u32WndNum;i++)
    {
        VoChn = i;
        VpssGrp = i;
        SAMPLE_COMM_VO_UnBindVpss(VoDev,VoChn,VpssGrp+8,VpssChn_VoHD0);
    }
    SAMPLE_IT_StopHD(VoDev, &stVoPubAttr, enVoMode);
END_16_MixCap_0:
    SAMPLE_COMM_SYS_Exit();
    
    return s32Ret;  
}


HI_S32 SAMPLE_IT_6fpsD1_6fpsCif(HI_VOID)
{
    return 0;
}


/******************************************************************************
* function :  VI:16*2cif; VO:HD0(HDMI  720P50), WBC to SD0(CVBS) video preview. 
******************************************************************************/
HI_S32 SAMPLE_IT_25fpsCif_25fpsQcif(HI_VOID)
{
    return 0;   
}



/******************************************************************************
* function    : main()
* Description : video preview sample
******************************************************************************/
int main(int argc, char *argv[])
{
    HI_S32 s32Ret;

    if ( (argc < 2) || (1 != strlen(argv[1])))
    {
        SAMPLE_IT_Usage(argv[0]);
        return HI_FAILURE;
    }

    signal(SIGINT, SAMPLE_IT_HandleSig);
    signal(SIGTERM, SAMPLE_IT_HandleSig);

    SAMPLE_IT_GlobalVarInit();
    
    switch (*argv[1])
    {
        case '0':/* mixed vi capture */
            printf("Attention: You need to set OS buffer up to 128MB and MMZ buffer up to more than 600MB!\n\n");
            s32Ret = SAMPLE_IT_10fps1080p_30fps960X540();
            break;      
        default:
            printf("the index is invaild!\n");
            SAMPLE_IT_Usage(argv[0]);
            return HI_FAILURE;
    }

    if (HI_SUCCESS == s32Ret)
        printf("program exit normally!\n");
    else
        printf("program exit abnormally!\n");
    exit(s32Ret);
}

