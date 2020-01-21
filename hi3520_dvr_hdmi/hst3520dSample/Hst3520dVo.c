#include "Hst3520dVo.h"
#include "Hst3520dChnAdapt.h"

VO_ATTR_S g_stCvbsVoAttr = {0};

int Hst3520D_Sample_GetVoAttr(VO_ATTR_S *pstVoAttr)
{
    if( NULL == pstVoAttr )
    {
        return HST_FAILURE;
    }

    memcpy(pstVoAttr, &g_stCvbsVoAttr, sizeof(VO_ATTR_S));
    
    return HST_SUCCEED;
}

int Hst3520D_Sample_SetVoAttr(VO_ATTR_S *pstVoAttr)
{
    if( NULL == pstVoAttr )
    {
        return HST_FAILURE;
    }

    memcpy(&g_stCvbsVoAttr, pstVoAttr, sizeof(VO_ATTR_S));
    
    return HST_SUCCEED;
}

/*int Hst3520D_Sample_DefaultCVBSVoAttr()
{
	unsigned int CodecNum = Hst3520d_Adapt_Get_CodecNum();

	if(2 == CodecNum)
	{
	    printf("[%s] CodecNum:%d\n", __func__, CodecNum);//

		g_stCvbsVoAttr.u32WndNum = 9;
		g_stCvbsVoAttr.enVoMode  = VO_MODE_9MUX;
	}
	else//默认4路
	{
	    printf("[%s] CodecNum:%d\n", __func__, CodecNum);
	
		g_stCvbsVoAttr.u32WndNum = 4;
		g_stCvbsVoAttr.enVoMode  = VO_MODE_4MUX;
	}

	g_stCvbsVoAttr.VoDev  = SAMPLE_VO_DEV_DSD0;
	g_stCvbsVoAttr.VoLayer = SAMPLE_VO_LAYER_VSD0;
	g_stCvbsVoAttr.VpssChnSDVo = VPSS_CHN3;
	g_stCvbsVoAttr.stVoPubAttr.enIntfSync = VO_OUTPUT_PAL;
	g_stCvbsVoAttr.stVoPubAttr.enIntfType = VO_INTF_CVBS;
	g_stCvbsVoAttr.stVoPubAttr.u32BgColor = 0x000000ff; 	   
	
    return HST_SUCCEED;
}

int Hst3520D_Sample_StartCVBSVo()
{
    printf("start vo SD0\n");
    
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i;
    
    VI_CHN ViChn;
    VO_CHN VoChn;
    VPSS_GRP VpssGrp;
    VPSS_CHN VpssChn_VoSD0 = VPSS_CHN3;
    VO_VIDEO_LAYER_ATTR_S stLayerAttr;

    s32Ret = SAMPLE_COMM_VO_StartDev(g_stCvbsVoAttr.VoDev, &(g_stCvbsVoAttr.stVoPubAttr));
    if (HI_SUCCESS != s32Ret)
    {
        printf("Start SAMPLE_COMM_VO_StartDev failed!\n");
        goto END_START_CVBS_VO;
    }

    memset(&(stLayerAttr), 0, sizeof(VO_VIDEO_LAYER_ATTR_S));
    s32Ret = SAMPLE_COMM_VO_GetWH(g_stCvbsVoAttr.stVoPubAttr.enIntfSync, \
    	&stLayerAttr.stImageSize.u32Width, \
    	&stLayerAttr.stImageSize.u32Height, \
    	&stLayerAttr.u32DispFrmRt);
    if (HI_SUCCESS != s32Ret)
    {
        printf("Start SAMPLE_COMM_VO_GetWH failed!\n");
        goto END_START_CVBS_VO;
    }

    stLayerAttr.enPixFormat = SAMPLE_PIXEL_FORMAT;
    stLayerAttr.stDispRect.s32X 	  = 0;
    stLayerAttr.stDispRect.s32Y 	  = 0;
    stLayerAttr.stDispRect.u32Width   = stLayerAttr.stImageSize.u32Width;
    stLayerAttr.stDispRect.u32Height  = stLayerAttr.stImageSize.u32Height;
    
    s32Ret = SAMPLE_COMM_VO_StartLayer(g_stCvbsVoAttr.VoLayer, &stLayerAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("Start SAMPLE_COMM_VO_StartLayer failed!\n");
        goto END_START_CVBS_VO;
    }

    s32Ret = SAMPLE_COMM_VO_StartChn(g_stCvbsVoAttr.VoLayer, g_stCvbsVoAttr.enVoMode);
    if (HI_SUCCESS != s32Ret)
    {
        printf("Start SAMPLE_COMM_VO_StartChn failed!\n");
        goto END_START_CVBS_VO;
    }
    

    for(i=0; i<g_stCvbsVoAttr.u32WndNum; i++)
    {
        VoChn = i;
        VpssGrp = i;

        printf("[%s] [VoLayer:%d][VoChn:%d]\n", __func__, g_stCvbsVoAttr.VoLayer, VoChn);

        s32Ret = SAMPLE_COMM_VO_BindVpss(g_stCvbsVoAttr.VoLayer, VoChn,
            VpssGrp, g_stCvbsVoAttr.VpssChnSDVo);
        
        if (HI_SUCCESS != s32Ret)
        {
            printf("SAMPLE_COMM_VO_BindVpss!\n");
            goto END_START_CVBS_VO;
        }
    }

    printf("-------- %s succeed --------\n", __func__);

    return HI_SUCCESS;

END_START_CVBS_VO:

	SAMPLE_COMM_VO_StopChn(g_stCvbsVoAttr.VoLayer, g_stCvbsVoAttr.enVoMode);
    
	for(i=0;i<g_stCvbsVoAttr.u32WndNum;i++)
	{
		VoChn = i;
		VpssGrp = i;
		SAMPLE_COMM_VO_UnBindVpss(g_stCvbsVoAttr.VoLayer,VoChn,VpssGrp,g_stCvbsVoAttr.VpssChnSDVo);
	}
    
	SAMPLE_COMM_VO_StopLayer(g_stCvbsVoAttr.VoLayer);
	SAMPLE_COMM_VO_StopDev(g_stCvbsVoAttr.VoDev);
    
	return s32Ret;
}


int Hst3520D_Sample_StopCVBSVo()
{
    VI_CHN ViChn;
    VO_CHN VoChn;
    VPSS_GRP VpssGrp;
    VPSS_CHN VpssChn_VoSD0 = VPSS_CHN3;
    HI_S32 i;

    SAMPLE_COMM_VO_StopChn(g_stCvbsVoAttr.VoLayer, g_stCvbsVoAttr.enVoMode);

    for(i=0;i<g_stCvbsVoAttr.u32WndNum;i++)
    {
        VoChn = i;
        VpssGrp = i;
        SAMPLE_COMM_VO_UnBindVpss(g_stCvbsVoAttr.VoLayer, VoChn, VpssGrp, g_stCvbsVoAttr.VpssChnSDVo);
    }

    SAMPLE_COMM_VO_StopLayer(g_stCvbsVoAttr.VoLayer);
    SAMPLE_COMM_VO_StopDev(g_stCvbsVoAttr.VoDev);

    return HST_SUCCEED;
}*/
int Hst3520D_Sample_DefaultCVBSVoAttr()
{
    g_stCvbsVoAttr.VoDev  = SAMPLE_VO_DEV_DSD0;
    g_stCvbsVoAttr.VoLayer = SAMPLE_VO_LAYER_VSD0;
	
    g_stCvbsVoAttr.u32WndNum = 0x01;
	
    g_stCvbsVoAttr.VpssChnSDVo = VPSS_CHN0;
	
    g_stCvbsVoAttr.enVoMode    = VO_MODE_1MUX;
	
    g_stCvbsVoAttr.stVoPubAttr.enIntfSync = VO_OUTPUT_PAL;
    g_stCvbsVoAttr.stVoPubAttr.enIntfType = VO_INTF_CVBS;
    g_stCvbsVoAttr.stVoPubAttr.u32BgColor = 0x000000ff;        

    printf("[%s]设置视频输出参数\n", __func__);
    return HST_SUCCEED;
}
int Hst3520D_Sample_VoModeGet(SAMPLE_VO_MODE_E *penVoMode,unsigned int u32WndNum)
{
	unsigned int ShowNum=0;
	int i=0;
	
	for(i=0;i<=16;i++)
	{
		if((u32WndNum>>i)&0x01)
		{
			ShowNum++;
		}
	}
	if(ShowNum>9)
	{
		*penVoMode=VO_MODE_16MUX;
	}
	else if(ShowNum>4)
	{
		*penVoMode=VO_MODE_9MUX;
	}
	else if(ShowNum>1)
	{
		*penVoMode=VO_MODE_4MUX;
	}
	else if(ShowNum==1)
	{
		*penVoMode=VO_MODE_1MUX;
	}
	else
	{
		*penVoMode=VO_MODE_1MUX;
	}
	printf("[%s %d]:u32WndNum 0x%x  penVoMode 0x%x\n",__func__,__LINE__,u32WndNum,*penVoMode);
	return 0;
}

int Hst3520D_Sample_StartCVBSVo()
{
    printf("start vo SD0\n");
    
    HI_S32 s32Ret = HI_SUCCESS;    
    VI_CHN ViChn;
	static unsigned int ls_u32Flag = 0;
    VO_CHN VoChn;
	static HI_U32 ls_u32WndNum = 0;
    VPSS_GRP VpssGrp;
    VPSS_CHN VpssChn_VoSD0 = g_stCvbsVoAttr.VpssChnSDVo;
    VO_VIDEO_LAYER_ATTR_S stLayerAttr;
    int i=0;
	Hst3520D_Sample_GetVideoState(&ls_u32WndNum);
	ls_u32WndNum = ~ls_u32WndNum;
	ls_u32WndNum &= 0x3F;
	if(g_stCvbsVoAttr.u32WndNum != ls_u32WndNum && 0 != ls_u32Flag)
	{
	 	printf("[%s,%d]停止输出\n",__FILE__,__LINE__);
		SAMPLE_COMM_VO_StopChn(g_stCvbsVoAttr.VoLayer, g_stCvbsVoAttr.enVoMode);
		VoChn = 0;
		for(i=0;i<=16;i++)
		{
			if((g_stCvbsVoAttr.u32WndNum>>i)&0x01)
			{
				VpssGrp = i;
				SAMPLE_COMM_VO_UnBindVpss(g_stCvbsVoAttr.VoLayer,VoChn,VpssGrp,g_stCvbsVoAttr.VpssChnSDVo);
				VoChn++;
			}
		}
		SAMPLE_COMM_VO_StopLayer(g_stCvbsVoAttr.VoLayer);
		SAMPLE_COMM_VO_StopDev(g_stCvbsVoAttr.VoDev);
	}
	else if(g_stCvbsVoAttr.u32WndNum == ls_u32WndNum)
	{
		return HI_SUCCESS;
	}
	g_stCvbsVoAttr.u32WndNum = ls_u32WndNum;
    // 1 开启输出设备SD
    printf("[%s]分组设置启动视频输出step1\n", __func__);
    s32Ret = SAMPLE_COMM_VO_StartDev(g_stCvbsVoAttr.VoDev, &(g_stCvbsVoAttr.stVoPubAttr));
    if (HI_SUCCESS != s32Ret)
    {
        printf("Start SAMPLE_COMM_VO_StartDev failed!\n");
        goto END_START_CVBS_VO;
    }

	// 2 视频层属性获取
	printf("[%s]分组设置启动视频输出step2\n", __func__);
    memset(&(stLayerAttr), 0, sizeof(VO_VIDEO_LAYER_ATTR_S));
	
    s32Ret = SAMPLE_COMM_VO_GetWH(g_stCvbsVoAttr.stVoPubAttr.enIntfSync, \
    	&stLayerAttr.stImageSize.u32Width, \
    	&stLayerAttr.stImageSize.u32Height, \
    	&stLayerAttr.u32DispFrmRt);
	
    if (HI_SUCCESS != s32Ret)
    {
        printf("Start SAMPLE_COMM_VO_GetWH failed!\n");
        goto END_START_CVBS_VO;
    }

    stLayerAttr.enPixFormat = SAMPLE_PIXEL_FORMAT;
    stLayerAttr.stDispRect.s32X 	  = 0;
    stLayerAttr.stDispRect.s32Y 	  = 0;
    stLayerAttr.stDispRect.u32Width   = stLayerAttr.stImageSize.u32Width;
    stLayerAttr.stDispRect.u32Height  = stLayerAttr.stImageSize.u32Height;
    // 3 开启视频层
    printf("[%s]分组设置启动视频输出step3\n", __func__);
    s32Ret = SAMPLE_COMM_VO_StartLayer(g_stCvbsVoAttr.VoLayer, &stLayerAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("Start SAMPLE_COMM_VO_StartLayer failed!\n");
        goto END_START_CVBS_VO;
    }
    // 4 开启并设置通道属性
    //g_stCvbsVoAttr.u32WndNum = 3;
    printf("[%s]分组设置启动视频输出step4\n", __func__);
    Hst3520D_Sample_VoModeGet(&g_stCvbsVoAttr.enVoMode,g_stCvbsVoAttr.u32WndNum);
	
    printf("[%s %d]***********enVoMode  0x%x u32WndNum 0x%x************\n",__func__,__LINE__,
		g_stCvbsVoAttr.enVoMode,g_stCvbsVoAttr.u32WndNum);
	
    s32Ret = SAMPLE_COMM_VO_StartChn(g_stCvbsVoAttr.VoLayer, g_stCvbsVoAttr.enVoMode);
    if (HI_SUCCESS != s32Ret)
    {
        printf("Start SAMPLE_COMM_VO_StartChn failed!\n");
        goto END_START_CVBS_VO;
    }
    VoChn =0;
	for(i=0;i<=16;i++)
	{
		if(0 != Hst3520d_Adapt_Get_ChnFmt(i))
		if((g_stCvbsVoAttr.u32WndNum>>i)&0x01)
	    {
	        VpssGrp = i;
	        printf("[%s] [VoLayer:%d][VoChn:%d] Bind [VpssGrp %d]\n", __func__, g_stCvbsVoAttr.VoLayer, VoChn,VpssGrp);
	        s32Ret = SAMPLE_COMM_VO_BindVpss(g_stCvbsVoAttr.VoLayer, VoChn,
	            VpssGrp, g_stCvbsVoAttr.VpssChnSDVo);
	        
	        if (HI_SUCCESS != s32Ret)
	        {
	            printf("SAMPLE_COMM_VO_BindVpss!\n");
	            goto END_START_CVBS_VO;
	        }
			VoChn++;
	    }
	}
    printf("-------- %s succeed --------\n", __func__);

	ls_u32Flag = 1;



	

    return HI_SUCCESS;

END_START_CVBS_VO:
    printf("[%s]分组设置启动视频输出step5\n", __func__);
	SAMPLE_COMM_VO_StopChn(g_stCvbsVoAttr.VoLayer, g_stCvbsVoAttr.enVoMode);
	VoChn = 0;
	for(i=0;i<=16;i++)
	{
		if((g_stCvbsVoAttr.u32WndNum>>i)&0x01)
		{
			VpssGrp = i;
			SAMPLE_COMM_VO_UnBindVpss(g_stCvbsVoAttr.VoLayer,VoChn,VpssGrp,g_stCvbsVoAttr.VpssChnSDVo);
			VoChn++;
		}
	}
	SAMPLE_COMM_VO_StopLayer(g_stCvbsVoAttr.VoLayer);
	SAMPLE_COMM_VO_StopDev(g_stCvbsVoAttr.VoDev);
    
	return s32Ret;
}

int Hst3520D_Sample_StopCVBSVo()
{
    VI_CHN ViChn;
    VO_CHN VoChn;
    VPSS_GRP VpssGrp;
    VPSS_CHN VpssChn_VoSD0 = g_stCvbsVoAttr.VpssChnSDVo;
    HI_S32 i;
	
    Hst3520D_Sample_VoModeGet(&g_stCvbsVoAttr.enVoMode,g_stCvbsVoAttr.u32WndNum);
    printf("[%s %d]***********enVoMode %d u32WndNum %d************\n",__func__,__LINE__,
		g_stCvbsVoAttr.enVoMode,g_stCvbsVoAttr.u32WndNum);
    SAMPLE_COMM_VO_StopChn(g_stCvbsVoAttr.VoLayer, g_stCvbsVoAttr.enVoMode);
	VoChn = 0;
	for(i=0;i<=16;i++)
	{
		if((g_stCvbsVoAttr.u32WndNum>>i)&0x01)
	    {
	        VpssGrp = i;
	        SAMPLE_COMM_VO_UnBindVpss(g_stCvbsVoAttr.VoLayer, VoChn, VpssGrp, g_stCvbsVoAttr.VpssChnSDVo);
			VoChn++;
		}
	}
    SAMPLE_COMM_VO_StopLayer(g_stCvbsVoAttr.VoLayer);
    SAMPLE_COMM_VO_StopDev(g_stCvbsVoAttr.VoDev);

    return HST_SUCCEED;
}



/**************************add by licabiao*****************************/
VO_ATTR_S g_stHdmiVoAttr = {0};

int Hst3520D_Sample_GetHdmiVoAttr(VO_ATTR_S *pstVoAttr)
{
    if( NULL == pstVoAttr )
    {
        return HST_FAILURE;
    }

    memcpy(pstVoAttr, &g_stHdmiVoAttr, sizeof(VO_ATTR_S));
    
    return HST_SUCCEED;
}

int Hst3520D_Sample_SetHdmiVoAttr(VO_ATTR_S *pstVoAttr)
{
    if( NULL == pstVoAttr )
    {
        return HST_FAILURE;
    }

    memcpy(&g_stHdmiVoAttr, pstVoAttr, sizeof(VO_ATTR_S));
    
    return HST_SUCCEED;
}

int Hst3520D_Sample_DefaultHdmiVoAttr()
{
    g_stHdmiVoAttr.VoDev  = SAMPLE_VO_DEV_DSD0;
    g_stHdmiVoAttr.VoLayer = SAMPLE_VO_LAYER_VSD0;
	
    g_stHdmiVoAttr.u32WndNum = 0x01;
	
    g_stHdmiVoAttr.VpssChnSDVo = VPSS_CHN0;
	
    g_stHdmiVoAttr.enVoMode    = VO_MODE_1MUX;
	
    g_stHdmiVoAttr.stVoPubAttr.enIntfSync = VO_OUTPUT_PAL;
    g_stHdmiVoAttr.stVoPubAttr.enIntfType = VO_INTF_HDMI;
    g_stHdmiVoAttr.stVoPubAttr.u32BgColor = 0x000000ff;        

    printf("[%s]设置视频输出参数\n", __func__);
    return HST_SUCCEED;
}
int Hst3520D_Sample_HdmiVoModeGet(SAMPLE_VO_MODE_E *penVoMode,unsigned int u32WndNum)
{
	unsigned int ShowNum=0;
	int i=0;
	
	for(i=0;i<=16;i++)
	{
		if((u32WndNum>>i)&0x01)
		{
			ShowNum++;
		}
	}
	if(ShowNum>9)
	{
		*penVoMode=VO_MODE_16MUX;
	}
	else if(ShowNum>4)
	{
		*penVoMode=VO_MODE_9MUX;
	}
	else if(ShowNum>1)
	{
		*penVoMode=VO_MODE_4MUX;
	}
	else if(ShowNum==1)
	{
		*penVoMode=VO_MODE_1MUX;
	}
	else
	{
		*penVoMode=VO_MODE_1MUX;
	}
	printf("[%s %d]:u32WndNum 0x%x  penVoMode 0x%x\n",__func__,__LINE__,u32WndNum,*penVoMode);
	return 0;
}

#if 1
//wisdom
int Hst_3520D_Sample_StartHdmiVo()
{
    printf("start vo SD0\n");
    
    HI_S32 s32Ret = HI_SUCCESS;    
    VI_CHN ViChn;
	static unsigned int ls_u32Flag = 0;
    VO_CHN VoChn;
	static HI_U32 ls_u32WndNum = 0;
    VPSS_GRP VpssGrp;
    VPSS_CHN VpssChn_VoSD0 = g_stHdmiVoAttr.VpssChnSDVo;
    VO_VIDEO_LAYER_ATTR_S stLayerAttr;
    int i=0;
	Hst3520D_Sample_GetVideoState(&ls_u32WndNum);
	ls_u32WndNum = ~ls_u32WndNum;
	ls_u32WndNum &= 0x3F;
	if(g_stHdmiVoAttr.u32WndNum != ls_u32WndNum && 0 != ls_u32Flag)
	{
	 	printf("[%s,%d]停止输出\n",__FILE__,__LINE__);
		SAMPLE_COMM_VO_StopChn(g_stHdmiVoAttr.VoLayer, g_stHdmiVoAttr.enVoMode);
		VoChn = 0;
		for(i=0;i<=16;i++)
		{
			if((g_stHdmiVoAttr.u32WndNum>>i)&0x01)
			{
				VpssGrp = i;
				SAMPLE_COMM_VO_UnBindVpss(g_stHdmiVoAttr.VoLayer,VoChn,VpssGrp,g_stHdmiVoAttr.VpssChnSDVo);
				VoChn++;
			}
		}
		SAMPLE_COMM_VO_StopLayer(g_stHdmiVoAttr.VoLayer);
		SAMPLE_COMM_VO_StopDev(g_stHdmiVoAttr.VoDev);
	}
	else if(g_stHdmiVoAttr.u32WndNum == ls_u32WndNum)
	{
		return HI_SUCCESS;
	}
	
	g_stHdmiVoAttr.u32WndNum = ls_u32WndNum;
    // 1 开启输出设备SD
    printf("[%s]分组设置启动视频输出step1\n", __func__);
    s32Ret = SAMPLE_COMM_VO_StartDev(g_stHdmiVoAttr.VoDev, &(g_stHdmiVoAttr.stVoPubAttr));
    if (HI_SUCCESS != s32Ret)
    {
        printf("Start SAMPLE_COMM_VO_StartDev failed!\n");
        goto END_START_CVBS_VO;
    }

	// 2 视频层属性获取
	printf("[%s]分组设置启动视频输出step2\n", __func__);
    memset(&(stLayerAttr), 0, sizeof(VO_VIDEO_LAYER_ATTR_S));
	
    s32Ret = SAMPLE_COMM_VO_GetWH(g_stHdmiVoAttr.stVoPubAttr.enIntfSync, \
    	&stLayerAttr.stImageSize.u32Width, \
    	&stLayerAttr.stImageSize.u32Height, \
    	&stLayerAttr.u32DispFrmRt);
	
    if (HI_SUCCESS != s32Ret)
    {
        printf("Start SAMPLE_COMM_VO_GetWH failed!\n");
        goto END_START_CVBS_VO;
    }

    stLayerAttr.enPixFormat = SAMPLE_PIXEL_FORMAT;
    stLayerAttr.stDispRect.s32X 	  = 0;
    stLayerAttr.stDispRect.s32Y 	  = 0;
    stLayerAttr.stDispRect.u32Width   = stLayerAttr.stImageSize.u32Width;
    stLayerAttr.stDispRect.u32Height  = stLayerAttr.stImageSize.u32Height;
    // 3 开启视频层
    printf("[%s]分组设置启动视频输出step3\n", __func__);
    s32Ret = SAMPLE_COMM_VO_StartLayer(g_stHdmiVoAttr.VoLayer, &stLayerAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("Start SAMPLE_COMM_VO_StartLayer failed!\n");
        goto END_START_CVBS_VO;
    }
    // 4 开启并设置通道属性
    //g_stCvbsVoAttr.u32WndNum = 3;
    printf("[%s]分组设置启动视频输出step4\n", __func__);
    Hst3520D_Sample_VoModeGet(&g_stHdmiVoAttr.enVoMode,g_stHdmiVoAttr.u32WndNum);
	
    printf("[%s %d]***********enVoMode  0x%x u32WndNum 0x%x************\n",__func__,__LINE__,
		g_stHdmiVoAttr.enVoMode,g_stHdmiVoAttr.u32WndNum);
	
    s32Ret = SAMPLE_COMM_VO_StartChn(g_stHdmiVoAttr.VoLayer, g_stHdmiVoAttr.enVoMode);
    if (HI_SUCCESS != s32Ret)
    {
        printf("Start SAMPLE_COMM_VO_StartChn failed!\n");
        goto END_START_CVBS_VO;
    }
    VoChn =0;
	for(i=0;i<=16;i++)
	{
		if(0 != Hst3520d_Adapt_Get_ChnFmt(i))
		if((g_stHdmiVoAttr.u32WndNum>>i)&0x01)
	    {
	        VpssGrp = i;
	        printf("[%s] [VoLayer:%d][VoChn:%d] Bind [VpssGrp %d]\n", __func__, g_stHdmiVoAttr.VoLayer, VoChn,VpssGrp);
	        s32Ret = SAMPLE_COMM_VO_BindVpss(g_stHdmiVoAttr.VoLayer, VoChn,
	            VpssGrp, g_stHdmiVoAttr.VpssChnSDVo);
	        
	        if (HI_SUCCESS != s32Ret)
	        {
	            printf("SAMPLE_COMM_VO_BindVpss!\n");
	            goto END_START_CVBS_VO;
	        }
			VoChn++;
	    }
	}
    printf("-------- %s succeed --------\n", __func__);

	ls_u32Flag = 1;

	//Init_hdmi(g_stHdmiVoAttr.stVoPubAttr.enIntfSync);

    return HI_SUCCESS;

END_START_CVBS_VO:
    printf("[%s]分组设置启动视频输出step5\n", __func__);
	SAMPLE_COMM_VO_StopChn(g_stHdmiVoAttr.VoLayer, g_stHdmiVoAttr.enVoMode);
	VoChn = 0;
	for(i=0;i<=16;i++)
	{
		if((g_stHdmiVoAttr.u32WndNum>>i)&0x01)
		{
			VpssGrp = i;
			SAMPLE_COMM_VO_UnBindVpss(g_stHdmiVoAttr.VoLayer,VoChn,VpssGrp,g_stHdmiVoAttr.VpssChnSDVo);
			VoChn++;
		}
	}
	SAMPLE_COMM_VO_StopLayer(g_stHdmiVoAttr.VoLayer);
	SAMPLE_COMM_VO_StopDev(g_stHdmiVoAttr.VoDev);
    
	return s32Ret;
}
#endif
#define HDMI_SUPPORT
int Hst3520D_Sample_StartHdmiVo()
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

int Hst3520D_Sample_StopHdmiVo()
{
    VI_CHN ViChn;
    VO_CHN VoChn;
    VPSS_GRP VpssGrp;
    VPSS_CHN VpssChn_VoSD0 = g_stHdmiVoAttr.VpssChnSDVo;
    HI_S32 i;
	
    Hst3520D_Sample_VoModeGet(&g_stHdmiVoAttr.enVoMode,g_stHdmiVoAttr.u32WndNum);
    printf("[%s %d]***********enVoMode %d u32WndNum %d************\n",__func__,__LINE__,
		g_stHdmiVoAttr.enVoMode,g_stHdmiVoAttr.u32WndNum);
    SAMPLE_COMM_VO_StopChn(g_stHdmiVoAttr.VoLayer, g_stHdmiVoAttr.enVoMode);
	VoChn = 0;
	for(i=0;i<=16;i++)
	{
		if((g_stHdmiVoAttr.u32WndNum>>i)&0x01)
	    {
	        VpssGrp = i;
	        SAMPLE_COMM_VO_UnBindVpss(g_stHdmiVoAttr.VoLayer, VoChn, VpssGrp, g_stHdmiVoAttr.VpssChnSDVo);
			VoChn++;
		}
	}
    SAMPLE_COMM_VO_StopLayer(g_stHdmiVoAttr.VoLayer);
    SAMPLE_COMM_VO_StopDev(g_stHdmiVoAttr.VoDev);

    return HST_SUCCEED;
}

#if 0
HI_S32 Init_hdmi(VO_INTF_SYNC_E enIntfSync)

{
	HI_HDMI_ATTR_S      stAttr;
    HI_HDMI_VIDEO_FMT_E enVideoFmt;
    HI_HDMI_INIT_PARA_S stHdmiPara;

    SAMPLE_COMM_VO_HdmiConvertSync(enIntfSync, &enVideoFmt);

    stHdmiPara.pfnHdmiEventCallback = NULL;
    stHdmiPara.pCallBackArgs = NULL;
    stHdmiPara.enForceMode = HI_HDMI_FORCE_HDMI;
    HI_MPI_HDMI_Init(&stHdmiPara);

    HI_MPI_HDMI_Open(HI_HDMI_ID_0);

    HI_MPI_HDMI_GetAttr(HI_HDMI_ID_0, &stAttr);

    stAttr.bEnableHdmi = HI_TRUE;
    
    stAttr.bEnableVideo = HI_TRUE;
    stAttr.enVideoFmt = enVideoFmt;

    stAttr.enVidOutMode = HI_HDMI_VIDEO_MODE_YCBCR444;
    stAttr.enDeepColorMode = HI_HDMI_DEEP_COLOR_OFF;
    stAttr.bxvYCCMode = HI_FALSE;

    stAttr.bEnableAudio = HI_FALSE;
    stAttr.enSoundIntf = HI_HDMI_SND_INTERFACE_I2S;
    stAttr.bIsMultiChannel = HI_FALSE;

    stAttr.enBitDepth = HI_HDMI_BIT_DEPTH_16;

    stAttr.bEnableAviInfoFrame = HI_TRUE;
    stAttr.bEnableAudInfoFrame = HI_TRUE;
    stAttr.bEnableSpdInfoFrame = HI_FALSE;
    stAttr.bEnableMpegInfoFrame = HI_FALSE;

    stAttr.bDebugFlag = HI_FALSE;          
    stAttr.bHDCPEnable = HI_FALSE;

    stAttr.b3DEnable = HI_FALSE;
    
    HI_MPI_HDMI_SetAttr(HI_HDMI_ID_0, &stAttr);

    HI_MPI_HDMI_Start(HI_HDMI_ID_0);
    
    printf("HDMI start success.\n");
    return HI_SUCCESS;
}
#endif

