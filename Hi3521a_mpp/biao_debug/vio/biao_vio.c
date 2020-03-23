/************************************************************
*Copyright (C),lcb0281at163.com lcb0281atgmail.com
*BlogAddr: caibiao-lee.blog.csdn.net
*FileName: biao_vio.c
*Description:测试视频输入输出
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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "sample_comm.h"
#include "biao_vio.h"

void BIAO_VIO_HandleSig(HI_S32 signo)
{
    if (SIGINT == signo || SIGTSTP == signo)
    {
        SAMPLE_COMM_SYS_Exit();
        printf("\033[0;31mprogram termination abnormally!\033[0;39m\n");
    }
    exit(-1);
}

/******************************************************** 
Function:    Check_NVP6134_VideoInputFMT
Description: 检测6134摄像头的输入状态和参数
Input:  none
OutPut: none
Return: 0: success，none 0:error
Others: 
Author: Caibiao Lee
Date:   2020-02-02
*********************************************************/
int Check_NVP6134_VideoInputFMT(void)
{
    int fd = -1;
    int i = 0;
    int l_s32Ret = 0;
    nvp6134_input_videofmt stInputFMT;

    bzero(&stInputFMT,sizeof(nvp6134_input_videofmt));

    fd = open(NVP6134_FILE, O_RDWR);
    if (fd < 0)
    {
        printf("[%s:%d] open nvp6134 (%s) fail\n", __func__, __LINE__, NVP6134_FILE);
        return -1;
    }

    l_s32Ret = ioctl(fd, IOC_VDEC_GET_INPUT_VIDEO_FMT, &stInputFMT);
    if(l_s32Ret < 0)
    {
        printf("[%s,%d][l_s32Ret:%d]\n",__FILE__,__LINE__,l_s32Ret);
    }

    close(fd);


    for(i=0;i<4;i++)
    {
        printf("i=%d videofmt     =0x%x \n",i,stInputFMT.getvideofmt[i]);
        printf("i=%d inputvideofmt=0x%x \n\n",i,stInputFMT.inputvideofmt[i]);
    }

    return 0;
}

/******************************************************** 
Function:    BIAO_VIO_4_720P
Description: 实现4摄像头输入然后将数据从VO的4个通道输出。
Input:  none
OutPut: none
Return: 0: success，none 0:error
Others: SAMPLE_VI_MODE_2_720P 这种模式是我根据我自己设备
        添加的，我设备只有一个6134，所以每个outport设置2
        路复用，2个outport最终      达到4路输入的功能。
Author: Caibiao Lee
Date:   2020-02-02
*********************************************************/
HI_S32 BIAO_VIO_4_720P(HI_VOID)
{
    SAMPLE_VI_MODE_E enViMode = SAMPLE_VI_MODE_2_720P;
    VIDEO_NORM_E enNorm = VIDEO_ENCODING_MODE_PAL;

    HI_U32 u32ViChnCnt = 4;
    HI_S32 s32VpssGrpCnt = 4;
    
    VB_CONF_S stVbConf;
    VPSS_GRP VpssGrp;
    VPSS_GRP_ATTR_S stGrpAttr;
    VPSS_CHN VpssChn_VoHD0 = VPSS_CHN2;
    VPSS_CHN VpssChn_VoSD0 = VPSS_CHN3;
    
    VO_DEV VoDev;
    VO_LAYER VoLayer;
    VO_CHN VoChn;
    VO_PUB_ATTR_S stVoPubAttr;
    VO_VIDEO_LAYER_ATTR_S stLayerAttr;
    SAMPLE_VO_MODE_E enVoMode, enPreVoMode;
    
    HI_S32 i;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32BlkSize;
    HI_CHAR ch;
    SIZE_S stSize;
    HI_U32 u32WndNum;
    
    /******************************************
     step  1: init variable 
    ******************************************/ 
    memset(&stVbConf,0,sizeof(VB_CONF_S));
    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(enNorm,\
                PIC_HD720, SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH,COMPRESS_MODE_SEG);
    stVbConf.u32MaxPoolCnt = 128;

    /* video buffer*/
    //todo: vb=15
    stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[0].u32BlkCnt = u32ViChnCnt * 8;

    /******************************************
     step 2: mpp system init. 
    ******************************************/
    s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("system init failed with %d!\n", s32Ret);
        goto END_4_720P_0;
    }

    /******************************************
     step 3: start vi dev & chn
    ******************************************/
    s32Ret = SAMPLE_COMM_VI_Start(enViMode, enNorm);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("start vi failed!\n");
        goto END_4_720P_0;
    }

    /******************************************
     step 4: start vpss and vi bind vpss
    ******************************************/
    s32Ret = SAMPLE_COMM_SYS_GetPicSize(enNorm, PIC_HD720, &stSize);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
        goto END_4_720P_1;
    }

    memset(&stGrpAttr,0,sizeof(VPSS_GRP_ATTR_S));
    stGrpAttr.u32MaxW = stSize.u32Width;
    stGrpAttr.u32MaxH = stSize.u32Height;
    stGrpAttr.bNrEn = HI_TRUE;
    stGrpAttr.enDieMode = VPSS_DIE_MODE_NODIE;
    stGrpAttr.enPixFmt = SAMPLE_PIXEL_FORMAT;
    s32Ret = SAMPLE_COMM_VPSS_Start(s32VpssGrpCnt, &stSize, VPSS_MAX_CHN_NUM, &stGrpAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Vpss failed!\n");
        goto END_4_720P_1;
    }

    s32Ret = SAMPLE_COMM_VI_BindVpss(enViMode);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Vi bind Vpss failed!\n");
        goto END_4_720P_2;
    }

    /******************************************
     step 5: start vo HD0 (HDMI+VGA), multi-screen, you can switch mode
    ******************************************/
    printf("start vo HD0.\n");
    VoDev = SAMPLE_VO_DEV_DHD0;
    VoLayer = SAMPLE_VO_LAYER_VHD0;
    u32WndNum = 4;
    enVoMode = VO_MODE_4MUX;

    stVoPubAttr.enIntfSync = VO_OUTPUT_1280x1024_60;
    stVoPubAttr.enIntfType = VO_INTF_HDMI|VO_INTF_VGA;
    stVoPubAttr.u32BgColor = 0x000000ff;
    s32Ret = SAMPLE_COMM_VO_StartDev(VoDev, &stVoPubAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start SAMPLE_COMM_VO_StartDev failed!\n");
        goto END_4_720P_3;
    }

    memset(&(stLayerAttr), 0 , sizeof(VO_VIDEO_LAYER_ATTR_S));
    s32Ret = SAMPLE_COMM_VO_GetWH(stVoPubAttr.enIntfSync, \
        &stLayerAttr.stImageSize.u32Width, \
        &stLayerAttr.stImageSize.u32Height, \
        &stLayerAttr.u32DispFrmRt);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start SAMPLE_COMM_VO_GetWH failed!\n");
        goto END_4_720P_3;
    }

    stLayerAttr.enPixFormat = SAMPLE_PIXEL_FORMAT;
    stLayerAttr.stDispRect.s32X       = 0;
    stLayerAttr.stDispRect.s32Y       = 0;
    stLayerAttr.stDispRect.u32Width   = stLayerAttr.stImageSize.u32Width;
    stLayerAttr.stDispRect.u32Height  = stLayerAttr.stImageSize.u32Height;
    s32Ret = SAMPLE_COMM_VO_StartLayer(VoLayer, &stLayerAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start SAMPLE_COMM_VO_StartLayer failed!\n");
        goto END_4_720P_3;
    }

    s32Ret = SAMPLE_COMM_VO_StartChn(VoLayer, enVoMode);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start SAMPLE_COMM_VO_StartChn failed!\n");
        goto END_4_720P_4;
    }

    /* if it's displayed on HDMI, we should start HDMI */
    if (stVoPubAttr.enIntfType & VO_INTF_HDMI)
    {
        if (HI_SUCCESS != SAMPLE_COMM_VO_HdmiStart(stVoPubAttr.enIntfSync))
        {
            SAMPLE_PRT("Start SAMPLE_COMM_VO_HdmiStart failed!\n");
            goto END_4_720P_4;
        }
    }

    for(i=0;i<u32WndNum;i++)
    {
        VoChn = i;
        VpssGrp = i;
        
        s32Ret = SAMPLE_COMM_VO_BindVpss(VoDev,VoChn,VpssGrp,VpssChn_VoHD0);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Start VO failed!\n");
            goto END_4_720P_4;
        }
    }

END_4_720P_5:
    VoDev = SAMPLE_VO_DEV_DSD0;
    VoLayer = SAMPLE_VO_LAYER_VSD0;
    u32WndNum = 8;
    enVoMode = VO_MODE_9MUX;
    SAMPLE_COMM_VO_StopChn(VoLayer, enVoMode);
    for(i=0;i<u32WndNum;i++)
    {
        VoChn = i;
        VpssGrp = i;
        SAMPLE_COMM_VO_UnBindVpss(VoLayer,VoChn,VpssGrp,VpssChn_VoSD0);
    }
    SAMPLE_COMM_VO_StopLayer(VoLayer);
    SAMPLE_COMM_VO_StopDev(VoDev);

END_4_720P_4:

    if (stVoPubAttr.enIntfType & VO_INTF_HDMI)
    {
        SAMPLE_COMM_VO_HdmiStop();
    }

    VoDev = SAMPLE_VO_DEV_DHD0;
    VoLayer = SAMPLE_VO_LAYER_VHD0;
    u32WndNum = 8;
    enVoMode = VO_MODE_9MUX;    
    SAMPLE_COMM_VO_StopChn(VoLayer, enVoMode);
    for(i=0;i<u32WndNum;i++)
    {
        VoChn = i;
        VpssGrp = i;
        SAMPLE_COMM_VO_UnBindVpss(VoLayer,VoChn,VpssGrp,VpssChn_VoHD0);
    }
    SAMPLE_COMM_VO_StopLayer(VoLayer);
    SAMPLE_COMM_VO_StopDev(VoDev);
END_4_720P_3:   //vi unbind vpss
    SAMPLE_COMM_VI_UnBindVpss(enViMode);
END_4_720P_2:   //vpss stop
    SAMPLE_COMM_VPSS_Stop(s32VpssGrpCnt, VPSS_MAX_CHN_NUM);
END_4_720P_1:   //vi stop
    SAMPLE_COMM_VI_Stop(enViMode);
END_4_720P_0:   //system exit
    SAMPLE_COMM_SYS_Exit();
    
    return s32Ret;
}

int BIAO_VIO_DEBUG(void)
{
    HI_S32 s32Ret = HI_FAILURE;

    signal(SIGINT, BIAO_VIO_HandleSig);
    signal(SIGTERM, BIAO_VIO_HandleSig);
   
    Check_NVP6134_VideoInputFMT();
    s32Ret = BIAO_VIO_4_720P();


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


