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
#include "hi_comm_sys.h"
#include "mpi_sys.h"
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
    
    printf("usage: ./vo_vga_config\n\t[voDev]:  [0 1]\n\t[Matrix] :[3,4]\n\t[Luma]: [0,100] \n\t[Contrast]: [0,100] \n\t[Hue]: [0,100] \n\t[Satuature]: [0,100] \n\t[u32Gain]: [0,0x3f] \n\t[strength]: [0,255]\n");
    printf("sample: ./vo_vga_config 0 3 50 50 50 50 20 128\n");
}

HI_S32 vo_vga_config(HI_U32 u32Dev,HI_U32 enCscMode,HI_U32 u32Luma, HI_U32 u32Contrast, HI_U32 u32Hue, HI_U32 u32Saturation, HI_U32 u32Gain, HI_U32 u32Strength)
{
    VO_VGA_PARAM_S stVgaParam;
    usage();
    if(u32Dev < 0|| u32Dev >1)
    {
        usage();
        return -1;
    }
    if (enCscMode != 3 && enCscMode != 4)
    {
        usage();
        return -1;
    }
    if (u32Contrast < 0 || u32Contrast > 100)
    {
        usage();
        return -1;
    }
    if (u32Hue < 0 || u32Hue > 100)
    {
        usage();
        return -1;
    }
    if (u32Luma < 0 || u32Luma > 100)
    {
        usage();
        return -1;
    }
    if (u32Saturation < 0 || u32Saturation > 100)
    {
        usage();
        return -1;
    }
    if (u32Gain < 0x0 || u32Gain > 0x3F)
    {
        usage();
        return -1;
    }
    if(u32Strength < 0 || u32Strength > 255)
    {
        usage();
        return -1;
    }

    stVgaParam.stCSC.enCscMatrix = enCscMode;
    stVgaParam.stCSC.u32Contrast = u32Contrast;
    stVgaParam.stCSC.u32Hue = u32Hue;
    stVgaParam.stCSC.u32Luma = u32Luma;
    stVgaParam.stCSC.u32Saturation = u32Saturation;
    stVgaParam.u32Gain = u32Gain;
    stVgaParam.s32SharpenStrength = u32Strength;
    VO_CHECK_RET(HI_MPI_VO_SetVgaParam(u32Dev, &stVgaParam), "HI_MPI_VO_SetVgaParam");
    VO_CHECK_RET(HI_MPI_VO_GetVgaParam(u32Dev, &stVgaParam), "HI_MPI_VO_SetVgaParam");    
    printf("matrix %d, con %d, hue %d, luma %d, stu %d, sharp %d\n", stVgaParam.stCSC.enCscMatrix,
        stVgaParam.stCSC.u32Contrast, stVgaParam.stCSC.u32Hue, stVgaParam.stCSC.u32Luma, stVgaParam.stCSC.u32Saturation,
        stVgaParam.s32SharpenStrength);
    
    return HI_SUCCESS;
}

HI_S32 main(int argc, char *argv[])
{
    HI_U32 u32Luma;
    HI_U32 u32Contrast;
    HI_U32 u32Hue;
    HI_U32 u32Saturation;
    HI_U32 u32Gain; 
    HI_U32 u32Matrix;    
    HI_U32 u32Strength;    
    HI_U32 u32Dev;
    
    if (argc > 1)
    {
        u32Dev = atoi(argv[1]);
    }

	if (argc > 2)
    {
        u32Matrix = atoi(argv[2]);
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
    
    if (argc > 8)
    {
        u32Strength = atoi(argv[8]);
    }

    vo_vga_config(u32Dev,u32Matrix,u32Luma, u32Contrast, u32Hue, u32Saturation,u32Gain, u32Strength);

	return HI_SUCCESS;
}


