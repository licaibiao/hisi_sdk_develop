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
#include "hi_comm_vpss.h"
#include "mpi_vpss.h"

#define VALUE_BETWEEN(x,min,max) (((x)>=(min)) && ((x) <= (max)))

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

/* sp420 转存为 p420 ; sp422 转存为 p422  */
void sample_yuv_dump(VIDEO_FRAME_S * pVBuf, FILE *pfd)
{
    unsigned int w, h;
    char * pVBufVirt_Y;
    char * pVBufVirt_C;
    char * pMemContent;
    unsigned char TmpBuff[8192];                //如果这个值太小，图像很大的话存不了
    HI_U32 phy_addr,size;
	HI_CHAR *pUserPageAddr[2];
    PIXEL_FORMAT_E  enPixelFormat = pVBuf->enPixelFormat;
    HI_U32 u32UvHeight;/* 存为planar 格式时的UV分量的高度 */
    
    if (PIXEL_FORMAT_YUV_SEMIPLANAR_420 == enPixelFormat)
    {
        size = (pVBuf->u32Stride[0])*(pVBuf->u32Height)*3/2;    
        u32UvHeight = pVBuf->u32Height/2;
    }
    else
    {
        size = (pVBuf->u32Stride[0])*(pVBuf->u32Height)*2;   
        u32UvHeight = pVBuf->u32Height;
    }

    phy_addr = pVBuf->u32PhyAddr[0];

    //printf("phy_addr:%x, size:%d\n", phy_addr, size);
    pUserPageAddr[0] = (HI_CHAR *) HI_MPI_SYS_Mmap(phy_addr, size);	
    if (NULL == pUserPageAddr[0])
    {
        return;
    }
    //printf("stride: %d,%d\n",pVBuf->u32Stride[0],pVBuf->u32Stride[1] );
    
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
    
    fprintf(stderr, "done %d!\n", pVBuf->u32TimeRef);
    fflush(stderr);
    
    HI_MPI_SYS_Munmap(pUserPageAddr[0], size);    
}


HI_VOID *SAMPLE_MISC_VpssDump(VPSS_GRP Grp,VPSS_CHN Chn,HI_U32 u32FrameCnt,HI_U32 u32Width,HI_U32 u32Height,HI_U32 u32PixelFormat)
{	
    VIDEO_FRAME_INFO_S stFrame;
    HI_CHAR szYuvName[128];
    HI_CHAR szPixFrm[10];
    FILE *pfd;  
    VPSS_GRP VpssGrp = Grp;
    VPSS_CHN VpssChn = Chn;
    HI_U32 u32Cnt = u32FrameCnt;
    HI_U32 u32Depth = 2;   
    HI_U32 u32OrigDepth = 0;
    VPSS_CHN_MODE_S stOrigVpssMode, stVpssMode={0};
    HI_S32 s32MilliSec = 2000;
    
    if (HI_MPI_VPSS_GetChnMode(VpssGrp,VpssChn,&stOrigVpssMode) != HI_SUCCESS)
    {
    	printf("get mode error!!!\n");
        return (HI_VOID *)-1;
    }

    if (HI_MPI_VPSS_GetDepth(VpssGrp, VpssChn, &u32OrigDepth) != HI_SUCCESS)
    {
        printf("get depth error!!!\n");
        return (HI_VOID *)-1;
    }

    stVpssMode = stOrigVpssMode;
    stVpssMode.enChnMode = VPSS_CHN_MODE_USER;
    if (u32PixelFormat == 0)
    {
        stVpssMode.enPixelFormat = PIXEL_FORMAT_YUV_SEMIPLANAR_420;    	
    }
    else
    {
       stVpssMode.enPixelFormat = PIXEL_FORMAT_YUV_SEMIPLANAR_422;    	
    }
    stVpssMode.u32Width = u32Width;
    stVpssMode.u32Height = u32Height;
    stVpssMode.enCompressMode = COMPRESS_MODE_NONE;
    stVpssMode.bDouble = HI_FALSE;
    if (HI_MPI_VPSS_SetChnMode(VpssGrp,VpssChn,&stVpssMode) != HI_SUCCESS)
    {
    	printf("set mode error!!!\n");
        return (HI_VOID *)-1;
    }
    if (HI_MPI_VPSS_SetDepth(VpssGrp,VpssChn,u32Depth)!=HI_SUCCESS)
    {
    	printf("set depth error!!!\n");
        return (HI_VOID *)-1;
    }
    
    memset(&stFrame,0,sizeof(stFrame));
    while (HI_MPI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stFrame, s32MilliSec)!=HI_SUCCESS)
    {
    	printf("get frame error!!!\n");
        usleep(40000);
    } 
    
    /* make file name */
    strcpy(szPixFrm, 
           (PIXEL_FORMAT_YUV_SEMIPLANAR_420 == stFrame.stVFrame.enPixelFormat)?"sp420":"sp422");    
    sprintf(szYuvName, "./vpss_grp%d_chn%d_w%d_h%d_%s_%d.yuv", VpssGrp, VpssChn, 
        stFrame.stVFrame.u32Width, stFrame.stVFrame.u32Height,szPixFrm,u32Cnt);        
	printf("Dump YUV frame of vpss chn %d  to file: \"%s\"\n", VpssChn, szYuvName);
    fflush(stdout);

    HI_MPI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stFrame);	 
    /* open file */
    pfd = fopen(szYuvName, "wb");
    
    if (NULL == pfd)
    {
        return (HI_VOID *)-1;
    }
    
    /* get frame  */    
    while (u32Cnt--)
    {        
        if (HI_MPI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stFrame, s32MilliSec) != HI_SUCCESS)
        {        
            printf("Get frame fail \n");
            usleep(1000);
            continue;
        }
        sample_yuv_dump(&stFrame.stVFrame, pfd);
        
        printf("Get VpssGrp %d frame %d!!\n", VpssGrp,u32Cnt);
        /* release frame after using */
        HI_MPI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stFrame);  
    }
    fclose(pfd);

    HI_MPI_VPSS_SetDepth(VpssGrp, VpssChn, u32OrigDepth);
    HI_MPI_VPSS_SetChnMode(VpssGrp,VpssChn,&stOrigVpssMode); 
    
    return (HI_VOID *)0;
}

HI_S32 main(int argc, char *argv[])
{	 
    VPSS_GRP VpssGrp = 0;
    VPSS_CHN VpssChn = 0;
    HI_U32 u32FrmCnt = 1;
    HI_U32 u32Width = 720;
    HI_U32 u32Height = 576;
    HI_U32 u32PixelFormat = 0;

    printf("Usage: ./vpss_dump [Grp] [Chn] [Count] [Width] [Height] [PixelFormat]\n");
    printf("Grp: vpss Grp id.\n");
    printf("Chn: vpss Chn id.\n");
    printf("Count: frame count.\n");       
    printf("Width: image width.\n");        
    printf("Height: image Height.\n");
    printf("PixelFormat: image pixel format, 0 for semiplanar420 ang 1 for semiplanar422.\n");      
    
    if (argc < 7)
    {
    	printf("wrong arg!!!!\n\n");
    	return -1;
    }
      
    VpssGrp = atoi(argv[1]);
    if (!VALUE_BETWEEN(VpssGrp, 0, VPSS_MAX_GRP_NUM))
    {
    	printf("grp id must be [0,%d]!!!!\n\n", VPSS_MAX_GRP_NUM);
    	return -1;
    }

    VpssChn = atoi(argv[2]);/* chn id*/
    if (!VALUE_BETWEEN(VpssChn, 0, VPSS_MAX_CHN_NUM))
    {
    	printf("chn id must be [0,%d]!!!!\n\n", VPSS_MAX_CHN_NUM);
    	return -1;
    }
    u32FrmCnt = atoi(argv[3]);/* frame count*/

    u32Width = atoi(argv[4]);
    if (!VALUE_BETWEEN(u32Width, 64, 8192))
    {
    	printf("image width must be [64, 8192]!!!!\n\n");
    	return -1;
    }
    u32Height = atoi(argv[5]);   
    if (!VALUE_BETWEEN(u32Height, 64, 8192)) 
    {
    	printf("image height must be [64, 8192]!!!!\n\n");
    	return -1;
    } 
    u32PixelFormat = atoi(argv[6]);    
    if (!VALUE_BETWEEN(u32PixelFormat, 0, 1))     
    {
    	printf("image pixelformat must be [0,1]!!!!\n\n");
    	return -1;
    } 

    MEM_DEV_OPEN();
    SAMPLE_MISC_VpssDump(VpssGrp,VpssChn,u32FrmCnt,u32Width,u32Height,u32PixelFormat);
    MEM_DEV_CLOSE();

    return HI_SUCCESS;
}
