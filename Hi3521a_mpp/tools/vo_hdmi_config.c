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
    
    printf("usage: ./vo_hdmi_config\n\t[voDev]:  [0 1]\n\t[Matrix] :[0,2]\n\t[Luma]: [0,100] \n\t[Contrast]: [0,100] \n\t[Hue]: [0,100] \n\t[Satuature]: [0,100]\n");
    printf("sample: ./vo_hdmi_config 0 0 50 50 50 50\n");
}

HI_S32 vo_hdmi_config(HI_U32 u32Dev,HI_U32 enCscMode,HI_U32 u32Luma, HI_U32 u32Contrast, HI_U32 u32Hue, HI_U32 u32Saturation)
{
    VO_HDMI_PARAM_S stHdmiParam;
    usage();
    if(u32Dev != 0)
    {
        usage();
        return -1;
    }
    if (enCscMode > VO_CSC_MATRIX_BT709_TO_BT601)
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

    stHdmiParam.stCSC.enCscMatrix = enCscMode;
    stHdmiParam.stCSC.u32Contrast = u32Contrast;
    stHdmiParam.stCSC.u32Hue = u32Hue;
    stHdmiParam.stCSC.u32Luma = u32Luma;
    stHdmiParam.stCSC.u32Saturation = u32Saturation;
    VO_CHECK_RET(HI_MPI_VO_SetHdmiParam(u32Dev, &stHdmiParam), "HI_MPI_VO_SetHdmiParam");
    VO_CHECK_RET(HI_MPI_VO_GetHdmiParam(u32Dev, &stHdmiParam), "HI_MPI_VO_GetHdmiParam");    
    printf("get cur : matrix %d, luma %d, con %d, hue %d, stu %d\n", stHdmiParam.stCSC.enCscMatrix,
        stHdmiParam.stCSC.u32Luma, stHdmiParam.stCSC.u32Contrast, stHdmiParam.stCSC.u32Hue, stHdmiParam.stCSC.u32Saturation);
    
    return HI_SUCCESS;
}

HI_S32 main(int argc, char *argv[])
{
    HI_U32 u32Luma;
    HI_U32 u32Contrast;
    HI_U32 u32Hue;
    HI_U32 u32Saturation;
    HI_U32 u32Matrix;       
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

    vo_hdmi_config(u32Dev,u32Matrix,u32Luma, u32Contrast, u32Hue, u32Saturation);

	return HI_SUCCESS;
}


