#include "Hst3520dVi.h"
#include "sample_comm.h"
#include "Hst3520dChnAdapt.h"

VI_ATTR_S g_stViAttr = {0};
#define VI_GROUP_NUM  (1)
VI_ATTR_S g_stViGroupAttr[VI_GROUP_NUM];

#define VIDEO_FMT_LOSS		(0x0)
#define VIDEO_FMT_SDNT		(0x1)
#define VIDEO_FMT_SDPAL		(0x2)

int Hst3520D_Sample_GetViAttr(VI_ATTR_S *pstViAttr)
{
    if( NULL == pstViAttr )
    {
        return HST_FAILURE;
    }

    memcpy(pstViAttr, &g_stViAttr, sizeof(VI_ATTR_S));
    
    return HST_SUCCEED;
}

int Hst3520D_Sample_SetViAttr(VI_ATTR_S *pstViAttr)
{
    if( NULL == pstViAttr )
    {
        return HST_FAILURE;
    }

    memcpy(&g_stViAttr, pstViAttr, sizeof(VI_ATTR_S));

    return HST_SUCCEED;
}

/*int Hst3520D_Sample_SelfAdaptationViAttr()
{
    int iPhyChn = 0;
    int CamNumD1 = 0;
	int CamNum720P = 0;
	int CamNum1080P = 0;

	unsigned int RegVal_720P = 0;
	unsigned int RegVal_1080P = 0;
	
    Hst3520d_Adapt_Probe_CodecNum();
	Hst3520d_Adapt_Probe_CodecType();
    Hst3520d_Adapt_Probe_VideoInputFMT();

    unsigned int u32MaxChnNum = Hst3520d_Adapt_Get_ChnNum();

	if(AD_TYPE_NVP6134C == Hst3520d_Adapt_Get_CodecType())
	{
		RegVal_720P = 0x12;
		RegVal_1080P = 0x72;
	}

	if(AD_TYPE_NVP6124 == Hst3520d_Adapt_Get_CodecType())
	{
		RegVal_720P = 0x8;
		RegVal_1080P = 0x80;
	}

	for(iPhyChn = 0; iPhyChn < u32MaxChnNum; iPhyChn++)
	{
		if(VIDEO_FMT_SDPAL == Hst3520d_Adapt_Get_ChnFmt(iPhyChn))
		{
			CamNumD1++;
		}

		if(RegVal_720P == Hst3520d_Adapt_Get_ChnFmt(iPhyChn))
		{
			CamNum720P++;
		}

		if(RegVal_1080P == Hst3520d_Adapt_Get_ChnFmt(iPhyChn))
		{
			CamNum1080P++;
		}
	}

	printf("[%s][CamNumD1:%d][CamNum720P:%d][CamNum1080P:%d]\n", __func__,
		CamNumD1, CamNum720P, CamNum1080P);

	if(((CamNumD1 > CamNum720P)&&(CamNumD1 > CamNum1080P))
		|| ((CamNumD1 == CamNum720P)&&(CamNumD1 > CamNum1080P))
		|| ((CamNumD1 == CamNum1080P)&&(CamNumD1 > CamNum720P))
		|| ((CamNumD1 == CamNum1080P == CamNum720P)&&(CamNumD1 > 0)))
	{
		Hst3520D_Sample_DefaultD1ViAttr();
		printf("[%s][Hst3520D_Sample_DefaultD1ViAttr]\n", __func__);
	}
	
	if(((CamNum720P > CamNumD1)&&(CamNum720P >= CamNum1080P))
		||(0 == CamNumD1 == CamNum1080P == CamNum720P))
	{
		Hst3520D_Sample_Default720PViAttr();
		printf("[%s][Hst3520D_Sample_Default720PViAttr]\n", __func__);
	}

	if((CamNum1080P > CamNumD1)&&(CamNum1080P > CamNum720P))
	{
		Hst3520D_Sample_Default1080PViAttr();
		printf("[%s][Hst3520D_Sample_Default1080PViAttr]\n", __func__);
	}

	return 0;
}*/
int Hst3520D_Sample_SelfAdaptationViAttr(CAMERA_INPUT_STATUS_S *pastCameraStatus,CAMERA_RESOLUTION_STATUS_E *paeGroupStat)
{
	int i = 0;
	int CamNumD1 = 0;
	int CamNum720P = 0;
	int CamNum1080P = 0;
	int l_InputNum= 0;

	unsigned int RegVal_720P = 0;
	unsigned int RegVal_1080P = 0;
	
	unsigned int l_u32MaxChnNum = Hst3520d_Adapt_Get_ChnNum();
	CAMERA_RESOLUTION_STATUS_E l_eResolutionStat = CAMERA_RESOLUTION_NULL;
	unsigned char l_arrViDevChnCnt[VI_GROUP_NUM] = {0}; 

    if((NULL == pastCameraStatus) || (NULL == paeGroupStat))
    {
		 return -1;
	}
       
	
	//SAMPLE_COMM_SelfAdaptationAD();
	Hst3520d_Adapt_Probe_CodecType();

	if(AD_TYPE_NVP6134C == Hst3520d_Adapt_Get_CodecType())
	{
		RegVal_720P = 0x12;
		RegVal_1080P = 0x72;
	}

	if(AD_TYPE_NVP6124 == Hst3520d_Adapt_Get_CodecType())
	{
		RegVal_720P = 0x8;
		RegVal_1080P = 0x80;
	}

	//Hst3520D_Sample_GetVideoFMT();
	Hst3520d_Adapt_Probe_VideoInputFMT();
	
	for(i = 0; i < l_u32MaxChnNum; i++)
	{
		if(VIDEO_FMT_SDPAL == Hst3520d_Adapt_Get_ChnFmt(i))
		{
			pastCameraStatus[i].eDetectStat		=CAMERA_DETECT_NORMAL;
			pastCameraStatus[i].eResolutionStat	=CAMERA_RESOLUTION_D1;
			pastCameraStatus[i].eSignalStat		=CAMERA_SIGNAL_CVBS;
			CamNumD1++;
		}
		else if(RegVal_720P == Hst3520d_Adapt_Get_ChnFmt(i))
		{
			pastCameraStatus[i].eDetectStat		=CAMERA_DETECT_NORMAL;
			pastCameraStatus[i].eResolutionStat	=CAMERA_RESOLUTION_720P;
			pastCameraStatus[i].eSignalStat		=CAMERA_SIGNAL_AHD;
			CamNum720P++;
		}

		else if(RegVal_1080P == Hst3520d_Adapt_Get_ChnFmt(i))
		{
			pastCameraStatus[i].eDetectStat		=CAMERA_DETECT_NORMAL;
			pastCameraStatus[i].eResolutionStat	=CAMERA_RESOLUTION_1080P;
			pastCameraStatus[i].eSignalStat		=CAMERA_SIGNAL_AHD;
			CamNum1080P++;
		}
		else
		{
			pastCameraStatus[i].eDetectStat		=CAMERA_DETECT_NULL;	
			pastCameraStatus[i].eResolutionStat	=CAMERA_RESOLUTION_NULL;
			pastCameraStatus[i].eSignalStat		=CAMERA_SIGNAL_NULL;
		}
	    printf("[%s %d]摄像头[%d]类型:%d\n",__func__,__LINE__,i,pastCameraStatus[i].eResolutionStat);
	}

	paeGroupStat[0]=CAMERA_RESOLUTION_NULL;
	paeGroupStat[1]=CAMERA_RESOLUTION_NULL;
	
	l_InputNum=CamNum1080P+CamNumD1+CamNum720P;
	memset(g_stViGroupAttr,0,VI_GROUP_NUM*sizeof(VI_ATTR_S));
	if(l_InputNum)
	{
#if 0	
    	if(HstSdkAL_CheckDevType(DEV_VERSION_BT10))
    	{
    	    l_arrViDevChnCnt[0] = l_arrViDevChnCnt[1] = 2; //通道接口是硬件就定死了的
    		if(pastCameraStatus[0].eResolutionStat!=pastCameraStatus[1].eResolutionStat)
    		{
    			if((CAMERA_RESOLUTION_NULL==pastCameraStatus[0].eResolutionStat||CAMERA_RESOLUTION_NULL==pastCameraStatus[1].eResolutionStat))
    			{
    				paeGroupStat[0]=(CAMERA_RESOLUTION_NULL==pastCameraStatus[0].eResolutionStat)?pastCameraStatus[1].eResolutionStat:pastCameraStatus[0].eResolutionStat;	
    			}
    			else
    			{
    				paeGroupStat[0]=pastCameraStatus[0].eResolutionStat;
    			}
    		}
    		
    		if(pastCameraStatus[0].eResolutionStat==pastCameraStatus[1].eResolutionStat)
    		{
    			paeGroupStat[0]=pastCameraStatus[0].eResolutionStat;
    		}
    		
    		if(pastCameraStatus[2].eResolutionStat!=pastCameraStatus[3].eResolutionStat)
    		{	
    			if(CAMERA_RESOLUTION_NULL==pastCameraStatus[2].eResolutionStat||CAMERA_RESOLUTION_NULL==pastCameraStatus[3].eResolutionStat)
    			{
    				paeGroupStat[1]=(CAMERA_RESOLUTION_NULL==pastCameraStatus[2].eResolutionStat)?pastCameraStatus[3].eResolutionStat:pastCameraStatus[2].eResolutionStat;	
    			}
    			else
    			{
    				paeGroupStat[1]=pastCameraStatus[2].eResolutionStat;
    			}
    		}
    		if(pastCameraStatus[2].eResolutionStat==pastCameraStatus[3].eResolutionStat)
    		{
    			paeGroupStat[1]=pastCameraStatus[2].eResolutionStat;
    		}
    	}
#endif		
    	//else if(HstSdkAL_CheckDevType(DEV_VERSION_BT20))
		{
            //BT20上0-3是Dev0,4-5是DEV1按照最高原则配置
            l_arrViDevChnCnt[0] = 4;//通道接口是硬件就定死了的
            //l_arrViDevChnCnt[1] = 2;
            l_arrViDevChnCnt[1] = 4;//20181102BT20的DEV1是接了4路只是外面拉出2路而已
            l_eResolutionStat = CAMERA_RESOLUTION_NULL;
            for(i = 0;i < 4;i++)
            {
                if(l_eResolutionStat < pastCameraStatus[i].eResolutionStat)
                    l_eResolutionStat = pastCameraStatus[i].eResolutionStat;
            }
            paeGroupStat[0] = l_eResolutionStat;
            
            l_eResolutionStat = CAMERA_RESOLUTION_NULL;
            //for(i = 4;i < 6;i++)
            for(i = 4;i < 8;i++)//20181102BT20的DEV1是接了4路只是外面拉出2路而已
            {
                if(l_eResolutionStat < pastCameraStatus[i].eResolutionStat)
                    l_eResolutionStat = pastCameraStatus[i].eResolutionStat;
            }
            paeGroupStat[1] = l_eResolutionStat;
		}
        //else
        //{
        //    printf("[%s %d]摄像头检测参数Error:当前类型设备参数未配置\n",__func__,__LINE__);
        //    return -1;
       // }
		
		printf("[%s %d]摄像头检测参数:%d,%d\n",__func__,__LINE__,paeGroupStat[0],paeGroupStat[1]);
		for(i=0;i<VI_GROUP_NUM;i++)
		{
			if(CAMERA_RESOLUTION_D1==paeGroupStat[i])
			{
				Hst3520D_Sample_Default_ViGroupAttr(i,PIC_D1,l_arrViDevChnCnt);
			}
			else if(CAMERA_RESOLUTION_720P==paeGroupStat[i])
			{
				Hst3520D_Sample_Default_ViGroupAttr(i,PIC_HD720,l_arrViDevChnCnt);
			}
			else if(CAMERA_RESOLUTION_1080P==paeGroupStat[i])
			{
				Hst3520D_Sample_Default_ViGroupAttr(i,PIC_HD1080,l_arrViDevChnCnt);
			}
		}
	}
	
	return l_InputNum;
}


int Hst3520D_Sample_DefaultD1ViAttr()
{
	HI_U32 CodecNum = Hst3520d_Adapt_Get_CodecNum();

	if(2 == CodecNum)
	{
		g_stViAttr.enViMode   = SAMPLE_VI_MODE_8_D1;
	}
	else
	{
		g_stViAttr.enViMode   = SAMPLE_VI_MODE_4_D1;
	}

	g_stViAttr.u32ViChnCnt    = Hst3520d_Adapt_Get_ChnNum();
	g_stViAttr.s32VpssGrpCnt  = Hst3520d_Adapt_Get_ChnNum();
	g_stViAttr.s32VpssChCnt   = 4;
	g_stViAttr.VpssChnSDVo    = VPSS_CHN0;

	g_stViAttr.enNorm		  = VIDEO_ENCODING_MODE_PAL;
	g_stViAttr.enPicSize	  = PIC_D1;
	g_stViAttr.enPixFmt 	  = SAMPLE_PIXEL_FORMAT;
	g_stViAttr.u32AlignWidth  = SAMPLE_SYS_ALIGN_WIDTH;
	g_stViAttr.enCompFmt	  = COMPRESS_MODE_SEG;

    return HST_SUCCEED;
}

int Hst3520D_Sample_Default720PViAttr()
{
	HI_S32 CodecNum  = Hst3520d_Adapt_Get_CodecNum();

	if(2 == CodecNum)
	{
		g_stViAttr.enViMode   = SAMPLE_VI_MODE_8_720P;
	}
	else
	{
		g_stViAttr.enViMode   = SAMPLE_VI_MODE_4_720P;
	}

	g_stViAttr.u32ViChnCnt    = Hst3520d_Adapt_Get_ChnNum();
	g_stViAttr.s32VpssGrpCnt  = Hst3520d_Adapt_Get_ChnNum();
	g_stViAttr.s32VpssChCnt   = 4;
	g_stViAttr.VpssChnSDVo    = VPSS_CHN0;

	g_stViAttr.enNorm		  = VIDEO_ENCODING_MODE_PAL;
	g_stViAttr.enPicSize	  = PIC_HD720;
	g_stViAttr.enPixFmt 	  = SAMPLE_PIXEL_FORMAT;
	g_stViAttr.u32AlignWidth  = SAMPLE_SYS_ALIGN_WIDTH;
	g_stViAttr.enCompFmt	  = COMPRESS_MODE_SEG;

    return HST_SUCCEED;
}

int Hst3520D_Sample_Default1080PViAttr()
{
    g_stViAttr.u32ViChnCnt    = 4;/*3520v300不支持8路1080P编解码*/
    g_stViAttr.s32VpssGrpCnt  = 4;/*3520v300不支持8路1080P编解码*/
    g_stViAttr.s32VpssChCnt   = 4;
    g_stViAttr.VpssChnSDVo    = VPSS_CHN0;
    g_stViAttr.enViMode       = SAMPLE_VI_MODE_4_1080P;

	g_stViAttr.enNorm		  = VIDEO_ENCODING_MODE_PAL;
	g_stViAttr.enPicSize	  = PIC_HD1080;
	g_stViAttr.enPixFmt 	  = SAMPLE_PIXEL_FORMAT;
	g_stViAttr.u32AlignWidth  = SAMPLE_SYS_ALIGN_WIDTH;
	g_stViAttr.enCompFmt	  = COMPRESS_MODE_SEG;
	
    return HST_SUCCEED;
}

int Hst3520D_Sample_Default_ViGroupAttr(unsigned char u8vigroup,PIC_SIZE_E enPicSize,unsigned char *pu8ChnCnt)
{//20181031 add by tmf 增加通道路数设置以兼容bt20
	if((VI_GROUP_NUM <= u8vigroup) || (NULL == pu8ChnCnt))
		return 0;

	g_stViGroupAttr[u8vigroup].u32ViChnCnt    = pu8ChnCnt[u8vigroup];  //目前BT20只是用4+2的组合
	g_stViGroupAttr[u8vigroup].s32VpssGrpCnt  = pu8ChnCnt[u8vigroup];
	if(pu8ChnCnt[u8vigroup] > 2)
	    g_stViGroupAttr[u8vigroup].s32VpssChCnt   = 4;
	else
	    g_stViGroupAttr[u8vigroup].s32VpssChCnt   = 2;
    //g_stViGroupAttr[u8vigroup].s32VpssChCnt   = 2; //目前只用2个设备而已
    g_stViGroupAttr[u8vigroup].VpssChnSDVo    = VPSS_CHN0;
	g_stViGroupAttr[u8vigroup].enNorm         = VIDEO_ENCODING_MODE_PAL;
	g_stViGroupAttr[u8vigroup].enPicSize      = enPicSize;

	switch(enPicSize)
	{
		case PIC_D1:
		    if(2 == pu8ChnCnt[u8vigroup]) 
			    g_stViGroupAttr[u8vigroup].enViMode = SAMPLE_VI_MODE_2_D1;
			else if(4 == pu8ChnCnt[u8vigroup])
			    g_stViGroupAttr[u8vigroup].enViMode = SAMPLE_VI_MODE_4_D1;
			else //默认按2路处理
			    g_stViGroupAttr[u8vigroup].enViMode = SAMPLE_VI_MODE_2_D1;
			break;
		case PIC_HD720:
		    if(2 == pu8ChnCnt[u8vigroup]) 
                g_stViGroupAttr[u8vigroup].enViMode = SAMPLE_VI_MODE_2_720P;
		    else if(4 == pu8ChnCnt[u8vigroup])
                g_stViGroupAttr[u8vigroup].enViMode = SAMPLE_VI_MODE_4_720P;
		    else //默认按2路处理
			    g_stViGroupAttr[u8vigroup].enViMode = SAMPLE_VI_MODE_2_720P;
			break;
		case PIC_HD1080:
		    if(2 == pu8ChnCnt[u8vigroup]) 
                g_stViGroupAttr[u8vigroup].enViMode = SAMPLE_VI_MODE_2_1080P;
		    else if(4 == pu8ChnCnt[u8vigroup])
                g_stViGroupAttr[u8vigroup].enViMode = SAMPLE_VI_MODE_4_1080P;
		    else //默认按2路处理
			    g_stViGroupAttr[u8vigroup].enViMode = SAMPLE_VI_MODE_2_1080P;
			break;
	}
	g_stViGroupAttr[u8vigroup].enPixFmt       = SAMPLE_PIXEL_FORMAT;
	g_stViGroupAttr[u8vigroup].u32AlignWidth  = SAMPLE_SYS_ALIGN_WIDTH;
	g_stViGroupAttr[u8vigroup].enCompFmt      = COMPRESS_MODE_SEG;

	printf("[%s %d][摄像头组%d]类型:enPicSize=%d,enViMode=%d,u32ViChnCnt=%d\n",__func__,__LINE__,u8vigroup,enPicSize,g_stViGroupAttr[u8vigroup].enViMode,g_stViGroupAttr[u8vigroup].u32ViChnCnt);
    return HST_SUCCEED;
}

HI_S32 VI_WISDOM_NVP6124_CfgByGroup(unsigned char u8Group,VIDEO_NORM_E enVideoMode,SAMPLE_VI_6134_MODE_E enViMode)
{

    int fd, i;
    int video_mode;
	nvp6124_opt_mode optmode;
	nvp6124_chn_mode schnmode;

    int chip_cnt=1;

    fd = open(NVP6124_FILE, O_RDWR);
    if (fd < 0)
    {
        printf("open nvp6124 (%s) fail\n", NVP6124_FILE);
        return -1;
    }

    video_mode = (VIDEO_ENCODING_MODE_PAL == enVideoMode) ? 1 : 0 ;

    switch(enViMode)
    {
        case SAMPLE_VI_MODE_2MUX_D1:
        {
			for(i=0;i<chip_cnt*4;i++)
			{
				schnmode.ch = i;
				schnmode.vformat = video_mode;
				schnmode.chmode = NVP6124_VI_SD;
				if(0==u8Group)
				{
					if(0>schnmode.ch||1<schnmode.ch)
					{
						continue;
					}
				}
				if(1==u8Group)
				{
					if(2>schnmode.ch||3<schnmode.ch)
					{
						continue;
					}
				}
				ioctl(fd, IOC_VDEC_SET_CHNMODE, &schnmode);
			}
			for(i=0;i<chip_cnt;i++)
			{
				optmode.chipsel = i;
				optmode.portmode = NVP6124_OUTMODE_2MUX_SD;
				if(0==u8Group)
				{
					optmode.portsel = 2;
					optmode.chid = 0;
					ioctl(fd, IOC_VDEC_SET_OUTPORTMODE, &optmode);
				}
				if(1==u8Group)
				{
					optmode.portsel = 3;
					optmode.chid = 1;
					ioctl(fd, IOC_VDEC_SET_OUTPORTMODE, &optmode);
				}
			}
			break;
        }
    	case SAMPLE_VI_MODE_6124_960H:
			for(i=0;i<chip_cnt*4;i++)
			{
				schnmode.ch = i;
				schnmode.vformat = video_mode;
				schnmode.chmode = NVP6124_VI_SD;
				ioctl(fd, IOC_VDEC_SET_CHNMODE, &schnmode);
			}
			for(i=0;i<chip_cnt;i++)
			{
				optmode.chipsel = i;
				optmode.portsel = 2;
				optmode.portmode = NVP6124_OUTMODE_4MUX_SD;
				optmode.chid = 0;
				ioctl(fd, IOC_VDEC_SET_OUTPORTMODE, &optmode);
                optmode.chipsel = i;
				optmode.portsel = 3;
				optmode.portmode = NVP6124_OUTMODE_4MUX_SD;
				optmode.chid = 0;
				ioctl(fd, IOC_VDEC_SET_OUTPORTMODE, &optmode);
			}
			break;
		case SAMPLE_VI_MODE_6124_HDX:  
			for(i=0;i<chip_cnt*4;i++)
			{
				schnmode.ch = i;
				schnmode.vformat = video_mode;
				schnmode.chmode = NVP6124_VI_720P_2530;
				ioctl(fd, IOC_VDEC_SET_CHNMODE, &schnmode);
			}
			for(i=0;i<chip_cnt;i++)
			{
				optmode.chipsel = i;
				optmode.portsel = 2;
				optmode.portmode = NVP6124_OUTMODE_4MUX_HD_X;
				optmode.chid = 0;
				ioctl(fd, IOC_VDEC_SET_OUTPORTMODE, &optmode);
			}
            for(i=0;i<chip_cnt;i++)
			{
				optmode.chipsel = i;
				optmode.portsel = 3;
				optmode.portmode = NVP6124_OUTMODE_4MUX_HD_X;
				optmode.chid = 0;
				ioctl(fd, IOC_VDEC_SET_OUTPORTMODE, &optmode);
			}
			break;
		case SAMPLE_VI_MODE_6124_HD:   
			for(i=0;i<chip_cnt*4;i++)
			{
				schnmode.ch = i;
				schnmode.vformat = video_mode;
				schnmode.chmode = NVP6124_VI_720P_2530;
				
				if(0==u8Group)
				{
					if(0>schnmode.ch||1<schnmode.ch)
					{
						continue;
					}
				}
				if(1==u8Group)
				{
					if(2>schnmode.ch||3<schnmode.ch)
					{
						continue;
					}
				}
				printf(" [%s %d] 720p u8Group %d ch %d vformat %d chmode %d \n",__func__,__LINE__,
					u8Group,schnmode.ch,schnmode.vformat,schnmode.chmode);
				ioctl(fd, IOC_VDEC_SET_CHNMODE, &schnmode);
			}

			for(i=0;i<chip_cnt;i++)
			{
				optmode.chipsel = i;
				optmode.portmode = NVP6124_OUTMODE_2MUX_HD;
				if(0==u8Group)
				{
					optmode.portsel = 2;
					optmode.chid = 0;
					ioctl(fd, IOC_VDEC_SET_OUTPORTMODE, &optmode);
				}
				if(1==u8Group)
				{
					optmode.portsel = 3;
					optmode.chid = 1;
					ioctl(fd, IOC_VDEC_SET_OUTPORTMODE, &optmode);
				}
			}
			break;
		case SAMPLE_VI_MODE_6124_FHDX: 
			for(i=0;i<chip_cnt*4;i++)
			{
				schnmode.ch = i;
				schnmode.vformat = video_mode;
				schnmode.chmode = NVP6124_VI_1080P_2530;
				ioctl(fd, IOC_VDEC_SET_CHNMODE, &schnmode);
			}
			for(i=0;i<chip_cnt;i++)
			{
				optmode.chipsel = i;
				optmode.portsel = 2;
				optmode.portmode = NVP6124_OUTMODE_2MUX_FHD_X;
				optmode.chid = 0;
				ioctl(fd, IOC_VDEC_SET_OUTPORTMODE, &optmode);
				optmode.portsel = 3;
				optmode.portmode = NVP6124_OUTMODE_2MUX_FHD_X;
				optmode.chid = 1;
				ioctl(fd, IOC_VDEC_SET_OUTPORTMODE, &optmode);
			}
			break;
		case SAMPLE_VI_MODE_6124_FHD:   
			for(i=0;i<chip_cnt*4;i++)
			{
				schnmode.ch = i;
				schnmode.vformat = video_mode;
				schnmode.chmode = NVP6124_VI_1080P_2530;
				ioctl(fd, IOC_VDEC_SET_CHNMODE, &schnmode);
			}
			for(i=0;i<chip_cnt;i++)
			{
				optmode.chipsel = i;
				optmode.portsel = 2;
				optmode.portmode = NVP6124_OUTMODE_1MUX_FHD;
				optmode.chid = 0;
				ioctl(fd, IOC_VDEC_SET_OUTPORTMODE, &optmode);
				optmode.portsel = 3;
				optmode.portmode = NVP6124_OUTMODE_1MUX_FHD;
				optmode.chid = 1;
				ioctl(fd, IOC_VDEC_SET_OUTPORTMODE, &optmode);
			}
			break;
    	case SAMPLE_VI_MODE_960H_720P_2MUX:
			for(i=0;i<chip_cnt*4;i++)
			{
				schnmode.ch = i;
				schnmode.vformat = video_mode;
				schnmode.chmode = i%2?NVP6124_VI_720P_2530:NVP6124_VI_1920H;
				ioctl(fd, IOC_VDEC_SET_CHNMODE, &schnmode);
			}
			for(i=0;i<chip_cnt;i++)
			{
				optmode.chipsel = i;
				optmode.portsel = 2;
				optmode.portmode = NVP6124_OUTMODE_2MUX_HD;
				optmode.chid = 0;
				ioctl(fd, IOC_VDEC_SET_OUTPORTMODE, &optmode);
				optmode.portsel = 3;
				optmode.portmode = NVP6124_OUTMODE_2MUX_HD;
				optmode.chid = 1;
				ioctl(fd, IOC_VDEC_SET_OUTPORTMODE, &optmode);
			}
			break;
        case SAMPLE_VI_MODE_6124_2MUX_FHD:            
		{
			for(i=0;i<chip_cnt*4;i++)
			{
				schnmode.ch = i;
				schnmode.vformat = video_mode;
				schnmode.chmode = NVP6124_VI_1080P_2530;
			
				if(0==u8Group)
				{
					if(0>schnmode.ch||1<schnmode.ch)
					{
						continue;
					}
				}
				if(1==u8Group)
				{
					if(2>schnmode.ch||3<schnmode.ch)
					{
						continue;
					}
				}
				ioctl(fd, IOC_VDEC_SET_CHNMODE, &schnmode);
				printf("[%s %d] 1080p u8Group %d ch %d vformat %d chmode %d \n",__func__,__LINE__,
					u8Group,schnmode.ch,schnmode.vformat,schnmode.chmode);
			}
			for(i=0;i<chip_cnt;i++)
			{
				optmode.chipsel = i;
				optmode.portmode = NVP6124_OUTMODE_2MUX_FHD;
				if(0==u8Group)
				{
					optmode.portsel = 2;
					optmode.chid = 0;
					ioctl(fd, IOC_VDEC_SET_OUTPORTMODE, &optmode);
				}
				if(1==u8Group)
				{
					optmode.portsel = 3;
					optmode.chid = 1;
					ioctl(fd, IOC_VDEC_SET_OUTPORTMODE, &optmode);
				}
			}

			break;
		}

            
        case SAMPLE_VI_MODE_6124_4MUX_HD:
                #if 0
            for(i=0;i<chip_cnt*4;i++)
            {
                schnmode.ch = i;
                schnmode.vformat = video_mode;
                schnmode.chmode = NVP6124_VI_720P_2530;
                ioctl(fd, IOC_VDEC_SET_CHNMODE, &schnmode);
            }
            for(i=0;i<chip_cnt;i++)
            {
                optmode.chipsel = i;
                optmode.portsel = 2;
                optmode.portmode = NVP6124_OUTMODE_4MUX_HD;
                optmode.chid = 0;
                ioctl(fd, IOC_VDEC_SET_OUTPORTMODE, &optmode);
                optmode.portsel = 3;
                optmode.portmode = NVP6124_OUTMODE_4MUX_HD;
                optmode.chid = 1;
                ioctl(fd, IOC_VDEC_SET_OUTPORTMODE, &optmode);
            }
            #endif
            printf("do not support 4mux 720P\n");
			close(fd);
            return -1;
            break;

		default:
			printf("enViMode %d not supported\n", enViMode);
			break;
    }

    close(fd);
    return 0;
}

HI_S32 VI_WISDOM_NVP6134_CfgByGroup(unsigned char u8Group,VIDEO_NORM_E enVideoMode,SAMPLE_VI_6134_MODE_E enViMode)
{
    int fd, i;
    int video_mode;
	nvp6134_opt_mode optmode;
	nvp6134_chn_mode schnmode;

    int chip_cnt=1;

    fd = open(NVP6134A_FILE, O_RDWR);
    if (fd < 0)
    {
        printf("open nvp6134 (%s) fail\n", NVP6134A_FILE);
        return -1;
    }

    video_mode = (VIDEO_ENCODING_MODE_PAL == enVideoMode) ? 1 : 0 ;

    switch(enViMode)
    {
        case SAMPLE_VI_MODE_6134_2MUX_D1:
        {
			for(i=0;i<chip_cnt*4;i++)
			{
				schnmode.ch = i;
				schnmode.vformat = video_mode;
				schnmode.chmode = NVP6134_VI_720H;
				if(0==u8Group)
				{
					if(0>schnmode.ch||1<schnmode.ch)
					{
						continue;
					}
				}
				if(1==u8Group)
				{
					if(2>schnmode.ch||3<schnmode.ch)
					{
						continue;
					}
				}
				ioctl(fd, IOC_VDEC_SET_CHNMODE, &schnmode);
				//printf(" SAMPLE_VI_MODE_6134_2MUX_D1 u8Group %d ch %d vformat %d chmode %d \n",u8Group,schnmode.ch,schnmode.vformat,schnmode.chmode);
			}
			for(i=0;i<chip_cnt;i++)
			{
				if(0==u8Group)
				{
					optmode.chipsel = i;
					optmode.portsel = 2;
					optmode.portmode = NVP6134_OUTMODE_2MUX_SD;
					optmode.chid = 0;
					ioctl(fd, IOC_VDEC_SET_OUTPORTMODE, &optmode);
				}
				if(1==u8Group)
                {
	                optmode.chipsel = i;
					optmode.portsel = 1;
					optmode.portmode = NVP6134_OUTMODE_2MUX_SD;
					optmode.chid = 1;
					ioctl(fd, IOC_VDEC_SET_OUTPORTMODE, &optmode);
				}
			}
			break;
        }
    	case SAMPLE_VI_MODE_6134_960H:
			for(i=0;i<chip_cnt*4;i++)
			{
				schnmode.ch = i;
				schnmode.vformat = video_mode;
				schnmode.chmode = NVP6134_VI_720H;
				ioctl(fd, IOC_VDEC_SET_CHNMODE, &schnmode);
			}
			for(i=0;i<chip_cnt;i++)
			{
				optmode.chipsel = i;
				optmode.portsel = 2;
				optmode.portmode = NVP6134_OUTMODE_4MUX_SD;
				optmode.chid = 0;
				ioctl(fd, IOC_VDEC_SET_OUTPORTMODE, &optmode);
                optmode.chipsel = i;
				optmode.portsel = 1;
				optmode.portmode = NVP6134_OUTMODE_4MUX_SD;
				optmode.chid = 0;
				ioctl(fd, IOC_VDEC_SET_OUTPORTMODE, &optmode);
			}
			break;
		case SAMPLE_VI_MODE_6134_HDX: 
			for(i=0;i<chip_cnt*4;i++)
			{
				schnmode.ch = i;
				schnmode.vformat = video_mode;
				schnmode.chmode = NVP6134_VI_720P_2530;
				ioctl(fd, IOC_VDEC_SET_CHNMODE, &schnmode);
			}
			for(i=0;i<chip_cnt;i++)
			{
				optmode.chipsel = i;
				optmode.portsel = 2;
				optmode.portmode = NVP6134_OUTMODE_4MUX_HD_X;
				optmode.chid = 0;
				ioctl(fd, IOC_VDEC_SET_OUTPORTMODE, &optmode);
			}
            for(i=0;i<chip_cnt;i++)
			{
				optmode.chipsel = i;
				optmode.portsel = 1;
				optmode.portmode = NVP6134_OUTMODE_4MUX_HD_X;
				optmode.chid = 0;
				ioctl(fd, IOC_VDEC_SET_OUTPORTMODE, &optmode);
			}
			break;
		case SAMPLE_VI_MODE_6134_HD:
			for(i=0;i<chip_cnt*4;i++)
			{
				schnmode.ch = i;
				schnmode.vformat = video_mode;
				schnmode.chmode = NVP6134_VI_720P_2530;
				
				if(0==u8Group)
				{
					if(0>schnmode.ch||1<schnmode.ch)
					{
						continue;
					}
				}
				if(1==u8Group)
				{
					if(2>schnmode.ch||3<schnmode.ch)
					{
						continue;
					}
				}
				ioctl(fd, IOC_VDEC_SET_CHNMODE, &schnmode);
				//printf(" [%s %d] u8Group %d ch %d vformat %d chmode %d \n",__func__,__LINE__,
				//	u8Group,schnmode.ch,schnmode.vformat,schnmode.chmode);
			}
			for(i=0;i<chip_cnt;i++)
			{
				if(0==u8Group)
				{
					optmode.chipsel = i;
					optmode.portsel = 2;
					optmode.portmode = NVP6134_OUTMODE_2MUX_HD;
					optmode.chid = 0;
					ioctl(fd, IOC_VDEC_SET_OUTPORTMODE, &optmode);
				}
				if(1==u8Group)
				{
					optmode.chipsel = i;
					optmode.portsel = 1;
					optmode.portmode = NVP6134_OUTMODE_2MUX_HD;
					optmode.chid = 1;
					ioctl(fd, IOC_VDEC_SET_OUTPORTMODE, &optmode);
				}
			}
			break;
		case SAMPLE_VI_MODE_6134_FHDX:
			for(i=0;i<chip_cnt*4;i++)
			{
				schnmode.ch = i;
				schnmode.vformat = video_mode;
				schnmode.chmode = NVP6134_VI_1080P_2530;
				ioctl(fd, IOC_VDEC_SET_CHNMODE, &schnmode);
			}
			for(i=0;i<chip_cnt;i++)
			{
				optmode.chipsel = i;
				optmode.portsel = 2;
				optmode.portmode = NVP6134_OUTMODE_2MUX_FHD_X;
				optmode.chid = 0;
				ioctl(fd, IOC_VDEC_SET_OUTPORTMODE, &optmode);
				optmode.portsel = 1;
				optmode.portmode = NVP6134_OUTMODE_2MUX_FHD_X;
				optmode.chid = 1;
				ioctl(fd, IOC_VDEC_SET_OUTPORTMODE, &optmode);
			}
			break;
		case SAMPLE_VI_MODE_6134_FHD:
			for(i=0;i<chip_cnt*4;i++)
			{
				schnmode.ch = i;
				schnmode.vformat = video_mode;
				schnmode.chmode = NVP6134_VI_1080P_2530;
				ioctl(fd, IOC_VDEC_SET_CHNMODE, &schnmode);
			}
			for(i=0;i<chip_cnt;i++)
			{
				optmode.chipsel = i;
				optmode.portsel = 2;
				optmode.portmode = NVP6134_OUTMODE_1MUX_FHD;
				optmode.chid = 0;
				ioctl(fd, IOC_VDEC_SET_OUTPORTMODE, &optmode);
				optmode.portsel = 1;
				optmode.portmode = NVP6134_OUTMODE_1MUX_FHD;
				optmode.chid = 1;
				ioctl(fd, IOC_VDEC_SET_OUTPORTMODE, &optmode);
			}
			break;
    	case SAMPLE_VI_MODE_6134_960H_720P_2MUX:
			for(i=0;i<chip_cnt*4;i++)
			{
				schnmode.ch = i;
				schnmode.vformat = video_mode;
				schnmode.chmode = i%2?NVP6134_VI_720P_2530:NVP6134_VI_1920H;
				ioctl(fd, IOC_VDEC_SET_CHNMODE, &schnmode);
			}
			for(i=0;i<chip_cnt;i++)
			{
				optmode.chipsel = i;
				optmode.portsel = 2;
				optmode.portmode = NVP6134_OUTMODE_2MUX_HD;
				optmode.chid = 0;
				ioctl(fd, IOC_VDEC_SET_OUTPORTMODE, &optmode);
				optmode.portsel = 1;
				optmode.portmode = NVP6134_OUTMODE_2MUX_HD;
				optmode.chid = 1;
				ioctl(fd, IOC_VDEC_SET_OUTPORTMODE, &optmode);
			}
			break;
        case SAMPLE_VI_MODE_6134_2MUX_FHD:
            for(i=0;i<chip_cnt*4;i++)
            {
                schnmode.ch = i;
                schnmode.vformat = video_mode;
                schnmode.chmode = NVP6134_VI_1080P_2530;
			
				if(0==u8Group)
				{
					if(0>schnmode.ch||1<schnmode.ch)
					{
						continue;
					}
				}
				if(1==u8Group)
				{
					if(2>schnmode.ch||3<schnmode.ch)
					{
						continue;
					}
				}
                ioctl(fd, IOC_VDEC_SET_CHNMODE, &schnmode);
				//printf("[%s %d] 1080P SET u8Group %d ch %d vformat %d chmode %d \n",__func__,__LINE__,
				//	u8Group,schnmode.ch,schnmode.vformat,schnmode.chmode);
            }
            for(i=0;i<chip_cnt;i++)
            {
           		if(0==u8Group)
              	{
	                optmode.chipsel = i;
	                optmode.portsel = 2;
	                optmode.portmode = NVP6134_OUTMODE_2MUX_FHD;
	                optmode.chid = 0;
	                ioctl(fd, IOC_VDEC_SET_OUTPORTMODE, &optmode);
           	  	}
				if(1==u8Group)
				{
					optmode.chipsel = i;
	                optmode.portsel = 1;
	                optmode.portmode = NVP6134_OUTMODE_2MUX_FHD;
	                optmode.chid = 1;
	                ioctl(fd, IOC_VDEC_SET_OUTPORTMODE, &optmode);
				}
            }
            break;
        /*case SAMPLE_VI_MODE_6134_4MUX_HD:
                #if 0
            for(i=0;i<chip_cnt*4;i++)
            {
                schnmode.ch = i;
                schnmode.vformat = video_mode;
                schnmode.chmode = NVP6124_VI_720P_2530;
                ioctl(fd, IOC_VDEC_SET_CHNMODE, &schnmode);
            }
            for(i=0;i<chip_cnt;i++)
            {
                optmode.chipsel = i;
                optmode.portsel = 2;
                optmode.portmode = NVP6124_OUTMODE_4MUX_HD;
                optmode.chid = 0;
                ioctl(fd, IOC_VDEC_SET_OUTPORTMODE, &optmode);
                optmode.portsel = 3;
                optmode.portmode = NVP6124_OUTMODE_4MUX_HD;
                optmode.chid = 1;
                ioctl(fd, IOC_VDEC_SET_OUTPORTMODE, &optmode);
            }
            #endif
            printf("do not support 4mux 720P!!!\n");
			close(fd);
            return -1;
            break;*/
        case SAMPLE_VI_MODE_6134_4MUX_D1:  //add by tmf20181102
            if(0==u8Group)
            {
                chip_cnt = 1;
                for(i=0;i<chip_cnt*4;i++)
			    {
    				schnmode.ch = i;
    				schnmode.vformat = video_mode;
    				schnmode.chmode = NVP6134_VI_720H;
    				ioctl(fd, IOC_VDEC_SET_CHNMODE, &schnmode);
    				printf(" SAMPLE_VI_MODE_6134_4MUX_D1 u8Group %d ch %d vformat %d chmode %d \n",u8Group,schnmode.ch,schnmode.vformat,schnmode.chmode);
    			}
    			for(i=0;i<chip_cnt;i++)
			    {
                    optmode.chipsel = i;
					optmode.portsel = 1;
					optmode.portmode = NVP6134_OUTMODE_4MUX_SD;
					optmode.chid = 0;
					ioctl(fd, IOC_VDEC_SET_OUTPORTMODE, &optmode);
			    }
            }
            else if(1==u8Group)
            {
                chip_cnt = 2;
                for(i=4;i<chip_cnt*4;i++)
			    {
    				schnmode.ch = i;
    				schnmode.vformat = video_mode;
    				schnmode.chmode = NVP6134_VI_720H;
    				ioctl(fd, IOC_VDEC_SET_CHNMODE, &schnmode);
    				printf(" SAMPLE_VI_MODE_6134_4MUX_D1 u8Group %d ch %d vformat %d chmode %d \n",u8Group,schnmode.ch,schnmode.vformat,schnmode.chmode);
    			}
    			for(i=1;i<chip_cnt;i++)
			    {
                    optmode.chipsel = i;
					optmode.portsel = 2;
					optmode.portmode = NVP6134_OUTMODE_4MUX_SD;
					optmode.chid = 1;
					ioctl(fd, IOC_VDEC_SET_OUTPORTMODE, &optmode);
			    }
            }
            break;
        case SAMPLE_VI_MODE_6134_4MUX_HD://add by tmf20181102
            if(0==u8Group)
            {
                chip_cnt = 1;
                for(i=0;i<chip_cnt*4;i++)
			    {
    				schnmode.ch = i;
    				schnmode.vformat = video_mode;
    				schnmode.chmode = NVP6134_VI_720P_2530;
    				ioctl(fd, IOC_VDEC_SET_CHNMODE, &schnmode);
    				printf(" SAMPLE_VI_MODE_6134_4MUX_720P u8Group %d ch %d vformat %d chmode %d \n",u8Group,schnmode.ch,schnmode.vformat,schnmode.chmode);
    			}
    			for(i=0;i<chip_cnt;i++)
			    {
                    optmode.chipsel = i;
					optmode.portsel = 1;
					optmode.portmode = NVP6134_OUTMODE_4MUX_HD;
					optmode.chid = 0;
					ioctl(fd, IOC_VDEC_SET_OUTPORTMODE, &optmode);
			    }
            }
            else if(1==u8Group)
            {
                chip_cnt = 2;
                for(i=4;i<chip_cnt*4;i++)
			    {
    				schnmode.ch = i;
    				schnmode.vformat = video_mode;
    				schnmode.chmode = NVP6134_VI_720P_2530;
    				ioctl(fd, IOC_VDEC_SET_CHNMODE, &schnmode);
    				printf(" SAMPLE_VI_MODE_6134_4MUX_720P u8Group %d ch %d vformat %d chmode %d \n",u8Group,schnmode.ch,schnmode.vformat,schnmode.chmode);
    			}
    			for(i=1;i<chip_cnt;i++)
			    {
                    optmode.chipsel = i;
					optmode.portsel = 2;
					optmode.portmode = NVP6134_OUTMODE_4MUX_HD;
					optmode.chid = 1;
					ioctl(fd, IOC_VDEC_SET_OUTPORTMODE, &optmode);
			    }
            }
            break;
        case SAMPLE_VI_MODE_6134_4MUX_FHD://add by tmf20181105
            if(0==u8Group)
            {
                chip_cnt = 1;
                for(i=0;i<chip_cnt*4;i++) //通道照样设置但是只有前面两路有效
                {
    				schnmode.ch = i;
    				schnmode.vformat = video_mode;
    				schnmode.chmode = NVP6134_VI_1080P_2530;
    				ioctl(fd, IOC_VDEC_SET_CHNMODE, &schnmode);
    				printf(" SAMPLE_VI_MODE_6134_4MUX_1080P u8Group %d ch %d vformat %d chmode %d \n",u8Group,schnmode.ch,schnmode.vformat,schnmode.chmode);
    			}
    			for(i=0;i<chip_cnt;i++)
			    {
                    optmode.chipsel = i;
					optmode.portsel = 1;
					optmode.portmode = NVP6134_OUTMODE_2MUX_FHD; //按照2路1080P复合输入处理BT20的设置，芯片不支持1080P的4路符合输入20181106
					optmode.chid = 0;
					ioctl(fd, IOC_VDEC_SET_OUTPORTMODE, &optmode);
			    }
            }
            else if(1==u8Group)
            {
                chip_cnt = 2;
                for(i=4;i<chip_cnt*4;i++)//通道照样设置但是只有前面两路有效
                {
    				schnmode.ch = i;
    				schnmode.vformat = video_mode;
    				schnmode.chmode = NVP6134_VI_1080P_2530;
    				ioctl(fd, IOC_VDEC_SET_CHNMODE, &schnmode);
    				printf(" SAMPLE_VI_MODE_6134_4MUX_1080P u8Group %d ch %d vformat %d chmode %d \n",u8Group,schnmode.ch,schnmode.vformat,schnmode.chmode);
    			}
    			for(i=1;i<chip_cnt;i++)
			    {
                    optmode.chipsel = i;
					optmode.portsel = 2;
					optmode.portmode = NVP6134_OUTMODE_2MUX_FHD;//按照2路1080P复合输入处理BT20的设置，芯片不支持1080P的4路符合输入20181106
					optmode.chid = 1;
					ioctl(fd, IOC_VDEC_SET_OUTPORTMODE, &optmode);
			    }
            }
            break;
		default:
			printf("enViMode %d not supported!!!\n", enViMode);
			break;
    }

    close(fd);
    return 0;
}

int Hst3520_VbSet_ByGroup(VB_CONF_S *pstVbConf)
{
	unsigned int l_u32BlkSize=0;
 	unsigned int l_u32VpssGrpCnt =0;
    //HI_S32 s32Ret = HI_SUCCESS;
    int l_s32Pool=0;

    //add by tmf,20181106,重新调整内存分配
    unsigned char l_u8DevPram[4] = {8,8,0,0}; //Dev内存分配系数
    
    if(PIC_HD1080 == g_stViGroupAttr[0].enPicSize)
    {
        SAMPLE_PRT("Dev0输入类型:1080P\n");
        l_u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(VIDEO_ENCODING_MODE_PAL,PIC_HD1080,SAMPLE_PIXEL_FORMAT,SAMPLE_SYS_ALIGN_WIDTH,COMPRESS_MODE_SEG);
    }
    else if(PIC_HD720==g_stViGroupAttr[0].enPicSize)
    {
        SAMPLE_PRT("Dev0输入类型:720P\n");
        l_u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(VIDEO_ENCODING_MODE_PAL,PIC_HD720,SAMPLE_PIXEL_FORMAT,SAMPLE_SYS_ALIGN_WIDTH,COMPRESS_MODE_SEG);
    }
    else //其余默认按照D1处理
    {
        SAMPLE_PRT("Dev0输入类型:D1\n");
        l_u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(VIDEO_ENCODING_MODE_PAL,PIC_D1,SAMPLE_PIXEL_FORMAT,SAMPLE_SYS_ALIGN_WIDTH,COMPRESS_MODE_SEG);
    }
    pstVbConf->astCommPool[l_s32Pool].u32BlkSize = l_u32BlkSize;
    if(PIC_HD1080 == g_stViGroupAttr[0].enPicSize) //最多支持2路复合输入
        pstVbConf->astCommPool[l_s32Pool].u32BlkCnt = ((g_stViGroupAttr[0].u32ViChnCnt > 2) ? 2:g_stViGroupAttr[0].u32ViChnCnt) * l_u8DevPram[0];	
    else
        pstVbConf->astCommPool[l_s32Pool].u32BlkCnt = g_stViGroupAttr[0].u32ViChnCnt * l_u8DevPram[0];
    printf("[%s %d]Dev[%d]内存分配:u32BlkSize=%ld,u32BlkCnt=%ld\n",__func__,__LINE__,l_s32Pool,pstVbConf->astCommPool[l_s32Pool].u32BlkSize,pstVbConf->astCommPool[l_s32Pool].u32BlkCnt);
    l_s32Pool++;

    if(PIC_HD1080 == g_stViGroupAttr[1].enPicSize)
    {
        SAMPLE_PRT("Dev1输入类型:1080P\n");
        l_u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(VIDEO_ENCODING_MODE_PAL,PIC_HD1080,SAMPLE_PIXEL_FORMAT,SAMPLE_SYS_ALIGN_WIDTH,COMPRESS_MODE_SEG);
    }
    else if(PIC_HD720==g_stViGroupAttr[1].enPicSize)
    {
        SAMPLE_PRT("Dev1输入类型:720P\n");
        l_u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(VIDEO_ENCODING_MODE_PAL,PIC_HD720,SAMPLE_PIXEL_FORMAT,SAMPLE_SYS_ALIGN_WIDTH,COMPRESS_MODE_SEG);
    }
    else //其余默认按照D1处理
    {
        SAMPLE_PRT("Dev1输入类型:D1\n");
        l_u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(VIDEO_ENCODING_MODE_PAL,PIC_D1,SAMPLE_PIXEL_FORMAT,SAMPLE_SYS_ALIGN_WIDTH,COMPRESS_MODE_SEG);
    }
    pstVbConf->astCommPool[l_s32Pool].u32BlkSize = l_u32BlkSize;
    //pstVbConf->astCommPool[l_s32Pool].u32BlkCnt = g_stViGroupAttr[1].u32ViChnCnt * l_u8DevPram[1];
    pstVbConf->astCommPool[l_s32Pool].u32BlkCnt = ((g_stViGroupAttr[1].u32ViChnCnt > 2) ? 2:g_stViGroupAttr[1].u32ViChnCnt) * l_u8DevPram[1]; //Dev目前的BT10和BT20只使用了2路
    printf("[%s %d]Dev[%d]内存分配:u32BlkSize=%ld,u32BlkCnt=%ld\n",__func__,__LINE__,l_s32Pool,pstVbConf->astCommPool[l_s32Pool].u32BlkSize,pstVbConf->astCommPool[l_s32Pool].u32BlkCnt);
    l_s32Pool++;
    //Dev2,3没有使用直接按照最低配置
    SAMPLE_PRT("Dev2和Dev3暂时未使用\n");
    l_u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(VIDEO_ENCODING_MODE_PAL,PIC_CIF,SAMPLE_PIXEL_FORMAT,SAMPLE_SYS_ALIGN_WIDTH,COMPRESS_MODE_SEG);
	pstVbConf->astCommPool[l_s32Pool].u32BlkSize = l_u32BlkSize;
	pstVbConf->astCommPool[l_s32Pool].u32BlkCnt = 4 * l_u8DevPram[2];	
    printf("[%s %d]Dev[%d]内存分配:u32BlkSize=%ld,u32BlkCnt=%ld\n",__func__,__LINE__,l_s32Pool,pstVbConf->astCommPool[l_s32Pool].u32BlkSize,pstVbConf->astCommPool[l_s32Pool].u32BlkCnt);
	l_s32Pool++;
	l_u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(VIDEO_ENCODING_MODE_PAL,PIC_CIF,SAMPLE_PIXEL_FORMAT,SAMPLE_SYS_ALIGN_WIDTH,COMPRESS_MODE_SEG);
	pstVbConf->astCommPool[l_s32Pool].u32BlkSize = l_u32BlkSize;
	pstVbConf->astCommPool[l_s32Pool].u32BlkCnt = 4 * l_u8DevPram[3];	
    printf("[%s %d]Dev[%d]内存分配:u32BlkSize=%ld,u32BlkCnt=%ld\n",__func__,__LINE__,l_s32Pool,pstVbConf->astCommPool[l_s32Pool].u32BlkSize,pstVbConf->astCommPool[l_s32Pool].u32BlkCnt);
	l_s32Pool++;
	
/*	SIZE_S stSize;

	//组合1 PIC_HD1080 PIC_HD720
	if((PIC_HD1080==g_stViGroupAttr[0].enPicSize&&PIC_HD720==g_stViGroupAttr[1].enPicSize)||
		(PIC_HD720==g_stViGroupAttr[0].enPicSize&&PIC_HD1080==g_stViGroupAttr[1].enPicSize))
	{
		SAMPLE_PRT("混合组合(PIC_HD720 PIC_HD1080)!!!!\n");
		l_u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(VIDEO_ENCODING_MODE_PAL,PIC_HD1080,SAMPLE_PIXEL_FORMAT,SAMPLE_SYS_ALIGN_WIDTH,COMPRESS_MODE_SEG);
		pstVbConf->astCommPool[l_s32Pool].u32BlkSize = l_u32BlkSize;
		pstVbConf->astCommPool[l_s32Pool].u32BlkCnt = (g_stViGroupAttr[0].u32ViChnCnt+g_stViGroupAttr[1].u32ViChnCnt)*7;	
		l_s32Pool++;
	    l_u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(VIDEO_ENCODING_MODE_PAL,PIC_HD720,SAMPLE_PIXEL_FORMAT,SAMPLE_SYS_ALIGN_WIDTH,COMPRESS_MODE_SEG);
		pstVbConf->astCommPool[l_s32Pool].u32BlkSize = l_u32BlkSize;
		pstVbConf->astCommPool[l_s32Pool].u32BlkCnt = (g_stViGroupAttr[0].u32ViChnCnt+g_stViGroupAttr[1].u32ViChnCnt)*7;	
		l_s32Pool++;	
		l_u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(VIDEO_ENCODING_MODE_PAL,PIC_HD720,SAMPLE_PIXEL_FORMAT,SAMPLE_SYS_ALIGN_WIDTH,COMPRESS_MODE_SEG);
		pstVbConf->astCommPool[l_s32Pool].u32BlkSize = l_u32BlkSize;
		pstVbConf->astCommPool[l_s32Pool].u32BlkCnt = (g_stViGroupAttr[0].u32ViChnCnt+g_stViGroupAttr[1].u32ViChnCnt)*7;	
		l_s32Pool++;
		l_u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(VIDEO_ENCODING_MODE_PAL,PIC_CIF,SAMPLE_PIXEL_FORMAT,SAMPLE_SYS_ALIGN_WIDTH,COMPRESS_MODE_SEG);
		pstVbConf->astCommPool[l_s32Pool].u32BlkSize = l_u32BlkSize;
		pstVbConf->astCommPool[l_s32Pool].u32BlkCnt = (g_stViGroupAttr[0].u32ViChnCnt+g_stViGroupAttr[1].u32ViChnCnt)*8;	
		l_s32Pool++;

	}
	//组合2 PIC_HD1080 PIC_D1
	if((PIC_HD1080==g_stViGroupAttr[0].enPicSize&&PIC_D1==g_stViGroupAttr[1].enPicSize)||
	(PIC_D1==g_stViGroupAttr[0].enPicSize&&PIC_HD1080==g_stViGroupAttr[1].enPicSize))
	{
		SAMPLE_PRT("混合组合(PIC_HD1080 PIC_D1)!!!!\n");
		l_u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(VIDEO_ENCODING_MODE_PAL,PIC_HD1080,SAMPLE_PIXEL_FORMAT,SAMPLE_SYS_ALIGN_WIDTH,COMPRESS_MODE_SEG);
		pstVbConf->astCommPool[l_s32Pool].u32BlkSize = l_u32BlkSize;
		pstVbConf->astCommPool[l_s32Pool].u32BlkCnt = (g_stViGroupAttr[0].u32ViChnCnt+g_stViGroupAttr[1].u32ViChnCnt)*6;	
		l_s32Pool++;
	    l_u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(VIDEO_ENCODING_MODE_PAL,PIC_HD1080,SAMPLE_PIXEL_FORMAT,SAMPLE_SYS_ALIGN_WIDTH,COMPRESS_MODE_SEG);
		pstVbConf->astCommPool[l_s32Pool].u32BlkSize = l_u32BlkSize;
		pstVbConf->astCommPool[l_s32Pool].u32BlkCnt = (g_stViGroupAttr[0].u32ViChnCnt+g_stViGroupAttr[1].u32ViChnCnt)*6;	
		l_s32Pool++;	
		l_u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(VIDEO_ENCODING_MODE_PAL,PIC_HD720,SAMPLE_PIXEL_FORMAT,SAMPLE_SYS_ALIGN_WIDTH,COMPRESS_MODE_SEG);
		pstVbConf->astCommPool[l_s32Pool].u32BlkSize = l_u32BlkSize;
		pstVbConf->astCommPool[l_s32Pool].u32BlkCnt = (g_stViGroupAttr[0].u32ViChnCnt+g_stViGroupAttr[1].u32ViChnCnt)*6;	
		l_s32Pool++;
		l_u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(VIDEO_ENCODING_MODE_PAL,PIC_CIF,SAMPLE_PIXEL_FORMAT,SAMPLE_SYS_ALIGN_WIDTH,COMPRESS_MODE_SEG);
		pstVbConf->astCommPool[l_s32Pool].u32BlkSize = l_u32BlkSize;
		pstVbConf->astCommPool[l_s32Pool].u32BlkCnt = (g_stViGroupAttr[0].u32ViChnCnt+g_stViGroupAttr[1].u32ViChnCnt)*8;	
		l_s32Pool++;

	}
	//混合组合3 PIC_HD720 PIC_D1
	if((PIC_HD720==g_stViGroupAttr[0].enPicSize&&PIC_D1==g_stViGroupAttr[1].enPicSize)||
	(PIC_D1==g_stViGroupAttr[0].enPicSize&&PIC_HD720==g_stViGroupAttr[1].enPicSize))
	{
		SAMPLE_PRT("混合组合(PIC_HD720 PIC_D1)!!!!\n");
		l_u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(VIDEO_ENCODING_MODE_PAL,PIC_HD720,SAMPLE_PIXEL_FORMAT,SAMPLE_SYS_ALIGN_WIDTH,COMPRESS_MODE_SEG);
		pstVbConf->astCommPool[l_s32Pool].u32BlkSize = l_u32BlkSize;
		pstVbConf->astCommPool[l_s32Pool].u32BlkCnt = (g_stViGroupAttr[0].u32ViChnCnt+g_stViGroupAttr[1].u32ViChnCnt)*8;	
        printf("[%s %d]Dev[%d]内存分配:u32BlkSize=%ld,u32BlkCnt=%ld\n",__func__,__LINE__,l_s32Pool,pstVbConf->astCommPool[l_s32Pool].u32BlkSize,pstVbConf->astCommPool[l_s32Pool].u32BlkCnt);
		l_s32Pool++;
	    l_u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(VIDEO_ENCODING_MODE_PAL,PIC_HD720,SAMPLE_PIXEL_FORMAT,SAMPLE_SYS_ALIGN_WIDTH,COMPRESS_MODE_SEG);
		pstVbConf->astCommPool[l_s32Pool].u32BlkSize = l_u32BlkSize;
		pstVbConf->astCommPool[l_s32Pool].u32BlkCnt = (g_stViGroupAttr[0].u32ViChnCnt+g_stViGroupAttr[1].u32ViChnCnt)*8;	
        printf("[%s %d]Dev[%d]内存分配:u32BlkSize=%ld,u32BlkCnt=%ld\n",__func__,__LINE__,l_s32Pool,pstVbConf->astCommPool[l_s32Pool].u32BlkSize,pstVbConf->astCommPool[l_s32Pool].u32BlkCnt);
		l_s32Pool++;	
		l_u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(VIDEO_ENCODING_MODE_PAL,PIC_D1,SAMPLE_PIXEL_FORMAT,SAMPLE_SYS_ALIGN_WIDTH,COMPRESS_MODE_SEG);
		pstVbConf->astCommPool[l_s32Pool].u32BlkSize = l_u32BlkSize;
		pstVbConf->astCommPool[l_s32Pool].u32BlkCnt = (g_stViGroupAttr[0].u32ViChnCnt+g_stViGroupAttr[1].u32ViChnCnt)*8;	
        printf("[%s %d]Dev[%d]内存分配:u32BlkSize=%ld,u32BlkCnt=%ld\n",__func__,__LINE__,l_s32Pool,pstVbConf->astCommPool[l_s32Pool].u32BlkSize,pstVbConf->astCommPool[l_s32Pool].u32BlkCnt);
		l_s32Pool++;
		l_u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(VIDEO_ENCODING_MODE_PAL,PIC_CIF,SAMPLE_PIXEL_FORMAT,SAMPLE_SYS_ALIGN_WIDTH,COMPRESS_MODE_SEG);
		pstVbConf->astCommPool[l_s32Pool].u32BlkSize = l_u32BlkSize;
		pstVbConf->astCommPool[l_s32Pool].u32BlkCnt = (g_stViGroupAttr[0].u32ViChnCnt+g_stViGroupAttr[1].u32ViChnCnt)*8;	
        printf("[%s %d]Dev[%d]内存分配:u32BlkSize=%ld,u32BlkCnt=%ld\n",__func__,__LINE__,l_s32Pool,pstVbConf->astCommPool[l_s32Pool].u32BlkSize,pstVbConf->astCommPool[l_s32Pool].u32BlkCnt);
		l_s32Pool++;
	}
	//组合6 PIC_D1*4
	if(PIC_D1==g_stViGroupAttr[0].enPicSize&&PIC_D1==g_stViGroupAttr[1].enPicSize)
	{
		SAMPLE_PRT("双组组合(PIC_D1*4)!!!!\n");
		l_u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(VIDEO_ENCODING_MODE_PAL,PIC_D1,SAMPLE_PIXEL_FORMAT,SAMPLE_SYS_ALIGN_WIDTH,COMPRESS_MODE_SEG);
		pstVbConf->astCommPool[l_s32Pool].u32BlkSize = l_u32BlkSize;
		pstVbConf->astCommPool[l_s32Pool].u32BlkCnt = (g_stViGroupAttr[0].u32ViChnCnt+g_stViGroupAttr[1].u32ViChnCnt)*8;	
		l_s32Pool++;
	    l_u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(VIDEO_ENCODING_MODE_PAL,PIC_D1,SAMPLE_PIXEL_FORMAT,SAMPLE_SYS_ALIGN_WIDTH,COMPRESS_MODE_SEG);
		pstVbConf->astCommPool[l_s32Pool].u32BlkSize = l_u32BlkSize;
		pstVbConf->astCommPool[l_s32Pool].u32BlkCnt = (g_stViGroupAttr[0].u32ViChnCnt+g_stViGroupAttr[1].u32ViChnCnt)*8;	
		l_s32Pool++;	
		l_u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(VIDEO_ENCODING_MODE_PAL,PIC_D1,SAMPLE_PIXEL_FORMAT,SAMPLE_SYS_ALIGN_WIDTH,COMPRESS_MODE_SEG);
		pstVbConf->astCommPool[l_s32Pool].u32BlkSize = l_u32BlkSize;
		pstVbConf->astCommPool[l_s32Pool].u32BlkCnt = (g_stViGroupAttr[0].u32ViChnCnt+g_stViGroupAttr[1].u32ViChnCnt)*8;	
		l_s32Pool++;
		l_u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(VIDEO_ENCODING_MODE_PAL,PIC_CIF,SAMPLE_PIXEL_FORMAT,SAMPLE_SYS_ALIGN_WIDTH,COMPRESS_MODE_SEG);
		pstVbConf->astCommPool[l_s32Pool].u32BlkSize = l_u32BlkSize;
		pstVbConf->astCommPool[l_s32Pool].u32BlkCnt = (g_stViGroupAttr[0].u32ViChnCnt+g_stViGroupAttr[1].u32ViChnCnt)*8;	
		l_s32Pool++;

	}

	//组合4 PIC_HD720*4
	if(PIC_HD720==g_stViGroupAttr[0].enPicSize&&PIC_HD720==g_stViGroupAttr[1].enPicSize)
	{
		SAMPLE_PRT("双组组合(PIC_HD720*4)!!!!\n");
		l_u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(VIDEO_ENCODING_MODE_PAL,PIC_HD720,SAMPLE_PIXEL_FORMAT,SAMPLE_SYS_ALIGN_WIDTH,COMPRESS_MODE_SEG);
		pstVbConf->astCommPool[l_s32Pool].u32BlkSize = l_u32BlkSize;
		pstVbConf->astCommPool[l_s32Pool].u32BlkCnt = (g_stViGroupAttr[0].u32ViChnCnt+g_stViGroupAttr[1].u32ViChnCnt)*8;	
		l_s32Pool++;
	    l_u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(VIDEO_ENCODING_MODE_PAL,PIC_HD720,SAMPLE_PIXEL_FORMAT,SAMPLE_SYS_ALIGN_WIDTH,COMPRESS_MODE_SEG);
		pstVbConf->astCommPool[l_s32Pool].u32BlkSize = l_u32BlkSize;
		pstVbConf->astCommPool[l_s32Pool].u32BlkCnt = (g_stViGroupAttr[0].u32ViChnCnt+g_stViGroupAttr[1].u32ViChnCnt)*8;	
		l_s32Pool++;	
		l_u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(VIDEO_ENCODING_MODE_PAL,PIC_D1,SAMPLE_PIXEL_FORMAT,SAMPLE_SYS_ALIGN_WIDTH,COMPRESS_MODE_SEG);
		pstVbConf->astCommPool[l_s32Pool].u32BlkSize = l_u32BlkSize;
		pstVbConf->astCommPool[l_s32Pool].u32BlkCnt = (g_stViGroupAttr[0].u32ViChnCnt+g_stViGroupAttr[1].u32ViChnCnt)*8;	
		l_s32Pool++;
		l_u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(VIDEO_ENCODING_MODE_PAL,PIC_CIF,SAMPLE_PIXEL_FORMAT,SAMPLE_SYS_ALIGN_WIDTH,COMPRESS_MODE_SEG);
		pstVbConf->astCommPool[l_s32Pool].u32BlkSize = l_u32BlkSize;
		pstVbConf->astCommPool[l_s32Pool].u32BlkCnt = (g_stViGroupAttr[0].u32ViChnCnt+g_stViGroupAttr[1].u32ViChnCnt)*8;	
		l_s32Pool++;

	}
	//组合5 PIC_HD1080*4
	if(PIC_HD1080==g_stViGroupAttr[0].enPicSize&&PIC_HD1080==g_stViGroupAttr[1].enPicSize)
	{
		SAMPLE_PRT("双组组合(PIC_HD1080*4)!!!!\n");
		l_u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(VIDEO_ENCODING_MODE_PAL,PIC_HD1080,SAMPLE_PIXEL_FORMAT,SAMPLE_SYS_ALIGN_WIDTH,COMPRESS_MODE_SEG);
		pstVbConf->astCommPool[l_s32Pool].u32BlkSize = l_u32BlkSize;
		pstVbConf->astCommPool[l_s32Pool].u32BlkCnt = (g_stViGroupAttr[0].u32ViChnCnt+g_stViGroupAttr[1].u32ViChnCnt)*6;	
		l_s32Pool++;
	    l_u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(VIDEO_ENCODING_MODE_PAL,PIC_HD1080,SAMPLE_PIXEL_FORMAT,SAMPLE_SYS_ALIGN_WIDTH,COMPRESS_MODE_SEG);
		pstVbConf->astCommPool[l_s32Pool].u32BlkSize = l_u32BlkSize;
		pstVbConf->astCommPool[l_s32Pool].u32BlkCnt = (g_stViGroupAttr[0].u32ViChnCnt+g_stViGroupAttr[1].u32ViChnCnt)*6;	
		l_s32Pool++;	
		l_u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(VIDEO_ENCODING_MODE_PAL,PIC_HD720,SAMPLE_PIXEL_FORMAT,SAMPLE_SYS_ALIGN_WIDTH,COMPRESS_MODE_SEG);
		pstVbConf->astCommPool[l_s32Pool].u32BlkSize = l_u32BlkSize;
		pstVbConf->astCommPool[l_s32Pool].u32BlkCnt = (g_stViGroupAttr[0].u32ViChnCnt+g_stViGroupAttr[1].u32ViChnCnt)*6;	
		l_s32Pool++;
		l_u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(VIDEO_ENCODING_MODE_PAL,PIC_CIF,SAMPLE_PIXEL_FORMAT,SAMPLE_SYS_ALIGN_WIDTH,COMPRESS_MODE_SEG);
		pstVbConf->astCommPool[l_s32Pool].u32BlkSize = l_u32BlkSize;
		pstVbConf->astCommPool[l_s32Pool].u32BlkCnt = (g_stViGroupAttr[0].u32ViChnCnt+g_stViGroupAttr[1].u32ViChnCnt)*8;	
		l_s32Pool++;

	}


	//组合5 PIC_HD1080*2
	if((PIC_HD1080==g_stViGroupAttr[0].enPicSize&&0==g_stViGroupAttr[1].enPicSize)||
		(0==g_stViGroupAttr[0].enPicSize&&PIC_HD1080==g_stViGroupAttr[1].enPicSize))
	{
	
		SAMPLE_PRT("单组组合(PIC_HD1080*2)!!!!\n");
		l_u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(VIDEO_ENCODING_MODE_PAL,PIC_HD1080,SAMPLE_PIXEL_FORMAT,SAMPLE_SYS_ALIGN_WIDTH,COMPRESS_MODE_SEG);
		pstVbConf->astCommPool[l_s32Pool].u32BlkSize = l_u32BlkSize;
		pstVbConf->astCommPool[l_s32Pool].u32BlkCnt = (g_stViGroupAttr[0].u32ViChnCnt+g_stViGroupAttr[1].u32ViChnCnt)*10;	
		l_s32Pool++;
		l_u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(VIDEO_ENCODING_MODE_PAL,PIC_HD1080,SAMPLE_PIXEL_FORMAT,SAMPLE_SYS_ALIGN_WIDTH,COMPRESS_MODE_SEG);
		pstVbConf->astCommPool[l_s32Pool].u32BlkSize = l_u32BlkSize;
		pstVbConf->astCommPool[l_s32Pool].u32BlkCnt = (g_stViGroupAttr[0].u32ViChnCnt+g_stViGroupAttr[1].u32ViChnCnt)*10;	
		l_s32Pool++;	
		l_u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(VIDEO_ENCODING_MODE_PAL,PIC_HD720,SAMPLE_PIXEL_FORMAT,SAMPLE_SYS_ALIGN_WIDTH,COMPRESS_MODE_SEG);
		pstVbConf->astCommPool[l_s32Pool].u32BlkSize = l_u32BlkSize;
		pstVbConf->astCommPool[l_s32Pool].u32BlkCnt = (g_stViGroupAttr[0].u32ViChnCnt+g_stViGroupAttr[1].u32ViChnCnt)*8;	
		l_s32Pool++;
		l_u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(VIDEO_ENCODING_MODE_PAL,PIC_HD720,SAMPLE_PIXEL_FORMAT,SAMPLE_SYS_ALIGN_WIDTH,COMPRESS_MODE_SEG);
		pstVbConf->astCommPool[l_s32Pool].u32BlkSize = l_u32BlkSize;
		pstVbConf->astCommPool[l_s32Pool].u32BlkCnt = (g_stViGroupAttr[0].u32ViChnCnt+g_stViGroupAttr[1].u32ViChnCnt)*8;	
		l_s32Pool++;
		l_u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(VIDEO_ENCODING_MODE_PAL,PIC_CIF,SAMPLE_PIXEL_FORMAT,SAMPLE_SYS_ALIGN_WIDTH,COMPRESS_MODE_SEG);
		pstVbConf->astCommPool[l_s32Pool].u32BlkSize = l_u32BlkSize;
		pstVbConf->astCommPool[l_s32Pool].u32BlkCnt = (g_stViGroupAttr[0].u32ViChnCnt+g_stViGroupAttr[1].u32ViChnCnt)*16;	
		l_s32Pool++;

	}
	//组合4 PIC_HD720*2
	if((PIC_HD720==g_stViGroupAttr[0].enPicSize&&0==g_stViGroupAttr[1].enPicSize)||
		(0==g_stViGroupAttr[0].enPicSize&&PIC_HD720==g_stViGroupAttr[1].enPicSize))
	{
	
		SAMPLE_PRT("单组组合(PIC_HD720*2)!!!!\n");
		l_u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(VIDEO_ENCODING_MODE_PAL,PIC_HD720,SAMPLE_PIXEL_FORMAT,SAMPLE_SYS_ALIGN_WIDTH,COMPRESS_MODE_SEG);
		pstVbConf->astCommPool[l_s32Pool].u32BlkSize = l_u32BlkSize;
		pstVbConf->astCommPool[l_s32Pool].u32BlkCnt = (g_stViGroupAttr[0].u32ViChnCnt+g_stViGroupAttr[1].u32ViChnCnt)*12;	
		l_s32Pool++;
		l_u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(VIDEO_ENCODING_MODE_PAL,PIC_HD720,SAMPLE_PIXEL_FORMAT,SAMPLE_SYS_ALIGN_WIDTH,COMPRESS_MODE_SEG);
		pstVbConf->astCommPool[l_s32Pool].u32BlkSize = l_u32BlkSize;
		pstVbConf->astCommPool[l_s32Pool].u32BlkCnt = (g_stViGroupAttr[0].u32ViChnCnt+g_stViGroupAttr[1].u32ViChnCnt)*12;	
		l_s32Pool++;	
		l_u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(VIDEO_ENCODING_MODE_PAL,PIC_D1,SAMPLE_PIXEL_FORMAT,SAMPLE_SYS_ALIGN_WIDTH,COMPRESS_MODE_SEG);
		pstVbConf->astCommPool[l_s32Pool].u32BlkSize = l_u32BlkSize;
		pstVbConf->astCommPool[l_s32Pool].u32BlkCnt = (g_stViGroupAttr[0].u32ViChnCnt+g_stViGroupAttr[1].u32ViChnCnt)*12;	
		l_s32Pool++;
		l_u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(VIDEO_ENCODING_MODE_PAL,PIC_D1,SAMPLE_PIXEL_FORMAT,SAMPLE_SYS_ALIGN_WIDTH,COMPRESS_MODE_SEG);
		pstVbConf->astCommPool[l_s32Pool].u32BlkSize = l_u32BlkSize;
		pstVbConf->astCommPool[l_s32Pool].u32BlkCnt = (g_stViGroupAttr[0].u32ViChnCnt+g_stViGroupAttr[1].u32ViChnCnt)*12;	
		l_s32Pool++;
		l_u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(VIDEO_ENCODING_MODE_PAL,PIC_CIF,SAMPLE_PIXEL_FORMAT,SAMPLE_SYS_ALIGN_WIDTH,COMPRESS_MODE_SEG);
		pstVbConf->astCommPool[l_s32Pool].u32BlkSize = l_u32BlkSize;
		pstVbConf->astCommPool[l_s32Pool].u32BlkCnt = (g_stViGroupAttr[0].u32ViChnCnt+g_stViGroupAttr[1].u32ViChnCnt)*16;	
		l_s32Pool++;
	}
	//组合5 PIC_D1*2
	if((PIC_D1==g_stViGroupAttr[0].enPicSize&&0==g_stViGroupAttr[1].enPicSize)||
		(0==g_stViGroupAttr[0].enPicSize&&PIC_D1==g_stViGroupAttr[1].enPicSize))
	{
	
		SAMPLE_PRT("单组组合(PIC_D1*2)!!!!\n");
		l_u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(VIDEO_ENCODING_MODE_PAL,PIC_D1,SAMPLE_PIXEL_FORMAT,SAMPLE_SYS_ALIGN_WIDTH,COMPRESS_MODE_SEG);
		pstVbConf->astCommPool[l_s32Pool].u32BlkSize = l_u32BlkSize;
		pstVbConf->astCommPool[l_s32Pool].u32BlkCnt = (g_stViGroupAttr[0].u32ViChnCnt+g_stViGroupAttr[1].u32ViChnCnt)*16;	
		l_s32Pool++;
		l_u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(VIDEO_ENCODING_MODE_PAL,PIC_D1,SAMPLE_PIXEL_FORMAT,SAMPLE_SYS_ALIGN_WIDTH,COMPRESS_MODE_SEG);
		pstVbConf->astCommPool[l_s32Pool].u32BlkSize = l_u32BlkSize;
		pstVbConf->astCommPool[l_s32Pool].u32BlkCnt = (g_stViGroupAttr[0].u32ViChnCnt+g_stViGroupAttr[1].u32ViChnCnt)*16;	
		l_s32Pool++;	
		l_u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(VIDEO_ENCODING_MODE_PAL,PIC_CIF,SAMPLE_PIXEL_FORMAT,SAMPLE_SYS_ALIGN_WIDTH,COMPRESS_MODE_SEG);
		pstVbConf->astCommPool[l_s32Pool].u32BlkSize = l_u32BlkSize;
		pstVbConf->astCommPool[l_s32Pool].u32BlkCnt = (g_stViGroupAttr[0].u32ViChnCnt+g_stViGroupAttr[1].u32ViChnCnt)*16;	
		l_s32Pool++;
		l_u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(VIDEO_ENCODING_MODE_PAL,PIC_CIF,SAMPLE_PIXEL_FORMAT,SAMPLE_SYS_ALIGN_WIDTH,COMPRESS_MODE_SEG);
		pstVbConf->astCommPool[l_s32Pool].u32BlkSize = l_u32BlkSize;
		pstVbConf->astCommPool[l_s32Pool].u32BlkCnt = (g_stViGroupAttr[0].u32ViChnCnt+g_stViGroupAttr[1].u32ViChnCnt)*16;	
		l_s32Pool++;
	}
*/
}

int Hst3520D_Sample_Start_GroupVi(unsigned char u8vigroup)
{
    VPSS_GRP_ATTR_S stGrpAttr;
    
    VPSS_GRP VpssGrp;
    VPSS_CHN_MODE_S stVpssChnMode;

	unsigned int l_u32BlkSize=0;
 	unsigned int l_u32VpssGrpCnt =0;
    VB_CONF_S stVbConf;
    HI_S32 s32Ret = HI_SUCCESS;
    int i=0;
	
	SIZE_S stSize;
	if(VI_GROUP_NUM <= u8vigroup)
	{
		return 0;
	}
	/******************************************
    step  1: init variable 
    ******************************************/	
    printf("[%s]分组设置启动视频输入step1\n", __func__);
    memset(&stVbConf,0,sizeof(VB_CONF_S));
	
	unsigned char StartGroup=0;
	stVbConf.u32MaxPoolCnt = 256;
	Hst3520_VbSet_ByGroup(&stVbConf);

    /******************************************
    step 2: mpp system init. 
    ******************************************/
    printf("[%s]分组设置启动视频输入step2\n", __func__);
    s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
    if (HI_SUCCESS != s32Ret)
    {
        printf("[%s] SAMPLE_COMM_SYS_Init failed with %d!\n", __func__, s32Ret);
        goto END_VI_START_0;
    }
	printf("-----------------[%s:%d]--------------------mpp system init ok!! \n", __func__, __LINE__);
	
	for(i=0;i<VI_GROUP_NUM;i++)
	{
		  /******************************************
		  step 3: start vi dev & chn
		  ******************************************/
		  if(0 < g_stViGroupAttr[i].u32ViChnCnt)
		  {
		      printf("\n[%s]启动分组设置传入参数[%d]:enViMode=0x%x,enNorm=0x%x,u32ViChnCnt=%d\n", __func__, i,g_stViGroupAttr[i].enViMode,g_stViGroupAttr[i].enNorm,g_stViGroupAttr[i].u32ViChnCnt);
			  s32Ret = SAMPLE_COMM_VI_StartGroup(i,g_stViGroupAttr[i].enViMode,g_stViGroupAttr[i].enNorm);
			  if (HI_SUCCESS != s32Ret)
			  {
				  printf("[%s] SAMPLE_COMM_VI_Start failed with %d!\n", __func__, s32Ret);
				  goto END_VI_START_0;
			  }
	 
		  }
	}
	printf("-----------------[%s:%d]--------------------step 3: start vi dev & chn  ok!! \n", __func__, __LINE__);

    for(i=0;i<VI_GROUP_NUM;i++)
	{
		/******************************************
	    step 3: start vi dev & chn
	    ******************************************/
	    printf("[%s]分组设置启动视频输入step3\n", __func__);
		unsigned char TureGroup=0;
		StartGroup=i;
		TureGroup=i;  
		if(AD_TYPE_NVP6134C == Hst3520d_Adapt_Get_CodecType())
	    {
	        //if(HstSdkAL_CheckDevType(DEV_VERSION_BT10)) //add by tmf20181031这个在BT10中要反过来
	    	//    TureGroup=VI_GROUP_NUM-i-1; //顺序需要反过来
		}
	   	/******************************************
		step 4: start vpss and vi bind vpss
		******************************************/
		printf("[%s]分组设置启动视频输入step4\n", __func__);
		if(0<g_stViGroupAttr[i].u32ViChnCnt)
		{
			stSize.u32Height=0;
			stSize.u32Width=0;
			s32Ret = SAMPLE_COMM_SYS_GetPicSize(g_stViGroupAttr[i].enNorm,g_stViGroupAttr[i].enPicSize, &stSize);
			if (HI_SUCCESS != s32Ret)
			{
				printf("[%s] SAMPLE_COMM_SYS_GetPicSize failed with %d!\n",  __func__, s32Ret);
				goto END_VI_START_1;
			}			
			memset(&stGrpAttr,0,sizeof(VPSS_GRP_ATTR_S));
			stGrpAttr.u32MaxW = stSize.u32Width;
			stGrpAttr.u32MaxH = stSize.u32Height;
			stGrpAttr.bNrEn = HI_TRUE;
			stGrpAttr.enDieMode = VPSS_DIE_MODE_NODIE;
			stGrpAttr.enPixFmt =  g_stViGroupAttr[i].enPixFmt;
			printf("[%s %d][Group %d ][u32Height:%d][u32Width:%d]\n", __func__,__LINE__, i,stSize.u32Height, stSize.u32Width);
			printf("[%s %d]TureGroup %d\n", __func__,__LINE__,TureGroup);
			s32Ret = SAMPLE_COMM_VPSS_StartByGroup(TureGroup,g_stViGroupAttr[i].s32VpssGrpCnt, &stSize, g_stViGroupAttr[i].s32VpssChCnt, &stGrpAttr);
			if (HI_SUCCESS != s32Ret)
			{
				printf("[%s] SAMPLE_COMM_VPSS_Start failed with %d!\n", __func__, s32Ret);
				goto END_VI_START_1;
			}
			printf("-----------------[%s:%d]--------------------step 4: start vpss ok!!\n", __func__, __LINE__);
		}
		
	   	/******************************************
		step 5: bind vpss
		******************************************/
		printf("[%s]分组设置启动视频输入step5:TureGroup=%d,u32ViChnCnt=%d,enViMode=%d\n", __func__,TureGroup,g_stViGroupAttr[TureGroup].u32ViChnCnt,g_stViGroupAttr[TureGroup].enViMode);
		if(0<g_stViGroupAttr[TureGroup].u32ViChnCnt)
		{
			s32Ret = SAMPLE_COMM_VI_BindVpssByGroup(TureGroup,g_stViGroupAttr[TureGroup].enViMode,VPSS_CHN0);
			if (HI_SUCCESS != s32Ret)
			{
				printf("[%s] SAMPLE_COMM_VI_BindVpss failed with %d!\n", __func__, s32Ret);
				goto END_VI_START_2;
			}
			printf("-----------------[%s:%d]--------------------step 5: bind vpss ok!! \n", __func__, __LINE__);
		}
		
	}

	return HST_SUCCEED;

END_VI_START_2:	//vpss stop
    SAMPLE_COMM_VPSS_Stop(g_stViAttr.s32VpssGrpCnt, g_stViAttr.s32VpssChCnt);
END_VI_START_1:	//vi stop
    SAMPLE_COMM_VI_Stop(g_stViAttr.enViMode);
END_VI_START_0: //system exit
    SAMPLE_COMM_SYS_Exit();

    return s32Ret;

}

int Hst3520D_Sample_StartVi()
{
    int i = 0;
    VB_CONF_S stVbConf;
    VPSS_GRP_ATTR_S stGrpAttr;
    
    VPSS_GRP VpssGrp;
    VPSS_CHN_MODE_S stVpssChnMode;

    HI_U32 u32BlkSize;
    HI_S32 s32Ret = HI_SUCCESS;
    SIZE_S stSize;
 	HI_S32 s32VpssGrpCnt =0;
    /******************************************
    step  1: init variable 
    ******************************************/	
    memset(&stVbConf,0,sizeof(VB_CONF_S));

    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(g_stViAttr.enNorm,\
    g_stViAttr.enPicSize, g_stViAttr.enPixFmt, g_stViAttr.u32AlignWidth, g_stViAttr.enCompFmt);
	switch(g_stViAttr.enPicSize)
	{
        case PIC_QCIF:
        case PIC_CIF:
        case PIC_D1:
			s32VpssGrpCnt=24;
            break;
        case PIC_960H:
        case PIC_2CIF:
        case PIC_QVGA:    /* 320 * 240 */
        case PIC_VGA:     /* 640 * 480 */
        case PIC_XGA:     /* 1024 * 768 */
        case PIC_SXGA:    /* 1400 * 1050 */
        case PIC_UXGA:    /* 1600 * 1200 */
        case PIC_QXGA:    /* 2048 * 1536 */
        case PIC_WVGA:    /* 854 * 480 */
        case PIC_WSXGA:   /* 1680 * 1050 */
        case PIC_WUXGA:   /* 1920 * 1200 */
        case PIC_WQXGA:   /* 2560 * 1600 */
        case PIC_HD720:   /* 1280 * 720 */
			s32VpssGrpCnt=24;
            break;
        case PIC_HD1080:  /* 1920 * 1080 */
			s32VpssGrpCnt=8;           
			break;
        default:
            return HI_FAILURE;
    }

    stVbConf.u32MaxPoolCnt = 256;
    stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[0].u32BlkCnt = s32VpssGrpCnt;

    stVbConf.astCommPool[1].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[1].u32BlkCnt = s32VpssGrpCnt;

    stVbConf.astCommPool[2].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[2].u32BlkCnt = s32VpssGrpCnt;

    stVbConf.astCommPool[3].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[3].u32BlkCnt = s32VpssGrpCnt*2;

	SAMPLE_PRT("u32BlkSize: %d \n", u32BlkSize);
	SAMPLE_PRT("s32VpssGrpCnt: %d \n", s32VpssGrpCnt);
	SAMPLE_PRT("g_stViAttr.s32VpssGrpCnt: %d \n", g_stViAttr.s32VpssGrpCnt);

    /******************************************
    step 2: mpp system init. 
    ******************************************/
    s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
    if (HI_SUCCESS != s32Ret)
    {
        printf("[%s] SAMPLE_COMM_SYS_Init failed with %d!\n", __func__, s32Ret);
        goto END_VI_START_0;
    }

    /******************************************
    step 3: start vi dev & chn
    ******************************************/
    s32Ret = SAMPLE_COMM_VI_Start(g_stViAttr.enViMode, g_stViAttr.enNorm,
        Hst3520d_Adapt_Get_CodecType(),
        Hst3520d_Adapt_Get_CodecNum());
    if (HI_SUCCESS != s32Ret)
    {
        printf("[%s] SAMPLE_COMM_VI_Start failed with %d!\n", __func__, s32Ret);
        goto END_VI_START_0;
    }

    /******************************************
    step 4: start vpss and vi bind vpss
    ******************************************/
    s32Ret = SAMPLE_COMM_SYS_GetPicSize(g_stViAttr.enNorm, g_stViAttr.enPicSize, &stSize);
    if (HI_SUCCESS != s32Ret)
    {
        printf("[%s] SAMPLE_COMM_SYS_GetPicSize failed with %d!\n",  __func__, s32Ret);
        goto END_VI_START_1;
    }

    printf("---------[%s][u32Height:%d][u32Width:%d]---------\n", __func__, stSize.u32Height, stSize.u32Width);
    
    memset(&stGrpAttr,0,sizeof(VPSS_GRP_ATTR_S));
    stGrpAttr.u32MaxW = stSize.u32Width;
    stGrpAttr.u32MaxH = stSize.u32Height;
    stGrpAttr.bNrEn = HI_TRUE;
    stGrpAttr.enDieMode = VPSS_DIE_MODE_NODIE;
    stGrpAttr.enPixFmt =  g_stViAttr.enPixFmt;

    s32Ret = SAMPLE_COMM_VPSS_Start(g_stViAttr.s32VpssGrpCnt, &stSize, g_stViAttr.s32VpssChCnt, &stGrpAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("[%s] SAMPLE_COMM_VPSS_Start failed with %d!\n", __func__, s32Ret);
        goto END_VI_START_1;
    }

    for (i=0; i<g_stViAttr.s32VpssGrpCnt; i++)
    {
        VpssGrp = i;

        s32Ret = HI_MPI_VPSS_GetChnMode(VpssGrp, g_stViAttr.VpssChnSDVo, &stVpssChnMode);
        if (HI_SUCCESS != s32Ret)
        {
            printf("[%s] HI_MPI_VPSS_GetChnMode failed with %d!\n", __func__, s32Ret);
            goto END_VI_START_2;
        }

        memset(&stVpssChnMode,0,sizeof(VPSS_CHN_MODE_S));
        stVpssChnMode.enChnMode = VPSS_CHN_MODE_USER;
        stVpssChnMode.u32Width  = stSize.u32Width;
        stVpssChnMode.u32Height = stSize.u32Height;
        stVpssChnMode.stFrameRate.s32DstFrmRate = -1;
        stVpssChnMode.stFrameRate.s32SrcFrmRate = -1;
        stVpssChnMode.enPixelFormat  = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
        stVpssChnMode.enCompressMode = COMPRESS_MODE_NONE;

        s32Ret = HI_MPI_VPSS_SetChnMode(VpssGrp, g_stViAttr.VpssChnSDVo, &stVpssChnMode);
        if (HI_SUCCESS != s32Ret)
        {
            printf("[%s] HI_MPI_VPSS_SetChnMode failed with %d!\n", __func__, s32Ret);
            goto END_VI_START_2;
        }
    }

    s32Ret = SAMPLE_COMM_VI_BindVpss(g_stViAttr.enViMode, g_stViAttr.VpssChnSDVo,
        Hst3520d_Adapt_Get_CodecType());
    if (HI_SUCCESS != s32Ret)
    {
        printf("[%s] SAMPLE_COMM_VI_BindVpss failed with %d!\n", __func__, s32Ret);
        goto END_VI_START_2;
    }

    return HST_SUCCEED;

END_VI_START_2:	//vpss stop
    SAMPLE_COMM_VPSS_Stop(g_stViAttr.s32VpssGrpCnt, g_stViAttr.s32VpssChCnt);
END_VI_START_1:	//vi stop
    SAMPLE_COMM_VI_Stop(g_stViAttr.enViMode);
END_VI_START_0: //system exit
    SAMPLE_COMM_SYS_Exit();

    return s32Ret;
}

int Hst3520D_Sample_StopVi()
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = SAMPLE_COMM_VI_UnBindVpss(g_stViAttr.enViMode, g_stViAttr.VpssChnSDVo);
    if (HI_SUCCESS != s32Ret)
    {
        printf("[%s] SAMPLE_COMM_VI_UnBindVpss failed with %d!\n", __func__, s32Ret);
        return HST_FAILURE;
    }

    s32Ret = SAMPLE_COMM_VPSS_Stop(g_stViAttr.s32VpssGrpCnt, g_stViAttr.s32VpssChCnt);
    if (HI_SUCCESS != s32Ret)
    {
        printf("[%s] SAMPLE_COMM_VPSS_Stop failed with %d!\n", __func__, s32Ret);
        return HST_FAILURE;
    }

    s32Ret = SAMPLE_COMM_VI_Stop(g_stViAttr.enViMode);
    if (HI_SUCCESS != s32Ret)
    {
        printf("[%s] SAMPLE_COMM_VI_Stop failed with %d!\n", __func__, s32Ret);
        return HST_FAILURE;
    }

    SAMPLE_COMM_SYS_Exit();

    return HST_SUCCEED;
}


int Hst3520D_Sample_GetVideoState(unsigned int *pu32VideoState)
{
	if(AD_TYPE_NVP6134C == Hst3520d_Adapt_Get_CodecType())
	{
	    //printf("[%s,%d]:获取摄像头状态NVP6134C\n",__FILE__,__LINE__);
		return VI_WISDOM_NVP6134_GetVideoState(pu32VideoState, Hst3520d_Adapt_Get_CodecNum());
	}

	if(AD_TYPE_NVP6124 == Hst3520d_Adapt_Get_CodecType())
	{
	    //printf("[%s,%d]:获取摄像头状态NVP6124\n",__FILE__,__LINE__);
		return VI_WISDOM_NVP6124_GetVideoState(pu32VideoState);
	}
}

int Hst3520D_Sample_GetCurVencSzie(VIDEO_CHN_E enVeChnNo)
{
    int Rst = -1;
    VIDEO_CHN_CFG_S stChnCfg; //20181029/add by tmf/直接采用映射表

    Rst = Hst3520d_Adapt_Get_VideoChnCfg(enVeChnNo,&stChnCfg);
    if((-1 == Rst) )//|| HstSdkAL_CheckDevType(DEV_VERSION_NULL))
    {
        printf("[%s]获取自适应视频大小失败:enVeChnNo=%d,Rst=%ld\n",__func__, enVeChnNo,Rst);
        return Rst;
    }

   // if(HstSdkAL_CheckDevType(DEV_VERSION_BT10))
   // {
   //     if(stChnCfg.s32ViChn < 2)
   //         Rst = g_stViGroupAttr[0].enPicSize;
   //     else
   //         Rst = g_stViGroupAttr[1].enPicSize;
   // }
   // else if(HstSdkAL_CheckDevType(DEV_VERSION_BT20))
    {
        if(stChnCfg.s32ViChn < 4)
            Rst = g_stViGroupAttr[0].enPicSize;
        else
            Rst = g_stViGroupAttr[1].enPicSize;
    }
   // else
    {
      //  Rst = -1;
     //   printf("[%s]获取自适应视频大小错误[设备类型未知]:enVeChnNo=%d,Rst=%ld\n",__func__, enVeChnNo,Rst);

    }
    printf("[%s]获取自适应视频大小:enVeChnNo=%d,Rst=%ld\n",__func__, enVeChnNo,Rst);

    return Rst;
}
