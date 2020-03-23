/******************************************************************************
  A simple program of Hisilicon mpp vda implementation.
  the flow as follows:
    1) init mpp system.
    2) start vi( internal isp, ViDev 0, vichn0) and vo (HD)                  
    3) vda md & od start & print information
    4) stop vi vo and system.
  Copyright (C), 2010-2020, Hisilicon Tech. Co., Ltd.
 ******************************************************************************
    Modification:  2013-7 Created
******************************************************************************/
#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>

#include "sample_comm.h"
#include "loadbmp.h"

VIDEO_NORM_E gs_enNorm = VIDEO_ENCODING_MODE_PAL;

HI_BOOL bExit   = HI_FALSE;
HI_BOOL bTravel = HI_FALSE;
HI_BOOL bShape1 = HI_FALSE;
HI_BOOL bShape2 = HI_FALSE;
HI_BOOL bArea   = HI_FALSE;

#define VDA_WIDTH_ALIGN  16
#define VDA_HEIGHT_ALIGN 16

typedef struct vdec_sendparam
{
    pthread_t Pid;
    HI_BOOL bRun;
    VDEC_CHN VdChn;    
    PAYLOAD_TYPE_E enPayload;
    HI_S32 s32MinBufSize;
    VIDEO_MODE_E enVideoMode;
}VDEC_SENDPARAM_S;

typedef struct hiRGN_OSD_REVERSE_INFO_S
{
    RGN_HANDLE Handle;
    HI_U8 u8PerPixelLumaThrd;

    VPSS_GRP VpssGrp;
    VPSS_REGION_INFO_S stLumaRgnInfo;    

}RGN_OSD_REVERSE_INFO_S;


/******************************************************************************
* Description : to process abnormal case                                        
******************************************************************************/
void SAMPLE_VDA_HandleSig(HI_S32 signo)
{
    if (SIGINT == signo || SIGTSTP == signo)
    {
        SAMPLE_COMM_SYS_Exit();
        printf("\033[0;31mprogram exit abnormally!\033[0;39m\n");
    }

    exit(0);
}


/******************************************************************************
* Description : show usage
******************************************************************************/
HI_VOID SAMPLE_VDA_Usage(HI_VOID)
{
    printf("\n\n/************************************/\n");
    printf("please choose the case which you want to run:\n");
    printf("\t0:  MD\n");
    printf("\t1:  OD\n");
    printf("\tq:  quit the whole sample\n");
    printf("sample command:");
    return;
}


HI_S32 VDEC_SendEos(VDEC_CHN Vdchn)
{
    return HI_SUCCESS;
}



HI_S32 SAMPLE_VDA_SYS_Init(HI_VOID)
{
    HI_S32 s32Ret;
    HI_U32 u32BlkSize;
    VB_CONF_S struVbConf, stVbConf;
    MPP_SYS_CONF_S struSysConf;

    HI_MPI_SYS_Exit();
    HI_MPI_VB_Exit(); 

    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(gs_enNorm, PIC_D1, SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH, COMPRESS_MODE_SEG);

    memset(&struVbConf, 0, sizeof(VB_CONF_S));
     
    struVbConf.u32MaxPoolCnt             = 32;
    struVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
    struVbConf.astCommPool[0].u32BlkCnt  = 50;

    struVbConf.astCommPool[1].u32BlkSize = 1920*1088*2;
    struVbConf.astCommPool[1].u32BlkCnt  = 10;

    s32Ret = HI_MPI_VB_SetConf(&struVbConf);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_VB_SetConf fail! s32Ret:0x%x\n", s32Ret);
        return s32Ret;
    }
    
    s32Ret = HI_MPI_VB_Init();
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_VB_Init fail! s32Ret:0x%x\n", s32Ret);
        return s32Ret;
    }
    
    struSysConf.u32AlignWidth = 64;
    s32Ret = HI_MPI_SYS_SetConf(&struSysConf);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_SYS_SetConf fail! s32Ret:0x%x\n", s32Ret);
        (HI_VOID)HI_MPI_VB_Exit();
        return s32Ret;
    }
    
    s32Ret = HI_MPI_SYS_Init();
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_SYS_Init fail! s32Ret:0x%x\n", s32Ret);
        (HI_VOID)HI_MPI_VB_Exit();
        return s32Ret;
    }   

    s32Ret = HI_MPI_VB_ExitModCommPool(VB_UID_VDEC);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_VB_ExitModCommPool fail! s32Ret:0x%x\n", s32Ret);
        (HI_VOID)HI_MPI_VB_Exit();
        return s32Ret;
    }
    
    memset(&stVbConf, 0, sizeof(VB_CONF_S));
    
    stVbConf.u32MaxPoolCnt               = 3;
    stVbConf.astCommPool[0].u32BlkSize   = 720*576*2;
    stVbConf.astCommPool[0].u32BlkCnt    = 32;

    stVbConf.astCommPool[1].u32BlkSize   = 720*576/4;
    stVbConf.astCommPool[1].u32BlkCnt    = 32;

    stVbConf.astCommPool[2].u32BlkSize   = 1920*1088*2;
    stVbConf.astCommPool[2].u32BlkCnt    = 10;
    
    s32Ret = HI_MPI_VB_SetModPoolConf(VB_UID_VDEC, &stVbConf);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_VB_SetModPoolConf fail! s32Ret:0x%x\n", s32Ret);
        (HI_VOID)HI_MPI_VB_Exit();
        return s32Ret;
    }
    
    s32Ret = HI_MPI_VB_InitModCommPool(VB_UID_VDEC);		
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_VB_InitModCommPool fail! s32Ret:0x%x\n", s32Ret);
        (HI_VOID)HI_MPI_VB_Exit();
        return s32Ret;
    }
    
    return HI_SUCCESS;
}


HI_S32 SAMPLE_VDA_SYS_Exit(HI_VOID)
{
    HI_MPI_VB_ExitModCommPool(VB_UID_VDEC);
    HI_MPI_SYS_Exit();
    HI_MPI_VB_Exit();

    return HI_SUCCESS;
}

HI_S32 SAMPLE_VDA_GetVoLayer(VO_DEV VoDev)
{
    HI_S32 s32LayerNum;
    
    if (0 == VoDev)
    {
        s32LayerNum = 0;
    }
    else if (1 == VoDev)
    {
        s32LayerNum = 2;
    }
    else
    {
        return -1;
    }

    return s32LayerNum;
}


HI_S32 SAMPLE_VDA_GetVoDisplayNum(HI_U32 u32VoChnNum)
{
    HI_S32 s32DispNum;

    if (1 == u32VoChnNum)
    {
        s32DispNum = 1;
    }
    else if (4 == u32VoChnNum)
    {
        s32DispNum = 2;
    }
    else if (9 == u32VoChnNum)
    {
        s32DispNum = 3;
    }
    else if (16 == u32VoChnNum)
    {
        s32DispNum = 4;
    }
    else
    {
        return -1;
    }

    return s32DispNum;
}



HI_S32 SAMPLE_VDA_GetVoAttr(VO_DEV VoDev, VO_INTF_SYNC_E enIntfSync, VO_PUB_ATTR_S *pstPubAttr,
        VO_VIDEO_LAYER_ATTR_S *pstLayerAttr, HI_S32 s32SquareSort, VO_CHN_ATTR_S *astChnAttr)
{    
    VO_INTF_TYPE_E enIntfType;
    HI_U32 u32Frmt, u32Width, u32Height, j;
 
    switch (VoDev)
    {
        case 0: enIntfType = VO_INTF_VGA | VO_INTF_HDMI; break;
        case 1: enIntfType = VO_INTF_CVBS; break;
    }

    switch (enIntfSync)
    {
        case VO_OUTPUT_PAL      :    u32Width = 720;  u32Height = 576;  u32Frmt = 25; break;
        case VO_OUTPUT_NTSC     :    u32Width = 720;  u32Height = 480;  u32Frmt = 30; break;
        case VO_OUTPUT_1080P24  :    u32Width = 1920; u32Height = 1080; u32Frmt = 24; break;
        case VO_OUTPUT_1080P25  :    u32Width = 1920; u32Height = 1080; u32Frmt = 25; break;
        case VO_OUTPUT_1080P30  :    u32Width = 1920; u32Height = 1080; u32Frmt = 30; break;
        case VO_OUTPUT_720P50   :    u32Width = 1280; u32Height = 720;  u32Frmt = 50; break;
        case VO_OUTPUT_720P60   :    u32Width = 1280; u32Height = 720;  u32Frmt = 60; break;
        case VO_OUTPUT_1080I50  :    u32Width = 1920; u32Height = 1080; u32Frmt = 50; break;
        case VO_OUTPUT_1080I60  :    u32Width = 1920; u32Height = 1080; u32Frmt = 60; break;
        case VO_OUTPUT_1080P50  :    u32Width = 1920; u32Height = 1080; u32Frmt = 50; break;
        case VO_OUTPUT_1080P60  :    u32Width = 1920; u32Height = 1080; u32Frmt = 60; break;
        case VO_OUTPUT_576P50   :    u32Width = 720;  u32Height = 576;  u32Frmt = 50; break;
        case VO_OUTPUT_480P60   :    u32Width = 720;  u32Height = 480;  u32Frmt = 60; break;
        case VO_OUTPUT_800x600_60:   u32Width = 800;  u32Height = 600;  u32Frmt = 60; break;
        case VO_OUTPUT_1024x768_60:  u32Width = 1024; u32Height = 768;  u32Frmt = 60; break;
        case VO_OUTPUT_1280x1024_60: u32Width = 1280; u32Height = 1024; u32Frmt = 60; break;
        case VO_OUTPUT_1366x768_60:  u32Width = 1366; u32Height = 768;  u32Frmt = 60; break;
        case VO_OUTPUT_1440x900_60:  u32Width = 1440; u32Height = 900;  u32Frmt = 60; break;
        case VO_OUTPUT_1280x800_60:  u32Width = 1280; u32Height = 800;  u32Frmt = 60; break;

        default: return HI_FAILURE;
    }

    if (NULL != pstPubAttr)
    {
        pstPubAttr->enIntfSync = enIntfSync;
        pstPubAttr->u32BgColor = 0; //0xFF; //BLUE
        pstPubAttr->enIntfType = enIntfType;
    }

    if (NULL != pstLayerAttr)
    {
        pstLayerAttr->stDispRect.s32X       = 0;
        pstLayerAttr->stDispRect.s32Y       = 0;
        pstLayerAttr->stDispRect.u32Width   = u32Width;
        pstLayerAttr->stDispRect.u32Height  = u32Height;
        pstLayerAttr->stImageSize.u32Width  = u32Width;
        pstLayerAttr->stImageSize.u32Height = u32Height;
        pstLayerAttr->bDoubleFrame          = HI_FALSE;
        pstLayerAttr->bClusterMode          = HI_FALSE;
        pstLayerAttr->u32DispFrmRt          = u32Frmt;
        pstLayerAttr->enPixFormat           = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
    }

    if (NULL != astChnAttr)
    {
        for (j=0; j<(s32SquareSort * s32SquareSort); j++)
        {
            astChnAttr[j].stRect.s32X       = ALIGN_BACK((u32Width / s32SquareSort) * (j % s32SquareSort), 4);
            astChnAttr[j].stRect.s32Y       = ALIGN_BACK((u32Height / s32SquareSort) * (j / s32SquareSort), 4);
            astChnAttr[j].stRect.u32Width   = ALIGN_BACK(u32Width / s32SquareSort, 4);
            astChnAttr[j].stRect.u32Height  = ALIGN_BACK(u32Height / s32SquareSort, 4);
            astChnAttr[j].u32Priority       = 0;
            astChnAttr[j].bDeflicker        = HI_FALSE;
        }
    }
    
    return HI_SUCCESS;
}

HI_S32 SAMPLE_VDA_StartVpss(HI_S32 s32VpssGrpNum, HI_U32 u32VpssChn)
{
    HI_S32 i = 0;
    HI_S32 s32Ret;
    HI_U32 u32Depth;
    VPSS_CHN_MODE_S stVpssChnMode;
    VPSS_GRP_ATTR_S stGrpAttr;
    HI_U32 u32OverlayMask;
        
    stGrpAttr.u32MaxW   = 720;
    stGrpAttr.u32MaxH   = 576;
    stGrpAttr.enPixFmt  = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
    stGrpAttr.enDieMode = VPSS_DIE_MODE_NODIE;
    stGrpAttr.bIeEn     = HI_FALSE;
    stGrpAttr.bNrEn     = HI_FALSE;
    stGrpAttr.bHistEn   = HI_FALSE;
    stGrpAttr.bDciEn    = HI_FALSE;
    stGrpAttr.bEsEn     = HI_FALSE;
	
    for (i = 0; i < s32VpssGrpNum; i++)
    {
        s32Ret = HI_MPI_VPSS_CreateGrp(i, &stGrpAttr);
        if (HI_SUCCESS != s32Ret)
        {
            printf("creat vpss grp%d fail, s32Ret: 0x%x.\n", i, s32Ret);
            return s32Ret;

        }

        s32Ret = HI_MPI_VPSS_EnableChn(i, u32VpssChn);
        if (HI_SUCCESS != s32Ret)
        {
            printf("creat vpss grp%d chnl%d fail, s32Ret: 0x%x.\n", i, u32VpssChn, s32Ret);
            return s32Ret;
        }

        s32Ret = HI_MPI_VPSS_GetChnMode(i, u32VpssChn, &stVpssChnMode);
        if (HI_SUCCESS != s32Ret)
        {
            printf("get vpss grp%d chn%d mode fail, s32Ret: 0x%x.\n", i, u32VpssChn, s32Ret);
            return s32Ret;
        }
        
        stVpssChnMode.bDouble 	     = HI_FALSE;
        stVpssChnMode.enChnMode 	 = VPSS_CHN_MODE_USER;
        stVpssChnMode.enPixelFormat  = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
        stVpssChnMode.u32Width 	     = 720;
        stVpssChnMode.u32Height 	 = 576;
        stVpssChnMode.enCompressMode = COMPRESS_MODE_NONE;
        
        s32Ret = HI_MPI_VPSS_SetChnMode(i, u32VpssChn, &stVpssChnMode);
        if (HI_SUCCESS != s32Ret)
        {
            printf("set vpss grp%d chn%d mode fail, s32Ret: 0x%x.\n", i, u32VpssChn, s32Ret);
            return s32Ret;
        }
        
        s32Ret = HI_MPI_VPSS_StartGrp(i);
        if (HI_SUCCESS != s32Ret)
        {
            printf("start vpss grp%d fail, s32Ret: 0x%x.\n", i, s32Ret);
            return s32Ret;
        }

        u32Depth = 6;
        s32Ret = HI_MPI_VPSS_SetDepth(i, u32VpssChn, u32Depth);
        if (HI_SUCCESS != s32Ret)
        {
            printf("HI_MPI_VPSS_SetDepth fail! Grp: %d, Chn: %d! s32Ret: 0x%x.\n", i, u32VpssChn, s32Ret);
            return s32Ret;
        }

        u32OverlayMask = 255;
        s32Ret = HI_MPI_VPSS_SetChnOverlay(i, u32VpssChn, u32OverlayMask);
        if (HI_SUCCESS != s32Ret)
        {
            printf("HI_MPI_VPSS_SetChnOverlay fail! Grp: %d, Chn: %d! s32Ret: 0x%x.\n", i, u32VpssChn, s32Ret);
            return s32Ret;
        }

    }

    return HI_SUCCESS;
}


HI_S32 SAMPLE_VDA_StartVpssHD(HI_S32 s32VpssGrpNum, HI_U32 u32VpssChn)
{
    HI_S32 i = 0;
    HI_S32 s32Ret;
    HI_U32 u32Depth;
    VPSS_CHN_MODE_S stVpssChnMode;
    VPSS_GRP_ATTR_S stGrpAttr;
    HI_U32 u32OverlayMask;
        
    stGrpAttr.u32MaxW      = 1920;
    stGrpAttr.u32MaxH      = 1080;
    stGrpAttr.enPixFmt     = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
    stGrpAttr.enDieMode    = VPSS_DIE_MODE_AUTO;
    stGrpAttr.bIeEn        = HI_FALSE;
    stGrpAttr.bNrEn        = HI_FALSE;
    stGrpAttr.bHistEn      = HI_FALSE;
    stGrpAttr.bDciEn       = HI_FALSE;
        	
    for (i = 0; i < s32VpssGrpNum; i++)
    {
        s32Ret = HI_MPI_VPSS_CreateGrp(i, &stGrpAttr);
        if (HI_SUCCESS != s32Ret)
        {
            printf("creat vpss grp%d fail! s32Ret: 0x%x.\n", i, s32Ret);
            return s32Ret;

        }

        s32Ret = HI_MPI_VPSS_EnableChn(i, u32VpssChn);
        if (HI_SUCCESS != s32Ret)
        {
            printf("creat vpss grp%d chnl%d fail! s32Ret: 0x%x.\n", i, u32VpssChn, s32Ret);
            return s32Ret;
        }

        stVpssChnMode.bDouble 	     = HI_FALSE;
        stVpssChnMode.enChnMode 	 = VPSS_CHN_MODE_AUTO;
        stVpssChnMode.enPixelFormat  = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
        stVpssChnMode.u32Width 	     = 1920;
        stVpssChnMode.u32Height 	 = 1080;
        stVpssChnMode.enCompressMode = COMPRESS_MODE_NONE;
        
        s32Ret = HI_MPI_VPSS_SetChnMode(i, u32VpssChn, &stVpssChnMode);
        if (HI_SUCCESS != s32Ret)
        {
            printf("set vpss grp%d chn%d mode fail! s32Ret: 0x%x.\n", i, u32VpssChn, s32Ret);
            return s32Ret;
        }
        
        s32Ret = HI_MPI_VPSS_StartGrp(i);
        if (HI_SUCCESS != s32Ret)
        {
            printf("start vpss grp%d fail! s32Ret: 0x%x.\n", i, s32Ret);
            return s32Ret;
        }

        u32Depth = 6;
        s32Ret = HI_MPI_VPSS_SetDepth(i, u32VpssChn, u32Depth);
        if (HI_SUCCESS != s32Ret)
        {
            printf("HI_MPI_VPSS_SetDepth fail! Grp: %d, Chn: %d! s32Ret: 0x%x.\n", i, u32VpssChn, s32Ret);
            return s32Ret;
        }

        u32OverlayMask = 255;
        s32Ret = HI_MPI_VPSS_SetChnOverlay(i, u32VpssChn, u32OverlayMask);
        if (HI_SUCCESS != s32Ret)
        {
            printf("HI_MPI_VPSS_SetChnOverlay fail! Grp: %d, Chn: %d! s32Ret: 0x%x.\n", i, u32VpssChn, s32Ret);
            return s32Ret;
        }

    }

    return HI_SUCCESS;
}


HI_S32 SAMPLE_VDA_StopVpss(HI_S32 s32VpssGrpNum)
{
    HI_S32 i = 0;
    HI_S32 s32Ret;

    for (i = 0; i < s32VpssGrpNum; i++)
    {
        s32Ret =  HI_MPI_VPSS_StopGrp(i);
        if (HI_SUCCESS != s32Ret)
        {
            printf("stop vpss grp%d fail! s32Ret: 0x%x.\n", i, s32Ret);
            return s32Ret;
        }

        s32Ret =  HI_MPI_VPSS_DestroyGrp(i);
        if (HI_SUCCESS != s32Ret)
        {
            printf("destroy vpss grp%d fail! s32Ret: 0x%x.\n", i, s32Ret);
            return s32Ret;
        }
        
    }
    //printf("destroy vpss ok!\n");

    return HI_SUCCESS;
}


HI_S32 SAMPLE_VDA_StartVdec(VDEC_CHN VdecChn)
{
    HI_S32 s32Ret;
    VDEC_CHN_ATTR_S stVdecAttr;
    
    stVdecAttr.enType       = PT_H264;
    stVdecAttr.u32Priority  = 1;//should be greater than 0
    stVdecAttr.u32PicWidth  = 720;
    stVdecAttr.u32PicHeight = 576;
    stVdecAttr.u32BufSize   = stVdecAttr.u32PicWidth * stVdecAttr.u32PicHeight;//This item should larger than u32Width*u32Height/2
    stVdecAttr.stVdecVideoAttr.u32RefFrameNum   = 1;
    stVdecAttr.stVdecVideoAttr.bTemporalMvpEnable = 0;
    stVdecAttr.stVdecVideoAttr.enMode           = VIDEO_MODE_FRAME;

    /* create vdec chn*/
    s32Ret = HI_MPI_VDEC_CreateChn(VdecChn, &stVdecAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_VDEC_CreateChn failed! s32Ret:0x%x.\n", s32Ret);
        return s32Ret;
    }

    /* start vdec to receive stream sent by user*/
    s32Ret = HI_MPI_VDEC_StartRecvStream(VdecChn);    
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_VDEC_StartRecvStream failed! s32Ret:0x%x.\n", s32Ret);
        return s32Ret;
    }
    
    printf("Vdec chn create and start receive stream ok!\n");

    return HI_SUCCESS;
}

HI_S32 SAMPLE_VDA_StopVdec(VDEC_CHN VdecChn)
{
    HI_S32 s32Ret;
    
    /* stop vdec to receive stream sent by user*/
    s32Ret = HI_MPI_VDEC_StopRecvStream(VdecChn);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_VDEC_StopRecvStream failed! s32Ret:0x%x.\n", s32Ret);
        return s32Ret;
    }
    
    /* destroy vdec chn*/
    s32Ret = HI_MPI_VDEC_DestroyChn(VdecChn);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_VDEC_DestroyChn failed! s32Ret:0x%x.\n", s32Ret);
        return s32Ret;
    }
    
    printf("Vdec chn stop receive stream and destroy ok!\n");
    
    return HI_SUCCESS;
}


HI_S32 SAMPLE_VDA_StartVo(VO_DEV VoDev, HI_U32 u32VoChnNum, VO_INTF_SYNC_E enIntfSync)
{
    HI_S32 i;
    HI_S32 s32Ret;
    HI_S32 s32DispNum;
    VO_LAYER VoLayer;
    VO_PUB_ATTR_S stPubAttr;
    VO_VIDEO_LAYER_ATTR_S stLayerAttr;
    VO_CHN_ATTR_S astChnAttr[16];

    s32DispNum = SAMPLE_VDA_GetVoDisplayNum(u32VoChnNum);
    if(s32DispNum < 0)
    {
        printf("SAMPLE_RGN_GetVoDisplayNum failed! u32VoChnNum: %d.\n", u32VoChnNum);
        return HI_FAILURE;
    }
    
    s32Ret = SAMPLE_VDA_GetVoAttr(VoDev, enIntfSync, &stPubAttr, &stLayerAttr, s32DispNum, astChnAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("SAMPLE_RGN_GetVoAttr failed!\n");
        return HI_FAILURE;
    }

    VoLayer = SAMPLE_VDA_GetVoLayer(VoDev);
    if(VoLayer < 0)
    {
        printf("SAMPLE_RGN_GetVoLayer failed! VoDev: %d.\n", VoDev);
        return HI_FAILURE;
    }
  
    s32Ret = HI_MPI_VO_Disable(VoDev);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_VO_Disable failed! s32Ret: 0x%x.\n", s32Ret);
        return s32Ret;
    }

    s32Ret = HI_MPI_VO_SetPubAttr(VoDev, &stPubAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_VO_SetPubAttr failed! s32Ret: 0x%x.\n", s32Ret);
        return s32Ret;
    }

    s32Ret = HI_MPI_VO_Enable(VoDev);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_VO_Enable failed! s32Ret: 0x%x.\n", s32Ret);
        return s32Ret;
    }
    //printf("VO dev:%d enable ok \n", VoDev);
  
    s32Ret = HI_MPI_VO_SetVideoLayerAttr(VoLayer, &stLayerAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_VO_SetVideoLayerAttr failed! s32Ret: 0x%x.\n", s32Ret);
        return s32Ret;
    }
    
    s32Ret = HI_MPI_VO_EnableVideoLayer(VoLayer);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_VO_EnableVideoLayer failed! s32Ret: 0x%x.\n", s32Ret);
        return s32Ret;
    }

    //printf("VO video layer:%d enable ok \n", VoDev);

    for (i = 0; i < u32VoChnNum; i++)
    {
        s32Ret = HI_MPI_VO_SetChnAttr(VoLayer, i, &astChnAttr[i]);
        if (HI_SUCCESS != s32Ret)
        {
            printf("HI_MPI_VO_SetChnAttr failed! s32Ret: 0x%x.\n", s32Ret);
            return s32Ret;
        }

        s32Ret = HI_MPI_VO_EnableChn(VoLayer, i);
        if (HI_SUCCESS != s32Ret)
        {
            printf("HI_MPI_VO_EnableChn failed! s32Ret: 0x%x.\n", s32Ret);
            return s32Ret;
        }
        
        //printf("VO chn:%d enable ok \n", i);
    }

    //printf("VO: %d enable ok!\n", VoDev);
    
    return HI_SUCCESS;
}


HI_S32 SAMPLE_VDA_StopVoChn(VO_DEV VoDev, HI_U32 u32VoChnNum)
{
    HI_S32 i;
    HI_S32 s32Ret;
    VO_LAYER VoLayer;
    
    VoLayer = SAMPLE_VDA_GetVoLayer(VoDev);
    if(VoLayer < 0)
    {
        printf("SAMPLE_RGN_GetVoLayer failed! VoDev: %d.\n", VoDev);
        return HI_FAILURE;
    }
    
    for (i = 0; i< u32VoChnNum; i++)
    {
        s32Ret = HI_MPI_VO_DisableChn(VoLayer, i);
        if (HI_SUCCESS != s32Ret)
        {
            printf("HI_MPI_VO_DisableChn failed! s32Ret: 0x%x.\n", s32Ret);
            return s32Ret;
        }

        //printf("VO chn : %d stop ok!\n", i);
    }
    
    return HI_SUCCESS;
}


HI_S32 SAMPLE_VDA_StopVoDev(VO_DEV VoDev)
{
    HI_S32 s32Ret;
    VO_LAYER VoLayer;

    VoLayer = SAMPLE_VDA_GetVoLayer(VoDev);
    if(VoLayer < 0)
    {
        printf("SAMPLE_RGN_GetVoLayer failed! VoDev: %d.\n", VoDev);
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_VO_DisableVideoLayer(VoLayer);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_VO_DisableVideoLayer failed! s32Ret: 0x%x.\n", s32Ret);
        return s32Ret;
    }

    s32Ret = HI_MPI_VO_Disable(VoDev);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_VO_Disable failed! s32Ret:0x%x.\n", s32Ret);
        return s32Ret;
    }

    //printf("VO dev: %d stop ok!\n", VoDev);
    
    return 0;
}


/******************************************************************************
 * function : send stream to vdec
 ******************************************************************************/
void* SAMPLE_VDA_VdecSendStream(void* p)
{
	VDEC_STREAM_S stStream;
	VDEC_SENDPARAM_S *pstSendParam;
	char sFileName[50], sFilePostfix[20] = ".h264";
	FILE* fp = NULL;
	HI_S32 s32Ret;
	//struct timeval *ptv; 
	HI_U8 *pu8Buf;
	HI_S32 i;
	HI_BOOL sHasReadStream = HI_FALSE; 
	HI_BOOL bFindStart, bFindEnd;
	HI_S32 start = 0;
	HI_S32 s32UsedBytes = 0, s32ReadLen = 0;
	HI_S32 len;
	HI_U64 u64pts = 0;

	pstSendParam = (VDEC_SENDPARAM_S *)p;

	/*open the stream file*/
	sprintf(sFileName, "D1chn25%s", sFilePostfix);
	fp = fopen(sFileName, "r");
	if (HI_NULL == fp)
	{
		printf("open file %s err\n", sFileName);
		return NULL;
	}
	//printf("open file [%s] ok!\n", sFileName);

	if(0 != pstSendParam->s32MinBufSize)
	{
		pu8Buf=malloc(pstSendParam->s32MinBufSize);
		if(NULL == pu8Buf)
		{
			printf("can't alloc %d in send stream thread:%d\n",pstSendParam->s32MinBufSize,pstSendParam->VdChn);
			fclose(fp);
			return (HI_VOID *)(HI_FAILURE);
		}
	}
	else
	{
		printf("none buffer to operate in send stream thread:%d\n",pstSendParam->VdChn);
		return (HI_VOID *)(HI_FAILURE);
	}
	
	//ptv = (struct timeval *)&stStream.u64PTS;

	while (HI_FALSE == bExit)
	{
	   if ( (pstSendParam->enVideoMode==VIDEO_MODE_FRAME) && (pstSendParam->enPayload == PT_MP4VIDEO) )
		{
			bFindStart = HI_FALSE;	
			bFindEnd   = HI_FALSE;
			fseek(fp, s32UsedBytes, SEEK_SET);
			s32ReadLen = fread(pu8Buf, 1, pstSendParam->s32MinBufSize, fp);
			if (s32ReadLen == 0)
			{
					s32UsedBytes = 0;
					fseek(fp, 0, SEEK_SET);
					s32ReadLen = fread(pu8Buf, 1, pstSendParam->s32MinBufSize, fp);
			}

			for (i=0; i<s32ReadLen-4; i++)
			{
				if (pu8Buf[i] == 0 && pu8Buf[i+1] == 0 && pu8Buf[i+2] == 1 && pu8Buf[i+3] == 0xB6)
				{
					bFindStart = HI_TRUE;
					i += 4;
					break;
				}
			}

			for (; i<s32ReadLen-4; i++)
			{
				if (pu8Buf[i  ] == 0 && pu8Buf[i+1] == 0 && pu8Buf[i+2] == 1 && pu8Buf[i+3] == 0xB6)
				{
					bFindEnd = HI_TRUE;
					break;
				}
			}

			s32ReadLen = i;
			if (bFindStart == HI_FALSE)
			{
				printf("SAMPLE_TEST: chn %d can not find start code! s32ReadLen %d, s32UsedBytes %d. \n", 
											pstSendParam->VdChn, s32ReadLen, s32UsedBytes);
			}
			else if (bFindEnd == HI_FALSE)
			{
				s32ReadLen = i+4;
			}
			
		}
		else if ( (pstSendParam->enVideoMode==VIDEO_MODE_FRAME) && (pstSendParam->enPayload == PT_H264) )
		{
			bFindStart = HI_FALSE;	
			bFindEnd   = HI_FALSE;
			fseek(fp, s32UsedBytes, SEEK_SET);
			s32ReadLen = fread(pu8Buf, 1, pstSendParam->s32MinBufSize, fp);
			if (s32ReadLen == 0)
			{
					s32UsedBytes = 0;
					fseek(fp, 0, SEEK_SET);
					s32ReadLen = fread(pu8Buf, 1, pstSendParam->s32MinBufSize, fp);
			}
		 
			for (i=0; i<s32ReadLen-5; i++)
			{
				if (  pu8Buf[i	] == 0 && pu8Buf[i+1] == 0 && pu8Buf[i+2] == 1 && 
					 ( (pu8Buf[i+3]&0x1F) == 0x5 || (pu8Buf[i+3]&0x1F) == 0x1 ) &&
					 ( (pu8Buf[i+4]&0x80) == 0x80)
				   )				 
				{
					bFindStart = HI_TRUE;
					i += 4;
					break;
				}
			}

			for (; i<s32ReadLen-5; i++)
			{
				if (  pu8Buf[i	] == 0 && pu8Buf[i+1] == 0 && pu8Buf[i+2] == 1 && 
					( ((pu8Buf[i+3]&0x1F) == 0x7) || ((pu8Buf[i+3]&0x1F) == 0x8) || ((pu8Buf[i+3]&0x1F) == 0x6)
					  || (((pu8Buf[i+3]&0x1F) == 0x5 || (pu8Buf[i+3]&0x1F) == 0x1) &&((pu8Buf[i+4]&0x80) == 0x80))
					)
				   )
				{
					bFindEnd = HI_TRUE;
					break;
				}
			}

			if(i > 0) s32ReadLen = i;
			if (bFindStart == HI_FALSE)
			{
				printf("SAMPLE_TEST: chn %d can not find start code!s32ReadLen %d, s32UsedBytes %d. \n", 
											pstSendParam->VdChn, s32ReadLen, s32UsedBytes);
			}
			else if (bFindEnd == HI_FALSE)
			{
				s32ReadLen = i+5;
			}
			
		}
		else if ((pstSendParam->enPayload == PT_MJPEG) )
		{
			bFindStart = HI_FALSE;	
			bFindEnd   = HI_FALSE;			
			fseek(fp, s32UsedBytes, SEEK_SET);
			s32ReadLen = fread(pu8Buf, 1, pstSendParam->s32MinBufSize, fp);
			if (s32ReadLen == 0)
			{
					s32UsedBytes = 0;
					fseek(fp, 0, SEEK_SET);
					s32ReadLen = fread(pu8Buf, 1, pstSendParam->s32MinBufSize, fp);
			}

		   
			for (i=0; i<s32ReadLen-2; i++)
			{
				if (pu8Buf[i] == 0xFF && pu8Buf[i+1] == 0xD8) 
				{  
					start = i;
					bFindStart = HI_TRUE;
					i = i + 2;
					break;
				}  
			}

			for (; i<s32ReadLen-4; i++)
			{
				if ( (pu8Buf[i] == 0xFF) && (pu8Buf[i+1]& 0xF0) == 0xE0 )
				{	
					 len = (pu8Buf[i+2]<<8) + pu8Buf[i+3];					  
					 i += 1 + len;					
				}
				else
				{
					break;
				}
			}

			for (; i<s32ReadLen-2; i++)
			{
				if (pu8Buf[i] == 0xFF && pu8Buf[i+1] == 0xD8)
				{
					bFindEnd = HI_TRUE;
					break;
				} 
			}					 
			s32ReadLen = i;
			if (bFindStart == HI_FALSE)
			{
				printf("SAMPLE_TEST: chn %d can not find start code! s32ReadLen %d, s32UsedBytes %d. \n", 
											pstSendParam->VdChn, s32ReadLen, s32UsedBytes);
			}
			else if (bFindEnd == HI_FALSE)
			{
				s32ReadLen = i+2;
			}
		}
		 else if ((pstSendParam->enPayload == PT_JPEG) )
		{
			if (HI_TRUE != sHasReadStream)
			{				
		   
				bFindStart = HI_FALSE;	
				bFindEnd   = HI_FALSE; 
				
				fseek(fp, s32UsedBytes, SEEK_SET);
				s32ReadLen = fread(pu8Buf, 1, pstSendParam->s32MinBufSize, fp);
				if (s32ReadLen == 0)
				{
						s32UsedBytes = 0;
						fseek(fp, 0, SEEK_SET);
						s32ReadLen = fread(pu8Buf, 1, pstSendParam->s32MinBufSize, fp);
				}

			   
				for (i=0; i<s32ReadLen-2; i++)
				{
					if (pu8Buf[i] == 0xFF && pu8Buf[i+1] == 0xD8) 
					{  
						start = i;
						bFindStart = HI_TRUE;
						i = i + 2;
						break;
					}  
				}

				for (; i<s32ReadLen-4; i++)
				{
					if ( (pu8Buf[i] == 0xFF) && (pu8Buf[i+1]& 0xF0) == 0xE0 )
					{	
						 len = (pu8Buf[i+2]<<8) + pu8Buf[i+3];					  
						 i += 1 + len;					
					}
					else
					{
						break;
					}
				}

				for (; i<s32ReadLen-2; i++)
				{
					if (pu8Buf[i] == 0xFF && pu8Buf[i+1] == 0xD8)
					{					 
						bFindEnd = HI_TRUE;
						break;
					} 
				}					 
				s32ReadLen = i;
				if (bFindStart == HI_FALSE)
				{
					printf("SAMPLE_TEST: chn %d can not find start code! s32ReadLen %d, s32UsedBytes %d. \n", 
												pstSendParam->VdChn, s32ReadLen, s32UsedBytes);
				}
				else if (bFindEnd == HI_FALSE)
				{
					s32ReadLen = i+2;
				}
				 sHasReadStream = HI_TRUE;
			}
		}
		else
		{
			fseek(fp, s32UsedBytes, SEEK_SET);
			s32ReadLen = fread(pu8Buf, 1, pstSendParam->s32MinBufSize, fp);
			if (s32ReadLen == 0)
			{
					s32UsedBytes = 0;
					fseek(fp, 0, SEEK_SET);
					s32ReadLen = fread(pu8Buf, 1, pstSendParam->s32MinBufSize, fp);
			}
		}

		stStream.u64PTS  = u64pts;
		stStream.pu8Addr = pu8Buf + start;
		stStream.u32Len  = s32ReadLen; 
		stStream.bEndOfFrame  = (pstSendParam->enVideoMode==VIDEO_MODE_FRAME)? HI_TRUE: HI_FALSE;
		stStream.bEndOfStream = HI_FALSE;					
		
		s32Ret=HI_MPI_VDEC_SendStream(pstSendParam->VdChn, &stStream, -1);
		if (HI_SUCCESS != s32Ret)
		{
			usleep(100);
		}
		else
		{
			s32UsedBytes = s32UsedBytes +s32ReadLen + start;	
			u64pts += 33333;
		}
	   usleep(1000);
	}
	fflush(stdout);
	free(pu8Buf);
	fclose(fp);

	return HI_NULL;
}

/******************************************************************************
* Description      : VDEC->VPSS->MD(480*270) 
******************************************************************************/
HI_S32 SAMPLE_VDA_MD(HI_VOID)

{
    HI_S32 s32Ret = HI_SUCCESS;
    VDA_CHN VdaChn_Md = 0;
    PIC_SIZE_E enSize_Md = PIC_CIF; 	/* vda picture size */
    VDEC_CHN VdecChn;
    HI_S32 s32VpssGrpNum;
    HI_U32 u32VpssChn;
    VO_DEV VoDev;
    VO_LAYER VoLayer;
    HI_U32 s32VoChnNum;
    VO_INTF_SYNC_E enIntfSync;
    pthread_t stVdecThread;
    VDEC_SENDPARAM_S stVdesSendPram;
    SIZE_S stSize;
    MPP_CHN_S stSrcChn, stDesChn;
    
    /*************************************************
    step  1: mpp system init
    *************************************************/
    s32Ret = SAMPLE_VDA_SYS_Init();
    if (HI_SUCCESS != s32Ret)
    {
        printf("system init failed! s32Ret: 0x%x.\n", s32Ret);
        goto END_0;
    }
        
    /*************************************************
    step 2: start vdec chn
    *************************************************/
    VdecChn = 0;
    s32Ret = SAMPLE_VDA_StartVdec(VdecChn);
    if(HI_SUCCESS != s32Ret)
    {
        printf("SAMPLE_VDA_StartVdec failed! s32Ret: 0x%x.\n", s32Ret);
        goto END_0;
    }
    

    /*************************************************
    step 3: start vpss group and chn
    *************************************************/
    s32VpssGrpNum = 1;
    u32VpssChn    = VPSS_CHN2;
    s32Ret = SAMPLE_VDA_StartVpss(s32VpssGrpNum, u32VpssChn);
    if(HI_SUCCESS != s32Ret)
    {
        printf("SAMPLE_VDA_StartVpss failed! s32Ret: 0x%x.\n", s32Ret);
        goto END_0;
    }
    
    /*************************************************
    step 4: start vo dev and chn
    *************************************************/
    VoDev       = 1;
    s32VoChnNum = 1;
    enIntfSync  = VO_OUTPUT_PAL;
    s32Ret = SAMPLE_VDA_StartVo(VoDev, s32VoChnNum, enIntfSync);
    if(HI_SUCCESS != s32Ret)
    {
        printf("SAMPLE_VDA_StartVo failed! s32Ret: 0x%x.\n", s32Ret);
        goto END_0;
    }

    /*************************************************
    step 5: bind vdec and vpss
    *************************************************/
    stSrcChn.enModId  = HI_ID_VDEC;
    stSrcChn.s32DevId = 0;
    stSrcChn.s32ChnId = 0;

    stDesChn.enModId  = HI_ID_VPSS;
    stDesChn.s32DevId = 0;
    stDesChn.s32ChnId = 0;

    s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDesChn);
    if(HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_SYS_Bind failed! s32Ret: 0x%x.\n", s32Ret);
        goto END_0;
    }

    /*************************************************
    step 6: bind vpss and vo
    *************************************************/
    VoLayer = SAMPLE_VDA_GetVoLayer(VoDev);
    if(VoLayer < 0)
    {
        printf("SAMPLE_VDA_GetVoLayer failed! VoDev: %d.\n", VoDev);
        s32Ret = HI_FAILURE;
        goto END_0;
    }
    
    stSrcChn.enModId  = HI_ID_VPSS;
    stSrcChn.s32DevId = 0;
    stSrcChn.s32ChnId = u32VpssChn;

    stDesChn.enModId  = HI_ID_VOU;
    stDesChn.s32DevId = VoLayer;
    stDesChn.s32ChnId = 0;

    s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDesChn);
    if(HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_SYS_Bind failed! s32Ret: 0x%x.\n", s32Ret);
        goto END_0;
    }
    
    /*************************************************
    step 7: create a thread for vdec to read stream
    from a file
    *************************************************/    
    stSize.u32Width  = 720;
    stSize.u32Height = 576;
    
    stVdesSendPram.bRun          = HI_TRUE;
    stVdesSendPram.VdChn         = VdecChn;
    stVdesSendPram.enPayload     = PT_H264;
    stVdesSendPram.enVideoMode   = VIDEO_MODE_FRAME;
    stVdesSendPram.s32MinBufSize = stSize.u32Height * stSize.u32Width / 2;
    pthread_create(&stVdecThread, NULL, SAMPLE_VDA_VdecSendStream, (HI_VOID*)&stVdesSendPram);


    /*************************************************
    step  8: start VDA MD process
    *************************************************/
    s32Ret = SAMPLE_COMM_SYS_GetPicSize(gs_enNorm, enSize_Md, &stSize);
    if (HI_SUCCESS != s32Ret)
    {
        printf("SAMPLE_COMM_SYS_GetPicSize failed! s32Ret: 0x%x.\n", s32Ret);
        goto END_0;
    }

    if (0 != stSize.u32Width % VDA_WIDTH_ALIGN)
    {
        stSize.u32Width = (stSize.u32Width / VDA_WIDTH_ALIGN + 1) * VDA_WIDTH_ALIGN;
    }

    if (0 != stSize.u32Height % VDA_HEIGHT_ALIGN)
    {
        stSize.u32Height = (stSize.u32Height / VDA_HEIGHT_ALIGN + 1) * VDA_HEIGHT_ALIGN;
    }
    
    s32Ret = SAMPLE_COMM_VDA_MdStart(VdaChn_Md, u32VpssChn, &stSize);
    if (HI_SUCCESS != s32Ret)
    {
        printf("VDA Md Start failed! s32Ret: 0x%x.\n", s32Ret);
        goto END_0;
    }
    
    printf("Press any key to stop!");    
    getchar();


    /*************************************************
    step 9: unbind vpss and vo
    *************************************************/
    SAMPLE_COMM_VDA_MdStop(VdaChn_Md, u32VpssChn);

    /*************************************************
    step 10: stop thread and release all the resource
    *************************************************/
    bExit = HI_TRUE;
    pthread_join(stVdecThread, 0);
    bExit = HI_FALSE;

    /*************************************************
    step 11: unbind vpss and vo
    *************************************************/
    s32Ret = HI_MPI_SYS_UnBind(&stSrcChn, &stDesChn);
    if(HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_SYS_UnBind failed! s32Ret: 0x%x.\n", s32Ret);
        goto END_0;
    }
    
    /*************************************************
    step 12: unbind vdec and vpss
    *************************************************/
    stSrcChn.enModId  = HI_ID_VDEC;
    stSrcChn.s32DevId = 0;
    stSrcChn.s32ChnId = 0;

    stDesChn.enModId  = HI_ID_VPSS;
    stDesChn.s32DevId = 0;
    stDesChn.s32ChnId = 0;
    
    s32Ret = HI_MPI_SYS_UnBind(&stSrcChn, &stDesChn);
    if(HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_SYS_UnBind failed! s32Ret: 0x%x.\n", s32Ret);
        goto END_0;
    }

    /*************************************************
    step 13: stop vo dev and chn
    *************************************************/
    s32Ret = SAMPLE_VDA_StopVoChn(VoDev, s32VoChnNum);
    if(HI_SUCCESS != s32Ret)
    {
        printf("SAMPLE_RGN_StopVoChn failed! s32Ret: 0x%x.\n", s32Ret);
        goto END_0;
    }
    
    s32Ret = SAMPLE_VDA_StopVoDev(VoDev);
    if(HI_SUCCESS != s32Ret)
    {
        printf("SAMPLE_RGN_StopVoDev failed! s32Ret: 0x%x.\n", s32Ret);
        goto END_0;
    }
    
    /*************************************************
    step 14: stop vpss group and chn
    *************************************************/
    s32Ret = SAMPLE_VDA_StopVpss(s32VpssGrpNum);
    if(HI_SUCCESS != s32Ret)
    {
        printf("SAMPLE_RGN_StopVpss failed! s32Ret: 0x%x.\n", s32Ret);
        goto END_0;
    }
    
    /*************************************************
    step 15: stop vdec chn
    *************************************************/
    s32Ret = SAMPLE_VDA_StopVdec(VdecChn);
    if(HI_SUCCESS != s32Ret)
    {
        printf("SAMPLE_RGN_StopVdec failed! s32Ret: 0x%x.\n", s32Ret);
        goto END_0;
    }
    
END_0:
    SAMPLE_VDA_SYS_Exit();
    
    return s32Ret;
}


/******************************************************************************
* Description      : VDEC->VPSS->OD(480*270) 
******************************************************************************/
HI_S32 SAMPLE_VDA_OD(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    VDA_CHN VdaChn_Od = 1;
    PIC_SIZE_E enSize_Od = PIC_CIF; 	/* vda picture size */   
    VDEC_CHN VdecChn;
    HI_S32 s32VpssGrpNum;
    HI_U32 u32VpssChn;
    VO_DEV VoDev;
    VO_LAYER VoLayer;
    HI_U32 s32VoChnNum;
    VO_INTF_SYNC_E enIntfSync;
    pthread_t stVdecThread;
    VDEC_SENDPARAM_S stVdesSendPram;
    SIZE_S stSize;
    MPP_CHN_S stSrcChn, stDesChn;    
	
    /*************************************************
    step 1: mpp system init. 
    *************************************************/
    s32Ret = SAMPLE_VDA_SYS_Init();
    if (HI_SUCCESS != s32Ret)
    {
        printf("system init failed! s32Ret: 0x%x.\n", s32Ret);
        goto END_0;
    }

    /*************************************************
    step 2: start vdec chn
    *************************************************/
    VdecChn = 0;
    s32Ret = SAMPLE_VDA_StartVdec(VdecChn);
    if(HI_SUCCESS != s32Ret)
    {
        printf("SAMPLE_RGN_StartVdec failed! s32Ret: 0x%x.\n", s32Ret);
        goto END_0;
    }
    
    /*************************************************
    step 3: start vpss group and chn
    *************************************************/
    s32VpssGrpNum = 1;
    u32VpssChn    = VPSS_CHN2;
    s32Ret = SAMPLE_VDA_StartVpss(s32VpssGrpNum, u32VpssChn);
    if(HI_SUCCESS != s32Ret)
    {
        printf("SAMPLE_RGN_StartVpss failed! s32Ret: 0x%x.\n", s32Ret);
        goto END_0;
    }
    
    /*************************************************
    step 4: start vo dev and chn
    *************************************************/
    VoDev       = 1;
    s32VoChnNum = 1;
    enIntfSync  = VO_OUTPUT_PAL;
    s32Ret = SAMPLE_VDA_StartVo(VoDev, s32VoChnNum, enIntfSync);
    if(HI_SUCCESS != s32Ret)
    {
        printf("SAMPLE_RGN_StartVo failed! s32Ret: 0x%x.\n", s32Ret);
        goto END_0;
    }
    
    /*************************************************
    step 5: bind vdec and vpss
    *************************************************/
    stSrcChn.enModId  = HI_ID_VDEC;
    stSrcChn.s32DevId = 0;
    stSrcChn.s32ChnId = 0;

    stDesChn.enModId  = HI_ID_VPSS;
    stDesChn.s32DevId = 0;
    stDesChn.s32ChnId = 0;

    s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDesChn);
    if(HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_SYS_Bind failed! s32Ret: 0x%x.\n", s32Ret);
        goto END_0;
    }

    /*************************************************
    step 6: bind vpss and vo
    *************************************************/
    VoLayer = SAMPLE_VDA_GetVoLayer(VoDev);
    if(VoLayer < 0)
    {
        printf("SAMPLE_RGN_GetVoLayer failed! VoDev: %d.\n", VoDev);
        s32Ret = HI_FAILURE;
        goto END_0;
    }
    
    stSrcChn.enModId  = HI_ID_VPSS;
    stSrcChn.s32DevId = 0;
    stSrcChn.s32ChnId = u32VpssChn;

    stDesChn.enModId  = HI_ID_VOU;
    stDesChn.s32DevId = VoLayer;
    stDesChn.s32ChnId = 0;

    s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDesChn);
    if(HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_SYS_Bind failed! s32Ret: 0x%x.\n", s32Ret);
        goto END_0;
    }

    
    /*************************************************
        step 7: create a thread for vdec to read stream from a file
    *************************************************/    
    stSize.u32Width  = 720;
    stSize.u32Height = 576;
    
    stVdesSendPram.bRun          = HI_TRUE;
    stVdesSendPram.VdChn         = VdecChn;
    stVdesSendPram.enPayload     = PT_H264;
    stVdesSendPram.enVideoMode   = VIDEO_MODE_FRAME;
    stVdesSendPram.s32MinBufSize = stSize.u32Height * stSize.u32Width / 2;
    pthread_create(&stVdecThread, NULL, SAMPLE_VDA_VdecSendStream, (HI_VOID*)&stVdesSendPram);

    /*************************************************
    step  8: start VDA OD process
    *************************************************/
    s32Ret = SAMPLE_COMM_SYS_GetPicSize(gs_enNorm, enSize_Od, &stSize);
    if (HI_SUCCESS != s32Ret)
    {
        printf("SAMPLE_COMM_SYS_GetPicSize failed! s32Ret: 0x%x.\n", s32Ret);
        goto END_0;
    }

    if (0 != stSize.u32Width % VDA_WIDTH_ALIGN)
    {
        stSize.u32Width = (stSize.u32Width / VDA_WIDTH_ALIGN + 1) * VDA_WIDTH_ALIGN;
    }

    if (0 != stSize.u32Height % VDA_HEIGHT_ALIGN)
    {
        stSize.u32Height = (stSize.u32Height / VDA_HEIGHT_ALIGN + 1) * VDA_HEIGHT_ALIGN;
    }
    
    s32Ret = SAMPLE_COMM_VDA_OdStart(VdaChn_Od, u32VpssChn, &stSize);
    if (HI_SUCCESS != s32Ret)
    {
        printf("VDA OD Start failed! s32Ret: 0x%x.\n", s32Ret);
        goto END_0;
    }

    printf("Press any key to stop!");
    getchar();

    /*************************************************
    step  9: stop VDA OD process
    *************************************************/
    SAMPLE_COMM_VDA_OdStop(VdaChn_Od, u32VpssChn);
    
    /*************************************************
    step 10: stop thread and release all the resource
    *************************************************/
    bExit = HI_TRUE;
    pthread_join(stVdecThread, 0);
    bExit = HI_FALSE;
    
    /*************************************************
    step 11: unbind vpss and vo
    *************************************************/
    s32Ret = HI_MPI_SYS_UnBind(&stSrcChn, &stDesChn);
    if(HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_SYS_UnBind failed! s32Ret: 0x%x.\n", s32Ret);
        goto END_0;
    }
    
    /*************************************************
    step 12: unbind vdec and vpss
    *************************************************/
    stSrcChn.enModId  = HI_ID_VDEC;
    stSrcChn.s32DevId = 0;
    stSrcChn.s32ChnId = 0;

    stDesChn.enModId  = HI_ID_VPSS;
    stDesChn.s32DevId = 0;
    stDesChn.s32ChnId = 0;
    
    s32Ret = HI_MPI_SYS_UnBind(&stSrcChn, &stDesChn);
    if(HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_SYS_UnBind failed! s32Ret: 0x%x.\n", s32Ret);
        goto END_0;
    }

    /*************************************************
    step 13: stop vo dev and chn
    *************************************************/
    s32Ret = SAMPLE_VDA_StopVoChn(VoDev, s32VoChnNum);
    if(HI_SUCCESS != s32Ret)
    {
        printf("SAMPLE_RGN_StopVoChn failed! s32Ret: 0x%x.\n", s32Ret);
        goto END_0;
    }
    
    s32Ret = SAMPLE_VDA_StopVoDev(VoDev);
    if(HI_SUCCESS != s32Ret)
    {
        printf("SAMPLE_RGN_StopVoDev failed! s32Ret: 0x%x.\n", s32Ret);
        goto END_0;
    }
    
    /*************************************************
    step 14: stop vpss group and chn
    *************************************************/
    s32Ret = SAMPLE_VDA_StopVpss(s32VpssGrpNum);
    if(HI_SUCCESS != s32Ret)
    {
        printf("SAMPLE_RGN_StopVpss failed! s32Ret: 0x%x.\n", s32Ret);
        goto END_0;
    }
    
    /*************************************************
    step 15: stop vdec chn
    *************************************************/
    s32Ret = SAMPLE_VDA_StopVdec(VdecChn);
    if(HI_SUCCESS != s32Ret)
    {
        printf("SAMPLE_RGN_StopVdec failed! s32Ret: 0x%x.\n", s32Ret);
        goto END_0;
    }
    
END_0:
    SAMPLE_VDA_SYS_Exit();
    return s32Ret;
}


int main(int argc, char *argv[])
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR ch;
    HI_BOOL bExit = HI_FALSE;

    signal(SIGINT, SAMPLE_VDA_HandleSig);
    signal(SIGTERM, SAMPLE_VDA_HandleSig);

    while (1)
    {
        SAMPLE_VDA_Usage();
        ch = getchar();
        if(10 == ch)
        {
            continue;
        }
        getchar();
        switch (ch)
        {
            case '0':   /* MD */
            {
                s32Ret = SAMPLE_VDA_MD();
                break;
            }
            case '1':   /* OD */
            {
                s32Ret = SAMPLE_VDA_OD();
                break;
            }
            case 'q':
            case 'Q':
            {
                bExit = HI_TRUE;
                break;
            }
            default :
            {
                printf("input invaild! please try again.\n");
                break;
            }
        }

        if (bExit)
        {
            break;
        }
    }

    return s32Ret;
}



#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
