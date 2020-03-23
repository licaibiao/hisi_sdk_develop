/******************************************************************************
  A simple program of Hisilicon HI3521A video input and output implementation.
  Copyright (C), 2014-2015, Hisilicon Tech. Co., Ltd.
 ******************************************************************************
    Modification:  2015-1 Created
******************************************************************************/

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

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

#define HDMI_SUPPORT

/******************************************************************************
* function : show usage
******************************************************************************/
void SAMPLE_VIO_Usage(char *sPrgNm)
{
    printf("Usage : %s + <index> (eg: %s 0)\n", sPrgNm,sPrgNm);
    printf("index:\n");
	printf("\t0:  VI 8chn, 720P input, HD/SD VO \n");
	printf("\t1:  VI 16chn ,960H input, HD/SD VO\n");
	printf("\t2:  VI 8chn ,dual ,1080p input, HD wbc SD VO\n");
	printf("\t3:  VI 1Mux D1 input, HD ZoomIn\n");
	printf("\t4:  VI 1Mux D1 input, SD ZommIn\n");
	printf("\tq:  quit\n");

    return;
}

/******************************************************************************
* function : to process abnormal case                                         
******************************************************************************/
void SAMPLE_VIO_HandleSig(HI_S32 signo)
{
    if (SIGINT == signo || SIGTSTP == signo)
    {
        SAMPLE_COMM_SYS_Exit();
        printf("\033[0;31mprogram termination abnormally!\033[0;39m\n");
    }
    exit(-1);
}

/******************************************************************************
* function : VI(720P: 8 windows) -> VPSS -> HD0(1080P)
                                         -> SD0(D1)
******************************************************************************/
HI_S32 SAMPLE_VIO_8_720P(HI_VOID)
{
	SAMPLE_VI_MODE_E enViMode = SAMPLE_VI_MODE_8_720P;
	VIDEO_NORM_E enNorm = VIDEO_ENCODING_MODE_NTSC;

	HI_U32 u32ViChnCnt = 8;
	HI_S32 s32VpssGrpCnt = 8;
	
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
		goto END_8_720P_0;
	}

	/******************************************
	 step 3: start vi dev & chn
	******************************************/
	s32Ret = SAMPLE_COMM_VI_Start(enViMode, enNorm);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("start vi failed!\n");
		goto END_8_720P_0;
	}

	/******************************************
	 step 4: start vpss and vi bind vpss
	******************************************/
	s32Ret = SAMPLE_COMM_SYS_GetPicSize(enNorm, PIC_HD720, &stSize);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
		goto END_8_720P_1;
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
		goto END_8_720P_1;
	}

	s32Ret = SAMPLE_COMM_VI_BindVpss(enViMode);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Vi bind Vpss failed!\n");
		goto END_8_720P_2;
	}

	/******************************************
	 step 5: start vo HD0 (HDMI+VGA), multi-screen, you can switch mode
	******************************************/
	printf("start vo HD0.\n");
	VoDev = SAMPLE_VO_DEV_DHD0;
	VoLayer = SAMPLE_VO_LAYER_VHD0;
	u32WndNum = 8;
	enVoMode = VO_MODE_9MUX;

	stVoPubAttr.enIntfSync = VO_OUTPUT_1080P60;
	stVoPubAttr.enIntfType = VO_INTF_HDMI|VO_INTF_VGA;
	stVoPubAttr.u32BgColor = 0x000000ff;
	s32Ret = SAMPLE_COMM_VO_StartDev(VoDev, &stVoPubAttr);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_StartDev failed!\n");
		goto END_8_720P_3;
	}

	memset(&(stLayerAttr), 0 , sizeof(VO_VIDEO_LAYER_ATTR_S));
	s32Ret = SAMPLE_COMM_VO_GetWH(stVoPubAttr.enIntfSync, \
		&stLayerAttr.stImageSize.u32Width, \
		&stLayerAttr.stImageSize.u32Height, \
		&stLayerAttr.u32DispFrmRt);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_GetWH failed!\n");
		goto END_8_720P_3;
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
		goto END_8_720P_3;
	}

	s32Ret = SAMPLE_COMM_VO_StartChn(VoLayer, enVoMode);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_StartChn failed!\n");
		goto END_8_720P_4;
	}
#ifdef HDMI_SUPPORT
	/* if it's displayed on HDMI, we should start HDMI */
	if (stVoPubAttr.enIntfType & VO_INTF_HDMI)
	{
		if (HI_SUCCESS != SAMPLE_COMM_VO_HdmiStart(stVoPubAttr.enIntfSync))
		{
			SAMPLE_PRT("Start SAMPLE_COMM_VO_HdmiStart failed!\n");
			goto END_8_720P_4;
		}
	}
#endif
	for(i=0;i<u32WndNum;i++)
	{
		VoChn = i;
		VpssGrp = i;
		
		s32Ret = SAMPLE_COMM_VO_BindVpss(VoDev,VoChn,VpssGrp,VpssChn_VoHD0);
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("Start VO failed!\n");
			goto END_8_720P_4;
		}
	}
	/******************************************
	step 6: start vo SD0 (CVBS)
	******************************************/
	printf("start vo SD0\n");
	VoDev = SAMPLE_VO_DEV_DSD0;
	VoLayer = SAMPLE_VO_LAYER_VSD0;
	u32WndNum = 8;
	enVoMode = VO_MODE_9MUX;
	
	stVoPubAttr.enIntfSync = VO_OUTPUT_NTSC;
	stVoPubAttr.enIntfType = VO_INTF_CVBS;
	stVoPubAttr.u32BgColor = 0x000000ff;
	s32Ret = SAMPLE_COMM_VO_StartDev(VoDev, &stVoPubAttr);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_StartDev failed!\n");
		goto END_8_720P_4;
	}
	
	memset(&(stLayerAttr), 0 , sizeof(VO_VIDEO_LAYER_ATTR_S));
	s32Ret = SAMPLE_COMM_VO_GetWH(stVoPubAttr.enIntfSync, \
		&stLayerAttr.stImageSize.u32Width, \
		&stLayerAttr.stImageSize.u32Height, \
		&stLayerAttr.u32DispFrmRt);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_GetWH failed!\n");
		goto END_8_720P_4;
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
		goto END_8_720P_4;
	}
	
	s32Ret = SAMPLE_COMM_VO_StartChn(VoLayer, enVoMode);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_StartChn failed!\n");
		goto END_8_720P_5;
	}
	
	for(i=0;i<u32WndNum;i++)
	{
		VoChn = i;
		VpssGrp = i;
		
		s32Ret = SAMPLE_COMM_VO_BindVpss(VoLayer,VoChn,VpssGrp,VpssChn_VoSD0);
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("Start VO failed!\n");
			goto END_8_720P_5;
		}
	}

	/******************************************
	step 7: HD0 switch mode 
	******************************************/
	VoDev = SAMPLE_VO_DEV_DHD0;
	VoLayer = SAMPLE_VO_LAYER_VHD0;
	enVoMode = VO_MODE_9MUX;
	while(1)
	{
		enPreVoMode = enVoMode;
	
		printf("please choose preview mode, press 'q' to exit this sample.\n"); 
		printf("\t0) 1 preview\n");
		printf("\t1) 4 preview\n");
		printf("\t2) 8 preview\n");
		printf("\tq) quit\n");
 
		ch = getchar();
        if(10 == ch)
        {
            continue;
        }
		getchar();
		if ('0' == ch)
		{
			u32WndNum = 1;
			enVoMode = VO_MODE_1MUX;
		}
		else if ('1' == ch)
		{
			u32WndNum = 4;
			enVoMode = VO_MODE_4MUX;
		}
		/*Indeed only 8 chns show*/
		else if ('2' == ch)
		{
			u32WndNum = 9;
			enVoMode = VO_MODE_9MUX;
		}
		else if ('q' == ch)
		{
			break;
		}
		else
		{
			SAMPLE_PRT("preview mode invaild! please try again.\n");
			continue;
		}
		SAMPLE_PRT("vo(%d) switch to %d mode\n", VoDev, u32WndNum);

		s32Ret= HI_MPI_VO_SetAttrBegin(VoLayer);
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("Start VO failed!\n");
			goto END_8_720P_5;
		}
		
		s32Ret = SAMPLE_COMM_VO_StopChn(VoLayer, enPreVoMode);
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("Start VO failed!\n");
			goto END_8_720P_5;
		}

		s32Ret = SAMPLE_COMM_VO_StartChn(VoLayer, enVoMode);
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("Start VO failed!\n");
			goto END_8_720P_5;
		}
		s32Ret= HI_MPI_VO_SetAttrEnd(VoLayer);
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("Start VO failed!\n");
			goto END_8_720P_5;
		}
	}

	/******************************************
	 step 8: exit process
	******************************************/
END_8_720P_5:
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

END_8_720P_4:
	#ifdef HDMI_SUPPORT
	if (stVoPubAttr.enIntfType & VO_INTF_HDMI)
	{
		SAMPLE_COMM_VO_HdmiStop();
	}
	#endif
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
END_8_720P_3:	//vi unbind vpss
	SAMPLE_COMM_VI_UnBindVpss(enViMode);
END_8_720P_2:	//vpss stop
	SAMPLE_COMM_VPSS_Stop(s32VpssGrpCnt, VPSS_MAX_CHN_NUM);
END_8_720P_1:	//vi stop
	SAMPLE_COMM_VI_Stop(enViMode);
END_8_720P_0:	//system exit
	SAMPLE_COMM_SYS_Exit();
	
	return s32Ret;
}

/******************************************************************************
* function : VI(960H@30: 8 windows) -> VPSS -> HD0(1080P@60)
                                    -> SD0(D1)
******************************************************************************/
HI_S32 SAMPLE_VIO_16_960H(HI_VOID)
{
	SAMPLE_VI_MODE_E enViMode = SAMPLE_VI_MODE_16_960H;
	VIDEO_NORM_E enNorm = VIDEO_ENCODING_MODE_NTSC;

	HI_U32 u32ViChnCnt = 16;
	HI_S32 s32VpssGrpCnt = 16;
	
	VB_CONF_S stVbConf;
	VI_CHN ViChn;
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
				PIC_960H, SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH,COMPRESS_MODE_SEG);
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
		goto END_8_960H_0;
	}

	/******************************************
	 step 3: start vi dev & chn
	******************************************/
	s32Ret = SAMPLE_COMM_VI_Start(enViMode, enNorm);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("start vi failed!\n");
		goto END_8_960H_0;
	}

	/******************************************
	 step 4: start vpss and vi bind vpss
	******************************************/
	s32Ret = SAMPLE_COMM_SYS_GetPicSize(enNorm, PIC_960H, &stSize);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
		goto END_8_960H_1;
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
		goto END_8_960H_1;
	}

	s32Ret = SAMPLE_COMM_VI_BindVpss(enViMode);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Vi bind Vpss failed!\n");
		goto END_8_960H_2;
	}

	/******************************************
	 step 5: start vo HD0 (HDMI+VGA), multi-screen, you can switch mode
	******************************************/
	printf("start vo HD0.\n");
	VoDev = SAMPLE_VO_DEV_DHD0;
	VoLayer = SAMPLE_VO_LAYER_VHD0;
	u32WndNum = 16;
	enVoMode = VO_MODE_16MUX;
	
	stVoPubAttr.enIntfSync = VO_OUTPUT_1080P60;
	stVoPubAttr.enIntfType = VO_INTF_HDMI|VO_INTF_VGA;
	stVoPubAttr.u32BgColor = 0x000000ff;
	s32Ret = SAMPLE_COMM_VO_StartDev(VoDev, &stVoPubAttr);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_StartDev failed!\n");
		goto END_8_960H_3;
	}

	memset(&(stLayerAttr), 0 , sizeof(VO_VIDEO_LAYER_ATTR_S));
	s32Ret = SAMPLE_COMM_VO_GetWH(stVoPubAttr.enIntfSync, \
		&stLayerAttr.stImageSize.u32Width, \
		&stLayerAttr.stImageSize.u32Height, \
		&stLayerAttr.u32DispFrmRt);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_GetWH failed!\n");
		goto END_8_960H_3;
	}
	
	stLayerAttr.enPixFormat = SAMPLE_PIXEL_FORMAT;
	stLayerAttr.stDispRect.s32X 	  = 0;
	stLayerAttr.stDispRect.s32Y 	  = 0;
	stLayerAttr.stDispRect.u32Width   = stLayerAttr.stImageSize.u32Width;
	stLayerAttr.stDispRect.u32Height  = stLayerAttr.stImageSize.u32Height;
	stLayerAttr.bDoubleFrame = HI_TRUE;
	s32Ret = SAMPLE_COMM_VO_StartLayer(VoLayer, &stLayerAttr);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_StartLayer failed!\n");
		goto END_8_960H_3;
	}
	
	s32Ret = SAMPLE_COMM_VO_StartChn(VoLayer, enVoMode);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_StartChn failed!\n");
		goto END_8_960H_4;
	}

#ifdef HDMI_SUPPORT
	/* if it's displayed on HDMI, we should start HDMI */
	if (stVoPubAttr.enIntfType & VO_INTF_HDMI)
	{
		if (HI_SUCCESS != SAMPLE_COMM_VO_HdmiStart(stVoPubAttr.enIntfSync))
		{
			SAMPLE_PRT("Start SAMPLE_COMM_VO_HdmiStart failed!\n");
			goto END_8_960H_4;
		}
	}
#endif

	for(i=0;i<u32WndNum;i++)
	{
		VoChn = i;
		VpssGrp = i;
		
		s32Ret = SAMPLE_COMM_VO_BindVpss(VoDev,VoChn,VpssGrp,VpssChn_VoHD0);
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("Start VO failed!\n");
			goto END_8_960H_4;
		}
	}

	/******************************************
	step 6: start vo SD0 (CVBS)
	******************************************/
	printf("start vo SD0\n");
	VoDev = SAMPLE_VO_DEV_DSD0;
	VoLayer = SAMPLE_VO_LAYER_VSD0;
	u32WndNum = 16;
	enVoMode = VO_MODE_16MUX;
	
	stVoPubAttr.enIntfSync = VO_OUTPUT_NTSC;
	stVoPubAttr.enIntfType = VO_INTF_CVBS;
	stVoPubAttr.u32BgColor = 0x000000ff;
	s32Ret = SAMPLE_COMM_VO_StartDev(VoDev, &stVoPubAttr);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_StartDev failed!\n");
		goto END_8_960H_4;
	}

	memset(&(stLayerAttr), 0 , sizeof(VO_VIDEO_LAYER_ATTR_S));
	s32Ret = SAMPLE_COMM_VO_GetWH(stVoPubAttr.enIntfSync, \
		&stLayerAttr.stImageSize.u32Width, \
		&stLayerAttr.stImageSize.u32Height, \
		&stLayerAttr.u32DispFrmRt);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_GetWH failed!\n");
		goto END_8_960H_4;
	}

	stLayerAttr.enPixFormat = SAMPLE_PIXEL_FORMAT;
	stLayerAttr.stDispRect.s32X 	  = 0;
	stLayerAttr.stDispRect.s32Y 	  = 0;
	stLayerAttr.stDispRect.u32Width   = stLayerAttr.stImageSize.u32Width;
	stLayerAttr.stDispRect.u32Height  = stLayerAttr.stImageSize.u32Height;
	s32Ret = SAMPLE_COMM_VO_StartLayer(VoLayer, &stLayerAttr);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_StartLayer failed!\n");
		goto END_8_960H_4;
	}
	
	s32Ret = SAMPLE_COMM_VO_StartChn(VoLayer, enVoMode);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_StartChn failed!\n");
		goto END_8_960H_5;
	}

	for(i=0;i<u32WndNum;i++)
	{
		VoChn = i;
		ViChn = i;
		
		s32Ret = SAMPLE_COMM_VO_BindVi(VoLayer,VoChn,ViChn);
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("Start VO failed!\n");
			goto END_8_960H_5;
		}
	}
	
	/******************************************
	step 7: HD0 switch mode 
	******************************************/
	VoDev = SAMPLE_VO_DEV_DHD0;
	VoLayer = SAMPLE_VO_LAYER_VHD0;
	enVoMode = VO_MODE_16MUX;
	while(1)
	{
		enPreVoMode = enVoMode;
	
		printf("please choose preview mode, press 'q' to exit this sample.\n"); 
		printf("\t0) 1 preview\n");
		printf("\t1) 4 preview\n");
		printf("\t2) 8 preview\n");
        printf("\t3) 16 preview\n");
		printf("\tq) quit\n");
 
		ch = getchar();
        if(10 == ch)
        {
            continue;
        }
		getchar();
		if ('0' == ch)
		{
			u32WndNum = 1;
			enVoMode = VO_MODE_1MUX;
		}
		else if ('1' == ch)
		{
			u32WndNum = 4;
			enVoMode = VO_MODE_4MUX;
		}
		/*Indeed only 8 chns show*/
		else if ('2' == ch)
		{
			u32WndNum = 9;
			enVoMode = VO_MODE_9MUX;
		}
		else if ('3' == ch)
		{
			u32WndNum = 16;
			enVoMode = VO_MODE_16MUX;
		}
		else if ('q' == ch)
		{
			break;
		}
		else
		{
			SAMPLE_PRT("preview mode invaild! please try again.\n");
			continue;
		}
		SAMPLE_PRT("vo(%d) switch to %d mode\n", VoDev, u32WndNum);

		s32Ret= HI_MPI_VO_SetAttrBegin(VoLayer);
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("Start VO failed!\n");
			goto END_8_960H_5;
		}
		
		s32Ret = SAMPLE_COMM_VO_StopChn(VoLayer, enPreVoMode);
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("Start VO failed!\n");
			goto END_8_960H_5;
		}

		s32Ret = SAMPLE_COMM_VO_StartChn(VoLayer, enVoMode);
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("Start VO failed!\n");
			goto END_8_960H_5;
		}
		s32Ret= HI_MPI_VO_SetAttrEnd(VoLayer);
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("Start VO failed!\n");
			goto END_8_960H_5;
		}
	}

	/******************************************
	 step 8: exit process
	******************************************/
END_8_960H_5:
	VoDev = SAMPLE_VO_DEV_DSD0;
	VoLayer = SAMPLE_VO_LAYER_VSD0;
	u32WndNum = 16;
	enVoMode = VO_MODE_16MUX;
	SAMPLE_COMM_VO_StopChn(VoLayer, enVoMode);
	for(i=0;i<u32WndNum;i++)
	{
		VoChn = i;
		VpssGrp = i;
		SAMPLE_COMM_VO_UnBindVpss(VoLayer,VoChn,VpssGrp,VpssChn_VoSD0);
	}
	SAMPLE_COMM_VO_StopLayer(VoLayer);
	SAMPLE_COMM_VO_StopDev(VoDev);
END_8_960H_4:
	#ifdef HDMI_SUPPORT
	if (stVoPubAttr.enIntfType & VO_INTF_HDMI)
	{
		SAMPLE_COMM_VO_HdmiStop();
	}
	#endif
	VoDev = SAMPLE_VO_DEV_DHD0;
	VoLayer = SAMPLE_VO_LAYER_VHD0;
	u32WndNum = 16;
	enVoMode = VO_MODE_16MUX;	
	SAMPLE_COMM_VO_StopChn(VoDev, enVoMode);
	for(i=0;i<u32WndNum;i++)
	{
		VoChn = i;
		VpssGrp = i;
		SAMPLE_COMM_VO_UnBindVpss(VoDev,VoChn,VpssGrp,VpssChn_VoHD0);
	}
	SAMPLE_COMM_VO_StopLayer(VoLayer);
	SAMPLE_COMM_VO_StopDev(VoDev);
END_8_960H_3:	//vi unbind vpss
	SAMPLE_COMM_VI_UnBindVpss(enViMode);
END_8_960H_2:	//vpss stop
	SAMPLE_COMM_VPSS_Stop(s32VpssGrpCnt, VPSS_MAX_CHN_NUM);
END_8_960H_1:	//vi stop
	SAMPLE_COMM_VI_Stop(enViMode);
END_8_960H_0: //system exit
	SAMPLE_COMM_SYS_Exit();
	
	return s32Ret;
}

/******************************************************************************
* function : VI(D1: 8 windows) -> VPSS -> HD0(1080P@60) -> WBC -> SD0(D1)
******************************************************************************/
HI_S32 SAMPLE_VIO_8_1080P_DUAL(HI_VOID)
{
    SAMPLE_VI_MODE_E enViMode = SAMPLE_VI_MODE_8_1080P;
	VIDEO_NORM_E enNorm = VIDEO_ENCODING_MODE_NTSC;

    HI_U32 u32ViChnCnt = 8;
    HI_S32 s32VpssGrpCnt = 8;
    
    VB_CONF_S stVbConf;
    VPSS_GRP VpssGrp;
    VPSS_GRP_ATTR_S stGrpAttr;
    VPSS_CHN VpssChn_VoHD0 = VPSS_CHN2;
    VO_DEV VoDev;
	VO_LAYER VoLayer;
    VO_CHN VoChn;
    VO_PUB_ATTR_S stVoPubAttr;
	VO_VIDEO_LAYER_ATTR_S stLayerAttr;
    SAMPLE_VO_MODE_E enVoMode;
    
    HI_S32 i;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32BlkSize;
    HI_CHAR ch;
    SIZE_S stSize;
    HI_U32 u32WndNum;
	
	VO_WBC VoWbc;
    VO_WBC_ATTR_S stWbcAttr;    
    VO_WBC_SOURCE_S stWbcSource;   

    /******************************************
     step  1: init variable 
    ******************************************/    
    memset(&stVbConf,0,sizeof(VB_CONF_S));
    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(enNorm,\
                PIC_HD1080, SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH,COMPRESS_MODE_SEG);
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
        goto END_8X1080P_0;
    }

    /******************************************
     step 3: start vi dev & chn
    ******************************************/
    s32Ret = SAMPLE_COMM_VI_Start(enViMode, enNorm);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("start vi failed!\n");
        goto END_8X1080P_0;
    }

    /******************************************
     step 4: start vpss and vi bind vpss
    ******************************************/
    s32Ret = SAMPLE_COMM_SYS_GetPicSize(enNorm, PIC_HD1080, &stSize);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
        goto END_8X1080P_0;
    }

	memset(&stGrpAttr,0,sizeof(VPSS_GRP_ATTR_S));
	stGrpAttr.u32MaxW = stSize.u32Width;
	stGrpAttr.u32MaxH = stSize.u32Height;
	stGrpAttr.bNrEn = HI_TRUE;
    stGrpAttr.enDieMode = VPSS_DIE_MODE_NODIE;
	stGrpAttr.enPixFmt = SAMPLE_PIXEL_FORMAT;
    s32Ret = SAMPLE_COMM_VPSS_Start(s32VpssGrpCnt, &stSize, VPSS_MAX_CHN_NUM,NULL);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Vpss failed!\n");
        goto END_8X1080P_1;
    }

    s32Ret = SAMPLE_COMM_VI_BindVpss(enViMode);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Vi bind Vpss failed!\n");
        goto END_8X1080P_2;
    }

    /******************************************
     step 5: start vo HD0 (HDMI+VGA), multi-screen, you can switch mode
    ******************************************/
    printf("start vo HD0.\n");
    VoDev = SAMPLE_VO_DEV_DHD0;
	VoLayer = SAMPLE_VO_LAYER_VHD0;
    u32WndNum = 8;
    enVoMode = VO_MODE_9MUX;
    
    stVoPubAttr.enIntfSync = VO_OUTPUT_1080P60;
	stVoPubAttr.enIntfType = VO_INTF_HDMI|VO_INTF_VGA;
    stVoPubAttr.u32BgColor = 0x000000ff;
	s32Ret = SAMPLE_COMM_VO_StartDev(VoDev, &stVoPubAttr);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_StartDev failed!\n");
		goto END_8X1080P_3;
	}

	memset(&(stLayerAttr), 0 , sizeof(VO_VIDEO_LAYER_ATTR_S));
	s32Ret = SAMPLE_COMM_VO_GetWH(stVoPubAttr.enIntfSync, \
		&stLayerAttr.stImageSize.u32Width, \
		&stLayerAttr.stImageSize.u32Height, \
		&stLayerAttr.u32DispFrmRt);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_GetWH failed!\n");
		goto END_8X1080P_3;
	}
	
	stLayerAttr.enPixFormat = SAMPLE_PIXEL_FORMAT;
	stLayerAttr.stDispRect.s32X 	  = 0;
	stLayerAttr.stDispRect.s32Y 	  = 0;
	stLayerAttr.stDispRect.u32Width   = stLayerAttr.stImageSize.u32Width;
	stLayerAttr.stDispRect.u32Height  = stLayerAttr.stImageSize.u32Height;
	s32Ret = SAMPLE_COMM_VO_StartLayer(VoLayer, &stLayerAttr);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_StartLayer failed!\n");
		goto END_8X1080P_3;
	}

    s32Ret = SAMPLE_COMM_VO_StartChn(VoLayer, enVoMode);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start SAMPLE_COMM_VO_StartChn failed!\n");
        goto END_8X1080P_4;
    }

#ifdef HDMI_SUPPORT
    /* if it's displayed on HDMI, we should start HDMI */
    if (stVoPubAttr.enIntfType & VO_INTF_HDMI)
    {
        if (HI_SUCCESS != SAMPLE_COMM_VO_HdmiStart(stVoPubAttr.enIntfSync))
        {
            SAMPLE_PRT("Start SAMPLE_COMM_VO_HdmiStart failed!\n");
            goto END_8X1080P_4;
        }
    }
#endif

    for(i=0;i<u32WndNum;i++)
    {
        VoChn = i;
        VpssGrp = i;
        
        s32Ret = SAMPLE_COMM_VO_BindVpss(VoDev,VoChn,VpssGrp,VpssChn_VoHD0);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Start VO failed!\n");
            goto END_8X1080P_4;
        }
    }

	/******************************************
	step 7: start vo SD0 (CVBS) (WBC target) 
	******************************************/
	printf("start vo SD0: wbc from hd0\n");
	VoDev = SAMPLE_VO_DEV_DSD0;
	VoLayer = SAMPLE_VO_LAYER_VSD0;
	u32WndNum = 1;
	enVoMode = VO_MODE_1MUX;
	
	stVoPubAttr.enIntfSync = VO_OUTPUT_NTSC;
	stVoPubAttr.enIntfType = VO_INTF_CVBS;
	stVoPubAttr.u32BgColor = 0x000000ff;
	s32Ret = SAMPLE_COMM_VO_StartDev(VoDev, &stVoPubAttr);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_StartDev failed!\n");
		goto END_8X1080P_4;
	}

	memset(&(stLayerAttr), 0 , sizeof(VO_VIDEO_LAYER_ATTR_S));
	s32Ret = SAMPLE_COMM_VO_GetWH(stVoPubAttr.enIntfSync, \
		&stLayerAttr.stImageSize.u32Width, \
		&stLayerAttr.stImageSize.u32Height, \
		&stLayerAttr.u32DispFrmRt);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_GetWH failed!\n");
		goto END_8X1080P_4;
	}

	stLayerAttr.enPixFormat = SAMPLE_PIXEL_FORMAT;
	stLayerAttr.stDispRect.s32X 	  = 0;
	stLayerAttr.stDispRect.s32Y 	  = 0;
	stLayerAttr.stDispRect.u32Width   = stLayerAttr.stImageSize.u32Width;
	stLayerAttr.stDispRect.u32Height  = stLayerAttr.stImageSize.u32Height;
	s32Ret = SAMPLE_COMM_VO_StartLayer(VoLayer, &stLayerAttr);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_StartLayer failed!\n");
		goto END_8X1080P_4;
	}
	
	s32Ret = SAMPLE_COMM_VO_StartChn(VoLayer, enVoMode);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_StartChn failed!\n");
		goto END_8X1080P_5;
	}

	/*WBC*/
	VoWbc = SAMPLE_VO_WBC_BASE;
    /******Wbc bind source*******/
    stWbcSource.enSourceType = VO_WBC_SOURCE_DEV;
    stWbcSource.u32SourceId = SAMPLE_VO_DEV_DHD0;
    
    s32Ret = SAMPLE_COMM_WBC_BindVo(VoWbc,&stWbcSource);    
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        goto  END_8X1080P_5;
    }    
    /*******start Wbc*********/
    s32Ret = SAMPLE_COMM_VO_GetWH(VO_OUTPUT_PAL, \
		&stWbcAttr.stTargetSize.u32Width, \
		&stWbcAttr.stTargetSize.u32Height, \
		&stWbcAttr.u32FrameRate);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        goto  END_8X1080P_5;
    }
	stWbcAttr.enPixelFormat = SAMPLE_PIXEL_FORMAT;
    s32Ret = SAMPLE_COMM_VO_StartWbc(VoWbc,&stWbcAttr);    
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        goto  END_8X1080P_6;
    }
    /*****bind wbc to target***/       
    s32Ret = SAMPLE_COMM_VO_BindVoWbc(VoWbc,SAMPLE_VO_LAYER_VSD0,0);    
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        goto  END_8X1080P_6;
    }
	
	/******************************************
	step 8: HD0 switch mode 
	******************************************/
	VoDev = SAMPLE_VO_DEV_DHD0;
	VoLayer = SAMPLE_VO_LAYER_VHD0;
	enVoMode = VO_MODE_9MUX;
    while(1)
    {    
        printf("press 'q' to exit this sample.\n"); 
 
        ch = getchar();
        if(10 == ch)
        {
            continue;
        }
        getchar();
        if ('q' == ch)
        {
            break;
        }
        else
        {
            SAMPLE_PRT("input invaild! please try again.\n");
            continue;
        }	
    }

    /******************************************
     step 8: exit process
    ******************************************/

END_8X1080P_6:
	SAMPLE_COMM_VO_UnBindVoWbc(SAMPLE_VO_DEV_DSD0, 0);
	HI_MPI_VO_DisableWbc(SAMPLE_VO_DEV_DHD0);

END_8X1080P_5:
	VoDev = SAMPLE_VO_DEV_DSD0;
	VoLayer = SAMPLE_VO_LAYER_VSD0;
	VoChn = 0;
	enVoMode = VO_MODE_9MUX;
    SAMPLE_COMM_VO_StopChn(VoLayer, enVoMode);
	SAMPLE_COMM_VO_StopLayer(VoLayer);
    SAMPLE_COMM_VO_StopDev(VoDev);

END_8X1080P_4:
#ifdef HDMI_SUPPORT
	if (stVoPubAttr.enIntfType & VO_INTF_HDMI)
    {
        SAMPLE_COMM_VO_HdmiStop();
    }
#endif
    VoDev = SAMPLE_VO_DEV_DHD0;
	VoLayer = SAMPLE_VO_LAYER_VHD0;
    u32WndNum = 8;
    enVoMode = VO_MODE_9MUX;   
    SAMPLE_COMM_VO_StopChn(VoDev, enVoMode);
    for(i=0;i<u32WndNum;i++)
    {
        VoChn = i;
        VpssGrp = i;
        SAMPLE_COMM_VO_UnBindVpss(VoDev,VoChn,VpssGrp,VpssChn_VoHD0);
    }
	SAMPLE_COMM_VO_StopLayer(VoLayer);
    SAMPLE_COMM_VO_StopDev(VoDev);

END_8X1080P_3:
    SAMPLE_COMM_VI_UnBindVpss(enViMode);
END_8X1080P_2:
    SAMPLE_COMM_VPSS_Stop(s32VpssGrpCnt, VPSS_MAX_CHN_NUM);
END_8X1080P_1:
    SAMPLE_COMM_VI_Stop(enViMode);
END_8X1080P_0:
    SAMPLE_COMM_SYS_Exit();
    
    return s32Ret;
}

/******************************************************************************
* function : VI(720P) -> VPSS grp 0 -> VO HD0(1080p)
*            VI(720P) -> VPSS grp 1 -> VO PIP
******************************************************************************/
HI_S32 SAMPLE_VIO_HD_ZoomIn()
{
    SAMPLE_VI_MODE_E enViMode = SAMPLE_VI_MODE_8_720P;
	VIDEO_NORM_E enNorm = VIDEO_ENCODING_MODE_NTSC;

    HI_U32 u32ViChnCnt = 8;
    HI_S32 s32VpssGrpCnt = 8;
    VPSS_GRP VpssGrp = 0;
    VPSS_GRP VpssGrp_Clip = 1;
	VPSS_CHN VpssChn_VoHD0 = VPSS_CHN2;
	VPSS_CHN VpssChn_VoPIP = VPSS_CHN2;
    VO_DEV VoDev = SAMPLE_VO_DEV_DHD0;
	VO_LAYER VoLayer = SAMPLE_VO_LAYER_VHD0;
	VO_LAYER VoLayerPip = SAMPLE_VO_LAYER_VPIP;
    VO_CHN VoChn = 0;
    
    VB_CONF_S stVbConf;
    VPSS_GRP_ATTR_S stGrpAttr;
    VO_PUB_ATTR_S stVoPubAttr; 
    SAMPLE_VO_MODE_E enVoMode;
    VPSS_CROP_INFO_S stVpssCropInfo;
	VO_VIDEO_LAYER_ATTR_S stLayerAttr;
    SIZE_S stSize;
    
    HI_S32 i;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32BlkSize;
    HI_CHAR ch;
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
        goto END_HDZOOMIN_0;
    }

    /******************************************
     step 3: start vi dev & chn
    ******************************************/
    s32Ret = SAMPLE_COMM_VI_Start(enViMode, enNorm);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("start vi failed!\n");
        goto END_HDZOOMIN_0;
    }

    /******************************************
     step 4: start vpss and vi bind vpss
    ******************************************/
    s32Ret = SAMPLE_COMM_SYS_GetPicSize(enNorm, PIC_HD720, &stSize);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
        goto END_HDZOOMIN_0;
    }

	memset(&stGrpAttr,0,sizeof(VPSS_GRP_ATTR_S));
	stGrpAttr.u32MaxW = stSize.u32Width;
	stGrpAttr.u32MaxH = stSize.u32Height;
	stGrpAttr.bNrEn = HI_TRUE;
    stGrpAttr.enDieMode = VPSS_DIE_MODE_NODIE;
	stGrpAttr.enPixFmt = SAMPLE_PIXEL_FORMAT;
    s32Ret = SAMPLE_COMM_VPSS_Start(s32VpssGrpCnt, &stSize, VPSS_MAX_CHN_NUM,NULL);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Vpss failed!\n");
        goto END_HDZOOMIN_1;
    }

    s32Ret = SAMPLE_COMM_VI_BindVpss(enViMode);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Vi bind Vpss failed!\n");
        goto END_HDZOOMIN_2;
    }

    /******************************************
     step 5: start VO to preview
    ******************************************/
	printf("start vo HD0.\n");
	VoDev = SAMPLE_VO_DEV_DHD0;
	VoLayer = SAMPLE_VO_LAYER_VHD0;
	u32WndNum = 1;
	enVoMode = VO_MODE_1MUX;
	
	stVoPubAttr.enIntfSync = VO_OUTPUT_1080P60;
	stVoPubAttr.enIntfType = VO_INTF_HDMI|VO_INTF_VGA;
	stVoPubAttr.u32BgColor = 0x000000ff;
	s32Ret = SAMPLE_COMM_VO_StartDev(VoDev, &stVoPubAttr);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_StartDev failed!\n");
		goto END_HDZOOMIN_3;
	}

	memset(&(stLayerAttr), 0 , sizeof(VO_VIDEO_LAYER_ATTR_S));
	s32Ret = SAMPLE_COMM_VO_GetWH(stVoPubAttr.enIntfSync, \
		&stLayerAttr.stImageSize.u32Width, \
		&stLayerAttr.stImageSize.u32Height, \
		&stLayerAttr.u32DispFrmRt);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_GetWH failed!\n");
		goto END_HDZOOMIN_3;
	}
	stLayerAttr.enPixFormat = SAMPLE_PIXEL_FORMAT;
	stLayerAttr.stDispRect.s32X 	  = 0;
	stLayerAttr.stDispRect.s32Y 	  = 0;
	stLayerAttr.stDispRect.u32Width   = stLayerAttr.stImageSize.u32Width;
	stLayerAttr.stDispRect.u32Height  = stLayerAttr.stImageSize.u32Height;
	s32Ret = SAMPLE_COMM_VO_StartLayer(VoLayer, &stLayerAttr);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_StartLayer failed!\n");
		goto END_HDZOOMIN_3;
	}

	s32Ret = SAMPLE_COMM_VO_StartChn(VoLayer, enVoMode);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_StartChn failed!\n");
		goto END_HDZOOMIN_4;
	}

#ifdef HDMI_SUPPORT
	/* if it's displayed on HDMI, we should start HDMI */
	if (stVoPubAttr.enIntfType & VO_INTF_HDMI)
	{
        HDMI_CALLBACK_ARGS_S stHdmiCallbackArgs;
		if (HI_SUCCESS != SAMPLE_COMM_VO_HdmiCallbackStart(stVoPubAttr.enIntfSync, &stHdmiCallbackArgs))
		{
			SAMPLE_PRT("Start SAMPLE_COMM_VO_HdmiStart failed!\n");
			goto END_HDZOOMIN_5;
		}
	}
#endif

	VoChn = 0;
	VpssGrp = 0;
	s32Ret = SAMPLE_COMM_VO_BindVpss(VoDev,VoChn,VpssGrp,VpssChn_VoHD0);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start VO failed!\n");
		goto END_HDZOOMIN_5;
	}

    /******************************************
     step 6: Clip process
    ******************************************/
    printf("press any key to show hd zoom.\n");
    getchar();
    /*** enable vo pip layer ***/
    stLayerAttr.bClusterMode = HI_TRUE;
    stLayerAttr.bDoubleFrame = HI_FALSE;
    stLayerAttr.enPixFormat = SAMPLE_PIXEL_FORMAT;
	s32Ret = HI_MPI_VO_BindVideoLayer(VoLayerPip, VoDev);
	if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_VO_BindVideoLayer failed with %#x!\n", s32Ret);
        goto END_HDZOOMIN_5;
    }

	memset(&stLayerAttr, 0 , sizeof(VO_VIDEO_LAYER_ATTR_S));
    s32Ret = SAMPLE_COMM_VO_GetWH(VO_OUTPUT_PAL, \
        &stLayerAttr.stDispRect.u32Width, \
        &stLayerAttr.stDispRect.u32Height, \
        &stLayerAttr.u32DispFrmRt);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        goto  END_HDZOOMIN_6;
    }
    stLayerAttr.stImageSize.u32Width = stLayerAttr.stDispRect.u32Width;
    stLayerAttr.stImageSize.u32Height = stLayerAttr.stDispRect.u32Height;
	stLayerAttr.enPixFormat = SAMPLE_PIXEL_FORMAT;
    s32Ret = SAMPLE_COMM_VO_StartLayer(VoLayerPip, &stLayerAttr);
    if (HI_SUCCESS != s32Ret)
    {
       SAMPLE_PRT("SAMPLE_COMM_VO_StartLayer failed!\n");
       goto END_HDZOOMIN_6;
    }

	enVoMode = VO_MODE_1MUX;
    s32Ret = SAMPLE_COMM_VO_StartChn(VoLayerPip,enVoMode);    
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        goto  END_HDZOOMIN_7;
    }

	s32Ret = SAMPLE_COMM_VO_BindVpss(VoLayerPip, VoChn, VpssGrp_Clip, VpssChn_VoPIP);//VpssGrp_Clip
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VO_BindVpss failed!\n");
        goto END_HDZOOMIN_8;
    }

	/*** enable vpss group clip ***/
    stVpssCropInfo.bEnable = HI_TRUE;
    stVpssCropInfo.enCropCoordinate = VPSS_CROP_ABS_COOR;
    stVpssCropInfo.stCropRect.s32X = 0;
    stVpssCropInfo.stCropRect.s32Y = 0;
    stVpssCropInfo.stCropRect.u32Height = 400;
    stVpssCropInfo.stCropRect.u32Width = 400;
    s32Ret = HI_MPI_VPSS_SetGrpCrop(VpssGrp_Clip, &stVpssCropInfo);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_VPSS_SetGrpCrop failed with %#x!\n", s32Ret);
        goto END_HDZOOMIN_8;
    }
	
    printf("\npress 'q' to stop sample ... ... \n");
    while('q' != (ch = getchar()) )  {}
    
    /******************************************
     step 7: exit process
    ******************************************/	
END_HDZOOMIN_8:
	enVoMode = VO_MODE_1MUX;
	SAMPLE_COMM_VO_StopChn(VoLayerPip,enVoMode);
    SAMPLE_COMM_VO_UnBindVpss(SAMPLE_VO_LAYER_VHD0, VoChn, VpssGrp_Clip, VpssChn_VoPIP);
END_HDZOOMIN_7:
	SAMPLE_COMM_VO_StopLayer(VoLayerPip);	
END_HDZOOMIN_6:
	HI_MPI_VO_UnBindVideoLayer(SAMPLE_VO_DEV_DHD0,SAMPLE_VO_DEV_DHD0);
END_HDZOOMIN_5:
	VoDev = SAMPLE_VO_DEV_DHD0;
	VoLayer = SAMPLE_VO_LAYER_VHD0;
    u32WndNum = 1;
    enVoMode = VO_MODE_1MUX;
    SAMPLE_COMM_VO_StopChn(VoDev, enVoMode);
	for(i=0;i<u32WndNum;i++)
    {
        VoChn = i;
        VpssGrp = i;
        SAMPLE_COMM_VO_UnBindVpss(VoDev,VoChn,VpssGrp,VpssChn_VoHD0);
    }
END_HDZOOMIN_4:
	SAMPLE_COMM_VO_StopLayer(SAMPLE_VO_LAYER_VHD0);
	#ifdef HDMI_SUPPORT
    if (stVoPubAttr.enIntfType & VO_INTF_HDMI)
    {
        SAMPLE_COMM_VO_HdmiStop();
    }
	#endif
	SAMPLE_COMM_VO_StopDev(VoDev);
END_HDZOOMIN_3:    
    SAMPLE_COMM_VI_UnBindVpss(enViMode);
END_HDZOOMIN_2:
    SAMPLE_COMM_VPSS_Stop(s32VpssGrpCnt, VPSS_MAX_CHN_NUM);
END_HDZOOMIN_1:
    SAMPLE_COMM_VI_Stop(enViMode);
END_HDZOOMIN_0:
    SAMPLE_COMM_SYS_Exit();
    
    return s32Ret;
}

/******************************************************************************
* function : VI(720P) -> VO SD0(D1)
*                     -> VO SD0 ZoomIn
******************************************************************************/
HI_U32 SAMPLE_VIO_SD_ZoomIn()
{
	SAMPLE_VI_MODE_E enViMode = SAMPLE_VI_MODE_8_720P;
	VIDEO_NORM_E enNorm = VIDEO_ENCODING_MODE_NTSC;

    HI_U32 u32ViChnCnt = 8;
	
	VB_CONF_S stVbConf;    
	HI_U32 u32BlkSize;
	SIZE_S stSize;	  
	VO_DEV VoDev = SAMPLE_VO_DEV_DSD0;	 
	VO_LAYER VoLayer = SAMPLE_VO_LAYER_VSD0;
	VO_CHN VoChn_Clip = 0;	  
	VO_CHN VoChn_Full = 1;
	VO_PUB_ATTR_S stVoPubAttr; 
	VO_VIDEO_LAYER_ATTR_S stLayerAttr;	   
	VO_CHN_ATTR_S stChnAttr;	
	VO_ZOOM_ATTR_S stZoomAttr;	  
	HI_CHAR ch;
	HI_S32 s32Ret = HI_SUCCESS;
	MPP_CHN_S stSrcChn, stDestChn;
	
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
        goto END_SDZOOMIN_0;
    }

    /******************************************
     step 3: start vi dev & chn
    ******************************************/
    s32Ret = SAMPLE_COMM_VI_Start(enViMode, enNorm);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("start vi failed!\n");
        goto END_SDZOOMIN_0;
    }  

	/******************************************
	 step 4: start DSD0 
	******************************************/
	VoDev = SAMPLE_VO_DEV_DSD0;    
	stVoPubAttr.enIntfSync = VO_OUTPUT_PAL;
	stVoPubAttr.enIntfType = VO_INTF_CVBS;
	stVoPubAttr.u32BgColor = 0x000000ff;
	s32Ret = SAMPLE_COMM_VO_StartDev(VoDev, &stVoPubAttr);	  
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("SAMPLE_COMM_VO_StartDev failed!\n");
		goto END_SDZOOMIN_1;
	}
	/***start Layer VSD0 ***/
	VoLayer = SAMPLE_VO_LAYER_VSD0;
	stLayerAttr.bClusterMode = HI_FALSE;
	stLayerAttr.bDoubleFrame = HI_FALSE;
	stLayerAttr.enPixFormat = SAMPLE_PIXEL_FORMAT;	  
	s32Ret = SAMPLE_COMM_VO_GetWH(stVoPubAttr.enIntfSync, \
		&stLayerAttr.stDispRect.u32Width, &stLayerAttr.stDispRect.u32Height, &stLayerAttr.u32DispFrmRt);
	if (s32Ret != HI_SUCCESS)
	{
		SAMPLE_PRT("failed with %#x!\n", s32Ret);
		goto  END_SDZOOMIN_2;
	}
	stLayerAttr.stImageSize.u32Width = stLayerAttr.stDispRect.u32Width;
	stLayerAttr.stImageSize.u32Height = stLayerAttr.stDispRect.u32Height;

	s32Ret = SAMPLE_COMM_VO_StartLayer(VoLayer, &stLayerAttr);	  
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("SAMPLE_COMM_VO_StartLayer failed!\n");
		goto END_SDZOOMIN_2;
	}	 
	/***start 1 Chn in  VSD0 Layer for base show***/
	stChnAttr.bDeflicker = HI_FALSE;
	stChnAttr.u32Priority = 0;	  
	stChnAttr.stRect.s32X = 0;
	stChnAttr.stRect.s32Y = 0;
	stChnAttr.stRect.u32Width = stLayerAttr.stDispRect.u32Width;
	stChnAttr.stRect.u32Height = stLayerAttr.stDispRect.u32Height;
	
	s32Ret = HI_MPI_VO_SetChnAttr(VoLayer, VoChn_Clip, &stChnAttr);
	if (s32Ret != HI_SUCCESS)
	{
		printf("%s(%d):failed with %#x!\n",__FUNCTION__,__LINE__,  s32Ret);
		goto  END_SDZOOMIN_3;
	}
	
	s32Ret = HI_MPI_VO_EnableChn(VoLayer, VoChn_Clip);
	if (s32Ret != HI_SUCCESS)
	{
		SAMPLE_PRT("failed with %#x!\n", s32Ret);
		goto  END_SDZOOMIN_3;
	}	 
	/**************vo bind to Vi 1 chn*************/
    stSrcChn.enModId    = HI_ID_VIU;
    stSrcChn.s32DevId   = 0;
    stSrcChn.s32ChnId   = 0;
    
    stDestChn.enModId   = HI_ID_VOU;
    stDestChn.s32ChnId  = VoChn_Clip;
    stDestChn.s32DevId  = VoLayer;
    
    s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
	if (HI_SUCCESS != s32Ret)
	{
	   SAMPLE_PRT("HI_MPI_SYS_Bind failed!\n");
	   goto END_SDZOOMIN_4;
	}	  
	/******************************************
	 step 5: Clip process
	******************************************/    
	printf("press any key to show sd zoom.\n");
	getchar();	  
	/**************start 1 Chn in  VSD0 Layer for Full show************/
	stChnAttr.bDeflicker = HI_FALSE;
	stChnAttr.u32Priority = 1; 
	
	stChnAttr.stRect.s32X = 360;
	stChnAttr.stRect.s32Y = 288;	
	s32Ret = SAMPLE_COMM_SYS_GetPicSize(VIDEO_ENCODING_MODE_PAL, PIC_CIF, &stSize);
	if (s32Ret != HI_SUCCESS)
	{
		SAMPLE_PRT("failed with %#x!\n", s32Ret);
		goto  END_SDZOOMIN_4;
	}
	stChnAttr.stRect.u32Width = stSize.u32Width;
	stChnAttr.stRect.u32Height = stSize.u32Height;
	
	s32Ret = HI_MPI_VO_SetChnAttr(VoLayer, VoChn_Full, &stChnAttr);
	if (s32Ret != HI_SUCCESS)
	{
		printf("%s(%d):failed with %#x!\n",__FUNCTION__,__LINE__,  s32Ret);
		goto  END_SDZOOMIN_5;
	}
	
	s32Ret = HI_MPI_VO_EnableChn(VoLayer, VoChn_Full);
	if (s32Ret != HI_SUCCESS)
	{
		SAMPLE_PRT("failed with %#x!\n", s32Ret);
		goto  END_SDZOOMIN_5;
	}	 
	/**************vo bind to VI 0 chn*************/
	stSrcChn.enModId    = HI_ID_VIU;
    stSrcChn.s32DevId   = 0;
    stSrcChn.s32ChnId   = 0;
    
    stDestChn.enModId   = HI_ID_VOU;
    stDestChn.s32ChnId  = VoChn_Full;
    stDestChn.s32DevId  = VoLayer;
    
    s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
	if (HI_SUCCESS != s32Ret)
	{
	   SAMPLE_PRT("HI_MPI_SYS_Bind failed!\n");
	   goto END_SDZOOMIN_6;
	}
	/***zoom in for VoChn_Clip chn***/
	stZoomAttr.enZoomType = VOU_ZOOM_IN_RATIO;
	stZoomAttr.stZoomRatio.u32XRatio = 500;
	stZoomAttr.stZoomRatio.u32YRatio = 500;
	stZoomAttr.stZoomRatio.u32WRatio = 500;
	stZoomAttr.stZoomRatio.u32HRatio = 500;
	
	s32Ret = HI_MPI_VO_SetZoomInWindow(VoLayer, VoChn_Clip, &stZoomAttr);
	if (HI_SUCCESS != s32Ret)
	{
	   SAMPLE_PRT("HI_MPI_VO_SetZoomInWindow failed!\n");
	   goto END_SDZOOMIN_7;
	} 
	while(1)
	{
		printf("press 'q' to exit this sample.\n"); 	   
		ch = getchar();
        if(10 == ch)
        {
            continue;
        }
		getchar();
		if ('q' == ch)
		{
			break;
		}
		else
		{
			SAMPLE_PRT("the input is invaild! please try again.\n");
			continue;
		}

	}
	/******************************************
	 step 6: exit process
	******************************************/
END_SDZOOMIN_7: 	   
	VoLayer = SAMPLE_VO_LAYER_VSD0;
	stDestChn.s32ChnId = VoChn_Full;
    HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
END_SDZOOMIN_6:
	VoLayer = SAMPLE_VO_LAYER_VSD0;
	HI_MPI_VO_DisableChn(VoLayer,VoChn_Full);
END_SDZOOMIN_5: 	   
	VoLayer = SAMPLE_VO_LAYER_VSD0;
    stDestChn.s32ChnId = VoChn_Clip;
    HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
END_SDZOOMIN_4: 	   
	VoLayer = SAMPLE_VO_LAYER_VSD0;
	HI_MPI_VO_DisableChn(VoLayer,VoChn_Clip);
END_SDZOOMIN_3: 	   
	VoLayer = SAMPLE_VO_LAYER_VSD0;
	SAMPLE_COMM_VO_StopLayer(VoLayer);
END_SDZOOMIN_2: 	   
	VoDev = SAMPLE_VO_DEV_DSD0;    
	SAMPLE_COMM_VO_StopDev(VoDev);
END_SDZOOMIN_1:
	SAMPLE_COMM_VI_Stop(enViMode);
END_SDZOOMIN_0:
	SAMPLE_COMM_SYS_Exit();

return s32Ret;
}

/******************************************************************************
* function    : main()
* Description : video preview sample
******************************************************************************/
int main(int argc, char *argv[])
{
    HI_S32 s32Ret = HI_FAILURE;

    if ( (argc < 2) || (1 != strlen(argv[1])))
    {
        SAMPLE_VIO_Usage(argv[0]);
        return HI_FAILURE;
    }
    signal(SIGINT, SAMPLE_VIO_HandleSig);
    signal(SIGTERM, SAMPLE_VIO_HandleSig);

	switch(*argv[1])
	{
		case '0':/* VI:8*720P;VPSS VO:HD0(HDMI|VGA); VPSS VO:SD0(CVBS)*/
			s32Ret = SAMPLE_VIO_8_720P();
			break;
		case '1':/* VI:8*960H;VPSS VO:HD0(HDMI|VGA); VI:SD0(CVBS)*/
			s32Ret = SAMPLE_VIO_16_960H();
			break;
		case '2':/* VI:8*D1;VPSS VO:HD0(HDMI|VGA) WBC SD0(CVBS)*/
			s32Ret = SAMPLE_VIO_8_1080P_DUAL();
			break;
		case '3':/* VI:1*D1 HD PIP ZoomIn*/
			s32Ret = SAMPLE_VIO_HD_ZoomIn();
			break;
		case '4':/* VI:1*D1 SD ZoomIn*/
			s32Ret = SAMPLE_VIO_SD_ZoomIn();
			break;
        default:
            printf("input invaild! please try again.\n");
            SAMPLE_VIO_Usage(argv[0]);
            return HI_FAILURE;
	}

    if (HI_SUCCESS == s32Ret)
        printf("program exit normally!\n");
    else
        printf("program exit abnormally!\n");
    exit(s32Ret);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

