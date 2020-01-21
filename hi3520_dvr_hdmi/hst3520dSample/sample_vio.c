#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "hstChnConfig.h"
#include "hstSdkAL.h"
#include "hstSdkDefine.h"
#include "hstSdkStruct.h"
//#include "HstSrvChnAttr.h"
#include "sample_vio.h"


#define FOR_VERSION_BT	20


#define CHANNEL_NUM 	4
#define	BIT_SET(a,b)	((a) |= (1<<(b)))
#define	BIT_CLEAN(a,b)	((a)=((a) & ((a)^(1<<(b)))))
#define	BIT_CHECK(a,b)	((a) & (1<<(b)))

unsigned int u32OutChannelstatus;


int MPP_Init(void)
{
    static char ls_bInitFlag = 0;

	static CAMERA_RESOLUTION_STATUS_E ls_arrIniteGroupStat[HST_PHY_CHN_NUM/2]={CAMERA_RESOLUTION_NULL,CAMERA_RESOLUTION_NULL};
	static CAMERA_INPUT_STATUS_S ls_arrCameraStatus[HST_PHY_CHN_NUM];
	static CAMERA_RESOLUTION_STATUS_E ls_arrGroupStat[HST_PHY_CHN_NUM];
	int i = 0;

	//Hst3520D_Sample_SelfAdaptationViAttr(ls_arrCameraStatus,ls_arrGroupStat);
    if(0 == ls_bInitFlag)
    {   
        ls_bInitFlag = 1;
		//Hst3520D_Sample_Start_GroupVi();
		//Hst3520D_Sample_DefaultCVBSVoAttr();

		//VO_ATTR_S stVoAttr = {0}; //增加和BT10一致
		//Hst3520D_Sample_GetVoAttr(&stVoAttr);
		//if(20 == FOR_VERSION_BT)
		//{
		//	stVoAttr.u32WndNum = 0x3f;
		//}	
		//else
		//{
		//	stVoAttr.u32WndNum = 0x0f;
		//}
			
		//Hst3520D_Sample_SetVoAttr(&stVoAttr);

		Hst3520D_Sample_StartHdmiVo();

		memcpy(ls_arrIniteGroupStat,ls_arrGroupStat,sizeof(ls_arrIniteGroupStat)*HST_PHY_CHN_NUM/2);

		//Hst3520D_Sample_StartHdmiVo();
	}

	
}

int Hst3520D_StartHdmiVo()
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

	printf("biao debug 001\n");
	
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

	printf("biao debug 002\n");

	/******************************************
	 step 2: mpp system init. 
	******************************************/
	s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("system init failed with %d!\n", s32Ret);
		goto END_8_720P_0;
	}

	printf("biao debug 003\n");

	/******************************************
	 step 3: start vi dev & chn
	******************************************/
	s32Ret = SAMPLE_COMM_VI_Start(enViMode, enNorm, Hst3520d_Adapt_Get_CodecType(),
        Hst3520d_Adapt_Get_CodecNum());
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

	printf("biao debug 004\n");

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

	printf("biao debug 005\n");
	/**!!!!!!!!!  1 add by licaibao*/
	s32Ret = SAMPLE_COMM_VI_BindVpss(enViMode,1, Hst3520d_Adapt_Get_CodecType());
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
	stLayerAttr.stDispRect.s32X 	  = 0;
	stLayerAttr.stDispRect.s32Y 	  = 0;
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
	printf("biao debug 1110\n");
	
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
	stLayerAttr.stDispRect.s32X 	  = 0;
	stLayerAttr.stDispRect.s32Y 	  = 0;
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

	printf("biao debug 2222 \n");

	/******************************************
	step 7: HD0 switch mode 
	******************************************/
	VoDev = SAMPLE_VO_DEV_DHD0;
	VoLayer = SAMPLE_VO_LAYER_VHD0;
	enVoMode = VO_MODE_9MUX;

	/**add by licabiao**/
	u32WndNum = 9;
	enVoMode = VO_MODE_9MUX;

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


	return 0;

	
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
	/*******1 add by licaibiao ******/
	SAMPLE_COMM_VI_UnBindVpss(enViMode,1);
END_8_720P_2:	//vpss stop
	SAMPLE_COMM_VPSS_Stop(s32VpssGrpCnt, VPSS_MAX_CHN_NUM);
END_8_720P_1:	//vi stop
	SAMPLE_COMM_VI_Stop(enViMode);
END_8_720P_0:	//system exit
	SAMPLE_COMM_SYS_Exit();
	
	return s32Ret;


}

int Hst3520D_Mpp_init(void)
{
	SAMPLE_VI_MODE_E enViMode = SAMPLE_VI_MODE_4_1080P;
	VIDEO_NORM_E enNorm = VIDEO_ENCODING_MODE_PAL;

	HI_U32 u32ViChnCnt = CHANNEL_NUM;
	HI_S32 s32VpssGrpCnt = CHANNEL_NUM;
	
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
				PIC_HD1080, SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH,COMPRESS_MODE_SEG);
	stVbConf.u32MaxPoolCnt = 256;

	/* video buffer*/
	//todo: vb=15
	stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
	stVbConf.astCommPool[0].u32BlkCnt = u32ViChnCnt * CHANNEL_NUM;


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
	s32Ret = SAMPLE_COMM_VI_Start(enViMode, enNorm, Hst3520d_Adapt_Get_CodecType(),
        Hst3520d_Adapt_Get_CodecNum());
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("start vi failed!\n");
		goto END_8_720P_0;
	}

	/******************************************
	 step 4: start vpss and vi bind vpss
	******************************************/
	s32Ret = SAMPLE_COMM_SYS_GetPicSize(enNorm, PIC_HD1080, &stSize);
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

	s32Ret = SAMPLE_COMM_VI_BindVpss(enViMode,1, Hst3520d_Adapt_Get_CodecType());
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
	u32WndNum = 4;
	enVoMode = VO_MODE_4MUX;

	stVoPubAttr.enIntfSync = VO_OUTPUT_1280x1024_60;
	stVoPubAttr.enIntfType = VO_INTF_HDMI|VO_INTF_VGA;
	//stVoPubAttr.enIntfType = VO_INTF_HDMI;
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
	stLayerAttr.stDispRect.s32X 	  = 0;
	stLayerAttr.stDispRect.s32Y 	  = 0;
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

	/* if it's displayed on HDMI, we should start HDMI */
	if (stVoPubAttr.enIntfType & VO_INTF_HDMI)
	{
		if (HI_SUCCESS != SAMPLE_COMM_VO_HdmiStart(stVoPubAttr.enIntfSync))
		{
			SAMPLE_PRT("Start SAMPLE_COMM_VO_HdmiStart failed!\n");
			goto END_8_720P_4;
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
			goto END_8_720P_4;
		}
	}

	for(i=0;i<CHANNEL_NUM;i++)
	{
		BIT_SET(u32OutChannelstatus,i);
	}


	
	return 0;
	
	printf("biao debug 1110\n");
	
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
	
	stLayerAttr.enPixFormat 		  = SAMPLE_PIXEL_FORMAT;
	stLayerAttr.stDispRect.s32X 	  = 0;
	stLayerAttr.stDispRect.s32Y 	  = 0;
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


	return 0;


	/******************************************
	step 7: HD0 switch mode 
	******************************************/
	VoDev = SAMPLE_VO_DEV_DHD0;
	VoLayer = SAMPLE_VO_LAYER_VHD0;
	enVoMode = VO_MODE_9MUX;

	/**add by licabiao**/
	u32WndNum = 4;
	enVoMode = VO_MODE_9MUX;

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
	/*******1 add by licaibiao ******/
	SAMPLE_COMM_VI_UnBindVpss(enViMode,1);
END_8_720P_2:	//vpss stop
	SAMPLE_COMM_VPSS_Stop(s32VpssGrpCnt, VPSS_MAX_CHN_NUM);
END_8_720P_1:	//vi stop
	SAMPLE_COMM_VI_Stop(enViMode);
END_8_720P_0:	//system exit
	SAMPLE_COMM_SYS_Exit();
	
	return s32Ret;


}


int ExitVoReleaseData(void)
{

}

/**一个窗口显示一个通道**/
int OneWindowsDisplayChannel(unsigned char u8ChNum)
{
	int i = 0;
	int s32Ret = 0;
	HI_U32 u32WndNum = 0;
	HI_U32 u32Square = 0;
	HI_U32 u32Width = 0;
	HI_U32 u32Height = 0;
	HI_S32 s32ChnFrmRate;
	HI_U8  u8ChannelNum = 0;
	VO_LAYER VoLayer;
	SAMPLE_VO_MODE_E enVoMode;
	SAMPLE_VO_MODE_E enPreVoMode;
	VO_CHN_ATTR_S stChnAttr;
	VO_VIDEO_LAYER_ATTR_S stLayerAttr;

	VoLayer = 0;
	u8ChannelNum = u8ChNum;


	s32Ret= HI_MPI_VO_SetAttrBegin(VoLayer);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start VO failed!\n");
		ExitVoReleaseData();
		return HI_FAILURE;
	}
#if 0	
	s32Ret = SAMPLE_COMM_VO_StopChn(VoLayer, enPreVoMode);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start VO failed!\n");
		ExitVoReleaseData();
	}
#endif	
	for(i=0;i<CHANNEL_NUM;i++)
	{
		if(BIT_CHECK(u32OutChannelstatus,i))
		{
			SAMPLE_PRT("disable channe %d \n",i);
			s32Ret = HI_MPI_VO_DisableChn(VoLayer, i);
			if (HI_SUCCESS != s32Ret)
			{
				SAMPLE_PRT("HI_MPI_VO_DisableChn %d failed!\n",i);
				ExitVoReleaseData();
				return HI_FAILURE;
			}else
			{
				BIT_CLEAN(u32OutChannelstatus,i);
			}
		}
	}

#if 0
	s32Ret = SAMPLE_COMM_VO_StartChn(VoLayer, enVoMode);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start VO failed!\n");
		ExitVoReleaseData();

	}
#endif

	s32Ret = HI_MPI_VO_GetVideoLayerAttr(VoLayer, &stLayerAttr);
	if (s32Ret != HI_SUCCESS)
	{
		SAMPLE_PRT("failed with %#x!\n", s32Ret);
		return HI_FAILURE;
	}

	u32Width = stLayerAttr.stImageSize.u32Width;
	u32Height = stLayerAttr.stImageSize.u32Height;

	SAMPLE_PRT("u32Width = %d u32Height = %d \n", u32Width,u32Height);


    if (stLayerAttr.u32DispFrmRt <= 0)
    {
        s32ChnFrmRate = 30;
    }
    else if (stLayerAttr.u32DispFrmRt > 30)
    {
        s32ChnFrmRate = stLayerAttr.u32DispFrmRt / 2;
    }
    else
    {
        s32ChnFrmRate = stLayerAttr.u32DispFrmRt;
    }

	stChnAttr.stRect.s32X = 0;
	stChnAttr.stRect.s32Y = 0;
	stChnAttr.stRect.u32Width = u32Width;
	stChnAttr.stRect.u32Height = u32Height;

	stChnAttr.u32Priority       = 0;
	stChnAttr.bDeflicker        = HI_FALSE;

	s32Ret = HI_MPI_VO_SetChnAttr(VoLayer, u8ChannelNum, &stChnAttr);
	if (s32Ret != HI_SUCCESS)
	{
		printf("%s(%d):failed with %#x!\n", __FUNCTION__, __LINE__, s32Ret);
		ExitVoReleaseData();
		return HI_FAILURE;
	}
	
	printf("[%s] [i:%d] [s32ChnFrmRate:%d] [VoLayer:%d]\n",
		__func__, i, s32ChnFrmRate, VoLayer);
	
	s32Ret = HI_MPI_VO_SetChnFrameRate(VoLayer, u8ChannelNum, s32ChnFrmRate);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("failed with %#x!\n", s32Ret);
		ExitVoReleaseData();
		return HI_FAILURE;
	}
	
	s32Ret = HI_MPI_VO_EnableChn(VoLayer, u8ChannelNum);
	if (s32Ret != HI_SUCCESS)
	{
		SAMPLE_PRT("failed with %#x! VoLayer = %d ch = %d\n", s32Ret,VoLayer,u8ChannelNum);
		ExitVoReleaseData();
		return HI_FAILURE;
	}

	BIT_SET(u32OutChannelstatus,u8ChannelNum);
	
	s32Ret= HI_MPI_VO_SetAttrEnd(VoLayer);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start VO failed!\n");
		ExitVoReleaseData();
		return HI_FAILURE;
	}

	
	return HI_SUCCESS;

};

/**两个窗口分别显示1，2通道**/
int TwoWindowsDispalyCha_1_2(void) 
{
	int i = 0;
	int s32Ret = 0;
	HI_U32 u32WndNum = 0;
	HI_U32 u32Square = 0;
	HI_U32 u32Width = 0;
	HI_U32 u32Height = 0;
	HI_S32 s32ChnFrmRate;
	HI_U8  u8ChannelNum = 0;
	VO_LAYER VoLayer;
	SAMPLE_VO_MODE_E enVoMode;
	SAMPLE_VO_MODE_E enPreVoMode;
	VO_CHN_ATTR_S stChnAttr;
	VO_VIDEO_LAYER_ATTR_S stLayerAttr;

	VoLayer = 0;
	u8ChannelNum = 0;


	s32Ret= HI_MPI_VO_SetAttrBegin(VoLayer);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start VO failed!\n");
		ExitVoReleaseData();
		return HI_FAILURE;
	}

	for(i=0;i<CHANNEL_NUM;i++)
	{
		if(BIT_CHECK(u32OutChannelstatus,i))
		{
			SAMPLE_PRT("disable channe %d \n",i);
			s32Ret = HI_MPI_VO_DisableChn(VoLayer, i);
			if (HI_SUCCESS != s32Ret)
			{
				SAMPLE_PRT("HI_MPI_VO_DisableChn %d failed!\n",i);
				ExitVoReleaseData();
				return HI_FAILURE;
			}else
			{
				BIT_CLEAN(u32OutChannelstatus,i);
			}
		}
	}



	s32Ret = HI_MPI_VO_GetVideoLayerAttr(VoLayer, &stLayerAttr);
	if (s32Ret != HI_SUCCESS)
	{
		SAMPLE_PRT("failed with %#x!\n", s32Ret);
		return HI_FAILURE;
	}

	u32Width = stLayerAttr.stImageSize.u32Width;
	u32Height = stLayerAttr.stImageSize.u32Height;

	SAMPLE_PRT("u32Width = %d u32Height = %d \n", u32Width,u32Height);


	if (stLayerAttr.u32DispFrmRt <= 0)
	{
		s32ChnFrmRate = 30;
	}
	else if (stLayerAttr.u32DispFrmRt > 30)
	{
		s32ChnFrmRate = stLayerAttr.u32DispFrmRt / 2;
	}
	else
	{
		s32ChnFrmRate = stLayerAttr.u32DispFrmRt;
	}

	for(i=0;i<2;i++)
	{

		u8ChannelNum = i;
		if(0==u8ChannelNum)
		{
			stChnAttr.stRect.s32X = 0;
			stChnAttr.stRect.s32Y = 0;
			stChnAttr.stRect.u32Width  = ALIGN_BACK(u32Width,2);
			stChnAttr.stRect.u32Height = ALIGN_BACK(u32Height/2,2);
		}else if(1==u8ChannelNum)
		{
			stChnAttr.stRect.s32X = 0;
			stChnAttr.stRect.s32Y = ALIGN_BACK(u32Height/2,2);
			stChnAttr.stRect.u32Width  = ALIGN_BACK(u32Width,2);
			stChnAttr.stRect.u32Height = ALIGN_BACK(u32Height/2,2);
		}else
		{
			
		}

		stChnAttr.u32Priority		= 0;
		stChnAttr.bDeflicker		= HI_FALSE;

		s32Ret = HI_MPI_VO_SetChnAttr(VoLayer, u8ChannelNum, &stChnAttr);
		if (s32Ret != HI_SUCCESS)
		{
			printf("%s(%d):failed with %#x!\n", __FUNCTION__, __LINE__, s32Ret);
			ExitVoReleaseData();
			return HI_FAILURE;
		}
		
		printf("[%s] [i:%d] [s32ChnFrmRate:%d] [VoLayer:%d]\n",
			__func__, i, s32ChnFrmRate, VoLayer);
		
		s32Ret = HI_MPI_VO_SetChnFrameRate(VoLayer, u8ChannelNum, s32ChnFrmRate);
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("failed with %#x!\n", s32Ret);
			ExitVoReleaseData();
			return HI_FAILURE;
		}
		
		s32Ret = HI_MPI_VO_EnableChn(VoLayer, u8ChannelNum);
		if (s32Ret != HI_SUCCESS)
		{
			SAMPLE_PRT("failed with %#x! VoLayer = %d ch = %d\n", s32Ret,VoLayer,u8ChannelNum);
			ExitVoReleaseData();
			return HI_FAILURE;
		}
		BIT_SET(u32OutChannelstatus,u8ChannelNum);

	}	
	
	s32Ret= HI_MPI_VO_SetAttrEnd(VoLayer);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start VO failed!\n");
		ExitVoReleaseData();
		return HI_FAILURE;
	}

	return HI_SUCCESS;
}

/**两个窗口分别显示3,4通道**/
int TwoWindowsDispalyCha_3_4(void) 
{
	int i = 0;
	int s32Ret = 0;
	HI_U32 u32WndNum = 0;
	HI_U32 u32Square = 0;
	HI_U32 u32Width = 0;
	HI_U32 u32Height = 0;
	HI_S32 s32ChnFrmRate;
	HI_U8  u8ChannelNum = 2;
	VO_LAYER VoLayer;
	SAMPLE_VO_MODE_E enVoMode;
	SAMPLE_VO_MODE_E enPreVoMode;
	VO_CHN_ATTR_S stChnAttr;
	VO_VIDEO_LAYER_ATTR_S stLayerAttr;

	VoLayer = 0;
	u8ChannelNum = 2;


	s32Ret= HI_MPI_VO_SetAttrBegin(VoLayer);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start VO failed!\n");
		ExitVoReleaseData();
		return HI_FAILURE;
	}

	for(i=0;i<CHANNEL_NUM;i++)
	{
		if(BIT_CHECK(u32OutChannelstatus,i))
		{
			SAMPLE_PRT("disable channe %d \n",i);
			s32Ret = HI_MPI_VO_DisableChn(VoLayer, i);
			if (HI_SUCCESS != s32Ret)
			{
				SAMPLE_PRT("HI_MPI_VO_DisableChn %d failed!\n",i);
				ExitVoReleaseData();
				return HI_FAILURE;
			}else
			{
				BIT_CLEAN(u32OutChannelstatus,i);
			}
		}
	}



	s32Ret = HI_MPI_VO_GetVideoLayerAttr(VoLayer, &stLayerAttr);
	if (s32Ret != HI_SUCCESS)
	{
		SAMPLE_PRT("failed with %#x!\n", s32Ret);
		return HI_FAILURE;
	}

	u32Width = stLayerAttr.stImageSize.u32Width;
	u32Height = stLayerAttr.stImageSize.u32Height;

	SAMPLE_PRT("u32Width = %d u32Height = %d \n", u32Width,u32Height);


	if (stLayerAttr.u32DispFrmRt <= 0)
	{
		s32ChnFrmRate = 30;
	}
	else if (stLayerAttr.u32DispFrmRt > 30)
	{
		s32ChnFrmRate = stLayerAttr.u32DispFrmRt / 2;
	}
	else
	{
		s32ChnFrmRate = stLayerAttr.u32DispFrmRt;
	}

	for(i=2;i<4;i++)
	{

		u8ChannelNum = i;
		if(2==u8ChannelNum)
		{
			stChnAttr.stRect.s32X = 0;
			stChnAttr.stRect.s32Y = 0;
			stChnAttr.stRect.u32Width  = ALIGN_BACK(u32Width,2);
			stChnAttr.stRect.u32Height = ALIGN_BACK(u32Height/2,2);

		}else if(3==u8ChannelNum)
		{
			stChnAttr.stRect.s32X = 0;
			stChnAttr.stRect.s32Y = ALIGN_BACK(u32Height/2,2);
			stChnAttr.stRect.u32Width  = ALIGN_BACK(u32Width,2);
			stChnAttr.stRect.u32Height = ALIGN_BACK(u32Height/2,2);

		}else
		{
			
		}

		stChnAttr.u32Priority		= 0;
		stChnAttr.bDeflicker		= HI_FALSE;

		s32Ret = HI_MPI_VO_SetChnAttr(VoLayer, u8ChannelNum, &stChnAttr);
		if (s32Ret != HI_SUCCESS)
		{
			printf("%s(%d):failed with %#x!\n", __FUNCTION__, __LINE__, s32Ret);
			ExitVoReleaseData();
			return HI_FAILURE;
		}
		
		printf("[%s] [i:%d] [s32ChnFrmRate:%d] [VoLayer:%d]\n",
			__func__, i, s32ChnFrmRate, VoLayer);
		
		s32Ret = HI_MPI_VO_SetChnFrameRate(VoLayer, u8ChannelNum, s32ChnFrmRate);
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("failed with %#x!\n", s32Ret);
			ExitVoReleaseData();
			return HI_FAILURE;
		}
		
		s32Ret = HI_MPI_VO_EnableChn(VoLayer, u8ChannelNum);
		if (s32Ret != HI_SUCCESS)
		{
			SAMPLE_PRT("failed with %#x! VoLayer = %d ch = %d\n", s32Ret,VoLayer,u8ChannelNum);
			ExitVoReleaseData();
			return HI_FAILURE;
		}

		BIT_SET(u32OutChannelstatus,u8ChannelNum);
	}	
	
	s32Ret= HI_MPI_VO_SetAttrEnd(VoLayer);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start VO failed!\n");
		ExitVoReleaseData();
		return HI_FAILURE;
	}

	return HI_SUCCESS;
}


/**三个个窗口分别显示1 2 3 通道**/
int ThreeWindowsDispalyCha_1_2_3_mode01(void) 
{
	int i = 0;
	int s32Ret = 0;
	HI_U32 u32WndNum = 0;
	HI_U32 u32Square = 0;
	HI_U32 u32Width = 0;
	HI_U32 u32Height = 0;
	HI_S32 s32ChnFrmRate;
	HI_U8  u8ChannelNum;
	VO_LAYER VoLayer;
	SAMPLE_VO_MODE_E enVoMode;
	SAMPLE_VO_MODE_E enPreVoMode;
	VO_CHN_ATTR_S stChnAttr;
	VO_VIDEO_LAYER_ATTR_S stLayerAttr;

	VoLayer = 0;
	u8ChannelNum = 0;


	s32Ret= HI_MPI_VO_SetAttrBegin(VoLayer);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start VO failed!\n");
		ExitVoReleaseData();
		return HI_FAILURE;
	}

	for(i=0;i<CHANNEL_NUM;i++)
	{
		if(BIT_CHECK(u32OutChannelstatus,i))
		{
			SAMPLE_PRT("disable channe %d \n",i);
			s32Ret = HI_MPI_VO_DisableChn(VoLayer, i);
			if (HI_SUCCESS != s32Ret)
			{
				SAMPLE_PRT("HI_MPI_VO_DisableChn %d failed!\n",i);
				ExitVoReleaseData();
				return HI_FAILURE;
			}else
			{
				BIT_CLEAN(u32OutChannelstatus,i);
			}
		}
	}



	s32Ret = HI_MPI_VO_GetVideoLayerAttr(VoLayer, &stLayerAttr);
	if (s32Ret != HI_SUCCESS)
	{
		SAMPLE_PRT("failed with %#x!\n", s32Ret);
		return HI_FAILURE;
	}

	u32Width = stLayerAttr.stImageSize.u32Width;
	u32Height = stLayerAttr.stImageSize.u32Height;

	SAMPLE_PRT("u32Width = %d u32Height = %d \n", u32Width,u32Height);


	if (stLayerAttr.u32DispFrmRt <= 0)
	{
		s32ChnFrmRate = 30;
	}
	else if (stLayerAttr.u32DispFrmRt > 30)
	{
		s32ChnFrmRate = stLayerAttr.u32DispFrmRt / 2;
	}
	else
	{
		s32ChnFrmRate = stLayerAttr.u32DispFrmRt;
	}

	for(i=0;i<3;i++)
	{
		u8ChannelNum = i;
		if(0==u8ChannelNum)
		{
			stChnAttr.stRect.s32X = 0;
			stChnAttr.stRect.s32Y = 0;
			stChnAttr.stRect.u32Width  = ALIGN_BACK(u32Width/2,2);
			stChnAttr.stRect.u32Height = ALIGN_BACK(u32Height/2,2);
		}else if(1==u8ChannelNum)
		{
			stChnAttr.stRect.s32X = ALIGN_BACK(u32Width/2,2);
			stChnAttr.stRect.s32Y = 0;
			stChnAttr.stRect.u32Width  = ALIGN_BACK(u32Width/2,2);
			stChnAttr.stRect.u32Height = ALIGN_BACK(u32Height/2,2);
		}else if(2 ==u8ChannelNum)
		{
			stChnAttr.stRect.s32X = 0;
			stChnAttr.stRect.s32Y = ALIGN_BACK(u32Height/2,2);
			stChnAttr.stRect.u32Width  = ALIGN_BACK(u32Width,2);
			stChnAttr.stRect.u32Height = ALIGN_BACK(u32Height/2,2);
		}

		stChnAttr.u32Priority		= 0;
		stChnAttr.bDeflicker		= HI_FALSE;

		s32Ret = HI_MPI_VO_SetChnAttr(VoLayer, u8ChannelNum, &stChnAttr);
		if (s32Ret != HI_SUCCESS)
		{
			printf("%s(%d):failed with %#x!\n", __FUNCTION__, __LINE__, s32Ret);
			ExitVoReleaseData();
			return HI_FAILURE;
		}
		
		printf("[%s] [i:%d] [s32ChnFrmRate:%d] [VoLayer:%d]\n",
			__func__, i, s32ChnFrmRate, VoLayer);
		
		s32Ret = HI_MPI_VO_SetChnFrameRate(VoLayer, u8ChannelNum, s32ChnFrmRate);
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("failed with %#x!\n", s32Ret);
			ExitVoReleaseData();
			return HI_FAILURE;
		}
		
		s32Ret = HI_MPI_VO_EnableChn(VoLayer, u8ChannelNum);
		if (s32Ret != HI_SUCCESS)
		{
			SAMPLE_PRT("failed with %#x! VoLayer = %d ch = %d\n", s32Ret,VoLayer,u8ChannelNum);
			ExitVoReleaseData();
			return HI_FAILURE;
		}

		BIT_SET(u32OutChannelstatus,u8ChannelNum);
	}	
	
	s32Ret= HI_MPI_VO_SetAttrEnd(VoLayer);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start VO failed!\n");
		ExitVoReleaseData();
		return HI_FAILURE;
	}

	return HI_SUCCESS;
}


/**三个个窗口分别显示1 2 3 通道**/
int ThreeWindowsDispalyCha_1_2_3_mode02(void) 
{
	int i = 0;
	int s32Ret = 0;
	HI_U32 u32WndNum = 0;
	HI_U32 u32Square = 0;
	HI_U32 u32Width = 0;
	HI_U32 u32Height = 0;
	HI_S32 s32ChnFrmRate;
	HI_U8  u8ChannelNum;
	VO_LAYER VoLayer;
	SAMPLE_VO_MODE_E enVoMode;
	SAMPLE_VO_MODE_E enPreVoMode;
	VO_CHN_ATTR_S stChnAttr;
	VO_VIDEO_LAYER_ATTR_S stLayerAttr;

	VoLayer = 0;
	u8ChannelNum = 0;


	s32Ret= HI_MPI_VO_SetAttrBegin(VoLayer);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start VO failed!\n");
		ExitVoReleaseData();
		return HI_FAILURE;
	}

	for(i=0;i<CHANNEL_NUM;i++)
	{
		if(BIT_CHECK(u32OutChannelstatus,i))
		{
			SAMPLE_PRT("disable channe %d \n",i);
			s32Ret = HI_MPI_VO_DisableChn(VoLayer, i);
			if (HI_SUCCESS != s32Ret)
			{
				SAMPLE_PRT("HI_MPI_VO_DisableChn %d failed!\n",i);
				ExitVoReleaseData();
				return HI_FAILURE;
			}else
			{
				BIT_CLEAN(u32OutChannelstatus,i);
			}
		}
	}



	s32Ret = HI_MPI_VO_GetVideoLayerAttr(VoLayer, &stLayerAttr);
	if (s32Ret != HI_SUCCESS)
	{
		SAMPLE_PRT("failed with %#x!\n", s32Ret);
		return HI_FAILURE;
	}

	u32Width = stLayerAttr.stImageSize.u32Width;
	u32Height = stLayerAttr.stImageSize.u32Height;

	SAMPLE_PRT("u32Width = %d u32Height = %d \n", u32Width,u32Height);


	if (stLayerAttr.u32DispFrmRt <= 0)
	{
		s32ChnFrmRate = 30;
	}
	else if (stLayerAttr.u32DispFrmRt > 30)
	{
		s32ChnFrmRate = stLayerAttr.u32DispFrmRt / 2;
	}
	else
	{
		s32ChnFrmRate = stLayerAttr.u32DispFrmRt;
	}

	for(i=0;i<3;i++)
	{
		u8ChannelNum = i;
		if(0==u8ChannelNum)
		{
			stChnAttr.stRect.s32X = 0;
			stChnAttr.stRect.s32Y = 0;
			stChnAttr.stRect.u32Width  = ALIGN_BACK(u32Width,2);
			stChnAttr.stRect.u32Height = ALIGN_BACK(u32Height/2,2);
		}else if(1==u8ChannelNum)
		{
			stChnAttr.stRect.s32X = 0;
			stChnAttr.stRect.s32Y = ALIGN_BACK(u32Height/2,2);
			stChnAttr.stRect.u32Width  = ALIGN_BACK(u32Width/2,2);
			stChnAttr.stRect.u32Height = ALIGN_BACK(u32Height/2,2);
		}else if(2 ==u8ChannelNum)
		{
			stChnAttr.stRect.s32X = ALIGN_BACK(u32Width/2,2);
			stChnAttr.stRect.s32Y = ALIGN_BACK(u32Height/2,2);
			stChnAttr.stRect.u32Width  = ALIGN_BACK(u32Width/2,2);
			stChnAttr.stRect.u32Height = ALIGN_BACK(u32Height/2,2);
		}

		stChnAttr.u32Priority		= 0;
		stChnAttr.bDeflicker		= HI_FALSE;

		s32Ret = HI_MPI_VO_SetChnAttr(VoLayer, u8ChannelNum, &stChnAttr);
		if (s32Ret != HI_SUCCESS)
		{
			printf("%s(%d):failed with %#x!\n", __FUNCTION__, __LINE__, s32Ret);
			ExitVoReleaseData();
			return HI_FAILURE;
		}
		
		printf("[%s] [i:%d] [s32ChnFrmRate:%d] [VoLayer:%d]\n",
			__func__, i, s32ChnFrmRate, VoLayer);
		
		s32Ret = HI_MPI_VO_SetChnFrameRate(VoLayer, u8ChannelNum, s32ChnFrmRate);
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("failed with %#x!\n", s32Ret);
			ExitVoReleaseData();
			return HI_FAILURE;
		}
		
		s32Ret = HI_MPI_VO_EnableChn(VoLayer, u8ChannelNum);
		if (s32Ret != HI_SUCCESS)
		{
			SAMPLE_PRT("failed with %#x! VoLayer = %d ch = %d\n", s32Ret,VoLayer,u8ChannelNum);
			ExitVoReleaseData();
			return HI_FAILURE;
		}

		BIT_SET(u32OutChannelstatus,u8ChannelNum);
	}	
	
	s32Ret= HI_MPI_VO_SetAttrEnd(VoLayer);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start VO failed!\n");
		ExitVoReleaseData();
		return HI_FAILURE;
	}

	return HI_SUCCESS;
}

/**三个个窗口分别显示1 2 3 通道**/
int ThreeWindowsDispalyCha_1_2_3_mode03(void) 
{
	int i = 0;
	int s32Ret = 0;
	HI_U32 u32WndNum = 0;
	HI_U32 u32Square = 0;
	HI_U32 u32Width = 0;
	HI_U32 u32Height = 0;
	HI_S32 s32ChnFrmRate;
	HI_U8  u8ChannelNum;
	VO_LAYER VoLayer;
	SAMPLE_VO_MODE_E enVoMode;
	SAMPLE_VO_MODE_E enPreVoMode;
	VO_CHN_ATTR_S stChnAttr;
	VO_VIDEO_LAYER_ATTR_S stLayerAttr;

	VoLayer = 0;
	u8ChannelNum = 0;


	s32Ret= HI_MPI_VO_SetAttrBegin(VoLayer);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start VO failed!\n");
		ExitVoReleaseData();
		return HI_FAILURE;
	}

	for(i=0;i<CHANNEL_NUM;i++)
	{
		if(BIT_CHECK(u32OutChannelstatus,i))
		{
			SAMPLE_PRT("disable channe %d \n",i);
			s32Ret = HI_MPI_VO_DisableChn(VoLayer, i);
			if (HI_SUCCESS != s32Ret)
			{
				SAMPLE_PRT("HI_MPI_VO_DisableChn %d failed!\n",i);
				ExitVoReleaseData();
				return HI_FAILURE;
			}else
			{
				BIT_CLEAN(u32OutChannelstatus,i);
			}
		}
	}



	s32Ret = HI_MPI_VO_GetVideoLayerAttr(VoLayer, &stLayerAttr);
	if (s32Ret != HI_SUCCESS)
	{
		SAMPLE_PRT("failed with %#x!\n", s32Ret);
		return HI_FAILURE;
	}

	u32Width = stLayerAttr.stImageSize.u32Width;
	u32Height = stLayerAttr.stImageSize.u32Height;

	SAMPLE_PRT("u32Width = %d u32Height = %d \n", u32Width,u32Height);


	if (stLayerAttr.u32DispFrmRt <= 0)
	{
		s32ChnFrmRate = 30;
	}
	else if (stLayerAttr.u32DispFrmRt > 30)
	{
		s32ChnFrmRate = stLayerAttr.u32DispFrmRt / 2;
	}
	else
	{
		s32ChnFrmRate = stLayerAttr.u32DispFrmRt;
	}

	for(i=0;i<3;i++)
	{
		u8ChannelNum = i;
		if(0==u8ChannelNum)
		{
			stChnAttr.stRect.s32X = 0;
			stChnAttr.stRect.s32Y = 0;
			stChnAttr.stRect.u32Width  = ALIGN_BACK(u32Width,2);
			stChnAttr.stRect.u32Height = ALIGN_BACK(u32Height/3,2);
		}else if(1==u8ChannelNum)
		{
			stChnAttr.stRect.s32X = 0;
			stChnAttr.stRect.s32Y = ALIGN_BACK(u32Height/3,2);
			stChnAttr.stRect.u32Width  = ALIGN_BACK(u32Width,2);
			stChnAttr.stRect.u32Height = ALIGN_BACK(u32Height/3,2);
		}else if(2 ==u8ChannelNum)
		{
			stChnAttr.stRect.s32X = 0;
			stChnAttr.stRect.s32Y = ALIGN_BACK((u32Height/3)*2,2);
			stChnAttr.stRect.u32Width  = ALIGN_BACK(u32Width,2);
			stChnAttr.stRect.u32Height = ALIGN_BACK(u32Height/3,2);
		}

		stChnAttr.u32Priority		= 0;
		stChnAttr.bDeflicker		= HI_FALSE;

		s32Ret = HI_MPI_VO_SetChnAttr(VoLayer, u8ChannelNum, &stChnAttr);
		if (s32Ret != HI_SUCCESS)
		{
			printf("%s(%d):failed with %#x!\n", __FUNCTION__, __LINE__, s32Ret);
			ExitVoReleaseData();
			return HI_FAILURE;
		}
		
		printf("[%s] [i:%d] [s32ChnFrmRate:%d] [VoLayer:%d]\n",
			__func__, i, s32ChnFrmRate, VoLayer);
		
		s32Ret = HI_MPI_VO_SetChnFrameRate(VoLayer, u8ChannelNum, s32ChnFrmRate);
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("failed with %#x!\n", s32Ret);
			ExitVoReleaseData();
			return HI_FAILURE;
		}
		
		s32Ret = HI_MPI_VO_EnableChn(VoLayer, u8ChannelNum);
		if (s32Ret != HI_SUCCESS)
		{
			SAMPLE_PRT("failed with %#x! VoLayer = %d ch = %d\n", s32Ret,VoLayer,u8ChannelNum);
			ExitVoReleaseData();
			return HI_FAILURE;
		}

		BIT_SET(u32OutChannelstatus,u8ChannelNum);
	}	
	
	s32Ret= HI_MPI_VO_SetAttrEnd(VoLayer);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start VO failed!\n");
		ExitVoReleaseData();
		return HI_FAILURE;
	}

	return HI_SUCCESS;
}


/**四个窗口分别显示1 2 3 4 通道**/
int FourWindowsDispalyCha_1_2_3_4(void) 
{
	int i = 0;
	int s32Ret = 0;
	HI_U32 u32WndNum = 0;
	HI_U32 u32Square = 0;
	HI_U32 u32Width = 0;
	HI_U32 u32Height = 0;
	HI_S32 s32ChnFrmRate;
	HI_U8  u8ChannelNum;
	VO_LAYER VoLayer;
	SAMPLE_VO_MODE_E enVoMode;
	SAMPLE_VO_MODE_E enPreVoMode;
	VO_CHN_ATTR_S stChnAttr;
	VO_VIDEO_LAYER_ATTR_S stLayerAttr;

	VoLayer = 0;
	u8ChannelNum = 0;


	s32Ret= HI_MPI_VO_SetAttrBegin(VoLayer);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start VO failed!\n");
		ExitVoReleaseData();
		return HI_FAILURE;
	}

	for(i=0;i<CHANNEL_NUM;i++)
	{
		if(BIT_CHECK(u32OutChannelstatus,i))
		{
			SAMPLE_PRT("disable channe %d \n",i);
			s32Ret = HI_MPI_VO_DisableChn(VoLayer, i);
			if (HI_SUCCESS != s32Ret)
			{
				SAMPLE_PRT("HI_MPI_VO_DisableChn %d failed!\n",i);
				ExitVoReleaseData();
				return HI_FAILURE;
			}else
			{
				BIT_CLEAN(u32OutChannelstatus,i);
			}
		}
	}



	s32Ret = HI_MPI_VO_GetVideoLayerAttr(VoLayer, &stLayerAttr);
	if (s32Ret != HI_SUCCESS)
	{
		SAMPLE_PRT("failed with %#x!\n", s32Ret);
		return HI_FAILURE;
	}

	u32Width = stLayerAttr.stImageSize.u32Width;
	u32Height = stLayerAttr.stImageSize.u32Height;

	SAMPLE_PRT("u32Width = %d u32Height = %d \n", u32Width,u32Height);


	if (stLayerAttr.u32DispFrmRt <= 0)
	{
		s32ChnFrmRate = 30;
	}
	else if (stLayerAttr.u32DispFrmRt > 30)
	{
		s32ChnFrmRate = stLayerAttr.u32DispFrmRt / 2;
	}
	else
	{
		s32ChnFrmRate = stLayerAttr.u32DispFrmRt;
	}

	for(i=0;i<4;i++)
	{
		u8ChannelNum = i;
#if 0		
		if(0==u8ChannelNum)
		{
			stChnAttr.stRect.s32X = 0;
			stChnAttr.stRect.s32Y = 0;
			stChnAttr.stRect.u32Width  = ALIGN_BACK(u32Width/3,2);
			stChnAttr.stRect.u32Height = ALIGN_BACK(u32Height,2);
		}else if(1==u8ChannelNum)
		{
			stChnAttr.stRect.s32X = ALIGN_BACK(u32Width/3,2);
			stChnAttr.stRect.s32Y = 0;
			stChnAttr.stRect.u32Width  = ALIGN_BACK(u32Width/3,2);
			stChnAttr.stRect.u32Height = ALIGN_BACK(u32Height/2,2);
		}else if(2 ==u8ChannelNum)
		{
			stChnAttr.stRect.s32X = ALIGN_BACK(u32Width/3,2);
			stChnAttr.stRect.s32Y = ALIGN_BACK(u32Height/2,2);
			stChnAttr.stRect.u32Width  = ALIGN_BACK(u32Width/3,2);
			stChnAttr.stRect.u32Height = ALIGN_BACK(u32Height/2,2);
		}
		else if(3 ==u8ChannelNum)
		{
			stChnAttr.stRect.s32X = ALIGN_BACK((u32Width/3)*2,2);
			stChnAttr.stRect.s32Y = 0;
			stChnAttr.stRect.u32Width  = ALIGN_BACK(u32Width/3,2);
			stChnAttr.stRect.u32Height = ALIGN_BACK(u32Height,2);
		}
#endif
		if(0==u8ChannelNum)
		{
			stChnAttr.stRect.s32X = 0;
			stChnAttr.stRect.s32Y = 0;
			stChnAttr.stRect.u32Width  = ALIGN_BACK(u32Width,2);
			stChnAttr.stRect.u32Height = ALIGN_BACK(u32Height/3,2);
		}else if(1==u8ChannelNum)
		{
			stChnAttr.stRect.s32X = 0;
			stChnAttr.stRect.s32Y = ALIGN_BACK(u32Height/3,2);
			stChnAttr.stRect.u32Width  = ALIGN_BACK(u32Width/2,2);
			stChnAttr.stRect.u32Height = ALIGN_BACK(u32Height/3,2);
		}else if(2 ==u8ChannelNum)
		{
			stChnAttr.stRect.s32X = ALIGN_BACK(u32Width/2,2);
			stChnAttr.stRect.s32Y = ALIGN_BACK(u32Height/3,2);
			stChnAttr.stRect.u32Width  = ALIGN_BACK(u32Width/2,2);
			stChnAttr.stRect.u32Height = ALIGN_BACK(u32Height/3,2);
		}
		else if(3 ==u8ChannelNum)
		{
			stChnAttr.stRect.s32X = 0;
			stChnAttr.stRect.s32Y = ALIGN_BACK((u32Height/3)*2,2);
			stChnAttr.stRect.u32Width  = ALIGN_BACK(u32Width,2);
			stChnAttr.stRect.u32Height = ALIGN_BACK(u32Height/3,2);
		}


		stChnAttr.u32Priority		= 0;
		stChnAttr.bDeflicker		= HI_FALSE;

		s32Ret = HI_MPI_VO_SetChnAttr(VoLayer, u8ChannelNum, &stChnAttr);
		if (s32Ret != HI_SUCCESS)
		{
			printf("%s(%d):failed with %#x!\n", __FUNCTION__, __LINE__, s32Ret);
			ExitVoReleaseData();
			return HI_FAILURE;
		}
		
		printf("[%s] [i:%d] [s32ChnFrmRate:%d] [VoLayer:%d]\n",
			__func__, i, s32ChnFrmRate, VoLayer);
		
		s32Ret = HI_MPI_VO_SetChnFrameRate(VoLayer, u8ChannelNum, s32ChnFrmRate);
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("failed with %#x!\n", s32Ret);
			ExitVoReleaseData();
			return HI_FAILURE;
		}
		
		s32Ret = HI_MPI_VO_EnableChn(VoLayer, u8ChannelNum);
		if (s32Ret != HI_SUCCESS)
		{
			SAMPLE_PRT("failed with %#x! VoLayer = %d ch = %d\n", s32Ret,VoLayer,u8ChannelNum);
			ExitVoReleaseData();
			return HI_FAILURE;
		}
		
		BIT_SET(u32OutChannelstatus,u8ChannelNum);

	}	
	
	s32Ret= HI_MPI_VO_SetAttrEnd(VoLayer);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start VO failed!\n");
		ExitVoReleaseData();
		return HI_FAILURE;
	}

	return HI_SUCCESS;
}









