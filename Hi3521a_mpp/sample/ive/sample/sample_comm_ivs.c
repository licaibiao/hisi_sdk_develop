/******************************************************************************
  Some simple Hisilicon Hi3536 ivs functions.

  Copyright (C), 2010-2021, Hisilicon Tech. Co., Ltd.
 ******************************************************************************
    Modification:  2014-12 Created
******************************************************************************/
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

#include "sample_comm_ivs.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* Begin of #ifdef __cplusplus */

/************************** sample ivs ********************************/


/*****************************************************************************
* function : set sys conf. 
*****************************************************************************/
HI_VOID	SAMPLE_COMM_IVS_SysConf(VB_CONF_S *pstVbConf, SIZE_S *pstSize, HI_U32 u32Cnt)
{	
	HI_U32 i, u32Align, u32AlignW, u32AlignH;
	u32Align = SAMPLE_SYS_ALIGN_WIDTH;
	
    memset(pstVbConf, 0, sizeof(VB_CONF_S));
    pstVbConf->u32MaxPoolCnt = 1;	

	for(i=0; i<u32Cnt; i++)
	{
		u32AlignW = ALIGN_UP(pstSize[i].u32Width, u32Align);
		u32AlignH = ALIGN_UP(pstSize[i].u32Height, u32Align);
		pstVbConf->astCommPool[i].u32BlkSize = (u32AlignW * u32AlignH * 3) >> 1;
    	pstVbConf->astCommPool[i].u32BlkCnt	 = 20;
	}
}

/*****************************************************************************
* function : set vpss group attribute. 
*****************************************************************************/
HI_VOID	SAMPLE_COMM_IVS_VpssGrpAttr(HI_U32 u32VpssGrpCnt, VPSS_GRP_ATTR_S *pstVpssGrpAttr, SIZE_S *pstSize)
{
	HI_U32 i;

	memset(pstVpssGrpAttr, 0, sizeof(VPSS_GRP_ATTR_S));
	
	for(i=0; i<u32VpssGrpCnt; i++)
	{
    	pstVpssGrpAttr->enDieMode = VPSS_DIE_MODE_NODIE;
    	pstVpssGrpAttr->bIeEn     = HI_FALSE;
    	pstVpssGrpAttr->bDciEn    = HI_TRUE;
    	pstVpssGrpAttr->bNrEn     = HI_TRUE;
		pstVpssGrpAttr->bEsEn     = HI_FALSE;
    	pstVpssGrpAttr->bHistEn   = HI_FALSE;
    	pstVpssGrpAttr->enPixFmt  = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
    	pstVpssGrpAttr->u32MaxW   = ALIGN_UP(pstSize->u32Width,  16);
    	pstVpssGrpAttr->u32MaxH   = ALIGN_UP(pstSize->u32Height, 16);
	}
}

/*****************************************************************************
* function : start vpss. VPSS chn with frame
*****************************************************************************/
HI_S32 SAMPLE_COMM_IVS_VpssStart(HI_S32 s32VpssGrpCnt, SIZE_S astSize[],
VPSS_CHN aVpssChn[], HI_S32 s32VpssChnCnt, VPSS_GRP_ATTR_S *pstVpssGrpAttr)
{
    VPSS_GRP VpssGrp;
    VPSS_CHN VpssChn;
    VPSS_GRP_ATTR_S stGrpAttr = {0};
    //VPSS_CHN_ATTR_S stChnAttr = {0};
    VPSS_GRP_PARAM_S stVpssParam = {0};
	VPSS_CHN_MODE_S  stVpssChnMode = {0};
	VPSS_FRAME_RATE_S stVpssFrmRate = {30, 30};
		
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i, j;
	HI_U32 u32Depth = 3;

	HI_ASSERT(s32VpssGrpCnt>0);
	HI_ASSERT(s32VpssChnCnt>0);
	
    /*** Set Vpss Grp Attr ***/
    if(NULL == pstVpssGrpAttr)
    {
        stGrpAttr.u32MaxW = astSize[0].u32Width;
        stGrpAttr.u32MaxH = astSize[0].u32Height;
        stGrpAttr.bIeEn = HI_FALSE;
        stGrpAttr.bNrEn = HI_TRUE;
        stGrpAttr.bHistEn = HI_FALSE;
        stGrpAttr.enDieMode = VPSS_DIE_MODE_NODIE;
        stGrpAttr.enPixFmt = SAMPLE_PIXEL_FORMAT;
    }
    else
    {
        memcpy(&stGrpAttr,pstVpssGrpAttr,sizeof(VPSS_GRP_ATTR_S));
    }
    

    for(i=0; i<s32VpssGrpCnt; i++)
    {
        VpssGrp = i;
        /*** create vpss group ***/
        s32Ret = HI_MPI_VPSS_CreateGrp(VpssGrp, &stGrpAttr);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("HI_MPI_VPSS_CreateGrp failed with errno: %#x!\n", s32Ret);
            return HI_FAILURE;
        }

        /*** set vpss param ***/
        s32Ret = HI_MPI_VPSS_GetGrpParam(VpssGrp, &stVpssParam);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("HI_MPI_VPSS_GetGrpParam failed with errno: %#x!\n", s32Ret);
            return HI_FAILURE;
        }
        
        stVpssParam.u32IeStrength = 0;
        s32Ret = HI_MPI_VPSS_SetGrpParam(VpssGrp, &stVpssParam);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("HI_MPI_VPSS_SetGrpParam failed with errno: %#x!\n", s32Ret);
            return HI_FAILURE;
        }
		
		s32Ret = HI_MPI_VPSS_SetGrpFrameRate(VpssGrp, &stVpssFrmRate);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("HI_MPI_VPSS_SetGrpFrameRate failed with errno: %#x!\n", s32Ret);
            return HI_FAILURE;
        }

        /*** enable vpss chn, with frame ***/
        for(j=0; j<s32VpssChnCnt; j++)
        {
            VpssChn = aVpssChn[j];

			stVpssChnMode.enChnMode = VPSS_CHN_MODE_USER;
			stVpssChnMode.u32Width  = astSize[j].u32Width;
			stVpssChnMode.u32Height = astSize[j].u32Height;
			stVpssChnMode.enPixelFormat = SAMPLE_PIXEL_FORMAT;
			stVpssChnMode.enCompressMode = COMPRESS_MODE_NONE;
			    
            s32Ret = HI_MPI_VPSS_EnableChn(VpssGrp, VpssChn);
			if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_PRT("HI_MPI_VPSS_EnableChn failed with errno: %#x\n", s32Ret);
                return HI_FAILURE;
            }

			s32Ret = HI_MPI_VPSS_SetChnMode(VpssGrp, VpssChn, &stVpssChnMode);
			if (HI_SUCCESS != s32Ret)
			{
				SAMPLE_PRT("HI_MPI_VPSS_SetChnMode failed with errno: %#x\n", s32Ret);
				return HI_FAILURE;
			}			

			s32Ret = HI_MPI_VPSS_SetDepth(VpssGrp, VpssChn, u32Depth);
			if (HI_SUCCESS != s32Ret)
			{
				SAMPLE_PRT("HI_MPI_VPSS_SetDepth failed with errno: %#x\n", s32Ret);
				return HI_FAILURE;
			}
			
        }
        
        /*** start vpss group ***/
        s32Ret = HI_MPI_VPSS_StartGrp(VpssGrp);
        if (HI_SUCCESS != s32Ret)
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
HI_S32 SAMPLE_COMM_IVS_VpssStart2(HI_S32 s32VpssGrpCnt, SIZE_S astSize[],
VPSS_CHN aVpssChn[], HI_S32 s32VpssChnCnt, VPSS_GRP_ATTR_S *pstVpssGrpAttr)
{
    VPSS_GRP VpssGrp;
    VPSS_CHN VpssChn;
    VPSS_GRP_ATTR_S stGrpAttr = {0};
    //VPSS_CHN_ATTR_S stChnAttr = {0};
    VPSS_GRP_PARAM_S stVpssParam = {0};
	VPSS_CHN_MODE_S  stVpssChnMode = {0};
	VPSS_FRAME_RATE_S stVpssFrmRate = {30, 30};
		
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i, j;
	HI_U32 u32Depth = 3;

	HI_ASSERT(s32VpssGrpCnt>0);
	HI_ASSERT(s32VpssChnCnt==2);
	
    /*** Set Vpss Grp Attr ***/
    if(NULL == pstVpssGrpAttr)
    {
        stGrpAttr.u32MaxW = astSize[0].u32Width;
        stGrpAttr.u32MaxH = astSize[0].u32Height;
        stGrpAttr.bIeEn = HI_FALSE;
        stGrpAttr.bNrEn = HI_TRUE;
        stGrpAttr.bHistEn = HI_FALSE;
        stGrpAttr.enDieMode = VPSS_DIE_MODE_NODIE;
        stGrpAttr.enPixFmt = SAMPLE_PIXEL_FORMAT;
    }
    else
    {
        memcpy(&stGrpAttr,pstVpssGrpAttr,sizeof(VPSS_GRP_ATTR_S));
    }    

    for(i=0; i<s32VpssGrpCnt; i++)
    {
        VpssGrp = i;
        /*** create vpss group ***/
        s32Ret = HI_MPI_VPSS_CreateGrp(VpssGrp, &stGrpAttr);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("HI_MPI_VPSS_CreateGrp failed with errno: %#x!\n", s32Ret);
            return HI_FAILURE;
        }

        /*** set vpss param ***/
        s32Ret = HI_MPI_VPSS_GetGrpParam(VpssGrp, &stVpssParam);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("HI_MPI_VPSS_GetGrpParam failed with errno: %#x!\n", s32Ret);
            return HI_FAILURE;
        }
        
        stVpssParam.u32IeStrength = 0;
        s32Ret = HI_MPI_VPSS_SetGrpParam(VpssGrp, &stVpssParam);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("HI_MPI_VPSS_SetGrpParam failed with errno: %#x!\n", s32Ret);
            return HI_FAILURE;
        }
		
		s32Ret = HI_MPI_VPSS_SetGrpFrameRate(VpssGrp, &stVpssFrmRate);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("HI_MPI_VPSS_SetGrpFrameRate failed with errno: %#x!\n", s32Ret);
            return HI_FAILURE;
        }

        /*** enable vpss chn, with frame ***/
        for(j=0; j<s32VpssChnCnt; j++)
        {
            VpssChn = aVpssChn[j];

			stVpssChnMode.enChnMode = (0==j ? VPSS_CHN_MODE_AUTO : VPSS_CHN_MODE_USER);
			stVpssChnMode.u32Width  = astSize[j].u32Width;
			stVpssChnMode.u32Height = astSize[j].u32Height;
			stVpssChnMode.enPixelFormat = SAMPLE_PIXEL_FORMAT;
			stVpssChnMode.enCompressMode = COMPRESS_MODE_NONE;
			stVpssChnMode.stFrameRate.s32SrcFrmRate = stVpssFrmRate.s32SrcFrmRate;
			stVpssChnMode.stFrameRate.s32DstFrmRate = (0==j ? stVpssFrmRate.s32DstFrmRate : 10);
			    
            s32Ret = HI_MPI_VPSS_EnableChn(VpssGrp, VpssChn);
			if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_PRT("HI_MPI_VPSS_EnableChn failed with errno: %#x\n", s32Ret);
                return HI_FAILURE;
            }

			s32Ret = HI_MPI_VPSS_SetChnMode(VpssGrp, VpssChn, &stVpssChnMode);
			if (HI_SUCCESS != s32Ret)
			{
				SAMPLE_PRT("HI_MPI_VPSS_SetChnMode failed with errno: %#x\n", s32Ret);
				return HI_FAILURE;
			}			

			s32Ret = HI_MPI_VPSS_SetDepth(VpssGrp, VpssChn, u32Depth);
			if (HI_SUCCESS != s32Ret)
			{
				SAMPLE_PRT("HI_MPI_VPSS_SetDepth failed with errno: %#x\n", s32Ret);
				return HI_FAILURE;
			}
			
        }
        
        /*** start vpss group ***/
        s32Ret = HI_MPI_VPSS_StartGrp(VpssGrp);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("HI_MPI_VPSS_StartGrp failed with %#x\n", s32Ret);
            return HI_FAILURE;
        }

    }
    return HI_SUCCESS;
}


/*****************************************************************************
* function : stop vpss
*****************************************************************************/
HI_S32 SAMPLE_COMM_IVS_VpssStop(HI_S32 s32VpssGrpCnt, HI_S32 s32VpssChnCnt)
{
    HI_S32 i, j;
    HI_S32 s32Ret = HI_SUCCESS;
    VPSS_GRP VpssGrp;
    VPSS_CHN VpssChn;

    for(i=0; i<s32VpssGrpCnt; i++)
    {
        VpssGrp = i;
        s32Ret = HI_MPI_VPSS_StopGrp(VpssGrp);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("HI_MPI_VPSS_StopGrp failed with errno: %#x!\n", s32Ret);
            return HI_FAILURE;
        }
        for(j=0; j<s32VpssChnCnt; j++)
        {
            VpssChn = j;
            s32Ret = HI_MPI_VPSS_DisableChn(VpssGrp, VpssChn);
            if (s32Ret != HI_SUCCESS)
            {
                SAMPLE_PRT("HI_MPI_VPSS_DisableChn failed with errno: %#x!\n", s32Ret);
                return HI_FAILURE;
            }
        }
    
        s32Ret = HI_MPI_VPSS_DestroyGrp(VpssGrp);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("HI_MPI_VPSS_DestroyGrp failed with errno: %#x!\n", s32Ret);
            return HI_FAILURE;
        }
    }

    return HI_SUCCESS;
}


HI_S32 SAMPLE_COMM_IVS_VgsFrmObj(VIDEO_FRAME_INFO_S *pstFrmInfo, VGS_COVER_S *pstVgsCover, HI_U32 u32ObjNum)
{
	HI_S32 s32Ret = HI_SUCCESS;
	
    VGS_HANDLE VgsHandle;
    HI_U16 i, n, r;
    VGS_TASK_ATTR_S stVgsTask;

    if(0 == u32ObjNum)
    {
        return HI_SUCCESS;
    }
	
    s32Ret = HI_MPI_VGS_BeginJob(&VgsHandle);
    if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("Vgs begin job fail,Error(%#x)\n",s32Ret);
        return s32Ret;
    }

    memcpy(&stVgsTask.stImgIn,pstFrmInfo,sizeof(VIDEO_FRAME_INFO_S));
    memcpy(&stVgsTask.stImgOut,pstFrmInfo,sizeof(VIDEO_FRAME_INFO_S));

	n = u32ObjNum/SAMPLE_MAX_VGS_COVER;
	r = u32ObjNum%SAMPLE_MAX_VGS_COVER;
    for(i = 0;i < n; i++)
    {
		s32Ret = HI_MPI_VGS_AddCoverTask(VgsHandle, &stVgsTask, pstVgsCover, SAMPLE_MAX_VGS_COVER);
        if(s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("HI_MPI_VGS_AddCoverTask fail,Error(%#x)\n",s32Ret);
            HI_MPI_VGS_CancelJob(VgsHandle);
            return s32Ret;
        }        
    }
	
	s32Ret = HI_MPI_VGS_AddCoverTask(VgsHandle, &stVgsTask, &pstVgsCover[n*SAMPLE_MAX_VGS_COVER], r);
	if(s32Ret != HI_SUCCESS)
	{
		SAMPLE_PRT("HI_MPI_VGS_AddCoverTask fail,Error(%#x)\n",s32Ret);
		HI_MPI_VGS_CancelJob(VgsHandle);
		return s32Ret;
	}	   

    s32Ret = HI_MPI_VGS_EndJob(VgsHandle);
    if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("HI_MPI_VGS_EndJob fail,Error(%#x)\n",s32Ret);
        HI_MPI_VGS_CancelJob(VgsHandle);
        return s32Ret;
    }
    return s32Ret;	
}


/************************** sample ive ********************************/

static HI_BOOL bMpiInit = HI_FALSE;

HI_U16 SAMPLE_COMM_IVE_CalcStride(HI_U16 u16Width, HI_U8 u8Align)
{
	return (u16Width + (u8Align - u16Width%u8Align)%u8Align);
}

static HI_S32 SAMPLE_IVE_MPI_Init(HI_VOID)
{
    HI_S32 s32Ret;
    VB_CONF_S struVbConf;
    MPP_SYS_CONF_S struSysConf;

    HI_MPI_SYS_Exit();
    HI_MPI_VB_Exit();

    memset(&struVbConf,0,sizeof(VB_CONF_S));
    struVbConf.u32MaxPoolCnt             = 128;
    struVbConf.astCommPool[0].u32BlkSize = 1920*1080*2;
    struVbConf.astCommPool[0].u32BlkCnt  = 40;
    s32Ret = HI_MPI_VB_SetConf(&struVbConf);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_VB_SetConf fail,Error(%#x)\n",s32Ret);
        return s32Ret;
    }
    s32Ret = HI_MPI_VB_Init();
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_VB_Init fail,Error(%#x)\n",s32Ret);
        return s32Ret;
    }
    struSysConf.u32AlignWidth = 64;
    s32Ret = HI_MPI_SYS_SetConf(&struSysConf);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_SYS_SetConf fail,Error(%#x)\n",s32Ret);
        (HI_VOID)HI_MPI_VB_Exit();
        return s32Ret;
    }
    
    s32Ret = HI_MPI_SYS_Init();
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_SYS_Init fail,Error(%#x)\n",s32Ret);
        (HI_VOID)HI_MPI_VB_Exit();
        return s32Ret;
    }    

    return HI_SUCCESS;
}


HI_VOID SAMPLE_COMM_IVE_CheckIveMpiInit(HI_VOID)
{
   if(HI_FALSE == bMpiInit)
    {
        if (SAMPLE_IVE_MPI_Init())
        {
           SAMPLE_PRT("Ive mpi init failed!\n");            
            exit(-1);
        }
        bMpiInit = HI_TRUE;
    }
}
HI_S32 SAMPLE_COMM_IVE_IveMpiExit(HI_VOID)
{
	bMpiInit = HI_FALSE;
	if (HI_MPI_SYS_Exit())
	{
		SAMPLE_PRT("Sys exit failed!\n");  
		return HI_FAILURE;
	}

	if (HI_MPI_VB_Exit())
	{
        SAMPLE_PRT("Vb exit failed!\n");        
        return HI_FAILURE;
    }
    
	return HI_SUCCESS;
}

HI_S32 SAMPLE_COMM_VGS_AddDrawRectJob(VGS_HANDLE VgsHandle, IVE_IMAGE_S *pstSrc, IVE_IMAGE_S *pstDst, 
                    RECT_S *pstRect, HI_U16 u16RectNum)
{
    HI_S32 s32Ret = HI_SUCCESS;
    VGS_TASK_ATTR_S stVgsTask;
    VGS_COVER_S stVgsCover;
    HI_U16 i;
	
    memset(&stVgsTask, 0, sizeof(VGS_TASK_ATTR_S));
 
    stVgsTask.stImgIn.stVFrame.enPixelFormat = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
    stVgsTask.stImgIn.stVFrame.enCompressMode = COMPRESS_MODE_NONE;
    stVgsTask.stImgIn.stVFrame.enVideoFormat = VIDEO_FORMAT_LINEAR;
    stVgsTask.stImgIn.stVFrame.u32Field = VIDEO_FIELD_FRAME;
    stVgsTask.stImgIn.stVFrame.u32Width = pstSrc->u16Width;
    stVgsTask.stImgIn.stVFrame.u32Height = pstSrc->u16Height;
	
    stVgsTask.stImgIn.stVFrame.u32PhyAddr[0] = pstSrc->u32PhyAddr[0]; 
    stVgsTask.stImgIn.stVFrame.u32PhyAddr[1] = pstSrc->u32PhyAddr[1];
    stVgsTask.stImgIn.stVFrame.pVirAddr[0] = pstSrc->pu8VirAddr[0];
    stVgsTask.stImgIn.stVFrame.pVirAddr[1] = pstSrc->pu8VirAddr[1];
    
    stVgsTask.stImgIn.stVFrame.u32Stride[0] = pstSrc->u16Stride[0];
    stVgsTask.stImgIn.stVFrame.u32Stride[1] = pstSrc->u16Stride[1];
    stVgsTask.stImgIn.stVFrame.u64pts = 12;
    stVgsTask.stImgIn.stVFrame.u32TimeRef = 24;

    stVgsTask.stImgOut.stVFrame.enPixelFormat = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
    stVgsTask.stImgOut.stVFrame.enCompressMode = COMPRESS_MODE_NONE;
    stVgsTask.stImgOut.stVFrame.enVideoFormat = VIDEO_FORMAT_LINEAR;
    stVgsTask.stImgOut.stVFrame.u32Field = VIDEO_FIELD_FRAME;
    stVgsTask.stImgOut.stVFrame.u32Width = pstDst->u16Width;
    stVgsTask.stImgOut.stVFrame.u32Height = pstDst->u16Height;
	
    stVgsTask.stImgOut.stVFrame.u32PhyAddr[0] = pstDst->u32PhyAddr[0];
    stVgsTask.stImgOut.stVFrame.u32PhyAddr[1] = pstDst->u32PhyAddr[1];
    stVgsTask.stImgOut.stVFrame.pVirAddr[0] = pstDst->pu8VirAddr[0];
    stVgsTask.stImgOut.stVFrame.pVirAddr[1] = pstDst->pu8VirAddr[1];
    
    stVgsTask.stImgOut.stVFrame.u32Stride[0] = pstDst->u16Stride[0];
    stVgsTask.stImgOut.stVFrame.u32Stride[1] = pstDst->u16Stride[1];
    stVgsTask.stImgOut.stVFrame.u64pts = 12;
    stVgsTask.stImgOut.stVFrame.u32TimeRef = 24;

#if 1
    stVgsCover.enCoverType = COVER_RECT;
#else
	stVgsCover.enCoverType = COVER_QUAD_RANGLE;
#endif
    for(i = 0; i < u16RectNum;i++)
    {
	#if 1
        stVgsCover.stDstRect.s32X = (pstRect[i].s32X / 2) * 2;
        stVgsCover.stDstRect.s32Y = (pstRect[i].s32Y / 2) * 2;
        stVgsCover.stDstRect.u32Width = (pstRect[i].u32Width / 2) * 2;
        stVgsCover.stDstRect.u32Height = (pstRect[i].u32Height / 2) * 2;
		
        stVgsCover.u32Color = 0x0000FF00;
        if(stVgsCover.stDstRect.s32X + stVgsCover.stDstRect.u32Width < stVgsTask.stImgOut.stVFrame.u32Width 
            && stVgsCover.stDstRect.s32Y + stVgsCover.stDstRect.u32Height < stVgsTask.stImgOut.stVFrame.u32Height)
        {		
	#else
		stVgsCover.stQuadRangle.bSolid = HI_FALSE;
		stVgsCover.stQuadRangle.u32Thick = 2;
		stVgsCover.stQuadRangle.stPoint[0].s32X = pstRect[i].s32X 		& (~1);
		stVgsCover.stQuadRangle.stPoint[0].s32Y = pstRect[i].s32Y 		& (~1);
		stVgsCover.stQuadRangle.stPoint[1].s32X = (pstRect[i].s32X + 4) & (~1);
		stVgsCover.stQuadRangle.stPoint[1].s32Y = pstRect[i].s32Y 		& (~1);
		stVgsCover.stQuadRangle.stPoint[2].s32X = (pstRect[i].s32X + 4) & (~1);
		stVgsCover.stQuadRangle.stPoint[2].s32Y = (pstRect[i].s32Y + 4)	& (~1);
		stVgsCover.stQuadRangle.stPoint[3].s32X = pstRect[i].s32X 		& (~1);
		stVgsCover.stQuadRangle.stPoint[3].s32Y = (pstRect[i].s32Y + 4) & (~1);
		
		if(stVgsCover.stQuadRangle.stPoint[1].s32X < stVgsTask.stImgOut.stVFrame.u32Width
		&& stVgsCover.stQuadRangle.stPoint[2].s32Y < stVgsTask.stImgOut.stVFrame.u32Height)
		{
	#endif
            s32Ret = HI_MPI_VGS_AddCoverTask(VgsHandle,&stVgsTask,&stVgsCover,1);
            if(s32Ret != HI_SUCCESS)
            {
                SAMPLE_PRT("HI_MPI_VGS_AddCoverTask fail,Error(%#x)\n",s32Ret);
                HI_MPI_VGS_CancelJob(VgsHandle);
                return s32Ret;
            }    
        }
        
    }
   
    return s32Ret;
}

HI_S32 SAMPLE_COMM_VGS_FillRect(VIDEO_FRAME_INFO_S *pstFrmInfo, SAMPLE_RECT_ARRAY_S *pstRect, HI_U32 u32Color)
{
    VGS_HANDLE VgsHandle;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U16 i;
    VGS_TASK_ATTR_S stVgsTask;
    VGS_COVER_S stVgsAddCover;

    if(0 == pstRect->u16Num)
    {
        return s32Ret;
    }
    s32Ret = HI_MPI_VGS_BeginJob(&VgsHandle);
    if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("Vgs begin job fail,Error(%#x)\n",s32Ret);
        return s32Ret;
    }

    memcpy(&stVgsTask.stImgIn,pstFrmInfo,sizeof(VIDEO_FRAME_INFO_S));
    memcpy(&stVgsTask.stImgOut,pstFrmInfo,sizeof(VIDEO_FRAME_INFO_S));

    stVgsAddCover.enCoverType = COVER_QUAD_RANGLE;
    stVgsAddCover.u32Color = u32Color;
    for(i = 0;i < pstRect->u16Num;i++)
    {
        stVgsAddCover.stQuadRangle.bSolid = HI_FALSE;
        stVgsAddCover.stQuadRangle.u32Thick = 2;
        memcpy(stVgsAddCover.stQuadRangle.stPoint,pstRect->astRect[i].astPoint,sizeof(pstRect->astRect[i].astPoint));
        s32Ret = HI_MPI_VGS_AddCoverTask(VgsHandle, &stVgsTask, &stVgsAddCover, 1);
        if(s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("HI_MPI_VGS_AddCoverTask fail,Error(%#x)\n",s32Ret);
            HI_MPI_VGS_CancelJob(VgsHandle);
            return s32Ret;
        }
        
    }

    s32Ret = HI_MPI_VGS_EndJob(VgsHandle);
    if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("HI_MPI_VGS_EndJob fail,Error(%#x)\n",s32Ret);
        HI_MPI_VGS_CancelJob(VgsHandle);
        return s32Ret;
    }
    return s32Ret;
    
}

HI_S32 SAMPLE_COMM_IVE_ReadFile(IVE_IMAGE_S *pstImg, FILE *pFp)
{
	HI_U16 y;
	HI_U8 *pU8;
	HI_U16 height;
	HI_U16 width;
    HI_U16 loop;
	
	if (feof(pFp)) 
	{
		SAMPLE_PRT("end of file!\n");
		fseek(pFp, 0 , SEEK_SET);
	}

	height = pstImg->u16Height;
	width = pstImg->u16Width;

	switch(pstImg->enType)
	{
	case  IVE_IMAGE_TYPE_U8C1:
		{
			pU8 = pstImg->pu8VirAddr[0];
			for (y = 0; y < height; y++)
			{
				if ( 1 != fread(pU8,width,1,pFp))
				{
					SAMPLE_PRT("Read file fail\n");
					return HI_FAILURE;
				}

				pU8 += pstImg->u16Stride[0];
			}
		}
		break;
	case  IVE_IMAGE_TYPE_YUV420SP:
		{
			pU8 = pstImg->pu8VirAddr[0];
			for (y = 0; y < height; y++)
			{
				if ( 1 != fread(pU8,width,1,pFp))
				{
					SAMPLE_PRT("Read file fail\n");
					return HI_FAILURE;
				}

				pU8 += pstImg->u16Stride[0];
			}

			pU8 = pstImg->pu8VirAddr[1];
			for (y = 0; y < height/2; y++)
			{
				if ( 1 != fread(pU8,width,1,pFp))
				{
					SAMPLE_PRT("Read file fail\n");
					return HI_FAILURE;
				}

				pU8 += pstImg->u16Stride[1];
			}
		}
		break;
	case IVE_IMAGE_TYPE_YUV422SP:
		{
			pU8 = pstImg->pu8VirAddr[0];
			for (y = 0; y < height; y++)
			{
				if ( 1 != fread(pU8,width,1,pFp))
				{
					SAMPLE_PRT("Read file fail\n");
					return HI_FAILURE;
				}

				pU8 += pstImg->u16Stride[0];
			}

			pU8 = pstImg->pu8VirAddr[1];
			for (y = 0; y < height; y++)
			{
				if ( 1 != fread(pU8,width,1,pFp))
				{
					SAMPLE_PRT("Read file fail\n");
					return HI_FAILURE;
				}

				pU8 += pstImg->u16Stride[1];
			}
		}
		break;
	case IVE_IMAGE_TYPE_U8C3_PACKAGE:
		{
			pU8 = pstImg->pu8VirAddr[0];
			for (y = 0; y < height; y++)
			{
				if ( 1 != fread(pU8,width * 3,1,pFp))
				{
					SAMPLE_PRT("Read file fail\n");
					return HI_FAILURE;
				}

				pU8 += pstImg->u16Stride[0] * 3;
			}
			
		}
		break;
	case IVE_IMAGE_TYPE_U8C3_PLANAR:
		{
			for (loop = 0; loop < 3; loop++)
			{
				pU8 = pstImg->pu8VirAddr[loop];
				for (y = 0; y < height; y++)
				{
					if ( 1 != fread(pU8,width,1,pFp))
					{
						SAMPLE_PRT("Read file fail\n");
						return HI_FAILURE;
					}

					pU8 += pstImg->u16Stride[loop];
				}
			}
			
		}
		break;
	case IVE_IMAGE_TYPE_S16C1:
	case IVE_IMAGE_TYPE_U16C1:
		{
			pU8 = pstImg->pu8VirAddr[0];
			for( y = 0; y < height; y++ ) 
			{ 
				if( sizeof(HI_U16) != fread(pU8, width, sizeof(HI_U16), pFp) )
				{
					SAMPLE_PRT("Read file fail\n");
					return HI_FAILURE;
				}

				pU8 += pstImg->u16Stride[0] * 2;
			}
		}
		break;
	default:
		break;
	}

	return HI_SUCCESS;
}

HI_S32 SAMPLE_COMM_IVE_WriteFile(IVE_IMAGE_S *pstImg, FILE *pFp)
{
	HI_U16 y;
	HI_U8 *pU8;
	HI_U16 height;
	HI_U16 width;

	height = pstImg->u16Height;
	width = pstImg->u16Width;

	switch(pstImg->enType)
	{
	case  IVE_IMAGE_TYPE_U8C1:
    case  IVE_IMAGE_TYPE_S8C1:
		{
			pU8 = pstImg->pu8VirAddr[0];
			for (y = 0; y < height; y++)
			{
				if ( 1 != fwrite(pU8,width,1,pFp))
				{
					SAMPLE_PRT("Write file fail\n");
					return HI_FAILURE;
				}

				pU8 += pstImg->u16Stride[0];
			}
		}
		break;
	case  IVE_IMAGE_TYPE_YUV420SP:
		{
			pU8 = pstImg->pu8VirAddr[0];
			for (y = 0; y < height; y++)
			{
				if ( width != fwrite(pU8,1,width,pFp))
				{
					SAMPLE_PRT("Write file fail\n");
					return HI_FAILURE;
				}

				pU8 += pstImg->u16Stride[0];
			}

			pU8 = pstImg->pu8VirAddr[1];
			for (y = 0; y < height/2; y++)
			{
				if ( width != fwrite(pU8,1,width,pFp))
				{
					SAMPLE_PRT("Write file fail\n");
					return HI_FAILURE;
				}

				pU8 += pstImg->u16Stride[1];
			}
		}
		break;
	case IVE_IMAGE_TYPE_YUV422SP:
		{
			pU8 = pstImg->pu8VirAddr[0];
			for (y = 0; y < height; y++)
			{
				if ( width != fwrite(pU8,1,width,pFp))
				{
					SAMPLE_PRT("Write file fail\n");
					return HI_FAILURE;
				}

				pU8 += pstImg->u16Stride[0];
			}
           
			pU8 = pstImg->pu8VirAddr[1];
			for (y = 0; y < height; y++)
			{
				if ( width != fwrite(pU8,1,width,pFp))
				{
					SAMPLE_PRT("Write file fail\n");
					return HI_FAILURE;
				}

				pU8 += pstImg->u16Stride[1];
			}
		}
		break;
    #if 0
	case IVE_IMAGE_TYPE_U8C3_PACKAGE:
		{
			pU8 = image->pu8VirAddr[0];
			for (y = 0; y < height; y++)
			{
				if ( 1 != fread(pU8,width * 3,1,fp))
				{
					printf("read file error,line:%d\n",__LINE__);
					return HI_FAILURE;
				}

				pU8 += image->u16Stride[0] * 3;
			}
			
		}
		break;
	case IVE_IMAGE_TYPE_U8C3_PLANAR:
		{
			for (HI_U16 loop = 0; loop < 3; loop++)
			{
				pU8 = image->pu8VirAddr[loop];
				for (y = 0; y < height; y++)
				{
					if ( 1 != fread(pU8,width,1,fp))
					{
						printf("read file error,line:%d\n",__LINE__);
						return HI_FAILURE;
					}

					pU8 += image->u16Stride[loop];
				}
			}
			
		}
		break;
    #endif
    case IVE_IMAGE_TYPE_S16C1:
	case  IVE_IMAGE_TYPE_U16C1:
		{
			pU8 = pstImg->pu8VirAddr[0];
			for( y = 0; y < height; y++ ) 
			{ 
				if( sizeof(HI_U16) != fwrite(pU8, width, sizeof(HI_U16), pFp) )
				{
					SAMPLE_PRT("Write file fail\n");
					return HI_FAILURE;
				}

				pU8 += pstImg->u16Stride[0] * 2;
			}
		}
		break;
    case IVE_IMAGE_TYPE_U32C1:
    {
       
        pU8 = pstImg->pu8VirAddr[0];
        for( y = 0; y < height; y++ ) 
        { 
        	if( width != fwrite(pU8, sizeof(HI_U32), width, pFp) )
        	{
        		SAMPLE_PRT("Write file fail\n");
        		return HI_FAILURE;
        	}

        	pU8 += pstImg->u16Stride[0] * 4;
        }
       break; 
    }
    
	default:
		break;
	}

	return HI_SUCCESS;
}

HI_VOID SAMPLE_COMM_IVE_BlobToRect(IVE_CCBLOB_S *pstBlob, SAMPLE_RECT_ARRAY_S *pstRect,
                                            HI_U16 u16RectMaxNum,HI_U16 u16AreaThrStep,
                                            HI_FLOAT f32SclX, HI_FLOAT f32SclY)
{
    HI_U16 u16Num;
    HI_U16 i,j,k;
    HI_U16 u16Thr= 0;
	HI_BOOL bValid;
	
    if(pstBlob->u8RegionNum > u16RectMaxNum)
    {
     
		u16Thr = pstBlob->u16CurAreaThr;
		do
		{
			u16Num = 0;
			u16Thr += u16AreaThrStep; 
			for(i = 0;i < 254;i++)
			{
				if(pstBlob->astRegion[i].u32Area > u16Thr)
				{
					u16Num++;
				}
			}	  
		}while(u16Num > u16RectMaxNum);  

    }    

   u16Num = 0;
   
   for(i = 0;i < 254;i++)
    {
        if(pstBlob->astRegion[i].u32Area > u16Thr)
        {

            pstRect->astRect[u16Num].astPoint[0].s32X = (HI_S32)(pstBlob->astRegion[i].u16Left * f32SclX) & (~1) ;
			pstRect->astRect[u16Num].astPoint[0].s32Y = (HI_S32)(pstBlob->astRegion[i].u16Top * f32SclY) & (~1);

			pstRect->astRect[u16Num].astPoint[1].s32X = (HI_S32)(pstBlob->astRegion[i].u16Right * f32SclX) & (~1);
			pstRect->astRect[u16Num].astPoint[1].s32Y = (HI_S32)(pstBlob->astRegion[i].u16Top * f32SclY) & (~1);

			pstRect->astRect[u16Num].astPoint[2].s32X = (HI_S32)(pstBlob->astRegion[i].u16Right * f32SclX) & (~1);
			pstRect->astRect[u16Num].astPoint[2].s32Y = (HI_S32)(pstBlob->astRegion[i].u16Bottom * f32SclY) & (~1);

			pstRect->astRect[u16Num].astPoint[3].s32X = (HI_S32)(pstBlob->astRegion[i].u16Left * f32SclX) & (~1);
			pstRect->astRect[u16Num].astPoint[3].s32Y = (HI_S32)(pstBlob->astRegion[i].u16Bottom * f32SclY) & (~1);

			bValid = HI_TRUE;
			for(j = 0; j < 3;j++)
			{
			  for (k = j + 1; k < 4;k++)
			  {
				  if ((pstRect->astRect[u16Num].astPoint[j].s32X == pstRect->astRect[u16Num].astPoint[k].s32X)
				  	 &&(pstRect->astRect[u16Num].astPoint[j].s32Y == pstRect->astRect[u16Num].astPoint[k].s32Y))
				  	{
				  	    bValid = HI_FALSE;
						break;
				     }

			  }
			}
			if (HI_TRUE == bValid)
			{
				u16Num++;
			}
        }
    }

	pstRect->u16Num = u16Num;
}

/******************************************************************************
* function : Create ive image
******************************************************************************/
HI_S32 SAMPLE_COMM_IVE_CreateImage(IVE_IMAGE_S *pstImg,IVE_IMAGE_TYPE_E enType,HI_U16 u16Width,HI_U16 u16Height)
{
	HI_U32 u32Size = 0;
	HI_S32 s32Ret;
	if (NULL == pstImg)
	{
		SAMPLE_PRT("pstImg is null\n");
		return HI_FAILURE;
	}

	pstImg->enType = enType;
	pstImg->u16Width = u16Width;
	pstImg->u16Height = u16Height;
	pstImg->u16Stride[0] = SAMPLE_COMM_IVE_CalcStride(pstImg->u16Width,IVE_ALIGN);

	switch(enType)
	{
	case IVE_IMAGE_TYPE_U8C1:
	case IVE_IMAGE_TYPE_S8C1:
		{
			u32Size = pstImg->u16Stride[0] * pstImg->u16Height;
			s32Ret = HI_MPI_SYS_MmzAlloc(&pstImg->u32PhyAddr[0], (void**)&pstImg->pu8VirAddr[0], NULL, HI_NULL, u32Size);
			if(s32Ret != HI_SUCCESS)
			{
				SAMPLE_PRT("Mmz Alloc fail,Error(%#x)\n",s32Ret);
				return s32Ret;
			}
		}
		break;
	case IVE_IMAGE_TYPE_YUV420SP:
		{
			u32Size = pstImg->u16Stride[0] * pstImg->u16Height * 3 / 2;
			s32Ret = HI_MPI_SYS_MmzAlloc(&pstImg->u32PhyAddr[0], (void**)&pstImg->pu8VirAddr[0], NULL, HI_NULL, u32Size);
			if(s32Ret != HI_SUCCESS)
			{
				SAMPLE_PRT("Mmz Alloc fail,Error(%#x)\n",s32Ret);
				return s32Ret;
			}
			pstImg->u16Stride[1] = pstImg->u16Stride[0];
			pstImg->u32PhyAddr[1] = pstImg->u32PhyAddr[0] + pstImg->u16Stride[0] * pstImg->u16Height;
			pstImg->pu8VirAddr[1] = pstImg->pu8VirAddr[0] + pstImg->u16Stride[0] * pstImg->u16Height;

		}
		break;
	case IVE_IMAGE_TYPE_YUV422SP:
		{
			u32Size = pstImg->u16Stride[0] * pstImg->u16Height * 2;
			s32Ret = HI_MPI_SYS_MmzAlloc(&pstImg->u32PhyAddr[0], (void**)&pstImg->pu8VirAddr[0], NULL, HI_NULL, u32Size);
			if(s32Ret != HI_SUCCESS)
			{
				SAMPLE_PRT("Mmz Alloc fail,Error(%#x)\n",s32Ret);
				return s32Ret;
			}
			pstImg->u16Stride[1] = pstImg->u16Stride[0];
			pstImg->u32PhyAddr[1] = pstImg->u32PhyAddr[0] + pstImg->u16Stride[0] * pstImg->u16Height;
			pstImg->pu8VirAddr[1] = pstImg->pu8VirAddr[0] + pstImg->u16Stride[0] * pstImg->u16Height;

		}
		break;
	case IVE_IMAGE_TYPE_YUV420P:
		break;
	case IVE_IMAGE_TYPE_YUV422P:
		break;
	case IVE_IMAGE_TYPE_S8C2_PACKAGE:
		break;
	case IVE_IMAGE_TYPE_S8C2_PLANAR:
		break;
	case IVE_IMAGE_TYPE_S16C1:
	case IVE_IMAGE_TYPE_U16C1:
		{
			
			u32Size = pstImg->u16Stride[0] * pstImg->u16Height * sizeof(HI_U16);
			s32Ret = HI_MPI_SYS_MmzAlloc(&pstImg->u32PhyAddr[0], (void**)&pstImg->pu8VirAddr[0], NULL, HI_NULL, u32Size);
			if(s32Ret != HI_SUCCESS)
			{
				SAMPLE_PRT("Mmz Alloc fail,Error(%#x)\n",s32Ret);
				return s32Ret;
			}
		}
		break;
	case IVE_IMAGE_TYPE_U8C3_PACKAGE:
		{
			u32Size = pstImg->u16Stride[0] * pstImg->u16Height * 3;
			s32Ret = HI_MPI_SYS_MmzAlloc(&pstImg->u32PhyAddr[0], (void**)&pstImg->pu8VirAddr[0], NULL, HI_NULL, u32Size);
			if(s32Ret != HI_SUCCESS)
			{
				SAMPLE_PRT("Mmz Alloc fail,Error(%#x)\n",s32Ret);
				return s32Ret;
			}
			pstImg->pu8VirAddr[1] = pstImg->pu8VirAddr[0] +1;
			pstImg->pu8VirAddr[2] = pstImg->pu8VirAddr[1] + 1;			
			pstImg->u32PhyAddr[1] = pstImg->u32PhyAddr[0] + 1;
			pstImg->u32PhyAddr[2] = pstImg->u32PhyAddr[1] + 1;
			pstImg->u16Stride[1] = pstImg->u16Stride[0];
			pstImg->u16Stride[2] = pstImg->u16Stride[0];
		}
		break;
	case IVE_IMAGE_TYPE_U8C3_PLANAR:
		break;
	case IVE_IMAGE_TYPE_S32C1:
	case IVE_IMAGE_TYPE_U32C1:
		{
			u32Size = pstImg->u16Stride[0] * pstImg->u16Height * sizeof(HI_U32);
			s32Ret = HI_MPI_SYS_MmzAlloc(&pstImg->u32PhyAddr[0], (void**)&pstImg->pu8VirAddr[0], NULL, HI_NULL, u32Size);
			if(s32Ret != HI_SUCCESS)
			{
				SAMPLE_PRT("Mmz Alloc fail,Error(%#x)\n",s32Ret);
				return s32Ret;
			}
		}
		break;
	case IVE_IMAGE_TYPE_S64C1:
	case IVE_IMAGE_TYPE_U64C1:
		{
			
			u32Size = pstImg->u16Stride[0] * pstImg->u16Height * sizeof(HI_U64);
			s32Ret = HI_MPI_SYS_MmzAlloc(&pstImg->u32PhyAddr[0], (void**)&pstImg->pu8VirAddr[0], NULL, HI_NULL, u32Size);
			if(s32Ret != HI_SUCCESS)
			{
				SAMPLE_PRT("Mmz Alloc fail,Error(%#x)\n",s32Ret);
				return s32Ret;
			}
		}
		break;
	default:
		break;
			
	}

	return HI_SUCCESS;
}
/******************************************************************************
* function : Create memory info
******************************************************************************/
HI_S32 SAMPLE_COMM_IVE_CreateMemInfo(IVE_MEM_INFO_S*pstMemInfo,HI_U32 u32Size)
{
	HI_S32 s32Ret;

	if (NULL == pstMemInfo)
	{
		SAMPLE_PRT("pstMemInfo is null\n");
		return HI_FAILURE;
	}
	pstMemInfo->u32Size = u32Size;
	s32Ret = HI_MPI_SYS_MmzAlloc(&pstMemInfo->u32PhyAddr, (void**)&pstMemInfo->pu8VirAddr, NULL, HI_NULL, u32Size);
	if(s32Ret != HI_SUCCESS)
	{
		SAMPLE_PRT("Mmz Alloc fail,Error(%#x)\n",s32Ret);
		return HI_FAILURE;
	}

	return HI_SUCCESS;
}
/******************************************************************************
* function : Create ive image by cached
******************************************************************************/
HI_S32 SAMPLE_COMM_IVE_CreateImageByCached(IVE_IMAGE_S *pstImg,
	IVE_IMAGE_TYPE_E enType,HI_U16 u16Width,HI_U16 u16Height)
{
	HI_U32 u32Size = 0;
	HI_S32 s32Ret;
	if (NULL == pstImg)
	{
		SAMPLE_PRT("pstImg is null\n");
		return HI_FAILURE;
	}

	pstImg->enType = enType;
	pstImg->u16Width = u16Width;
	pstImg->u16Height = u16Height;
	pstImg->u16Stride[0] = SAMPLE_COMM_IVE_CalcStride(pstImg->u16Width,IVE_ALIGN);

	switch(enType)
	{
	case IVE_IMAGE_TYPE_U8C1:
	case IVE_IMAGE_TYPE_S8C1:
		{
			u32Size = pstImg->u16Stride[0] * pstImg->u16Height;
			s32Ret = HI_MPI_SYS_MmzAlloc_Cached(&pstImg->u32PhyAddr[0], (void**)&pstImg->pu8VirAddr[0], NULL, HI_NULL, u32Size);
			if(s32Ret != HI_SUCCESS)
			{
				SAMPLE_PRT("Mmz Alloc fail,Error(%#x)\n",s32Ret);
				return s32Ret;
			}
		}
		break;
	case IVE_IMAGE_TYPE_YUV420SP:
		break;
	case IVE_IMAGE_TYPE_YUV422SP:
		break;
	case IVE_IMAGE_TYPE_YUV420P:
		break;
	case IVE_IMAGE_TYPE_YUV422P:
		break;
	case IVE_IMAGE_TYPE_S8C2_PACKAGE:
		break;
	case IVE_IMAGE_TYPE_S8C2_PLANAR:
		break;
	case IVE_IMAGE_TYPE_S16C1:
	case IVE_IMAGE_TYPE_U16C1:
		{
			
			u32Size = pstImg->u16Stride[0] * pstImg->u16Height * sizeof(HI_U16);
			s32Ret = HI_MPI_SYS_MmzAlloc_Cached(&pstImg->u32PhyAddr[0], (void**)&pstImg->pu8VirAddr[0], NULL, HI_NULL, u32Size);
			if(s32Ret != HI_SUCCESS)
			{
				SAMPLE_PRT("Mmz Alloc fail,Error(%#x)\n",s32Ret);
				return s32Ret;
			}
		}
		break;
	case IVE_IMAGE_TYPE_U8C3_PACKAGE:
		break;
	case IVE_IMAGE_TYPE_U8C3_PLANAR:
		break;
	case IVE_IMAGE_TYPE_S32C1:
	case IVE_IMAGE_TYPE_U32C1:
		{
			u32Size = pstImg->u16Stride[0] * pstImg->u16Height * sizeof(HI_U32);
			s32Ret = HI_MPI_SYS_MmzAlloc_Cached(&pstImg->u32PhyAddr[0], (void**)&pstImg->pu8VirAddr[0], NULL, HI_NULL, u32Size);
			if(s32Ret != HI_SUCCESS)
			{
				SAMPLE_PRT("Mmz Alloc fail,Error(%#x)\n",s32Ret);
				return s32Ret;
			}
		}
		break;
	case IVE_IMAGE_TYPE_S64C1:
	case IVE_IMAGE_TYPE_U64C1:
		{
			
			u32Size = pstImg->u16Stride[0] * pstImg->u16Height * sizeof(HI_U64);
			s32Ret = HI_MPI_SYS_MmzAlloc_Cached(&pstImg->u32PhyAddr[0], (void**)&pstImg->pu8VirAddr[0], NULL, HI_NULL, u32Size);
			if(s32Ret != HI_SUCCESS)
			{
				SAMPLE_PRT("Mmz Alloc fail,Error(%#x)\n",s32Ret);
				return s32Ret;
			}
		}
		break;
	default:
		break;
			
	}

	return HI_SUCCESS;

}

/******************************************************************************
* function : Dma frame info to  ive image
******************************************************************************/
HI_S32 SAMPLE_COMM_IVE_DmaImage(VIDEO_FRAME_INFO_S *pstFrameInfo,IVE_DST_IMAGE_S *pstDst,HI_BOOL bInstant)
{
	HI_S32 s32Ret;
	IVE_HANDLE hIveHandle;
	IVE_SRC_DATA_S stSrcData;
	IVE_DST_DATA_S stDstData;
	IVE_DMA_CTRL_S stCtrl = {IVE_DMA_MODE_DIRECT_COPY,0};
	HI_BOOL bFinish = HI_FALSE;
	HI_BOOL bBlock = HI_TRUE;

	//fill src
	stSrcData.pu8VirAddr = (HI_U8*)pstFrameInfo->stVFrame.pVirAddr[0];
	stSrcData.u32PhyAddr = pstFrameInfo->stVFrame.u32PhyAddr[0];
	stSrcData.u16Width   = (HI_U16)pstFrameInfo->stVFrame.u32Width;
	stSrcData.u16Height  = (HI_U16)pstFrameInfo->stVFrame.u32Height;
	stSrcData.u16Stride  = (HI_U16)pstFrameInfo->stVFrame.u32Stride[0];

	//fill dst
	stDstData.pu8VirAddr = pstDst->pu8VirAddr[0];
	stDstData.u32PhyAddr = pstDst->u32PhyAddr[0];
	stDstData.u16Width   = pstDst->u16Width;
	stDstData.u16Height  = pstDst->u16Height;
	stDstData.u16Stride  = pstDst->u16Stride[0];

	s32Ret = HI_MPI_IVE_DMA(&hIveHandle,&stSrcData,&stDstData,&stCtrl,bInstant);	
	if (HI_SUCCESS != s32Ret)
	{
        SAMPLE_PRT("HI_MPI_IVE_DMA fail,Error(%#x)\n",s32Ret);
       return s32Ret;
    }
	
	if (HI_TRUE == bInstant)
	{
		s32Ret = HI_MPI_IVE_Query(hIveHandle,&bFinish,bBlock);			
		while(HI_ERR_IVE_QUERY_TIMEOUT == s32Ret)
		{
			usleep(100);					
			s32Ret = HI_MPI_IVE_Query(hIveHandle,&bFinish,bBlock);	
		}
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("HI_MPI_IVE_Query fail,Error(%#x)\n",s32Ret);
		   return s32Ret;
		}
	}

	return HI_SUCCESS;
}



#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

