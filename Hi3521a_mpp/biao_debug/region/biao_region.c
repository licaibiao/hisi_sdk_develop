/************************************************************
*Copyright (C),lcb0281at163.com lcb0281atgmail.com
*BlogAddr: caibiao-lee.blog.csdn.net
*FileName: biao_region.c
*Description:区域管理测试程序。包括VO直接HDMI输出水印加视频，
    编码输出h264带水印视频文件
*Date:     2020-02-03
*Author:   Caibiao Lee
*Version:  V1.0
*Others:
*History:
***********************************************************/
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
#include "osd_string.h"

HI_BOOL bExit   = HI_FALSE;

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


#define VIDEO_1080P_W   (1920)
#define VIDEO_1080P_H   (1080)

#define VIDEO_D1_W   (720)
#define VIDEO_D1_H   (576)


#define DECODE_VIDEO_W  VIDEO_1080P_W
#define DECODE_VIDEO_H  VIDEO_1080P_H

#define ENCODE_VIDEO_W  VIDEO_1080P_W
#define ENCODE_VIDEO_H  VIDEO_1080P_H


#define OVERLAY_W       900
#define OVERLAY_H       70
#define OVERLAY_X       480
#define OVERLAY_Y       420

#define BITMAP_PICTURE_PATH   "/data/mm2.bmp"
//#define DECODE_H264_FILE      "/data/stream_chn0.h264"
#define DECODE_H264_FILE      "/data/1080P.h264"

#define ENCODE_H264_FILE      "venc_encode_stream.h264"

/************************************************* 
Function:    BIAO_RGN_SYS_Init  
Description: 为区域管理分配缓存
Input:  none
OutPut: none
Return: 0: success，none 0:error
Others:
Author: Caibiao Lee
Date:   2020-03-08
*************************************************/
static HI_S32 BIAO_RGN_SYS_Init(HI_VOID)
{
    HI_S32 s32Ret;
    VB_CONF_S struVbConf, stVbConf;
    MPP_SYS_CONF_S struSysConf;

    HI_MPI_SYS_Exit();
    HI_MPI_VB_Exit(); 

    memset(&struVbConf, 0, sizeof(VB_CONF_S));
    
    struVbConf.u32MaxPoolCnt             = 32;
    struVbConf.astCommPool[0].u32BlkSize = DECODE_VIDEO_W*DECODE_VIDEO_H*2;
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
    stVbConf.astCommPool[0].u32BlkSize   = DECODE_VIDEO_W*DECODE_VIDEO_H*2;
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

/************************************************* 
Function:    BIAO_RGN_CreateOverlayForVenc  
Description: 创建区域管理Overlay
Input:  
    Handle：句柄的开始号
    u32Num：需创建Overlay的个数
OutPut: none
Return: 0: success，none 0:error
Others:
Author: Caibiao Lee
Date:   2020-03-08
*************************************************/
HI_S32 BIAO_RGN_CreateOverlayForVenc(RGN_HANDLE Handle, HI_U32 u32Num)
{
    HI_S32 i;
    HI_S32 s32Ret;
    MPP_CHN_S stChn;
    RGN_ATTR_S stRgnAttr;
    RGN_CHN_ATTR_S stChnAttr;
    
    stChn.enModId  = HI_ID_VENC; /**模块号**/
    stChn.s32DevId = 0;          /**设备号**/
    stChn.s32ChnId = 0;          /**通道号**/

    if(1==u32Num)
    {
        stChn.enModId  = HI_ID_VENC; /**模块号**/
        stChn.s32DevId = 0;          /**设备号**/
        stChn.s32ChnId = 0;          /**通道号**/
    
        /**创建区域**/
        stRgnAttr.enType = OVERLAY_RGN;  /**区域类型:叠加**/
        stRgnAttr.unAttr.stOverlay.enPixelFmt       = PIXEL_FORMAT_RGB_1555; /**像素格式**/
        stRgnAttr.unAttr.stOverlay.stSize.u32Width  = OVERLAY_W;//240;        /**区域宽**/
        stRgnAttr.unAttr.stOverlay.stSize.u32Height = OVERLAY_H;//192;        /**区域高**/
        stRgnAttr.unAttr.stOverlay.u32BgColor       = 0;//0x00007c00; /**区域背景颜色**/

        s32Ret = HI_MPI_RGN_Create(Handle, &stRgnAttr);
        if(s32Ret != HI_SUCCESS)
        {
            SAMPLE_RGN_NOT_PASS(s32Ret);
        }

        /**将区域叠加到通道**/
        /**设置叠加区域的通道显示属性**/
        stChnAttr.bShow  = HI_TRUE;
        stChnAttr.enType = OVERLAY_RGN;
        stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = OVERLAY_X;//240;
        stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = OVERLAY_Y;//192;
        stChnAttr.unChnAttr.stOverlayChn.u32BgAlpha   = 80;
        stChnAttr.unChnAttr.stOverlayChn.u32FgAlpha   = 80;
        stChnAttr.unChnAttr.stOverlayChn.u32Layer     = Handle;

        /**设置QP属性**/
        stChnAttr.unChnAttr.stOverlayChn.stQpInfo.bAbsQp = HI_FALSE;
        stChnAttr.unChnAttr.stOverlayChn.stQpInfo.s32Qp  = 0;

        /**定义 OSD 反色相关属性**/
        /**单元反色区域，反色处理的基本单元,[16, 64]，需 16 对齐**/
        stChnAttr.unChnAttr.stOverlayChn.stInvertColor.stInvColArea.u32Height = 16;
        stChnAttr.unChnAttr.stOverlayChn.stInvertColor.stInvColArea.u32Width  = 16;

        /**亮度阈值,取值范围：[0, 255]**/
        stChnAttr.unChnAttr.stOverlayChn.stInvertColor.u32LumThresh = 128;

        /**OSD 反色触发模式**/
        stChnAttr.unChnAttr.stOverlayChn.stInvertColor.enChgMod     = LESSTHAN_LUM_THRESH;

        /**OSD 反色开关。**/
        stChnAttr.unChnAttr.stOverlayChn.stInvertColor.bInvColEn    = HI_FALSE;

        s32Ret = HI_MPI_RGN_AttachToChn(Handle, &stChn, &stChnAttr);
        if(s32Ret != HI_SUCCESS)
        {
            SAMPLE_RGN_NOT_PASS(s32Ret);
        }       
    }else
    {
        stChn.enModId  = HI_ID_VENC; /**模块号**/
        stChn.s32DevId = 0;          /**设备号**/
        stChn.s32ChnId = 0;          /**通道号**/
    
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

    }

    return HI_SUCCESS;
    
}

/************************************************* 
Function:    BIAO_RGN_StartVdec  
Description: 开启视频解码通道
Input:  
    VdecChn:需要开启的解码通道
OutPut: none
Return: 0: success，none 0:error
Others:
Author: Caibiao Lee
Date:   2020-03-08
*************************************************/
HI_S32 BIAO_RGN_StartVdec(VDEC_CHN VdecChn)
{
    HI_S32 s32Ret;
    VDEC_CHN_ATTR_S stVdecAttr;
    
    stVdecAttr.enType       = PT_H264;
    stVdecAttr.u32Priority  = 1;  /*u32Priority must be larger than 0*/
    stVdecAttr.u32PicWidth  = DECODE_VIDEO_W;
    stVdecAttr.u32PicHeight = DECODE_VIDEO_H;
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


/************************************************* 
Function:    BIAO_RGN_StartVenc  
Description: 开启视频编码通道
Input:  
    VdecChn:需要开启的编码通道
OutPut: none
Return: 0: success，none 0:error
Others:
Author: Caibiao Lee
Date:   2020-03-08
*************************************************/
HI_S32 BIAO_RGN_StartVenc(VENC_CHN VencChn)
{
    HI_S32 s32Ret;
    HI_U32 u32PicWidth;
    HI_U32 u32PicHeight;
    VENC_CHN_ATTR_S stChnAttr;

    u32PicWidth  = ENCODE_VIDEO_W;
    u32PicHeight = ENCODE_VIDEO_H;
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

/************************************************* 
Function:    BIAO_RGN_UpdateCanvas  
Description: 开启视频编码通道
Input:  
    filename:    bmp图片地址
    pstBitmap：   位图结构体，位图数据地址为画布的虚拟地址
    bFil：        是否需要填充颜色
    u16FilColor：需要填充的颜色值
    pstSize：     画布的大小
    u32Stride:   图像跨距
    enPixelFmt： 像素格式
OutPut: none
Return: 0: success，none 0:error
Others:
Author: Caibiao Lee
Date:   2020-03-08
*************************************************/
HI_S32 BIAO_RGN_UpdateCanvas(const char *filename, BITMAP_S *pstBitmap, 
    HI_BOOL bFil, HI_U32 u16FilColor, SIZE_S *pstSize, HI_U32 u32Stride, 
    PIXEL_FORMAT_E enPixelFmt)
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
    CreateSurfaceByCanvas(filename, &Surface, (HI_U8*)(pstBitmap->pData), 
        pstSize->u32Width, pstSize->u32Height, u32Stride);
	
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


void* BIAO_UpdateCanvas(void* p)
{
    int l_s32Handle= 0; 
    l_s32Handle = *(int *)p; 
    int s32Ret = 0;
    RGN_ATTR_S stRgnAttrSet;
    RGN_CANVAS_INFO_S stCanvasInfo;
    BITMAP_S stBitmap;
    SIZE_S stSize;

    printf("%s %d handle = %d \n",__FUNCTION__,__LINE__,l_s32Handle);
    
    while(HI_FALSE == bExit)
    {
        usleep(30000);
        CreateTimeBmpPicture();

        s32Ret = HI_MPI_RGN_GetAttr(l_s32Handle, &stRgnAttrSet);
        if(HI_SUCCESS != s32Ret)
        {
            printf("HI_MPI_RGN_GetAttr failed! s32Ret: 0x%x.\n", s32Ret);
            break;
        }
        
        s32Ret = HI_MPI_RGN_GetCanvasInfo(l_s32Handle, &stCanvasInfo);
        if(HI_SUCCESS != s32Ret)
        {
            printf("HI_MPI_RGN_GetCanvasInfo failed! s32Ret: 0x%x.\n", s32Ret);
            break;
        }
            
        stBitmap.pData   = (HI_VOID *)stCanvasInfo.u32VirtAddr;
        stSize.u32Width  = stCanvasInfo.stSize.u32Width;
        stSize.u32Height = stCanvasInfo.stSize.u32Height;
        s32Ret = BIAO_RGN_UpdateCanvas(TIMER_BMP_PATH, &stBitmap,HI_FALSE, 0, &stSize, 
            stCanvasInfo.u32Stride, stRgnAttrSet.unAttr.stOverlay.enPixelFmt);
        if(HI_SUCCESS != s32Ret)
        {
            printf("SAMPLE_RGN_UpdateCanvas failed! s32Ret: 0x%x.\n", s32Ret);
            break;
        }
        
        s32Ret = HI_MPI_RGN_UpdateCanvas(l_s32Handle);
        if(HI_SUCCESS != s32Ret)
        {
            printf("HI_MPI_RGN_UpdateCanvas failed! s32Ret: 0x%x.\n", s32Ret);
            break;
        }      
    }
}



/************************************************* 
Function:    BIAO_RGN_VdecSendStream  
Description: 将h264文件按视频帧读取，读取之后送入解码器中
Input:  
    void* p 线程启动参数
OutPut: none
Return: 0: success，none 0:error
Others:
Author: Caibiao Lee
Date:   2020-03-08
*************************************************/
void* BIAO_RGN_VdecSendStream(void* p)
{
	VDEC_STREAM_S stStream;
	VDEC_SENDPARAM_S *pstSendParam;
	char sFileName[64], sFilePostfix[20] = ".h264";
	FILE* fp = NULL;
	HI_S32 s32Ret;

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
	sprintf(sFileName, "%s", DECODE_H264_FILE);
	fp = fopen(sFileName, "r");
	if (HI_NULL == fp)
	{
		printf("open file %s err\n", sFileName);
		return NULL;
	}
	printf("open file [%s] ok!\n", sFileName);

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

HI_VOID BIAO_RGN_WriteStreamToFile( FILE * fp, VENC_PACK_S * pstNalu )
{
    HI_U8 * p;
    HI_U32 u32Len;

    p = (HI_U8 *) pstNalu->pu8Addr+pstNalu->u32Offset;

    u32Len = pstNalu->u32Len-pstNalu->u32Offset;

    fwrite(p, u32Len, sizeof(HI_U8), fp);
    
    fflush(fp);
}

HI_S32 BIAO_RGN_DestroyRegion(RGN_HANDLE Handle, HI_U32 u32Num)
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



HI_S32 BIAO_RGN_LoadBmp(const char *filename, BITMAP_S *pstBitmap, HI_BOOL bFil, HI_U32 u16FilColor)
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




HI_S32 BIAO_RGN_StopVenc(VENC_CHN VencChn)
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


HI_S32 BIAO_RGN_StopVdec(VDEC_CHN VdecChn)
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



HI_S32 BIAO_RGN_GetStream( HI_S32 chnId,FILE* pfStream ,HI_BOOL bByFrame)
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
                BIAO_RGN_WriteStreamToFile(pfStream, pstPack);
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




/************************************************* 
Function:    BIAO_RGN_VencGetStream  
Description: 从编码器中获取视频，然后存成文件
Input:  
    void *pData 线程启动参数
OutPut: none
Return: 0: success，none 0:error
Others:
Author: Caibiao Lee
Date:   2020-03-08
*************************************************/
HI_VOID *BIAO_RGN_VencGetStream(void *pData)
{
    VENC_CHN_STAT_S stChnStat;
    VENC_PTHREAD_INFO_S *pstVencThread;
    HI_S32 i = 0;
    HI_S32 s32Ret = HI_FAILURE;
    BITMAP_S stBitmap;
    char acmd[128];
   
    HI_ASSERT(HI_NULL != pData);

    pstVencThread = (VENC_PTHREAD_INFO_S *)pData;
    while (HI_FALSE == bExit || i < pstVencThread->s32FrmCnt)
    {        
        s32Ret = HI_MPI_VENC_Query( pstVencThread->VeChnId, &stChnStat );
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_RGN_NOT_PASS(s32Ret);
        }
        if (stChnStat.u32CurPacks > 0 || stChnStat.u32LeftStreamBytes >0)  
        {           
            BIAO_RGN_GetStream(pstVencThread->VeChnId, pstVencThread->pstream, HI_TRUE);
            i++;
        }
    }
 
    printf("\nchn %d finish!\n", pstVencThread->VeChnId);
    
    return HI_NULL;
}


/************************************************* 
Function:    BIAO_RGN_AddOsdToVenc  
Description: 将解码器与编码器绑定，区域通道与编码通道绑定，
    从h264文件中读取数据流，输入到解码器中，由解码器中流向编码器，
    最后将编码器产生的数据存成文件。编码之后的图像带有区域图像的水印。
Input:  none
OutPut: none
Return: 0: success，none 0:error
Others: 解码器输入的分辨率与编码器的输出分辨率可以不相同，
    比如将1080P图像解码后，可以再编码成720P图像。
Author: Caibiao Lee
Date:   2020-03-08
*************************************************/
HI_S32 BIAO_RGN_AddOsdToVenc(HI_VOID)
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
    VDEC_SENDPARAM_S stVdesSendPram;
    VENC_PTHREAD_INFO_S stVencGetPram;
    SIZE_S stSize;
    FILE * pastream = NULL;
    HI_U32 i;
    int l_s32CanvasHandle = 0;

    /**分配缓存**/
    s32Ret = BIAO_RGN_SYS_Init(); 
    if(HI_SUCCESS != s32Ret)
    {
        printf("SAMPLE_RGN_SYS_Init failed! s32Ret: 0x%x.\n", s32Ret);
        goto END_O_VENC0;
    }
    
    /**创建区域，并将它添加到编码通道**/
    OverlayHandle    = 0;
    u32OverlayRgnNum = 1;
    s32Ret = BIAO_RGN_CreateOverlayForVenc(OverlayHandle, u32OverlayRgnNum);
    if(HI_SUCCESS != s32Ret)
    {
        printf("SAMPLE_RGN_CreateOverlayForVenc failed! s32Ret: 0x%x.\n", s32Ret);
        goto END_O_VENC1;
    }
    
    /**开启解码通道**/
    VdecChn = 0;
    s32Ret = BIAO_RGN_StartVdec(VdecChn);
    if(HI_SUCCESS != s32Ret)
    {
        printf("SAMPLE_RGN_StartVdec failed! s32Ret: 0x%x.\n", s32Ret);
        goto END_O_VENC2;
    }
    
    /**开启编码通道**/
    VencChn = 0;
    s32Ret = BIAO_RGN_StartVenc(VencChn);
    if(HI_SUCCESS != s32Ret)
    {
        printf("SAMPLE_RGN_StartVenc failed! s32Ret: 0x%x.\n", s32Ret);
        goto END_O_VENC3;
    }
    
    /**将解码通道绑定到编码通道**/
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

#if 0
    /**加载位图到区域**/
    /**这里只加载一次，做成动态水印，需要添加一个线程动态改变区域像素**/
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
        s32Ret = BIAO_RGN_UpdateCanvas(BITMAP_PICTURE_PATH, &stBitmap, 
            HI_FALSE, 0, &stSize, stCanvasInfo.u32Stride, 
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
#endif

    /**创建一个线程用来从h264文件中读取数据，模拟h264数据流**/   
    stSize.u32Width  = DECODE_VIDEO_W;
    stSize.u32Height = DECODE_VIDEO_H;
    
    stVdesSendPram.bRun          = HI_TRUE;
    stVdesSendPram.VdChn         = VdecChn;
    stVdesSendPram.enPayload     = PT_H264;
    stVdesSendPram.enVideoMode   = VIDEO_MODE_FRAME;
    stVdesSendPram.s32MinBufSize = stSize.u32Height * stSize.u32Width / 2;
    pthread_create(&g_stVdecThread, NULL, BIAO_RGN_VdecSendStream, (HI_VOID*)&stVdesSendPram);


    /**更新OSD内容**/
    l_s32CanvasHandle = 0;
    pthread_create(&g_stRgnOsdThread, NULL, BIAO_UpdateCanvas, (HI_VOID*)&l_s32CanvasHandle);


    /**创建一个线程，将编码器输出的数据存成文件**/
    char pfilename[64]; 
    sprintf(pfilename, ENCODE_H264_FILE);
    pastream = fopen(pfilename, "wb");  
    HI_ASSERT( NULL != pastream);

    stVencGetPram.pstream   = pastream;
    stVencGetPram.VeChnId   = VencChn;
    stVencGetPram.s32FrmCnt = 0;
    pthread_create(&g_stVencThread, 0, BIAO_RGN_VencGetStream, (HI_VOID *)&stVencGetPram);

    printf("\n#############Sample start ok! Press Enter to switch!#############\n");

    
    /*************************************************
    step 8: stop thread and release all the resource
    *************************************************/

    /**延时之后推出编解码**/
    sleep(10);
    bExit = HI_TRUE;
    pthread_join(g_stVdecThread, 0);

    pthread_join(g_stVencThread, 0);

    pthread_join(g_stRgnOsdThread, 0);
    
    bExit = HI_FALSE;
    
END_O_VENC4:
    HI_MPI_SYS_UnBind(&stSrcChn, &stDesChn);

END_O_VENC3:
    BIAO_RGN_StopVenc(VencChn);
    
END_O_VENC2:
    BIAO_RGN_StopVdec(VdecChn);

END_O_VENC1:    
    BIAO_RGN_DestroyRegion(OverlayHandle, u32OverlayRgnNum);       

END_O_VENC0:
    SAMPLE_COMM_SYS_Exit();
    
    return s32Ret;
}

/************************************************* 
Function:    BIAO_RGN_AddOverlayToVpss  
Description: 将区使用域添加到VPSS(视频子系统),从h264文件中读取数据
    输入到解码器，然后再输出的视频输出模块，最后h264画面和区域水印
    显示在显示器中。
Input:  none
OutPut: none
Return: 0: success，none 0:error
Others: 该实例HDMI接口输出
Author: Caibiao Lee
Date:   2020-03-08
*************************************************/
HI_S32 BIAO_RGN_AddOverlayToVpss(void)
{
#if 0
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 u32RgnOverlayNum;
    HI_S32 u32RgnCoverNum;
    RGN_HANDLE OverlayHandle;
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
    VDEC_SENDPARAM_S stVdesSendPram;
    RGN_ATTR_INFO_S stRgnAttrInfo0;
    RGN_ATTR_INFO_S stRgnAttrInfo1;
    SIZE_S stSize;
    MPP_CHN_S stChn;
    RGN_ATTR_S stRgnAttrSet;
    RGN_CHN_ATTR_S stChnAttr;
    VDEC_CHN_ATTR_S stVdecAttr;

    HI_U32 u32Depth;
    VPSS_CHN_MODE_S stVpssChnMode;
    VPSS_GRP_ATTR_S stGrpAttr;
    HI_U32 u32OverlayMask;

    RGN_ATTR_S stRgnAttr;

    s32Ret = SAMPLE_RGN_SYS_Init();
    if(HI_SUCCESS != s32Ret)
    {
        printf("SAMPLE_RGN_SYS_Init failed! s32Ret: 0x%x.\n", s32Ret);
        goto END_OC_VPSS0;
    }

    /*************************************************
    step 1: create region and attach to vpss group
    *************************************************/
    /* Add cover to vpss group */
    stChn.enModId  = HI_ID_VPSS;
    stChn.s32DevId = 0;
    stChn.s32ChnId = 0;
        
    stRgnAttr.enType = COVER_RGN;

    s32Ret = HI_MPI_RGN_Create(0, &stRgnAttr);
    if(s32Ret != HI_SUCCESS)
    {
        printf("HI_MPI_RGN_Create fail! s32Ret: 0x%x.\n", s32Ret);
        goto END_OC_VPSS1;
    }

    stChnAttr.bShow  = HI_TRUE;
    stChnAttr.enType = COVER_RGN;
    stChnAttr.unChnAttr.stCoverChn.enCoverType = AREA_RECT;
    stChnAttr.unChnAttr.stCoverChn.stRect.s32X      = 210;
    stChnAttr.unChnAttr.stCoverChn.stRect.s32Y      = 200;
    stChnAttr.unChnAttr.stCoverChn.stRect.u32Height = 200 ;
    stChnAttr.unChnAttr.stCoverChn.stRect.u32Width  = 300 ;
    stChnAttr.unChnAttr.stCoverChn.u32Color         = 0x892f1b;//0x000000ff;
    stChnAttr.unChnAttr.stCoverChn.u32Layer         = 0;
    
    s32Ret = HI_MPI_RGN_AttachToChn(0, &stChn, &stChnAttr);
    if(s32Ret != HI_SUCCESS)
    {
        printf("HI_MPI_RGN_AttachToChn fail! s32Ret: 0x%x.\n", s32Ret);
        goto END_OC_VPSS1;
    }

    /*************************************************
    step 2: start vdec chn
    *************************************************/
    VdecChn = 0;
    stVdecAttr.enType       = PT_H264;
    stVdecAttr.u32Priority  = 1;  /*u32Priority must be larger than 0*/
    stVdecAttr.u32PicWidth  = 1920;
    stVdecAttr.u32PicHeight = 1080;
    stVdecAttr.u32BufSize   = stVdecAttr.u32PicWidth * stVdecAttr.u32PicHeight;//This item should larger than u32Width*u32Height/2
    stVdecAttr.stVdecVideoAttr.u32RefFrameNum   = 1;
    stVdecAttr.stVdecVideoAttr.bTemporalMvpEnable = 0;
    stVdecAttr.stVdecVideoAttr.enMode           = VIDEO_MODE_FRAME;

    /* create vdec chn*/
    s32Ret = HI_MPI_VDEC_CreateChn(VdecChn, &stVdecAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_VDEC_CreateChn failed! s32Ret:0x%x.\n", s32Ret);
        //return END_OC_VPSS2;
    }

    /* start vdec chn to receive stream sent by user*/
    s32Ret = HI_MPI_VDEC_StartRecvStream(VdecChn);    
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_VDEC_StartRecvStream failed! s32Ret:0x%x.\n", s32Ret);
        return s32Ret;
    }
    
    printf("Vdec chn create and start receive stream ok!\n");

    /*************************************************
    step 3: start vpss group and chn
    *************************************************/
    s32VpssGrpNum = 1;
    u32VpssChn    = VPSS_CHN0;        
    stGrpAttr.u32MaxW      = 1920;
    stGrpAttr.u32MaxH      = 1080;
    stGrpAttr.enPixFmt     = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
    stGrpAttr.enDieMode    = VPSS_DIE_MODE_NODIE;
    stGrpAttr.bIeEn        = HI_FALSE;
    stGrpAttr.bNrEn        = HI_FALSE;
    stGrpAttr.bHistEn      = HI_FALSE;
    stGrpAttr.bDciEn       = HI_FALSE;
    stGrpAttr.bEsEn        = HI_FALSE;
    
    s32Ret = HI_MPI_VPSS_CreateGrp(0, &stGrpAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("creat vpss grp%d fail! s32Ret: 0x%x.\n", 0, s32Ret);
        goto END_OC_VPSS3;
    }

    s32Ret = HI_MPI_VPSS_EnableChn(0, u32VpssChn);
    if (HI_SUCCESS != s32Ret)
    {
        printf("creat vpss grp%d chnl%d fail! s32Ret: 0x%x.\n", 0, u32VpssChn, s32Ret);
        goto END_OC_VPSS3;
    }

    stVpssChnMode.bDouble 	     = HI_FALSE;
    stVpssChnMode.enChnMode 	 = VPSS_CHN_MODE_AUTO;
    stVpssChnMode.enPixelFormat  = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
    stVpssChnMode.u32Width 	     = 1920;
    stVpssChnMode.u32Height 	 = 1080;
    stVpssChnMode.enCompressMode = COMPRESS_MODE_NONE;
    
    s32Ret = HI_MPI_VPSS_SetChnMode(0, u32VpssChn, &stVpssChnMode);
    if (HI_SUCCESS != s32Ret)
    {
        printf("set vpss grp%d chn%d mode fail! s32Ret: 0x%x.\n", 0, u32VpssChn, s32Ret);
        goto END_OC_VPSS3;
    }
    
    s32Ret = HI_MPI_VPSS_StartGrp(0);
    if (HI_SUCCESS != s32Ret)
    {
        printf("start vpss grp%d fail! s32Ret: 0x%x.\n", 0, s32Ret);
        goto END_OC_VPSS3;
    }

    u32Depth = 6;
    s32Ret = HI_MPI_VPSS_SetDepth(0, u32VpssChn, u32Depth);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_VPSS_SetDepth fail! Grp: %d, Chn: %d! s32Ret: 0x%x.\n", 0, u32VpssChn, s32Ret);
        goto END_OC_VPSS3;
    }

    u32OverlayMask = 255;
    s32Ret = HI_MPI_VPSS_SetChnOverlay(0, u32VpssChn, u32OverlayMask);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_VPSS_SetChnOverlay fail! Grp: %d, Chn: %d! s32Ret: 0x%x.\n", 0, u32VpssChn, s32Ret);
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


    if (HI_SUCCESS != SAMPLE_COMM_VO_HdmiStart(enIntfSync))
    {
        SAMPLE_PRT("Start SAMPLE_COMM_VO_HdmiStart failed!\n");
        goto END_OC_VPSS5;
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
        goto END_OC_VPSS6;
    }


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
    step 7: load bitmap to region
    *************************************************/
#if 0    
    OverlayHandle = 0 ;
    s32Ret = HI_MPI_RGN_GetAttr(OverlayHandle, &stRgnAttrSet);
    if(HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_RGN_GetAttr failed! s32Ret: 0x%x.\n", s32Ret);
        goto END_OC_VPSS7;
    }
    
    s32Ret = HI_MPI_RGN_GetCanvasInfo(OverlayHandle, &stCanvasInfo);
    if(HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_RGN_GetCanvasInfo failed! s32Ret: 0x%x.\n", s32Ret);
        goto END_OC_VPSS7;
    }
        
    stBitmap.pData   = (HI_VOID *)stCanvasInfo.u32VirtAddr;
    stSize.u32Width  = stCanvasInfo.stSize.u32Width;
    stSize.u32Height = stCanvasInfo.stSize.u32Height;
    s32Ret = SAMPLE_RGN_UpdateCanvas("mm2.bmp", &stBitmap, HI_FALSE, 0, &stSize, stCanvasInfo.u32Stride, 
        stRgnAttrSet.unAttr.stOverlay.enPixelFmt);
    if(HI_SUCCESS != s32Ret)
    {
        printf("SAMPLE_RGN_UpdateCanvas failed! s32Ret: 0x%x.\n", s32Ret);
        goto END_OC_VPSS7;
    }
    
    s32Ret = HI_MPI_RGN_UpdateCanvas(OverlayHandle);
    if(HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_RGN_UpdateCanvas failed! s32Ret: 0x%x.\n", s32Ret);
        goto END_OC_VPSS7;
    }
#endif


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
    pthread_create(&g_stVdecThread, NULL, BIAO_RGN_VdecSendStream, (HI_VOID*)&stVdesSendPram);


    printf("\n#############Sample start ok! Press Enter to switch!#############\n");

    /*************************************************
    step 11: stop thread and release all the resource
    *************************************************/
    getchar();
    bExit = HI_TRUE;

    pthread_join(g_stVdecThread, 0);

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


END_OC_VPSS5:
    SAMPLE_COMM_VO_HdmiStop();

END_OC_VPSS4:
    SAMPLE_RGN_StopVoChn(VoDev, s32VoChnNum);
    SAMPLE_RGN_StopVoDev(VoDev);


END_OC_VPSS3:
    SAMPLE_RGN_StopVpss(s32VpssGrpNum);

END_OC_VPSS2:
    SAMPLE_RGN_StopVdec(VdecChn);

END_OC_VPSS1:
    SAMPLE_RGN_DestroyRegion(0, 1);

END_OC_VPSS0:    
    SAMPLE_COMM_SYS_Exit();
#endif
    return HI_SUCCESS;   

}


int BIAO_REGION_DEBUG(void)
{
    //BIAO_RGN_AddOverlayToVpss();
    BIAO_RGN_AddOsdToVenc();
    return 0;
}


