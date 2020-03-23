#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>

#include "sample_comm.h"
#include "sample_comm_ivs.h"
#include "sample_ive_main.h"


#define MAX_POINT_NUM 200
#define MIN_DIST 5

typedef struct hiSAMPLE_IVE_ST_LK_S
{
	IVE_SRC_IMAGE_S  stSrc;
	IVE_DST_IMAGE_S  stDst;
	IVE_SRC_IMAGE_S  stSrcYUV;
	IVE_SRC_IMAGE_S	 astPrePyr[3];
	IVE_SRC_IMAGE_S	 astCurPyr[3];
	IVE_IMAGE_S stPyrTmp;
	IVE_DST_MEM_INFO_S stDstCorner;
	IVE_MEM_INFO_S   stMv;
	IVE_SRC_MEM_INFO_S astPoint[3];
	IVE_ST_CANDI_CORNER_CTRL_S  stStCandiCornerCtrl;
	IVE_ST_CORNER_CTRL_S	 stStCornerCtrl;
	IVE_LK_OPTICAL_FLOW_CTRL_S	stLkCtrl;
	FILE *pFpSrc;
	FILE *pFpDst;

}SAMPLE_IVE_ST_LK_S;

/******************************************************************************
* function : Copy pyr
******************************************************************************/
static HI_VOID SAMPLE_IVE_CopyPyr(IVE_IMAGE_S *pstPyrSrc, IVE_IMAGE_S *pstPyrDst, HI_U32 u32Level)
{
    HI_U32 i;
    for(i = 0;i < u32Level;i++)
    {
        memcpy(pstPyrDst[i].pu8VirAddr[0],pstPyrSrc[i].pu8VirAddr[0],pstPyrSrc[i].u16Stride[0] * pstPyrSrc[i].u16Height);
    }
}
/******************************************************************************
* function : St lk uninit
******************************************************************************/
static HI_VOID SAMPLE_IVE_St_Lk_Uninit(SAMPLE_IVE_ST_LK_S *pstStLk)
{
    HI_U16 i;
    IVE_MMZ_FREE(pstStLk->stSrc.u32PhyAddr[0],pstStLk->stSrc.pu8VirAddr[0]);
	IVE_MMZ_FREE(pstStLk->stDst.u32PhyAddr[0],pstStLk->stDst.pu8VirAddr[0]);
	IVE_MMZ_FREE(pstStLk->stStCandiCornerCtrl.stMem.u32PhyAddr,pstStLk->stStCandiCornerCtrl.stMem.pu8VirAddr);
    IVE_MMZ_FREE(pstStLk->stDstCorner.u32PhyAddr, pstStLk->stDstCorner.pu8VirAddr);

    for(i = 0;i < 3;i++)
    {
        IVE_MMZ_FREE(pstStLk->astPrePyr[i].u32PhyAddr[0], pstStLk->astPrePyr[i].pu8VirAddr[0]);
        IVE_MMZ_FREE(pstStLk->astCurPyr[i].u32PhyAddr[0], pstStLk->astCurPyr[i].pu8VirAddr[0]);
        IVE_MMZ_FREE(pstStLk->astPoint[i].u32PhyAddr, pstStLk->astPoint[i].pu8VirAddr);
    }
	IVE_MMZ_FREE(pstStLk->stPyrTmp.u32PhyAddr[0],pstStLk->stPyrTmp.pu8VirAddr[0]);
    IVE_MMZ_FREE(pstStLk->stMv.u32PhyAddr,pstStLk->stMv.pu8VirAddr);
    IVE_MMZ_FREE(pstStLk->stSrcYUV.u32PhyAddr[0],pstStLk->stSrcYUV.pu8VirAddr[0]);

    IVE_CLOSE_FILE(pstStLk->pFpSrc);
    IVE_CLOSE_FILE(pstStLk->pFpDst);
}

/******************************************************************************
* function : St lk init
******************************************************************************/
static HI_S32 SAMPLE_IVE_St_Lk_Init(SAMPLE_IVE_ST_LK_S *pstStLk,
	HI_CHAR *pchSrcFileName,HI_CHAR *pchDstFileName,HI_U16 u16Width,HI_U16 u16Height,
	HI_U16 u16PyrWidth,HI_U16 u16PyrHeight)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32Size = 0;
    HI_U16 i;

	memset(pstStLk,0,sizeof(SAMPLE_IVE_ST_LK_S));

	s32Ret = SAMPLE_COMM_IVE_CreateImage(&pstStLk->stSrc,IVE_IMAGE_TYPE_U8C1,u16Width,u16Height);
	if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("SAMPLE_COMM_IVE_CreateImage fail\n");
        goto ST_LK_INIT_FAIL;
    }   
	s32Ret = SAMPLE_COMM_IVE_CreateImage(&pstStLk->stDst,IVE_IMAGE_TYPE_U8C1,u16Width,u16Height);
	if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("SAMPLE_COMM_IVE_CreateImage fail\n");
        goto ST_LK_INIT_FAIL;
    }      
	s32Ret = SAMPLE_COMM_IVE_CreateImage(&pstStLk->stSrcYUV,IVE_IMAGE_TYPE_YUV420SP,u16Width,u16Height);
	if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("SAMPLE_COMM_IVE_CreateImage fail\n");
        goto ST_LK_INIT_FAIL;
    }   
	pstStLk->stStCandiCornerCtrl.u0q8QualityLevel = 25;
    u32Size = 4 * SAMPLE_COMM_IVE_CalcStride(u16Width, IVE_ALIGN) * u16Height + sizeof(IVE_ST_MAX_EIG_S);
	s32Ret = SAMPLE_COMM_IVE_CreateMemInfo(&(pstStLk->stStCandiCornerCtrl.stMem), u32Size);    
	if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("SAMPLE_COMM_IVE_CreateMemInfo fail\n");
        goto ST_LK_INIT_FAIL;
    }      
    pstStLk->stStCornerCtrl.u16MaxCornerNum = MAX_POINT_NUM;
    pstStLk->stStCornerCtrl.u16MinDist = MIN_DIST;
	u32Size = sizeof(IVE_ST_CORNER_INFO_S);
	s32Ret = SAMPLE_COMM_IVE_CreateMemInfo(&(pstStLk->stDstCorner), u32Size);    
	if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("SAMPLE_COMM_IVE_CreateMemInfo fail\n");
        goto ST_LK_INIT_FAIL;
    }    

    pstStLk->stLkCtrl.u0q8Epsilon = 2;
    pstStLk->stLkCtrl.u0q8MinEigThr = 100;
    pstStLk->stLkCtrl.u16CornerNum = MAX_POINT_NUM;
    pstStLk->stLkCtrl.u8IterCount = 10;
	s32Ret = SAMPLE_COMM_IVE_CreateImage(&pstStLk->stPyrTmp,IVE_IMAGE_TYPE_U8C1,u16PyrWidth,u16PyrHeight);
	if(s32Ret != HI_SUCCESS)
	{
		SAMPLE_PRT("SAMPLE_COMM_IVE_CreateImage fail\n");
		goto ST_LK_INIT_FAIL;
	} 
	
    u32Size = sizeof(IVE_POINT_S25Q7_S) * pstStLk->stLkCtrl.u16CornerNum;
    for(i = 0;i < 3;i++)
    {
        if(i != 0)
        {
			u16PyrWidth /= 2;
			u16PyrHeight /= 2;             
        }        

   		s32Ret = SAMPLE_COMM_IVE_CreateImage(&pstStLk->astPrePyr[i],IVE_IMAGE_TYPE_U8C1,u16PyrWidth,u16PyrHeight);
		if(s32Ret != HI_SUCCESS)
	    {
	        SAMPLE_PRT("SAMPLE_COMM_IVE_CreateImage fail\n");
	        goto ST_LK_INIT_FAIL;
	    } 
		s32Ret = SAMPLE_COMM_IVE_CreateImage(&pstStLk->astCurPyr[i],IVE_IMAGE_TYPE_U8C1,u16PyrWidth,u16PyrHeight);
		if(s32Ret != HI_SUCCESS)
	    {
	        SAMPLE_PRT("SAMPLE_COMM_IVE_CreateImage fail\n");
	        goto ST_LK_INIT_FAIL;
	    } 

		s32Ret = SAMPLE_COMM_IVE_CreateMemInfo(&(pstStLk->astPoint[i]),u32Size);
        if(s32Ret != HI_SUCCESS)
	    {
	        SAMPLE_PRT("SAMPLE_COMM_IVE_CreateMemInfo fail\n");
	        goto ST_LK_INIT_FAIL;
	    } 
    }
	

	u32Size =  sizeof(IVE_MV_S9Q7_S) * pstStLk->stLkCtrl.u16CornerNum;
	s32Ret = SAMPLE_COMM_IVE_CreateMemInfo(&(pstStLk->stMv),u32Size);
	if(s32Ret != HI_SUCCESS)
	{
		SAMPLE_PRT("SAMPLE_COMM_IVE_CreateMemInfo fail\n");
		goto ST_LK_INIT_FAIL;
	} 
    memset(pstStLk->stMv.pu8VirAddr,0,pstStLk->stMv.u32Size);
    
    pstStLk->pFpSrc = fopen(pchSrcFileName,"rb");
    if(HI_NULL == pstStLk->pFpSrc)
    {
        SAMPLE_PRT("Open file %s fail\n",pchSrcFileName);
        s32Ret = HI_FAILURE;
        goto ST_LK_INIT_FAIL;
    }

    pstStLk->pFpDst = fopen(pchDstFileName,"wb");
    if(HI_NULL == pstStLk->pFpDst)
    {
        SAMPLE_PRT("Open file %s fail\n",pchDstFileName);
        s32Ret = HI_FAILURE;
        goto ST_LK_INIT_FAIL;
    }

ST_LK_INIT_FAIL:
    
    if(HI_SUCCESS != s32Ret)
	{
		SAMPLE_IVE_St_Lk_Uninit(pstStLk);
	}
	return s32Ret;      
}
/******************************************************************************
* function : Pyr down
******************************************************************************/
static HI_S32 SAMPLE_IVE_PyrDown(SAMPLE_IVE_ST_LK_S *pstStLk,IVE_SRC_IMAGE_S *pstSrc, IVE_DST_IMAGE_S *pstDst)
{
    HI_S32 s32Ret = HI_SUCCESS;
    IVE_HANDLE hIveHandle;
    HI_BOOL bBlock = HI_TRUE;
    HI_BOOL bFinish = HI_FALSE;
    HI_BOOL bInstant = HI_TRUE;
    IVE_SRC_DATA_S  stDataSrc;
    IVE_DST_DATA_S  stDataDst;
    IVE_DMA_CTRL_S  stDmaCtrl = {IVE_DMA_MODE_INTERVAL_COPY,
                                    0,2,1,2};
    IVE_FILTER_CTRL_S stFltCtrl = {{1,2,3,2,1,
				 2,5,6,5,2,
				 3,6,8,6,3,
				 2,5,6,5,2,
				 1,2,3,2,1},7};

	pstStLk->stPyrTmp.u16Width = pstSrc->u16Width;
	pstStLk->stPyrTmp.u16Height = pstSrc->u16Height;    

    s32Ret = HI_MPI_IVE_Filter(&hIveHandle, pstSrc, &pstStLk->stPyrTmp,&stFltCtrl,  bInstant);
    if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("HI_MPI_IVE_Filter fail,Error(%#x)\n",s32Ret);
        return s32Ret;
    }
    stDataSrc.pu8VirAddr = pstStLk->stPyrTmp.pu8VirAddr[0];
    stDataSrc.u32PhyAddr = pstStLk->stPyrTmp.u32PhyAddr[0];
    stDataSrc.u16Stride = pstStLk->stPyrTmp.u16Stride[0];
    stDataSrc.u16Width = pstStLk->stPyrTmp.u16Width;
    stDataSrc.u16Height = pstStLk->stPyrTmp.u16Height;

    stDataDst.pu8VirAddr = pstDst->pu8VirAddr[0];
    stDataDst.u32PhyAddr = pstDst->u32PhyAddr[0];
    stDataDst.u16Stride = pstDst->u16Stride[0];
    stDataDst.u16Width = pstDst->u16Width;
    stDataDst.u16Height = pstDst->u16Height;
    s32Ret = HI_MPI_IVE_DMA(&hIveHandle, &stDataSrc, &stDataDst, &stDmaCtrl, bInstant);
    if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("HI_MPI_IVE_DMA fail,Error(%#x)",s32Ret);
        return s32Ret;
    }
    s32Ret = HI_MPI_IVE_Query(hIveHandle, &bFinish, bBlock);	
	while (HI_ERR_IVE_QUERY_TIMEOUT == s32Ret)
	{
		usleep(200);
		s32Ret = HI_MPI_IVE_Query(hIveHandle, &bFinish, bBlock);
	}
    if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("HI_MPI_IVE_Query fail,Error(%#x)\n",s32Ret);
        return s32Ret;
    }
    
    return s32Ret;
}
/******************************************************************************
* function : St lk proc
******************************************************************************/
static HI_VOID SAMPLE_IVE_St_LkProc(SAMPLE_IVE_ST_LK_S *pstStLk)
{
    HI_U32 u32FrameNum = 10;
    HI_S16 i,j,k;
    HI_U16 u16RectNum;
    HI_S32 s32Ret = HI_SUCCESS;	
  	HI_BOOL bInstant = HI_TRUE;
    HI_BOOL bBlock = HI_TRUE;
    HI_BOOL bFinish = HI_FALSE;
	VGS_HANDLE VgsHandle;   
    IVE_HANDLE IveHandle;
    RECT_S  astRect[MAX_POINT_NUM];
    IVE_POINT_S25Q7_S astPointTmp[MAX_POINT_NUM];
    IVE_ST_CORNER_INFO_S *pstCornerInfo = (IVE_ST_CORNER_INFO_S *)pstStLk->stDstCorner.pu8VirAddr;
    IVE_POINT_S25Q7_S *pstPoint[3] = {(IVE_POINT_S25Q7_S *)pstStLk->astPoint[0].pu8VirAddr,
                                      (IVE_POINT_S25Q7_S *)pstStLk->astPoint[1].pu8VirAddr,
                                      (IVE_POINT_S25Q7_S *)pstStLk->astPoint[2].pu8VirAddr};
    IVE_MV_S9Q7_S *pstMv = (IVE_MV_S9Q7_S *)pstStLk->stMv.pu8VirAddr;
    
    for(i = 0;i < u32FrameNum;i++)
    {
    	SAMPLE_PRT("Proc frame %d\n",i);
        s32Ret = SAMPLE_COMM_IVE_ReadFile(&pstStLk->stSrc,pstStLk->pFpSrc);
        if(s32Ret != HI_SUCCESS)
        {
           SAMPLE_PRT("SAMPLE_COMM_IVE_ReadFile fail\n");
           break;
        }

        if(0 == i)
        {
            s32Ret = HI_MPI_IVE_STCandiCorner(&IveHandle, &pstStLk->stSrc, &pstStLk->stDst,
				&pstStLk->stStCandiCornerCtrl, bInstant);
            if(s32Ret != HI_SUCCESS)
            {
                SAMPLE_PRT("HI_MPI_IVE_STCandiCorner fail,Error(%#x)\n",s32Ret);
                break;
            }
			
            s32Ret = HI_MPI_IVE_Query(IveHandle, &bFinish, bBlock);			
			while(HI_ERR_IVE_QUERY_TIMEOUT == s32Ret)
			{
				usleep(100);					
				s32Ret = HI_MPI_IVE_Query(IveHandle,&bFinish,bBlock);	
			}
            if(s32Ret != HI_SUCCESS)
            {
                SAMPLE_PRT("HI_MPI_IVE_Query fail,Error(%#x)\n",s32Ret);
                break;
            }
			
            s32Ret = HI_MPI_IVE_STCorner(&pstStLk->stDst, &pstStLk->stDstCorner, &pstStLk->stStCornerCtrl);
            if(s32Ret != HI_SUCCESS)
            {
                SAMPLE_PRT("HI_MPI_IVE_STCorner fail,Error(%#x)\n",s32Ret);
                break;
            }
            pstStLk->stLkCtrl.u16CornerNum = pstCornerInfo->u16CornerNum;
            for(k = 0; k < pstStLk->stLkCtrl.u16CornerNum;k++)
            {
                astPointTmp[k].s25q7X = (HI_S32)(pstCornerInfo->astCorner[k].u16X << 7);
                astPointTmp[k].s25q7Y = (HI_S32)(pstCornerInfo->astCorner[k].u16Y << 7);
            }
            
        }

        for(k = 0;k < pstCornerInfo->u16CornerNum;k++)
        {
            for(j = 0; j < 3;j++)
            {
                if(0 == j)
                {
                    pstPoint[0][k].s25q7X = astPointTmp[k].s25q7X;
                    pstPoint[0][k].s25q7Y = astPointTmp[k].s25q7Y;    
                }
                else
                {
                    pstPoint[j][k].s25q7X = pstPoint[j - 1][k].s25q7X / 2;
                    pstPoint[j][k].s25q7Y = pstPoint[j - 1][k].s25q7Y / 2;
                }
                
            }
        }

        for(j = 0;j < 3;j++)
        {
            if(0 == j)
            {
                memcpy(pstStLk->astCurPyr[j].pu8VirAddr[0],pstStLk->stSrc.pu8VirAddr[0], 
					pstStLk->stSrc.u16Stride[0] * pstStLk->stSrc.u16Height);
            }
            else
            {
                s32Ret = SAMPLE_IVE_PyrDown(pstStLk,&pstStLk->astCurPyr[j - 1],&pstStLk->astCurPyr[j]);
                if(s32Ret != HI_SUCCESS)
                {
                    SAMPLE_PRT("SAMPLE_IVE_PyrDown fail\n");
                    return;
                }
            }
           
        }

       memcpy(pstStLk->stSrcYUV.pu8VirAddr[0],pstStLk->stSrc.pu8VirAddr[0],pstStLk->stSrcYUV.u16Stride[0] * pstStLk->stSrcYUV.u16Height);
       memset(pstStLk->stSrcYUV.pu8VirAddr[1],128,pstStLk->stSrcYUV.u16Stride[1] * pstStLk->stSrcYUV.u16Height / 2);
       
        if(i > 0)
        {
            for(j = 2;j >= 0;j--)
            {
                s32Ret = HI_MPI_IVE_LKOpticalFlow(&IveHandle, &pstStLk->astPrePyr[j], &pstStLk->astCurPyr[j], 
                            &pstStLk->astPoint[j], &pstStLk->stMv, &pstStLk->stLkCtrl, bInstant);
                if(s32Ret != HI_SUCCESS)
                {
                    SAMPLE_PRT("HI_MPI_IVE_LKOpticalFlow fail,Error(%#x)\n",s32Ret);
                    return;
                }
                
                s32Ret = HI_MPI_IVE_Query(IveHandle, &bFinish, bBlock);
				while (HI_ERR_IVE_QUERY_TIMEOUT == s32Ret)
				{
					usleep(200);
					s32Ret = HI_MPI_IVE_Query(IveHandle, &bFinish, bBlock);
				}
                if(s32Ret != HI_SUCCESS)
                {
                    SAMPLE_PRT("HI_MPI_IVE_Query fail,Error(%#x)\n",s32Ret);
                    return;
                }
            }
            
            for(k = 0,u16RectNum = 0; k < pstCornerInfo->u16CornerNum;k++)
            {
                astPointTmp[k].s25q7X = (pstPoint[0][k].s25q7X + pstMv[k].s9q7Dx);
                astPointTmp[k].s25q7Y = (pstPoint[0][k].s25q7Y+ pstMv[k].s9q7Dy);            
                if(0 == pstMv[k].s32Status)
                {
                    astRect[u16RectNum].s32X = astPointTmp[k].s25q7X / 128 - 1;
                    astRect[u16RectNum].s32Y = astPointTmp[k].s25q7Y / 128 - 1;
                    astRect[u16RectNum].u32Width = 4;
                    astRect[u16RectNum].u32Height = 4;
                    u16RectNum++;
                    
                }                
            }
            if(u16RectNum > 0)
            {
                s32Ret =  HI_MPI_VGS_BeginJob(&VgsHandle);
                if(s32Ret != HI_SUCCESS)
                {
                    SAMPLE_PRT("HI_MPI_VGS_BeginJob fail,Error(%#x)\n",s32Ret);
                    break;
                }
                s32Ret = SAMPLE_COMM_VGS_AddDrawRectJob(VgsHandle, &pstStLk->stSrcYUV, &pstStLk->stSrcYUV, astRect, u16RectNum);
                if(s32Ret != HI_SUCCESS)
                {
                    SAMPLE_PRT("SAMPLE_COMM_VGS_AddDrawRectJob fail\n");
                    break;
                }
                s32Ret = HI_MPI_VGS_EndJob(VgsHandle);
                if(s32Ret != HI_SUCCESS)
                {
                    HI_MPI_VGS_CancelJob(VgsHandle);
                    SAMPLE_PRT("HI_MPI_VGS_EndJob fail,Error(%#x)\n",s32Ret);
                    break;
                }
            }           
            
        }
        
        SAMPLE_COMM_IVE_WriteFile(&pstStLk->stSrcYUV,pstStLk->pFpDst);
       
        SAMPLE_IVE_CopyPyr(pstStLk->astCurPyr,pstStLk->astPrePyr,3);
    }
    return;
    
}
/******************************************************************************
* function : show St Lk sample
******************************************************************************/
HI_VOID SAMPLE_IVE_St_Lk(HI_VOID)
{
	SAMPLE_IVE_ST_LK_S stStLk;
	HI_CHAR *pchSrcFileName = "./data/input/stlk/st_lk_720x576.yuv";
	HI_CHAR *pchDstFileName = "./data/output/stlk/st_lk_720x576.yuv";
	HI_U16 u16Width = 720;
	HI_U16 u16Height = 576;
	HI_U16 u16PyrWidth = 720;
	HI_U16 u16PyrHeight = 576;
	HI_S32 s32Ret;
	
    SAMPLE_COMM_IVE_CheckIveMpiInit();
	
   s32Ret =  SAMPLE_IVE_St_Lk_Init(&stStLk,	pchSrcFileName,pchDstFileName,
   	u16Width,u16Height,u16PyrWidth,u16PyrHeight);
    if(HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_IVE_St_Lk_Init fail\n");
        goto ST_LK_FAIL;
    }

    SAMPLE_IVE_St_LkProc(&stStLk);
    
    SAMPLE_IVE_St_Lk_Uninit(&stStLk);

ST_LK_FAIL:
    SAMPLE_COMM_IVE_IveMpiExit();
}




