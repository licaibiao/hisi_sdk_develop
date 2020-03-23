#include <sys/mman.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "hi_common.h"
#include "hi_comm_video.h"
#include "hi_comm_sys.h"
#include "mpi_sys.h"
#include "hi_comm_vb.h"
#include "mpi_vb.h"
#include "hi_comm_vi.h"
#include "mpi_vi.h"

//////////////////////////////////////////////////////////////
//#define DIS_DATA_DEBUG

#define MAX_FRM_CNT     256
#define MAX_FRM_WIDTH   4096

VI_CHN_ATTR_S stChnAttrBackup;

#if 1
static HI_S32 s_s32MemDev = -1;

#define MEM_DEV_OPEN() \
do {\
    	if (s_s32MemDev <= 0)\
        {\
            s_s32MemDev = open("/dev/mem", O_CREAT|O_RDWR|O_SYNC);\
            if (s_s32MemDev < 0)\
            {\
                perror("Open dev/mem error");\
                return -1;\
            }\
        }\
}while(0)

#define MEM_DEV_CLOSE() \
do {\
        HI_S32 s32Ret;\
    	if (s_s32MemDev > 0)\
        {\
            s32Ret = close(s_s32MemDev);\
            if(HI_SUCCESS != s32Ret)\
            {\
                perror("Close mem/dev Fail");\
                return s32Ret;\
            }\
            s_s32MemDev = -1;\
        }\
}while(0)

HI_VOID * COMM_SYS_Mmap(HI_U32 u32PhyAddr, HI_U32 u32Size)
{
    HI_U32 u32Diff;
    HI_U32 u32PagePhy;
    HI_U32 u32PageSize;
    HI_U8 * pPageAddr;   

    /* The mmap address should align with page */
    u32PagePhy = u32PhyAddr & 0xfffff000;   
    u32Diff    = u32PhyAddr - u32PagePhy;

    /* The mmap size shuld be mutliples of 1024 */
    u32PageSize = ((u32Size + u32Diff - 1) & 0xfffff000) + 0x1000;  
    pPageAddr   = mmap ((void *)0, u32PageSize, PROT_READ|PROT_WRITE,
                                    MAP_SHARED, s_s32MemDev, u32PagePhy);   
    if (MAP_FAILED == pPageAddr )   
    {    
        perror("mmap error");
        return NULL;    
    }
    return (HI_VOID *) (pPageAddr + u32Diff);
}

HI_S32 COMM_SYS_Munmap(HI_VOID* pVirAddr, HI_U32 u32Size)
{
    HI_U32 u32PageAddr;
    HI_U32 u32PageSize;
    HI_U32 u32Diff;

    u32PageAddr = (((HI_U32)pVirAddr) & 0xfffff000);
    u32Diff     = (HI_U32)pVirAddr - u32PageAddr;
    u32PageSize = ((u32Size + u32Diff - 1) & 0xfffff000) + 0x1000;

    return munmap((HI_VOID*)u32PageAddr, u32PageSize);
}
#endif


#ifdef DIS_DATA_DEBUG
#define DIS_STATS_NUM   9
typedef struct hiVI_DIS_STATS_S
{
    HI_S32 as32HDelta[DIS_STATS_NUM];
    HI_S32 as32HSad[DIS_STATS_NUM];
    HI_S32 as32HMv[DIS_STATS_NUM];
    HI_S32 as32VDelta[DIS_STATS_NUM];
    HI_S32 as32VSad[DIS_STATS_NUM];
    HI_S32 as32VMv[DIS_STATS_NUM];
    HI_U32 u32HMotion;
    HI_U32 u32VMotion;
    HI_U32 u32HOffset;
    HI_U32 u32VOffset;
}VI_DIS_STATS_S;

void vi_dump_save_one_dis(VI_CHN ViChn, VIDEO_FRAME_S *pVBuf)
{
    char g_strDisBuff[256] = {'\0'};
    char szDisFileName[128];
    static FILE *pstDisFd = NULL;
    HI_U32 u32DisBufOffset;
    VI_DIS_STATS_S *pstDisStats;
    int j;
    HI_U32 u32BufSize;
    int iBufSize = 0;
    int size = 256;

    if (pstDisFd == NULL)
    {
        sprintf(szDisFileName, "./vi_%dp_01_dis_result.txt", pVBuf->u32Width);

        pstDisFd = fopen(szDisFileName, "wb");
        if (NULL == pstDisFd)
        {
            return -1;
        }
    }

    u32BufSize = pVBuf->u32Stride[0] * pVBuf->u32Height;
    if (PIXEL_FORMAT_YUV_SEMIPLANAR_422 == pVBuf->enPixelFormat)
    {
        u32BufSize *= 2;
    }
    else
    {
        u32BufSize = u32BufSize * 3 >> 1;
    }
    
    u32DisBufOffset = pVBuf->u32PhyAddr[0] + u32BufSize;
    pstDisStats = (VI_DIS_STATS_S *) COMM_SYS_Mmap(u32DisBufOffset, size);

    sprintf(g_strDisBuff, "%s\n", \
    "v_delta[i], v_sad[i], v_mv[i], h_delta[i], h_sad[i], h_mv[i]");
    iBufSize = strlen(g_strDisBuff) + 1; 
    fwrite(g_strDisBuff, iBufSize, 1, pstDisFd);
    

    for(j=0; j<9; j++)
    {
        sprintf(g_strDisBuff, "%8d, %8d, %8d, %8d, %8d, %8d\n",\
            pstDisStats->as32VDelta[j], pstDisStats->as32VSad[j], pstDisStats->as32VMv[j], \
            pstDisStats->as32HDelta[j], pstDisStats->as32HSad[j], pstDisStats->as32HMv[j]);  
        iBufSize = strlen(g_strDisBuff)+1; 
        fwrite(g_strDisBuff, iBufSize, 1, pstDisFd);
    }

    sprintf(g_strDisBuff, "%s\n", "H_Motion, V_Motion, *H_Offset, *V_Offset");
    iBufSize = strlen(g_strDisBuff)+1; 
    fwrite(g_strDisBuff, iBufSize, 1, pstDisFd);

    sprintf(g_strDisBuff, "%8d  %8d  %8d  %8d\n\n", \
    pstDisStats->u32HMotion, pstDisStats->u32VMotion, 
    pstDisStats->u32HOffset, pstDisStats->u32VOffset);
    iBufSize = strlen(g_strDisBuff)+1; 
    fwrite(g_strDisBuff, iBufSize, 1, pstDisFd);
    fflush(pstDisFd);

    COMM_SYS_Munmap((HI_VOID*)pstDisStats, size);
}
#endif


/* sp420 转存为 p420 ; sp422 转存为 p422  */
void vi_dump_save_one_frame(VIDEO_FRAME_S * pVBuf, FILE *pfd)
{
    unsigned int w, h;
    char * pVBufVirt_Y;
    char * pVBufVirt_C;
    char * pMemContent;
    unsigned char TmpBuff[MAX_FRM_WIDTH];                //如果这个值太小，图像很大的话存不了
    HI_U32 phy_addr,size;
	HI_CHAR *pUserPageAddr[2];
    PIXEL_FORMAT_E  enPixelFormat = pVBuf->enPixelFormat;
    HI_U32 u32UvHeight;/* 存为planar 格式时的UV分量的高度 */

    if (pVBuf->u32Width > MAX_FRM_WIDTH)
    {
        printf("Over max frame width: %d, can't support.\n", MAX_FRM_WIDTH);
        return;
    }

    if (PIXEL_FORMAT_YUV_SEMIPLANAR_420 == enPixelFormat)
    {
        size = (pVBuf->u32Stride[0])*(pVBuf->u32Height)*3/2;
        u32UvHeight = pVBuf->u32Height/2;
    }
    else if(PIXEL_FORMAT_YUV_SEMIPLANAR_422 == enPixelFormat)
    {
        size = (pVBuf->u32Stride[0])*(pVBuf->u32Height)*2;
        u32UvHeight = pVBuf->u32Height;
    }
    else
    {
        size = (pVBuf->u32Stride[0])*(pVBuf->u32Height);
        u32UvHeight = 0;
    }

    phy_addr = pVBuf->u32PhyAddr[0];

    pUserPageAddr[0] = (HI_CHAR *) COMM_SYS_Mmap(phy_addr, size);
    if (NULL == pUserPageAddr[0])
    {
        return;
    }
    printf("stride0: %d,stride1:%d, width: %d\n",pVBuf->u32Stride[0],pVBuf->u32Stride[1] ,pVBuf->u32Width);
    
	pVBufVirt_Y = pUserPageAddr[0];
	pVBufVirt_C = pVBufVirt_Y + (pVBuf->u32Stride[0])*(pVBuf->u32Height);

    /* save Y ----------------------------------------------------------------*/
    fprintf(stderr, "saving......Y......");
    fflush(stderr);
    for(h=0; h<pVBuf->u32Height; h++)
    {
        pMemContent = pVBufVirt_Y + h*pVBuf->u32Stride[0];
        fwrite(pMemContent, pVBuf->u32Width, 1, pfd);
    }
    fflush(pfd);

    /* save U ----------------------------------------------------------------*/
    if(u32UvHeight>0)
    {
        fprintf(stderr, "U......");
        fflush(stderr);       
        for(h=0; h<u32UvHeight; h++)
        {
            pMemContent = pVBufVirt_C + h*pVBuf->u32Stride[1];

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
            pMemContent = pVBufVirt_C + h*pVBuf->u32Stride[1];

            for(w=0; w<pVBuf->u32Width/2; w++)
            {
                TmpBuff[w] = *pMemContent;
                pMemContent += 2;
            }
            fwrite(TmpBuff, pVBuf->u32Width/2, 1, pfd);
        }
        fflush(pfd);
    }

    fprintf(stderr, "done %d!\n", pVBuf->u32TimeRef);
    fflush(stderr);

    COMM_SYS_Munmap(pUserPageAddr[0], size);
}

HI_S32 SAMPLE_VI_BackupAttr(VI_CHN ViChn)
{
    VI_CHN_ATTR_S stChnAttr;
    
    MEM_DEV_OPEN();

    if (ViChn != 0)
        return 0;

    if (HI_MPI_VI_GetChnAttr(ViChn, &stChnAttrBackup))
    {
        printf("HI_MPI_VI_GetChnAttr err, vi chn %d \n", ViChn);
        return -1;
    }

    /* 清队列 */
    HI_MPI_VI_SetFrameDepth(ViChn, 0);
    sleep(1);    

#if 0
    printf("compress mode: %d -> %d. \n", stChnAttrBackup.enCompressMode, COMPRESS_MODE_NONE);
    /* 配置成非压 */
    memcpy(&stChnAttr, &stChnAttrBackup, sizeof(VI_CHN_ATTR_S));
    stChnAttr.enCompressMode = COMPRESS_MODE_NONE;
#endif

    memcpy(&stChnAttr, &stChnAttrBackup, sizeof(VI_CHN_ATTR_S));

    
    if (HI_MPI_VI_SetChnAttr(ViChn, &stChnAttr))
    {
        printf("HI_MPI_VI_SetChnAttr err, vi chn %d \n", ViChn);
        return -1;
    }    
   
    return 0;
}

HI_S32 SAMPLE_VI_RestoreAttr(VI_CHN ViChn)
{
    MEM_DEV_CLOSE();

    if (ViChn != 0)
        return 0;

    //printf("restore compress mode: %d\n", stChnAttrBackup.enCompressMode);
    if (HI_MPI_VI_SetChnAttr(ViChn, &stChnAttrBackup))
    {
        printf("HI_MPI_VI_SetChnAttr err, vi chn %d \n", ViChn);
        return -1;
    }

    return 0;
}

HI_S32 SAMPLE_MISC_ViDump(VI_CHN ViChn, HI_U32 u32Cnt)
{
    HI_S32 i, j;
    VIDEO_FRAME_INFO_S stFrame;
    VIDEO_FRAME_INFO_S astFrame[MAX_FRM_CNT];
    HI_CHAR szYuvName[128];
    HI_CHAR szPixFrm[10];
    FILE *pfd;
    HI_S32 s32MilliSec = 2000;
    HI_CHAR filename[40];

    printf("\nNOTICE: This tool only can be used for TESTING !!!\n");
    printf("usage: ./vi_dump [vichn] [frmcnt]. sample: ./vi_dump 0 5\n\n");

    if (HI_MPI_VI_SetFrameDepth(ViChn, 1))
    {
        printf("HI_MPI_VI_SetFrameDepth err, vi chn %d \n", ViChn);
        return -1;
    }

    usleep(90000);

    if (HI_MPI_VI_GetFrame(ViChn, &astFrame[0], s32MilliSec))
    {
        printf("HI_MPI_VI_GetFrame err, vi chn %d \n", ViChn);
        return -1;
    }

    /* make file name */
    if(PIXEL_FORMAT_YUV_SEMIPLANAR_420 == astFrame[0].stVFrame.enPixelFormat)
    {
        strcpy(szPixFrm,"sp420");
    }
    else if (PIXEL_FORMAT_YUV_SEMIPLANAR_422 == astFrame[0].stVFrame.enPixelFormat)
    {
        strcpy(szPixFrm,"sp422");
    }
    else if (PIXEL_FORMAT_YUV_400 == astFrame[0].stVFrame.enPixelFormat)
    {
         strcpy(szPixFrm,"single");
    }
    else 
    {

    }       
    
    sprintf(szYuvName, "./vi_chn_%d_%d_%d_%s_%d.yuv", ViChn,
        astFrame[0].stVFrame.u32Width, astFrame[0].stVFrame.u32Height,szPixFrm,u32Cnt);
	printf("Dump YUV frame of vi chn %d  to file: width = %d, height = %d \n", ViChn, 
        astFrame[0].stVFrame.u32Width, astFrame[0].stVFrame.u32Height);
    
    /* open file */
    pfd = fopen(szYuvName, "wb");
    if (NULL == pfd)
    {
        return -1;
    }

    /* get VI frame  */
    for (i=1; i<u32Cnt; i++)
    {
        if (HI_MPI_VI_GetFrame(ViChn, &astFrame[i], s32MilliSec) < 0)
        {
            printf("get vi chn %d frame err\n", ViChn);
            printf("only get %d frame\n", i);
            break;
        }
    }

    for(j=0; j<i; j++)
    {
        /* save VI frame to file */
        vi_dump_save_one_frame(&astFrame[j].stVFrame, pfd);

        #ifdef DIS_DATA_DEBUG
        vi_dump_save_one_dis(ViChn, &astFrame[j].stVFrame);
        #endif

        /* release frame after using */
        HI_MPI_VI_ReleaseFrame(ViChn, &astFrame[j]);
    }   

    fclose(pfd);

	return 0;
}

HI_S32 main(int argc, char *argv[])
{
    VI_CHN ViChn = 0;
    HI_U32 u32FrmCnt = 1;

    if (argc > 1)/* VI通道号*/
    {
        ViChn = atoi(argv[1]);
    }

    if (argc > 2)
    {
        u32FrmCnt = atoi(argv[2]);/* 需要采集的帧数目*/
    }

    MEM_DEV_OPEN();

    #if 0
    if (SAMPLE_VI_BackupAttr(ViChn))
        return -1;
    #endif

    SAMPLE_MISC_ViDump(ViChn, u32FrmCnt);


    #if 0
    if (SAMPLE_VI_RestoreAttr(ViChn))
        return -1;
    #endif
    MEM_DEV_CLOSE();

	return HI_SUCCESS;
}



