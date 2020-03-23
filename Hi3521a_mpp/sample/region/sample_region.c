/******************************************************************************
  A simple program of Hisilicon HI3531 osd implementation.
  the flow as follows:
    1) init mpp system.
    2) start vi ( internal isp, ViDev 0, 2 vichn)                  
    3) start venc
    4) osd process, you can see video from some H264 streams files. the video will show as follows step:
        4.1) create some cover/osd regions
        4.2) display  cover/osd regions ( One Region -- Multi-VencGroup )
        4.3) change all vencGroups Regions' Layer
        4.4) change all vencGroups Regions' position
        4.5) change all vencGroups Regions' color
        4.6) change all vencGroups Regions' alpha (front and backgroud)
        4.7) load bmp form bmp-file to Region-0
        4.8) change BmpRegion-0
    6) stop venc
    7) stop vi and system.
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
#include <unistd.h>
#include <pthread.h>
#include <signal.h>

#include "loadbmp.h"
#include "sample_comm.h"
#include "hi_tde_api.h"
#include "hi_tde_type.h"

HI_BOOL bExit   = HI_FALSE;
HI_BOOL bTravel = HI_FALSE;
HI_BOOL bShape1 = HI_FALSE;
HI_BOOL bShape2 = HI_FALSE;
HI_BOOL bArea   = HI_FALSE;

pthread_t g_stRgnOsdThread;
pthread_t g_stRgnCoverThread;
pthread_t g_stVdecThread;
pthread_t g_stOsdReverseThread;
pthread_t g_stRgnMosaicThread;
pthread_t g_stVencThread;
pthread_t g_stRgnThread;

static HI_S32   gs_s32RgnCntCur = 0;

#define OSD_REVERSE_RGN_MAXCNT 16
#define MAX_VENC_WORK_CHN_NUM  4

#define SAMPLE_RGN_NOT_PASS(err)\
do {\
	printf("\033[0;31mtest case <%s>not pass at line:%d err:%x\033[0;39m\n",\
		__FUNCTION__,__LINE__,err);\
    exit(-1);\
}while(0)

pthread_mutex_t Rgnmutex_Tmp = PTHREAD_MUTEX_INITIALIZER;

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

typedef struct PTHREAD_VENC
{
    VENC_CHN VeChnId;
    HI_S32 s32FrmCnt;
    FILE  *pstream;
}VENC_PTHREAD_INFO_S;

typedef struct hiRGN_SIZE_S
{
    HI_U32 u32RgnNum;
    SIZE_S stSIZE;
}RGN_SIZE_S;

typedef struct hiRGN_ATTR_INFO_S
{
    RGN_HANDLE Handle;
    HI_U32 u32RgnNum;
}RGN_ATTR_INFO_S;

HI_S32 VDEC_SendEos(VDEC_CHN Vdchn)
{
    return HI_SUCCESS;
}


static HI_S32 SAMPLE_RGN_SYS_Init(HI_VOID)
{
    HI_S32 s32Ret;
    VB_CONF_S struVbConf, stVbConf;
    MPP_SYS_CONF_S struSysConf;

    HI_MPI_SYS_Exit();
    HI_MPI_VB_Exit(); 

    memset(&struVbConf, 0, sizeof(VB_CONF_S));
    
    struVbConf.u32MaxPoolCnt             = 32;
    struVbConf.astCommPool[0].u32BlkSize = 1920*1088*2;
    struVbConf.astCommPool[0].u32BlkCnt  = 20;

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
    
    stVbConf.u32MaxPoolCnt               = 2;

    stVbConf.astCommPool[0].u32BlkSize   = 1920*1088*2;
    stVbConf.astCommPool[0].u32BlkCnt    = 32;

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

HI_S32 SAMPLE_RGN_SYS_Exit(HI_VOID)
{
    HI_MPI_SYS_Exit();
    HI_MPI_VB_ExitModCommPool(VB_UID_VDEC);
    HI_MPI_VB_Exit();

    return HI_SUCCESS;
}

HI_VOID SAMPLE_RGN_Usage(HI_VOID)
{
    printf("press sample command as follows!\n");
    printf("\t 0) VPSS: file->VDEC->VPSS(COVER+OVERLAY)->VO     HDMI VGA 720P60\n");
    printf("\t 1) VPSS: file->VDEC->VPSS(COVEREX+OVERLAYEX+LINE)->VO     HDMI VGA 720P60\n");
	printf("\t 2) VPSS: file->VDEC->VPSS(MOSAIC)->VO     HDMI VGA 720P60\n");
	printf("\t 3)   VO  : file->VDEC->VO(COVER+OVERLAY+LINE)->VO     HDMI VGA 720P60\n");
    printf("\t 4) VENC: file->VDEC->VENC(OVERLAY)->file\n");
    printf("\t q) quit the whole sample\n");
    printf("sample command:");
    return;
}


/******************************************************************************
* function : to process abnormal case                                        
******************************************************************************/
HI_VOID SAMPLE_RGN_HandleSig(HI_S32 signo)
{
    if (SIGINT == signo || SIGTSTP == signo)
    {
        if(HI_FALSE == bExit)
        {
            bExit = HI_TRUE;
    		if(g_stVdecThread)
    		{
                pthread_join(g_stVdecThread, 0);
    		    g_stVdecThread = 0;
    		}
			if(g_stVencThread)
            {
                pthread_join(g_stVencThread, 0);
    			g_stVencThread = 0;
            }
    		if(g_stRgnOsdThread)
    		{
    		    pthread_join(g_stRgnOsdThread, 0);
    			g_stRgnOsdThread = 0;
    		}
            if(g_stRgnCoverThread)
            {
                pthread_join(g_stRgnCoverThread, 0);
    			g_stRgnCoverThread = 0;
            }
            if(g_stOsdReverseThread)
            {
                pthread_join(g_stOsdReverseThread, 0);
    			g_stOsdReverseThread = 0;
            }
    		if(g_stRgnMosaicThread)
            {
                pthread_join(g_stRgnMosaicThread, 0);
    			g_stRgnMosaicThread = 0;
            }
    		if(g_stRgnThread)
            {
                pthread_join(g_stRgnThread, 0);
    			g_stRgnThread = 0;
            }

			HI_MPI_RGN_Destroy(gs_s32RgnCntCur);
            SAMPLE_COMM_SYS_Exit();
            printf("\033[0;31mprogram exit abnormally!\033[0;39m\n");
    		bExit = HI_FALSE;
        }
		else
		{
		    printf("\033[0;31mprogram is exiting!!!\033[0;39m\n");
			return;
		}
    }

    exit(0);
}
    
/******************************************************************************
* funciton : osd region change color
******************************************************************************/
HI_S32 SAMPLE_RGN_ChgColor(RGN_HANDLE RgnHandle, HI_U32 u32Color)
{
    HI_S32 s32Ret;
    RGN_ATTR_S stRgnAttr;

    s32Ret = HI_MPI_RGN_GetAttr(RgnHandle, &stRgnAttr);
    if(HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_RGN_GetAttr (%d)) failed with %#x!\n", RgnHandle, s32Ret);
        return HI_FAILURE;
    }

    stRgnAttr.unAttr.stOverlay.u32BgColor = u32Color;

    s32Ret = HI_MPI_RGN_SetAttr(RgnHandle, &stRgnAttr);
    if(HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_RGN_SetAttr (%d)) failed with %#x!\n", RgnHandle, s32Ret);
        return HI_FAILURE;
    }
    
    return HI_SUCCESS;
}
 
/******************************************************************************
* funciton : load bmp from file
******************************************************************************/
HI_S32 SAMPLE_RGN_LoadBmp(const char *filename, BITMAP_S *pstBitmap, HI_BOOL bFil, HI_U32 u16FilColor)
{
    OSD_SURFACE_S Surface;
    OSD_BITMAPFILEHEADER bmpFileHeader;
    OSD_BITMAPINFO bmpInfo;
    HI_U32 u32BytePerPix = 0;
    
    if(GetBmpInfo(filename, &bmpFileHeader, &bmpInfo) < 0)
    {
        printf("GetBmpInfo err!\n");
        return HI_FAILURE;
    }

    Surface.enColorFmt = OSD_COLOR_FMT_RGB1555;
    u32BytePerPix      = 2;
    
    pstBitmap->pData = malloc(u32BytePerPix * (bmpInfo.bmiHeader.biWidth) * (bmpInfo.bmiHeader.biHeight));
	
    if(NULL == pstBitmap->pData)
    {
        printf("malloc osd memroy err!\n");        
        return HI_FAILURE;
    }
    
    CreateSurfaceByBitMap(filename, &Surface, (HI_U8*)(pstBitmap->pData));
	
    pstBitmap->u32Width      = Surface.u16Width;
    pstBitmap->u32Height     = Surface.u16Height;
    pstBitmap->enPixelFormat = PIXEL_FORMAT_RGB_1555;
    
    
    int i,j;
    HI_U16 *pu16Temp;
    pu16Temp = (HI_U16*)pstBitmap->pData;
    
    if (bFil)
    {
        for (i=0; i<pstBitmap->u32Height; i++)
        {
            for (j=0; j<pstBitmap->u32Width; j++)
            {
                if (u16FilColor == *pu16Temp)
                {
                    *pu16Temp &= 0x7FFF;
                }
                pu16Temp++;
            }
        }
    }
        
    return HI_SUCCESS;
}


HI_S32 SAMPLE_RGN_UpdateCanvas(const char *filename, BITMAP_S *pstBitmap, HI_BOOL bFil, 
    HI_U32 u16FilColor, SIZE_S *pstSize, HI_U32 u32Stride, PIXEL_FORMAT_E enPixelFmt)
{
    OSD_SURFACE_S Surface;
    OSD_BITMAPFILEHEADER bmpFileHeader;
    OSD_BITMAPINFO bmpInfo;

    if(GetBmpInfo(filename, &bmpFileHeader, &bmpInfo) < 0)
    {
        printf("GetBmpInfo err!\n");
        return HI_FAILURE;
    }

    if (PIXEL_FORMAT_RGB_1555 == enPixelFmt)
    {
        Surface.enColorFmt = OSD_COLOR_FMT_RGB1555;
    }
    else if (PIXEL_FORMAT_RGB_4444 == enPixelFmt)
    {
        Surface.enColorFmt = OSD_COLOR_FMT_RGB4444;
    }
    else if (PIXEL_FORMAT_RGB_8888 == enPixelFmt)
    {
        Surface.enColorFmt = OSD_COLOR_FMT_RGB8888;
    }
    else
    {
        printf("Pixel format is not support!\n");        
        return HI_FAILURE;
    }
	
    if(NULL == pstBitmap->pData)
    {
        printf("malloc osd memroy err!\n");        
        return HI_FAILURE;
    }
    CreateSurfaceByCanvas(filename, &Surface, (HI_U8*)(pstBitmap->pData), pstSize->u32Width, pstSize->u32Height, u32Stride);
	
    pstBitmap->u32Width  = Surface.u16Width;
    pstBitmap->u32Height = Surface.u16Height;
    
    if (PIXEL_FORMAT_RGB_1555 == enPixelFmt)
    {
        pstBitmap->enPixelFormat = PIXEL_FORMAT_RGB_1555;
    }
    else if (PIXEL_FORMAT_RGB_4444 == enPixelFmt)
    {
        pstBitmap->enPixelFormat = PIXEL_FORMAT_RGB_4444;
    }
    else if (PIXEL_FORMAT_RGB_8888 == enPixelFmt)
    {
        pstBitmap->enPixelFormat = PIXEL_FORMAT_RGB_8888;
    }

    int i,j;
    HI_U16 *pu16Temp;
    pu16Temp = (HI_U16*)pstBitmap->pData;
    
    if (bFil)
    {
        for (i=0; i<pstBitmap->u32Height; i++)
        {
            for (j=0; j<pstBitmap->u32Width; j++)
            {
                if (u16FilColor == *pu16Temp)
                {
                    *pu16Temp &= 0x7FFF;
                }
                pu16Temp++;
            }
        }
    }
        
    return HI_SUCCESS;
}


HI_S32 SAMPLE_RGN_ConvOsdCavasToTdeSurface(TDE2_SURFACE_S *pstSurface, const RGN_CANVAS_INFO_S *pstCanvasInfo)
{
    HI_ASSERT((NULL != pstSurface) && (NULL != pstCanvasInfo));
    
    switch (pstCanvasInfo->enPixelFmt)
    {
        case PIXEL_FORMAT_RGB_4444:
        {
            pstSurface->enColorFmt = TDE2_COLOR_FMT_ARGB4444;
            break ;
        }
        case PIXEL_FORMAT_RGB_1555:
        {
            pstSurface->enColorFmt = TDE2_COLOR_FMT_ARGB1555;
            break ;
        }
        case PIXEL_FORMAT_RGB_8888:
        {
            pstSurface->enColorFmt = TDE2_COLOR_FMT_ARGB8888;
            break ;
        }
        default :
        {
            printf("[Func]:%s [Line]:%d [Info]:invalid Osd pixel format(%d)\n", 
                __FUNCTION__, __LINE__, pstCanvasInfo->enPixelFmt);
            return HI_FAILURE;
        }
    }

    pstSurface->bAlphaExt1555 = HI_FALSE;
    pstSurface->bAlphaMax255  = HI_TRUE;
    pstSurface->u32PhyAddr    = pstCanvasInfo->u32PhyAddr;
    pstSurface->u32Width      = pstCanvasInfo->stSize.u32Width;
    pstSurface->u32Height     = pstCanvasInfo->stSize.u32Height;
    pstSurface->u32Stride     = pstCanvasInfo->u32Stride;

    return HI_SUCCESS;
}


HI_S32 SAMPLE_RGN_AddReverseColorTask(TDE_HANDLE handle, 
    TDE2_SURFACE_S *pstForeGround, TDE2_RECT_S *pstForeGroundRect, 
    TDE2_SURFACE_S *pstBackGround, TDE2_RECT_S *pstBackGroundRect)
{
    HI_S32 s32Ret;
    TDE2_OPT_S stOpt = {0};
    
    HI_ASSERT(NULL != pstForeGround);
    HI_ASSERT(NULL != pstForeGroundRect);
    HI_ASSERT(NULL != pstBackGround);
    HI_ASSERT(NULL != pstBackGroundRect);

    stOpt.enAluCmd        = TDE2_ALUCMD_ROP;
    stOpt.enRopCode_Alpha = TDE2_ROP_COPYPEN;
    stOpt.enRopCode_Color = TDE2_ROP_NOT;
    
    s32Ret =  HI_TDE2_Bitblit(handle, pstBackGround, pstBackGroundRect, pstForeGround, 
            pstForeGroundRect, pstBackGround, pstBackGroundRect, &stOpt);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_TDE2_Bitblit fail! s32Ret: 0x%x.\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
    
}


HI_S32 SAMPLE_RGN_ReverseOsdColor(TDE2_SURFACE_S *pstSrcSurface, TDE2_SURFACE_S *pstDstSurface, 
    const VPSS_REGION_INFO_S *pstRgnInfo)
{
    HI_S32 i;
    HI_S32 s32Ret;
    TDE_HANDLE handle;
    TDE2_RECT_S stRect;
    

    HI_ASSERT(NULL != pstSrcSurface);
    HI_ASSERT(NULL != pstDstSurface);
    HI_ASSERT(NULL != pstRgnInfo);

    s32Ret = HI_TDE2_Open();
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_TDE2_Open fail! s32Ret: 0x%x.\n", s32Ret);
        return s32Ret;
    }
    
    handle = HI_TDE2_BeginJob();
    if (handle < 0)
    {
        printf("HI_TDE2_BeginJob fail!\n");
        return HI_FAILURE;
    }

    stRect.s32Xpos = 0;
    stRect.s32Ypos = 0;
    stRect.u32Width  = pstSrcSurface->u32Width;
    stRect.u32Height = pstSrcSurface->u32Height;
    s32Ret = HI_TDE2_QuickCopy(handle, pstSrcSurface, &stRect, pstDstSurface, &stRect);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_TDE2_QuickCopy fail! s32Ret: 0x%x.\n", s32Ret);
        HI_TDE2_CancelJob(handle);
        return s32Ret;
    }
    
    for (i = 0; i < pstRgnInfo->u32RegionNum; ++i)
    {
        stRect.s32Xpos   = pstRgnInfo->pstRegion[i].s32X;
        stRect.s32Ypos   = pstRgnInfo->pstRegion[i].s32Y;
        stRect.u32Width  = pstRgnInfo->pstRegion[i].u32Width;
        stRect.u32Height = pstRgnInfo->pstRegion[i].u32Height;
        
        s32Ret = SAMPLE_RGN_AddReverseColorTask(handle, pstSrcSurface, &stRect, pstDstSurface, &stRect);
        if (HI_SUCCESS != s32Ret)
        {
            printf("SAMPLE_RGN_AddReverseColorTask fail! s32Ret: 0x%x.\n", s32Ret);
            HI_TDE2_CancelJob(handle);
            return s32Ret;
        }
    }
    
    s32Ret = HI_TDE2_EndJob(handle, HI_FALSE, HI_FALSE, 10);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_TDE2_EndJob fail! s32Ret: 0x%x.\n", s32Ret);
        HI_TDE2_CancelJob(handle);
        return s32Ret;
    }
    
    s32Ret = HI_TDE2_WaitForDone(handle);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_TDE2_WaitForDone fail! s32Ret: 0x%x.\n", s32Ret);
        return s32Ret;
    }
     
    return HI_SUCCESS;
}


HI_S32 SAMPLE_RGN_GetVoDisplayNum(HI_U32 u32VoChnNum)
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


HI_S32 SAMPLE_RGN_GetVoLayer(VO_DEV VoDev)
{
    HI_S32 s32LayerNum;
    
    if (SAMPLE_VO_DEV_DHD0 == VoDev)
    {
        s32LayerNum = SAMPLE_VO_LAYER_VHD0;
    }
    else if (SAMPLE_VO_DEV_DSD0 == VoDev)
    {
        s32LayerNum = SAMPLE_VO_LAYER_VSD0;
    }
    else
    {
        return -1;
    }

    return s32LayerNum;
}

HI_S32 SAMPLE_RGN_CreateCover(RGN_HANDLE Handle, HI_U32 u32Num)
{
    HI_S32 i;
    HI_S32 s32Ret;
    MPP_CHN_S stChn;
    RGN_ATTR_S stRgnAttr;
    RGN_CHN_ATTR_S stChnAttr;
    
    /* Add cover to vpss group */
    stChn.enModId  = HI_ID_VPSS;
    stChn.s32DevId = 0;
    stChn.s32ChnId = 0;
        
    /* Create cover and attach to vpss group */
    for (i=Handle; i<(Handle + u32Num); i++)
    {
        stRgnAttr.enType = COVER_RGN;

        s32Ret = HI_MPI_RGN_Create(i, &stRgnAttr);
        if(s32Ret != HI_SUCCESS)
        {
            printf("HI_MPI_RGN_Create fail! s32Ret: 0x%x.\n", s32Ret);
            return s32Ret;
        }

        stChnAttr.bShow  = HI_TRUE;
        stChnAttr.enType = COVER_RGN;
        stChnAttr.unChnAttr.stCoverChn.enCoverType = AREA_RECT;
        stChnAttr.unChnAttr.stCoverChn.stRect.s32X      = 200+60 * (i - Handle);
        stChnAttr.unChnAttr.stCoverChn.stRect.s32Y      = 200+60 * (i - Handle);
        stChnAttr.unChnAttr.stCoverChn.stRect.u32Height = 100 ;
        stChnAttr.unChnAttr.stCoverChn.stRect.u32Width  = 100 ;//* (i - Handle + 1);
        stChnAttr.unChnAttr.stCoverChn.u32Color         = 0x000000ff;
        if (1 == i%COVER_MAX_NUM_VPSS)
        {
            stChnAttr.unChnAttr.stCoverChn.u32Color     = 0x0000ff00;
        }
        else if (2 == i%COVER_MAX_NUM_VPSS)
        {
            stChnAttr.unChnAttr.stCoverChn.u32Color     = 0x00ff0000;
        }
        else if (3 == i%COVER_MAX_NUM_VPSS)
        {
            stChnAttr.unChnAttr.stCoverChn.u32Color     = 0xff000000;
        }
        stChnAttr.unChnAttr.stCoverChn.u32Layer         = i - Handle;
        
        s32Ret = HI_MPI_RGN_AttachToChn(i, &stChn, &stChnAttr);
        if(s32Ret != HI_SUCCESS)
        {
            printf("HI_MPI_RGN_AttachToChn fail! s32Ret: 0x%x.\n", s32Ret);
            return s32Ret;
        }
    }

    return HI_SUCCESS;
    
}


HI_S32 SAMPLE_RGN_CreateOverlayForVpss(RGN_HANDLE Handle, HI_U32 u32Num)
{
    HI_S32 i;
    HI_S32 s32Ret;
    MPP_CHN_S stChn;
    RGN_ATTR_S stRgnAttrSet;
    RGN_CHN_ATTR_S stChnAttr;

    /*attach the OSD to the vpss*/
    stChn.enModId  = HI_ID_VPSS;
    stChn.s32DevId = 0;
    stChn.s32ChnId = 0;
        
    for (i=Handle; i<(Handle + u32Num); i++)    
    {
        stRgnAttrSet.enType = OVERLAY_RGN;
        stRgnAttrSet.unAttr.stOverlay.enPixelFmt       = PIXEL_FORMAT_RGB_1555;
        stRgnAttrSet.unAttr.stOverlay.stSize.u32Width  = 100;
        stRgnAttrSet.unAttr.stOverlay.stSize.u32Height = 100;
        stRgnAttrSet.unAttr.stOverlay.u32BgColor       = 0x000003e0;
        
        if (1 == i%u32Num)
        {
            stRgnAttrSet.unAttr.stOverlay.stSize.u32Width  = 100;
            stRgnAttrSet.unAttr.stOverlay.stSize.u32Height = 100;
            stRgnAttrSet.unAttr.stOverlay.u32BgColor       = 0x0000001f; 
        }
        else if (2 == i%u32Num)
        {
            stRgnAttrSet.unAttr.stOverlay.stSize.u32Width  = 100;
            stRgnAttrSet.unAttr.stOverlay.stSize.u32Height = 100;
            stRgnAttrSet.unAttr.stOverlay.u32BgColor       = 0x00007c00;     
        }
        else if (3 == i%u32Num)
        {
            stRgnAttrSet.unAttr.stOverlay.stSize.u32Width  = 120;
            stRgnAttrSet.unAttr.stOverlay.stSize.u32Height = 120;
            stRgnAttrSet.unAttr.stOverlay.u32BgColor       = 0x000007ff;   
        }

        
        s32Ret = HI_MPI_RGN_Create(i, &stRgnAttrSet);
        if(s32Ret != HI_SUCCESS)
        {
            printf("HI_MPI_RGN_Create failed! s32Ret: 0x%x.\n", s32Ret);
            return s32Ret;
        }

        stChnAttr.bShow  = HI_TRUE;
        stChnAttr.enType = OVERLAY_RGN;
        stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = 48;
        stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = 48;
        stChnAttr.unChnAttr.stOverlayChn.u32BgAlpha   = 255;
        stChnAttr.unChnAttr.stOverlayChn.u32FgAlpha   = 255;
        stChnAttr.unChnAttr.stOverlayChn.u32Layer     = i;
        if (1 == i%4)
        {
            stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = 130;
            stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = 330;
        }
        else if (2 == i%4)
        {
            stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = 270;
            stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = 300;
        }
        else if (3 == i%4)
        {
            stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = 180;
            stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = 400;
        }
        s32Ret = HI_MPI_RGN_AttachToChn(i, &stChn, &stChnAttr);
        if(s32Ret != HI_SUCCESS)
        {
            printf("HI_MPI_RGN_AttachToChn failed! s32Ret: 0x%x.\n", s32Ret);
            return s32Ret;
        }
    }
    
    return HI_SUCCESS;
    
}

HI_S32 SAMPLE_RGN_CreateMosaicForVpss(RGN_HANDLE Handle, HI_U32 u32Num)
{
    HI_S32 i;
    HI_S32 s32Ret;
    MPP_CHN_S stChn;
    RGN_ATTR_S stRgnAttr;
    RGN_CHN_ATTR_S stChnAttr;
    
    /* Add Mosaic to vpss group */
    stChn.enModId  = HI_ID_VPSS;
    stChn.s32DevId = 0;
    stChn.s32ChnId = 0;
        
    /* Create Mosaic and attach to vpss group */
    for (i=Handle; i<(Handle + u32Num); i++)
    {
        stRgnAttr.enType = MOSAIC_RGN;

        s32Ret = HI_MPI_RGN_Create(i, &stRgnAttr);
        if(s32Ret != HI_SUCCESS)
        {
            printf("HI_MPI_RGN_Create fail! s32Ret: 0x%x.\n", s32Ret);
            return s32Ret;
        }

        stChnAttr.bShow  = HI_TRUE;
        stChnAttr.enType = MOSAIC_RGN;

		if ((i - Handle) == 0)
		{
			stChnAttr.unChnAttr.stMosaicChn.enBlkSize = MOSAIC_BLK_SIZE_8;
		}
		else if ((i - Handle) == 1)
		{
			stChnAttr.unChnAttr.stMosaicChn.enBlkSize = MOSAIC_BLK_SIZE_16;
		}
		else if ((i - Handle) == 2)
		{
			stChnAttr.unChnAttr.stMosaicChn.enBlkSize = MOSAIC_BLK_SIZE_32;
		}
		else
		{
			stChnAttr.unChnAttr.stMosaicChn.enBlkSize = MOSAIC_BLK_SIZE_32;
		}
		
        stChnAttr.unChnAttr.stMosaicChn.stRect.s32X      = 96 * (i - Handle);
        stChnAttr.unChnAttr.stMosaicChn.stRect.s32Y      = 96 * (i - Handle);
        stChnAttr.unChnAttr.stMosaicChn.stRect.u32Height = 64 ;
        stChnAttr.unChnAttr.stMosaicChn.stRect.u32Width  = 64 ;//* (i - Handle + 1);
        stChnAttr.unChnAttr.stMosaicChn.u32Layer         = i - Handle;
        
        s32Ret = HI_MPI_RGN_AttachToChn(i, &stChn, &stChnAttr);
        if(s32Ret != HI_SUCCESS)
        {
            printf("HI_MPI_RGN_AttachToChn fail! s32Ret: 0x%x.\n", s32Ret);
            return s32Ret;
        }
    }

    return HI_SUCCESS;
    
}


HI_S32 SAMPLE_RGN_CreateOverlayForVenc(RGN_HANDLE Handle, HI_U32 u32Num)
{
    HI_S32 i;
    HI_S32 s32Ret;
    MPP_CHN_S stChn;
    RGN_ATTR_S stRgnAttr;
    RGN_CHN_ATTR_S stChnAttr;
    
    /* Add cover to vpss group */
    stChn.enModId  = HI_ID_VENC;
    stChn.s32DevId = 0;
    stChn.s32ChnId = 0;
        
    for (i=Handle; i<(Handle + u32Num); i++)
    {
        stRgnAttr.enType = OVERLAY_RGN;
        stRgnAttr.unAttr.stOverlay.enPixelFmt       = PIXEL_FORMAT_RGB_1555;
        stRgnAttr.unAttr.stOverlay.stSize.u32Width  = 128;
        stRgnAttr.unAttr.stOverlay.stSize.u32Height = 128;
        stRgnAttr.unAttr.stOverlay.u32BgColor       = 0x00007c00;

        s32Ret = HI_MPI_RGN_Create(i, &stRgnAttr);
        if(s32Ret != HI_SUCCESS)
        {
            SAMPLE_RGN_NOT_PASS(s32Ret);
        }
    
        stChnAttr.bShow  = HI_TRUE;
        stChnAttr.enType = OVERLAY_RGN;
        stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = 80*i;
        stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = 80*i;
        stChnAttr.unChnAttr.stOverlayChn.u32BgAlpha   = 128;
        stChnAttr.unChnAttr.stOverlayChn.u32FgAlpha   = 128;
        stChnAttr.unChnAttr.stOverlayChn.u32Layer     = i;
        
        stChnAttr.unChnAttr.stOverlayChn.stQpInfo.bAbsQp = HI_FALSE;
        stChnAttr.unChnAttr.stOverlayChn.stQpInfo.s32Qp  = 0;

        stChnAttr.unChnAttr.stOverlayChn.stInvertColor.stInvColArea.u32Height = 16*(i%2+1);
        stChnAttr.unChnAttr.stOverlayChn.stInvertColor.stInvColArea.u32Width  = 16*(i%2+1);
        stChnAttr.unChnAttr.stOverlayChn.stInvertColor.u32LumThresh = 128;
        stChnAttr.unChnAttr.stOverlayChn.stInvertColor.enChgMod     = LESSTHAN_LUM_THRESH;
        stChnAttr.unChnAttr.stOverlayChn.stInvertColor.bInvColEn    = HI_FALSE;

        s32Ret = HI_MPI_RGN_AttachToChn(i, &stChn, &stChnAttr);
        if(s32Ret != HI_SUCCESS)
        {
            SAMPLE_RGN_NOT_PASS(s32Ret);
        }
    } 

    return HI_SUCCESS;
    
}

HI_S32 SAMPLE_RGN_CreateOverlayex(RGN_HANDLE Handle, HI_U32  ChnId,HI_U32 u32Num,MOD_ID_E u32ModId)
{
    HI_S32 i;
    HI_S32 s32Ret;
    MPP_CHN_S stChn;
    RGN_ATTR_S stRgnAttr;
    RGN_CHN_ATTR_S stChnAttr;
    
    /* Add cover to vpss group */
    stChn.enModId  = u32ModId;
    stChn.s32DevId = 0;
    stChn.s32ChnId = ChnId;
        
    for (i=Handle; i<(Handle + u32Num); i++)
    {
        stRgnAttr.enType = OVERLAYEX_RGN;
        stRgnAttr.unAttr.stOverlayEx.enPixelFmt       = PIXEL_FORMAT_RGB_1555;
        stRgnAttr.unAttr.stOverlayEx.stSize.u32Width  = 128;
        stRgnAttr.unAttr.stOverlayEx.stSize.u32Height = 128;
        stRgnAttr.unAttr.stOverlayEx.u32BgColor       = 0x00007c00;

        s32Ret = HI_MPI_RGN_Create(i, &stRgnAttr);
        if(s32Ret != HI_SUCCESS)
        {
            SAMPLE_RGN_NOT_PASS(s32Ret);
        }
    
        stChnAttr.bShow  = HI_TRUE;
        stChnAttr.enType = OVERLAYEX_RGN;
        stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32X = 80*i;
        stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32Y = 80*i;
        stChnAttr.unChnAttr.stOverlayExChn.u32BgAlpha   = 128;
        stChnAttr.unChnAttr.stOverlayExChn.u32FgAlpha   = 128;
        stChnAttr.unChnAttr.stOverlayExChn.u32Layer     = i;
#if 0        
        stChnAttr.unChnAttr.stOverlayExChn.stQpInfo.bAbsQp = HI_FALSE;
        stChnAttr.unChnAttr.stOverlayExChn.stQpInfo.s32Qp  = 0;

        stChnAttr.unChnAttr.stOverlayExChn.stInvertColor.stInvColArea.u32Height = 16*(i%2+1);
        stChnAttr.unChnAttr.stOverlayExChn.stInvertColor.stInvColArea.u32Width  = 16*(i%2+1);
        stChnAttr.unChnAttr.stOverlayExChn.stInvertColor.u32LumThresh = 128;
        stChnAttr.unChnAttr.stOverlayExChn.stInvertColor.enChgMod     = LESSTHAN_LUM_THRESH;
        stChnAttr.unChnAttr.stOverlayExChn.stInvertColor.bInvColEn    = HI_TRUE;
        if (i%2)
        {
            stChnAttr.unChnAttr.stOverlayExChn.stInvertColor.bInvColEn = HI_FALSE;
        }
#endif
        s32Ret = HI_MPI_RGN_AttachToChn(i, &stChn, &stChnAttr);
        if(s32Ret != HI_SUCCESS)
        {
            SAMPLE_RGN_NOT_PASS(s32Ret);
        }
    } 

    return HI_SUCCESS;
    
}


HI_S32 SAMPLE_RGN_CreateCoverex(RGN_HANDLE Handle, HI_U32  ChnId,HI_U32 u32Num,MOD_ID_E u32ModId)
{
    HI_S32 i;
    HI_S32 s32Ret;
    MPP_CHN_S stChn;
    RGN_ATTR_S stRgnAttr;
    RGN_CHN_ATTR_S stChnAttr;
    
    /* Add cover to vpss group */
    stChn.enModId  = u32ModId;
    stChn.s32DevId = 0;
    stChn.s32ChnId = ChnId;

    /* Create cover and attach to vpss group */
    for (i=Handle; i<(Handle + u32Num); i++)
    {
        stRgnAttr.enType = COVEREX_RGN;

        s32Ret = HI_MPI_RGN_Create(i, &stRgnAttr);
        if(s32Ret != HI_SUCCESS)
        {
            printf("HI_MPI_RGN_Create fail! s32Ret: 0x%x.\n", s32Ret);
            return s32Ret;
        }

        stChnAttr.bShow  = HI_TRUE;
        stChnAttr.enType = COVEREX_RGN;
        stChnAttr.unChnAttr.stCoverExChn.stRect.s32X      = 60 * (i - Handle)+500;
        stChnAttr.unChnAttr.stCoverExChn.stRect.s32Y      = 60 * (i - Handle);
        stChnAttr.unChnAttr.stCoverExChn.stRect.u32Height = 64 * (i - Handle + 1);
        stChnAttr.unChnAttr.stCoverExChn.stRect.u32Width  = 64 * (i - Handle + 1);
        stChnAttr.unChnAttr.stCoverExChn.u32Color         = 0x0000ff;
        if (1 == i%COVER_MAX_NUM_VPSS)
        {
            stChnAttr.unChnAttr.stCoverExChn.u32Color     = 0x00ff00;
        }
        else if (2 == i%COVER_MAX_NUM_VPSS)
        {
            stChnAttr.unChnAttr.stCoverExChn.u32Color     = 0x00ff00;
        }
        else if (3 == i%COVER_MAX_NUM_VPSS)
        {
            stChnAttr.unChnAttr.stCoverExChn.u32Color     = 0xff0000;
        }
        stChnAttr.unChnAttr.stCoverExChn.u32Layer         = i - Handle;
        stChnAttr.unChnAttr.stCoverExChn.enCoverType = AREA_RECT;
        
        s32Ret = HI_MPI_RGN_AttachToChn(i, &stChn, &stChnAttr);
        if(s32Ret != HI_SUCCESS)
        {
            printf("HI_MPI_RGN_AttachToChn fail! s32Ret: 0x%x.\n", s32Ret);
            return s32Ret;
        }
    }

    return HI_SUCCESS;
    
}

HI_S32 SAMPLE_RGN_CreateLine(RGN_HANDLE Handle, HI_U32  ChnId,HI_U32 u32Num,MOD_ID_E u32ModId)
{
    HI_S32 i;
    HI_S32 s32Ret;
    MPP_CHN_S stChn;
    RGN_ATTR_S stRgnAttr;
    RGN_CHN_ATTR_S stChnAttr;
    
    /* Add cover to vpss group */
    stChn.enModId  = u32ModId;
    stChn.s32DevId = 0;
    stChn.s32ChnId = ChnId;

    /* Create cover and attach to vpss group */
    for (i=Handle; i<(Handle + u32Num); i++)
    {
        stRgnAttr.enType = LINE_RGN;

        s32Ret = HI_MPI_RGN_Create(i, &stRgnAttr);
        if(s32Ret != HI_SUCCESS)
        {
            printf("HI_MPI_RGN_Create fail! s32Ret: 0x%x.\n", s32Ret);
            return s32Ret;
        }

        stChnAttr.bShow  = HI_TRUE;
        stChnAttr.enType = LINE_RGN;
        stChnAttr.unChnAttr.stLineChn.u32LineWidth = 6;
        stChnAttr.unChnAttr.stLineChn.u32LineColor = 0x000000ff;
        stChnAttr.unChnAttr.stLineChn.stLinePoints[0].s32X = 300 * (i - Handle);
        stChnAttr.unChnAttr.stLineChn.stLinePoints[0].s32Y= 120 * (i - Handle);
        stChnAttr.unChnAttr.stLineChn.stLinePoints[1].s32X = 300 * (i - Handle);
        stChnAttr.unChnAttr.stLineChn.stLinePoints[1].s32Y= 120* (i - Handle)+300;
        
        s32Ret = HI_MPI_RGN_AttachToChn(i, &stChn, &stChnAttr);
        if(s32Ret != HI_SUCCESS)
        {
            printf("HI_MPI_RGN_AttachToChn fail! s32Ret: 0x%x.\n", s32Ret);
            return s32Ret;
        }
    }

    return HI_SUCCESS;
    
}

HI_S32 SAMPLE_RGN_GetVoAttr(VO_DEV VoDev, VO_INTF_SYNC_E enIntfSync, VO_PUB_ATTR_S *pstPubAttr,
        VO_VIDEO_LAYER_ATTR_S *pstLayerAttr, HI_S32 s32SquareSort, VO_CHN_ATTR_S *astChnAttr)
{    
    VO_INTF_TYPE_E enIntfType;
    HI_U32 u32Frmt, u32Width, u32Height, j;
 
    switch (VoDev)
    {
        case 0: enIntfType = VO_INTF_VGA | VO_INTF_HDMI; break;
        case 1: enIntfType = VO_INTF_BT1120; break;
        case 2: enIntfType = VO_INTF_CVBS; break;
        case 3: enIntfType = VO_INTF_CVBS; break;
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


HI_S32 SAMPLE_RGN_StartVpss(HI_S32 s32VpssGrpNum, HI_U32 u32VpssChn)
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


HI_S32 SAMPLE_RGN_StartVpssHD(HI_S32 s32VpssGrpNum, HI_U32 u32VpssChn)
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
    stGrpAttr.enDieMode    = VPSS_DIE_MODE_NODIE;
    stGrpAttr.bIeEn        = HI_FALSE;
    stGrpAttr.bNrEn        = HI_FALSE;
    stGrpAttr.bHistEn      = HI_FALSE;
    stGrpAttr.bDciEn       = HI_FALSE;
    stGrpAttr.bEsEn = HI_FALSE;
    
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


HI_S32 SAMPLE_RGN_StopVpss(HI_S32 s32VpssGrpNum)
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

HI_S32 SAMPLE_RGN_StartVo(VO_DEV VoDev, HI_U32 u32VoChnNum, VO_INTF_SYNC_E enIntfSync)
{
    HI_S32 i;
    HI_S32 s32Ret;
    HI_S32 s32DispNum;
    VO_LAYER VoLayer;
    VO_PUB_ATTR_S stPubAttr;
    VO_VIDEO_LAYER_ATTR_S stLayerAttr;
    VO_CHN_ATTR_S astChnAttr[16];

    s32DispNum = SAMPLE_RGN_GetVoDisplayNum(u32VoChnNum);
    if(s32DispNum < 0)
    {
        printf("SAMPLE_RGN_GetVoDisplayNum failed! u32VoChnNum: %d.\n", u32VoChnNum);
        return HI_FAILURE;
    }
    
    s32Ret = SAMPLE_RGN_GetVoAttr(VoDev, enIntfSync, &stPubAttr, &stLayerAttr, s32DispNum, astChnAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("SAMPLE_RGN_GetVoAttr failed!\n");
        return HI_FAILURE;
    }

    VoLayer = SAMPLE_RGN_GetVoLayer(VoDev);
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

HI_S32 SAMPLE_RGN_StopVoChn(VO_DEV VoDev, HI_U32 u32VoChnNum)
{
    HI_S32 i;
    HI_S32 s32Ret;
    VO_LAYER VoLayer;
    
    VoLayer = SAMPLE_RGN_GetVoLayer(VoDev);
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


HI_S32 SAMPLE_RGN_StopVoDev(VO_DEV VoDev)
{
    HI_S32 s32Ret;
    VO_LAYER VoLayer;

    VoLayer = SAMPLE_RGN_GetVoLayer(VoDev);
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

HI_S32 SAMPLE_RGN_StartVdec(VDEC_CHN VdecChn)
{
    HI_S32 s32Ret;
    VDEC_CHN_ATTR_S stVdecAttr;
    
    stVdecAttr.enType       = PT_H264;
    stVdecAttr.u32Priority  = 1;  /*u32Priority must be larger than 0*/
    stVdecAttr.u32PicWidth  = 1920;
    stVdecAttr.u32PicHeight = 1080;
    stVdecAttr.u32BufSize   = stVdecAttr.u32PicWidth * stVdecAttr.u32PicHeight;//This item should larger than u32Width*u32Height/2
    stVdecAttr.stVdecVideoAttr.u32RefFrameNum   = 1;
    //stVdecAttr.stVdecVideoAttr.s32SupportBFrame = 0;
    stVdecAttr.stVdecVideoAttr.bTemporalMvpEnable = 0;
    stVdecAttr.stVdecVideoAttr.enMode           = VIDEO_MODE_FRAME;

    /* create vdec chn*/
    s32Ret = HI_MPI_VDEC_CreateChn(VdecChn, &stVdecAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_VDEC_CreateChn failed! s32Ret:0x%x.\n", s32Ret);
        return s32Ret;
    }

    /* start vdec chn to receive stream sent by user*/
    s32Ret = HI_MPI_VDEC_StartRecvStream(VdecChn);    
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_VDEC_StartRecvStream failed! s32Ret:0x%x.\n", s32Ret);
        return s32Ret;
    }
    
    printf("Vdec chn create and start receive stream ok!\n");

    return HI_SUCCESS;
}

HI_S32 SAMPLE_RGN_StopVdec(VDEC_CHN VdecChn)
{
    HI_S32 s32Ret;
    
    /* stop vdec chn to receive stream sent by user*/
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


HI_S32 SAMPLE_RGN_StartVenc(VENC_CHN VencChn)
{
    HI_S32 s32Ret;
    HI_U32 u32PicWidth;
    HI_U32 u32PicHeight;
    VENC_CHN_ATTR_S stChnAttr;

    u32PicWidth  = 720;
    u32PicHeight = 576;
    stChnAttr.stVeAttr.enType               = PT_H264;
    stChnAttr.stVeAttr.stAttrH264e.bByFrame = HI_TRUE;
    
    stChnAttr.stVeAttr.stAttrH264e.u32MaxPicHeight = u32PicHeight;
    stChnAttr.stVeAttr.stAttrH264e.u32MaxPicWidth  = u32PicWidth;
    stChnAttr.stVeAttr.stAttrH264e.u32PicHeight    = u32PicHeight;
    stChnAttr.stVeAttr.stAttrH264e.u32PicWidth     = u32PicWidth;
    stChnAttr.stVeAttr.stAttrH264e.u32BufSize      = u32PicWidth * u32PicHeight * 2;
    stChnAttr.stVeAttr.stAttrH264e.u32Profile      = 0;

    stChnAttr.stRcAttr.enRcMode                        = VENC_RC_MODE_H264CBR;
    stChnAttr.stRcAttr.stAttrH264Cbr.fr32DstFrmRate    = 25;
    stChnAttr.stRcAttr.stAttrH264Cbr.u32BitRate        = 1024 * 2;
    stChnAttr.stRcAttr.stAttrH264Cbr.u32FluctuateLevel = 0;
    stChnAttr.stRcAttr.stAttrH264Cbr.u32Gop            = 25;
    stChnAttr.stRcAttr.stAttrH264Cbr.u32SrcFrmRate     = 25;
    stChnAttr.stRcAttr.stAttrH264Cbr.u32StatTime       = 1;
    
    s32Ret = HI_MPI_VENC_CreateChn(VencChn, &stChnAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_VENC_CreateChn error, s32Ret: 0x%x.\n", s32Ret);
        return s32Ret;
    }
    
    s32Ret = HI_MPI_VENC_StartRecvPic(VencChn);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_VENC_StartRecvPic error, s32Ret: 0x%x.\n", s32Ret);
        return s32Ret;
    }

    printf("Venc chn create ok, and start receive picture!\n");
    
    return HI_SUCCESS;
}

HI_S32 SAMPLE_RGN_StopVenc(VENC_CHN VencChn)
{
    HI_S32 s32Ret;

    s32Ret = HI_MPI_VENC_StopRecvPic(VencChn);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_VENC_StopRecvPic error, s32Ret: 0x%x.\n", s32Ret);
        return s32Ret;
    }

    s32Ret = HI_MPI_VENC_DestroyChn(VencChn);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_VENC_DestroyChn error, s32Ret: 0x%x.\n", s32Ret);
        return s32Ret;
    }

    printf("Venc chn stop receive picture and destroy ok!\n");
    
    return HI_SUCCESS;
}


/******************************************************************************
 * function : send stream to vdec
 ******************************************************************************/
void* SAMPLE_RGN_VpssCoverDynamicDisplay(void* p)
{
    HI_S32 s32Ret;
    HI_U32 u32RgnNum;
    RGN_HANDLE Handle;
    RGN_HANDLE startHandle;
    MPP_CHN_S stChn;
    RGN_CHN_ATTR_S stChnAttr;
    RGN_ATTR_INFO_S *pstRgnAttrInfo = NULL;
    
    stChn.enModId  = HI_ID_VPSS;
    stChn.s32DevId = 0;
    stChn.s32ChnId = 0;

    pstRgnAttrInfo = (RGN_ATTR_INFO_S *)p;
    startHandle    = pstRgnAttrInfo->Handle;
    u32RgnNum      = pstRgnAttrInfo->u32RgnNum;

    if (u32RgnNum > COVER_MAX_NUM_VPSS)
    {
        printf("cover num(%d) is bigger than COVER_MAX_NUM_VPSS(%d)..\n", u32RgnNum, COVER_MAX_NUM_VPSS);
        return NULL;
    }
    
    while (HI_FALSE == bExit)
    {
        for (Handle=startHandle; Handle<(startHandle+u32RgnNum); Handle++)
        {
            stChnAttr.bShow  = HI_TRUE;
            stChnAttr.enType = COVER_RGN;
            stChnAttr.unChnAttr.stCoverChn.stRect.s32X      = 200+32*(Handle - startHandle);
            stChnAttr.unChnAttr.stCoverChn.stRect.s32Y      = 32*(Handle - startHandle);
            stChnAttr.unChnAttr.stCoverChn.stRect.u32Height = 150;
            stChnAttr.unChnAttr.stCoverChn.stRect.u32Width  = 150;
            stChnAttr.unChnAttr.stCoverChn.u32Color         = 0xffff;
            if (Handle%2)
            {
                stChnAttr.unChnAttr.stCoverChn.u32Color = 0xff0000;
            }
            stChnAttr.unChnAttr.stCoverChn.enCoverType = AREA_RECT;
            stChnAttr.unChnAttr.stCoverChn.u32Layer     = Handle - startHandle; 

            s32Ret = HI_MPI_RGN_SetDisplayAttr(Handle, &stChn, &stChnAttr);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_RGN_NOT_PASS(s32Ret);
            }
        }   
        sleep(3);

        for (Handle=startHandle; Handle<(startHandle+u32RgnNum); Handle++)
        {    
            stChnAttr.unChnAttr.stCoverChn.stRect.s32X      = 260+32*(Handle - startHandle);
            stChnAttr.unChnAttr.stCoverChn.stRect.s32Y      = 52*(Handle - startHandle);
            stChnAttr.unChnAttr.stCoverChn.stRect.u32Height = 120;
            stChnAttr.unChnAttr.stCoverChn.stRect.u32Width  = 120;
            stChnAttr.unChnAttr.stCoverChn.u32Color         = 0xffff;
            if (Handle%2)
            {
                stChnAttr.unChnAttr.stCoverChn.u32Color = 0xff0000;
            }
            stChnAttr.unChnAttr.stCoverChn.enCoverType = AREA_RECT;
            stChnAttr.unChnAttr.stCoverChn.u32Layer     = Handle - startHandle;
            
            s32Ret = HI_MPI_RGN_SetDisplayAttr(Handle, &stChn, &stChnAttr);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_RGN_NOT_PASS(s32Ret);
            }
        }
        sleep(3);

        for (Handle=startHandle; Handle<(startHandle+u32RgnNum); Handle++)
        {
            stChnAttr.unChnAttr.stCoverChn.stRect.s32X      = 200+32*(Handle - startHandle);
            stChnAttr.unChnAttr.stCoverChn.stRect.s32Y      = 72*(Handle - startHandle);
            stChnAttr.unChnAttr.stCoverChn.stRect.u32Height = 100;
            stChnAttr.unChnAttr.stCoverChn.stRect.u32Width  = 100;
            stChnAttr.unChnAttr.stCoverChn.u32Color         = 0x000000ff;
            if (1 == Handle)
            {
                stChnAttr.unChnAttr.stCoverChn.u32Color  = 0x0000ff00;
            }
            else if (2 == Handle)
            {
                stChnAttr.unChnAttr.stCoverChn.u32Color  = 0x00ff0000;
            }
            else if (3 == Handle)
            {
                stChnAttr.unChnAttr.stCoverChn.u32Color  = 0xff000000;
            }
            stChnAttr.unChnAttr.stCoverChn.enCoverType = AREA_RECT;
            stChnAttr.unChnAttr.stCoverChn.u32Layer      = Handle - startHandle;
            
            s32Ret = HI_MPI_RGN_SetDisplayAttr(Handle, &stChn, &stChnAttr);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_RGN_NOT_PASS(s32Ret);
            }
        }
        sleep(3);

        for (Handle=startHandle; Handle<(startHandle+u32RgnNum); Handle++)
        {    
            stChnAttr.unChnAttr.stCoverChn.stRect.s32X      = 200+32*(Handle - startHandle);
            stChnAttr.unChnAttr.stCoverChn.stRect.s32Y      = 92*(Handle - startHandle);
            stChnAttr.unChnAttr.stCoverChn.stRect.u32Height = 80;
            stChnAttr.unChnAttr.stCoverChn.stRect.u32Width  = 80;
            stChnAttr.unChnAttr.stCoverChn.u32Color         = 0x000000ff;
            if (1 == Handle)
            {
                stChnAttr.unChnAttr.stCoverChn.u32Color  = 0x0000ff00;
            }
            else if (2 == Handle)
            {
                stChnAttr.unChnAttr.stCoverChn.u32Color  = 0x00ff0000;
            }
            else if (3 == Handle)
            {
                stChnAttr.unChnAttr.stCoverChn.u32Color  = 0xff000000;
            }
            stChnAttr.unChnAttr.stCoverChn.enCoverType = AREA_RECT;
            stChnAttr.unChnAttr.stCoverChn.u32Layer      = Handle - startHandle;
            
            s32Ret = HI_MPI_RGN_SetDisplayAttr(Handle, &stChn, &stChnAttr);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_RGN_NOT_PASS(s32Ret);
            }
        }
        
        sleep(2);
    }

    return (HI_VOID *)HI_SUCCESS;
}





/******************************************************************************
 * function : send stream to vdec
 ******************************************************************************/
void* SAMPLE_RGN_VpssOSdDynamicDisplay(void* p)
{
    HI_S32 s32Ret;
    HI_U32 u32RgnNum;
    RGN_HANDLE Handle;
    RGN_HANDLE startHandle;
    MPP_CHN_S stChn;
    RGN_CHN_ATTR_S stChnAttr;
    RGN_ATTR_S stRgnAttr;
    RGN_ATTR_INFO_S *pstRgnAttrInfo = NULL;
    
    stChn.enModId  = HI_ID_VPSS;
    stChn.s32DevId = 0;
    stChn.s32ChnId = 0;

    pstRgnAttrInfo = (RGN_ATTR_INFO_S *)p;
    startHandle    = pstRgnAttrInfo->Handle;
    u32RgnNum      = pstRgnAttrInfo->u32RgnNum;

    if (u32RgnNum > OVERLAY_MAX_NUM_VPSS)
    {
        printf("overlay num(%d) is bigger than OVERLAY_MAX_NUM_VPSS(%d)..\n", u32RgnNum, OVERLAY_MAX_NUM_VPSS);
        return NULL;
    }
    
    while (HI_FALSE == bExit)
    {
        for (Handle=startHandle; Handle<(startHandle+u32RgnNum); Handle++)
        {
            stChnAttr.bShow  = HI_TRUE;
            stChnAttr.enType = OVERLAY_RGN;
            stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = 50*(Handle - startHandle);
            stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = 50*(Handle - startHandle);
            stChnAttr.unChnAttr.stOverlayChn.u32BgAlpha   = 30*(OVERLAY_MAX_NUM_VPSS - (Handle - startHandle));
            stChnAttr.unChnAttr.stOverlayChn.u32FgAlpha   = 30*(OVERLAY_MAX_NUM_VPSS - (Handle - startHandle));
            stChnAttr.unChnAttr.stOverlayChn.u32Layer     = Handle - startHandle;
            
            s32Ret = HI_MPI_RGN_SetDisplayAttr(Handle, &stChn, &stChnAttr);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_RGN_NOT_PASS(s32Ret);
            }
        }   
        sleep(3);

        for (Handle=startHandle; Handle<(startHandle+u32RgnNum); Handle++)
        {    
            stChnAttr.bShow  = HI_TRUE;
            stChnAttr.enType = OVERLAY_RGN;
            stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = 80*(Handle - startHandle);
            stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = 80*(Handle - startHandle);
            stChnAttr.unChnAttr.stOverlayChn.u32BgAlpha   = 30*(OVERLAY_MAX_NUM_VPSS - (Handle - startHandle));
            stChnAttr.unChnAttr.stOverlayChn.u32FgAlpha   = 30*(OVERLAY_MAX_NUM_VPSS - (Handle - startHandle));
            stChnAttr.unChnAttr.stOverlayChn.u32Layer     = Handle - startHandle;
            
            s32Ret = HI_MPI_RGN_SetDisplayAttr(Handle, &stChn, &stChnAttr);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_RGN_NOT_PASS(s32Ret);
            }
        }
        sleep(3);

        for (Handle=startHandle; Handle<(startHandle+u32RgnNum); Handle++)
        {
            stChnAttr.bShow  = HI_TRUE;
            stChnAttr.enType = OVERLAY_RGN;
            stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = 100*(Handle - startHandle);
            stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = 100*(Handle - startHandle);
            stChnAttr.unChnAttr.stOverlayChn.u32BgAlpha   = 30*(OVERLAY_MAX_NUM_VPSS - (Handle - startHandle));
            stChnAttr.unChnAttr.stOverlayChn.u32FgAlpha   = 30*(OVERLAY_MAX_NUM_VPSS - (Handle - startHandle));
            stChnAttr.unChnAttr.stOverlayChn.u32Layer     = Handle - startHandle;
            
            s32Ret = HI_MPI_RGN_SetDisplayAttr(Handle, &stChn, &stChnAttr);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_RGN_NOT_PASS(s32Ret);
            }
        }
        sleep(3);

        for (Handle=startHandle; Handle<(startHandle+u32RgnNum); Handle++)
        {    
            stChnAttr.bShow  = HI_TRUE;
            stChnAttr.enType = OVERLAY_RGN;
            stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = 120*(Handle - startHandle);
            stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = 120*(Handle - startHandle);
            stChnAttr.unChnAttr.stOverlayChn.u32BgAlpha   = 30*(OVERLAY_MAX_NUM_VPSS - (Handle - startHandle));
            stChnAttr.unChnAttr.stOverlayChn.u32FgAlpha   = 30*(OVERLAY_MAX_NUM_VPSS - (Handle - startHandle));
            stChnAttr.unChnAttr.stOverlayChn.u32Layer     = Handle - startHandle;
            
            s32Ret = HI_MPI_RGN_SetDisplayAttr(Handle, &stChn, &stChnAttr);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_RGN_NOT_PASS(s32Ret);
            }
        }
        
        sleep(2);

        for (Handle=startHandle; Handle<(startHandle+u32RgnNum); Handle++)
        {    
            s32Ret = HI_MPI_RGN_GetAttr(Handle, &stRgnAttr);

            stRgnAttr.enType = OVERLAY_RGN;

            stRgnAttr.unAttr.stOverlay.u32BgColor = 0x0000cccc;

            
            s32Ret = HI_MPI_RGN_SetAttr(Handle, &stRgnAttr);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_RGN_NOT_PASS(s32Ret);
            }
        }
        
        sleep(2);

        for (Handle=startHandle; Handle<(startHandle+u32RgnNum); Handle++)
        {    
            s32Ret = HI_MPI_RGN_GetAttr(Handle, &stRgnAttr);

            stRgnAttr.enType = OVERLAY_RGN;

            stRgnAttr.unAttr.stOverlay.u32BgColor = 0x00cc8000;
            
            
            s32Ret = HI_MPI_RGN_SetAttr(Handle, &stRgnAttr);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_RGN_NOT_PASS(s32Ret);
            }
        }
        
        sleep(2);

        for (Handle=startHandle; Handle<(startHandle+u32RgnNum); Handle++)
        {    
            s32Ret = HI_MPI_RGN_GetAttr(Handle, &stRgnAttr);

            stRgnAttr.enType = OVERLAY_RGN;

            stRgnAttr.unAttr.stOverlay.u32BgColor = 0x00FFFF00;
          
            s32Ret = HI_MPI_RGN_SetAttr(Handle, &stRgnAttr);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_RGN_NOT_PASS(s32Ret);
            }
        }
        
        sleep(2);
    }

    return (HI_VOID *)HI_SUCCESS;
}


/******************************************************************************
 * function : send stream to vdec
 ******************************************************************************/
void* SAMPLE_RGN_VencOSdDynamicDisplay(void* p)
{
    HI_S32 s32Ret;
    RGN_HANDLE Handle;
    MPP_CHN_S stChn;
    RGN_CHN_ATTR_S stChnAttr;

    stChn.enModId  = HI_ID_VENC;
    stChn.s32DevId = 0;
    stChn.s32ChnId = 0;

        
    while (HI_FALSE == bExit)
    {
        /* changes of bgcolor and location */
        for (Handle=0; Handle<4; Handle++)
        {
            stChnAttr.bShow  = HI_TRUE;
            stChnAttr.enType = OVERLAY_RGN;
            stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = 100*Handle;
            stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = 60*Handle;
            stChnAttr.unChnAttr.stOverlayChn.u32BgAlpha   = 16*(8-Handle);
            stChnAttr.unChnAttr.stOverlayChn.u32FgAlpha   = 16*(8-Handle);
            stChnAttr.unChnAttr.stOverlayChn.u32Layer     = Handle;

            stChnAttr.unChnAttr.stOverlayChn.stQpInfo.bAbsQp = HI_FALSE;
            stChnAttr.unChnAttr.stOverlayChn.stQpInfo.s32Qp  = 0;

            stChnAttr.unChnAttr.stOverlayChn.stInvertColor.stInvColArea.u32Height = 16*(Handle%2+1);
            stChnAttr.unChnAttr.stOverlayChn.stInvertColor.stInvColArea.u32Width  = 16*(Handle%2+1);
            stChnAttr.unChnAttr.stOverlayChn.stInvertColor.u32LumThresh = 128;
            stChnAttr.unChnAttr.stOverlayChn.stInvertColor.enChgMod     = LESSTHAN_LUM_THRESH;
            stChnAttr.unChnAttr.stOverlayChn.stInvertColor.bInvColEn    = HI_FALSE;
        
            s32Ret = HI_MPI_RGN_SetDisplayAttr(Handle, &stChn, &stChnAttr);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_RGN_NOT_PASS(s32Ret);
            }
        }   
        sleep(3);

        for (Handle=0; Handle<4; Handle++)
        {    
            stChnAttr.bShow  = HI_TRUE;
            stChnAttr.enType = OVERLAY_RGN;
            stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = 120*Handle;
            stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = 70*Handle;
            stChnAttr.unChnAttr.stOverlayChn.u32BgAlpha   = 16*(8-Handle);
            stChnAttr.unChnAttr.stOverlayChn.u32FgAlpha   = 16*(8-Handle);
            stChnAttr.unChnAttr.stOverlayChn.u32Layer     = Handle;

            stChnAttr.unChnAttr.stOverlayChn.stQpInfo.bAbsQp = HI_FALSE;
            stChnAttr.unChnAttr.stOverlayChn.stQpInfo.s32Qp  = 0;

            stChnAttr.unChnAttr.stOverlayChn.stInvertColor.stInvColArea.u32Height = 16*(Handle%2+1);
            stChnAttr.unChnAttr.stOverlayChn.stInvertColor.stInvColArea.u32Width  = 16*(Handle%2+1);
            stChnAttr.unChnAttr.stOverlayChn.stInvertColor.u32LumThresh = 128;
            stChnAttr.unChnAttr.stOverlayChn.stInvertColor.enChgMod     = LESSTHAN_LUM_THRESH;
            stChnAttr.unChnAttr.stOverlayChn.stInvertColor.bInvColEn    = HI_FALSE;
            
            s32Ret = HI_MPI_RGN_SetDisplayAttr(Handle, &stChn, &stChnAttr);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_RGN_NOT_PASS(s32Ret);
            }
        }
        sleep(3);

        for (Handle=0; Handle<4; Handle++)
        {
            stChnAttr.bShow  = HI_TRUE;
            stChnAttr.enType = OVERLAY_RGN;
            stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = 140*Handle;
            stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = 80*Handle;
            stChnAttr.unChnAttr.stOverlayChn.u32BgAlpha   = 16*(8-Handle);
            stChnAttr.unChnAttr.stOverlayChn.u32FgAlpha   = 16*(8-Handle);
            stChnAttr.unChnAttr.stOverlayChn.u32Layer     = Handle;

            stChnAttr.unChnAttr.stOverlayChn.stQpInfo.bAbsQp = HI_FALSE;
            stChnAttr.unChnAttr.stOverlayChn.stQpInfo.s32Qp  = 0;

            stChnAttr.unChnAttr.stOverlayChn.stInvertColor.stInvColArea.u32Height = 16*(Handle%2+1);
            stChnAttr.unChnAttr.stOverlayChn.stInvertColor.stInvColArea.u32Width  = 16*(Handle%2+1);
            stChnAttr.unChnAttr.stOverlayChn.stInvertColor.u32LumThresh = 128;
            stChnAttr.unChnAttr.stOverlayChn.stInvertColor.enChgMod     = LESSTHAN_LUM_THRESH;
            stChnAttr.unChnAttr.stOverlayChn.stInvertColor.bInvColEn    = HI_FALSE;
            
            s32Ret = HI_MPI_RGN_SetDisplayAttr(Handle, &stChn, &stChnAttr);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_RGN_NOT_PASS(s32Ret);
            }
        }
        sleep(3);

        for (Handle=0; Handle<4; Handle++)
        {    
            stChnAttr.bShow  = HI_TRUE;
            stChnAttr.enType = OVERLAY_RGN;
            stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = 160*Handle;
            stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = 90*Handle;
            stChnAttr.unChnAttr.stOverlayChn.u32BgAlpha   = 16*(8-Handle);
            stChnAttr.unChnAttr.stOverlayChn.u32FgAlpha   = 16*(8-Handle);
            stChnAttr.unChnAttr.stOverlayChn.u32Layer     = Handle;

            stChnAttr.unChnAttr.stOverlayChn.stQpInfo.bAbsQp = HI_FALSE;
            stChnAttr.unChnAttr.stOverlayChn.stQpInfo.s32Qp  = 0;

            stChnAttr.unChnAttr.stOverlayChn.stInvertColor.stInvColArea.u32Height = 16*(Handle%2+1);
            stChnAttr.unChnAttr.stOverlayChn.stInvertColor.stInvColArea.u32Width  = 16*(Handle%2+1);
            stChnAttr.unChnAttr.stOverlayChn.stInvertColor.u32LumThresh = 128;
            stChnAttr.unChnAttr.stOverlayChn.stInvertColor.enChgMod     = LESSTHAN_LUM_THRESH;
            stChnAttr.unChnAttr.stOverlayChn.stInvertColor.bInvColEn    = HI_FALSE;
            
            s32Ret = HI_MPI_RGN_SetDisplayAttr(Handle, &stChn, &stChnAttr);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_RGN_NOT_PASS(s32Ret);
            }
        }
        
        sleep(2);
    }

    return (HI_VOID *)HI_SUCCESS;
}

/******************************************************************************
 * function : send stream to vdec
 ******************************************************************************/
void* SAMPLE_RGN_VpssOverlayexDynamicDisplay(void* p)
{
    HI_S32 s32Ret;
    HI_U32 u32RgnNum;
    RGN_HANDLE Handle;
    RGN_HANDLE startHandle;
    MPP_CHN_S stChn;
    RGN_CHN_ATTR_S stChnAttr;
    RGN_ATTR_INFO_S *pstRgnAttrInfo = NULL;
    
    stChn.enModId  = HI_ID_VPSS;
    stChn.s32DevId = 0;
    stChn.s32ChnId = 0;

    pstRgnAttrInfo = (RGN_ATTR_INFO_S *)p;
    startHandle    = pstRgnAttrInfo->Handle;
    u32RgnNum      = pstRgnAttrInfo->u32RgnNum;

    if (u32RgnNum > OVERLAY_MAX_NUM_VPSS)
    {
        printf("cover num(%d) is bigger than OVERLAY_MAX_NUM_VPSS(%d)..\n", u32RgnNum, OVERLAY_MAX_NUM_VPSS);
        return NULL;
    }
    
    while (HI_FALSE == bExit)
    {
        for (Handle=startHandle; Handle<(startHandle+u32RgnNum); Handle++)
        {
            stChnAttr.bShow  = HI_TRUE;
            stChnAttr.enType = OVERLAYEX_RGN;
            stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32X = 80*(Handle - startHandle);
            stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32Y = 80*(Handle - startHandle);
            stChnAttr.unChnAttr.stOverlayExChn.u32BgAlpha   = 30*(OVERLAY_MAX_NUM_VPSS - (Handle - startHandle));
            stChnAttr.unChnAttr.stOverlayExChn.u32FgAlpha   = 30*(OVERLAY_MAX_NUM_VPSS - (Handle - startHandle));
            stChnAttr.unChnAttr.stOverlayExChn.u32Layer     = Handle - startHandle;
            
            s32Ret = HI_MPI_RGN_SetDisplayAttr(Handle, &stChn, &stChnAttr);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_RGN_NOT_PASS(s32Ret);
            }
        }   
        sleep(3);

        for (Handle=startHandle; Handle<(startHandle+u32RgnNum); Handle++)
        {    
            stChnAttr.bShow  = HI_TRUE;
            stChnAttr.enType = OVERLAYEX_RGN;
            stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32X = 120*(Handle - startHandle);
            stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32Y = 120*(Handle - startHandle);
            stChnAttr.unChnAttr.stOverlayExChn.u32BgAlpha   = 30*(OVERLAY_MAX_NUM_VPSS - (Handle - startHandle));
            stChnAttr.unChnAttr.stOverlayExChn.u32FgAlpha   = 30*(OVERLAY_MAX_NUM_VPSS - (Handle - startHandle));
            stChnAttr.unChnAttr.stOverlayExChn.u32Layer     = Handle - startHandle;
            
            s32Ret = HI_MPI_RGN_SetDisplayAttr(Handle, &stChn, &stChnAttr);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_RGN_NOT_PASS(s32Ret);
            }
        }
        sleep(3);

        for (Handle=startHandle; Handle<(startHandle+u32RgnNum); Handle++)
        {
            stChnAttr.bShow  = HI_TRUE;
            stChnAttr.enType = OVERLAYEX_RGN;
            stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32X = 160*(Handle - startHandle);
            stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32Y = 160*(Handle - startHandle);
            stChnAttr.unChnAttr.stOverlayExChn.u32BgAlpha   = 30*(OVERLAY_MAX_NUM_VPSS - (Handle - startHandle));
            stChnAttr.unChnAttr.stOverlayExChn.u32FgAlpha   = 30*(OVERLAY_MAX_NUM_VPSS - (Handle - startHandle));
            stChnAttr.unChnAttr.stOverlayExChn.u32Layer     = Handle - startHandle;
            
            s32Ret = HI_MPI_RGN_SetDisplayAttr(Handle, &stChn, &stChnAttr);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_RGN_NOT_PASS(s32Ret);
            }
        }
        sleep(3);

        for (Handle=startHandle; Handle<(startHandle+u32RgnNum); Handle++)
        {    
            stChnAttr.bShow  = HI_TRUE;
            stChnAttr.enType = OVERLAYEX_RGN;
            stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32X = 200*(Handle - startHandle);
            stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32Y = 200*(Handle - startHandle);
            stChnAttr.unChnAttr.stOverlayExChn.u32BgAlpha   = 30*(OVERLAY_MAX_NUM_VPSS - (Handle - startHandle));
            stChnAttr.unChnAttr.stOverlayExChn.u32FgAlpha   = 30*(OVERLAY_MAX_NUM_VPSS - (Handle - startHandle));
            stChnAttr.unChnAttr.stOverlayExChn.u32Layer     = Handle - startHandle;
            
            s32Ret = HI_MPI_RGN_SetDisplayAttr(Handle, &stChn, &stChnAttr);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_RGN_NOT_PASS(s32Ret);
            }
        }
        
        sleep(2);
    }

    return (HI_VOID *)HI_SUCCESS;
}

/******************************************************************************
 * function : send stream to vdec
 ******************************************************************************/
void* SAMPLE_RGN_VpssCoverexDynamicDisplay(void* p)
{
    HI_S32 s32Ret;
    HI_U32 u32RgnNum;
    RGN_HANDLE Handle;
    RGN_HANDLE startHandle;
    MPP_CHN_S stChn;
    RGN_CHN_ATTR_S stChnAttr;
    RGN_ATTR_INFO_S *pstRgnAttrInfo = NULL;
    
    stChn.enModId  = HI_ID_VPSS;
    stChn.s32DevId = 0;
    stChn.s32ChnId = 0;

    pstRgnAttrInfo = (RGN_ATTR_INFO_S *)p;
    startHandle    = pstRgnAttrInfo->Handle;
    u32RgnNum      = pstRgnAttrInfo->u32RgnNum;

    if (u32RgnNum > COVER_MAX_NUM_VPSS)
    {
        printf("cover num(%d) is bigger than COVER_MAX_NUM_VPSS(%d)..\n", u32RgnNum, COVER_MAX_NUM_VPSS);
        return NULL;
    }
    
    while (HI_FALSE == bExit)
    {
        for (Handle=startHandle; Handle<(startHandle+u32RgnNum); Handle++)
        {
            stChnAttr.bShow  = HI_TRUE;
            stChnAttr.enType = COVEREX_RGN;
            stChnAttr.unChnAttr.stCoverExChn.stRect.s32X      = 60 * (Handle - startHandle)+400;
            stChnAttr.unChnAttr.stCoverExChn.stRect.s32Y      = 60 * (Handle - startHandle);
            stChnAttr.unChnAttr.stCoverExChn.stRect.u32Height = 60 * (Handle - startHandle)+100;
            stChnAttr.unChnAttr.stCoverExChn.stRect.u32Width  = 60 * (Handle - startHandle)+100;
            stChnAttr.unChnAttr.stCoverExChn.u32Color         = 0x0000ff;
            if (Handle%2)
            {
                stChnAttr.unChnAttr.stCoverExChn.u32Color = 0xff0000;
            }
            stChnAttr.unChnAttr.stCoverExChn.u32Layer     = Handle - startHandle; 
            stChnAttr.unChnAttr.stCoverExChn.enCoverType = AREA_RECT;

            s32Ret = HI_MPI_RGN_SetDisplayAttr(Handle, &stChn, &stChnAttr);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_RGN_NOT_PASS(s32Ret);
            }
        }   
        sleep(3);

        for (Handle=startHandle; Handle<(startHandle+u32RgnNum); Handle++)
        {    
            stChnAttr.unChnAttr.stCoverExChn.stRect.s32X      = 60 * (Handle - startHandle)+500;
            stChnAttr.unChnAttr.stCoverExChn.stRect.s32Y      = 60 * (Handle - startHandle);
            stChnAttr.unChnAttr.stCoverExChn.stRect.u32Height = 60 * (Handle - startHandle)+200;
            stChnAttr.unChnAttr.stCoverExChn.stRect.u32Width  = 60 * (Handle - startHandle)+200;
            stChnAttr.unChnAttr.stCoverExChn.u32Color         = 0x00ff00;
            if (Handle%2)
            {
                stChnAttr.unChnAttr.stCoverExChn.u32Color = 0xff0000;
            }
            stChnAttr.unChnAttr.stCoverExChn.u32Layer     = Handle - startHandle;
            stChnAttr.unChnAttr.stCoverExChn.enCoverType = AREA_RECT;
            
            s32Ret = HI_MPI_RGN_SetDisplayAttr(Handle, &stChn, &stChnAttr);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_RGN_NOT_PASS(s32Ret);
            }
        }
        sleep(3);

        for (Handle=startHandle; Handle<(startHandle+u32RgnNum); Handle++)
        {
            stChnAttr.unChnAttr.stCoverExChn.stRect.s32X      = 60 * (Handle - startHandle)+600;
            stChnAttr.unChnAttr.stCoverExChn.stRect.s32Y      = 60 * (Handle - startHandle);
            stChnAttr.unChnAttr.stCoverExChn.stRect.u32Height = 60 * (Handle - startHandle)+300;
            stChnAttr.unChnAttr.stCoverExChn.stRect.u32Width  = 60 * (Handle - startHandle)+300;
            stChnAttr.unChnAttr.stCoverExChn.u32Color         = 0x0000ff;
            if (1 == Handle)
            {
                stChnAttr.unChnAttr.stCoverExChn.u32Color  = 0x00ff00;
            }
            else if (2 == Handle)
            {
                stChnAttr.unChnAttr.stCoverExChn.u32Color  = 0xff0000;
            }
            else if (3 == Handle)
            {
                stChnAttr.unChnAttr.stCoverExChn.u32Color  = 0xff0000;
            }
            stChnAttr.unChnAttr.stCoverExChn.u32Layer      = Handle - startHandle;
            stChnAttr.unChnAttr.stCoverExChn.enCoverType = AREA_RECT;
            
            s32Ret = HI_MPI_RGN_SetDisplayAttr(Handle, &stChn, &stChnAttr);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_RGN_NOT_PASS(s32Ret);
            }
        }
        sleep(3);

        for (Handle=startHandle; Handle<(startHandle+u32RgnNum); Handle++)
        {    
            stChnAttr.unChnAttr.stCoverExChn.stRect.s32X      = 60 * (Handle - startHandle)+600;
            stChnAttr.unChnAttr.stCoverExChn.stRect.s32Y      = 60 * (Handle - startHandle);
            stChnAttr.unChnAttr.stCoverExChn.stRect.u32Height = 60 * (Handle - startHandle)+400;
            stChnAttr.unChnAttr.stCoverExChn.stRect.u32Width  = 60 * (Handle - startHandle)+400;
            stChnAttr.unChnAttr.stCoverExChn.u32Color         = 0x0000ff;
            if (1 == Handle)
            {
                stChnAttr.unChnAttr.stCoverExChn.u32Color  = 0x00ff00;
            }
            else if (2 == Handle)
            {
                stChnAttr.unChnAttr.stCoverExChn.u32Color  = 0xff0000;
            }
            else if (3 == Handle)
            {
                stChnAttr.unChnAttr.stCoverExChn.u32Color  = 0xff0000;
            }
            stChnAttr.unChnAttr.stCoverExChn.u32Layer      = Handle - startHandle;
            stChnAttr.unChnAttr.stCoverExChn.enCoverType = AREA_RECT;
            
            s32Ret = HI_MPI_RGN_SetDisplayAttr(Handle, &stChn, &stChnAttr);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_RGN_NOT_PASS(s32Ret);
            }
        }
        
        sleep(2);
    }

    return (HI_VOID *)HI_SUCCESS;
}

/******************************************************************************
 * function : send stream to vdec
 ******************************************************************************/
void* SAMPLE_RGN_VpssMosaicDisplay(void* p)
{
    HI_S32 s32Ret;
    HI_U32 u32RgnNum;
    RGN_HANDLE Handle;
    RGN_HANDLE startHandle;
    MPP_CHN_S stChn;
    RGN_CHN_ATTR_S stChnAttr;
    RGN_ATTR_INFO_S *pstRgnAttrInfo = NULL;
    
    stChn.enModId  = HI_ID_VPSS;
    stChn.s32DevId = 0;
    stChn.s32ChnId = 0;

    pstRgnAttrInfo = (RGN_ATTR_INFO_S *)p;
    startHandle    = pstRgnAttrInfo->Handle;
    u32RgnNum      = pstRgnAttrInfo->u32RgnNum;

    if (u32RgnNum > MASAIC_MAX_NUM_VPSS)
    {
        printf("overlay num(%d) is bigger than OVERLAY_MAX_NUM_VPSS(%d)..\n", u32RgnNum, OVERLAY_MAX_NUM_VPSS);
        return NULL;
    }
    
    while (HI_FALSE == bExit)
    {
        for (Handle=startHandle; Handle<(startHandle+u32RgnNum); Handle++)
        {
        	s32Ret = HI_MPI_RGN_GetDisplayAttr(Handle, &stChn, &stChnAttr);
			if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_RGN_NOT_PASS(s32Ret);
            }
			
			stChnAttr.bShow  = HI_TRUE;

            stChnAttr.unChnAttr.stMosaicChn.stRect.s32X = 64*(rand()%6);
            stChnAttr.unChnAttr.stMosaicChn.stRect.s32Y = 64*(rand()%6);
            
            s32Ret = HI_MPI_RGN_SetDisplayAttr(Handle, &stChn, &stChnAttr);
        }
		
        sleep(2);

    }

    return (HI_VOID *)HI_SUCCESS;
}


/******************************************************************************
 * function : send stream to vdec
 ******************************************************************************/
void* SAMPLE_RGN_VouOverlayexDynamicDisplay(void* p)
{
    HI_S32 s32Ret;
    HI_U32 u32RgnNum;
    RGN_HANDLE Handle;
    RGN_HANDLE startHandle;
    MPP_CHN_S stChn;
    RGN_CHN_ATTR_S stChnAttr;
    RGN_ATTR_INFO_S *pstRgnAttrInfo = NULL;
    
    stChn.enModId  = HI_ID_VOU;
    stChn.s32DevId = 0;
    stChn.s32ChnId = 0;

    pstRgnAttrInfo = (RGN_ATTR_INFO_S *)p;
    startHandle    = pstRgnAttrInfo->Handle;
    u32RgnNum      = pstRgnAttrInfo->u32RgnNum;

    if (u32RgnNum > OVERLAYEX_MAX_NUM_VO)
    {
        printf("cover num(%d) is bigger than OVERLAY_MAX_NUM_VPSS(%d)..\n", u32RgnNum, OVERLAYEX_MAX_NUM_VO);
        return NULL;
    }
    
    while (HI_FALSE == bExit)
    {
        for (Handle=startHandle; Handle<(startHandle+u32RgnNum); Handle++)
        {
            stChnAttr.bShow  = HI_TRUE;
            stChnAttr.enType = OVERLAYEX_RGN;
            stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32X = 80*(Handle - startHandle);
            stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32Y = 80*(Handle - startHandle);
            stChnAttr.unChnAttr.stOverlayExChn.u32BgAlpha   = 30*(OVERLAYEX_MAX_NUM_VO - (Handle - startHandle));
            stChnAttr.unChnAttr.stOverlayExChn.u32FgAlpha   = 30*(OVERLAYEX_MAX_NUM_VO - (Handle - startHandle));
            stChnAttr.unChnAttr.stOverlayExChn.u32Layer     = Handle - startHandle;
            
            s32Ret = HI_MPI_RGN_SetDisplayAttr(Handle, &stChn, &stChnAttr);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_RGN_NOT_PASS(s32Ret);
            }
        }   
        sleep(3);

        for (Handle=startHandle; Handle<(startHandle+u32RgnNum); Handle++)
        {    
            stChnAttr.bShow  = HI_TRUE;
            stChnAttr.enType = OVERLAYEX_RGN;
            stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32X = 120*(Handle - startHandle);
            stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32Y = 120*(Handle - startHandle);
            stChnAttr.unChnAttr.stOverlayExChn.u32BgAlpha   = 30*(OVERLAYEX_MAX_NUM_VO - (Handle - startHandle));
            stChnAttr.unChnAttr.stOverlayExChn.u32FgAlpha   = 30*(OVERLAYEX_MAX_NUM_VO - (Handle - startHandle));
            stChnAttr.unChnAttr.stOverlayExChn.u32Layer     = Handle - startHandle;
            
            s32Ret = HI_MPI_RGN_SetDisplayAttr(Handle, &stChn, &stChnAttr);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_RGN_NOT_PASS(s32Ret);
            }
        }
        sleep(3);

        for (Handle=startHandle; Handle<(startHandle+u32RgnNum); Handle++)
        {
            stChnAttr.bShow  = HI_TRUE;
            stChnAttr.enType = OVERLAYEX_RGN;
            stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32X = 160*(Handle - startHandle);
            stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32Y = 160*(Handle - startHandle);
            stChnAttr.unChnAttr.stOverlayExChn.u32BgAlpha   = 30*(OVERLAYEX_MAX_NUM_VO - (Handle - startHandle));
            stChnAttr.unChnAttr.stOverlayExChn.u32FgAlpha   = 30*(OVERLAYEX_MAX_NUM_VO - (Handle - startHandle));
            stChnAttr.unChnAttr.stOverlayExChn.u32Layer     = Handle - startHandle;
            
            s32Ret = HI_MPI_RGN_SetDisplayAttr(Handle, &stChn, &stChnAttr);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_RGN_NOT_PASS(s32Ret);
            }
        }
        sleep(3);

        for (Handle=startHandle; Handle<(startHandle+u32RgnNum); Handle++)
        {    
            stChnAttr.bShow  = HI_TRUE;
            stChnAttr.enType = OVERLAYEX_RGN;
            stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32X = 200*(Handle - startHandle);
            stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32Y = 200*(Handle - startHandle);
            stChnAttr.unChnAttr.stOverlayExChn.u32BgAlpha   = 30*(OVERLAYEX_MAX_NUM_VO - (Handle - startHandle));
            stChnAttr.unChnAttr.stOverlayExChn.u32FgAlpha   = 30*(OVERLAYEX_MAX_NUM_VO - (Handle - startHandle));
            stChnAttr.unChnAttr.stOverlayExChn.u32Layer     = Handle - startHandle;
            
            s32Ret = HI_MPI_RGN_SetDisplayAttr(Handle, &stChn, &stChnAttr);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_RGN_NOT_PASS(s32Ret);
            }
        }
        
        sleep(2);
    }

    return (HI_VOID *)HI_SUCCESS;
}

/******************************************************************************
 * function : send stream to vdec
 ******************************************************************************/
void* SAMPLE_RGN_VouCoverexDynamicDisplay(void* p)
{
    HI_S32 s32Ret;
    HI_U32 u32RgnNum;
    RGN_HANDLE Handle;
    RGN_HANDLE startHandle;
    MPP_CHN_S stChn;
    RGN_CHN_ATTR_S stChnAttr;
    RGN_ATTR_INFO_S *pstRgnAttrInfo = NULL;
    
    stChn.enModId  = HI_ID_VOU;
    stChn.s32DevId = 0;
    stChn.s32ChnId = 0;

    pstRgnAttrInfo = (RGN_ATTR_INFO_S *)p;
    startHandle    = pstRgnAttrInfo->Handle;
    u32RgnNum      = pstRgnAttrInfo->u32RgnNum;

    if (u32RgnNum > COVER_MAX_NUM_VPSS)
    {
        printf("cover num(%d) is bigger than COVER_MAX_NUM_VPSS(%d)..\n", u32RgnNum, COVER_MAX_NUM_VPSS);
        return NULL;
    }
    
    while (HI_FALSE == bExit)
    {
        for (Handle=startHandle; Handle<(startHandle+u32RgnNum); Handle++)
        {
            stChnAttr.bShow  = HI_TRUE;
            stChnAttr.enType = COVEREX_RGN;
            stChnAttr.unChnAttr.stCoverExChn.stRect.s32X      = 60 * (Handle - startHandle)+400;
            stChnAttr.unChnAttr.stCoverExChn.stRect.s32Y      = 60 * (Handle - startHandle);
            stChnAttr.unChnAttr.stCoverExChn.stRect.u32Height = 60 * (Handle - startHandle)+100;
            stChnAttr.unChnAttr.stCoverExChn.stRect.u32Width  = 60 * (Handle - startHandle)+100;
            stChnAttr.unChnAttr.stCoverExChn.u32Color         = 0x0000ff;
            if (Handle%2)
            {
                stChnAttr.unChnAttr.stCoverExChn.u32Color = 0xff0000;
            }
            stChnAttr.unChnAttr.stCoverExChn.u32Layer     = Handle - startHandle; 
            stChnAttr.unChnAttr.stCoverExChn.enCoverType = AREA_RECT;

            s32Ret = HI_MPI_RGN_SetDisplayAttr(Handle, &stChn, &stChnAttr);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_RGN_NOT_PASS(s32Ret);
            }
        }   
        sleep(3);

        for (Handle=startHandle; Handle<(startHandle+u32RgnNum); Handle++)
        {    
            stChnAttr.unChnAttr.stCoverExChn.stRect.s32X      = 60 * (Handle - startHandle)+500;
            stChnAttr.unChnAttr.stCoverExChn.stRect.s32Y      = 60 * (Handle - startHandle);
            stChnAttr.unChnAttr.stCoverExChn.stRect.u32Height = 60 * (Handle - startHandle)+200;
            stChnAttr.unChnAttr.stCoverExChn.stRect.u32Width  = 60 * (Handle - startHandle)+200;
            stChnAttr.unChnAttr.stCoverExChn.u32Color         = 0x00ff00;
            if (Handle%2)
            {
                stChnAttr.unChnAttr.stCoverExChn.u32Color = 0xff0000;
            }
            stChnAttr.unChnAttr.stCoverExChn.u32Layer     = Handle - startHandle;
            stChnAttr.unChnAttr.stCoverExChn.enCoverType = AREA_RECT;
            
            s32Ret = HI_MPI_RGN_SetDisplayAttr(Handle, &stChn, &stChnAttr);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_RGN_NOT_PASS(s32Ret);
            }
        }
        sleep(3);

        for (Handle=startHandle; Handle<(startHandle+u32RgnNum); Handle++)
        {
            stChnAttr.unChnAttr.stCoverExChn.stRect.s32X      = 60 * (Handle - startHandle)+600;
            stChnAttr.unChnAttr.stCoverExChn.stRect.s32Y      = 60 * (Handle - startHandle);
            stChnAttr.unChnAttr.stCoverExChn.stRect.u32Height = 60 * (Handle - startHandle)+300;
            stChnAttr.unChnAttr.stCoverExChn.stRect.u32Width  = 60 * (Handle - startHandle)+300;
            stChnAttr.unChnAttr.stCoverExChn.u32Color         = 0x0000ff;
            if (1 == Handle)
            {
                stChnAttr.unChnAttr.stCoverExChn.u32Color  = 0x00ff00;
            }
            else if (2 == Handle)
            {
                stChnAttr.unChnAttr.stCoverExChn.u32Color  = 0xff0000;
            }
            else if (3 == Handle)
            {
                stChnAttr.unChnAttr.stCoverExChn.u32Color  = 0xff0000;
            }
            stChnAttr.unChnAttr.stCoverExChn.u32Layer      = Handle - startHandle;
            stChnAttr.unChnAttr.stCoverExChn.enCoverType = AREA_RECT;
            
            s32Ret = HI_MPI_RGN_SetDisplayAttr(Handle, &stChn, &stChnAttr);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_RGN_NOT_PASS(s32Ret);
            }
        }
        sleep(3);

        for (Handle=startHandle; Handle<(startHandle+u32RgnNum); Handle++)
        {    
            stChnAttr.unChnAttr.stCoverExChn.stRect.s32X      = 60 * (Handle - startHandle)+600;
            stChnAttr.unChnAttr.stCoverExChn.stRect.s32Y      = 60 * (Handle - startHandle);
            stChnAttr.unChnAttr.stCoverExChn.stRect.u32Height = 60 * (Handle - startHandle)+400;
            stChnAttr.unChnAttr.stCoverExChn.stRect.u32Width  = 60 * (Handle - startHandle)+400;
            stChnAttr.unChnAttr.stCoverExChn.u32Color         = 0x0000ff;
            if (1 == Handle)
            {
                stChnAttr.unChnAttr.stCoverExChn.u32Color  = 0x00ff00;
            }
            else if (2 == Handle)
            {
                stChnAttr.unChnAttr.stCoverExChn.u32Color  = 0xff0000;
            }
            else if (3 == Handle)
            {
                stChnAttr.unChnAttr.stCoverExChn.u32Color  = 0xff0000;
            }
            stChnAttr.unChnAttr.stCoverExChn.u32Layer      = Handle - startHandle;
            stChnAttr.unChnAttr.stCoverExChn.enCoverType = AREA_RECT;
            
            s32Ret = HI_MPI_RGN_SetDisplayAttr(Handle, &stChn, &stChnAttr);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_RGN_NOT_PASS(s32Ret);
            }
        }
        
        sleep(2);
    }

    return (HI_VOID *)HI_SUCCESS;
}



HI_VOID SAMPLE_RGN_WriteStreamToFile( FILE * fp, VENC_PACK_S * pstNalu )
{
    HI_U8 * p;
    HI_U32 u32Len;

    p = (HI_U8 *) pstNalu->pu8Addr+pstNalu->u32Offset;

    u32Len = pstNalu->u32Len-pstNalu->u32Offset;

    fwrite(p, u32Len, sizeof(HI_U8), fp);
    
    fflush(fp);
}

    
HI_S32 SAMPLE_RGN_CreateRegion(RGN_SIZE_S stRgnsize)
{
    RGN_CHN_ATTR_S stChnAttr;
    RGN_ATTR_S stRgnAttr;
    HI_U32 i,j;
    HI_S32 s32Ret;
    MPP_CHN_S stChn;
    
    for(i=0; i<stRgnsize.u32RgnNum; i++)
    {
        /*create rgn*/
        stRgnAttr.enType = OVERLAY_RGN;

        stRgnAttr.unAttr.stOverlay.enPixelFmt = PIXEL_FORMAT_RGB_1555;
        if(bShape1)
        {
            stRgnAttr.unAttr.stOverlay.stSize.u32Width  = 720;
            stRgnAttr.unAttr.stOverlay.stSize.u32Height = 96;
        }
        else if(bShape2)
        {
            stRgnAttr.unAttr.stOverlay.stSize.u32Width  = 96;
            stRgnAttr.unAttr.stOverlay.stSize.u32Height = 576;
        }
        else if(bArea)
        {
            stRgnAttr.unAttr.stOverlay.stSize.u32Width  = stRgnsize.stSIZE.u32Width;
            stRgnAttr.unAttr.stOverlay.stSize.u32Height = stRgnsize.stSIZE.u32Height;
        }
        else
        {
            stRgnAttr.unAttr.stOverlay.stSize.u32Width  = 96;
            stRgnAttr.unAttr.stOverlay.stSize.u32Height = 96;
        }
        stRgnAttr.unAttr.stOverlay.u32BgColor           = 0xfc + (0xff*i);

        s32Ret = HI_MPI_RGN_Create(i, &stRgnAttr);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_RGN_NOT_PASS(s32Ret);
        }

        for(j=0; j<MAX_VENC_WORK_CHN_NUM; j++)
        {
            stChn.enModId  = HI_ID_VENC;
            stChn.s32DevId = j;
            stChn.s32ChnId = 0;

            stChnAttr.bShow  = HI_TRUE;
            stChnAttr.enType = OVERLAY_RGN;
            stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = 96*(i+1);
            stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = 24*(j+1);
            if(bArea)
            {
                stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = 0;
                stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = 0;
            }
            stChnAttr.unChnAttr.stOverlayChn.u32BgAlpha = 128;
            stChnAttr.unChnAttr.stOverlayChn.u32FgAlpha = 128;
            stChnAttr.unChnAttr.stOverlayChn.u32Layer   = i;

            stChnAttr.unChnAttr.stOverlayChn.stQpInfo.bAbsQp = HI_FALSE;
            stChnAttr.unChnAttr.stOverlayChn.stQpInfo.s32Qp  = 0;
            
            stChnAttr.unChnAttr.stOverlayChn.stInvertColor.bInvColEn = HI_FALSE;
            
            s32Ret = HI_MPI_RGN_AttachToChn(i, &stChn, &stChnAttr);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_RGN_NOT_PASS(s32Ret);
            }
        }
    }
    
    return HI_SUCCESS;
}


HI_S32 SAMPLE_RGN_DestroyRegion(RGN_HANDLE Handle, HI_U32 u32Num)
{
    HI_S32 i;
    HI_S32 s32Ret;    
        
    for (i=Handle; i<(Handle + u32Num); i++)
    {
        s32Ret = HI_MPI_RGN_Destroy(i);
        if (HI_SUCCESS != s32Ret)
        {
            printf("HI_MPI_RGN_Destroy failed! s32Ret: 0x%x.\n", s32Ret);
            return s32Ret;
        }
    }

    return HI_SUCCESS;
    
}
 


HI_S32 SAMPLE_RGN_GetStream( HI_S32 chnId,FILE* pfStream ,HI_BOOL bByFrame)
{
    VENC_STREAM_S   stStream;
    HI_U32          u32Cnt;
    HI_S32          s32Ret;
    VENC_CHN_STAT_S stChnStat;
    VENC_PACK_S*    pstPack;

    memset(&stChnStat, 0, sizeof(VENC_CHN_STAT_S));
    s32Ret = HI_MPI_VENC_Query(chnId, &stChnStat);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_VENC_Query failed! s32Ret: 0x%x.\n", s32Ret);
        return s32Ret;
    }
    
    if(0 == stChnStat.u32CurPacks)
    {
        return HI_FAILURE;
    }
    
    /*malloc buf according to the number of stream packets*/
    stStream.pstPack      = (VENC_PACK_S *) malloc( sizeof(VENC_PACK_S)*stChnStat.u32CurPacks);
    stStream.u32PackCount = stChnStat.u32CurPacks;

#ifndef HI_IO_NOBLOCK
#define HI_IO_NOBLOCK 1
#endif

    /*get the stream*/
    s32Ret = HI_MPI_VENC_GetStream(chnId, &stStream, 0);
    if (HI_SUCCESS != s32Ret)
    {
        printf("Query ok but get fail ret:0x%x\n",s32Ret);
        exit(-1);
        goto __FAIL__;
    }

    pstPack = stStream.pstPack;

    for (u32Cnt=0; u32Cnt<stChnStat.u32CurPacks; u32Cnt++)
    {
        if ( HI_SUCCESS == s32Ret )
        {
            if(NULL != pfStream)
            {
                SAMPLE_RGN_WriteStreamToFile(pfStream, pstPack);
            }
        }
        else
        {
            break;
        }
        pstPack++;
    }
    
    /*release the stream*/
    if (HI_SUCCESS == s32Ret)
    {
        HI_MPI_VENC_ReleaseStream(chnId, &stStream);
        HI_ASSERT( HI_SUCCESS == s32Ret );
    }

    if(NULL != stStream.pstPack)
    {
        free(stStream.pstPack);
    }
    
    return HI_SUCCESS;
    
__FAIL__:
    if(NULL != stStream.pstPack)
    {
        free(stStream.pstPack);
    }
    return HI_FAILURE;
    
}


HI_VOID *SAMPLE_RGN_VencGetStream(void *pData)
{
    VENC_CHN_STAT_S stChnStat;
    VENC_PTHREAD_INFO_S *pstVencThread;
    FILE * pastream;
    HI_S32 i = 0;
    HI_S32 s32Ret = HI_FAILURE;
    HI_S32 s32cnt = 0;
    RGN_HANDLE Handle;
    MPP_CHN_S stChn;
    RGN_CHN_ATTR_S stChnAttr;
    RGN_SIZE_S stRgnsize;
    HI_U32 s32Tmp = 0;
    BITMAP_S stBitmap;
    char acmd[128];

   
    HI_ASSERT(HI_NULL != pData);

    pstVencThread = (VENC_PTHREAD_INFO_S *)pData;
    sprintf(acmd, "[[ -e jpeg%d ]] || mkdir jpeg%d",pstVencThread->VeChnId,pstVencThread->VeChnId); 
    system(acmd);

    while (HI_FALSE == bExit || i < pstVencThread->s32FrmCnt)
    {
        /*change the bitmap*/
        Handle = 0;
         
        s32Ret = HI_MPI_VENC_Query( pstVencThread->VeChnId, &stChnStat );
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_RGN_NOT_PASS(s32Ret);
        }
        if (stChnStat.u32CurPacks > 0 || stChnStat.u32LeftStreamBytes >0)  
        {
            if(bTravel)
            {
                char pfilename[60]; 
                sprintf(pfilename,"./jpeg%d/stream_fff_009_%d.jpeg",pstVencThread->VeChnId, i);
                pastream = fopen( pfilename, "wb" );  //open stream file
                HI_ASSERT( NULL != pastream);              
                
                /*change the position to display*/       
                Handle = 0;
                stChn.enModId  = HI_ID_VENC;
                stChn.s32DevId = pstVencThread->VeChnId;
                stChn.s32ChnId = 0;
                    
                s32Ret = HI_MPI_RGN_GetDisplayAttr(Handle,&stChn,&stChnAttr);
                if (HI_SUCCESS != s32Ret)
                {
                    SAMPLE_RGN_NOT_PASS(s32Ret);
                }
                stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = 36 * (s32cnt % 19);        
                stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = 32 * (s32cnt  / 19);

                s32Ret = HI_MPI_RGN_SetDisplayAttr(Handle,&stChn,&stChnAttr);
                if (HI_SUCCESS != s32Ret)
                {
                    SAMPLE_RGN_NOT_PASS(s32Ret);
                }               


                printf("1stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = %d\n",stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X);
                printf("1stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = %d\n",stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y);
                
                usleep(500);

                s32Ret = HI_MPI_RGN_GetDisplayAttr(Handle,&stChn,&stChnAttr);
                if (HI_SUCCESS != s32Ret)
                {
                    SAMPLE_RGN_NOT_PASS(s32Ret);
                }

                printf("stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = %d\n",stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X);
                printf("stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = %d\n",stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y);

                
                if(++s32cnt > (19 * 16) - 1)
                {
                    s32cnt = 0;
                }
                printf("s32cnt = %d\n", s32cnt);
                
                /*get the stream*/                         
                SAMPLE_RGN_GetStream(pstVencThread->VeChnId, pastream, HI_TRUE);
                
                i++;

                fclose(pastream);
                printf("i = %d\n", i);
                
            }
            else if(bShape1)
            {
                char pfilename[60]; 
                 s32Tmp = 12;
                if(pstVencThread->VeChnId > 0)
                    s32Tmp = 14;
                sprintf(pfilename,"./jpeg%d/stream_fff_0%d_%d.jpeg",pstVencThread->VeChnId, s32Tmp, i);
                pastream = fopen( pfilename, "wb" );  
                HI_ASSERT( NULL != pastream);
               
                
                /*change the position to display*/      
                Handle = 0;
                
                stChn.enModId = HI_ID_VENC;
                stChn.s32DevId = pstVencThread->VeChnId;
                stChn.s32ChnId = 0;
                    
                s32Ret = HI_MPI_RGN_GetDisplayAttr(Handle,&stChn,&stChnAttr);
                if (HI_SUCCESS != s32Ret)
                {
                    SAMPLE_RGN_NOT_PASS(s32Ret);
                }

                stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = 0;        
                stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = 32 * s32cnt;

                s32Ret = HI_MPI_RGN_SetDisplayAttr(Handle,&stChn,&stChnAttr);
                if (HI_SUCCESS != s32Ret)
                {
                    SAMPLE_RGN_NOT_PASS(s32Ret);
                }               


                printf("1stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = %d\n",stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X);
                printf("1stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = %d\n",stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y);
                sleep(1);

                s32Ret = HI_MPI_RGN_GetDisplayAttr(Handle,&stChn,&stChnAttr);
                if (HI_SUCCESS != s32Ret)
                {
                    SAMPLE_RGN_NOT_PASS(s32Ret);
                }

                printf("stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = %d\n",stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X);
                printf("stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = %d\n",stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y);

                
                if(++s32cnt > 25)
                {
                    s32cnt = 0;
                }
                printf("s32cnt = %d\n", s32cnt);
                
                /*get the steam*/                         
                SAMPLE_RGN_GetStream(pstVencThread->VeChnId, pastream, HI_TRUE);             
                i++;

                fclose(pastream);
                printf("i = %d\n", i);
            }
            else if(bShape2)
            {
                char pfilename[60]; 
                s32Tmp = 13;
                if(pstVencThread->VeChnId > 0)
                    s32Tmp = 15;
                sprintf(pfilename,"./jpeg%d/stream_fff_0%d_%d.jpeg",pstVencThread->VeChnId, s32Tmp,i);
                pastream = fopen( pfilename, "wb" );
                HI_ASSERT( NULL != pastream);
               
            
                /*change the position to display*/      
                Handle = 0;
                
                stChn.enModId = HI_ID_VENC;
                stChn.s32DevId = pstVencThread->VeChnId;
                stChn.s32ChnId = 0;
                    
                s32Ret = HI_MPI_RGN_GetDisplayAttr(Handle,&stChn,&stChnAttr);
                if (HI_SUCCESS != s32Ret)
                {
                    SAMPLE_RGN_NOT_PASS(s32Ret);
                }

                stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = 32 * s32cnt;
                stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = 0;

                s32Ret = HI_MPI_RGN_SetDisplayAttr(Handle,&stChn,&stChnAttr);
                if (HI_SUCCESS != s32Ret)
                {
                    SAMPLE_RGN_NOT_PASS(s32Ret);
                }               


                printf("1stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = %d\n",stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X);
                printf("1stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = %d\n",stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y);
                sleep(1);

                s32Ret = HI_MPI_RGN_GetDisplayAttr(Handle, &stChn, &stChnAttr);
                if (HI_SUCCESS != s32Ret)
                {
                    SAMPLE_RGN_NOT_PASS(s32Ret);
                }

                printf("stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = %d\n",stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X);
                printf("stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = %d\n",stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y);

                
                if(++s32cnt > 25)
                {
                    s32cnt = 0;
                }
                printf("s32cnt = %d\n", s32cnt);
                
                /*get the stream*/                         
                SAMPLE_RGN_GetStream(pstVencThread->VeChnId, pastream, HI_TRUE);             
                i++;

                fclose(pastream);
                printf("i = %d\n", i);
            }
            else if(bArea)
            {
                char pfilename[60]; 
                s32Tmp = 16;
                if(pstVencThread->VeChnId > 0)
                    s32Tmp = 17;
                sprintf(pfilename,"./jpeg%d/stream_fff_0%d_%d.jpeg",pstVencThread->VeChnId, s32Tmp,i);
                pastream = fopen( pfilename, "wb" );  
                HI_ASSERT( NULL != pastream);

                pthread_mutex_lock(&Rgnmutex_Tmp);
                
                stRgnsize.u32RgnNum = 1;
                SAMPLE_RGN_DestroyRegion(Handle, stRgnsize.u32RgnNum);
                stRgnsize.stSIZE.u32Height = 36 * (s32cnt + 1);
                stRgnsize.stSIZE.u32Width = 36 * (s32cnt + 1);
                SAMPLE_RGN_CreateRegion(stRgnsize);

                 /*change the bitmap*/
                 Handle = 0;
                 
                 SAMPLE_RGN_LoadBmp("mm.bmp", &stBitmap, HI_FALSE, 0);
                 
                 s32Ret = HI_MPI_RGN_SetBitMap(Handle,&stBitmap);
                 if (HI_SUCCESS != s32Ret)
                 {
                     SAMPLE_RGN_NOT_PASS(s32Ret);
                 }

                 free(stBitmap.pData);

                if(++s32cnt > 25)
                {
                    s32cnt = 0;
                }

                pthread_mutex_unlock(&Rgnmutex_Tmp);
                
                printf("s32cnt = %d\n", s32cnt);
                usleep(50000);

                /*get the stream*/                         
                SAMPLE_RGN_GetStream(pstVencThread->VeChnId, pastream, HI_TRUE);             
                i++;
            }
            else
            {     
                 SAMPLE_RGN_GetStream(pstVencThread->VeChnId, pstVencThread->pstream, HI_TRUE);
                 i++;
            }
        }
    }
    printf("\nchn %d finish!\n", pstVencThread->VeChnId);
    
    return HI_NULL;
}


/******************************************************************************
 * function : send stream to vdec
 ******************************************************************************/
void* SAMPLE_RGN_VdecSendStream(void* p)
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
	sprintf(sFileName, "stream_chn0%s", sFilePostfix);
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


HI_VOID *SAMPLE_RGN_VpssOsdReverse(void *pData)
{
    HI_S32 i = 0, j = 0;
    RGN_HANDLE Handle;
    TDE2_SURFACE_S stRgnOrignSurface = {0};
    TDE2_SURFACE_S stRgnSurface = {0};
    RGN_CANVAS_INFO_S stCanvasInfo;
    TDE_HANDLE hTde;
    TDE2_RECT_S stRect = {0};
    VPSS_REGION_INFO_S stReverseRgnInfo;
    HI_U32 au32LumaData[OSD_REVERSE_RGN_MAXCNT];
    RECT_S astOsdRevRect[OSD_REVERSE_RGN_MAXCNT];
    RGN_OSD_REVERSE_INFO_S *pstOsdReverseInfo;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 *pu32VirAddr = NULL;
    MPP_CHN_S stMppChn =  {0};
    RGN_CHN_ATTR_S stOsdChnAttr = {0};

    HI_ASSERT(NULL != pData);
    pstOsdReverseInfo = (RGN_OSD_REVERSE_INFO_S*)pData;
    Handle = pstOsdReverseInfo->Handle;
    HI_ASSERT(OSD_REVERSE_RGN_MAXCNT >= pstOsdReverseInfo->stLumaRgnInfo.u32RegionNum);

    srand(time(NULL));

    /* 1.get current osd info */
    s32Ret = HI_MPI_RGN_GetCanvasInfo(Handle, &stCanvasInfo);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_RGN_GetCanvasInfo fail! s32Ret: 0x%x.\n", s32Ret);
        return NULL;
    }
    
    s32Ret = HI_MPI_RGN_UpdateCanvas(Handle);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_RGN_UpdateCanvas fail! s32Ret: 0x%x.\n", s32Ret);
        return NULL;
    }

    /* 2.make a backup of current osd */
    s32Ret = SAMPLE_RGN_ConvOsdCavasToTdeSurface(&stRgnSurface, &stCanvasInfo);
    if (HI_SUCCESS != s32Ret)
    {
        printf("Func: %s, line: %d! s32Ret: 0x%x.\n", __FUNCTION__, __LINE__, s32Ret);
        return NULL;
    }
    
    memcpy(&stRgnOrignSurface, &stRgnSurface, sizeof(stRgnOrignSurface));
    
    s32Ret = HI_MPI_SYS_MmzAlloc(&stRgnOrignSurface.u32PhyAddr, (void **)(&pu32VirAddr), 
        NULL, NULL, stRgnSurface.u32Stride*stRgnSurface.u32Height);
    if (HI_SUCCESS != s32Ret)
    {
        HI_MPI_SYS_MmzFree(stRgnOrignSurface.u32PhyAddr, pu32VirAddr);
        return NULL;
    }

    s32Ret = HI_TDE2_Open();
    if (HI_SUCCESS != s32Ret)
    {
        HI_MPI_SYS_MmzFree(stRgnOrignSurface.u32PhyAddr, pu32VirAddr);
        return NULL;
    }
    
    hTde = HI_TDE2_BeginJob();
    HI_ASSERT(hTde >= 0);
    stRect.u32Width  = stRgnSurface.u32Width;
    stRect.u32Height = stRgnSurface.u32Height;
    s32Ret = HI_TDE2_QuickCopy(hTde, &stRgnSurface, &stRect, &stRgnOrignSurface, &stRect);
    if (HI_SUCCESS != s32Ret)    
    {
        printf("[Func]:%s [Line]:%d [Info]:HI_TDE2_QuickCopy failed\n", __FUNCTION__, __LINE__);
        HI_TDE2_CancelJob(hTde);
        HI_MPI_SYS_MmzFree(stRgnOrignSurface.u32PhyAddr, pu32VirAddr);
        return NULL;
    }
    
    s32Ret = HI_TDE2_EndJob(hTde, HI_FALSE, HI_FALSE, 10);
    if (HI_SUCCESS != s32Ret)
    {
        HI_TDE2_CancelJob(hTde);
        HI_MPI_SYS_MmzFree(stRgnOrignSurface.u32PhyAddr, pu32VirAddr);
        return NULL;
    }
    s32Ret = HI_TDE2_WaitForDone(hTde);
    if (HI_SUCCESS != s32Ret)
    {
        HI_MPI_SYS_MmzFree(stRgnOrignSurface.u32PhyAddr, pu32VirAddr);
        return NULL;
    }

    /* 3.get the  display attribute of OSD attached to vpss*/
    stMppChn.enModId  = HI_ID_VPSS;
    stMppChn.s32DevId = pstOsdReverseInfo->VpssGrp;
    stMppChn.s32ChnId = 0;
    s32Ret = HI_MPI_RGN_GetDisplayAttr(Handle, &stMppChn, &stOsdChnAttr);
    if (HI_SUCCESS != s32Ret)
    {
        HI_MPI_SYS_MmzFree(stRgnOrignSurface.u32PhyAddr, pu32VirAddr);
        return NULL;
    }
    
    stReverseRgnInfo.pstRegion = (RECT_S *)astOsdRevRect;
    while (HI_FALSE == bExit)
    {
        /* 4.get the sum of luma of a region specified by user*/
        s32Ret = HI_MPI_VPSS_GetGrpRegionLuma(pstOsdReverseInfo->VpssGrp, &(pstOsdReverseInfo->stLumaRgnInfo), au32LumaData, 200);
        if (HI_SUCCESS != s32Ret)
        {
            printf("[Func]:%s [Line]:%d [Info]:HI_MPI_VPSS_GetRegionLuma VpssGrp=%d failed, s32Ret: 0x%x.\n", 
                __FUNCTION__, __LINE__, pstOsdReverseInfo->VpssGrp, s32Ret);
            continue ;
        }

        /* 5.decide which region to be reverse color according to the sum of the region*/
        for (i = 0, j = 0; i < pstOsdReverseInfo->stLumaRgnInfo.u32RegionNum; ++i)
        {
            pstOsdReverseInfo->u8PerPixelLumaThrd = rand() % 256;
            
            if (au32LumaData[i] > (pstOsdReverseInfo->u8PerPixelLumaThrd * 
                pstOsdReverseInfo->stLumaRgnInfo.pstRegion[i].u32Width * 
                pstOsdReverseInfo->stLumaRgnInfo.pstRegion[i].u32Height))
            {
                /* 6.get the regions to be reverse color */
                stReverseRgnInfo.pstRegion[j].s32X = pstOsdReverseInfo->stLumaRgnInfo.pstRegion[i].s32X 
                    - stOsdChnAttr.unChnAttr.stOverlayChn.stPoint.s32X;
                stReverseRgnInfo.pstRegion[j].s32Y = pstOsdReverseInfo->stLumaRgnInfo.pstRegion[i].s32Y 
                    - stOsdChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y;
                stReverseRgnInfo.pstRegion[j].u32Width = pstOsdReverseInfo->stLumaRgnInfo.pstRegion[i].u32Width;
                stReverseRgnInfo.pstRegion[j].u32Height = pstOsdReverseInfo->stLumaRgnInfo.pstRegion[i].u32Height;
                ++j;
            }
        }
        
        stReverseRgnInfo.u32RegionNum = j;
        
        /* 7.the the canvas to be update */
        if (HI_SUCCESS != HI_MPI_RGN_GetCanvasInfo(Handle, &stCanvasInfo))
        {
            printf("[Func]:%s [Line]:%d [Info]:HI_MPI_RGN_GetCanvasInfo failed\n", __FUNCTION__, __LINE__);
            HI_MPI_SYS_MmzFree(stRgnOrignSurface.u32PhyAddr, pu32VirAddr);
            return NULL;
        }
        if (HI_SUCCESS != SAMPLE_RGN_ConvOsdCavasToTdeSurface(&stRgnSurface, &stCanvasInfo))
        {
            printf("[Func]:%s [Line]:%d [Info]:SAMPLE_RGN_ConvOsdCavasToTdeSurface failed\n", __FUNCTION__, __LINE__);
            HI_MPI_SYS_MmzFree(stRgnOrignSurface.u32PhyAddr, pu32VirAddr);
            return NULL;
        }
        /* 8.reverse color */
        if (HI_SUCCESS != SAMPLE_RGN_ReverseOsdColor(&stRgnOrignSurface, &stRgnSurface, &stReverseRgnInfo))
        {
            printf("[Func]:%s [Line]:%d [Info]:SAMPLE_RGN_ReverseOsdColor failed\n", __FUNCTION__, __LINE__);
            HI_MPI_RGN_UpdateCanvas(Handle);
            HI_MPI_SYS_MmzFree(stRgnOrignSurface.u32PhyAddr, pu32VirAddr);
            return NULL;
        }
        
        /* 9.update OSD */
        if (HI_SUCCESS != HI_MPI_RGN_UpdateCanvas(Handle))
        {
            printf("[Func]:%s [Line]:%d [Info]:HI_MPI_RGN_UpdateCanvas failed\n", __FUNCTION__, __LINE__);
            HI_MPI_SYS_MmzFree(stRgnOrignSurface.u32PhyAddr, pu32VirAddr);
            return NULL;
        }
        
        sleep(2);
    }

    HI_MPI_SYS_MmzFree(stRgnOrignSurface.u32PhyAddr, pu32VirAddr);
    return HI_NULL;
    
}


/******************************************************************************************
    function :  Vpss cover and Osd
    process steps:                                      
    0) create some cover/osd regions                
    1) attach them to vpss   
    2) start a thread to handle color reverse   
    3) change Regions' Layer                        
    4) change Regions' position                     
    5) change Regions' alpha (front and backgroud)  
******************************************************************************************/
HI_S32 SAMPLE_RGN_AddCoverAndOsdToVpss(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 u32RgnOverlayNum;
    HI_S32 u32RgnCoverNum;
    RGN_HANDLE OverlayHandle;
    RGN_HANDLE CoverHandle;
    RGN_ATTR_S stRgnAttrSet;
    RGN_CANVAS_INFO_S stCanvasInfo;
    HI_S32 i;
    BITMAP_S stBitmap;
    VDEC_CHN VdecChn;
    HI_S32 s32VpssGrpNum;
    HI_U32 u32VpssChn;
    VO_DEV VoDev;
    VO_LAYER VoLayer;
    HI_U32 s32VoChnNum;
    VO_INTF_SYNC_E enIntfSync;
    MPP_CHN_S stSrcChn, stDesChn;
    //pthread_t stRgnOsdThread;
    //pthread_t stRgnCoverThread;
    //pthread_t stVdecThread;
    //pthread_t stOsdReverseThread;
    VDEC_SENDPARAM_S stVdesSendPram;
    RGN_ATTR_INFO_S stRgnAttrInfo0;
    RGN_ATTR_INFO_S stRgnAttrInfo1;
    SIZE_S stSize;

    s32Ret = SAMPLE_RGN_SYS_Init();
    if(HI_SUCCESS != s32Ret)
    {
        printf("SAMPLE_RGN_SYS_Init failed! s32Ret: 0x%x.\n", s32Ret);
        goto END_OC_VPSS0;
    }
    
    /*************************************************
    step 1: create region and attach to vpss group
    *************************************************/
    OverlayHandle    = 0;
    u32RgnOverlayNum = OVERLAY_MAX_NUM_VPSS / 2;
    s32Ret = SAMPLE_RGN_CreateOverlayForVpss(OverlayHandle, u32RgnOverlayNum);
    if(HI_SUCCESS != s32Ret)
    {
        printf("SAMPLE_RGN_CreateOverlayForVpss failed! s32Ret: 0x%x.\n", s32Ret);
        goto END_OC_VPSS1;
    }
    CoverHandle    = OverlayHandle + u32RgnOverlayNum;
    u32RgnCoverNum = COVER_MAX_NUM_VPSS;
    s32Ret = SAMPLE_RGN_CreateCover(CoverHandle, u32RgnCoverNum);    
    if(HI_SUCCESS != s32Ret)
    {
        printf("SAMPLE_RGN_CreateCover failed! s32Ret: 0x%x.\n", s32Ret);
        goto END_OC_VPSS2;
    }
    
    /*************************************************
    step 2: start vdec chn
    *************************************************/
    VdecChn = 0;
    s32Ret = SAMPLE_RGN_StartVdec(VdecChn);
    if(HI_SUCCESS != s32Ret)
    {
        printf("SAMPLE_RGN_StartVdec failed! s32Ret: 0x%x.\n", s32Ret);
        goto END_OC_VPSS3;
    }
    
    /*************************************************
    step 3: start vpss group and chn
    *************************************************/
    s32VpssGrpNum = 1;
    u32VpssChn    = VPSS_CHN0;
    s32Ret = SAMPLE_RGN_StartVpssHD(s32VpssGrpNum, u32VpssChn);
    if(HI_SUCCESS != s32Ret)
    {
        printf("SAMPLE_RGN_StartVpssHD failed! s32Ret: 0x%x.\n", s32Ret);
        goto END_OC_VPSS4;
    }
    
    /*************************************************
    step 4: start vo dev and chn
    *************************************************/
    VoDev       = SAMPLE_VO_DEV_DHD0;
    s32VoChnNum = 1;
    enIntfSync  = VO_OUTPUT_720P60;
    s32Ret = SAMPLE_RGN_StartVo(VoDev, s32VoChnNum, enIntfSync);
    if(HI_SUCCESS != s32Ret)
    {
        printf("SAMPLE_RGN_StartVo failed! s32Ret: 0x%x.\n", s32Ret);
        goto END_OC_VPSS5;
    }

	#ifndef HI_FPGA
    if (HI_SUCCESS != SAMPLE_COMM_VO_HdmiStart(enIntfSync))
    {
        SAMPLE_PRT("Start SAMPLE_COMM_VO_HdmiStart failed!\n");
        goto END_OC_VPSS6;
    }
	#endif
    
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
        goto END_OC_VPSS7;
    }
    
    /*************************************************
    step 6: bind vpss and vo
    *************************************************/
    VoLayer = SAMPLE_RGN_GetVoLayer(VoDev);
    if(VoLayer < 0)
    {
        printf("SAMPLE_RGN_GetVoLayer failed! VoDev: %d.\n", VoDev);
        goto END_OC_VPSS7;
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
        goto END_OC_VPSS8;
    }

    /*************************************************
    step 7: load bitmap to region
    *************************************************/
    for(i=0; i<u32RgnOverlayNum; i++)
    {
        s32Ret = HI_MPI_RGN_GetAttr(OverlayHandle+i, &stRgnAttrSet);
        if(HI_SUCCESS != s32Ret)
        {
            printf("HI_MPI_RGN_GetAttr failed! s32Ret: 0x%x.\n", s32Ret);
            goto END_OC_VPSS8;
        }
        
        s32Ret = HI_MPI_RGN_GetCanvasInfo(OverlayHandle+i, &stCanvasInfo);
        if(HI_SUCCESS != s32Ret)
        {
            printf("HI_MPI_RGN_GetCanvasInfo failed! s32Ret: 0x%x.\n", s32Ret);
            goto END_OC_VPSS8;
        }
            
        stBitmap.pData   = (HI_VOID *)stCanvasInfo.u32VirtAddr;
        stSize.u32Width  = stCanvasInfo.stSize.u32Width;
        stSize.u32Height = stCanvasInfo.stSize.u32Height;
        s32Ret = SAMPLE_RGN_UpdateCanvas("mm2.bmp", &stBitmap, HI_FALSE, 0, &stSize, stCanvasInfo.u32Stride, 
            stRgnAttrSet.unAttr.stOverlay.enPixelFmt);
        if(HI_SUCCESS != s32Ret)
        {
            printf("SAMPLE_RGN_UpdateCanvas failed! s32Ret: 0x%x.\n", s32Ret);
            goto END_OC_VPSS8;
        }
        
        s32Ret = HI_MPI_RGN_UpdateCanvas(OverlayHandle+i);
        if(HI_SUCCESS != s32Ret)
        {
            printf("HI_MPI_RGN_UpdateCanvas failed! s32Ret: 0x%x.\n", s32Ret);
            goto END_OC_VPSS8;
        }
    }

    /*************************************************
    step 8: create a thread to change region's 
    layer, position, alpha and other display attribute
    handle 3 region used to invert color
    *************************************************/
    stRgnAttrInfo0.Handle    = OverlayHandle;
    stRgnAttrInfo0.u32RgnNum = u32RgnOverlayNum-1; 
    pthread_create(&g_stRgnOsdThread, NULL, SAMPLE_RGN_VpssOSdDynamicDisplay, (HI_VOID*)&stRgnAttrInfo0); 
    stRgnAttrInfo1.Handle    = CoverHandle;
    stRgnAttrInfo1.u32RgnNum = u32RgnCoverNum;
    pthread_create(&g_stRgnCoverThread, NULL, SAMPLE_RGN_VpssCoverDynamicDisplay, (HI_VOID*)&stRgnAttrInfo1);
    
    /*************************************************
    step 9: create a thread for vdec to read stream
    from a file
    *************************************************/    
    stSize.u32Width  = 1920;
    stSize.u32Height = 1080;
    
    stVdesSendPram.bRun          = HI_TRUE;
    stVdesSendPram.VdChn         = VdecChn;
    stVdesSendPram.enPayload     = PT_H264;
    stVdesSendPram.enVideoMode   = VIDEO_MODE_FRAME;
    stVdesSendPram.s32MinBufSize = stSize.u32Height * stSize.u32Width / 2;
    pthread_create(&g_stVdecThread, NULL, SAMPLE_RGN_VdecSendStream, (HI_VOID*)&stVdesSendPram);
#if 1    
    /*************************************************
    step 10: start a thread to handle osd color reverse
    *************************************************/
	RGN_OSD_REVERSE_INFO_S	stOsdReverseInfo;
	MPP_CHN_S 				stChn;
    RGN_CHN_ATTR_S 			stChnAttr;
	RECT_S					astOsdLumaRect[8];
	HI_U32 					u32OsdRectCnt;
	//pthread_t				stOsdReverseThread;
	/*handle 3 region used to invert color*/
	RGN_HANDLE				InvertColorHandle = 3;

    u32OsdRectCnt                            = 3;
    stOsdReverseInfo.Handle                  = InvertColorHandle;
    stOsdReverseInfo.VpssGrp                 = 0;
    stOsdReverseInfo.u8PerPixelLumaThrd      = 128;
    stOsdReverseInfo.stLumaRgnInfo.u32RegionNum   = u32OsdRectCnt;
    stOsdReverseInfo.stLumaRgnInfo.pstRegion = astOsdLumaRect;

	stChn.enModId  = HI_ID_VPSS;
    stChn.s32DevId = 0;
    stChn.s32ChnId = 0;
    s32Ret = HI_MPI_RGN_GetDisplayAttr(InvertColorHandle, &stChn, &stChnAttr);
    if(HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_RGN_GetDisplayAttr failed! s32Ret: 0x%x.\n", s32Ret);
        goto END_OC_VPSS9;
    }

	s32Ret = HI_MPI_RGN_GetAttr(InvertColorHandle, &stRgnAttrSet);
	if(HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_RGN_GetAttr failed! s32Ret: 0x%x.\n", s32Ret);
        goto END_OC_VPSS9;
    }
	
	stSize.u32Width  = stRgnAttrSet.unAttr.stOverlayEx.stSize.u32Width;
    stSize.u32Height = stRgnAttrSet.unAttr.stOverlayEx.stSize.u32Height;
        
    for (i=0; i <u32OsdRectCnt; i++)
    {
        astOsdLumaRect[i].s32X = ((stSize.u32Width/ u32OsdRectCnt) * i) + stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X;
        astOsdLumaRect[i].s32Y = stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y;
        astOsdLumaRect[i].u32Width  = (stSize.u32Width / u32OsdRectCnt);
        astOsdLumaRect[i].u32Height = stSize.u32Height;
    }
    pthread_create(&g_stOsdReverseThread, NULL, SAMPLE_RGN_VpssOsdReverse, (HI_VOID*)&stOsdReverseInfo);

 #endif   

    printf("\n#############Sample start ok! Press Enter to switch!#############\n");

    /*************************************************
    step 11: stop thread and release all the resource
    *************************************************/
    getchar();
	
END_OC_VPSS9:
    bExit = HI_TRUE;
    //pthread_join(stOsdReverseThread, 0);
    pthread_join(g_stVdecThread, 0);
    pthread_join(g_stRgnOsdThread, 0);
    pthread_join(g_stRgnCoverThread, 0);
    pthread_join(g_stOsdReverseThread, 0);
    bExit = HI_FALSE;

END_OC_VPSS8:
    stSrcChn.enModId  = HI_ID_VPSS;
    stSrcChn.s32DevId = 0;
    stSrcChn.s32ChnId = u32VpssChn;

    stDesChn.enModId  = HI_ID_VOU;
    stDesChn.s32DevId = VoLayer;
    stDesChn.s32ChnId = 0;
    HI_MPI_SYS_UnBind(&stSrcChn, &stDesChn);
END_OC_VPSS7:
    stSrcChn.enModId  = HI_ID_VDEC;
    stSrcChn.s32DevId = 0;
    stSrcChn.s32ChnId = 0;

    stDesChn.enModId  = HI_ID_VPSS;
    stDesChn.s32DevId = 0;
    stDesChn.s32ChnId = 0;
    HI_MPI_SYS_UnBind(&stSrcChn, &stDesChn);

#ifndef HI_FPGA	
END_OC_VPSS6:
    SAMPLE_COMM_VO_HdmiStop();
#endif

END_OC_VPSS5:
    SAMPLE_RGN_StopVoChn(VoDev, s32VoChnNum);
    SAMPLE_RGN_StopVoDev(VoDev);

END_OC_VPSS4:
    SAMPLE_RGN_StopVpss(s32VpssGrpNum);

END_OC_VPSS3:
    SAMPLE_RGN_StopVdec(VdecChn);

END_OC_VPSS2:
    SAMPLE_RGN_DestroyRegion(OverlayHandle, u32RgnOverlayNum);
    
END_OC_VPSS1:    
    SAMPLE_RGN_DestroyRegion(CoverHandle, u32RgnCoverNum);
    
END_OC_VPSS0:    
    SAMPLE_COMM_SYS_Exit();

    return HI_SUCCESS;
}

HI_S32 SAMPLE_RGN_AddCoverOsdAndLineToVpss(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 u32OverlayexRgnNum;
    HI_S32 u32CoverexRgnNum;
    HI_S32 u32LineRgnNum;
    RGN_HANDLE OverlayexHandle;
    RGN_HANDLE CoverexHandle;
    RGN_HANDLE LineHandle;
    RGN_ATTR_S stRgnAttrSet;
    RGN_CANVAS_INFO_S stCanvasInfo;
    HI_S32 i;
    BITMAP_S stBitmap;
    VDEC_CHN VdecChn;
    HI_S32 s32VpssGrpNum;
    HI_U32 u32VpssChn;
    VO_DEV VoDev;
    VO_LAYER VoLayer;
    HI_U32 s32VoChnNum;
    VO_INTF_SYNC_E enIntfSync;
    MPP_CHN_S stSrcChn, stDesChn;
    //pthread_t stRgnOsdThread;
    //pthread_t stRgnCoverThread;
    //pthread_t stVdecThread;
    VDEC_SENDPARAM_S stVdesSendPram;
    RGN_ATTR_INFO_S stRgnAttrInfo0;
    RGN_ATTR_INFO_S stRgnAttrInfo1;
    SIZE_S stSize;
    HI_U32 ChnID;

    s32Ret = SAMPLE_RGN_SYS_Init();
    if(HI_SUCCESS != s32Ret)
    {
        printf("SAMPLE_RGN_SYS_Init failed! s32Ret: 0x%x.\n", s32Ret);
        goto END_OCL_VPSS0;
    }
    
    /*************************************************
    step 1: create region and attach to vpss group
    *************************************************/
    OverlayexHandle = 0;
    u32OverlayexRgnNum = OVERLAY_MAX_NUM_VPSS / 2;
    ChnID = VPSS_CHN0;

    s32Ret = SAMPLE_RGN_CreateOverlayex(OverlayexHandle, ChnID, u32OverlayexRgnNum, HI_ID_VPSS);    
    if(HI_SUCCESS != s32Ret)
    {
        printf("SAMPLE_RGN_CreateOverlayex failed! s32Ret: 0x%x.\n", s32Ret);
        goto END_OCL_VPSS1;
    }

    CoverexHandle = OverlayexHandle + u32OverlayexRgnNum;
    u32CoverexRgnNum = COVER_MAX_NUM_VPSS;
    ChnID = VPSS_CHN0;
    
    s32Ret = SAMPLE_RGN_CreateCoverex(CoverexHandle, ChnID, u32CoverexRgnNum, HI_ID_VPSS);    
    if(HI_SUCCESS != s32Ret)
    {
        printf("SAMPLE_RGN_CreateOverlayex failed! s32Ret: 0x%x.\n", s32Ret);
        goto END_OCL_VPSS2;
    }

    LineHandle    = CoverexHandle + u32CoverexRgnNum;
    u32LineRgnNum = LINE_MAX_NUM_VPSS;
    ChnID = VPSS_CHN0;
    
    s32Ret = SAMPLE_RGN_CreateLine(LineHandle, ChnID, u32LineRgnNum, HI_ID_VPSS);    
    if(HI_SUCCESS != s32Ret)
    {
        printf("SAMPLE_RGN_CreateOverlayex failed! s32Ret: 0x%x.\n", s32Ret);
        goto END_OCL_VPSS3;
    }

    /*************************************************
    step 2: start vdec chn
    *************************************************/
    VdecChn = 0;
    s32Ret = SAMPLE_RGN_StartVdec(VdecChn);
    if(HI_SUCCESS != s32Ret)
    {
        printf("SAMPLE_RGN_StartVdec failed! s32Ret: 0x%x.\n", s32Ret);
        goto END_OCL_VPSS4;
    }
    
    /*************************************************
    step 3: start vpss group and chn
    *************************************************/
    s32VpssGrpNum = 1;
    u32VpssChn    = VPSS_CHN0;
    s32Ret = SAMPLE_RGN_StartVpssHD(s32VpssGrpNum, u32VpssChn);
    if(HI_SUCCESS != s32Ret)
    {
        printf("SAMPLE_RGN_StartVpssHD failed! s32Ret: 0x%x.\n", s32Ret);
        goto END_OCL_VPSS5;
    }
    
    /*************************************************
    step 4: start vo dev and chn
    *************************************************/
    VoDev       = SAMPLE_VO_DEV_DHD0;
    s32VoChnNum = 1;
    enIntfSync  = VO_OUTPUT_720P60;
    s32Ret = SAMPLE_RGN_StartVo(VoDev, s32VoChnNum, enIntfSync);
    if(HI_SUCCESS != s32Ret)
    {
        printf("SAMPLE_RGN_StartVo failed! s32Ret: 0x%x.\n", s32Ret);
        goto END_OCL_VPSS6;
    }
	
	#ifndef HI_FPGA
    if (HI_SUCCESS != SAMPLE_COMM_VO_HdmiStart(enIntfSync))
    {
        SAMPLE_PRT("Start SAMPLE_COMM_VO_HdmiStart failed!\n");
        goto END_OCL_VPSS7;
    }
	#endif

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
        goto END_OCL_VPSS8;
    }
    
    /*************************************************
    step 6: bind vpss and vo
    *************************************************/
    VoLayer = SAMPLE_RGN_GetVoLayer(VoDev);
    if(VoLayer < 0)
    {
        printf("SAMPLE_RGN_GetVoLayer failed! VoDev: %d.\n", VoDev);
        goto END_OCL_VPSS8;
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
        goto END_OCL_VPSS9;
    }

    /*************************************************
    step 7: load bitmap to region
    *************************************************/
    for(i=0; i<u32OverlayexRgnNum; i++)
    {
        s32Ret = HI_MPI_RGN_GetAttr(OverlayexHandle+i, &stRgnAttrSet);
        if(HI_SUCCESS != s32Ret)
        {
            printf("HI_MPI_RGN_GetAttr failed! s32Ret: 0x%x.\n", s32Ret);
            goto END_OCL_VPSS9;
        }
        
        s32Ret = HI_MPI_RGN_GetCanvasInfo(OverlayexHandle+i, &stCanvasInfo);
        if(HI_SUCCESS != s32Ret)
        {
            printf("HI_MPI_RGN_GetCanvasInfo failed! s32Ret: 0x%x.\n", s32Ret);
            goto END_OCL_VPSS9;
        }
            
        stBitmap.pData   = (HI_VOID *)stCanvasInfo.u32VirtAddr;
        stSize.u32Width  = stCanvasInfo.stSize.u32Width;
        stSize.u32Height = stCanvasInfo.stSize.u32Height;
        s32Ret = SAMPLE_RGN_UpdateCanvas("mm2.bmp", &stBitmap, HI_FALSE, 0, &stSize, stCanvasInfo.u32Stride, 
            stRgnAttrSet.unAttr.stOverlay.enPixelFmt);
        if(HI_SUCCESS != s32Ret)
        {
            printf("SAMPLE_RGN_UpdateCanvas failed! s32Ret: 0x%x.\n", s32Ret);
            goto END_OCL_VPSS9;
        }
        
        s32Ret = HI_MPI_RGN_UpdateCanvas(OverlayexHandle+i);
        if(HI_SUCCESS != s32Ret)
        {
            printf("HI_MPI_RGN_UpdateCanvas failed! s32Ret: 0x%x.\n", s32Ret);
            goto END_OCL_VPSS9;
        }
    }

    /*************************************************
    step 8: create a thread to change region's 
    layer, position, alpha and other display attribute
    *************************************************/
    stRgnAttrInfo0.Handle    = OverlayexHandle;
    stRgnAttrInfo0.u32RgnNum = u32OverlayexRgnNum; 
    pthread_create(&g_stRgnCoverThread, NULL, SAMPLE_RGN_VpssOverlayexDynamicDisplay, (HI_VOID*)&stRgnAttrInfo0); 
    stRgnAttrInfo1.Handle    = CoverexHandle;
    stRgnAttrInfo1.u32RgnNum = u32CoverexRgnNum;
    pthread_create(&g_stRgnOsdThread, NULL, SAMPLE_RGN_VpssCoverexDynamicDisplay, (HI_VOID*)&stRgnAttrInfo1);
    
    /*************************************************
    step 9: create a thread for vdec to read stream
    from a file
    *************************************************/    
    stSize.u32Width  = 1920;
    stSize.u32Height = 1080;
    
    stVdesSendPram.bRun          = HI_TRUE;
    stVdesSendPram.VdChn         = VdecChn;
    stVdesSendPram.enPayload     = PT_H264;
    stVdesSendPram.enVideoMode   = VIDEO_MODE_FRAME;
    stVdesSendPram.s32MinBufSize = stSize.u32Height * stSize.u32Width / 2;
    pthread_create(&g_stVdecThread, NULL, SAMPLE_RGN_VdecSendStream, (HI_VOID*)&stVdesSendPram);

    printf("\n#############Sample start ok! Press Enter to switch!#############\n");
    

    /*************************************************
    step 11: stop thread and release all the resource
    *************************************************/
    getchar();
    bExit = HI_TRUE;

    pthread_join(g_stVdecThread, 0);

    pthread_join(g_stRgnOsdThread, 0);

    pthread_join(g_stRgnCoverThread, 0);
    
    bExit = HI_FALSE;
    
END_OCL_VPSS9:
    stSrcChn.enModId  = HI_ID_VPSS;
    stSrcChn.s32DevId = 0;
    stSrcChn.s32ChnId = u32VpssChn;

    stDesChn.enModId  = HI_ID_VOU;
    stDesChn.s32DevId = VoLayer;
    stDesChn.s32ChnId = 0;
    HI_MPI_SYS_UnBind(&stSrcChn, &stDesChn);
    
END_OCL_VPSS8:
    stSrcChn.enModId  = HI_ID_VDEC;
    stSrcChn.s32DevId = 0;
    stSrcChn.s32ChnId = 0;

    stDesChn.enModId  = HI_ID_VPSS;
    stDesChn.s32DevId = 0;
    stDesChn.s32ChnId = 0;
    
    HI_MPI_SYS_UnBind(&stSrcChn, &stDesChn);

#ifndef HI_FPGA	
END_OCL_VPSS7:
	SAMPLE_COMM_VO_HdmiStop();
#endif

END_OCL_VPSS6:
    SAMPLE_RGN_StopVoChn(VoDev, s32VoChnNum);
    SAMPLE_RGN_StopVoDev(VoDev);
    
END_OCL_VPSS5:
    SAMPLE_RGN_StopVpss(s32VpssGrpNum);
    
END_OCL_VPSS4:
    SAMPLE_RGN_StopVdec(VdecChn);
    
END_OCL_VPSS3:
    SAMPLE_RGN_DestroyRegion(OverlayexHandle, u32OverlayexRgnNum);
    
END_OCL_VPSS2:    
    SAMPLE_RGN_DestroyRegion(CoverexHandle, u32CoverexRgnNum);
    
END_OCL_VPSS1:
    SAMPLE_RGN_DestroyRegion(LineHandle, u32LineRgnNum);

END_OCL_VPSS0:
   SAMPLE_COMM_SYS_Exit();
    
    return HI_SUCCESS;
}


/******************************************************************************************
    function :  Vpss mosaic
    process steps:                                      
    0) create some mosaic regions                
    1) attach them to vpss   
    2) start a thread to handle move mosaic position   
******************************************************************************************/
HI_S32 SAMPLE_RGN_AddMosaicToVpss(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 u32MosaicNum;
    RGN_HANDLE MosaicHandle;
    VDEC_CHN VdecChn;
    HI_S32 s32VpssGrpNum;
    HI_U32 u32VpssChn;
    VO_DEV VoDev;
    VO_LAYER VoLayer;
    HI_U32 s32VoChnNum;
    VO_INTF_SYNC_E enIntfSync;
    MPP_CHN_S stSrcChn, stDesChn;
    //pthread_t stRgnMosaicThread;
    //pthread_t stVdecThread;

    VDEC_SENDPARAM_S stVdesSendPram;
    RGN_ATTR_INFO_S stRgnAttrInfo0;
    SIZE_S stSize;

    s32Ret = SAMPLE_RGN_SYS_Init();
    if(HI_SUCCESS != s32Ret)
    {
        printf("SAMPLE_RGN_SYS_Init failed! s32Ret: 0x%x.\n", s32Ret);
        goto END_OC_VPSS0;
    }
    
    /*************************************************
    step 1: create region and attach to vpss group
    *************************************************/
    MosaicHandle    = 0;
    u32MosaicNum = MASAIC_MAX_NUM_VPSS;
    s32Ret = SAMPLE_RGN_CreateMosaicForVpss(MosaicHandle, u32MosaicNum);
    if(HI_SUCCESS != s32Ret)
    {
        printf("SAMPLE_RGN_CreateOverlayForVpss failed! s32Ret: 0x%x.\n", s32Ret);
        goto END_OC_VPSS1;
    }
    
    /*************************************************
    step 2: start vdec chn
    *************************************************/
    VdecChn = 0;
    s32Ret = SAMPLE_RGN_StartVdec(VdecChn);
    if(HI_SUCCESS != s32Ret)
    {
        printf("SAMPLE_RGN_StartVdec failed! s32Ret: 0x%x.\n", s32Ret);
        goto END_OC_VPSS2;
    }
    
    /*************************************************
    step 3: start vpss group and chn
    *************************************************/
    s32VpssGrpNum = 1;
    u32VpssChn    = VPSS_CHN0;
    s32Ret = SAMPLE_RGN_StartVpssHD(s32VpssGrpNum, u32VpssChn);
    if(HI_SUCCESS != s32Ret)
    {
        printf("SAMPLE_RGN_StartVpssHD failed! s32Ret: 0x%x.\n", s32Ret);
        goto END_OC_VPSS3;
    }
    
    /*************************************************
    step 4: start vo dev and chn
    *************************************************/
    VoDev       = SAMPLE_VO_DEV_DHD0;
    s32VoChnNum = 1;
    enIntfSync  = VO_OUTPUT_720P60;
    s32Ret = SAMPLE_RGN_StartVo(VoDev, s32VoChnNum, enIntfSync);
    if(HI_SUCCESS != s32Ret)
    {
        printf("SAMPLE_RGN_StartVo failed! s32Ret: 0x%x.\n", s32Ret);
        goto END_OC_VPSS4;
    }

	#ifndef HI_FPGA
    if (HI_SUCCESS != SAMPLE_COMM_VO_HdmiStart(enIntfSync))
    {
        SAMPLE_PRT("Start SAMPLE_COMM_VO_HdmiStart failed!\n");
        goto END_OC_VPSS5;
    }
	#endif

    
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
        goto END_OC_VPSS6;
    }
    
    /*************************************************
    step 6: bind vpss and vo
    *************************************************/
    VoLayer = SAMPLE_RGN_GetVoLayer(VoDev);
    if(VoLayer < 0)
    {
        printf("SAMPLE_RGN_GetVoLayer failed! VoDev: %d.\n", VoDev);
        goto END_OC_VPSS6;
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
        goto END_OC_VPSS7;
    }

    /*************************************************
    step 7: create a thread to change region's 
    layer, position, and other display attribute
    *************************************************/
    stRgnAttrInfo0.Handle    = MosaicHandle;
    stRgnAttrInfo0.u32RgnNum = u32MosaicNum; 
    pthread_create(&g_stRgnMosaicThread, NULL, SAMPLE_RGN_VpssMosaicDisplay, (HI_VOID*)&stRgnAttrInfo0); 
    
    /*************************************************
    step 8: create a thread for vdec to read stream
    from a file
    *************************************************/    
    stSize.u32Width  = 1920;
    stSize.u32Height = 1080;
    
    stVdesSendPram.bRun          = HI_TRUE;
    stVdesSendPram.VdChn         = VdecChn;
    stVdesSendPram.enPayload     = PT_H264;
    stVdesSendPram.enVideoMode   = VIDEO_MODE_FRAME;
    stVdesSendPram.s32MinBufSize = stSize.u32Height * stSize.u32Width / 2;
    pthread_create(&g_stVdecThread, NULL, SAMPLE_RGN_VdecSendStream, (HI_VOID*)&stVdesSendPram);

    printf("\n#############Sample start ok! Press Enter to switch!#############\n");

    /*************************************************
    step 9: stop thread and release all the resource
    *************************************************/
    getchar();
	
    bExit = HI_TRUE;
    pthread_join(g_stVdecThread, 0);
    pthread_join(g_stRgnMosaicThread, 0);
    bExit = HI_FALSE;

END_OC_VPSS7:
    stSrcChn.enModId  = HI_ID_VPSS;
    stSrcChn.s32DevId = 0;
    stSrcChn.s32ChnId = u32VpssChn;

    stDesChn.enModId  = HI_ID_VOU;
    stDesChn.s32DevId = VoLayer;
    stDesChn.s32ChnId = 0;
    HI_MPI_SYS_UnBind(&stSrcChn, &stDesChn);
END_OC_VPSS6:
    stSrcChn.enModId  = HI_ID_VDEC;
    stSrcChn.s32DevId = 0;
    stSrcChn.s32ChnId = 0;

    stDesChn.enModId  = HI_ID_VPSS;
    stDesChn.s32DevId = 0;
    stDesChn.s32ChnId = 0;
    HI_MPI_SYS_UnBind(&stSrcChn, &stDesChn);

#ifndef HI_FPGA
END_OC_VPSS5:
	SAMPLE_COMM_VO_HdmiStop();
#endif

END_OC_VPSS4:
    SAMPLE_RGN_StopVoChn(VoDev, s32VoChnNum);
    SAMPLE_RGN_StopVoDev(VoDev);

END_OC_VPSS3:
    SAMPLE_RGN_StopVpss(s32VpssGrpNum);

END_OC_VPSS2:
    SAMPLE_RGN_StopVdec(VdecChn);

END_OC_VPSS1:
    SAMPLE_RGN_DestroyRegion(MosaicHandle, u32MosaicNum);
    
END_OC_VPSS0:    
    SAMPLE_COMM_SYS_Exit();

    return HI_SUCCESS;
}

HI_S32 SAMPLE_RGN_AddCoverOsdAndLineToVo(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 u32OverlayexRgnNum;
    HI_S32 u32CoverexRgnNum;
    HI_S32 u32LineRgnNum;
    RGN_HANDLE OverlayexHandle;
    RGN_HANDLE CoverexHandle;
    RGN_HANDLE LineHandle;
    RGN_ATTR_S stRgnAttrSet;
    RGN_CANVAS_INFO_S stCanvasInfo;
    BITMAP_S stBitmap;
    VDEC_CHN VdecChn;
    VO_DEV VoDev;
    VO_LAYER VoLayer;
    HI_U32 s32VoChnNum;
    VO_INTF_SYNC_E enIntfSync;
    MPP_CHN_S stSrcChn, stDesChn;
    //pthread_t stRgnOsdThread;
    //pthread_t stRgnCoverThread;
    //pthread_t stVdecThread;
    VDEC_SENDPARAM_S stVdesSendPram;
    RGN_ATTR_INFO_S stRgnAttrInfo0;
    RGN_ATTR_INFO_S stRgnAttrInfo1;
    SIZE_S stSize;
    HI_U32 ChnID;
    HI_U32 i;

    s32Ret = SAMPLE_RGN_SYS_Init();
    if(HI_SUCCESS != s32Ret)
    {
        printf("SAMPLE_RGN_SYS_Init failed! s32Ret: 0x%x.\n", s32Ret);
        goto END_OCL_VOU0;
    }
    
    /*************************************************
    step 1: create region and attach to vpss group
    *************************************************/
    OverlayexHandle    = 0;
    u32OverlayexRgnNum = OVERLAYEX_MAX_NUM_VO/ 2;
    ChnID = 0;

    s32Ret = SAMPLE_RGN_CreateOverlayex(OverlayexHandle, ChnID, u32OverlayexRgnNum, HI_ID_VOU);    
    if(HI_SUCCESS != s32Ret)
    {
        printf("SAMPLE_RGN_CreateOverlayex failed! s32Ret: 0x%x.\n", s32Ret);
        goto END_OCL_VOU1;
    }
    CoverexHandle    = OverlayexHandle + u32OverlayexRgnNum;
    u32CoverexRgnNum = COVEREX_MAX_NUM_VO;
    ChnID = 0;
    
    s32Ret = SAMPLE_RGN_CreateCoverex(CoverexHandle, ChnID, u32CoverexRgnNum, HI_ID_VOU);    
    if(HI_SUCCESS != s32Ret)
    {
        printf("SAMPLE_RGN_CreateOverlayex failed! s32Ret: 0x%x.\n", s32Ret);
        goto END_OCL_VOU2;
    }
    LineHandle    = CoverexHandle + u32CoverexRgnNum;
    u32LineRgnNum = LINE_MAX_NUM_VO;
    ChnID = 0;
    
    s32Ret = SAMPLE_RGN_CreateLine(LineHandle, ChnID, u32LineRgnNum, HI_ID_VOU);    
    if(HI_SUCCESS != s32Ret)
    {
        printf("SAMPLE_RGN_CreateLine failed! s32Ret: 0x%x.\n", s32Ret);
        goto END_OCL_VOU3;
    }

    /*************************************************
    step 2: start vdec chn
    *************************************************/
    VdecChn = 0;
    s32Ret = SAMPLE_RGN_StartVdec(VdecChn);
    if(HI_SUCCESS != s32Ret)
    {
        printf("SAMPLE_RGN_StartVdec failed! s32Ret: 0x%x.\n", s32Ret);
        goto END_OCL_VOU4;
    }

    /*************************************************
    step 3: start vo dev and chn
    *************************************************/
    VoDev       = SAMPLE_VO_DEV_DHD0;
    s32VoChnNum = 1;
    enIntfSync  = VO_OUTPUT_720P60;
    s32Ret = HI_MPI_VO_SetVideoLayerPartitionMode(SAMPLE_VO_LAYER_VHD0, VO_PART_MODE_SINGLE);
    if(HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_SYS_Bind failed! s32Ret: 0x%x.\n", s32Ret);
        goto END_OCL_VOU4;
    }
    s32Ret = SAMPLE_RGN_StartVo(VoDev, s32VoChnNum, enIntfSync);
    if(HI_SUCCESS != s32Ret)
    {
        printf("SAMPLE_RGN_StartVo failed! s32Ret: 0x%x.\n", s32Ret);
        goto END_OCL_VOU5;
    }

	#ifndef HI_FPGA
    if (HI_SUCCESS != SAMPLE_COMM_VO_HdmiStart(enIntfSync))
    {
        SAMPLE_PRT("Start SAMPLE_COMM_VO_HdmiStart failed!\n");
        goto END_OCL_VOU6;
    }
	#endif

    /*************************************************
    step 4: bind vdec and vo
    *************************************************/
    VoLayer = SAMPLE_RGN_GetVoLayer(VoDev);
    if(VoLayer < 0)
    {
        printf("SAMPLE_RGN_GetVoLayer failed! VoDev: %d.\n", VoDev);
        goto END_OCL_VOU6;
    }
    
    stSrcChn.enModId  = HI_ID_VDEC;
    stSrcChn.s32DevId = 0;
    stSrcChn.s32ChnId = 0;

    stDesChn.enModId  = HI_ID_VOU;
    stDesChn.s32DevId = VoLayer;
    stDesChn.s32ChnId = 0;

    s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDesChn);
    if(HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_SYS_Bind failed! s32Ret: 0x%x.\n", s32Ret);
        goto END_OCL_VOU7;
    }
    /*************************************************
    step 5: load bitmap to region
    *************************************************/
    for(i=0; i<u32OverlayexRgnNum; i++)
    {
        s32Ret = HI_MPI_RGN_GetAttr(OverlayexHandle+i, &stRgnAttrSet);
        if(HI_SUCCESS != s32Ret)
        {
            printf("HI_MPI_RGN_GetAttr failed! s32Ret: 0x%x.\n", s32Ret);
            goto END_OCL_VOU7;
        }
        
        s32Ret = HI_MPI_RGN_GetCanvasInfo(OverlayexHandle+i, &stCanvasInfo);
        if(HI_SUCCESS != s32Ret)
        {
            printf("HI_MPI_RGN_GetCanvasInfo failed! s32Ret: 0x%x.\n", s32Ret);
            goto END_OCL_VOU7;
        }
            
        stBitmap.pData   = (HI_VOID *)stCanvasInfo.u32VirtAddr;
        stSize.u32Width  = stCanvasInfo.stSize.u32Width;
        stSize.u32Height = stCanvasInfo.stSize.u32Height;
        s32Ret = SAMPLE_RGN_UpdateCanvas("mm2.bmp", &stBitmap, HI_FALSE, 0, &stSize, stCanvasInfo.u32Stride, 
            stRgnAttrSet.unAttr.stOverlay.enPixelFmt);
        if(HI_SUCCESS != s32Ret)
        {
            printf("SAMPLE_RGN_UpdateCanvas failed! s32Ret: 0x%x.\n", s32Ret);
            goto END_OCL_VOU7;
        }
        
        s32Ret = HI_MPI_RGN_UpdateCanvas(OverlayexHandle+i);
        if(HI_SUCCESS != s32Ret)
        {
            printf("HI_MPI_RGN_UpdateCanvas failed! s32Ret: 0x%x.\n", s32Ret);
            goto END_OCL_VOU7;
        }
    }
    /*************************************************
    step 6: create a thread to change region's 
    layer, position, alpha and other display attribute
    *************************************************/
    stRgnAttrInfo0.Handle    = OverlayexHandle;
    stRgnAttrInfo0.u32RgnNum = u32OverlayexRgnNum; 
    pthread_create(&g_stRgnCoverThread, NULL, SAMPLE_RGN_VouOverlayexDynamicDisplay, (HI_VOID*)&stRgnAttrInfo0); 
    stRgnAttrInfo1.Handle    = CoverexHandle;
    stRgnAttrInfo1.u32RgnNum = u32CoverexRgnNum;
    pthread_create(&g_stRgnOsdThread, NULL, SAMPLE_RGN_VouCoverexDynamicDisplay, (HI_VOID*)&stRgnAttrInfo1);
    
    /*************************************************
    step 7: create a thread for vdec to read stream
    from a file
    *************************************************/    
    stSize.u32Width  = 1920;
    stSize.u32Height = 1080;
    
    stVdesSendPram.bRun          = HI_TRUE;
    stVdesSendPram.VdChn         = VdecChn;
    stVdesSendPram.enPayload     = PT_H264;
    stVdesSendPram.enVideoMode   = VIDEO_MODE_FRAME;
    stVdesSendPram.s32MinBufSize = stSize.u32Height * stSize.u32Width / 2;
    pthread_create(&g_stVdecThread, NULL, SAMPLE_RGN_VdecSendStream, (HI_VOID*)&stVdesSendPram);
    
    printf("\n#############Sample start ok! Press Enter to switch!#############\n");
    /*************************************************
    step 8: stop thread and release all the resource
    *************************************************/
    getchar();
    bExit = HI_TRUE;

    pthread_join(g_stVdecThread, 0);

    pthread_join(g_stRgnOsdThread, 0);

    pthread_join(g_stRgnCoverThread, 0);
    
    bExit = HI_FALSE;
    
END_OCL_VOU7:
    stSrcChn.enModId  = HI_ID_VDEC;
    stSrcChn.s32DevId = 0;
    stSrcChn.s32ChnId = 0;

    stDesChn.enModId  = HI_ID_VPSS;
    stDesChn.s32DevId = 0;
    stDesChn.s32ChnId = 0;      
    HI_MPI_SYS_UnBind(&stSrcChn, &stDesChn);

#ifndef HI_FPGA
END_OCL_VOU6:
	SAMPLE_COMM_VO_HdmiStop();
#endif

END_OCL_VOU5:
    SAMPLE_RGN_StopVoChn(VoDev, s32VoChnNum);
    SAMPLE_RGN_StopVoDev(VoDev);
    
END_OCL_VOU4:
    SAMPLE_RGN_StopVdec(VdecChn);

END_OCL_VOU3:
    SAMPLE_RGN_DestroyRegion(OverlayexHandle, u32OverlayexRgnNum);

END_OCL_VOU2:    
    SAMPLE_RGN_DestroyRegion(CoverexHandle, u32CoverexRgnNum);

END_OCL_VOU1:
    SAMPLE_RGN_DestroyRegion(LineHandle, u32LineRgnNum);
    
END_OCL_VOU0:    
    SAMPLE_RGN_SYS_Exit();
    
    return HI_SUCCESS;
}

/******************************************************************************************
    function :  Venc OSD
    process steps:                                                          
      1) create some cover/osd regions                                  
      2) display  cover/osd regions ( One Region -- Multi-VencGroup )   
      3) change all vencGroups Regions' Layer                           
      4) change all vencGroups Regions' position                        
      5) change all vencGroups Regions' color                           
      6) change all vencGroups Regions' alpha (front and backgroud)     
      7) load bmp form bmp-file to Region-0                             
      8) change BmpRegion-0   
      9) enable color reverse for Region-0
******************************************************************************************/

HI_S32 SAMPLE_RGN_AddOsdToVenc(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    RGN_HANDLE OverlayHandle;
    HI_S32 u32OverlayRgnNum;
    MPP_CHN_S stSrcChn, stDesChn;
    RGN_ATTR_S stRgnAttrSet;
    RGN_CANVAS_INFO_S stCanvasInfo;
    BITMAP_S stBitmap;
    VENC_CHN VencChn;
    VDEC_CHN VdecChn;
    //pthread_t stVdecThread;
    //pthread_t stVencThread;
    //pthread_t stRgnThread;
    VDEC_SENDPARAM_S stVdesSendPram;
    VENC_PTHREAD_INFO_S stVencGetPram;
    SIZE_S stSize;
    FILE * pastream = NULL;
    HI_U32 i;

    s32Ret = SAMPLE_RGN_SYS_Init(); 
    if(HI_SUCCESS != s32Ret)
    {
        printf("SAMPLE_RGN_SYS_Init failed! s32Ret: 0x%x.\n", s32Ret);
        goto END_O_VENC0;
    }
    
    /*************************************************
    step 1: create region and attach to venc
    *************************************************/
    OverlayHandle    = 0;
    u32OverlayRgnNum = OVERLAY_MAX_NUM_VENC/2;
    s32Ret = SAMPLE_RGN_CreateOverlayForVenc(OverlayHandle, u32OverlayRgnNum);
    if(HI_SUCCESS != s32Ret)
    {
        printf("SAMPLE_RGN_CreateOverlayForVenc failed! s32Ret: 0x%x.\n", s32Ret);
        goto END_O_VENC1;
    }
    
    /*************************************************
    step 2: start vdec chn
    *************************************************/
    VdecChn = 0;
    s32Ret = SAMPLE_RGN_StartVdec(VdecChn);
    if(HI_SUCCESS != s32Ret)
    {
        printf("SAMPLE_RGN_StartVdec failed! s32Ret: 0x%x.\n", s32Ret);
        goto END_O_VENC2;
    }
    
    /*************************************************
    step 3: start venc chn
    *************************************************/
    VencChn = 0;
    s32Ret = SAMPLE_RGN_StartVenc(VencChn);
    if(HI_SUCCESS != s32Ret)
    {
        printf("SAMPLE_RGN_StartVenc failed! s32Ret: 0x%x.\n", s32Ret);
        goto END_O_VENC3;
    }
    
    /*************************************************
    step 4: bind vdec and venc
    *************************************************/
    stSrcChn.enModId  = HI_ID_VDEC;
    stSrcChn.s32DevId = 0;
    stSrcChn.s32ChnId = 0;

    stDesChn.enModId  = HI_ID_VENC;
    stDesChn.s32DevId = 0;
    stDesChn.s32ChnId = 0;

    s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDesChn);
    if(HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_SYS_Bind failed! s32Ret: 0x%x.\n", s32Ret);
        goto END_O_VENC4;
    }

    /*************************************************
    step 7: load bitmap to region
    *************************************************/
    for(i=0; i<u32OverlayRgnNum; i++)
    {
        s32Ret = HI_MPI_RGN_GetAttr(OverlayHandle+i, &stRgnAttrSet);
        if(HI_SUCCESS != s32Ret)
        {
            printf("HI_MPI_RGN_GetAttr failed! s32Ret: 0x%x.\n", s32Ret);
            goto END_O_VENC4;
        }
        
        s32Ret = HI_MPI_RGN_GetCanvasInfo(OverlayHandle+i, &stCanvasInfo);
        if(HI_SUCCESS != s32Ret)
        {
            printf("HI_MPI_RGN_GetCanvasInfo failed! s32Ret: 0x%x.\n", s32Ret);
            goto END_O_VENC4;
        }
            
        stBitmap.pData   = (HI_VOID *)stCanvasInfo.u32VirtAddr;
        stSize.u32Width  = stCanvasInfo.stSize.u32Width;
        stSize.u32Height = stCanvasInfo.stSize.u32Height;
        s32Ret = SAMPLE_RGN_UpdateCanvas("mm2.bmp", &stBitmap, HI_FALSE, 0, &stSize, stCanvasInfo.u32Stride, 
            stRgnAttrSet.unAttr.stOverlay.enPixelFmt);
        if(HI_SUCCESS != s32Ret)
        {
            printf("SAMPLE_RGN_UpdateCanvas failed! s32Ret: 0x%x.\n", s32Ret);
            goto END_O_VENC4;
        }
        
        s32Ret = HI_MPI_RGN_UpdateCanvas(OverlayHandle+i);
        if(HI_SUCCESS != s32Ret)
        {
            printf("HI_MPI_RGN_UpdateCanvas failed! s32Ret: 0x%x.\n", s32Ret);
            goto END_O_VENC4;
        }
    }

    /*************************************************
    step 5: create a thread to change region's 
    layer, position, alpha and other display attribute
    *************************************************/
    pthread_create(&g_stRgnThread, NULL, SAMPLE_RGN_VencOSdDynamicDisplay, NULL);    

    /*************************************************
    step 6: create a thread for vdec to read stream
    from a file
    *************************************************/    
    stSize.u32Width  = 720;
    stSize.u32Height = 576;
    
    stVdesSendPram.bRun          = HI_TRUE;
    stVdesSendPram.VdChn         = VdecChn;
    stVdesSendPram.enPayload     = PT_H264;
    stVdesSendPram.enVideoMode   = VIDEO_MODE_FRAME;
    stVdesSendPram.s32MinBufSize = stSize.u32Height * stSize.u32Width / 2;
    pthread_create(&g_stVdecThread, NULL, SAMPLE_RGN_VdecSendStream, (HI_VOID*)&stVdesSendPram);

    /*************************************************
    step 7: create a thread for venc to save stream
    to a file
    *************************************************/
    char pfilename[30]; 
    sprintf(pfilename, "venc_encode_stream.h264");
    pastream = fopen(pfilename, "wb");  
    HI_ASSERT( NULL != pastream);

    stVencGetPram.pstream   = pastream;
    stVencGetPram.VeChnId   = VencChn;
    stVencGetPram.s32FrmCnt = 0;
    pthread_create(&g_stVencThread, 0, SAMPLE_RGN_VencGetStream, (HI_VOID *)&stVencGetPram);

    printf("\n#############Sample start ok! Press Enter to switch!#############\n");

    
    /*************************************************
    step 8: stop thread and release all the resource
    *************************************************/
    getchar();
    bExit = HI_TRUE;
    pthread_join(g_stVdecThread, 0);

    pthread_join(g_stVencThread, 0);

    pthread_join(g_stRgnThread, 0);
    
    bExit = HI_FALSE;
    
END_O_VENC4:
    HI_MPI_SYS_UnBind(&stSrcChn, &stDesChn);

END_O_VENC3:
    SAMPLE_RGN_StopVenc(VencChn);
    
END_O_VENC2:
    SAMPLE_RGN_StopVdec(VdecChn);

END_O_VENC1:    
    SAMPLE_RGN_DestroyRegion(OverlayHandle, u32OverlayRgnNum);       

END_O_VENC0:
    SAMPLE_COMM_SYS_Exit();
    
    return s32Ret;
}

/******************************************************************************
* function    : main()
* Description : region
******************************************************************************/
int main(int argc, char *argv[])
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR ch;
    
    bExit = HI_FALSE;

    signal(SIGINT, SAMPLE_RGN_HandleSig);
    signal(SIGTERM, SAMPLE_RGN_HandleSig);


    while (1)
    {
        SAMPLE_RGN_Usage();
        ch = getchar();
        if(10 == ch)
        {
            continue;
        }
        getchar();
        switch (ch)
        {
            case '0': /* VPSS VDEC->VPSS(OSD+COVER)->VO 1080P30 */
            {
                s32Ret = SAMPLE_RGN_AddCoverAndOsdToVpss();
                break;
            }
            case '1':
            {
                s32Ret = SAMPLE_RGN_AddCoverOsdAndLineToVpss();
                break;
            }
			case '2':
			{
                s32Ret = SAMPLE_RGN_AddMosaicToVpss();
                break;
            }
            case '3':
            {
                s32Ret = SAMPLE_RGN_AddCoverOsdAndLineToVo();
                break;
            }  
            case '4': /* VENC VDEC->VENC(OSD)->file */
            {
                s32Ret = SAMPLE_RGN_AddOsdToVenc();
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
