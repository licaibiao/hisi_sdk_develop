/************************************************************
*Copyright (C),lcb0281at163.com lcb0281atgmail.com
*BlogAddr: caibiao-lee.blog.csdn.net
*FileName: biao_vda.c
*Description:视频检测和分析模块
*Date:     2020-02-03
*Author:   Caibiao Lee
*Version:  V1.0
*Others:实现视频运动检测和视频遮挡检测
*History:
***********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>

#include "sample_comm.h"
#include "biao_vda.h"

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



void BIAO_VDA_HandleSig(HI_S32 signo)
{
    if (SIGINT == signo || SIGTSTP == signo)
    {
        SAMPLE_COMM_SYS_Exit();
        printf("\033[0;31mprogram exit abnormally!\033[0;39m\n");
    }

    exit(0);
}

HI_S32 BIAO_VDA_SYS_Init(PIC_SIZE_E enSize,VIDEO_NORM_E enNorm)
{
    HI_S32 s32Ret;
    HI_U32 u32BlkSize;
    VB_CONF_S struVbConf, stVbConf;
    MPP_SYS_CONF_S struSysConf;

    HI_MPI_SYS_Exit();
    HI_MPI_VB_Exit(); 

    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(enNorm, enSize, SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH, COMPRESS_MODE_SEG);

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

    if(PIC_D1==enSize)
    {
        stVbConf.u32MaxPoolCnt               = 3;
        stVbConf.astCommPool[0].u32BlkSize   = 720*576*2;
        stVbConf.astCommPool[0].u32BlkCnt    = 32;
        
        stVbConf.astCommPool[1].u32BlkSize   = 720*576/4;
        stVbConf.astCommPool[1].u32BlkCnt    = 32;
        
        stVbConf.astCommPool[2].u32BlkSize   = 1920*1088*2;
        stVbConf.astCommPool[2].u32BlkCnt    = 10;

    }else if(PIC_HD720==enSize)
    {
        stVbConf.u32MaxPoolCnt               = 3;
        stVbConf.astCommPool[0].u32BlkSize   = 1280*720*2;
        stVbConf.astCommPool[0].u32BlkCnt    = 32;

        stVbConf.astCommPool[1].u32BlkSize   = 1280*720/4;
        stVbConf.astCommPool[1].u32BlkCnt    = 32;

        stVbConf.astCommPool[2].u32BlkSize   = 1920*1088*2;
        stVbConf.astCommPool[2].u32BlkCnt    = 10; 
    }else
    {
        printf("%s %d enSize = %d error \n",__FUNCTION__,__LINE__,enSize);
        return -1;
    }

    
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


HI_S32 BIAO_VDA_SYS_Exit(HI_VOID)
{
    HI_MPI_VB_ExitModCommPool(VB_UID_VDEC);
    HI_MPI_SYS_Exit();
    HI_MPI_VB_Exit();

    return HI_SUCCESS;
}


/******************************************************** 
Function:    BIAO_VDA_StartVpss
Description: 开启VPSS
Input:  none
OutPut: none
Return: 0: success，none 0:error
Others: 
Author: Caibiao Lee
Date:   2020-02-02
*********************************************************/
HI_S32 BIAO_VDA_StartVpss(HI_S32 s32VpssGrpNum, HI_U32 u32VpssChn,PIC_SIZE_E enInputSize,PIC_SIZE_E enOutputSize)
{
    HI_S32 i = 0;
    HI_S32 s32Ret;
    HI_U32 u32Depth;
    VPSS_CHN_MODE_S stVpssChnMode;
    VPSS_GRP_ATTR_S stGrpAttr;
    HI_U32 u32OverlayMask;
    VPSS_CROP_INFO_S stCropInfo;

    if(PIC_HD720==enInputSize)
    {
        stGrpAttr.u32MaxW   = 1280;
        stGrpAttr.u32MaxH   = 720;
    }else
    {
        stGrpAttr.u32MaxW   = 720;
        stGrpAttr.u32MaxH   = 576;        
    }

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

        /**输入与输出不一致，开启裁减功能，将图像裁减成输出的分辨率**/
        if(enOutputSize!=enInputSize)
        {
            s32Ret = HI_MPI_VPSS_GetGrpCrop(i, &stCropInfo);
            if(s32Ret != HI_SUCCESS)
            {
                printf("Get Crop %d fail, s32Ret: 0x%x.\n", i, s32Ret);
                return s32Ret;
            }
            stCropInfo.bEnable = 1;
            stCropInfo.enCropCoordinate = VPSS_CROP_ABS_COOR;
            stCropInfo.stCropRect.s32X = 0;
            stCropInfo.stCropRect.s32Y = 0;

            if(PIC_D1==enOutputSize)
            {
                stCropInfo.stCropRect.u32Width = 720;
                stCropInfo.stCropRect.u32Height = 576;
            }else
            {
                stCropInfo.stCropRect.u32Width = 368;
                stCropInfo.stCropRect.u32Height = 288;         
            }

            s32Ret = HI_MPI_VPSS_SetGrpCrop(i, &stCropInfo);
            if(s32Ret != HI_SUCCESS)
            {
                printf("Set Crop %d fail, s32Ret: 0x%x.\n", i, s32Ret);
                return s32Ret;
            }

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
        
        stVpssChnMode.bDouble        = HI_FALSE;
        stVpssChnMode.enChnMode      = VPSS_CHN_MODE_USER;
        stVpssChnMode.enPixelFormat  = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
        if(PIC_D1==enOutputSize)
        {
            stVpssChnMode.u32Width       = 720;
            stVpssChnMode.u32Height      = 576;
        }else
        {
            stVpssChnMode.u32Width       = 1280;
            stVpssChnMode.u32Height      = 720;
        }

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

/******************************************************** 
Function:    BIAO_VDA_StopVpss
Description: 停止VPSS功能
Input:  none
OutPut: none
Return: 0: success，none 0:error
Others: 
Author: Caibiao Lee
Date:   2020-02-02
*********************************************************/
HI_S32 BIAO_VDA_StopVpss(HI_S32 s32VpssGrpNum)
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

    return HI_SUCCESS;
}


/******************************************************** 
Function:    BIAO_VDA_MD
Description: 开启运动检测
Input:  none
OutPut: none
Return: 0: success，none 0:error
Others: 
    绑定关系为：    VI->VPSS->vda
Author: Caibiao Lee
Date:   2020-02-02
*********************************************************/
HI_S32 BIAO_VDA_MD(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    VDA_CHN VdaChn_Md = 1;
    SIZE_S stSize;
    MPP_CHN_S stSrcChn, stDesChn;  
    VDEC_SENDPARAM_S stVdesSendPram;
    VPSS_GRP_ATTR_S stGrpAttr;
    PIC_SIZE_E enSize = PIC_HD720; 
    PIC_SIZE_E VpssSize = PIC_D1; 
    VIDEO_NORM_E enNorm = VIDEO_ENCODING_MODE_PAL;
    SAMPLE_VI_MODE_E enViMode = SAMPLE_VI_MODE_2_720P;
    HI_S32 s32VpssGrpNum;
    HI_U32 u32VpssChn;

    /*************************************************
    step 1: mpp system init. 
    *************************************************/
    s32Ret = BIAO_VDA_SYS_Init(enSize,enNorm);
    if (HI_SUCCESS != s32Ret)
    {
        printf("system init failed! s32Ret: 0x%x.\n", s32Ret);
        goto END_0;
    }

    /******************************************
     step 2: start vi dev & chn
    ******************************************/
    s32Ret = SAMPLE_COMM_VI_Start(enViMode, enNorm);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("start vi failed!\n");
        goto END_1;
    }

    /*************************************************
    step 3: start vpss group and chn
    *************************************************/
    s32VpssGrpNum = 1;
    u32VpssChn    = VPSS_CHN2;
    s32Ret = BIAO_VDA_StartVpss(s32VpssGrpNum, u32VpssChn,enSize,VpssSize);
    if(HI_SUCCESS != s32Ret)
    {
        printf("SAMPLE_RGN_StartVpss failed! s32Ret: 0x%x.\n", s32Ret);
        goto END_2;
    }

    /*************************************************
    step 4: bind vi and vpss
    *************************************************/
    stSrcChn.enModId  = HI_ID_VIU;
    stSrcChn.s32DevId = 0;
    stSrcChn.s32ChnId = 0;

    stDesChn.enModId  = HI_ID_VPSS;
    stDesChn.s32DevId = 0;
    stDesChn.s32ChnId = u32VpssChn;

    s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDesChn);
    if(HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_SYS_Bind failed! s32Ret: 0x%x.\n", s32Ret);
        goto END_3;
    }

    /*************************************************
    step  5: start VDA OD process
    *************************************************/
    s32Ret = SAMPLE_COMM_SYS_GetPicSize(enNorm, VpssSize, &stSize);
    if (HI_SUCCESS != s32Ret)
    {
        printf("SAMPLE_COMM_SYS_GetPicSize failed! s32Ret: 0x%x.\n", s32Ret);
        goto END_4;
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
        printf("VDA OD Start failed! s32Ret: 0x%x.\n", s32Ret);
        goto END_4;
    }

    printf("Press any key to stop!");
    getchar();

END_4:
    SAMPLE_COMM_VDA_MdStop(VdaChn_Md, u32VpssChn);

END_3:   
    SAMPLE_COMM_VI_UnBindVpss(enViMode);
    
END_2:
    BIAO_VDA_StopVpss(s32VpssGrpNum);
    
END_1:
    SAMPLE_COMM_VI_Stop(enViMode);  
END_0:
    BIAO_VDA_SYS_Exit();
    return s32Ret;

}


/******************************************************** 
Function:    BIAO_VDA_OD
Description: 开启遮挡检测
Input:  none
OutPut: none
Return: 0: success，none 0:error
Others: 
    绑定关系为：    VI->VPSS->vda
Author: Caibiao Lee
Date:   2020-02-02
*********************************************************/
HI_S32 BIAO_VDA_OD(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    VDA_CHN VdaChn_Od = 1;
    SIZE_S stSize;
    MPP_CHN_S stSrcChn, stDesChn;  
    VDEC_SENDPARAM_S stVdesSendPram;
    VPSS_GRP_ATTR_S stGrpAttr;
    PIC_SIZE_E enSize = PIC_HD720; 
    PIC_SIZE_E VpssSize = PIC_D1; 
    VIDEO_NORM_E enNorm = VIDEO_ENCODING_MODE_PAL;
    SAMPLE_VI_MODE_E enViMode = SAMPLE_VI_MODE_2_720P;
    HI_S32 s32VpssGrpNum;
    HI_U32 u32VpssChn;

    /*************************************************
    step 1: mpp system init. 
    *************************************************/
    s32Ret = BIAO_VDA_SYS_Init(enSize,enNorm);
    if (HI_SUCCESS != s32Ret)
    {
        printf("system init failed! s32Ret: 0x%x.\n", s32Ret);
        goto END_0;
    }

    /******************************************
     step 2: start vi dev & chn
    ******************************************/
    s32Ret = SAMPLE_COMM_VI_Start(enViMode, enNorm);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("start vi failed!\n");
        goto END_1;
    }

    /*************************************************
    step 3: start vpss group and chn
    *************************************************/
    s32VpssGrpNum = 1;
    u32VpssChn    = VPSS_CHN2;
    s32Ret = BIAO_VDA_StartVpss(s32VpssGrpNum, u32VpssChn,enSize,VpssSize);
    if(HI_SUCCESS != s32Ret)
    {
        printf("SAMPLE_RGN_StartVpss failed! s32Ret: 0x%x.\n", s32Ret);
        goto END_2;
    }

    /*************************************************
    step 4: bind vi and vpss
    *************************************************/
    stSrcChn.enModId  = HI_ID_VIU;
    stSrcChn.s32DevId = 0;
    stSrcChn.s32ChnId = 0;

    stDesChn.enModId  = HI_ID_VPSS;
    stDesChn.s32DevId = 0;
    stDesChn.s32ChnId = u32VpssChn;

    s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDesChn);
    if(HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_SYS_Bind failed! s32Ret: 0x%x.\n", s32Ret);
        goto END_3;
    }

    /*************************************************
    step  5: start VDA OD process
    *************************************************/
    s32Ret = SAMPLE_COMM_SYS_GetPicSize(enNorm, VpssSize, &stSize);
    if (HI_SUCCESS != s32Ret)
    {
        printf("SAMPLE_COMM_SYS_GetPicSize failed! s32Ret: 0x%x.\n", s32Ret);
        goto END_4;
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
        goto END_4;
    }

    printf("Press any key to stop!");
    getchar();

END_4:
    SAMPLE_COMM_VDA_OdStop(VdaChn_Od, u32VpssChn);

END_3:   
    SAMPLE_COMM_VI_UnBindVpss(enViMode);
    
END_2:
    BIAO_VDA_StopVpss(s32VpssGrpNum);
    
END_1:
    SAMPLE_COMM_VI_Stop(enViMode);  
END_0:
    BIAO_VDA_SYS_Exit();
    return s32Ret;
}


/******************************************************** 
Function:    BIAO_VDA_DEBUG
Description: 视频侦测与分析调试接口
Input:  none
OutPut: none
Return: 0: success，none 0:error
Others: 
Author: Caibiao Lee
Date:   2020-02-02
*********************************************************/
int BIAO_VDA_DEBUG()
{

    HI_S32 s32Ret = HI_SUCCESS;

    signal(SIGINT, BIAO_VDA_HandleSig);
    signal(SIGTERM, BIAO_VDA_HandleSig);

    s32Ret = BIAO_VDA_MD();
    //s32Ret = BIAO_VDA_OD();

    return s32Ret;
}


