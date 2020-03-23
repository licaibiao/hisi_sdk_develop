#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "sample_comm.h"
#include "sample_comm_ivs.h"
#include "sample_ive_main.h"

#define VPSS_CHN_NUM 2

typedef struct hiSAMPLE_IVE_GMM_S
{
	IVE_SRC_IMAGE_S stSrc;
	IVE_DST_IMAGE_S stFg;
	IVE_DST_IMAGE_S stBg;
	IVE_MEM_INFO_S  stModel;
	IVE_IMAGE_S	stImg1;
	IVE_IMAGE_S	stImg2;
	IVE_DST_MEM_INFO_S stBlob;
	IVE_GMM_CTRL_S  stGmmCtrl;
	IVE_CCL_CTRL_S  stCclCtrl;
	IVE_FILTER_CTRL_S stFltCtrl;
	IVE_DILATE_CTRL_S stDilateCtrl;
	IVE_ERODE_CTRL_S stErodeCtrl;
	SAMPLE_RECT_ARRAY_S stRegion;
}SAMPLE_IVE_GMM_S;

typedef struct hiSAMPLE_IVE_GMM_THREAD_PARAM_S
{	
	HI_BOOL bStop;
	HI_BOOL bEncode;
	HI_BOOL bVo;

	HI_U32  u32FrmCnt;
	
	HI_U32 u32VpssGrpCnt;
	HI_U32 u32VpssChnNum;
	VPSS_CHN aVpssChn[VPSS_MAX_CHN_NUM];

	VENC_CHN VeH264Chn;	
	VO_LAYER VoLayer;
	VO_CHN   VoChn;
	
	SAMPLE_IVE_GMM_S *pstGmm;
}SAMPLE_IVE_GMM_THREAD_PARAM_S;

static HI_VOID SAMPLE_IVE_GMM_Uninit(SAMPLE_IVE_GMM_S *pstGmm)
{
    IVE_MMZ_FREE(pstGmm->stSrc.u32PhyAddr[0],pstGmm->stSrc.pu8VirAddr[0]);
	IVE_MMZ_FREE(pstGmm->stFg.u32PhyAddr[0],pstGmm->stFg.pu8VirAddr[0]);
    IVE_MMZ_FREE(pstGmm->stBg.u32PhyAddr[0],pstGmm->stBg.pu8VirAddr[0]);
	IVE_MMZ_FREE(pstGmm->stModel.u32PhyAddr,pstGmm->stModel.pu8VirAddr);
    IVE_MMZ_FREE(pstGmm->stImg1.u32PhyAddr[0],pstGmm->stImg1.pu8VirAddr[0]);
	IVE_MMZ_FREE(pstGmm->stImg2.u32PhyAddr[0],pstGmm->stImg2.pu8VirAddr[0]);
    IVE_MMZ_FREE(pstGmm->stBlob.u32PhyAddr,pstGmm->stBlob.pu8VirAddr);
}


static HI_S32 SAMPLE_IVE_GMM_Init(SAMPLE_IVE_GMM_S *pstGmm,HI_U16 u16Width,HI_U16 u16Height)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32Size = 0;
    HI_S8 as8Mask[25]={1,2,3,2,1,
		2,5,6,5,2,
		3,6,8,6,3,
		2,5,6,5,2,
		1,2,3,2,1};

	memset(pstGmm,0,sizeof(SAMPLE_IVE_GMM_S));

	s32Ret = SAMPLE_COMM_IVE_CreateImage(&(pstGmm->stSrc),IVE_IMAGE_TYPE_U8C1,u16Width,u16Height);
	SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS!= s32Ret, MD_INIT_FAIL, "SAMPLE_COMM_IVE_CreateImage fail\n");
	
	s32Ret = SAMPLE_COMM_IVE_CreateImage(&(pstGmm->stFg),IVE_IMAGE_TYPE_U8C1,u16Width,u16Height);
	SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS!= s32Ret, MD_INIT_FAIL, "SAMPLE_COMM_IVE_CreateImage fail\n");
	
	s32Ret = SAMPLE_COMM_IVE_CreateImage(&(pstGmm->stBg),IVE_IMAGE_TYPE_U8C1,u16Width,u16Height);
	SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS!= s32Ret, MD_INIT_FAIL, "SAMPLE_COMM_IVE_CreateImage fail\n");

    pstGmm->stGmmCtrl.u0q16InitWeight = 3276; //0.05
    pstGmm->stGmmCtrl.u0q16BgRatio = 52428;   //0.8
    pstGmm->stGmmCtrl.u22q10MaxVar = (2000 << 10);
    pstGmm->stGmmCtrl.u22q10MinVar = (200 << 10);
    pstGmm->stGmmCtrl.u22q10NoiseVar = (225<<10);
    pstGmm->stGmmCtrl.u8q8VarThr = 1600;
    pstGmm->stGmmCtrl.u8ModelNum = 3;
    pstGmm->stGmmCtrl.u0q16LearnRate = 327;
	
    u32Size = pstGmm->stSrc.u16Width * pstGmm->stSrc.u16Height * pstGmm->stGmmCtrl.u8ModelNum * 7;
	s32Ret = SAMPLE_COMM_IVE_CreateMemInfo(&pstGmm->stModel,u32Size);
	SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS!= s32Ret, MD_INIT_FAIL, "SAMPLE_COMM_IVE_CreateMemInfo fail\n");
    memset(pstGmm->stModel.pu8VirAddr, 0, pstGmm->stModel.u32Size);

	s32Ret = SAMPLE_COMM_IVE_CreateImage(&(pstGmm->stImg1),IVE_IMAGE_TYPE_U8C1,u16Width,u16Height);
	SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS!= s32Ret, MD_INIT_FAIL, "SAMPLE_COMM_IVE_CreateImage fail\n");
	
    s32Ret = SAMPLE_COMM_IVE_CreateImage(&(pstGmm->stImg2),IVE_IMAGE_TYPE_U8C1,u16Width,u16Height);
	SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS!= s32Ret, MD_INIT_FAIL, "SAMPLE_COMM_IVE_CreateImage fail\n");

    u32Size = sizeof(IVE_CCBLOB_S);
	s32Ret = SAMPLE_COMM_IVE_CreateMemInfo(&pstGmm->stBlob,u32Size);
	SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS!= s32Ret, MD_INIT_FAIL, "SAMPLE_COMM_IVE_CreateMemInfo fail\n");
   
    memcpy(pstGmm->stFltCtrl.as8Mask,as8Mask,25);
    pstGmm->stFltCtrl.u8Norm = 7;
    memset(pstGmm->stDilateCtrl.au8Mask,255,25);
    memset(pstGmm->stErodeCtrl.au8Mask,255,25);
    pstGmm->stCclCtrl.u16InitAreaThr = 16;
    pstGmm->stCclCtrl.u16Step = 4;
    
MD_INIT_FAIL:
    
    if(HI_SUCCESS != s32Ret)
	{
        SAMPLE_IVE_GMM_Uninit(pstGmm);
	}
	return s32Ret;      
}

HI_S32 SAMPLE_IVE_GMM_DetectObj(VIDEO_FRAME_INFO_S *pstBigFrm, 
	VIDEO_FRAME_INFO_S *pstSmallFrm, SAMPLE_IVE_GMM_S *pstGmm, HI_U32 *pu32ObjNum)
{
    HI_S32 s32Ret = HI_SUCCESS;
	
	IVE_DATA_S stSrc;
	IVE_DATA_S stDst;
    IVE_HANDLE IveHandle;
    HI_BOOL bFinish = HI_FALSE;
    HI_BOOL bBlock = HI_TRUE;
    HI_BOOL bInstant = HI_TRUE;
    IVE_CCBLOB_S *pstBlob;
	IVE_DMA_CTRL_S stDmaCtrl = {IVE_DMA_MODE_DIRECT_COPY,0};
	
	HI_FLOAT f32SclX, f32SclY;
	HI_U32 u32Color = 0x0000FF00;

    pstBlob = (IVE_CCBLOB_S *)pstGmm->stBlob.pu8VirAddr;

	//1.Get Y
	stSrc.pu8VirAddr = (HI_U8*)pstSmallFrm->stVFrame.pVirAddr[0];
	stSrc.u32PhyAddr = pstSmallFrm->stVFrame.u32PhyAddr[0];
	stSrc.u16Stride = (HI_U16)pstSmallFrm->stVFrame.u32Stride[0];
	stSrc.u16Width = (HI_U16)pstSmallFrm->stVFrame.u32Width;
	stSrc.u16Height = pstSmallFrm->stVFrame.u32Height;

	stDst.pu8VirAddr = pstGmm->stSrc.pu8VirAddr[0];
	stDst.u32PhyAddr = pstGmm->stSrc.u32PhyAddr[0];
	stDst.u16Stride 	= pstGmm->stSrc.u16Stride[0];
	stDst.u16Width 	= pstGmm->stSrc.u16Width;
	stDst.u16Height 	= pstGmm->stSrc.u16Height;
	bInstant = HI_FALSE;
	stDmaCtrl.enMode = IVE_DMA_MODE_DIRECT_COPY;
	
	s32Ret = HI_MPI_IVE_DMA(&IveHandle,&stSrc,&stDst,&stDmaCtrl,bInstant);
	SAMPLE_CHECK_EXPR_RET(HI_SUCCESS != s32Ret, s32Ret, "HI_MPI_IVE_DMA fail,Error(%#x)\n",s32Ret);
	
	//2.Filter
	s32Ret = HI_MPI_IVE_Filter(&IveHandle, &pstGmm->stSrc, &pstGmm->stImg1, &pstGmm->stFltCtrl, bInstant);
	SAMPLE_CHECK_EXPR_RET(HI_SUCCESS != s32Ret, s32Ret, "HI_MPI_IVE_Filter fail,Error(%#x)\n",s32Ret);
	
	//3.Gmm
	s32Ret = HI_MPI_IVE_GMM(&IveHandle, &pstGmm->stImg1, &pstGmm->stFg,&pstGmm->stBg,
	&pstGmm->stModel,&pstGmm->stGmmCtrl, bInstant);
	SAMPLE_CHECK_EXPR_RET(HI_SUCCESS != s32Ret, s32Ret, "HI_MPI_IVE_GMM fail,Error(%#x)\n",s32Ret);
	
	//4.Dilate
	s32Ret = HI_MPI_IVE_Dilate(&IveHandle, &pstGmm->stFg, &pstGmm->stImg1,&pstGmm->stDilateCtrl, bInstant);
	SAMPLE_CHECK_EXPR_RET(HI_SUCCESS != s32Ret, s32Ret, "HI_MPI_IVE_Dilate fail,Error(%#x)\n",s32Ret);
	
	//5.Erode
	s32Ret = HI_MPI_IVE_Erode(&IveHandle, &pstGmm->stImg1, &pstGmm->stImg2,&pstGmm->stErodeCtrl, bInstant);
	SAMPLE_CHECK_EXPR_RET(HI_SUCCESS != s32Ret, s32Ret, "HI_MPI_IVE_Erode fail,Error(%#x)\n",s32Ret);
	
	//6.CCL
	bInstant = HI_TRUE;
	s32Ret = HI_MPI_IVE_CCL(&IveHandle, &pstGmm->stImg2, &pstGmm->stBlob,&pstGmm->stCclCtrl, bInstant);
	SAMPLE_CHECK_EXPR_RET(HI_SUCCESS != s32Ret, s32Ret, "HI_MPI_IVE_CCL fail,Error(%#x)\n",s32Ret);
	
	//Wait task finish
	s32Ret = HI_MPI_IVE_Query(IveHandle, &bFinish, bBlock);	
	while(HI_ERR_IVE_QUERY_TIMEOUT == s32Ret)
	{
		usleep(100);					
		s32Ret = HI_MPI_IVE_Query(IveHandle,&bFinish,bBlock);	
	}
	SAMPLE_CHECK_EXPR_RET(HI_SUCCESS != s32Ret, s32Ret, "HI_MPI_IVE_Query fail,Error(%#x)\n",s32Ret);
	
	f32SclX = (HI_FLOAT)pstBigFrm->stVFrame.u32Width/pstSmallFrm->stVFrame.u32Width;
	f32SclY = (HI_FLOAT)pstBigFrm->stVFrame.u32Height/pstSmallFrm->stVFrame.u32Height;
	SAMPLE_COMM_IVE_BlobToRect(pstBlob,&(pstGmm->stRegion),MAX_OBJ_NUM,CCL_AREA_THR_STEP,f32SclX,f32SclY);
       
   s32Ret = SAMPLE_COMM_VGS_FillRect(pstBigFrm, &pstGmm->stRegion, u32Color);
   SAMPLE_CHECK_EXPR_RET(HI_SUCCESS != s32Ret,s32Ret, "SAMPLE_COMM_VGS_FillRect failed,Error(%#x)\n",s32Ret);

   *pu32ObjNum = pstGmm->stRegion.u16Num;
   
    return HI_SUCCESS;
}

HI_VOID * SAMPLE_IVE_GMM_Process(HI_VOID *pArgs)
{	
	HI_S32 s32Ret = HI_SUCCESS;
	SAMPLE_IVE_GMM_THREAD_PARAM_S *pstThreadParam;

	HI_U32 u32ObjNum, i;
	
	HI_U32 u32VpssGrpCnt;
	VPSS_CHN aVpssChn[VPSS_MAX_CHN_NUM];
	VENC_CHN VeH264Chn;	
	VO_LAYER VoLayer;
	VO_CHN   VoChn;	
	SAMPLE_IVE_GMM_S *pstGmm;

	pstThreadParam = (SAMPLE_IVE_GMM_THREAD_PARAM_S*)pArgs;
	u32VpssGrpCnt = pstThreadParam->u32VpssGrpCnt;
	aVpssChn[0]	  = pstThreadParam->aVpssChn[0];
	aVpssChn[1]	  = pstThreadParam->aVpssChn[1];
	VeH264Chn     = pstThreadParam->VeH264Chn;
	VoLayer       = pstThreadParam->VoLayer;
	VoChn		  = pstThreadParam->VoChn;
	pstGmm	  = pstThreadParam->pstGmm;
	
	while(HI_TRUE!=pstThreadParam->bStop)
	{				
		HI_S32 s32MilliSec = 2000000;
		VIDEO_FRAME_INFO_S stBigFrm, stSmallFrm;
		
		for(i=0; i<u32VpssGrpCnt; i++)
		{
			VPSS_GRP VpssGrp = i;
			
			s32Ret = HI_MPI_VPSS_GetChnFrame(VpssGrp, aVpssChn[1], &stSmallFrm, s32MilliSec);
			if(HI_SUCCESS!=s32Ret)
			{
				SAMPLE_PRT("HI_MPI_VPSS_GetChnFrame small frame failed, VPSS_GRP(%d), VPSS_CHN(%d), errno: %#x!",
					VpssGrp, aVpssChn[1], s32Ret);
				continue;
			}
			
			s32Ret = HI_MPI_VPSS_GetChnFrame(VpssGrp, aVpssChn[0], &stBigFrm, s32MilliSec);
			SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS!=s32Ret, FAIL_GET_BIG_FRM,
				"HI_MPI_VPSS_GetChnFrame big frame failed, VPSS_GRP(%d), VPSS_CHN(%d), errno: %#x!",
				VpssGrp, aVpssChn[0], s32Ret);
			
			s32Ret = SAMPLE_IVE_GMM_DetectObj(&stBigFrm, &stSmallFrm, pstGmm, &u32ObjNum);			
			SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS!=s32Ret, FAIL_GMM, "SAMPLE_IVE_GMM_DetectObj failed with errno: %#x!", s32Ret);

			pstThreadParam->u32FrmCnt++;			
			printf("%dth frame, %d %s detected!\n", pstThreadParam->u32FrmCnt, u32ObjNum, u32ObjNum>1?"objects":"object");
			
			//Send to venc
			 if(pstThreadParam->bEncode)
			 {
				s32Ret = HI_MPI_VENC_SendFrame(VeH264Chn, &stBigFrm, s32MilliSec);
				if(HI_SUCCESS != s32Ret)
				{
					SAMPLE_PRT("HI_MPI_VENC_SendFrame fail with errno: %#x\n",s32Ret);
				} 
			 }
			 //Send to vo
			 if(pstThreadParam->bVo)
			 {			  
				 s32Ret = HI_MPI_VO_SendFrame(VoLayer, VoChn, &stBigFrm, s32MilliSec);
				 if (HI_SUCCESS != s32Ret)
				 {
					 SAMPLE_PRT("HI_MPI_VO_SendFrame,Error(%#x)\n",s32Ret);
				 }
			 }
			 
		FAIL_GMM:	 
			 //Release small frame
			 s32Ret = HI_MPI_VPSS_ReleaseChnFrame(VpssGrp, aVpssChn[1], &stSmallFrm);
			 if (HI_SUCCESS != s32Ret)
			 {
				 SAMPLE_PRT("HI_MPI_VPSS_ReleaseChnFrame failed,VpssGrp(%d),VpssChn(%d),Error(%#x)\n",VpssGrp,aVpssChn[1],s32Ret);
			 }
			 
		FAIL_GET_BIG_FRM:
			//Release big frame
			s32Ret = HI_MPI_VPSS_ReleaseChnFrame(VpssGrp, aVpssChn[1], &stBigFrm);
			if (HI_SUCCESS != s32Ret)
			{
				SAMPLE_PRT("HI_MPI_VPSS_ReleaseChnFrame failed,VpssGrp(%d),VpssChn(%d),Error(%#x)\n",VpssGrp,aVpssChn[1],s32Ret);
			}
	
		}
	
		usleep(20000);
	}
	
    return (HI_VOID *)HI_SUCCESS;
}

HI_VOID SAMPLE_IVE_GMM_ThreadParam(SAMPLE_IVE_GMM_THREAD_PARAM_S *pstThreadParam,
	HI_BOOL bEncode, HI_BOOL bVo, HI_U32 u32VpssGrpCnt, HI_U32 u32VpssChnNum, VPSS_CHN aVpssChn[],
	VENC_CHN VeH264Chn, VO_LAYER VoLayer, VO_CHN VoChn, SAMPLE_IVE_GMM_S *pstGmm)
{
	HI_S32 i;

	HI_ASSERT(HI_NULL!=pstThreadParam);
	HI_ASSERT(u32VpssChnNum<VPSS_MAX_CHN_NUM);
	HI_ASSERT(HI_NULL!=pstGmm);

	memset(pstThreadParam, 0, sizeof(SAMPLE_IVE_GMM_THREAD_PARAM_S));

	pstThreadParam->bStop = HI_FALSE;
	pstThreadParam->bEncode = bEncode;
	pstThreadParam->bVo     = bVo;
	pstThreadParam->u32FrmCnt = 0;
	
	pstThreadParam->u32VpssGrpCnt = u32VpssGrpCnt;
	pstThreadParam->u32VpssChnNum = u32VpssChnNum;
	for(i=0; i<u32VpssChnNum; i++)
		pstThreadParam->aVpssChn[i] = aVpssChn[i];
	
	pstThreadParam->VeH264Chn = VeH264Chn;
	pstThreadParam->VoLayer   = VoLayer;
	pstThreadParam->VoChn     = VoChn;

	pstThreadParam->pstGmm = pstGmm;
}

HI_VOID SAMPLE_IVE_GMM_ThreadProc(SAMPLE_IVE_GMM_THREAD_PARAM_S *pstThreadParam, 
											  pthread_t *pIveThread)
{
	pthread_create(pIveThread, 0, SAMPLE_IVE_GMM_Process, (HI_VOID *)pstThreadParam);
}

HI_VOID SAMPLE_IVE_GMM_CmdCtrl(SAMPLE_IVE_GMM_THREAD_PARAM_S *pstThreadParam, pthread_t IveThread)
{
    char c=0;

    printf("\nSAMPLE_TEST: press 'e' to exit \n\n"); 

    while(1)    
    {
        c = getchar();
        if(10 == c)
        {
            continue;
        }
        getchar();
        if (c == 'e' || c == 'E')
        {
			pstThreadParam->bStop = HI_TRUE;
			pthread_join(IveThread,HI_NULL);
            break;
        }
    }
}


/******************************************************************************
* function : show GMM sample
******************************************************************************/
HI_S32 SAMPLE_IVE_GMM(HI_BOOL bEncode, HI_BOOL bVo)
{
    HI_S32 s32Ret = HI_SUCCESS, i;
	
    VB_CONF_S stVbConf,stModVbConf;
	SIZE_S astSize[VPSS_CHN_NUM] = {{HD_WIDTH, HD_HEIGHT}, {QVGA_WIDTH, QVGA_HEIGHT}};
    HI_U32 u32Profile = 1; /*0: baseline; 1:MP; 2:HP 3:svc-t */
    PAYLOAD_TYPE_E enStreamType = PT_H264;
	
    VDEC_CHN_ATTR_S stVdecChnAttr;
    VdecThreadParam stVdecSendParam;
    pthread_t VdecThread;
	HI_U32 u32VdecChnCnt = 1;
	
    VPSS_CHN aVpssChn[VPSS_CHN_NUM] = {VPSS_CHN0, VPSS_CHN3};
    VPSS_GRP_ATTR_S stVpssGrpAttr;
	HI_U32 u32VpssGrpCnt = 1;
	
    VENC_CHN VeH264Chn = 0;
    SAMPLE_RC_E enRcMode = SAMPLE_RC_CBR;
    PIC_SIZE_E enPicSize = PIC_HD1080;
    VIDEO_NORM_E enVideoNorm = VIDEO_ENCODING_MODE_PAL;
	HI_U32 u32VencChnCnt = 1;
		
    VO_DEV   VoDev	 = SAMPLE_VO_DEV_DHD0;
	VO_CHN   VoChn   = 0;
    VO_LAYER VoLayer = SAMPLE_VO_LAYER_VHD0;
    VO_PUB_ATTR_S stVoPubAttr;
    VO_VIDEO_LAYER_ATTR_S stVoLayerAttr;
    SAMPLE_VO_MODE_E enSampleVoMode = VO_MODE_1MUX;

	SAMPLE_IVE_GMM_S stGmm;
	SAMPLE_IVE_GMM_THREAD_PARAM_S stGmmThreadParam;
    pthread_t IveThread;
	HI_CHAR *pchStreamName = "./data/input/ive_perimeter_scene_1080p_01.h264";
	
    /******************************************
     step  1: Init SYS and common VB
    ******************************************/
	SAMPLE_COMM_IVS_SysConf(&stVbConf, astSize, VPSS_CHN_NUM);
	s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
	SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_SYS_VB, "init SYS and common VB failed!\n");
	
    /******************************************
     step 2: Init VDEC mod VB. 
    *****************************************/    	
    SAMPLE_COMM_VDEC_ModCommPoolConf(&stModVbConf, enStreamType, &astSize[0],
        u32VdecChnCnt);	
    s32Ret = SAMPLE_COMM_VDEC_InitModCommVb(&stModVbConf);
	SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_SYS_VB, "init VDEC mod VB failed!\n");
	
    /******************************************
     step  3: Init GMM
    ******************************************/
	s32Ret = SAMPLE_IVE_GMM_Init(&stGmm, astSize[1].u32Width,astSize[1].u32Height);
	SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_GMM_INIT, "SAMPLE_IVE_GMM_Init failed\n");
	
    /******************************************
     step 4:  start VDEC.
    *****************************************/
    SAMPLE_COMM_VDEC_ChnAttr(u32VdecChnCnt, &stVdecChnAttr, enStreamType, &astSize[0]);
	s32Ret =SAMPLE_COMM_VDEC_Start(u32VdecChnCnt, &stVdecChnAttr);
	SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,END_VDEC_START, "start VDEC failed with!\n");

    /******************************************
     step 5:  start VPSS and bind to VDEC.
    *****************************************/
    SAMPLE_COMM_IVS_VpssGrpAttr(u32VpssGrpCnt, &stVpssGrpAttr, &astSize[0]);
    s32Ret = SAMPLE_COMM_IVS_VpssStart(u32VpssGrpCnt, astSize, aVpssChn, VPSS_CHN_NUM, &stVpssGrpAttr);
	SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_VPSS_START, "start VPSS failed!\n");
	
    for(i=0;i<u32VpssGrpCnt;i++)
    {	
        s32Ret = SAMPLE_COMM_VDEC_BindVpss(i,i);
		SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_VPSS_BIND_VDEC, "SAMPLE_COMM_VDEC_BindVpss failed!\n");
    }
	
    /************************************************
    step 6:  start VO
    *************************************************/
    if(bVo)
    {		
		stVoPubAttr.enIntfSync = VO_OUTPUT_1080P30;
		stVoPubAttr.enIntfType = VO_INTF_VGA; //VO_INTF_HDMI | VO_INTF_VGA;
    	stVoPubAttr.u32BgColor = 0x0000FF;
		
		stVoLayerAttr.bClusterMode = HI_FALSE;
		stVoLayerAttr.bDoubleFrame = HI_FALSE;
		stVoLayerAttr.enPixFormat = SAMPLE_PIXEL_FORMAT;
		
		s32Ret = SAMPLE_COMM_VO_GetWH(stVoPubAttr.enIntfSync, \
			&stVoLayerAttr.stDispRect.u32Width, &stVoLayerAttr.stDispRect.u32Height, &stVoLayerAttr.u32DispFrmRt);
		SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_VPSS_BIND_VDEC, "SAMPLE_COMM_VO_GetWH failed!\n");
		
		stVoLayerAttr.stDispRect.s32X = 0;
		stVoLayerAttr.stDispRect.s32Y = 0;
		stVoLayerAttr.stImageSize.u32Width = stVoLayerAttr.stDispRect.u32Width;
		stVoLayerAttr.stImageSize.u32Height = stVoLayerAttr.stDispRect.u32Height;

		s32Ret = SAMPLE_COMM_VO_StartDev(VoDev, &stVoPubAttr);
		SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_VO_START, "SAMPLE_COMM_VO_StartDev failed!\n");
		
		s32Ret = SAMPLE_COMM_VO_HdmiStart(stVoPubAttr.enIntfSync);
		SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_HDMI_START, "SAMPLE_COMM_VO_HdmiStart failed!\n");
		
		s32Ret = SAMPLE_COMM_VO_StartLayer(VoLayer, &stVoLayerAttr);
		SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_LAYER_START, "SAMPLE_COMM_VO_StartLayer failed!\n");
		
		s32Ret = SAMPLE_COMM_VO_StartChn(VoLayer, enSampleVoMode);
		SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_VO_CHN, "SAMPLE_COMM_VO_StartChn failed!\n");
		
    }

	/************************************************
    step 7:  start VENC
    *************************************************/
    if(bEncode)
    {
    	s32Ret = SAMPLE_COMM_VENC_Start(VeH264Chn, enStreamType,enVideoNorm,enPicSize,enRcMode,u32Profile);
		SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_VENC_START, "SAMPLE_COMM_VENC_Start(h264 Venc) failed!\n");
	}

    /******************************************
    step 8: VDEC start send stream. 
    ******************************************/
    SAMPLE_COMM_VDEC_ThreadParam(u32VdecChnCnt, &stVdecSendParam, &stVdecChnAttr, pchStreamName);
    SAMPLE_COMM_VDEC_StartSendStream(u32VdecChnCnt, &stVdecSendParam, &VdecThread);

    /******************************************
    step 9: stream VENC process -- get stream, then save it to file. 
    ******************************************/
    if(bEncode)
    {
    	s32Ret = SAMPLE_COMM_VENC_StartGetStream(u32VencChnCnt);
		SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_VENC_GET_STREAM, "SAMPLE_COMM_VENC_StartGetStream failed!\n"); 
    }
	
    /******************************************
    step 10: GMM process -- get stream, process and send to VO or VENC. 
    ******************************************/
    SAMPLE_IVE_GMM_ThreadParam(&stGmmThreadParam,bEncode,bVo,u32VpssGrpCnt,VPSS_CHN_NUM,
    						  aVpssChn,VeH264Chn,VoLayer,VoChn, &stGmm);
    SAMPLE_IVE_GMM_ThreadProc(&stGmmThreadParam, &IveThread);
   
    /******************************************
    step 11: exit process
    ******************************************/
	SAMPLE_IVE_GMM_CmdCtrl(&stGmmThreadParam, IveThread);
	
END_VENC_GET_STREAM:
    if(bEncode)
    {
    	SAMPLE_COMM_VENC_StopGetStream();
    }
    SAMPLE_COMM_VDEC_StopSendStream(u32VdecChnCnt,&stVdecSendParam,&VdecThread);

	if(bEncode)
	{       
	END_VENC_START:
    	SAMPLE_COMM_VENC_Stop(VeH264Chn);
	}
	
	if(bVo)
	{
	END_VO_CHN: 
    	SAMPLE_COMM_VO_StopChn(VoLayer, enSampleVoMode);
   
	END_LAYER_START: 
    	SAMPLE_COMM_VO_StopLayer(VoLayer);
   
	END_HDMI_START: 
    	SAMPLE_COMM_VO_HdmiStop();
	
	END_VO_START:
    	SAMPLE_COMM_VO_StopDev(VoDev);
	}

END_VPSS_BIND_VDEC:
    for(i=0;i<u32VdecChnCnt;i++)
    {
        SAMPLE_COMM_VDEC_UnBindVpss(i,i);
    }
    
END_VPSS_START:
    SAMPLE_COMM_IVS_VpssStop(u32VpssGrpCnt,VPSS_MAX_CHN_NUM);	
    
END_VDEC_START:
    SAMPLE_COMM_VDEC_Stop(u32VdecChnCnt);
	
END_GMM_INIT:
	SAMPLE_IVE_GMM_Uninit(&stGmm);
	
END_SYS_VB:
    SAMPLE_COMM_SYS_Exit();
    
    return s32Ret;
}


