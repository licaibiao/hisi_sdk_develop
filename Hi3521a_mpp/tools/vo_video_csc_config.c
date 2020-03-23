/******************************************************************************

  Copyright (C), 2001-2011, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : vga_csc.c
  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       : 2012/03/15
  Description   : 
  History       :
  1.Date        : 2012/03/15
    Author      : n00168968
    Modification: Created file

******************************************************************************/
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
#include "hi_comm_vo.h"
#include "mpi_vo.h"
#include "hi_defines.h"

#define VO_CHECK_RET(express,name)\
    do{\
        HI_S32 Ret;\
        Ret = express;\
        if (HI_SUCCESS != Ret)\
        {\
            printf("%s failed at %s: LINE: %d with %#x!\n", name, __FUNCTION__, __LINE__, Ret);\
        }\
    }while(0)

HI_VOID usage(HI_VOID)
{    
    printf("\nNOTICE: This tool only can be used for TESTING !!!\n");    
    printf("NOTICE: Luma/Contrast/Hue/Satuature [0,100], Gain [0, 0x3f],enCscMatrix[0,2]\n");
    printf("enCscMatrix : 0 - (identity);1-(601-709);2-(709-601).\n");
    printf("usage: ./vo_video_csc_config [LayerId][enCscMatrix] [Luma] [Contrast] [Hue] [Satuature] [Gain]. sample: ./vo_video_csc_config 0 1 50 50 50 50 48\n\n");
}

HI_S32 vo_video_csc_config(HI_U32 u32LayerId, HI_U32 enCscMatrix, HI_U32 u32Luma, HI_U32 u32Contrast, HI_U32 u32Hue, HI_U32 u32Saturation, HI_U32 u32Gain)
{
    VO_CSC_S    stVideoCSC;
    usage();

    if (u32LayerId < 0 || u32LayerId > 4)
    {
        printf ("layer err \n");
        usage();
        return -1;
    }
    
    if (u32Contrast < 0 || u32Contrast > 100)
    {
        printf ("u32Contrast err \n");
        usage();
        return -1;
    }
    if (u32Hue < 0 || u32Hue > 100)
    {
        printf ("u32Hue err \n");
        usage();
        return -1;
    }
    if (u32Luma < 0 || u32Luma > 100)
    {
        printf ("u32Luma err \n");
        usage();
        return -1;
    }
    if (u32Saturation < 0 || u32Saturation > 100)
    {
        printf ("u32Saturation err \n");
        usage();
        return -1;
    }
    if (u32Gain < 0x0 || u32Gain > 0x3F)
    {
        printf ("u32Gain err \n");
        usage();
        return -1;
    }
    if (enCscMatrix < 0 || enCscMatrix > 2)
    {
        printf ("enCscMatrix err \n");
        usage();
        return -1;
    }
    stVideoCSC.enCscMatrix = enCscMatrix;
    stVideoCSC.u32Contrast = u32Contrast;
    stVideoCSC.u32Hue = u32Hue;
    stVideoCSC.u32Luma = u32Luma;
    stVideoCSC.u32Saturation = u32Saturation;
    VO_CHECK_RET(HI_MPI_VO_SetVideoLayerCSC(u32LayerId, &stVideoCSC), "HI_MPI_VO_SetVideoLayerCSC");
    VO_CHECK_RET(HI_MPI_VO_GetVideoLayerCSC(u32LayerId, &stVideoCSC), "HI_MPI_VO_GetVideoLayerCSC");    
    printf("matrix %d, con %d, hue %d, luma %d, stu %d\n", stVideoCSC.enCscMatrix,
        stVideoCSC.u32Contrast, stVideoCSC.u32Hue, stVideoCSC.u32Luma, stVideoCSC.u32Saturation);
    return HI_SUCCESS;
}

HI_S32 main(int argc, char *argv[])
{
    HI_U32 u32LayerId;
    HI_U32 u32Luma;
    HI_U32 u32Contrast;
    HI_U32 u32Hue;
    HI_U32 u32Saturation;
    HI_U32 u32Gain;    
    HI_U32 enCscMatrix; 

    if (argc > 1)
    {
        u32LayerId = atoi(argv[1]);
    }

    if (argc > 2)
    {
        enCscMatrix = atoi(argv[2]);
    }

	if (argc > 3)
    {
        u32Luma = atoi(argv[3]);
    }

	if (argc > 4)
    {
        u32Contrast = atoi(argv[4]);
    }

	if (argc > 5)
    {
        u32Hue = atoi(argv[5]);
    }

    if (argc > 6)
    {
        u32Saturation = atoi(argv[6]);
    }

    if (argc > 7)
    {
        u32Gain = atoi(argv[7]);
    }

    vo_video_csc_config(u32LayerId, enCscMatrix,u32Luma, u32Contrast, u32Hue, u32Saturation, u32Gain);

	return HI_SUCCESS;
}


