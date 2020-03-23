/******************************************************************************
  Hisilicon HI3531 sample programs head file.

  Copyright (C), 2010-2021, Hisilicon Tech. Co., Ltd.
 ******************************************************************************
    Modification:  2014-4 Created
******************************************************************************/

#ifndef _SAMPLE_COMM_IVS_H_
#define _SAMPLE_COMM_IVS_H_

#include "hi_comm_ive.h"
#include "hi_comm_vgs.h"

#include "mpi_ive.h"
#include "mpi_vgs.h"
#include "ivs_md.h"

#include "sample_comm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* Begin of #ifdef __cplusplus */

/************************** sample ivs ********************************/

#define VGA_WIDTH   640
#define VGA_HEIGHT  480
#define QVGA_WIDTH  320
#define QVGA_HEIGHT 240


#define SAMPLE_IVS_ALIGN       16

#define SAMPLE_MAX_FACE_NUM    256
#define SAMPLE_MAX_CANDIF_FACE 2048

#define SAMPLE_MAX_VGS_COVER   200

#define SAMPLE_CHECK_EXPR_RET(expr, ret, fmt...)\
do\
{\
	if(expr)\
	{\
		SAMPLE_PRT(fmt);\
		return (ret);\
	}\
}while(0)
#define SAMPLE_CHECK_EXPR_GOTO(expr, label, fmt...)\
do\
{\
	if(expr)\
	{\
		SAMPLE_PRT(fmt);\
		goto label;\
	}\
}while(0)

/*****************************************************************************
* function : set sys conf. 
*****************************************************************************/
HI_VOID	SAMPLE_COMM_IVS_SysConf(VB_CONF_S *pstVbConf, SIZE_S *pstSize, HI_U32 u32Cnt);

/*****************************************************************************
* function : set vpss group attribute. 
*****************************************************************************/
HI_VOID	SAMPLE_COMM_IVS_VpssGrpAttr(HI_U32 u32VpssGrpCnt, VPSS_GRP_ATTR_S *pstVpssGrpAttr, SIZE_S *pstSize);

/*****************************************************************************
* function : start vpss. VPSS chn with frame
*****************************************************************************/
HI_S32 SAMPLE_COMM_IVS_VpssStart(HI_S32 s32VpssGrpCnt, SIZE_S astSize[],
VPSS_CHN aVpssChn[], HI_S32 s32VpssChnCnt, VPSS_GRP_ATTR_S *pstVpssGrpAttr);

/*****************************************************************************
* function : start vpss. VPSS chn with frame
*****************************************************************************/
HI_S32 SAMPLE_COMM_IVS_VpssStart2(HI_S32 s32VpssGrpCnt, SIZE_S astSize[],
VPSS_CHN aVpssChn[], HI_S32 s32VpssChnCnt, VPSS_GRP_ATTR_S *pstVpssGrpAttr);

/*****************************************************************************
* function : stop vpss
*****************************************************************************/
HI_S32 SAMPLE_COMM_IVS_VpssStop(HI_S32 s32VpssGrpCnt, HI_S32 s32VpssChnCnt);

/*****************************************************************************
* function : vgs label obj by frame
*****************************************************************************/
HI_S32 SAMPLE_COMM_IVS_VgsFrmObj(VIDEO_FRAME_INFO_S *pstFrmInfo, VGS_COVER_S *pstVgsCover, HI_U32 u32ObjNum);


/************************** sample ive ********************************/

#define VIDEO_WIDTH 352
#define VIDEO_HEIGHT 288
#define IVE_ALIGN 16
#define IVE_CHAR_CALW 8
#define IVE_CHAR_CALH 8
#define IVE_CHAR_NUM (IVE_CHAR_CALW *IVE_CHAR_CALH)
#define IVE_FILE_NAME_LEN 256

#define CCL_AREA_THR_STEP 8
#define MAX_OBJ_NUM 50

#define SAMPLE_ALIGN_BACK(x, a)     ((a) * (((x) / (a))))

typedef struct hiSAMPLE_IVE_RECT_S
{
	POINT_S astPoint[4];
}SAMPLE_IVE_RECT_S;

typedef struct hiSAMPLE_RECT_ARRAY_S
{
    HI_U16 u16Num;
    SAMPLE_IVE_RECT_S astRect[MAX_OBJ_NUM];
}SAMPLE_RECT_ARRAY_S;

typedef struct hiIVE_LINEAR_DATA_S
{
	HI_S32 s32LinearNum;
	HI_S32 s32ThreshNum;
	POINT_S *pstLinearPoint;
}IVE_LINEAR_DATA_S;


//free mmz 
#define IVE_MMZ_FREE(phy,vir)\
do{\
	if ((0 != (phy)) && (NULL != (vir)))\
	{\
		 HI_MPI_SYS_MmzFree((phy),(vir));\
		 (phy) = 0;\
		 (vir) = NULL;\
	}\
}while(0)

#define IVE_CLOSE_FILE(fp)\
do{\
    if (NULL != (fp))\
    {\
        fclose((fp));\
        (fp) = NULL;\
    }\
}while(0)


/******************************************************************************
* function : Mpi init
******************************************************************************/
HI_VOID SAMPLE_COMM_IVE_CheckIveMpiInit(HI_VOID);
/******************************************************************************
* function : Mpi exit
******************************************************************************/
HI_S32 SAMPLE_COMM_IVE_IveMpiExit(HI_VOID);
/******************************************************************************
* function :VGS Add draw rect job
******************************************************************************/
HI_S32 SAMPLE_COMM_VGS_AddDrawRectJob(VGS_HANDLE VgsHandle, IVE_IMAGE_S *pstSrc, IVE_IMAGE_S *pstDst, 
	RECT_S *pstRect, HI_U16 u16RectNum);
/******************************************************************************
* function : Call vgs to fill rect
******************************************************************************/
HI_S32 SAMPLE_COMM_VGS_FillRect(VIDEO_FRAME_INFO_S *pstFrmInfo, SAMPLE_RECT_ARRAY_S *pstRect,HI_U32 u32Color);
/******************************************************************************
* function :Read file
******************************************************************************/                                                
HI_S32 SAMPLE_COMM_IVE_ReadFile(IVE_IMAGE_S *pstImg, FILE *pFp);
/******************************************************************************
* function :Write file
******************************************************************************/
HI_S32 SAMPLE_COMM_IVE_WriteFile(IVE_IMAGE_S *pstImg, FILE *pFp);
/******************************************************************************
* function :Calc stride
******************************************************************************/
HI_U16 SAMPLE_COMM_IVE_CalcStride(HI_U16 u16Width, HI_U8 u8Align);

/******************************************************************************
* function : Copy blob to rect
******************************************************************************/
HI_VOID SAMPLE_COMM_IVE_BlobToRect(IVE_CCBLOB_S *pstBlob, SAMPLE_RECT_ARRAY_S *pstRect,
                                            HI_U16 u16RectMaxNum,HI_U16 u16AreaThrStep,
                                            HI_FLOAT f32SclX, HI_FLOAT f32SclY);
/******************************************************************************
* function : Create ive image
******************************************************************************/
HI_S32 SAMPLE_COMM_IVE_CreateImage(IVE_IMAGE_S *pstImg,IVE_IMAGE_TYPE_E enType,
			HI_U16 u16Width,HI_U16 u16Height);
/******************************************************************************
* function : Create memory info
******************************************************************************/
HI_S32 SAMPLE_COMM_IVE_CreateMemInfo(IVE_MEM_INFO_S*pstMemInfo,HI_U32 u32Size);
/******************************************************************************
* function : Create ive image by cached
******************************************************************************/
HI_S32 SAMPLE_COMM_IVE_CreateImageByCached(IVE_IMAGE_S *pstImg,
		IVE_IMAGE_TYPE_E enType,HI_U16 u16Width,HI_U16 u16Height);
/******************************************************************************
* function : Dma frame info to  ive image
******************************************************************************/
HI_S32 SAMPLE_COMM_IVE_DmaImage(VIDEO_FRAME_INFO_S *pstFrameInfo,
		IVE_DST_IMAGE_S *pstDst,HI_BOOL bInstant);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif//_SAMPLE_COMM_IVS_H_
