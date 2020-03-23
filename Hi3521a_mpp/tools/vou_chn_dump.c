#include <sys/mman.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ioctl.h>

#include "hi_common.h"
#include "hi_comm_video.h"
#include "hi_comm_sys.h"
#include "mpi_sys.h"
#include "hi_comm_vb.h"
#include "mpi_vb.h"
#include "hi_comm_vo.h"
#include "mpi_vo.h"
#include "mpi_vgs.h"

typedef struct hiDUMP_MEMBUF_S
{
    VB_BLK  hBlock;
    VB_POOL hPool;
    HI_U32  u32PoolId;
    
    HI_U32  u32PhyAddr;
    HI_U8   *pVirAddr;
    HI_S32  s32Mdev;
} DUMP_MEMBUF_S;

static VIDEO_FRAME_INFO_S g_stFrame;
static char* g_pVBufVirt_Y = NULL;
static char* g_pVBufVirt_C = NULL;
static HI_U32 g_Ysize, g_Csize;
static FILE* g_pfd = NULL;
static VB_POOL g_hPool  = VB_INVALID_POOLID;
static DUMP_MEMBUF_S g_stMem;
static VO_LAYER g_VoLayer = 0;
static VO_CHN g_VoChn = 0;


/******************************************************************************
* function : to process abnormal case
******************************************************************************/
void VOU_Chn_HandleSig(HI_S32 signo)
{
    if (SIGINT == signo || SIGTERM == signo)
    {
        if (0 != g_stFrame.stVFrame.u32PhyAddr[0])
        {
            HI_MPI_VO_ReleaseChnFrame(0, g_VoChn, &g_stFrame);
            memset(&g_stFrame, 0, sizeof(VIDEO_FRAME_INFO_S));
        }

        if (NULL != g_pVBufVirt_Y)
        {
            HI_MPI_SYS_Munmap(g_pVBufVirt_Y, g_Ysize);
            g_pVBufVirt_Y = NULL;
        }

        if (NULL != g_pVBufVirt_C)
        {
            HI_MPI_SYS_Munmap(g_pVBufVirt_C, g_Csize);
            g_pVBufVirt_C = NULL;
        }

        if (NULL != g_pfd)
        {
            fclose(g_pfd);
            g_pfd = NULL;
        }

        if (VB_INVALID_HANDLE != g_stMem.hBlock)
        {
            HI_MPI_VB_ReleaseBlock(g_stMem.hBlock);
            g_stMem.hBlock = VB_INVALID_HANDLE;
        }

        if (VB_INVALID_POOLID != g_hPool)
        {
            HI_MPI_VB_DestroyPool( g_hPool );
        }

        printf("\033[0;31mprogram termination abnormally!\033[0;39m\n");
    }
    exit(-1);
}

/* sp420 转存为 p420 ; sp422 转存为 p422  */
void sample_yuv_dump(VIDEO_FRAME_S* pVBuf, FILE* pfd)
{
    unsigned int w, h;
    char* pMemContent;
    unsigned char TmpBuff[4096];                //如果这个值太小，图像很大的话存不了
    HI_U32 phy_addr;
    PIXEL_FORMAT_E  enPixelFormat = pVBuf->enPixelFormat;
    HI_U32 u32UvHeight;/* 存为planar 格式时的UV分量的高度 */

    g_Ysize = (pVBuf->u32Stride[0]) * (pVBuf->u32Height);
    if (PIXEL_FORMAT_YUV_SEMIPLANAR_420 == enPixelFormat)
    {
        g_Csize = (pVBuf->u32Stride[1]) * (pVBuf->u32Height) / 2;
        u32UvHeight = pVBuf->u32Height / 2;
    }
    else if (PIXEL_FORMAT_YUV_SEMIPLANAR_422 == enPixelFormat)
    {
        g_Csize = (pVBuf->u32Stride[1]) * (pVBuf->u32Height);
        u32UvHeight = pVBuf->u32Height;
    }
    else
    {
        g_Csize = 0;
        u32UvHeight = 0;
    }

    phy_addr = pVBuf->u32PhyAddr[0];

    g_pVBufVirt_Y = (HI_CHAR*) HI_MPI_SYS_Mmap(phy_addr, g_Ysize);
    if (NULL == g_pVBufVirt_Y)
    {
        return;
    }

    g_pVBufVirt_C = (HI_CHAR*) HI_MPI_SYS_Mmap(pVBuf->u32PhyAddr[1], g_Csize);
    if (NULL == g_pVBufVirt_C)
    {
        HI_MPI_SYS_Munmap(g_pVBufVirt_Y, g_Ysize);
        g_pVBufVirt_Y = NULL;
        return;
    }

    /* save Y ----------------------------------------------------------------*/
    fprintf(stderr, "saving......Y......");
    fflush(stderr);
    for(h=0; h<pVBuf->u32Height; h++)
    {
        pMemContent = g_pVBufVirt_Y + h * pVBuf->u32Stride[0];
        fwrite(pMemContent, pVBuf->u32Width, 1, pfd);
    }
    fflush(g_pfd);
    

    /* save U ----------------------------------------------------------------*/
    fprintf(stderr, "U......");
    fflush(stderr);
    for(h=0; h<u32UvHeight; h++)
    {
        pMemContent = g_pVBufVirt_C + h * pVBuf->u32Stride[1];

        pMemContent += 1;

        for(w=0; w<pVBuf->u32Width/2; w++)
        {
            TmpBuff[w] = *pMemContent;
            pMemContent += 2;
        }
        fwrite(TmpBuff, pVBuf->u32Width/2, 1, pfd);
    }
    fflush(pfd);

    /* save V ----------------------------------------------------------------*/
    fprintf(stderr, "V......");
    fflush(stderr);
    for(h=0; h<u32UvHeight; h++)    
    {
            pMemContent = g_pVBufVirt_C + h * pVBuf->u32Stride[1];

        for(w=0; w<pVBuf->u32Width/2; w++)
        {
            TmpBuff[w] = *pMemContent;
            pMemContent += 2;
        }
        fwrite(TmpBuff, pVBuf->u32Width/2, 1, pfd);
    }
    fflush(pfd);
    
    fprintf(stderr, "done %d!\n", pVBuf->u32TimeRef);
    fflush(stderr);
    
    HI_MPI_SYS_Munmap(g_pVBufVirt_Y, g_Ysize);
    g_pVBufVirt_Y = NULL;
	HI_MPI_SYS_Munmap(g_pVBufVirt_C, g_Csize);
	g_pVBufVirt_C = NULL;

}

HI_S32 SAMPLE_MISC_VoDump(VO_LAYER VoLayer, VO_CHN VoChn, HI_U32 u32Cnt)
{
    HI_S32 i, s32Ret;
    HI_CHAR szYuvName[128];
    HI_CHAR szPixFrm[10];
    HI_U32  u32BlkSize = 0;
    VIDEO_FRAME_INFO_S stFrmInfo;
    VGS_HANDLE hHandle;
    VGS_TASK_ATTR_S stTask;
    HI_U32 u32LumaSize              = 0;
    HI_U32 u32ChrmSize              = 0; 
    HI_U32 u32PicLStride            = 0;
    //HI_U32 u32PicCStride            = 0;
    HI_U32 u32Width                 = 0;
    HI_U32 u32Height                = 0;
    HI_BOOL bSendToVgs              = HI_FALSE;
    
    printf("\nNOTICE: This tool only can be used for TESTING !!!\n");
    printf("usage: ./vou_chn_dump [VoLayer] [vochn] [frmcnt]. sample: ./vou_chn_dump 0 0 5\n\n");

    /* Get Frame to make file name*/
    s32Ret = HI_MPI_VO_GetChnFrame(VoLayer, VoChn, &g_stFrame, 200);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_VO(%d)_GetChnFrame errno %#x\n", VoLayer, s32Ret);
        return -1;
    }

    /* make file name */
    strcpy(szPixFrm,(PIXEL_FORMAT_YUV_SEMIPLANAR_420 == g_stFrame.stVFrame.enPixelFormat)?"p420":"p422");
    sprintf(szYuvName, "./vo(%d,%d)_%s_%d_%dx%d.yuv",VoLayer,VoChn,
        szPixFrm,u32Cnt,g_stFrame.stVFrame.u32Width, g_stFrame.stVFrame.u32Height);
    printf("Dump YUV frame of vo(%d,%d) to file: \"%s\"\n",VoLayer, VoChn, szYuvName);
    
    bSendToVgs = (g_stFrame.stVFrame.enCompressMode > 0) || (g_stFrame.stVFrame.enVideoFormat > 0);
    
    HI_MPI_VO_ReleaseChnFrame(VoLayer, VoChn, &g_stFrame);
    
    /* open file */
    g_pfd = fopen(szYuvName, "wb");

    if (NULL == g_pfd)
    {
        return -1;
    }
    
    u32PicLStride = g_stFrame.stVFrame.u32Stride[0];
    //u32PicCStride = stFrame.stVFrame.u32Stride[0];
    u32LumaSize = g_stFrame.stVFrame.u32Stride[0]*g_stFrame.stVFrame.u32Height;
    u32ChrmSize = (g_stFrame.stVFrame.u32Stride[0] * g_stFrame.stVFrame.u32Height) >> (1+(PIXEL_FORMAT_YUV_SEMIPLANAR_420 == g_stFrame.stVFrame.enPixelFormat));
    u32Width    = g_stFrame.stVFrame.u32Width;
    u32Height   = g_stFrame.stVFrame.u32Height;

    if(bSendToVgs)
    {
        u32BlkSize = (PIXEL_FORMAT_YUV_SEMIPLANAR_420 == g_stFrame.stVFrame.enPixelFormat)
            ?(g_stFrame.stVFrame.u32Stride[0]*g_stFrame.stVFrame.u32Height*3>>1):(g_stFrame.stVFrame.u32Stride[0]*g_stFrame.stVFrame.u32Height*2);
        
        /*create comm vb pool*/
        g_hPool   = HI_MPI_VB_CreatePool( u32BlkSize, 2,NULL);
        if (g_hPool == VB_INVALID_POOLID)
        {
            printf("HI_MPI_VB_CreatePool failed! \n");
            goto END1;
        }

        g_stMem.hPool = g_hPool;
    }

    /* get VO frame  */
    for (i=0; i<u32Cnt; i++)
    {
        s32Ret = HI_MPI_VO_GetChnFrame(VoLayer, VoChn, &g_stFrame, 200);
        if (HI_SUCCESS != s32Ret)
        {
            printf("get vo(%d,%d) frame err\n", VoLayer, VoChn);
            printf("only get %d frame\n", i);
            break;
        }

        if(bSendToVgs)
        {
            while((g_stMem.hBlock = HI_MPI_VB_GetBlock(g_stMem.hPool, u32BlkSize,NULL)) == VB_INVALID_HANDLE)
            {
                 ;
            }

            g_stMem.u32PhyAddr = HI_MPI_VB_Handle2PhysAddr(g_stMem.hBlock);
            g_stMem.pVirAddr = (HI_U8 *) HI_MPI_SYS_Mmap( g_stMem.u32PhyAddr, u32BlkSize );
            if(g_stMem.pVirAddr == NULL)
            {
                printf("Mem dev may not open\n");
                HI_MPI_VB_ReleaseBlock(g_stMem.hBlock);
                goto END2;
            }
        
            memset(&stFrmInfo.stVFrame, 0, sizeof(VIDEO_FRAME_S));
            stFrmInfo.stVFrame.u32PhyAddr[0] = g_stMem.u32PhyAddr;
            stFrmInfo.stVFrame.u32PhyAddr[1] = stFrmInfo.stVFrame.u32PhyAddr[0] + u32LumaSize;
            stFrmInfo.stVFrame.u32PhyAddr[2] = stFrmInfo.stVFrame.u32PhyAddr[1] + u32ChrmSize;
            
            stFrmInfo.stVFrame.pVirAddr[0] = g_stMem.pVirAddr;
            stFrmInfo.stVFrame.pVirAddr[1] = (HI_U8 *) stFrmInfo.stVFrame.pVirAddr[0] + u32LumaSize;
            stFrmInfo.stVFrame.pVirAddr[2] = (HI_U8 *) stFrmInfo.stVFrame.pVirAddr[1] + u32ChrmSize;

            stFrmInfo.stVFrame.u32Width     = u32Width;
            stFrmInfo.stVFrame.u32Height    = u32Height;
            stFrmInfo.stVFrame.u32Stride[0] = u32PicLStride;
            stFrmInfo.stVFrame.u32Stride[1] = u32PicLStride;
            stFrmInfo.stVFrame.u32Stride[2] = u32PicLStride;

            stFrmInfo.stVFrame.enCompressMode = COMPRESS_MODE_NONE;
            stFrmInfo.stVFrame.enPixelFormat  = g_stFrame.stVFrame.enPixelFormat;
            stFrmInfo.stVFrame.enVideoFormat  = VIDEO_FORMAT_LINEAR;

            stFrmInfo.stVFrame.u64pts     = (i * 40);
            stFrmInfo.stVFrame.u32TimeRef = (i * 2);

            stFrmInfo.u32PoolId = g_hPool;

            s32Ret = HI_MPI_VGS_BeginJob(&hHandle);
            if(s32Ret != HI_SUCCESS)
        	{	
        	    printf("HI_MPI_VGS_BeginJob failed\n");
                HI_MPI_VB_ReleaseBlock(g_stMem.hBlock);
                g_stMem.hBlock = VB_INVALID_HANDLE;
                HI_MPI_VO_ReleaseChnFrame(VoLayer, VoChn, &g_stFrame);
        	    goto END2;
        	}
       
            memcpy(&stTask.stImgIn,&g_stFrame.stVFrame,sizeof(VIDEO_FRAME_INFO_S));
            memcpy(&stTask.stImgOut ,&stFrmInfo,sizeof(VIDEO_FRAME_INFO_S));
            s32Ret = HI_MPI_VGS_AddScaleTask(hHandle, &stTask);
            if(s32Ret != HI_SUCCESS)
        	{	
        	    printf("HI_MPI_VGS_AddScaleTask failed\n");
                HI_MPI_VGS_CancelJob(hHandle);
                HI_MPI_VB_ReleaseBlock(g_stMem.hBlock);
                g_stMem.hBlock = VB_INVALID_HANDLE;
                HI_MPI_VO_ReleaseChnFrame(VoLayer, VoChn, &g_stFrame);
        	    goto END2;
        	}

            s32Ret = HI_MPI_VGS_EndJob(hHandle);
            if(s32Ret != HI_SUCCESS)
        	{	
        	    printf("HI_MPI_VGS_EndJob failed\n");
                HI_MPI_VGS_CancelJob(hHandle);
                HI_MPI_VB_ReleaseBlock(g_stMem.hBlock);
                g_stMem.hBlock = VB_INVALID_HANDLE;
                HI_MPI_VO_ReleaseChnFrame(VoLayer, VoChn, &g_stFrame);
        	    goto END2;
        	}

            /* save VO frame to file */
            sample_yuv_dump(&stFrmInfo.stVFrame, g_pfd);

            HI_MPI_VB_ReleaseBlock(g_stMem.hBlock);
            g_stMem.hBlock = VB_INVALID_HANDLE;
        }
        else
        {
            /* save VO frame to file */
		    sample_yuv_dump(&g_stFrame.stVFrame, g_pfd);
        }

        /* release frame after using */
        s32Ret = HI_MPI_VO_ReleaseChnFrame(VoLayer, VoChn, &g_stFrame);
        if (HI_SUCCESS != s32Ret)
        {
            printf("Release vo(%d,%d) frame err\n", VoLayer, VoChn);
            printf("only get %d frame\n", i);
            break;
        }
    }

END2:
    if (VB_INVALID_POOLID != g_hPool)
    {
        HI_MPI_VB_DestroyPool(g_hPool);
        g_hPool = VB_INVALID_POOLID;
    }
    memset(&g_stFrame, 0, sizeof(VIDEO_FRAME_INFO_S));

END1:

    fclose(g_pfd);

	return 0;
}

HI_S32 main(int argc, char *argv[])
{
    HI_U32 u32FrmCnt = 1;

	/* VO设备号*/
    if (argc > 1)
    {
        g_VoLayer = atoi(argv[1]);
    }

	/* VO通道号 */
    if (argc > 2)
    {
        g_VoChn = atoi(argv[2]);
    }

	/* 需要采集的帧数目*/
    if (argc > 3)
    {
        u32FrmCnt = atoi(argv[3]);
        if (u32FrmCnt <= 0)
        {
            printf("The frmcnt(%d) is wrong!\n", u32FrmCnt);
            return HI_FAILURE;
        }
    }

    signal(SIGINT, VOU_Chn_HandleSig);
    signal(SIGTERM, VOU_Chn_HandleSig);
    SAMPLE_MISC_VoDump(g_VoLayer, g_VoChn, u32FrmCnt);

	return HI_SUCCESS;
}


