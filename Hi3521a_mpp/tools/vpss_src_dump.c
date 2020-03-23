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
#include "mpi_vgs.h"

#define VALUE_BETWEEN(x,min,max) (((x)>=(min)) && ((x) <= (max)))

static HI_S32 s_s32MemDev = 0;

typedef struct hiDUMP_MEMBUF_S
{
    VB_BLK  hBlock;
    VB_POOL hPool;
    HI_U32  u32PoolId;
    
    HI_U32  u32PhyAddr;
    HI_U8   *pVirAddr;
    HI_S32  s32Mdev;
} DUMP_MEMBUF_S;

HI_S32 memopen( void )
{
    if (s_s32MemDev <= 0)
    {
        s_s32MemDev = open ("/dev/mem", O_CREAT|O_RDWR|O_SYNC);
        if (s_s32MemDev <= 0)
        {
            return -1;
        }
    }
    return 0;
}

HI_VOID memclose()
{
	close(s_s32MemDev);
}


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


HI_S32 SAMPLE_MISC_VpssDumpSrcImage(VPSS_GRP Grp)
{
    HI_S32 i = 0;
    HI_S32 s32Ret;
    VIDEO_FRAME_INFO_S stFrame;
    HI_CHAR szYuvName[128];
    HI_CHAR szPixFrm[10];
    FILE *pfd;
    VB_POOL hPool  = VB_INVALID_POOLID;
    HI_U32  u32BlkSize = 0;

    DUMP_MEMBUF_S stMem = {0};
    VIDEO_FRAME_INFO_S stFrmInfo;
    VGS_HANDLE hHandle;
    VGS_TASK_ATTR_S stTask;
    HI_U32 u32LumaSize              = 0;
    HI_U32 u32ChrmSize              = 0; 
    HI_U32 u32PicLStride            = 0;
    HI_U32 u32PicCStride            = 0;
    HI_U32 u32Width                 = 0;
    HI_U32 u32Height                = 0;
    HI_BOOL bSendToVgs              = HI_FALSE;
    VPSS_GRP VpssGrp = Grp;
    //HI_S32 s32MilliSec = 0;
    
    /* get frame  */    
    while ((HI_MPI_VPSS_GetGrpFrame(VpssGrp,  &stFrame,0)!=HI_SUCCESS))
    {        
    	printf("get frame error!!!\n");
        sleep(1);
    }      

    /* make file name */
    strcpy(szPixFrm,(PIXEL_FORMAT_YUV_SEMIPLANAR_420 == stFrame.stVFrame.enPixelFormat)?"sp420":"sp422");
    sprintf(szYuvName, "./vpss%d_%d_%d_%s.yuv",VpssGrp,
        stFrame.stVFrame.u32Width, stFrame.stVFrame.u32Height,szPixFrm);
    
    printf("Dump YUV frame of vpss%d to file: \"%s\"\n",VpssGrp, szYuvName);
    
    bSendToVgs = (stFrame.stVFrame.enCompressMode > 0);
    
    /* open file */
    pfd = fopen(szYuvName, "wb");

    if (NULL == pfd)
    {
        return -1;
    }
    
    //memopen();

    u32PicLStride = stFrame.stVFrame.u32Stride[0];
    u32PicCStride = stFrame.stVFrame.u32Stride[0];
    u32LumaSize = stFrame.stVFrame.u32Stride[0]*stFrame.stVFrame.u32Height;
    u32ChrmSize = (stFrame.stVFrame.u32Stride[0] * stFrame.stVFrame.u32Height) >> (1+(PIXEL_FORMAT_YUV_SEMIPLANAR_420 == stFrame.stVFrame.enPixelFormat));
    u32Width    = stFrame.stVFrame.u32Width;
    u32Height   = stFrame.stVFrame.u32Height;

    if(bSendToVgs)
    {
        u32BlkSize = (PIXEL_FORMAT_YUV_SEMIPLANAR_420 == stFrame.stVFrame.enPixelFormat)
            ?(stFrame.stVFrame.u32Stride[0]*stFrame.stVFrame.u32Height*3>>1):(stFrame.stVFrame.u32Stride[0]*stFrame.stVFrame.u32Height*2);
        
        /*create comm vb pool*/
        hPool   = HI_MPI_VB_CreatePool( u32BlkSize,1,NULL);
        if (hPool == VB_INVALID_POOLID)
        {
            printf("HI_MPI_VB_CreatePool failed! \n");
            goto END1;
        }

        stMem.hPool = hPool;
    }

    if(bSendToVgs)
    {
        while((stMem.hBlock = HI_MPI_VB_GetBlock(stMem.hPool, u32BlkSize,NULL)) == VB_INVALID_HANDLE)
        {
             ;
        }

        stMem.u32PhyAddr = HI_MPI_VB_Handle2PhysAddr(stMem.hBlock);


        stMem.pVirAddr = (HI_U8 *) HI_MPI_SYS_Mmap( stMem.u32PhyAddr, u32BlkSize );
        if(stMem.pVirAddr == NULL)
        {
            printf("Mem dev may not open\n");
            HI_MPI_VB_ReleaseBlock(stMem.hBlock);
            goto END2;
        }
    
        memset(&stFrmInfo.stVFrame, 0, sizeof(VIDEO_FRAME_S));
        stFrmInfo.stVFrame.u32PhyAddr[0] = stMem.u32PhyAddr;
        stFrmInfo.stVFrame.u32PhyAddr[1] = stFrmInfo.stVFrame.u32PhyAddr[0] + u32LumaSize;
        stFrmInfo.stVFrame.u32PhyAddr[2] = stFrmInfo.stVFrame.u32PhyAddr[1] + u32ChrmSize;
        
        stFrmInfo.stVFrame.pVirAddr[0] = stMem.pVirAddr;
        stFrmInfo.stVFrame.pVirAddr[1] = (HI_U8 *) stFrmInfo.stVFrame.pVirAddr[0] + u32LumaSize;
        stFrmInfo.stVFrame.pVirAddr[2] = (HI_U8 *) stFrmInfo.stVFrame.pVirAddr[1] + u32ChrmSize;

        stFrmInfo.stVFrame.u32Width     = u32Width;
        stFrmInfo.stVFrame.u32Height    = u32Height;
        stFrmInfo.stVFrame.u32Stride[0] = u32PicLStride;
        stFrmInfo.stVFrame.u32Stride[1] = u32PicCStride;
        stFrmInfo.stVFrame.u32Stride[2] = u32PicCStride;

        stFrmInfo.stVFrame.enCompressMode = COMPRESS_MODE_NONE;
        stFrmInfo.stVFrame.enPixelFormat  = stFrame.stVFrame.enPixelFormat;
        stFrmInfo.stVFrame.enVideoFormat  = VIDEO_FORMAT_LINEAR;

        stFrmInfo.stVFrame.u64pts     = (i * 40);
        stFrmInfo.stVFrame.u32TimeRef = (i * 2);

        stFrmInfo.u32PoolId = hPool;

        s32Ret = HI_MPI_VGS_BeginJob(&hHandle);
        if(s32Ret != HI_SUCCESS)
    	{	
    	    printf("HI_MPI_VGS_BeginJob failed\n");
            HI_MPI_VB_ReleaseBlock(stMem.hBlock);
    	    goto END2;
    	}
   
        memcpy(&stTask.stImgIn,&stFrame.stVFrame,sizeof(VIDEO_FRAME_INFO_S));
        memcpy(&stTask.stImgOut ,&stFrmInfo,sizeof(VIDEO_FRAME_INFO_S));
        s32Ret = HI_MPI_VGS_AddScaleTask(hHandle, &stTask);
        if(s32Ret != HI_SUCCESS)
    	{	
    	    printf("HI_MPI_VGS_AddScaleTask failed\n");
            HI_MPI_VGS_CancelJob(hHandle);
            HI_MPI_VB_ReleaseBlock(stMem.hBlock);
    	    goto END2;
    	}

        s32Ret = HI_MPI_VGS_EndJob(hHandle);//, s32MilliSec);
        if(s32Ret != HI_SUCCESS)
    	{	
    	    printf("HI_MPI_VGS_EndJob failed\n");
            HI_MPI_VGS_CancelJob(hHandle);
            HI_MPI_VB_ReleaseBlock(stMem.hBlock);
    	    goto END2;
    	}
        sleep(1);
        /* save VO frame to file */
        sample_yuv_dump(&stFrmInfo.stVFrame, pfd);

        HI_MPI_VB_ReleaseBlock(stMem.hBlock);
    }
    else
    {
        /* save VO frame to file */
	    sample_yuv_dump(&stFrame.stVFrame, pfd);
    }
    

END2:
    if(hPool != VB_INVALID_POOLID)
    {
        HI_MPI_VB_DestroyPool( hPool );
    }

END1:

    //memclose();

    fclose(pfd);
    
    HI_MPI_VPSS_ReleaseGrpFrame(VpssGrp, &stFrame);

	return 0;
}

HI_S32 main(int argc, char *argv[])
{	 
    VPSS_GRP VpssGrp = 0;

    printf("Usage: ./vpss_dump [Grp] \n");
    printf("Grp: vpss Grp id.\n");
    
    
    if (argc < 2)
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
    
    SAMPLE_MISC_VpssDumpSrcImage(VpssGrp);

    return HI_SUCCESS;
}
