#include "Hst3520dChnAdapt.h"


#define ARRAY_SIZE(_A) (sizeof(_A) / sizeof((_A)[0]))


static unsigned int gs_u32CodecNum = 1;/*默认双CODEC芯片级联*/
static unsigned int gs_arrVideoFmt[16] = {0};/*各通道视频输入的制式*/
static AD_TYPE_EN gs_enCodecType = AD_TYPE_NVP6134C;/*默认CODEC芯片6134C*/


//const VIDEO_CHN_CFG_S g_astVideoChnCfgTbl[] =
const VIDEO_CHN_CFG_S g_astVideoChnCfgBt10Tbl[] =
{
    {AHD_CH1_STOR_V, 0, 0, 0, VPSS_MAIN_STREAM_CHN, 3},
    {AHD_CH2_STOR_V, 1, 1, 1, VPSS_MAIN_STREAM_CHN, 2},
    {AHD_CH3_STOR_V, 2, 2, 2, VPSS_MAIN_STREAM_CHN, 1},
    {AHD_CH4_STOR_V, 3, 3, 3, VPSS_MAIN_STREAM_CHN, 0},
    {AHD_CH5_STOR_V, 4, 4, 4, VPSS_MAIN_STREAM_CHN, 4},
    {AHD_CH6_STOR_V, 5, 5, 5, VPSS_MAIN_STREAM_CHN, 5},
    {AHD_CH7_STOR_V, 6, 6, 6, VPSS_MAIN_STREAM_CHN, 6},
    {AHD_CH8_STOR_V, 7, 7, 7, VPSS_MAIN_STREAM_CHN, 7},

    {AHD_CH1_SEND_V, 0, 8,  0, VPSS_MAIN_STREAM_CHN, 3},
    {AHD_CH2_SEND_V, 1, 9,  1, VPSS_MAIN_STREAM_CHN, 2},
    {AHD_CH3_SEND_V, 2, 10, 2, VPSS_MAIN_STREAM_CHN, 1},
    {AHD_CH4_SEND_V, 3, 11, 3, VPSS_MAIN_STREAM_CHN, 0},
    {AHD_CH5_SEND_V, 4, 12, 4, VPSS_MAIN_STREAM_CHN, 4},
    {AHD_CH6_SEND_V, 5, 13, 5, VPSS_MAIN_STREAM_CHN, 5},
    {AHD_CH7_SEND_V, 6, 14, 6, VPSS_MAIN_STREAM_CHN, 6},
    {AHD_CH8_SEND_V, 7, 15, 7, VPSS_MAIN_STREAM_CHN, 7},

    {AHD_CH1_SNAP, 0, 16, 0, VPSS_MAIN_STREAM_CHN, 3},
    {AHD_CH2_SNAP, 1, 17, 1, VPSS_MAIN_STREAM_CHN, 2},
    {AHD_CH3_SNAP, 2, 18, 2, VPSS_MAIN_STREAM_CHN, 1},
    {AHD_CH4_SNAP, 3, 19, 3, VPSS_MAIN_STREAM_CHN, 0},
    {AHD_CH5_SNAP, 4, 20, 4, VPSS_MAIN_STREAM_CHN, 4},
    {AHD_CH6_SNAP, 5, 21, 5, VPSS_MAIN_STREAM_CHN, 5},
    {AHD_CH7_SNAP, 6, 22, 6, VPSS_MAIN_STREAM_CHN, 6},
    {AHD_CH8_SNAP, 7, 23, 7, VPSS_MAIN_STREAM_CHN, 7},
};

const VIDEO_CHN_CFG_S g_astVideoChnCfgBt20Tbl[] =
{
    {AHD_CH1_STOR_V, 0, 0, 0, VPSS_MAIN_STREAM_CHN, 0},
    {AHD_CH2_STOR_V, 1, 1, 1, VPSS_MAIN_STREAM_CHN, 1},
    {AHD_CH3_STOR_V, 2, 2, 2, VPSS_MAIN_STREAM_CHN, 2},
    {AHD_CH4_STOR_V, 3, 3, 3, VPSS_MAIN_STREAM_CHN, 3},
    {AHD_CH5_STOR_V, 4, 4, 4, VPSS_MAIN_STREAM_CHN, 4},
    {AHD_CH6_STOR_V, 5, 5, 5, VPSS_MAIN_STREAM_CHN, 5},
    {AHD_CH7_STOR_V, 6, 6, 6, VPSS_MAIN_STREAM_CHN, 6},
    {AHD_CH8_STOR_V, 7, 7, 7, VPSS_MAIN_STREAM_CHN, 7},

    {AHD_CH1_SEND_V, 0, 8,  0, VPSS_MAIN_STREAM_CHN, 0},
    {AHD_CH2_SEND_V, 1, 9,  1, VPSS_MAIN_STREAM_CHN, 1},
    {AHD_CH3_SEND_V, 2, 10, 2, VPSS_MAIN_STREAM_CHN, 2},
    {AHD_CH4_SEND_V, 3, 11, 3, VPSS_MAIN_STREAM_CHN, 3},
    {AHD_CH5_SEND_V, 4, 12, 4, VPSS_MAIN_STREAM_CHN, 4},
    {AHD_CH6_SEND_V, 5, 13, 5, VPSS_MAIN_STREAM_CHN, 5},
    {AHD_CH7_SEND_V, 6, 14, 6, VPSS_MAIN_STREAM_CHN, 6},
    {AHD_CH8_SEND_V, 7, 15, 7, VPSS_MAIN_STREAM_CHN, 7},

    {AHD_CH1_SNAP, 0, 16, 0, VPSS_MAIN_STREAM_CHN, 0},
    {AHD_CH2_SNAP, 1, 17, 1, VPSS_MAIN_STREAM_CHN, 1},
    {AHD_CH3_SNAP, 2, 18, 2, VPSS_MAIN_STREAM_CHN, 2},
    {AHD_CH4_SNAP, 3, 19, 3, VPSS_MAIN_STREAM_CHN, 3},
    {AHD_CH5_SNAP, 4, 20, 4, VPSS_MAIN_STREAM_CHN, 4},
    {AHD_CH6_SNAP, 5, 21, 5, VPSS_MAIN_STREAM_CHN, 5},
    {AHD_CH7_SNAP, 6, 22, 6, VPSS_MAIN_STREAM_CHN, 6},
    {AHD_CH8_SNAP, 7, 23, 7, VPSS_MAIN_STREAM_CHN, 7},
};

//const AUDIO_CHN_CFG_S g_astAudioChnCfgTbl[] = 
const AUDIO_CHN_CFG_S g_astAudioChnCfgBt10Tbl[] = 
{
    {AHD_CH1_STOR_A, 0, 0, 3},
    {AHD_CH2_STOR_A, 1, 1, 2},
    {AHD_CH3_STOR_A, 2, 2, 1},
    {AHD_CH4_STOR_A, 3, 3, 0},
    {AHD_CH5_STOR_A, 4, 4, 4},
    {AHD_CH6_STOR_A, 5, 5, 5},
    {AHD_CH7_STOR_A, 6, 6, 6},
    {AHD_CH8_STOR_A, 7, 7, 7},

    {AHD_CH1_SEND_A, 0, 8,  3},
    {AHD_CH2_SEND_A, 1, 9,  2},
    {AHD_CH3_SEND_A, 2, 10, 1},
    {AHD_CH4_SEND_A, 3, 11, 0},
    {AHD_CH5_SEND_A, 4, 12, 4},
    {AHD_CH6_SEND_A, 5, 13, 5},
    {AHD_CH7_SEND_A, 6, 14, 6},
    {AHD_CH8_SEND_A, 7, 15, 7},

    {AHD_CH1_TAPE, 0, 16, 3},
    {AHD_CH2_TAPE, 1, 17, 2},
    {AHD_CH3_TAPE, 2, 18, 1},
    {AHD_CH4_TAPE, 3, 19, 0},
    {AHD_CH5_TAPE, 4, 20, 4},
    {AHD_CH6_TAPE, 5, 21, 5},
    {AHD_CH7_TAPE, 6, 22, 6},
    {AHD_CH8_TAPE, 7, 23, 7},
    
    {MIC_CH1_TALK, 4, 48},
};

const AUDIO_CHN_CFG_S g_astAudioChnCfgBt20Tbl[] = 
{
    {AHD_CH1_STOR_A, 0, 0, 0},
    {AHD_CH2_STOR_A, 1, 1, 1},
    {AHD_CH3_STOR_A, 2, 2, 2},
    {AHD_CH4_STOR_A, 3, 3, 3},
    {AHD_CH5_STOR_A, 4, 4, 4},
    {AHD_CH6_STOR_A, 5, 5, 5},
    {AHD_CH7_STOR_A, 6, 6, 6},
    {AHD_CH8_STOR_A, 7, 7, 7},

    {AHD_CH1_SEND_A, 0, 8,  0},
    {AHD_CH2_SEND_A, 1, 9,  1},
    {AHD_CH3_SEND_A, 2, 10, 2},
    {AHD_CH4_SEND_A, 3, 11, 3},
    {AHD_CH5_SEND_A, 4, 12, 4},
    {AHD_CH6_SEND_A, 5, 13, 5},
    {AHD_CH7_SEND_A, 6, 14, 6},
    {AHD_CH8_SEND_A, 7, 15, 7},

    {AHD_CH1_TAPE, 0, 16, 0},
    {AHD_CH2_TAPE, 1, 17, 1},
    {AHD_CH3_TAPE, 2, 18, 2},
    {AHD_CH4_TAPE, 3, 19, 3},
    {AHD_CH5_TAPE, 4, 20, 4},
    {AHD_CH6_TAPE, 5, 21, 5},
    {AHD_CH7_TAPE, 6, 22, 6},
    {AHD_CH8_TAPE, 7, 23, 7},

    {MIC_CH1_TALK, 4, 48},
};


HI_S32 Hst3520d_Adapt_Probe_CodecType()
{
   // if (0==access(NVP6124_FILE, F_OK))
   // { 
   //     gs_enCodecType = AD_TYPE_NVP6124;
   // } 
    //else if (0==access(NVP6134A_FILE, F_OK))
    //{ 
        gs_enCodecType = AD_TYPE_NVP6134C;
    //}

    printf("[%s:%d]:gs_enCodecType: %#x\n", __func__, __LINE__, gs_enCodecType);

    return 0;
}

HI_S32 Hst3520d_Adapt_Probe_CodecNum()
{
    unsigned int tempNum = 0;

    int	fd = open(NVP6134A_FILE, O_RDWR);
	if (fd < 0)
	{
		printf("[%s:%d] open nvp6134 (%s) fail\n", __func__, __LINE__, NVP6134A_FILE);
		return -1;
	}

	ioctl(fd, NVP6134_GET_CODEC_NUM, &tempNum);

	gs_u32CodecNum = tempNum;
	
	printf("[%s:%d] gs_u32CodecNum:%d\n", __func__, __LINE__, gs_u32CodecNum);

	close(fd);

    return 0;
}

HI_S32 Hst3520d_Adapt_Probe_Nvp6124FMT(nvp6124_input_videofmt *pstVideofmt)
{
	if(NULL == pstVideofmt)
	{
		printf("[%s:%d] Para Error!\n", __func__, __LINE__);
		return -1;
	}

	int fd = -1;

	fd = open(NVP6124_FILE, O_RDWR);
	if (fd < 0)
	{
		printf("[%s:%d] open nvp6124 (%s) fail\n", __func__, __LINE__, NVP6124_FILE);
		return -1;
	}

	ioctl(fd, IOC_VDEC_GET_INPUT_VIDEO_FMT, pstVideofmt);

	//printf("[%s:%d] [video_state : %#x]\n", __func__, __LINE__, *pu32VideoState);

	close(fd);
	
	return 0;
}

HI_S32 Hst3520d_Adapt_Probe_Nvp6134FMT(nvp6124_input_videofmt *pstVideofmt)
{
	if(NULL == pstVideofmt)
	{
		printf("[%s:%d] Para Error!\n", __func__, __LINE__);
		return -1;
	}

	int fd = -1;

	fd = open(NVP6134A_FILE, O_RDWR);
	if (fd < 0)
	{
		printf("[%s:%d] open nvp6134 (%s) fail\n", __func__, __LINE__, NVP6134A_FILE);
		return -1;
	}

	ioctl(fd, IOC_VDEC_GET_INPUT_VIDEO_FMT, pstVideofmt);

	//printf("[%s:%d] [pstVideofmt : %#x]\n", __func__, __LINE__, *pstVideofmt);
	close(fd);
	
	return 0;
}

HI_S32 Hst3520d_Adapt_Probe_VideoInputFMT()
{
	nvp6124_input_videofmt stInputFMT = {0};

    Hst3520d_Adapt_Probe_CodecType();

	if(AD_TYPE_NVP6134C == Hst3520d_Adapt_Get_CodecType())
	{
		Hst3520d_Adapt_Probe_Nvp6134FMT(&stInputFMT);
	}

	//if(AD_TYPE_NVP6124 == Hst3520d_Adapt_Get_CodecType())
	//{
	//	Hst3520d_Adapt_Probe_Nvp6124FMT(&stInputFMT);
	//}

	memcpy(gs_arrVideoFmt, stInputFMT.getvideofmt, sizeof(gs_arrVideoFmt));

	printf("[%s:%d][gs_arrVideoFmt: %#x %#x %#x %#x %#x %#x]\n", __func__, __LINE__, 
        gs_arrVideoFmt[0], gs_arrVideoFmt[1], gs_arrVideoFmt[2],
        gs_arrVideoFmt[3], gs_arrVideoFmt[4], gs_arrVideoFmt[5]);

	return 0;
}

HI_U32 Hst3520d_Adapt_Get_CodecNum()
{
    return gs_u32CodecNum;
}

HI_U32 Hst3520d_Adapt_Get_ChnNum()
{
    /***************************************************
            调取此接口前必须确认系统已经调取过
            Hst3520d_Adapt_Probe_CodecNum()
            获取了真实的AD芯片数目。2018-06-19
    ****************************************************/

    if(2 == gs_u32CodecNum)
    {
        return 8;
    }
    else
    {
        return 4;
    }
}

HI_U32 Hst3520d_Adapt_Get_ChnFmt(HI_U32 u32PhyChn)
{
    return gs_arrVideoFmt[u32PhyChn];
}

AD_TYPE_EN Hst3520d_Adapt_Get_CodecType()
{
    return gs_enCodecType;
}

HI_S32 Hst3520d_Adapt_Get_VideoChnCfg(VIDEO_CHN_E enVQueChn,
    VIDEO_CHN_CFG_S *pstVideoCfg)
{
    HI_S32 i = 0;

    if((NULL == pstVideoCfg)) //|| HstSdkAL_CheckDevType(DEV_VERSION_NULL)) //设备类型未知也返回失败20181027
    {
		printf("[%s:%d] Para Error!\n", __func__, __LINE__);
        return -1;
    }

#if 0
    if(HstSdkAL_CheckDevType(DEV_VERSION_BT10))//20181027根据不同的设备类型映射不同的表add by tmf
    {
    	for(i = 0; i < (HI_S32)ARRAY_SIZE(g_astVideoChnCfgBt10Tbl); i++)
    	{
            if(enVQueChn == g_astVideoChnCfgBt10Tbl[i].enVQueChn)
            {
                memcpy(pstVideoCfg, &g_astVideoChnCfgBt10Tbl[i], sizeof(VIDEO_CHN_CFG_S));
                return 0;
            }
    	}
	}
#endif	
	//else if(HstSdkAL_CheckDevType(DEV_VERSION_BT20))
	{
        for(i = 0; i < (HI_S32)ARRAY_SIZE(g_astVideoChnCfgBt20Tbl); i++)
    	{
            if(enVQueChn == g_astVideoChnCfgBt20Tbl[i].enVQueChn)
            {
                memcpy(pstVideoCfg, &g_astVideoChnCfgBt20Tbl[i], sizeof(VIDEO_CHN_CFG_S));
                return 0;
            }
    	}
	}
	    
    /*for(i = 0; i < (HI_S32)ARRAY_SIZE(g_astVideoChnCfgTbl); i++)
	{
        if(enVQueChn == g_astVideoChnCfgTbl[i].enVQueChn)
        {
            memcpy(pstVideoCfg, &g_astVideoChnCfgTbl[i], sizeof(VIDEO_CHN_CFG_S));
            return 0;
        }
	}*/
	printf("[%s:%d] 未找到匹配的视频通道!返回失败!\n", __func__, __LINE__);

    return -1;
}


HI_S32 Hst3520d_Adapt_Get_AudioChnCfg(AUDIO_CHN_E enAQueChn,
    AUDIO_CHN_CFG_S *pstAudioCfg)
{
    HI_S32 i = 0;

    if((NULL == pstAudioCfg)) //|| HstSdkAL_CheckDevType(DEV_VERSION_NULL)) //设备类型未知也返回失败20181027
    {
		printf("[%s:%d] Para Error!\n", __func__, __LINE__);
        return -1;
    }
#if 0    
    if(HstSdkAL_CheckDevType(DEV_VERSION_BT10)) //20181027根据不同的设备类型映射不同的表add by tmf
    {
        for(i = 0; i < (HI_S32)ARRAY_SIZE(g_astAudioChnCfgBt10Tbl); i++)
        {
            if(enAQueChn == g_astAudioChnCfgBt10Tbl[i].enAQueChn)
            {
                memcpy(pstAudioCfg, &g_astAudioChnCfgBt10Tbl[i], sizeof(AUDIO_CHN_CFG_S));
                return 0;
            }
        }
    }
#endif	
    //else if(HstSdkAL_CheckDevType(DEV_VERSION_BT20))
	{
        for(i = 0; i < (HI_S32)ARRAY_SIZE(g_astAudioChnCfgBt20Tbl); i++)
        {
            if(enAQueChn == g_astAudioChnCfgBt20Tbl[i].enAQueChn)
            {
                memcpy(pstAudioCfg, &g_astAudioChnCfgBt20Tbl[i], sizeof(AUDIO_CHN_CFG_S));
                return 0;
            }
        }
	}
    
	/*for(i = 0; i < (HI_S32)ARRAY_SIZE(g_astAudioChnCfgTbl); i++)
	{
        if(enAQueChn == g_astAudioChnCfgTbl[i].enAQueChn)
        {
            memcpy(pstAudioCfg, &g_astAudioChnCfgTbl[i], sizeof(AUDIO_CHN_CFG_S));
            return 0;
        }
	}*/

	printf("[%s:%d] 未找到匹配的音频通道!返回失败!\n", __func__, __LINE__);

    return -1;
}
