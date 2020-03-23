/************************************************************
*Copyright (C),lcb0281at163.com lcb0281atgmail.com
*BlogAddr: caibiao-lee.blog.csdn.net
*FileName: biao_venc.c
*Description:所有测试函数的入口
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

#include "sample_comm.h"

VIDEO_NORM_E gs_enNorm = VIDEO_ENCODING_MODE_NTSC;
#define SAMPLE_YUV_D1_FILEPATH         "SAMPLE_420_D1.yuv"
#define VPSS_BSTR_CHN           0
#define VPSS_LSTR_CHN           1


/******************************************************************************
* function : to process abnormal case                                         
******************************************************************************/
void BIAO_VENC_HandleSig(HI_S32 signo)
{
    if (SIGINT == signo || SIGTSTP == signo)
    {
        SAMPLE_COMM_SYS_Exit();
        printf("\033[0;31mprogram termination abnormally!\033[0;39m\n");
    }
    exit(-1);
}

/******************************************************************************
* function : to process abnormal case - the case of stream venc
******************************************************************************/
void BIAO_VENC_StreamHandleSig(HI_S32 signo)
{

    if (SIGINT == signo || SIGTSTP == signo)
    {
        SAMPLE_COMM_SYS_Exit();
        printf("\033[0;31mprogram exit abnormally!\033[0;39m\n");
    }

    exit(0);
}

HI_S32 BIAO_VENC_SVC_T_H264(HI_VOID)
{
    SAMPLE_VI_MODE_E enViMode = SAMPLE_VI_MODE_8_720P;

    HI_S32 s32VpssGrpCnt = 8;
    PAYLOAD_TYPE_E enPayLoad= PT_H264;
    PIC_SIZE_E enSize = PIC_HD720;
    HI_U32 u32Profile = 3;/*0: baseline; 1:MP; 2:HP 3:svc-t */
    VB_CONF_S stVbConf;
    VPSS_GRP VpssGrp;
    //VPSS_GRP_ATTR_S stGrpAttr;
    VENC_CHN VencChn;
    SAMPLE_RC_E enRcMode;
    
    HI_S32 i;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32BlkSize;
    HI_CHAR ch;
    SIZE_S stSize;

    /******************************************
     step  1: init variable 
    ******************************************/
    memset(&stVbConf,0,sizeof(VB_CONF_S));

    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(gs_enNorm,\
                PIC_HD720, SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH,COMPRESS_MODE_SEG);
    stVbConf.u32MaxPoolCnt = 128;

    /* video buffer*/
    stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[0].u32BlkCnt = s32VpssGrpCnt * 12;
    memset(stVbConf.astCommPool[0].acMmzName,0,
        sizeof(stVbConf.astCommPool[0].acMmzName));

    /******************************************
     step 2: mpp system init. 
    ******************************************/
    s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("system init failed with %d!\n", s32Ret);
        goto END_VENC_1HD_0;
    }

    /******************************************
     step 3: start vi dev & chn to capture
    ******************************************/
    s32Ret = SAMPLE_COMM_VI_Start(enViMode, gs_enNorm);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("start vi failed!\n");
        goto END_VENC_1HD_0;
    }
    
    /******************************************
     step 4: start vpss and vi bind vpss
    ******************************************/
    s32Ret = SAMPLE_COMM_SYS_GetPicSize(gs_enNorm, PIC_HD720, &stSize);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
        goto END_VENC_1HD_0;
    }
    
    s32Ret = SAMPLE_COMM_VPSS_Start(s32VpssGrpCnt, &stSize, 1, HI_NULL);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Vpss failed!\n");
        goto END_VENC_1HD_1;
    }

    s32Ret = SAMPLE_COMM_VI_BindVpss(enViMode);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Vi bind Vpss failed!\n");
        goto END_VENC_1HD_2;
    }

    /******************************************
     step 5: select rc mode
    ******************************************/
    while(1)
    {
        printf("please choose rc mode:\n"); 
        printf("\t0) CBR\n"); 
        printf("\t1) VBR\n"); 
        printf("\t2) FIXQP\n"); 
        ch = getchar();
        if(10 == ch)
        continue;
        getchar();
        if ('0' == ch)
        {
            enRcMode = SAMPLE_RC_CBR;
            break;
        }
        else if ('1' == ch)
        {
            enRcMode = SAMPLE_RC_VBR;
            break;
        }
        else if ('2' == ch)
        {
            enRcMode = SAMPLE_RC_FIXQP;
            break;
        }
        else
        {
            printf("rc mode invaild! please try again.\n");
            continue;
        }
    }
    /******************************************
     step 6: start stream venc (big + little)
    ******************************************/
    for (i=0; i<s32VpssGrpCnt; i++)
    {
        /*** main frame **/
        VencChn = i;
        VpssGrp = i;
        s32Ret = SAMPLE_COMM_VENC_Start(VencChn, enPayLoad,\
                                        gs_enNorm, enSize, enRcMode,u32Profile);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Start Venc failed!\n");
            goto END_VENC_1HD_3;
        }

        s32Ret = SAMPLE_COMM_VENC_BindVpss(VencChn, VpssGrp, 0);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Start Venc failed!\n");
            goto END_VENC_1HD_3;
        }

    }

    /******************************************
     step 7: stream venc process -- get stream, then save it to file. 
    ******************************************/
    s32Ret = SAMPLE_COMM_VENC_StartGetStream(s32VpssGrpCnt);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Venc failed!\n");
        goto END_VENC_1HD_3;
    }

    printf("please press twice ENTER to exit this sample\n");
    getchar();

    /******************************************
     step 8: exit process
    ******************************************/
    SAMPLE_COMM_VENC_StopGetStream();
    
END_VENC_1HD_3:
    for (i=0; i<s32VpssGrpCnt; i++)
    {
        VencChn = i;
        VpssGrp = i;
        SAMPLE_COMM_VENC_UnBindVpss(VencChn, VpssGrp, 0);
        SAMPLE_COMM_VENC_Stop(VencChn);
    }
    SAMPLE_COMM_VI_UnBindVpss(enViMode);
END_VENC_1HD_2:
    SAMPLE_COMM_VPSS_Stop(s32VpssGrpCnt, 1);
END_VENC_1HD_1:
    SAMPLE_COMM_VI_Stop(enViMode);
END_VENC_1HD_0:
    SAMPLE_COMM_SYS_Exit();
    
    return s32Ret;
}


int BIAO_VENC_DEBUG(void)
{
    HI_S32 s32Ret;

    signal(SIGINT, BIAO_VENC_HandleSig);
    signal(SIGTERM, BIAO_VENC_HandleSig);
    
    s32Ret = BIAO_VENC_SVC_T_H264();
    if (HI_SUCCESS == s32Ret)
    {
        printf("program exit normally!\n");
    }
    else
    {
        printf("program exit abnormally!\n");
    }
    
    exit(s32Ret);
}


