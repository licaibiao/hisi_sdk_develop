/******************************************************************************
  Some simple Hisilicon HI3531 video input functions.

  Copyright (C), 2010-2011, Hisilicon Tech. Co., Ltd.
 ******************************************************************************
    Modification:  2011-2 Created
******************************************************************************/
#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <math.h>
#include <unistd.h>
#include <signal.h>

#include "sample_comm.h"
#include "hstChnConfig.h"

/******************************************************************************
* function : Set vpss system memory location
******************************************************************************/
HI_S32 SAMPLE_COMM_VPSS_MemConfig()
{
    HI_CHAR * pcMmzName;
    MPP_CHN_S stMppChnVpss;
    HI_S32 s32Ret, i;

    /*vpss group max is VPSS_MAX_GRP_NUM, not need config vpss chn.*/
    for(i=0;i<VPSS_MAX_GRP_NUM;i++)
    {
        stMppChnVpss.enModId  = HI_ID_VPSS;
        stMppChnVpss.s32DevId = i;
        stMppChnVpss.s32ChnId = 0;

        if(0 == (i%2))
        {
            pcMmzName = NULL;  
        }
        else
        {
            pcMmzName = "ddr1";
        }

        /*vpss*/
        s32Ret = HI_MPI_SYS_SetMemConf(&stMppChnVpss, pcMmzName);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Vpss HI_MPI_SYS_SetMemConf ERR !\n");
            return HI_FAILURE;
        }
    }
    return HI_SUCCESS;
}

/*****************************************************************************
* function : start vpss. VPSS chn with Audto Mode
*****************************************************************************/
HI_S32 SAMPLE_WISDOM_VPSS_AutoMode_Start(VPSS_GRP VpssGrp, SIZE_S *pstSize, VPSS_CHN VpssChn,VPSS_GRP_ATTR_S *pstVpssGrpAttr)
{
    HI_S32 s32Ret;

    VPSS_GRP_ATTR_S stGrpAttr = {0};
    VPSS_CHN_ATTR_S stChnAttr = {0};
    VPSS_CHN_MODE_S stVpssMode = {0};

    VPSS_CROP_INFO_S stVpssCropInfo;

    /*** Set Vpss Grp Attr ***/
    if(NULL == pstVpssGrpAttr)
    {
        stGrpAttr.u32MaxW = pstSize->u32Width;
        stGrpAttr.u32MaxH = pstSize->u32Height;
        stGrpAttr.enPixFmt = SAMPLE_PIXEL_FORMAT;
        stGrpAttr.enDieMode = VPSS_DIE_MODE_NODIE;
        stGrpAttr.bIeEn = HI_FALSE;
        stGrpAttr.bNrEn = HI_TRUE;
        stGrpAttr.bDciEn = HI_FALSE;
        stGrpAttr.bHistEn = HI_FALSE;
        stGrpAttr.bEsEn = HI_FALSE;
    }
    else
    {
        memcpy(&stGrpAttr,pstVpssGrpAttr,sizeof(VPSS_GRP_ATTR_S));
    }
   
    /*** create vpss group ***/
    s32Ret = HI_MPI_VPSS_CreateGrp(VpssGrp, &stGrpAttr);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("[VpssGrp:%d] HI_MPI_VPSS_CreateGrp failed with %#x!\n", VpssGrp, s32Ret);
        return HI_FAILURE;
    }
#if 0
	/*** enable vpss group clip ***/
    stVpssCropInfo.bEnable = HI_TRUE;
    stVpssCropInfo.enCropCoordinate = VPSS_CROP_ABS_COOR;
    stVpssCropInfo.stCropRect.s32X = 0;
    stVpssCropInfo.stCropRect.s32Y = 0;
    stVpssCropInfo.stCropRect.u32Height = 720;
    stVpssCropInfo.stCropRect.u32Width = 576;
    s32Ret = HI_MPI_VPSS_SetGrpCrop(VpssGrp, &stVpssCropInfo);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_VPSS_SetGrpCrop failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }
#endif
    s32Ret = HI_MPI_VPSS_EnableChn(VpssGrp, VpssChn);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("HI_MPI_VPSS_EnableChn failed with %#x\n", s32Ret);
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_VPSS_GetChnMode(VpssGrp, VpssChn, &stVpssMode);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("get vpss grp%d chn%d mode fail, s32Ret: 0x%x.\n", VpssGrp, VpssChn, s32Ret);
        return s32Ret;
    }

    stVpssMode.bDouble 	     = HI_FALSE;
    stVpssMode.enChnMode 	 = VPSS_CHN_MODE_AUTO;
    stVpssMode.enPixelFormat = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
    stVpssMode.u32Width 	 = 720;//pstSize->u32Width;
    stVpssMode.u32Height 	 = 576;//pstSize->u32Height;
    stVpssMode.enCompressMode = COMPRESS_MODE_NONE;

    SAMPLE_PRT("SetVpssChnMode [grp:%d] [chn:%d] [u32Width:%d] [u32Height:%d].\n",
        VpssGrp, VpssChn, stVpssMode.u32Width, stVpssMode.u32Height);

    s32Ret = HI_MPI_VPSS_SetChnMode(VpssGrp, VpssChn, &stVpssMode);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("HI_MPI_VPSS_SetChnMode failed with %#x\n", s32Ret);
        return HI_FAILURE;
    }
    
    /*** start vpss group ***/
    s32Ret = HI_MPI_VPSS_StartGrp(VpssGrp);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("HI_MPI_VPSS_StartGrp failed with %#x\n", s32Ret);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

/*****************************************************************************
* function : start vpss. VPSS chn with frame
*****************************************************************************/

HI_S32 SAMPLE_COMM_VPSS_StartByGroup(unsigned char u8ViGroup,HI_S32 s32GrpCnt, SIZE_S *pstSize, HI_S32 s32ChnCnt,VPSS_GRP_ATTR_S *pstVpssGrpAttr)
{
    VPSS_GRP VpssGrp;
    VPSS_CHN VpssChn;
    VPSS_GRP_ATTR_S stGrpAttr = {0};
    VPSS_CHN_ATTR_S stChnAttr = {0};
    VPSS_GRP_PARAM_S stVpssParam = {0};
    HI_S32 s32Ret;
    HI_S32 i, j;
    VPSS_CHN_MODE_S stVpssMode = {0};
	HI_U32 u32Depth = 3;
    /*** Set Vpss Grp Attr ***/

    if(NULL == pstVpssGrpAttr)
    {
    	stGrpAttr.enDieMode = VPSS_DIE_MODE_NODIE;
    	stGrpAttr.bIeEn     = HI_FALSE;
    	stGrpAttr.bDciEn    = HI_TRUE;
    	stGrpAttr.bNrEn     = HI_TRUE;
		stGrpAttr.bEsEn     = HI_FALSE;
    	stGrpAttr.bHistEn   = HI_FALSE;
    	stGrpAttr.enPixFmt  = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
    	stGrpAttr.u32MaxW   = ALIGN_UP(pstSize->u32Width,  16);
    	stGrpAttr.u32MaxH   = ALIGN_UP(pstSize->u32Height, 16);
    }
    else
    {
        memcpy(&stGrpAttr,pstVpssGrpAttr,sizeof(VPSS_GRP_ATTR_S));
    }
    

    for(i=0; i<s32GrpCnt; i++)
    {
        //if(HstSdkAL_CheckDevType(DEV_VERSION_BT10))
        //    VpssGrp = i+u8ViGroup*2;
        //else if(HstSdkAL_CheckDevType(DEV_VERSION_BT20))
            VpssGrp = i+((0 == u8ViGroup)?0:4);
       // else
		//    SAMPLE_PRT("设备类型配置!\n");
        /*** create vpss group ***/
		
		printf("[%s %d] HI_MPI_VPSS_CreateGrp %d \n", __func__,__LINE__,VpssGrp);
        s32Ret = HI_MPI_VPSS_CreateGrp(VpssGrp, &stGrpAttr);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("[VpssGrp:%d] HI_MPI_VPSS_CreateGrp failed with %#x!\n", VpssGrp, s32Ret);
            return HI_FAILURE;
        }

        /*** set vpss param ***/
        s32Ret = HI_MPI_VPSS_GetGrpParam(VpssGrp, &stVpssParam);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("failed with %#x!\n", s32Ret);
            return HI_FAILURE;
        }
        
        stVpssParam.u32IeStrength = 0;
        s32Ret = HI_MPI_VPSS_SetGrpParam(VpssGrp, &stVpssParam);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("failed with %#x!\n", s32Ret);
            return HI_FAILURE;
        }

        /*** enable vpss chn, with frame ***/
		s32ChnCnt=1;
        for(j=0; j<s32ChnCnt; j++)
        {
            VpssChn = VPSS_CHN0;
            /* Set Vpss Chn attr */
            stChnAttr.bSpEn = HI_FALSE;
            stChnAttr.bUVInvert = HI_FALSE;
            stChnAttr.bBorderEn = HI_TRUE;
            stChnAttr.stBorder.u32Color = 0xff00;
            stChnAttr.stBorder.u32LeftWidth = 2;
            stChnAttr.stBorder.u32RightWidth = 2;
            stChnAttr.stBorder.u32TopWidth = 2;
            stChnAttr.stBorder.u32BottomWidth = 2;
            
            s32Ret = HI_MPI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stChnAttr);
            if (s32Ret != HI_SUCCESS)
            {
                SAMPLE_PRT("HI_MPI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
                return HI_FAILURE;
            }
#if 1
			stVpssMode.enChnMode = VPSS_CHN_MODE_USER;
			stVpssMode.u32Width  = pstSize->u32Width;
			stVpssMode.u32Height = pstSize->u32Height;
			
			stVpssMode.stFrameRate.s32DstFrmRate = -1;
			stVpssMode.stFrameRate.s32SrcFrmRate = -1;
			
			stVpssMode.enPixelFormat  = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
			stVpssMode.enCompressMode = COMPRESS_MODE_NONE;

			
			s32Ret = HI_MPI_VPSS_SetChnMode(VpssGrp, VpssChn, &stVpssMode);
			if (s32Ret != HI_SUCCESS)
			{
				SAMPLE_PRT("HI_MPI_VPSS_SetChnMode failed with %#x\n", s32Ret);
				return HI_FAILURE;
			}
			s32Ret = HI_MPI_VPSS_SetDepth(VpssGrp, VpssChn, u32Depth);
			if (HI_SUCCESS != s32Ret)
			{
				SAMPLE_PRT("HI_MPI_VPSS_SetDepth failed with errno: %#x\n", s32Ret);
				return HI_FAILURE;
			}
#endif
			printf("[%s %d] HI_MPI_VPSS_EnableChn VpssGrp %d VpssChn %d\n", __func__,__LINE__,VpssGrp,VpssChn);
            s32Ret = HI_MPI_VPSS_EnableChn(VpssGrp, VpssChn);
            if (s32Ret != HI_SUCCESS)
            {
                SAMPLE_PRT("HI_MPI_VPSS_EnableChn failed with %#x\n", s32Ret);
                return HI_FAILURE;
            }
        }
        
        /*** start vpss group ***/
        s32Ret = HI_MPI_VPSS_StartGrp(VpssGrp);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("HI_MPI_VPSS_StartGrp failed with %#x\n", s32Ret);
            return HI_FAILURE;
        }

    }
    return HI_SUCCESS;
}

/*****************************************************************************
* function : start vpss. VPSS chn with frame
*****************************************************************************/

HI_S32 SAMPLE_COMM_VPSS_Start(HI_S32 s32GrpCnt, SIZE_S *pstSize, HI_S32 s32ChnCnt,VPSS_GRP_ATTR_S *pstVpssGrpAttr)
{
    VPSS_GRP VpssGrp;
    VPSS_CHN VpssChn;
    VPSS_GRP_ATTR_S stGrpAttr = {0};
    VPSS_CHN_ATTR_S stChnAttr = {0};
    VPSS_GRP_PARAM_S stVpssParam = {0};
    HI_S32 s32Ret;
    HI_S32 i, j;
    VPSS_CHN_MODE_S stVpssMode = {0};

    /*** Set Vpss Grp Attr ***/

    if(NULL == pstVpssGrpAttr)
    {
        stGrpAttr.u32MaxW = pstSize->u32Width;
        stGrpAttr.u32MaxH = pstSize->u32Height;
        stGrpAttr.enPixFmt = SAMPLE_PIXEL_FORMAT;
        
        stGrpAttr.bIeEn = HI_FALSE;
        stGrpAttr.bNrEn = HI_TRUE;
        stGrpAttr.bDciEn = HI_FALSE;
        stGrpAttr.bHistEn = HI_FALSE;
        stGrpAttr.bEsEn = HI_FALSE;
        stGrpAttr.enDieMode = VPSS_DIE_MODE_NODIE;
    }
    else
    {
        memcpy(&stGrpAttr,pstVpssGrpAttr,sizeof(VPSS_GRP_ATTR_S));
    }
    

    for(i=0; i<s32GrpCnt; i++)
    {
        VpssGrp = i;
        /*** create vpss group ***/
        s32Ret = HI_MPI_VPSS_CreateGrp(VpssGrp, &stGrpAttr);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("[VpssGrp:%d] HI_MPI_VPSS_CreateGrp failed with %#x!\n", VpssGrp, s32Ret);
            return HI_FAILURE;
        }

        /*** set vpss param ***/
        s32Ret = HI_MPI_VPSS_GetGrpParam(VpssGrp, &stVpssParam);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("failed with %#x!\n", s32Ret);
            return HI_FAILURE;
        }
        
        stVpssParam.u32IeStrength = 0;
        s32Ret = HI_MPI_VPSS_SetGrpParam(VpssGrp, &stVpssParam);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("failed with %#x!\n", s32Ret);
            return HI_FAILURE;
        }

        /*** enable vpss chn, with frame ***/
        for(j=0; j<s32ChnCnt; j++)
        {
            VpssChn = j;
            /* Set Vpss Chn attr */
            stChnAttr.bSpEn = HI_FALSE;
            stChnAttr.bUVInvert = HI_FALSE;
            stChnAttr.bBorderEn = HI_TRUE;
            stChnAttr.stBorder.u32Color = 0xff00;
            stChnAttr.stBorder.u32LeftWidth = 2;
            stChnAttr.stBorder.u32RightWidth = 2;
            stChnAttr.stBorder.u32TopWidth = 2;
            stChnAttr.stBorder.u32BottomWidth = 2;
            
            s32Ret = HI_MPI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stChnAttr);
            if (s32Ret != HI_SUCCESS)
            {
                SAMPLE_PRT("HI_MPI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
                return HI_FAILURE;
            }
            /************************************************///modify by fjw 
            stVpssMode.enChnMode = VPSS_CHN_MODE_USER;
            stVpssMode.u32Width  = 720/3;
            stVpssMode.u32Height = 576/3;
            stVpssMode.enPixelFormat = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
            s32Ret = HI_MPI_VPSS_SetChnMode(VpssGrp, VPSS_CHN3, &stVpssMode);
            if (s32Ret != HI_SUCCESS)
            {
                SAMPLE_PRT("HI_MPI_VPSS_SetChnMode failed with %#x\n", s32Ret);
                return HI_FAILURE;
            }
            /************************************************///modified end
            s32Ret = HI_MPI_VPSS_EnableChn(VpssGrp, VpssChn);
            if (s32Ret != HI_SUCCESS)
            {
                SAMPLE_PRT("HI_MPI_VPSS_EnableChn failed with %#x\n", s32Ret);
                return HI_FAILURE;
            }
        }
        
        /*** start vpss group ***/
        s32Ret = HI_MPI_VPSS_StartGrp(VpssGrp);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("HI_MPI_VPSS_StartGrp failed with %#x\n", s32Ret);
            return HI_FAILURE;
        }

    }
    return HI_SUCCESS;
}


/*****************************************************************************
* function : disable vi dev
*****************************************************************************/

HI_S32 SAMPLE_WISDOM_VPSS_Stop(VPSS_GRP VpssGrp, VPSS_CHN VpssChn)
{
    HI_S32 s32Ret = HI_SUCCESS;
    //VPSS_CHN VpssChn;
    int i = 0;

    s32Ret = HI_MPI_VPSS_StopGrp(VpssGrp);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }

    for(i = 0; i < 4; i++)//调试阶段先这样用，后面不能直接摧毁整个VPSS组会影响编码。
    {
        s32Ret = HI_MPI_VPSS_DisableChn(VpssGrp, i);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("failed with %#x!\n", s32Ret);
            return HI_FAILURE;
        }
    }

    s32Ret = HI_MPI_VPSS_DestroyGrp(VpssGrp);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}


HI_S32 SAMPLE_COMM_VPSS_Stop(HI_S32 s32GrpCnt, HI_S32 s32ChnCnt)
{
    HI_S32 i, j;
    HI_S32 s32Ret = HI_SUCCESS;
    VPSS_GRP VpssGrp;
    VPSS_CHN VpssChn;

    for(i=0; i<s32GrpCnt; i++)
    {
        VpssGrp = i;
        s32Ret = HI_MPI_VPSS_StopGrp(VpssGrp);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("failed with %#x!\n", s32Ret);
            return HI_FAILURE;
        }
        for(j=0; j<s32ChnCnt; j++)
        {
            VpssChn = j;
            s32Ret = HI_MPI_VPSS_DisableChn(VpssGrp, VpssChn);
            if (s32Ret != HI_SUCCESS)
            {
                SAMPLE_PRT("failed with %#x!\n", s32Ret);
                return HI_FAILURE;
            }
        }
    
        s32Ret = HI_MPI_VPSS_DestroyGrp(VpssGrp);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("failed with %#x!\n", s32Ret);
            return HI_FAILURE;
        }
    }

    return HI_SUCCESS;
}
HI_S32 SAMPLE_COMM_DisableVpssPreScale(VPSS_GRP VpssGrp,SIZE_S stSize)
{
    HI_S32 s32Ret;
    VPSS_PRESCALE_INFO_S stPreScaleInfo;
        
    stPreScaleInfo.bPreScale = HI_FALSE;
    stPreScaleInfo.stDestSize.u32Width = stSize.u32Width;
    stPreScaleInfo.stDestSize.u32Height = stSize.u32Height;
    s32Ret = HI_MPI_VPSS_SetPreScale(VpssGrp, &stPreScaleInfo);
    if (s32Ret != HI_SUCCESS)
	{
	    SAMPLE_PRT("HI_MPI_VPSS_SetPreScale failed with %#x!\n", s32Ret);
	    return HI_FAILURE;
	}

    return s32Ret;
}
HI_S32 SAMPLE_COMM_EnableVpssPreScale(VPSS_GRP VpssGrp,SIZE_S stSize)
{
    HI_S32 s32Ret;
    VPSS_PRESCALE_INFO_S stPreScaleInfo;
        
    stPreScaleInfo.bPreScale = HI_TRUE;
    stPreScaleInfo.stDestSize.u32Width = stSize.u32Width;
    stPreScaleInfo.stDestSize.u32Height = stSize.u32Height;
    s32Ret = HI_MPI_VPSS_SetPreScale(VpssGrp, &stPreScaleInfo);
    if (s32Ret != HI_SUCCESS)
	{
	    SAMPLE_PRT("HI_MPI_VPSS_SetPreScale failed with %#x!\n", s32Ret);
	    return HI_FAILURE;
	}

    return s32Ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
