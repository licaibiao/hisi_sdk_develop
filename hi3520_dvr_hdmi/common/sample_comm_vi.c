/******************************************************************************
  Some simple Hisilicon HI3531 video input functions.

  Copyright (C), 2010-2011, Hisilicon Tech. Co., Ltd.
 ******************************************************************************
    Modification:  2011-8 Created
******************************************************************************/
#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <math.h>
#include <unistd.h>
#include <signal.h>

#include "sample_comm.h"
#include "Hst3520dChnAdapt.h"

VI_DEV_ATTR_S DEV_ATTR_BT656D1_2MUX =
{
    /*接口模式*/
    VI_MODE_BT656,
    /*1、2、4路工作模式*/
    VI_WORK_MODE_2Multiplex,
    /* r_mask    g_mask    b_mask*/
    {0xFF000000,    0x0},

	/* 双沿输入时必须设置 */
	VI_CLK_EDGE_SINGLE_UP,
	
    /*AdChnId*/
    {-1, -1, -1, -1},
    /*enDataSeq, 仅支持YUV格式*/
    VI_INPUT_DATA_YVYU,
    /*同步信息，对应reg手册的如下配置, --bt1120时序无效*/
    {
    /*port_vsync   port_vsync_neg     port_hsync        port_hsync_neg        */
    VI_VSYNC_FIELD, VI_VSYNC_NEG_HIGH, VI_HSYNC_VALID_SINGNAL,VI_HSYNC_NEG_HIGH,VI_VSYNC_VALID_SINGAL,VI_VSYNC_VALID_NEG_HIGH,

    /*timing信息，对应reg手册的如下配置*/
    /*hsync_hfb    hsync_act    hsync_hhb*/
    {0,            0,        0,
    /*vsync0_vhb vsync0_act vsync0_hhb*/
     0,            0,        0,
    /*vsync1_vhb vsync1_act vsync1_hhb*/
     0,            0,            0}
    },
    /*使用内部ISP*/
    VI_PATH_BYPASS,
    /*输入数据类型*/
    VI_DATA_TYPE_YUV
};

VI_DEV_ATTR_S DEV_ATTR_BT656D1_4MUX =
{
    /*接口模式*/
    VI_MODE_BT656,
    /*1、2、4路工作模式*/
    VI_WORK_MODE_4Multiplex,
    /* r_mask    g_mask    b_mask*/
    {0xFF000000,    0x0},

	/* 双沿输入时必须设置 */
	VI_CLK_EDGE_SINGLE_UP,
	
    /*AdChnId*/
    {-1, -1, -1, -1},
    /*enDataSeq, 仅支持YUV格式*/
    VI_INPUT_DATA_YVYU,
    /*同步信息，对应reg手册的如下配置, --bt1120时序无效*/
    {
    /*port_vsync   port_vsync_neg     port_hsync        port_hsync_neg        */
    VI_VSYNC_FIELD, VI_VSYNC_NEG_HIGH, VI_HSYNC_VALID_SINGNAL,VI_HSYNC_NEG_HIGH,VI_VSYNC_VALID_SINGAL,VI_VSYNC_VALID_NEG_HIGH,

    /*timing信息，对应reg手册的如下配置*/
    /*hsync_hfb    hsync_act    hsync_hhb*/
    {0,            0,        0,
    /*vsync0_vhb vsync0_act vsync0_hhb*/
     0,            0,        0,
    /*vsync1_vhb vsync1_act vsync1_hhb*/
     0,            0,            0}
    },
    /*使用内部ISP*/
    VI_PATH_BYPASS,
    /*输入数据类型*/
    VI_DATA_TYPE_YUV
};

VI_DEV_ATTR_S DEV_ATTR_6114_720P_2MUX_BASE =
{
    /*接口模式*/
    VI_MODE_BT656,
    /*1、2、4路工作模式*/
    VI_WORK_MODE_2Multiplex,
    /* r_mask    g_mask    b_mask*/
    {0xFF000000,    0x0},

	/* 双沿输入时必须设置 */
	VI_CLK_EDGE_SINGLE_UP,
	
    /*AdChnId*/
    {-1, -1, -1, -1},
    /*enDataSeq, 仅支持YUV格式*/
    VI_INPUT_DATA_YVYU,
    /*同步信息，对应reg手册的如下配置, --bt1120时序无效*/
    {
    /*port_vsync   port_vsync_neg     port_hsync        port_hsync_neg        */
    VI_VSYNC_FIELD, VI_VSYNC_NEG_HIGH, VI_HSYNC_VALID_SINGNAL,VI_HSYNC_NEG_HIGH,VI_VSYNC_VALID_SINGAL,VI_VSYNC_VALID_NEG_HIGH,

    /*timing信息，对应reg手册的如下配置*/
    /*hsync_hfb    hsync_act    hsync_hhb*/
    {0,            0,        0,
    /*vsync0_vhb vsync0_act vsync0_hhb*/
     0,            0,        0,
    /*vsync1_vhb vsync1_act vsync1_hhb*/
     0,            0,            0}
    },
    /*使用内部ISP*/
    VI_PATH_BYPASS,
    /*输入数据类型*/
    VI_DATA_TYPE_YUV
};

VI_DEV_ATTR_S DEV_ATTR_6134_720P_4MUX_BASE =
{
    /*add by lianzihao 2017.8.11*/
    /*接口模式*/
    VI_MODE_BT656,
    /*1、2、4路工作模式*/
    VI_WORK_MODE_4Multiplex,
    /* r_mask    g_mask    b_mask*/
    {0xFF000000,    0x0},

	/* 双沿输入时必须设置 */
	VI_CLK_EDGE_DOUBLE,
	
    /*AdChnId*/
    {-1, -1, -1, -1},
    /*enDataSeq, 仅支持YUV格式*/
    VI_INPUT_DATA_YVYU,
    /*同步信息，对应reg手册的如下配置, --bt1120时序无效*/
    {
    /*port_vsync   port_vsync_neg     port_hsync        port_hsync_neg        */
    VI_VSYNC_FIELD, VI_VSYNC_NEG_HIGH, VI_HSYNC_VALID_SINGNAL,VI_HSYNC_NEG_HIGH,VI_VSYNC_VALID_SINGAL,VI_VSYNC_VALID_NEG_HIGH,

    /*timing信息，对应reg手册的如下配置*/
    /*hsync_hfb    hsync_act    hsync_hhb*/
    {0,            0,        0,
    /*vsync0_vhb vsync0_act vsync0_hhb*/
     0,            0,        0,
    /*vsync1_vhb vsync1_act vsync1_hhb*/
     0,            0,            0}
    },
    /*使用内部ISP*/
    VI_PATH_BYPASS,
    /*输入数据类型*/
    VI_DATA_TYPE_YUV
};

//VI_DEV_ATTR_S DEV_ATTR_6134_1080P_4MUX_BASE =  //>>>20181106芯片不支持4路1080P的复合输入
//{
//    /*add by tmf 2018.11.05*/
//    /*接口模式*/
//    VI_MODE_BT656,
//    /*1、2、4路工作模式*/
//    VI_WORK_MODE_4Multiplex,
//    /* r_mask    g_mask    b_mask*/
//    {0xFF000000,    0x0},
//
//	/* 双沿输入时必须设置 */
//	VI_CLK_EDGE_DOUBLE,
//	
//    /*AdChnId*/
//    {-1, -1, -1, -1},
//    /*enDataSeq, 仅支持YUV格式*/
//    VI_INPUT_DATA_YVYU,
//    /*同步信息，对应reg手册的如下配置, --bt1120时序无效*/
//    {
//    /*port_vsync   port_vsync_neg     port_hsync        port_hsync_neg        */
//    VI_VSYNC_FIELD, VI_VSYNC_NEG_HIGH, VI_HSYNC_VALID_SINGNAL,VI_HSYNC_NEG_HIGH,VI_VSYNC_VALID_SINGAL,VI_VSYNC_VALID_NEG_HIGH,
//
//    /*timing信息，对应reg手册的如下配置*/
//    /*hsync_hfb    hsync_act    hsync_hhb*/
//    {0,            0,        0,
//    /*vsync0_vhb vsync0_act vsync0_hhb*/
//     0,            0,        0,
//    /*vsync1_vhb vsync1_act vsync1_hhb*/
//     0,            0,            0}
//    },
//    /*使用内部ISP*/
//    VI_PATH_BYPASS,
//    /*输入数据类型*/
//    VI_DATA_TYPE_YUV
//};

VI_DEV g_as32ViDev[VIU_MAX_DEV_NUM];
VI_CHN g_as32MaxChn[VIU_MAX_CHN_NUM];
VI_CHN g_as32SubChn[VIU_MAX_CHN_NUM];

#define ADCODEC_DEV    "/dev/adcodec"

static AD_TYPE_EN gs_enADType = AD_TYPE_NVP6134C;//add by lianzihao 2017-03-06

/************** Add By LianZiHao For NVP6134 2017-08-10 Start **********************/
static unsigned int gs_CodecNum = 2;
/************** Add By LianZiHao For NVP6134 2017-08-10 End **********************/


HI_S32 SAMPLE_TW2865_CfgV(VIDEO_NORM_E enVideoMode,VI_WORK_MODE_E enWorkMode)
{
#if 0
    int fd, i;
    int video_mode;
    tw2865_video_norm stVideoMode;
    tw2865_work_mode work_mode;

    int chip_cnt = 4;

    fd = open(TW2865_FILE, O_RDWR);
    if (fd < 0)
    {
        SAMPLE_PRT("open 2865 (%s) fail\n", TW2865_FILE);
        return -1;
    }

    video_mode = (VIDEO_ENCODING_MODE_PAL == enVideoMode) ? TW2865_PAL : TW2865_NTSC ;

    for (i=0; i<chip_cnt; i++)
    {
        stVideoMode.chip    = i;
        stVideoMode.mode    = video_mode;
        if (ioctl(fd, TW2865_SET_VIDEO_NORM, &stVideoMode))
        {
            SAMPLE_PRT("set tw2865(%d) video mode fail\n", i);
            close(fd);
            return -1;
        }
    }

    for (i=0; i<chip_cnt; i++)
    {
        work_mode.chip = i;
        if (VI_WORK_MODE_4Multiplex== enWorkMode)
        {
            work_mode.mode = TW2865_4D1_MODE;
        }
        else if (VI_WORK_MODE_2Multiplex== enWorkMode)
        {
            work_mode.mode = TW2865_2D1_MODE;
        }
        else if (VI_WORK_MODE_1Multiplex == enWorkMode)
        {
            work_mode.mode = TW2865_1D1_MODE;
        }
        else
        {
            SAMPLE_PRT("work mode not support\n");
            return -1;
        }
        ioctl(fd, TW2865_SET_WORK_MODE, &work_mode);
    }

    close(fd);
#endif
    return 0;
}

HI_S32 SAMPLE_TW2960_CfgV(VIDEO_NORM_E enVideoMode,VI_WORK_MODE_E enWorkMode)
{
#if 0
    int fd, i;
    int video_mode;
    tw2865_video_norm stVideoMode;
    tw2865_work_mode work_mode;

    int chip_cnt = 4;

    fd = open(TW2960_FILE, O_RDWR);
    if (fd < 0)
    {
        SAMPLE_PRT("open 2960(%s) fail\n", TW2960_FILE);
        return -1;
    }

    video_mode = (VIDEO_ENCODING_MODE_PAL == enVideoMode) ? TW2960_PAL : TW2960_NTSC ;

    for (i=0; i<chip_cnt; i++)
    {
        stVideoMode.chip    = i;
        stVideoMode.mode    = video_mode;
        if (ioctl(fd, TW2960_SET_VIDEO_NORM, &stVideoMode))
        {
            SAMPLE_PRT("set tw2865(%d) video mode fail\n", i);
            close(fd);
            return -1;
        }
    }

    for (i=0; i<chip_cnt; i++)
    {
        work_mode.chip = i;
        if (VI_WORK_MODE_4Multiplex== enWorkMode)
        {
            work_mode.mode = TW2960_4D1_MODE;
        }
        else if (VI_WORK_MODE_2Multiplex== enWorkMode)
        {
            work_mode.mode = TW2960_2D1_MODE;
        }
        else if (VI_WORK_MODE_1Multiplex == enWorkMode)
        {
            work_mode.mode = TW2960_1D1_MODE;
        }
        else
        {
            SAMPLE_PRT("work mode not support\n");
            return -1;
        }
        ioctl(fd, TW2960_SET_WORK_MODE, &work_mode);
    }

    close(fd);
#endif
    return 0;
}

HI_S32 SAMPLE_NVP6114_CfgV(VIDEO_NORM_E enVideoMode,VI_WORK_MODE_E enWorkMode)
{
    return 0;
}


#if 1//add by lianzihao 2016-10-14

HI_S32 VI_WISDOM_NVP6124_GetVideoState(unsigned int *pu32VideoState)
{
	if(NULL == pu32VideoState)
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

	ioctl(fd, IOC_VDEC_GET_VIDEO_LOSS, pu32VideoState);

	//printf("[%s:%d] [video_state : %#x]\n", __func__, __LINE__, *pu32VideoState);

	close(fd);
	
	return 0;
}


HI_S32 VI_WISDOM_NVP6124_GetVideoFMT(nvp6124_input_videofmt *pstVideofmt)
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

#endif


/************** Add By LianZiHao For NVP6134 2017-02-28 Start**********************/

HI_S32 VI_WISDOM_NVP6134_GetVideoState(unsigned int *pu32VideoState,
    HI_S32 s32CodecNum)
{
	if(NULL == pu32VideoState)
	{
		printf("[%s:%d] Para Error!\n", __func__, __LINE__);
		return -1;
	}

	int fd = -1;
    unsigned char ucdate = 0;
    int chip_cnt = s32CodecNum;
    
	fd = open(NVP6134A_FILE, O_RDWR);
	if (fd < 0)
	{
		printf("[%s:%d] open nvp6134 (%s) fail\n", __func__, __LINE__, NVP6134A_FILE);
		return -1;
	}

	ioctl(fd, IOC_VDEC_GET_VIDEO_LOSS, pu32VideoState);

    if(2 == chip_cnt)
    {
        ucdate = (unsigned char)((*pu32VideoState)&0x3f);
    }
    else
    {
        ucdate = (unsigned char)((*pu32VideoState)&0xf);
        ucdate = (ucdate<<2)|(ucdate>>2);
        ucdate=((ucdate<<1)&0xa)|((ucdate>>1)&0x5); 
    }
	    
    //printf("[%s,%d]:chip_cnt=0x%x,pu32VideoState=0x%x,ucdate=0x%x\n",__FILE__,__LINE__,chip_cnt,*pu32VideoState,ucdate);
	//printf("[video_state : %#x][ucdate : %#x]\n", ((*pu32VideoState)&0xf), ucdate);

    *pu32VideoState = ucdate;

	close(fd);
	
	return 0;
}


HI_S32 VI_WISDOM_NVP6134_GetVideoFMT(nvp6124_input_videofmt *pstVideofmt)
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


/************** Add By LianZiHao For NVP6134 2017-02-28 End **********************/




/************** Modify By LianZiHao For NVP6134 2017-08-10 Start**********************/
HI_S32 VI_WISDOM_NVP6134_CfgV(VIDEO_NORM_E enVideoMode,SAMPLE_VI_6134_MODE_E enViMode,   HI_S32 s32CodecNum)
{
    int fd, i;
    int video_mode;
	nvp6134_opt_mode optmode;
	nvp6134_chn_mode schnmode;

    int chip_cnt = s32CodecNum;

    fd = open(NVP6134A_FILE, O_RDWR);
    if (fd < 0)
    {
        printf("open nvp6134 (%s) fail\n", NVP6134A_FILE);
        return -1;
    }

    video_mode = (VIDEO_ENCODING_MODE_PAL == enVideoMode) ? 1 : 0 ;

    SAMPLE_PRT("enVideoMode : %d\n", enVideoMode);

    switch(enViMode)
    {
        case SAMPLE_VI_MODE_6134_2MUX_D1:
        {
            SAMPLE_PRT("SAMPLE_VI_MODE_6134_2MUX_D1!!!\n");
			for(i=0;i<chip_cnt*4;i++)
			{
				schnmode.ch = i;
				schnmode.vformat = video_mode;
				schnmode.chmode = NVP6134_VI_720H;
                printf(" ch %d vformat %d chmode %d \n",schnmode.ch,schnmode.vformat,schnmode.chmode);
				ioctl(fd, IOC_VDEC_SET_CHNMODE, &schnmode);
			}
			for(i=0;i<chip_cnt;i++)
			{
				optmode.chipsel = i;
				optmode.portsel = 2;
				optmode.portmode = NVP6134_OUTMODE_2MUX_SD;
				optmode.chid = 0;
				ioctl(fd, IOC_VDEC_SET_OUTPORTMODE, &optmode);
                optmode.chipsel = i;
				optmode.portsel = 1;
				optmode.portmode = NVP6134_OUTMODE_2MUX_SD;
				optmode.chid = 1;
				ioctl(fd, IOC_VDEC_SET_OUTPORTMODE, &optmode);
			}
            printf("[%s:%d] SET OK!!!\n", __func__, __LINE__);
			break;
        }
        case SAMPLE_VI_MODE_6134_4MUX_D1:
        {//add by lianzihao 2017.8.11
            SAMPLE_PRT("SAMPLE_VI_MODE_6134_4MUX_D1!!!\n");
			for(i=0;i<chip_cnt*4;i++)
			{
				schnmode.ch = i;
				schnmode.vformat = video_mode;
				schnmode.chmode = NVP6134_VI_720H;
                printf(" ch %d vformat %d chmode %d \n",schnmode.ch,schnmode.vformat,schnmode.chmode);
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
            printf("[%s:%d] SET OK!!!\n", __func__, __LINE__);
			break;
        }
    	case SAMPLE_VI_MODE_6134_960H:
    	{
            SAMPLE_PRT("SAMPLE_VI_MODE_6134_960H!!!\n");
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
            printf("[%s:%d] SET OK!!!\n", __func__, __LINE__);
			break;
		}
		case SAMPLE_VI_MODE_6134_HDX: 
		{
            SAMPLE_PRT("SAMPLE_VI_MODE_6134_HDX!!!\n");
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
            printf("[%s:%d] SET OK!!!\n", __func__, __LINE__);
			break;
		}
		case SAMPLE_VI_MODE_6134_HD:
		{
            SAMPLE_PRT("SAMPLE_VI_MODE_6134_HD!!!\n");
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
				optmode.portmode = NVP6134_OUTMODE_2MUX_HD;
				optmode.chid = 0;
				ioctl(fd, IOC_VDEC_SET_OUTPORTMODE, &optmode);
				optmode.portsel = 1;
				optmode.portmode = NVP6134_OUTMODE_2MUX_HD;
				optmode.chid = 1;
				ioctl(fd, IOC_VDEC_SET_OUTPORTMODE, &optmode);
			}
            printf("[%s:%d] SET OK!!!\n", __func__, __LINE__);
			break;
		}
		case SAMPLE_VI_MODE_6134_FHDX:
		{
            SAMPLE_PRT("SAMPLE_VI_MODE_6134_FHDX!!!\n");
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
            printf("[%s:%d] SET OK!!!\n", __func__, __LINE__);
			break;
		}
		case SAMPLE_VI_MODE_6134_FHD:
		{
            SAMPLE_PRT("SAMPLE_VI_MODE_6134_FHD!!!\n");
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
            printf("[%s:%d] SET OK!!!\n", __func__, __LINE__);
			break;
		}
    	case SAMPLE_VI_MODE_6134_960H_720P_2MUX:
    	{
            SAMPLE_PRT("SAMPLE_VI_MODE_6134_960H_720P_2MUX!!!\n");
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
            SAMPLE_PRT("[%s:%d] SET OK!!!\n", __func__, __LINE__);
			break;
		}
        case SAMPLE_VI_MODE_6134_2MUX_FHD:
        {
            SAMPLE_PRT("SAMPLE_VI_MODE_6134_2MUX_FHD!!!\n");
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
                optmode.portmode = NVP6134_OUTMODE_2MUX_FHD;
                optmode.chid = 0;
                ioctl(fd, IOC_VDEC_SET_OUTPORTMODE, &optmode);
                optmode.portsel = 1;
                optmode.portmode = NVP6134_OUTMODE_2MUX_FHD;
                optmode.chid = 1;
                ioctl(fd, IOC_VDEC_SET_OUTPORTMODE, &optmode);
            }
            printf("[%s:%d] SET OK!!!\n", __func__, __LINE__);
            break;
        }
        case SAMPLE_VI_MODE_6134_4MUX_HD:
        {
            //add by lianzihao 2017_11_15
            SAMPLE_PRT("SAMPLE_VI_MODE_6134_4MUX_HD!!!\n");

            for(i=0;i<chip_cnt*4;i++)
            {
                schnmode.ch = i;
                schnmode.vformat = video_mode;
                schnmode.chmode = NVP6134_VI_720P_2530;
                ioctl(fd, IOC_VDEC_SET_CHNMODE, &schnmode);
            }
            
            for(i=0;i<chip_cnt;i++)
            {
                optmode.chipsel = i;// 0 1
                //optmode.portsel = 2;
                //optmode.portmode = NVP6134_OUTMODE_4MUX_HD;
                //optmode.chid = 0;
                //ioctl(fd, IOC_VDEC_SET_OUTPORTMODE, &optmode);
                optmode.portsel = 1;
                optmode.portmode = NVP6134_OUTMODE_4MUX_HD;
                optmode.chid = 0;
                ioctl(fd, IOC_VDEC_SET_OUTPORTMODE, &optmode);
            }
            break;
        }
		default:
		{
			SAMPLE_PRT("enViMode %d not supported!!!\n", enViMode);
			break;
        }

        close(fd);
    }

    return 0;
}
/************** Add By LianZiHao For NVP6134 2017-02-20 End **********************/

HI_S32 VI_MST_NVP6124_CfgV(VIDEO_NORM_E enVideoMode,SAMPLE_VI_6124_MODE_E enViMode)
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
            printf("SAMPLE_VI_MODE_2MUX_D1!!\n");
			for(i=0;i<chip_cnt*4;i++)
			{
				schnmode.ch = i;
				schnmode.vformat = video_mode;
				schnmode.chmode = NVP6124_VI_SD;
                printf(" ch %d vformat %d chmode %d \n",schnmode.ch,schnmode.vformat,schnmode.chmode);
				ioctl(fd, IOC_VDEC_SET_CHNMODE, &schnmode);
			}
			for(i=0;i<chip_cnt;i++)
			{
				optmode.chipsel = i;
				optmode.portsel = 2;
				optmode.portmode = NVP6124_OUTMODE_2MUX_SD;
				optmode.chid = 0;
				ioctl(fd, IOC_VDEC_SET_OUTPORTMODE, &optmode);
                optmode.chipsel = i;
				optmode.portsel = 3;
				optmode.portmode = NVP6124_OUTMODE_2MUX_SD;
				optmode.chid = 1;
				ioctl(fd, IOC_VDEC_SET_OUTPORTMODE, &optmode);
			}
            printf("VI_MST_NVP6124_CfgV SET OK!!\n");
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
                optmode.portmode = NVP6124_OUTMODE_2MUX_FHD;
                optmode.chid = 0;
                ioctl(fd, IOC_VDEC_SET_OUTPORTMODE, &optmode);
                optmode.portsel = 3;
                optmode.portmode = NVP6124_OUTMODE_2MUX_FHD;
                optmode.chid = 1;
                ioctl(fd, IOC_VDEC_SET_OUTPORTMODE, &optmode);
            }
            break;

            
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
            return -1;
            break;

		default:
			printf("enViMode %d not supported\n", enViMode);
			break;
    }

    close(fd);
    return 0;
}


/*****************************************************************************
* function : set vi mask.
*****************************************************************************/
HI_VOID SAMPLE_COMM_VI_SetMask(VI_DEV ViDev, VI_DEV_ATTR_S *pstDevAttr)
{
    switch (ViDev % 4)
    {
        case 0:
            pstDevAttr->au32CompMask[0] = 0xFF;
            if (VI_MODE_BT1120_STANDARD == pstDevAttr->enIntfMode)
            {
                pstDevAttr->au32CompMask[1] = 0x00FF0000;
            }
            else if (VI_MODE_BT1120_INTERLEAVED == pstDevAttr->enIntfMode)
            {
                pstDevAttr->au32CompMask[1] = 0x0;
            }
            break;
        case 1:
            pstDevAttr->au32CompMask[0] = 0xFF00;
            if (VI_MODE_BT1120_INTERLEAVED == pstDevAttr->enIntfMode)
            {
                pstDevAttr->au32CompMask[1] = 0x0;
            }
            break;
        case 2:
            pstDevAttr->au32CompMask[0] = 0xFF0000;
            if (VI_MODE_BT1120_STANDARD == pstDevAttr->enIntfMode)
            {
                pstDevAttr->au32CompMask[1] = 0xFF;
            }
            else if (VI_MODE_BT1120_INTERLEAVED == pstDevAttr->enIntfMode)
            {
                pstDevAttr->au32CompMask[1] = 0x0;
            }
            break;
        case 3:
            pstDevAttr->au32CompMask[0] = 0xFF000000;
            if (VI_MODE_BT1120_INTERLEAVED == pstDevAttr->enIntfMode)
            {
                pstDevAttr->au32CompMask[1] = 0x0;
            }
            break;
        default:
            HI_ASSERT(0);
    }
}

HI_S32 SAMPLE_COMM_VI_Mode2Param(SAMPLE_VI_MODE_E enViMode, SAMPLE_VI_PARAM_S *pstViParam)
{
    switch (enViMode)
    {
        //20181101 add by tmf 按照分组设置调整参数
        case SAMPLE_VI_MODE_2_1080P:
		case SAMPLE_VI_MODE_2_720P:
		case SAMPLE_VI_MODE_2_D1:
		{
			printf("[%s %d ]enViMode: %d\n",__func__,__LINE__,enViMode);
			pstViParam->s32ViDevCnt = 1;
			pstViParam->s32ViDevInterval = 1;
			pstViParam->s32ViChnCnt = 2;
			pstViParam->s32ViChnInterval = 1;
			break;
		}
		case SAMPLE_VI_MODE_4_D1:
		case SAMPLE_VI_MODE_4_720P:
		//case SAMPLE_VI_MODE_4_1080P:
	    {
			printf("[%s %d ]enViMode: %d\n",__func__,__LINE__,enViMode);
			pstViParam->s32ViDevCnt = 1;
			pstViParam->s32ViDevInterval = 1;
			pstViParam->s32ViChnCnt = 4;
			pstViParam->s32ViChnInterval = 1;
			break;
		}
		//其余保持和之前一样
		case SAMPLE_VI_MODE_16_D1:
        case SAMPLE_VI_MODE_16_960H:
        case SAMPLE_VI_MODE_16_1280H:
        case SAMPLE_VI_MODE_16_HALF720P:
            pstViParam->s32ViDevCnt = 4;
            pstViParam->s32ViDevInterval = 1;
            pstViParam->s32ViChnCnt = 16;
            pstViParam->s32ViChnInterval = 1;
            break;
        case SAMPLE_VI_MODE_8_720P:
        case SAMPLE_VI_MODE_8_D1:
            pstViParam->s32ViDevCnt = 2;
            pstViParam->s32ViDevInterval = 1;
            pstViParam->s32ViChnCnt = 8;
            pstViParam->s32ViChnInterval = 1;
            break;
        case SAMPLE_VI_MODE_8_1080P:
            //use chn 0,2,4,6,8,10,12,14
            pstViParam->s32ViDevCnt = 4;
            pstViParam->s32ViDevInterval = 1;
            pstViParam->s32ViChnCnt = 8;
            pstViParam->s32ViChnInterval = 2;
            break;
        case SAMPLE_VI_MODE_4_3M:
            //use chn 0,4,8,12
            pstViParam->s32ViDevCnt = 4;
            pstViParam->s32ViDevInterval = 1;
            pstViParam->s32ViChnCnt = 4;
            pstViParam->s32ViChnInterval = 4;
            break;

        case SAMPLE_VI_MODE_4_1080P:
        {
	        printf("[%s %d ]enViMode: %d\n",__func__,__LINE__,enViMode);
            pstViParam->s32ViDevCnt = 2;
            pstViParam->s32ViDevInterval = 1;
            pstViParam->s32ViChnCnt = 4;
            pstViParam->s32ViChnInterval = 1;
            break;
        }	
		
        /*case SAMPLE_VI_MODE_4_D1:
        {
            pstViParam->s32ViDevCnt = 2;
            pstViParam->s32ViDevInterval = 1;
            pstViParam->s32ViChnCnt = 4;
            pstViParam->s32ViChnInterval = 1;
            break;
        }
        case SAMPLE_VI_MODE_16_D1:
        case SAMPLE_VI_MODE_16_960H:
        case SAMPLE_VI_MODE_16_1280H:
        case SAMPLE_VI_MODE_16_HALF720P:
            pstViParam->s32ViDevCnt = 4;
            pstViParam->s32ViDevInterval = 1;
            pstViParam->s32ViChnCnt = 16;
            pstViParam->s32ViChnInterval = 1;
            break;
    	case SAMPLE_VI_MODE_4_720P:
    	{
    		pstViParam->s32ViDevCnt = 2;
    		pstViParam->s32ViDevInterval = 1;
    		pstViParam->s32ViChnCnt = 4;
    		pstViParam->s32ViChnInterval = 1;
    		break;
    	}
        //case SAMPLE_VI_MODE_8_720P:
        //case SAMPLE_VI_MODE_8_D1:
        //    pstViParam->s32ViDevCnt = 2;
        //    pstViParam->s32ViDevInterval = 1;
        //    pstViParam->s32ViChnCnt = 8;
        //    pstViParam->s32ViChnInterval = 1;

		//	SAMPLE_PRT("pstViParam->s32ViDevCnt:%d\n", pstViParam->s32ViDevCnt);
		//	SAMPLE_PRT("pstViParam->s32ViDevInterval:%d\n", pstViParam->s32ViDevInterval);
		//	SAMPLE_PRT("pstViParam->s32ViChnCnt:%d\n", pstViParam->s32ViChnCnt);
		//	SAMPLE_PRT("pstViParam->s32ViChnInterval:%d\n", pstViParam->s32ViChnInterval);

         //   break;
        case SAMPLE_VI_MODE_8_D1: //>>>暂时调成BT10一致
        case SAMPLE_VI_MODE_8_720P:
        case SAMPLE_VI_MODE_8_1080P:
            //use chn 0,2,4,6,8,10,12,14
            pstViParam->s32ViDevCnt = 4;
            pstViParam->s32ViDevInterval = 1;
            pstViParam->s32ViChnCnt = 8;
            pstViParam->s32ViChnInterval = 2;
            break;
        case SAMPLE_VI_MODE_4_1080P:
        {
            pstViParam->s32ViDevCnt = 2;
            pstViParam->s32ViDevInterval = 1;
            pstViParam->s32ViChnCnt = 4;
            pstViParam->s32ViChnInterval = 1;
            break;
        }
        case SAMPLE_VI_MODE_4_3M:
            //use chn 0,4,8,12
            pstViParam->s32ViDevCnt = 4;
            pstViParam->s32ViDevInterval = 1;
            pstViParam->s32ViChnCnt = 4;
            pstViParam->s32ViChnInterval = 4;
            break;
        case SAMPLE_VI_MODE_2_1080P:
		case SAMPLE_VI_MODE_2_720P:
		case SAMPLE_VI_MODE_2_D1:
		{
			printf("[%s %d ]enViMode: %d\n",__func__,__LINE__,enViMode);
			pstViParam->s32ViDevCnt = 1;
			pstViParam->s32ViDevInterval = 1;
			pstViParam->s32ViChnCnt = 2;
			pstViParam->s32ViChnInterval = 1;
			break;
		}*/
        default:
            SAMPLE_PRT("ViMode invaild!\n");
            return HI_FAILURE;
    }
    return HI_SUCCESS;
}


/************** Add By LianZiHao For NVP6134 2017-02-20 Start**********************/
HI_S32 SAMPLE_WISDOM_VI_ADStart(SAMPLE_VI_MODE_E enViMode,
    VIDEO_NORM_E enNorm,  HI_S32 s32CodecNum)
{
    VI_WORK_MODE_E enWorkMode;
    HI_S32 s32Ret;
    SAMPLE_VI_6134_MODE_E enMode;
    
    switch (enViMode)
    {
		case SAMPLE_VI_MODE_4_D1:
        {
            SAMPLE_PRT("enViMode SAMPLE_VI_MODE_4_D1\n");
            
            enMode = SAMPLE_VI_MODE_6134_2MUX_D1;
            SAMPLE_PRT("enWorkMode SAMPLE_VI_MODE_6134_2MUX_D1\n");

            s32Ret = VI_WISDOM_NVP6134_CfgV(enNorm, enMode, s32CodecNum);
            if (s32Ret != HI_SUCCESS)
            {
                SAMPLE_PRT("VI_WISDOM_NVP6134_CfgV failed with %#x!\n",\
                        s32Ret);
                return HI_FAILURE;
            }
            break;
        }
		case SAMPLE_VI_MODE_8_D1:
		{
		    //add by lianzihao 2017.8.11
            SAMPLE_PRT("enViMode SAMPLE_VI_MODE_8_D1\n");

            enWorkMode = SAMPLE_VI_MODE_6134_4MUX_D1;

            SAMPLE_PRT("enWorkMode SAMPLE_VI_MODE_6134_4MUX_D1\n");
            s32Ret = VI_WISDOM_NVP6134_CfgV(enNorm, enWorkMode, s32CodecNum);
            if (s32Ret != HI_SUCCESS)
            {
                SAMPLE_PRT("VI_WISDOM_NVP6134_CfgV failed with %#x!\n",\
                        s32Ret);
                return HI_FAILURE;
            }
            break;
        }
        case SAMPLE_VI_MODE_16_960H:
        {
            enMode = SAMPLE_VI_MODE_6134_960H;
            s32Ret = VI_WISDOM_NVP6134_CfgV(enNorm, enMode, s32CodecNum);
            if (s32Ret != HI_SUCCESS)
            {
                SAMPLE_PRT("VI_WISDOM_NVP6134_CfgV failed with %#x!\n",\
                        s32Ret);
                return HI_FAILURE;
            }
            break;
        }
        case SAMPLE_VI_MODE_4_720P:
        {
            SAMPLE_PRT("enViMode SAMPLE_VI_MODE_4_720P\n");
            enMode = SAMPLE_VI_MODE_6134_HD;
            SAMPLE_PRT("enWorkMode SAMPLE_VI_MODE_6134_HD\n");

            s32Ret = VI_WISDOM_NVP6134_CfgV(enNorm, enMode, s32CodecNum);
            if (s32Ret != HI_SUCCESS)
            {
                SAMPLE_PRT("VI_WISDOM_NVP6134_CfgV failed with %#x!\n",\
                        s32Ret);
                return HI_FAILURE;
            }
            break;
        }
        case SAMPLE_VI_MODE_8_720P:
        {   //add by lianzihao 2017.8.11
            SAMPLE_PRT("enViMode SAMPLE_VI_MODE_8_720P\n");
        
            enMode = SAMPLE_VI_MODE_6134_4MUX_HD;

            SAMPLE_PRT("enWorkMode SAMPLE_VI_MODE_6134_4MUX_HD\n");
            s32Ret = VI_WISDOM_NVP6134_CfgV(enNorm, enMode, s32CodecNum);
            if (s32Ret != HI_SUCCESS)
            {
                SAMPLE_PRT("VI_WISDOM_NVP6134_CfgV failed with %#x!\n",\
                        s32Ret);
                return HI_FAILURE;
            }
            break;
        }
        case SAMPLE_VI_MODE_8_1080P:
        {
            SAMPLE_PRT("AD nvp6134 not support 8 ch 1080P!\n");
            break;
        }
        case SAMPLE_VI_MODE_4_1080P:
        {
            enMode = SAMPLE_VI_MODE_6134_2MUX_FHD;
            s32Ret = VI_WISDOM_NVP6134_CfgV(enNorm, enMode, s32CodecNum);
            if (s32Ret != HI_SUCCESS)
            {
                SAMPLE_PRT("VI_WISDOM_NVP6134_CfgV failed with %#x!\n",\
                        s32Ret);
                return HI_FAILURE;
            }
            break;
        }
        default:
            SAMPLE_PRT("AD not support!\n");
            return HI_FAILURE;
    }
    
    return HI_SUCCESS;
}
/************** Add By LianZiHao For NVP6134 2017-02-20 End **********************/


/*****************************************************************************
* function : get vi parameter, according to vi type
*****************************************************************************/
HI_S32 SAMPLE_COMM_VI_ADStart(SAMPLE_VI_MODE_E enViMode, VIDEO_NORM_E enNorm)
{
    VI_WORK_MODE_E enWorkMode;
    HI_S32 s32Ret;
    SAMPLE_VI_6124_MODE_E enMode;
    
    switch (enViMode)
    {
		case SAMPLE_VI_MODE_4_D1:
        {
            enMode = SAMPLE_VI_MODE_2MUX_D1;
            s32Ret = VI_MST_NVP6124_CfgV(enNorm, enMode);
            if (s32Ret != HI_SUCCESS)
            {
                SAMPLE_PRT("SAMPLE_TW2960_CfgV failed with %#x!\n",\
                        s32Ret);
                return HI_FAILURE;
            }
            break;
        }
		case SAMPLE_VI_MODE_16_D1:
            enWorkMode = VI_WORK_MODE_4Multiplex;
            s32Ret = SAMPLE_TW2865_CfgV(enNorm, enWorkMode);
            if (s32Ret != HI_SUCCESS)
            {
                SAMPLE_PRT("SAMPLE_TW2865_CfgV failed with %#x!\n",\
                        s32Ret);
                return HI_FAILURE;
            }
            break;
        case SAMPLE_VI_MODE_16_960H:            
            enMode = SAMPLE_VI_MODE_6124_960H;
            s32Ret = VI_MST_NVP6124_CfgV(enNorm, enMode);
            if (s32Ret != HI_SUCCESS)
            {
                SAMPLE_PRT("SAMPLE_TW2960_CfgV failed with %#x!\n",\
                        s32Ret);
                return HI_FAILURE;
            }
            break;

        case SAMPLE_VI_MODE_4_720P://add by lianzihao 2016-07-21            
            enMode = SAMPLE_VI_MODE_6124_HD;
            s32Ret = VI_MST_NVP6124_CfgV(enNorm, enMode);
            if (s32Ret != HI_SUCCESS)
            {
                SAMPLE_PRT("SAMPLE_NVP6114_CfgV failed with %#x!\n",\
                        s32Ret);
                return HI_FAILURE;
            }
            break;

        case SAMPLE_VI_MODE_8_720P:            
            enMode = SAMPLE_VI_MODE_6124_HD;
            s32Ret = VI_MST_NVP6124_CfgV(enNorm, enMode);
            if (s32Ret != HI_SUCCESS)
            {
                SAMPLE_PRT("SAMPLE_NVP6114_CfgV failed with %#x!\n",\
                        s32Ret);
                return HI_FAILURE;
            }
            break;
        case SAMPLE_VI_MODE_8_1080P:           
            enMode = SAMPLE_VI_MODE_6124_2MUX_FHD;
            s32Ret = VI_MST_NVP6124_CfgV(enNorm, enMode);
            if (s32Ret != HI_SUCCESS)
            {
                SAMPLE_PRT("SAMPLE_NVP6114_CfgV failed with %#x!\n",\
                        s32Ret);
                return HI_FAILURE;
            }
            break;
        case SAMPLE_VI_MODE_4_1080P:
        {
            enMode = SAMPLE_VI_MODE_6124_2MUX_FHD;
            s32Ret = VI_MST_NVP6124_CfgV(enNorm, enMode);
            if (s32Ret != HI_SUCCESS)
            {
                SAMPLE_PRT("SAMPLE_NVP6114_CfgV failed with %#x!\n",\
                        s32Ret);
                return HI_FAILURE;
            }
            break;
        }
        default:
            SAMPLE_PRT("AD not support!\n");
            return HI_FAILURE;
    }
    
    return HI_SUCCESS;
}


/*****************************************************************************
* function : get vi parameter, according to vi type
*****************************************************************************/
HI_S32 SAMPLE_COMM_VI_Mode2Size(SAMPLE_VI_MODE_E enViMode, VIDEO_NORM_E enNorm, RECT_S *pstCapRect, SIZE_S *pstDestSize)
{
    pstCapRect->s32X = 0;
    pstCapRect->s32Y = 0;
    
    switch (enViMode)
    {
        case SAMPLE_VI_MODE_4_D1:
        case SAMPLE_VI_MODE_8_D1:
        case SAMPLE_VI_MODE_16_D1:
        case SAMPLE_VI_MODE_2_D1:
            pstDestSize->u32Width = D1_WIDTH;
            pstDestSize->u32Height = (VIDEO_ENCODING_MODE_PAL==enNorm)?576:480;
            pstCapRect->u32Width = D1_WIDTH;
            pstCapRect->u32Height = (VIDEO_ENCODING_MODE_PAL==enNorm)?576:480;
            break;
        case SAMPLE_VI_MODE_16_960H:
            pstDestSize->u32Width = 960;
            pstDestSize->u32Height = (VIDEO_ENCODING_MODE_PAL==enNorm)?576:480;
            pstCapRect->u32Width = 960;
            pstCapRect->u32Height = (VIDEO_ENCODING_MODE_PAL==enNorm)?576:480;
            break;
        case SAMPLE_VI_MODE_4_720P://add by lianzihao 2016-07-21
        case SAMPLE_VI_MODE_8_720P:
        case SAMPLE_VI_MODE_16_720P:
        case SAMPLE_VI_MODE_2_720P:
            pstDestSize->u32Width  = _720P_WIDTH;
            pstDestSize->u32Height = _720P_HEIGHT;
            pstCapRect->u32Width  = _720P_WIDTH;
            pstCapRect->u32Height = _720P_HEIGHT;
            break;
            
     case SAMPLE_VI_MODE_8_1080P: 
     case SAMPLE_VI_MODE_4_1080P: 
     case SAMPLE_VI_MODE_2_1080P: 
            pstDestSize->u32Width  = HD_WIDTH;
            pstDestSize->u32Height = HD_HEIGHT;
            pstCapRect->u32Width  = HD_WIDTH;
            pstCapRect->u32Height = HD_HEIGHT;
            break;

        default:
            SAMPLE_PRT("vi mode invaild!\n");
            return HI_FAILURE;
    }
    
    return HI_SUCCESS;
}

/*****************************************************************************
* function : Get Vi Dev No. according to Vi_Chn No.
*****************************************************************************/
VI_DEV SAMPLE_COMM_VI_GetDev(SAMPLE_VI_MODE_E enViMode, VI_CHN ViChn)
{
    HI_S32 s32Ret, s32ChnPerDev;
    SAMPLE_VI_PARAM_S stViParam;

    s32Ret = SAMPLE_COMM_VI_Mode2Param(enViMode, &stViParam);
    if (HI_SUCCESS !=s32Ret)
    {
        SAMPLE_PRT("vi get param failed!\n");
        return (VI_DEV)-1;
    }

    s32ChnPerDev = stViParam.s32ViChnCnt / stViParam.s32ViDevCnt;
    return (VI_DEV)(ViChn /stViParam.s32ViChnInterval / s32ChnPerDev * stViParam.s32ViDevInterval);
}


/************** Add By LianZiHao For NVP6134 2017-02-20 Start**********************/
HI_S32 SAMPLE_WISDOM_VI_StartDev(VI_DEV ViDev, SAMPLE_VI_MODE_E enViMode)
{
    HI_S32 s32Ret;
    VI_DEV_ATTR_S stViDevAttr;
    memset(&stViDevAttr,0,sizeof(stViDevAttr));

    switch (enViMode)
    {
        case SAMPLE_VI_MODE_4_D1:
        {
            memcpy(&stViDevAttr,&DEV_ATTR_BT656D1_2MUX,sizeof(stViDevAttr));
            SAMPLE_COMM_VI_SetMask(ViDev,&stViDevAttr);
            break;
        }
        case SAMPLE_VI_MODE_16_D1:
            memcpy(&stViDevAttr,&DEV_ATTR_BT656D1_4MUX,sizeof(stViDevAttr));
            SAMPLE_COMM_VI_SetMask(ViDev,&stViDevAttr);
            break;
        case SAMPLE_VI_MODE_16_960H:
            memcpy(&stViDevAttr,&DEV_ATTR_BT656D1_4MUX,sizeof(stViDevAttr));
            SAMPLE_COMM_VI_SetMask(ViDev,&stViDevAttr);
            break;
        case SAMPLE_VI_MODE_4_720P://add by lianzihao 2016-07-21
        case SAMPLE_VI_MODE_8_720P:
        case SAMPLE_VI_MODE_16_720P:
        case SAMPLE_VI_MODE_8_1080P:
        case SAMPLE_VI_MODE_4_1080P:
            memcpy(&stViDevAttr,&DEV_ATTR_6114_720P_2MUX_BASE,sizeof(stViDevAttr));
            SAMPLE_COMM_VI_SetMask(ViDev,&stViDevAttr);
            break;
        default:
            SAMPLE_PRT("vi input type[%d] is invalid!\n", enViMode);
            return HI_FAILURE;
    }

    if(SAMPLE_VI_MODE_8_1080P == enViMode||SAMPLE_VI_MODE_4_1080P== enViMode)
    {
        stViDevAttr.enClkEdge = VI_CLK_EDGE_DOUBLE;
    }
    s32Ret = HI_MPI_VI_SetDevAttr(ViDev, &stViDevAttr);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("HI_MPI_VI_SetDevAttr failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_VI_EnableDev(ViDev);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("HI_MPI_VI_EnableDev failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}
/************** Add By LianZiHao For NVP6134 2017-02-20 End **********************/


/*****************************************************************************
* function : star vi dev (cfg vi_dev_attr; set_dev_cfg; enable dev)
*****************************************************************************/
HI_S32 SAMPLE_COMM_VI_StartDev(VI_DEV ViDev, SAMPLE_VI_MODE_E enViMode)
{
    HI_S32 s32Ret;
    VI_DEV_ATTR_S stViDevAttr;
    memset(&stViDevAttr,0,sizeof(stViDevAttr));

    switch (enViMode)
    {
        case SAMPLE_VI_MODE_4_D1: //add by tmf,分组设置需要启动4路符合输入20181101
            memcpy(&stViDevAttr,&DEV_ATTR_BT656D1_4MUX,sizeof(stViDevAttr));
            SAMPLE_COMM_VI_SetMask(ViDev,&stViDevAttr);
            break;
        //case SAMPLE_VI_MODE_4_D1:
        case SAMPLE_VI_MODE_2_D1:
        {
            memcpy(&stViDevAttr,&DEV_ATTR_BT656D1_2MUX,sizeof(stViDevAttr));
            SAMPLE_COMM_VI_SetMask(ViDev,&stViDevAttr);
            break;
        }
        case SAMPLE_VI_MODE_16_D1:
            memcpy(&stViDevAttr,&DEV_ATTR_BT656D1_4MUX,sizeof(stViDevAttr));
            SAMPLE_COMM_VI_SetMask(ViDev,&stViDevAttr);
            break;
        case SAMPLE_VI_MODE_8_D1://modify by lianzihao 2017.8.11
        {
            memcpy(&stViDevAttr,&DEV_ATTR_BT656D1_4MUX,sizeof(stViDevAttr));
            SAMPLE_COMM_VI_SetMask(ViDev,&stViDevAttr);
            break;
        }
        case SAMPLE_VI_MODE_8_720P://modify by lianzihao 2017.8.11
        {
            memcpy(&stViDevAttr,&DEV_ATTR_6134_720P_4MUX_BASE,sizeof(stViDevAttr));
            SAMPLE_COMM_VI_SetMask(ViDev,&stViDevAttr);
            break;
        }
        
        case SAMPLE_VI_MODE_16_960H:
        {
            memcpy(&stViDevAttr,&DEV_ATTR_BT656D1_4MUX,sizeof(stViDevAttr));
            SAMPLE_COMM_VI_SetMask(ViDev,&stViDevAttr);
            break;
        }
        case SAMPLE_VI_MODE_4_720P://add by tmf,分组设置需要启动4路符合输入20181101
            memcpy(&stViDevAttr,&DEV_ATTR_6134_720P_4MUX_BASE,sizeof(stViDevAttr));
            SAMPLE_COMM_VI_SetMask(ViDev,&stViDevAttr);
            break;
        /*case SAMPLE_VI_MODE_4_1080P://add by tmf,分组设置需要启动4路符合输入20181105
            memcpy(&stViDevAttr,&DEV_ATTR_6134_1080P_4MUX_BASE,sizeof(stViDevAttr));
            SAMPLE_COMM_VI_SetMask(ViDev,&stViDevAttr);
            break;*/
        //case SAMPLE_VI_MODE_4_720P://add by lianzihao 2016-07-21
        case SAMPLE_VI_MODE_16_720P:
        case SAMPLE_VI_MODE_8_1080P:
        case SAMPLE_VI_MODE_4_1080P: //>>>不支持4路复合1080P输入此时此处按照720P的2路复合双边采样即可20181106
        case SAMPLE_VI_MODE_2_1080P:
		case SAMPLE_VI_MODE_2_720P:
        {
            memcpy(&stViDevAttr,&DEV_ATTR_6114_720P_2MUX_BASE,sizeof(stViDevAttr));
            SAMPLE_COMM_VI_SetMask(ViDev,&stViDevAttr);
            break;
        }
        
        default:
        {
            SAMPLE_PRT("vi input type[%d] is invalid!\n", enViMode);
            return HI_FAILURE;
        }
    }

    if(SAMPLE_VI_MODE_2_1080P == enViMode || SAMPLE_VI_MODE_8_1080P == enViMode||SAMPLE_VI_MODE_4_1080P== enViMode)
    {
        stViDevAttr.enClkEdge = VI_CLK_EDGE_DOUBLE;
    }
    s32Ret = HI_MPI_VI_SetDevAttr(ViDev, &stViDevAttr);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("HI_MPI_VI_SetDevAttr failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_VI_EnableDev(ViDev);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("HI_MPI_VI_EnableDev failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

/*****************************************************************************
* function : star vi chn
*****************************************************************************/
HI_S32 SAMPLE_COMM_VI_StartChn(VI_CHN ViChn, RECT_S *pstCapRect, SIZE_S *pstTarSize, 
    SAMPLE_VI_MODE_E enViMode, SAMPLE_VI_CHN_SET_E enViChnSet)
{
    HI_S32 s32Ret;
    VI_CHN_ATTR_S stChnAttr;

    /* step  5: config & start vicap dev */
    memcpy(&stChnAttr.stCapRect, pstCapRect, sizeof(RECT_S));
    /* to show scale. this is a sample only, we want to show dist_size = D1 only */
    stChnAttr.stDestSize.u32Width = pstTarSize->u32Width;
    stChnAttr.stDestSize.u32Height = pstTarSize->u32Height;
    stChnAttr.enCapSel = VI_CAPSEL_BOTH;
    stChnAttr.enPixFormat = SAMPLE_PIXEL_FORMAT;   /* sp420 or sp422 */
    stChnAttr.bMirror = (VI_CHN_SET_MIRROR == enViChnSet)?HI_TRUE:HI_FALSE;
    stChnAttr.bFlip = (VI_CHN_SET_FILP == enViChnSet)?HI_TRUE:HI_FALSE;
    stChnAttr.s32SrcFrameRate = -1;
    stChnAttr.s32DstFrameRate = -1;
    switch (enViMode)
    {
        case SAMPLE_VI_MODE_4_D1:
        case SAMPLE_VI_MODE_2_D1:
        case SAMPLE_VI_MODE_16_D1:
        case SAMPLE_VI_MODE_16_960H:
        case SAMPLE_VI_MODE_16_1280H:
            stChnAttr.enScanMode = VI_SCAN_INTERLACED;
            break;
        case SAMPLE_VI_MODE_16_HALF720P:
        case SAMPLE_VI_MODE_2_720P:
        case SAMPLE_VI_MODE_4_720P://add by lianzihao 2016-07-21
        case SAMPLE_VI_MODE_8_720P:
        case SAMPLE_VI_MODE_16_720P:
        case SAMPLE_VI_MODE_2_1080P:
        case SAMPLE_VI_MODE_4_1080P:
        case SAMPLE_VI_MODE_8_1080P:
        case SAMPLE_VI_MODE_16_1080P:
        case SAMPLE_VI_MODE_4_3M:
            stChnAttr.enScanMode = VI_SCAN_PROGRESSIVE;
            break;
        default:
            SAMPLE_PRT("ViMode invaild!\n");
            return HI_FAILURE;
    }

    s32Ret = HI_MPI_VI_SetChnAttr(ViChn, &stChnAttr);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }
    
    s32Ret = HI_MPI_VI_EnableChn(ViChn);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

/*****************************************************************************
* function : star vi according to product type
*            if vi input is hd, we will start sub-chn for cvbs preview
*****************************************************************************/
HI_S32 SAMPLE_COMM_VI_StartGroup(unsigned char u8Group,SAMPLE_VI_MODE_E enViMode, VIDEO_NORM_E enNorm)
{
    VI_DEV ViDev;
    VI_CHN ViChn;
    HI_S32 i;
    HI_S32 s32Ret;
    SAMPLE_VI_PARAM_S stViParam;
    SIZE_S stTargetSize;
    RECT_S stCapRect;
    VI_CHN_BIND_ATTR_S stChnBindAttr;
    
    /*** get parameter from Sample_Vi_Mode ***/
    s32Ret = SAMPLE_COMM_VI_Mode2Param(enViMode, &stViParam);
    if (HI_SUCCESS !=s32Ret)
    {
        SAMPLE_PRT("vi get param failed!\n");
        return HI_FAILURE;
    }
    s32Ret = SAMPLE_COMM_VI_Mode2Size(enViMode, enNorm, &stCapRect, &stTargetSize);
    if (HI_SUCCESS !=s32Ret)
    {
        SAMPLE_PRT("vi get size failed!\n");
        return HI_FAILURE;
    }
    
    /*** Start AD ***/
	if(AD_TYPE_NVP6134C == Hst3520d_Adapt_Get_CodecType() || AD_TYPE_NVP6124 == Hst3520d_Adapt_Get_CodecType())
	{
        s32Ret = SAMPLE_WISDOM_VI_ADStart_ByGroup(u8Group,enViMode, enNorm);
	}

    if (HI_SUCCESS !=s32Ret)
    {
        SAMPLE_PRT("Start AD failed!\n");
        return HI_FAILURE;
    }
    
    /*** Start VI Dev ***/
 
    ViDev = u8Group;
    s32Ret = SAMPLE_COMM_VI_StartDev(ViDev, enViMode);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VI_StartDev failed with %#x\n", s32Ret);
        return HI_FAILURE;
    }
    /*** Start VI Chn ***/
    for(i=0; i<stViParam.s32ViChnCnt; i++)
    {
        //if(HstSdkAL_CheckDevType(DEV_VERSION_BT10))
        //    ViChn = i * stViParam.s32ViChnInterval+u8Group*2;
       // else if(HstSdkAL_CheckDevType(DEV_VERSION_BT20))
            ViChn = i * stViParam.s32ViChnInterval+((0 == u8Group)?0:4);
        //else
		//    SAMPLE_PRT("设备类型配置!\n");
		
		//20181031 add by tmf 兼容BT20的设备在分组中设置
        switch(enViMode)
        {
        case SAMPLE_VI_MODE_2_D1:
        case SAMPLE_VI_MODE_2_720P:
        case SAMPLE_VI_MODE_2_1080P:
            stChnBindAttr.ViDev = u8Group;
            if (ViChn%2 == 0)
                stChnBindAttr.ViWay = 0;
            else
                stChnBindAttr.ViWay = 1;  
            printf("2路:ViDev ViWay ViChn [%x  %x %x]\n",stChnBindAttr.ViDev,stChnBindAttr.ViWay,ViChn);
			s32Ret = HI_MPI_VI_GetChnBind(ViChn, &stChnBindAttr);
			if (HI_ERR_VI_FAILED_NOTBIND == s32Ret)
			{
                s32Ret = HI_MPI_VI_BindChn(ViChn, &stChnBindAttr);
                if (HI_SUCCESS != s32Ret)
                {
                    SAMPLE_PRT("call HI_MPI_VI_BindChn failed with %#x\n", s32Ret);
                    return HI_FAILURE;
                } 
            } 
            break;
        case SAMPLE_VI_MODE_4_D1:
        case SAMPLE_VI_MODE_4_720P:
        case SAMPLE_VI_MODE_4_1080P:
            stChnBindAttr.ViDev = u8Group;
            if (ViChn%4 == 0)//Dev0,1都接4路
                stChnBindAttr.ViWay = 0;
            if (ViChn%4 == 1)
                stChnBindAttr.ViWay = 1;
            if (ViChn%4 == 2)
                stChnBindAttr.ViWay = 2;
            if (ViChn%4 == 3)
                stChnBindAttr.ViWay = 3;
            /*if(0 == u8Group) //Dev0接4路,Dev1接2路
            {
                if (ViChn%4 == 0)
                    stChnBindAttr.ViWay = 0;
                if (ViChn%4 == 1)
                    stChnBindAttr.ViWay = 1;
                if (ViChn%4 == 2)
                    stChnBindAttr.ViWay = 2;
                if (ViChn%4 == 3)
                    stChnBindAttr.ViWay = 3;
            }
            else if(1 == u8Group)//Dev0接2路,Dev1接4路
            {
                if ((ViChn-2)%4 == 0)
                    stChnBindAttr.ViWay = 0;
                if ((ViChn-2)%4 == 1)
                    stChnBindAttr.ViWay = 1;
                if ((ViChn-2)%4 == 2)
                    stChnBindAttr.ViWay = 2;
                if ((ViChn-2)%4 == 3)
                    stChnBindAttr.ViWay = 3;
            }
            else
            {
                printf("[%s:%d]分组绑定设备号4路绑定出现未做的组合\n", __func__, __LINE__);
                return HI_FAILURE;
            }*/
            printf("4路:ViDev ViWay ViChn [%x  %x %x]\n",stChnBindAttr.ViDev,stChnBindAttr.ViWay,ViChn);
			s32Ret = HI_MPI_VI_GetChnBind(ViChn, &stChnBindAttr);
			if (HI_ERR_VI_FAILED_NOTBIND == s32Ret)
			{
                s32Ret = HI_MPI_VI_BindChn(ViChn, &stChnBindAttr);
                if (HI_SUCCESS != s32Ret)
                {
                    SAMPLE_PRT("call HI_MPI_VI_BindChn failed with %#x\n", s32Ret);
                    return HI_FAILURE;
                } 
            } 
            break;
        default:
            printf("[%s:%d]分组绑定设备号异常\n", __func__, __LINE__);
            return HI_FAILURE;
        }
        
        /*if (SAMPLE_VI_MODE_8_1080P == enViMode || SAMPLE_VI_MODE_8_720P == enViMode)
        {
            //When in the 8x1080p mode, bind chn 2,6,10,14 to way1 is needed
            if (ViChn%4 != 0)
            {
                s32Ret = HI_MPI_VI_GetChnBind(ViChn, &stChnBindAttr);
                if (HI_ERR_VI_FAILED_NOTBIND == s32Ret)
                {
                    stChnBindAttr.ViDev = ViChn/4;
                    stChnBindAttr.ViWay = 1;
                    s32Ret = HI_MPI_VI_BindChn(ViChn, &stChnBindAttr);
                    if (HI_SUCCESS != s32Ret)
                    {
                        SAMPLE_PRT("call HI_MPI_VI_BindChn failed with %#x\n", s32Ret);
                        return HI_FAILURE;
                    } 
                } 
            }
        }
        if (SAMPLE_VI_MODE_4_D1 == enViMode||SAMPLE_VI_MODE_2_D1 == enViMode)
        {
            stChnBindAttr.ViDev = ViChn/2;
            if (ViChn%2 == 0)
                stChnBindAttr.ViWay = 0;
            else
                stChnBindAttr.ViWay = 1;  
            printf("ViDev ViWay ViChn [%x  %x %x]\n",stChnBindAttr.ViDev,stChnBindAttr.ViWay,ViChn);
			s32Ret = HI_MPI_VI_GetChnBind(ViChn, &stChnBindAttr);
			if (HI_ERR_VI_FAILED_NOTBIND == s32Ret)
			{
                s32Ret = HI_MPI_VI_BindChn(ViChn, &stChnBindAttr);
                if (HI_SUCCESS != s32Ret)
                {
                    SAMPLE_PRT("call HI_MPI_VI_BindChn failed with %#x\n", s32Ret);
                    return HI_FAILURE;
                } 
            } 
        }
        if (SAMPLE_VI_MODE_4_720P == enViMode||SAMPLE_VI_MODE_2_720P == enViMode)//add by lianzihao 2016-07-21
        {
            stChnBindAttr.ViDev = ViChn/2;
            if (ViChn%2 == 0)
                stChnBindAttr.ViWay = 0;
            else
                stChnBindAttr.ViWay = 1;  
            printf("ViDev ViWay ViChn [%x  %x %x]\n",stChnBindAttr.ViDev,stChnBindAttr.ViWay,ViChn);
			s32Ret = HI_MPI_VI_GetChnBind(ViChn, &stChnBindAttr);
			if (HI_ERR_VI_FAILED_NOTBIND == s32Ret)
			{
                s32Ret = HI_MPI_VI_BindChn(ViChn, &stChnBindAttr);
                if (HI_SUCCESS != s32Ret)
                {
                    SAMPLE_PRT("call HI_MPI_VI_BindChn failed with %#x\n", s32Ret);
                    return HI_FAILURE;
                } 
            } 
        }
		
        if (SAMPLE_VI_MODE_4_1080P == enViMode ||SAMPLE_VI_MODE_2_1080P == enViMode)
        {
            stChnBindAttr.ViDev = ViChn/2;
            if (ViChn%2 == 0)
                stChnBindAttr.ViWay = 0;
            else
                stChnBindAttr.ViWay = 1;  
            printf("ViDev ViWay ViChn [%x  %x %x]\n",stChnBindAttr.ViDev,stChnBindAttr.ViWay,ViChn);
			s32Ret = HI_MPI_VI_GetChnBind(ViChn, &stChnBindAttr);
			if (HI_ERR_VI_FAILED_NOTBIND == s32Ret)
			{
                s32Ret = HI_MPI_VI_BindChn(ViChn, &stChnBindAttr);
                if (HI_SUCCESS != s32Ret)
                {
                    SAMPLE_PRT("call HI_MPI_VI_BindChn failed with %#x\n", s32Ret);
                    return HI_FAILURE;
                } 
            } 
        }*/
        s32Ret = SAMPLE_COMM_VI_StartChn(ViChn, &stCapRect, &stTargetSize, enViMode, VI_CHN_SET_NORMAL);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("call SAMPLE_COMM_VI_StarChn failed with %#x\n", s32Ret);
            return HI_FAILURE;
        } 
    }

    return HI_SUCCESS;
}

/************** Add By LianZiHao For NVP6134 2017-02-20 Start**********************/
HI_S32 SAMPLE_WISDOM_VI_ADStart_ByGroup(unsigned char u8Group,SAMPLE_VI_MODE_E enViMode, VIDEO_NORM_E enNorm)
{
    VI_WORK_MODE_E enWorkMode;
    HI_S32 s32Ret;
    SAMPLE_VI_6134_MODE_E enMode;
    
    switch (enViMode)
    {
        case SAMPLE_VI_MODE_4_D1: //add by tmf
            if(AD_TYPE_NVP6134C == Hst3520d_Adapt_Get_CodecType())
            {
	            enMode = SAMPLE_VI_MODE_6134_4MUX_D1;
	            s32Ret = VI_WISDOM_NVP6134_CfgByGroup(u8Group,enNorm, enMode);
                if (s32Ret != HI_SUCCESS)
                {
                    SAMPLE_PRT("VI_WISDOM_NVP6134_CfgV failed with %#x!\n",\
                            s32Ret);
                    return HI_FAILURE;
                }
        	}
            break;
    	case SAMPLE_VI_MODE_2_D1:
//		case SAMPLE_VI_MODE_4_D1:
        {
        	if(AD_TYPE_NVP6134C == Hst3520d_Adapt_Get_CodecType())
            {
	            enMode = SAMPLE_VI_MODE_6134_2MUX_D1;
	            s32Ret = VI_WISDOM_NVP6134_CfgByGroup(u8Group,enNorm, enMode);
	            if (s32Ret != HI_SUCCESS)
	            {
	                SAMPLE_PRT("VI_WISDOM_NVP6134_CfgV failed with %#x!\n",\
	                        s32Ret);
	                return HI_FAILURE;
	            }
        	}
        	if(AD_TYPE_NVP6124 == Hst3520d_Adapt_Get_CodecType())
    		{
	            enMode = SAMPLE_VI_MODE_2MUX_D1;
	            s32Ret = VI_WISDOM_NVP6124_CfgByGroup(u8Group,enNorm, enMode);
	            if (s32Ret != HI_SUCCESS)
	            {
	                SAMPLE_PRT("VI_WISDOM_NVP6134_CfgV failed with %#x!\n",\
	                        s32Ret);
	                return HI_FAILURE;
	            }
        	}
            break;
        }
		case SAMPLE_VI_MODE_16_D1:
            enWorkMode = VI_WORK_MODE_4Multiplex;
            s32Ret = SAMPLE_TW2865_CfgV(enNorm, enWorkMode);
            if (s32Ret != HI_SUCCESS)
            {
                SAMPLE_PRT("SAMPLE_TW2865_CfgV failed with %#x!\n",\
                        s32Ret);
                return HI_FAILURE;
            }
            break;
        case SAMPLE_VI_MODE_16_960H:            
            enMode = SAMPLE_VI_MODE_6134_960H;
            s32Ret = VI_WISDOM_NVP6134_CfgV(enNorm, enMode,1);
            if (s32Ret != HI_SUCCESS)
            {
                SAMPLE_PRT("VI_WISDOM_NVP6134_CfgV failed with %#x!\n",\
                        s32Ret);
                return HI_FAILURE;
            }
            break;
        case SAMPLE_VI_MODE_4_720P: //add by tmf
            if(AD_TYPE_NVP6134C == Hst3520d_Adapt_Get_CodecType())
            {
	            enMode = SAMPLE_VI_MODE_6134_4MUX_HD;
	            s32Ret = VI_WISDOM_NVP6134_CfgByGroup(u8Group,enNorm, enMode);
                if (s32Ret != HI_SUCCESS)
                {
                    SAMPLE_PRT("VI_WISDOM_NVP6134_CfgV failed with %#x!\n",\
                            s32Ret);
                    return HI_FAILURE;
                }
        	}
            break;
		case SAMPLE_VI_MODE_2_720P:  
//        case SAMPLE_VI_MODE_4_720P: 
        	if(AD_TYPE_NVP6134C == Hst3520d_Adapt_Get_CodecType())
            {
	            enMode = SAMPLE_VI_MODE_6134_HD;
	            s32Ret = VI_WISDOM_NVP6134_CfgByGroup(u8Group,enNorm, enMode);
	            if (s32Ret != HI_SUCCESS)
	            {
	                SAMPLE_PRT("VI_WISDOM_NVP6134_CfgV failed with %#x!\n",\
	                        s32Ret);
	                return HI_FAILURE;
	            }
        	}
        	if(AD_TYPE_NVP6124 == Hst3520d_Adapt_Get_CodecType())
            {
	            enMode = SAMPLE_VI_MODE_6124_HD;
				printf("[%s %d] u8Group %d SAMPLE_VI_MODE_6124_HD \n",__func__,__LINE__,u8Group);
	            s32Ret = VI_WISDOM_NVP6124_CfgByGroup(u8Group,enNorm, enMode);
	            if (s32Ret != HI_SUCCESS)
	            {
	                SAMPLE_PRT("VI_WISDOM_NVP6134_CfgV failed with %#x!\n",\
	                        s32Ret);
	                return HI_FAILURE;
	            }
        	}
            break;
        case SAMPLE_VI_MODE_8_720P:            
            enMode = SAMPLE_VI_MODE_6134_HD;
            s32Ret = VI_WISDOM_NVP6134_CfgV(enNorm, enMode,1);
            if (s32Ret != HI_SUCCESS)
            {
                SAMPLE_PRT("VI_WISDOM_NVP6134_CfgV failed with %#x!\n",\
                        s32Ret);
                return HI_FAILURE;
            }
            break;
        case SAMPLE_VI_MODE_8_1080P:           
            enMode = SAMPLE_VI_MODE_6134_2MUX_FHD;
            s32Ret = VI_WISDOM_NVP6134_CfgV(enNorm, enMode,1);
            if (s32Ret != HI_SUCCESS)
            {
                SAMPLE_PRT("VI_WISDOM_NVP6134_CfgV failed with %#x!\n",\
                        s32Ret);
                return HI_FAILURE;
            }
            break;
        case SAMPLE_VI_MODE_4_1080P: //add by tmf,BT20按照4路1080P复合进来但是实际执行按照2路1080P复合设置
            if(AD_TYPE_NVP6134C == Hst3520d_Adapt_Get_CodecType())
            {
	            enMode = SAMPLE_VI_MODE_6134_4MUX_FHD;
	            s32Ret = VI_WISDOM_NVP6134_CfgByGroup(u8Group,enNorm, enMode);
	            if (s32Ret != HI_SUCCESS)
	            {
	                SAMPLE_PRT("VI_WISDOM_NVP6134_CfgV failed with %#x!\n",\
	                        s32Ret);
	                return HI_FAILURE;
	            }
        	}
            break;
		case SAMPLE_VI_MODE_2_1080P:
//        case SAMPLE_VI_MODE_4_1080P: //>>>不能按BT10反着设置
        {
        
        	if(AD_TYPE_NVP6134C == Hst3520d_Adapt_Get_CodecType())
            {
	            enMode = SAMPLE_VI_MODE_6134_2MUX_FHD;
	            s32Ret = VI_WISDOM_NVP6134_CfgByGroup(u8Group,enNorm, enMode);
	            if (s32Ret != HI_SUCCESS)
	            {
	                SAMPLE_PRT("VI_WISDOM_NVP6134_CfgV failed with %#x!\n",\
	                        s32Ret);
	                return HI_FAILURE;
	            }
        	}
        	if(AD_TYPE_NVP6124 == Hst3520d_Adapt_Get_CodecType())
            {
	            enMode = SAMPLE_VI_MODE_6124_2MUX_FHD;
				printf("[%s %d] u8Group %d SAMPLE_VI_MODE_6124_2MUX_FHD \n",__func__,__LINE__,u8Group);
	            s32Ret = VI_WISDOM_NVP6124_CfgByGroup(u8Group,enNorm, enMode);
	            if (s32Ret != HI_SUCCESS)
	            {
	                SAMPLE_PRT("VI_WISDOM_NVP6134_CfgV failed with %#x!\n",\
	                        s32Ret);
	                return HI_FAILURE;
	            }
        	}
            break;
        }
        default:
            SAMPLE_PRT("AD not support!\n");
            return HI_FAILURE;
    }
    
    return HI_SUCCESS;
}

/*****************************************************************************
* function : star vi according to product type
*            if vi input is hd, we will start sub-chn for cvbs preview
*****************************************************************************/
HI_S32 SAMPLE_COMM_VI_Start(SAMPLE_VI_MODE_E enViMode, VIDEO_NORM_E enNorm,
    AD_TYPE_EN enType, HI_S32 s32CodecNum)
{
    VI_DEV ViDev;
    VI_CHN ViChn;
    HI_S32 i;
    HI_S32 s32Ret;
    SAMPLE_VI_PARAM_S stViParam;
    SIZE_S stTargetSize;
    RECT_S stCapRect;
    VI_CHN_BIND_ATTR_S stChnBindAttr;
    
    /*** get parameter from Sample_Vi_Mode ***/
    s32Ret = SAMPLE_COMM_VI_Mode2Param(enViMode, &stViParam);
    if (HI_SUCCESS !=s32Ret)
    {
        SAMPLE_PRT("vi get param failed!\n");
        return HI_FAILURE;
    }
    
    s32Ret = SAMPLE_COMM_VI_Mode2Size(enViMode, enNorm, &stCapRect, &stTargetSize);
    if (HI_SUCCESS !=s32Ret)
    {
        SAMPLE_PRT("vi get size failed!\n");
        return HI_FAILURE;
    }
    
    /*** Start AD ***/
	if(AD_TYPE_NVP6134C == enType)
	{
        s32Ret = SAMPLE_WISDOM_VI_ADStart(enViMode, enNorm, s32CodecNum);
	}

	//if(AD_TYPE_NVP6124 == enType)
	//{
    //    s32Ret = SAMPLE_COMM_VI_ADStart(enViMode, enNorm);
	//}

    if (HI_SUCCESS !=s32Ret)
    {
        SAMPLE_PRT("Start AD failed!\n");
        return HI_FAILURE;
    }
    
    /*** Start VI Dev ***/
    for(i=0; i<stViParam.s32ViDevCnt; i++)
    {
        ViDev = i * stViParam.s32ViDevInterval;
        s32Ret = SAMPLE_COMM_VI_StartDev(ViDev, enViMode);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("SAMPLE_COMM_VI_StartDev failed with %#x\n", s32Ret);
            return HI_FAILURE;
        }
    }
    
    /*** Start VI Chn ***/
    for(i=0; i<stViParam.s32ViChnCnt; i++)
    {
        ViChn = i * stViParam.s32ViChnInterval;

        if ((SAMPLE_VI_MODE_8_D1 == enViMode)
            || (SAMPLE_VI_MODE_8_720P == enViMode))
        {
                s32Ret = HI_MPI_VI_GetChnBind(ViChn, &stChnBindAttr);
                if (HI_ERR_VI_FAILED_NOTBIND == s32Ret)
                {
                    stChnBindAttr.ViDev = ViChn/4;
                    stChnBindAttr.ViWay = ViChn%4;
                    SAMPLE_PRT("[ViChn:%d][ViDev:%d][ViWay:%d]\n", ViChn, stChnBindAttr.ViDev, stChnBindAttr.ViWay);

                    s32Ret = HI_MPI_VI_BindChn(ViChn, &stChnBindAttr);
                    if (HI_SUCCESS != s32Ret)
                    {
                        SAMPLE_PRT("call HI_MPI_VI_BindChn failed with %#x\n", s32Ret);
                        return HI_FAILURE;
                    } 
                } 
        }

        if (SAMPLE_VI_MODE_4_D1 == enViMode)
        {
            s32Ret = HI_MPI_VI_GetChnBind(ViChn, &stChnBindAttr);
            if (HI_ERR_VI_FAILED_NOTBIND == s32Ret)
            {
                stChnBindAttr.ViDev = ViChn/2;
                if (ViChn%2 == 0)
                    stChnBindAttr.ViWay = 0;
                else
                    stChnBindAttr.ViWay = 1;    
                s32Ret = HI_MPI_VI_BindChn(ViChn, &stChnBindAttr);
                if (HI_SUCCESS != s32Ret)
                {
                    SAMPLE_PRT("call HI_MPI_VI_BindChn failed with %#x\n", s32Ret);
                    return HI_FAILURE;
                } 
            } 
        }

        if (SAMPLE_VI_MODE_4_720P == enViMode)//add by lianzihao 2016-07-21
        {
            s32Ret = HI_MPI_VI_GetChnBind(ViChn, &stChnBindAttr);
            if (HI_ERR_VI_FAILED_NOTBIND == s32Ret)
            {
                stChnBindAttr.ViDev = ViChn/2;
                if (ViChn%2 == 0)
                    stChnBindAttr.ViWay = 0;
                else
                    stChnBindAttr.ViWay = 1;  
                printf("ViDev %d ViWay %d \n",stChnBindAttr.ViDev,stChnBindAttr.ViWay );
                s32Ret = HI_MPI_VI_BindChn(ViChn, &stChnBindAttr);
                if (HI_SUCCESS != s32Ret)
                {
                    SAMPLE_PRT("call HI_MPI_VI_BindChn failed with %#x\n", s32Ret);
                    return HI_FAILURE;
                } 
            } 
        }
		
        if(SAMPLE_VI_MODE_4_1080P == enViMode)
        {
            s32Ret = HI_MPI_VI_GetChnBind(ViChn, &stChnBindAttr);
            if (HI_ERR_VI_FAILED_NOTBIND == s32Ret)
            {
                stChnBindAttr.ViDev = ViChn/2;
                if (ViChn%2 == 0)
                    stChnBindAttr.ViWay = 0;
                else
                    stChnBindAttr.ViWay = 1;  
                printf("ViChn % ViDev %d ViWay %d \n",ViChn, stChnBindAttr.ViDev,stChnBindAttr.ViWay );
                s32Ret = HI_MPI_VI_BindChn(ViChn, &stChnBindAttr);
                if (HI_SUCCESS != s32Ret)
                {
                    SAMPLE_PRT("call HI_MPI_VI_BindChn failed with %#x\n", s32Ret);
                    return HI_FAILURE;
                } 
            } 
        }
        s32Ret = SAMPLE_COMM_VI_StartChn(ViChn, &stCapRect, &stTargetSize, enViMode, VI_CHN_SET_NORMAL);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("call SAMPLE_COMM_VI_StarChn failed with %#x\n", s32Ret);
            return HI_FAILURE;
        } 
    }

    return HI_SUCCESS;
}
/*****************************************************************************
* function : stop vi accroding to product type
*****************************************************************************/
HI_S32 SAMPLE_COMM_VI_Stop(SAMPLE_VI_MODE_E enViMode)
{
    VI_DEV ViDev;
    VI_CHN ViChn;
    HI_S32 i;
    HI_S32 s32Ret;
    SAMPLE_VI_PARAM_S stViParam;

    /*** get parameter from Sample_Vi_Mode ***/
    s32Ret = SAMPLE_COMM_VI_Mode2Param(enViMode, &stViParam);
    if (HI_SUCCESS !=s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VI_Mode2Param failed!\n");
        return HI_FAILURE;
    }

    /*** Stop VI Chn ***/
    for(i=0;i<stViParam.s32ViChnCnt;i++)
    {
        /* Stop vi phy-chn */
        ViChn = i * stViParam.s32ViChnInterval;
        s32Ret = HI_MPI_VI_DisableChn(ViChn);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("SAMPLE_COMM_VI_StopChn failed with %#x\n",s32Ret);
            return HI_FAILURE;
        }
    }

    /*** Stop VI Dev ***/
    for(i=0; i<stViParam.s32ViDevCnt; i++)
    {
        ViDev = i * stViParam.s32ViDevInterval;
        s32Ret = HI_MPI_VI_DisableDev(ViDev);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("SAMPLE_COMM_VI_StopDev failed with %#x\n", s32Ret);
            return HI_FAILURE;
        }
    }

    return HI_SUCCESS;
}


/*****************************************************************************
* function : Vi chn bind vpss group
*****************************************************************************/
HI_S32 SAMPLE_WISDOM_VI_BindVpss(HI_U8 u8Chn, 
    SAMPLE_VI_MODE_E enViMode, VPSS_CHN VpssChn)
{
	HI_S32 s32Ret;
	VPSS_GRP VpssGrp;
	MPP_CHN_S stSrcChn;
	MPP_CHN_S stDestChn;
	SAMPLE_VI_PARAM_S stViParam;
	VI_CHN ViChn;

	s32Ret = SAMPLE_COMM_VI_Mode2Param(enViMode, &stViParam);
	if (HI_SUCCESS !=s32Ret)
	{
		SAMPLE_PRT("SAMPLE_COMM_VI_Mode2Param failed!\n");
		return HI_FAILURE;
	}

	ViChn = u8Chn * stViParam.s32ViChnInterval;
	VpssGrp = u8Chn;//add by lianzihao 20160518 for vi-->vpss-->vo  CVBS ouput.

	stSrcChn.enModId = HI_ID_VIU;
	stSrcChn.s32DevId = 0;
	stSrcChn.s32ChnId = ViChn;

	stDestChn.enModId = HI_ID_VPSS;
	stDestChn.s32DevId = VpssGrp;
	stDestChn.s32ChnId = VpssChn;

	s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
	if (s32Ret != HI_SUCCESS)
	{
		SAMPLE_PRT("failed with %#x!\n", s32Ret);
		return HI_FAILURE;
	}
    
	return HI_SUCCESS;
}

/*****************************************************************************
* function : Vi chn bind vpss group
*****************************************************************************/
HI_S32 SAMPLE_COMM_VI_BindVpssByGroup(unsigned char u8ViGroup,SAMPLE_VI_MODE_E enViMode, VPSS_CHN VpssChn)
{
	HI_S32 j, s32Ret;
	VPSS_GRP VpssGrp;
	MPP_CHN_S stSrcChn;
	MPP_CHN_S stDestChn;
	SAMPLE_VI_PARAM_S stViParam;
	VI_CHN ViChn;

	s32Ret = SAMPLE_COMM_VI_Mode2Param(enViMode, &stViParam);
	if (HI_SUCCESS !=s32Ret)
	{
		SAMPLE_PRT("SAMPLE_COMM_VI_Mode2Param failed!\n");
		return HI_FAILURE;
	}

	for (j=0; j<stViParam.s32ViChnCnt; j++)
	{
	    //if(HstSdkAL_CheckDevType(DEV_VERSION_BT10))
	    //{
		//    ViChn = j * stViParam.s32ViChnInterval+u8ViGroup*2;
		//    VpssGrp = j+u8ViGroup*2;
		//}
		//else if(HstSdkAL_CheckDevType(DEV_VERSION_BT20))
		{
            ViChn = j * stViParam.s32ViChnInterval+((0 == u8ViGroup)?0:4);
		    VpssGrp = j+((0 == u8ViGroup)?0:4);
		}
		//else
		//    SAMPLE_PRT("设备类型配置!\n");
		
		if(AD_TYPE_NVP6134C == Hst3520d_Adapt_Get_CodecType())
		{
			//VpssGrp = (stViParam.s32ViChnCnt*2-ViChn- 1); //顺序调换一下，硬件原因
            VIDEO_CHN_CFG_S stChnCfg; //20181029/add by tmf/直接采用映射表的Vpss的通道号
            if(-1 != Hst3520d_Adapt_Get_VideoChnCfg((VIDEO_CHN_E)ViChn, &stChnCfg))
            {
                VpssGrp = stChnCfg.s32VpssGrp;
                ViChn = stChnCfg.s32ViChn; //直接使用VI输入对其修正20181030
            }
		}
		
		stSrcChn.enModId = HI_ID_VIU;
		stSrcChn.s32DevId = 0;
		stSrcChn.s32ChnId = ViChn;

		stDestChn.enModId = HI_ID_VPSS;
		stDestChn.s32DevId = VpssGrp;
		stDestChn.s32ChnId = VpssChn;
		SAMPLE_PRT("ViChn(%d)--> VpssGrp(%d)\n", ViChn, VpssGrp);
		s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
		if (s32Ret != HI_SUCCESS)
		{
			SAMPLE_PRT("failed with %#x!\n", s32Ret);
			return HI_FAILURE;
		}
	    
	}
	return HI_SUCCESS;
}


/*****************************************************************************
* function : Vi chn bind vpss group
*****************************************************************************/
HI_S32 SAMPLE_COMM_VI_BindVpss(SAMPLE_VI_MODE_E enViMode,
    VPSS_CHN VpssChn, AD_TYPE_EN enType)
{
	HI_S32 j, s32Ret;
	VPSS_GRP VpssGrp;
	MPP_CHN_S stSrcChn;
	MPP_CHN_S stDestChn;
	SAMPLE_VI_PARAM_S stViParam;
	VI_CHN ViChn;

	s32Ret = SAMPLE_COMM_VI_Mode2Param(enViMode, &stViParam);
	if (HI_SUCCESS !=s32Ret)
	{
		SAMPLE_PRT("SAMPLE_COMM_VI_Mode2Param failed!\n");
		return HI_FAILURE;
	}

	for (j=0; j<stViParam.s32ViChnCnt; j++)
	{
		ViChn = j * stViParam.s32ViChnInterval;
		VpssGrp = j;//add by lianzihao 20160518 for vi-->vpss-->vo  CVBS ouput.

    	if(AD_TYPE_NVP6134C == enType)
        {
            if(4 == stViParam.s32ViChnCnt)
            {
                /***********************************************
                BT10 HST-MV-BT10-H3520的设备由于硬件设计原因需要
                VI与VPSS通道组取反绑定。2018-06-27
                ************************************************/
        		VpssGrp = (stViParam.s32ViChnCnt - j - 1);
            }

		    SAMPLE_PRT("DevId(0)ViChn(%d)--> VpssGrp(%d)VpssChn(%d)\n", ViChn, VpssGrp, VpssChn);
        }

		stSrcChn.enModId = HI_ID_VIU;
		stSrcChn.s32DevId = 0;
		stSrcChn.s32ChnId = ViChn;

		stDestChn.enModId = HI_ID_VPSS;
		stDestChn.s32DevId = VpssGrp;
		stDestChn.s32ChnId = VpssChn;

		s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
		if (s32Ret != HI_SUCCESS)
		{
			SAMPLE_PRT("failed with %#x!\n", s32Ret);
			return HI_FAILURE;
		}
	    
	}
	return HI_SUCCESS;
}


/*****************************************************************************
* function : Vi chn unbind vpss group
*****************************************************************************/
HI_S32 SAMPLE_COMM_VI_UnBindVpss(SAMPLE_VI_MODE_E enViMode, VPSS_CHN VpssChn)
{
    HI_S32 i, j, s32Ret;
    VPSS_GRP VpssGrp;
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;
    SAMPLE_VI_PARAM_S stViParam;
    VI_DEV ViDev;
    VI_CHN ViChn;

    s32Ret = SAMPLE_COMM_VI_Mode2Param(enViMode, &stViParam);
    if (HI_SUCCESS !=s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VI_Mode2Param failed!\n");
        return HI_FAILURE;
    }
    
    VpssGrp = 0;    
    for (i=0; i<stViParam.s32ViDevCnt; i++)
    {
        ViDev = i * stViParam.s32ViDevInterval;

        for (j=0; j<stViParam.s32ViChnCnt; j++)
        {
            ViChn = j * stViParam.s32ViChnInterval;
            
            stSrcChn.enModId = HI_ID_VIU;
            stSrcChn.s32DevId = ViDev;
            stSrcChn.s32ChnId = ViChn;
        
            stDestChn.enModId = HI_ID_VPSS;
            stDestChn.s32DevId = VpssGrp;
            stDestChn.s32ChnId = VpssChn;
        
            s32Ret = HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
            if (s32Ret != HI_SUCCESS)
            {
                SAMPLE_PRT("failed with %#x!\n", s32Ret);
                return HI_FAILURE;
            }
            
            VpssGrp ++;
        }
    }
    return HI_SUCCESS;
}

/******************************************************************************
* function : read frame
******************************************************************************/
HI_VOID SAMPLE_COMM_VI_ReadFrame(FILE * fp, HI_U8 * pY, HI_U8 * pU, HI_U8 * pV, HI_U32 width, HI_U32 height, HI_U32 stride, HI_U32 stride2)
{
    HI_U8 * pDst;

    HI_U32 u32Row;

    pDst = pY;
    for ( u32Row = 0; u32Row < height; u32Row++ )
    {
        fread( pDst, width, 1, fp );
        pDst += stride;
    }

    pDst = pU;
    for ( u32Row = 0; u32Row < height/2; u32Row++ )
    {
        fread( pDst, width/2, 1, fp );
        pDst += stride2;
    }

    pDst = pV;
    for ( u32Row = 0; u32Row < height/2; u32Row++ )
    {
        fread( pDst, width/2, 1, fp );
        pDst += stride2;
    }
}
 
/******************************************************************************
* function : Plan to Semi
******************************************************************************/
HI_S32 SAMPLE_COMM_VI_PlanToSemi(HI_U8 *pY, HI_S32 yStride,
                       HI_U8 *pU, HI_S32 uStride,
                       HI_U8 *pV, HI_S32 vStride,
                       HI_S32 picWidth, HI_S32 picHeight)
{
    HI_S32 i;
    HI_U8* pTmpU, *ptu;
    HI_U8* pTmpV, *ptv;
    HI_S32 s32HafW = uStride >>1 ;
    HI_S32 s32HafH = picHeight >>1 ;
    HI_S32 s32Size = s32HafW*s32HafH;

    pTmpU = malloc( s32Size ); ptu = pTmpU;
    pTmpV = malloc( s32Size ); ptv = pTmpV;

    memcpy(pTmpU,pU,s32Size);
    memcpy(pTmpV,pV,s32Size);

    for(i = 0;i<s32Size>>1;i++)
    {
        *pU++ = *pTmpV++;
        *pU++ = *pTmpU++;

    }
    for(i = 0;i<s32Size>>1;i++)
    {
        *pV++ = *pTmpV++;
        *pV++ = *pTmpU++;
    }

    free( ptu );
    free( ptv );

    return HI_SUCCESS;
}

/******************************************************************************
* function : Get from YUV
******************************************************************************/
HI_S32 SAMPLE_COMM_VI_GetVFrameFromYUV(FILE *pYUVFile, HI_U32 u32Width, HI_U32 u32Height,HI_U32 u32Stride, VIDEO_FRAME_INFO_S *pstVFrameInfo)
{
    HI_U32             u32LStride;
    HI_U32             u32CStride;
    HI_U32             u32LumaSize;
    HI_U32             u32ChrmSize;
    HI_U32             u32Size;
    VB_BLK VbBlk;
    HI_U32 u32PhyAddr;
    HI_U8 *pVirAddr;

    u32LStride  = u32Stride;
    u32CStride  = u32Stride;

    u32LumaSize = (u32LStride * u32Height);
    u32ChrmSize = (u32CStride * u32Height) >> 2;/* YUV 420 */
    u32Size = u32LumaSize + (u32ChrmSize << 1);

    /* alloc video buffer block ---------------------------------------------------------- */
    VbBlk = HI_MPI_VB_GetBlock(VB_INVALID_POOLID, u32Size, NULL);
    if (VB_INVALID_HANDLE == VbBlk)
    {
        SAMPLE_PRT("HI_MPI_VB_GetBlock err! size:%d\n",u32Size);
        return -1;
    }
    u32PhyAddr = HI_MPI_VB_Handle2PhysAddr(VbBlk);
    if (0 == u32PhyAddr)
    {
        return -1;
    }

    pVirAddr = (HI_U8 *) HI_MPI_SYS_Mmap(u32PhyAddr, u32Size);
    if (NULL == pVirAddr)
    {
        return -1;
    }

    pstVFrameInfo->u32PoolId = HI_MPI_VB_Handle2PoolId(VbBlk);
    if (VB_INVALID_POOLID == pstVFrameInfo->u32PoolId)
    {
        return -1;
    }
    SAMPLE_PRT("pool id :%d, phyAddr:%x,virAddr:%x\n" ,pstVFrameInfo->u32PoolId,u32PhyAddr,(int)pVirAddr);

    pstVFrameInfo->stVFrame.u32PhyAddr[0] = u32PhyAddr;
    pstVFrameInfo->stVFrame.u32PhyAddr[1] = pstVFrameInfo->stVFrame.u32PhyAddr[0] + u32LumaSize;
    pstVFrameInfo->stVFrame.u32PhyAddr[2] = pstVFrameInfo->stVFrame.u32PhyAddr[1] + u32ChrmSize;

    pstVFrameInfo->stVFrame.pVirAddr[0] = pVirAddr;
    pstVFrameInfo->stVFrame.pVirAddr[1] = pstVFrameInfo->stVFrame.pVirAddr[0] + u32LumaSize;
    pstVFrameInfo->stVFrame.pVirAddr[2] = pstVFrameInfo->stVFrame.pVirAddr[1] + u32ChrmSize;

    pstVFrameInfo->stVFrame.u32Width  = u32Width;
    pstVFrameInfo->stVFrame.u32Height = u32Height;
    pstVFrameInfo->stVFrame.u32Stride[0] = u32LStride;
    pstVFrameInfo->stVFrame.u32Stride[1] = u32CStride;
    pstVFrameInfo->stVFrame.u32Stride[2] = u32CStride;
    pstVFrameInfo->stVFrame.enPixelFormat = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
    pstVFrameInfo->stVFrame.u32Field = VIDEO_FIELD_INTERLACED;/* Intelaced D1,otherwise VIDEO_FIELD_FRAME */

    /* read Y U V data from file to the addr ----------------------------------------------*/
    SAMPLE_COMM_VI_ReadFrame(pYUVFile, pstVFrameInfo->stVFrame.pVirAddr[0],
       pstVFrameInfo->stVFrame.pVirAddr[1], pstVFrameInfo->stVFrame.pVirAddr[2],
       pstVFrameInfo->stVFrame.u32Width, pstVFrameInfo->stVFrame.u32Height,
       pstVFrameInfo->stVFrame.u32Stride[0], pstVFrameInfo->stVFrame.u32Stride[1] >> 1 );

    /* convert planar YUV420 to sem-planar YUV420 -----------------------------------------*/
    SAMPLE_COMM_VI_PlanToSemi(pstVFrameInfo->stVFrame.pVirAddr[0], pstVFrameInfo->stVFrame.u32Stride[0],
      pstVFrameInfo->stVFrame.pVirAddr[1], pstVFrameInfo->stVFrame.u32Stride[1],
      pstVFrameInfo->stVFrame.pVirAddr[2], pstVFrameInfo->stVFrame.u32Stride[1],
      pstVFrameInfo->stVFrame.u32Width, pstVFrameInfo->stVFrame.u32Height);

    HI_MPI_SYS_Munmap(pVirAddr, u32Size);
    return 0;
}

HI_S32 SAMPLE_COMM_VI_ChangeCapSize(VI_CHN ViChn, HI_U32 u32CapWidth, HI_U32 u32CapHeight,HI_U32 u32Width, HI_U32 u32Height)
{
    VI_CHN_ATTR_S stChnAttr;
	HI_S32 S32Ret = HI_SUCCESS;
	S32Ret = HI_MPI_VI_GetChnAttr(ViChn, &stChnAttr);
	if(HI_SUCCESS!= S32Ret)
	{
	    SAMPLE_PRT( "HI_MPI_VI_GetChnAttr failed\n");
	}
    stChnAttr.stCapRect.u32Width = u32CapWidth;
    stChnAttr.stCapRect.u32Height = u32CapHeight;
    stChnAttr.stDestSize.u32Width = u32Width;
    stChnAttr.stDestSize.u32Height = u32Height;

    S32Ret = HI_MPI_VI_SetChnAttr(ViChn, &stChnAttr);
	if(HI_SUCCESS!= S32Ret)
	{
	    SAMPLE_PRT( "HI_MPI_VI_SetChnAttr failed\n");
	}

    return HI_SUCCESS;
}

HI_S32 SAMPLE_COMM_VI_ChangeDestSize(VI_CHN ViChn, HI_U32 u32Width, HI_U32 u32Height)
{
    VI_CHN_ATTR_S stChnAttr;
	HI_S32 S32Ret = HI_SUCCESS;
	S32Ret = HI_MPI_VI_GetChnAttr(ViChn, &stChnAttr);
	if(HI_SUCCESS!= S32Ret)
	{
	    SAMPLE_PRT( "HI_MPI_VI_GetChnAttr failed\n");
	}
    stChnAttr.stDestSize.u32Width = u32Width;
    stChnAttr.stDestSize.u32Height = u32Height;

    S32Ret = HI_MPI_VI_SetChnAttr(ViChn, &stChnAttr);
	if(HI_SUCCESS!= S32Ret)
	{
	    SAMPLE_PRT( "HI_MPI_VI_SetChnAttr failed\n");
	}

    return HI_SUCCESS;
}


/*****************************************************************************
* function : star vi according to product type
*           only for Hi3521 MixCap 
*****************************************************************************/
HI_S32 SAMPLE_COMM_VI_MixCap_Start(SAMPLE_VI_MODE_E enViMode, VIDEO_NORM_E enNorm)
{
    VI_DEV ViDev;
    VI_CHN ViChn;
    HI_S32 i;
    HI_S32 s32Ret;
    SAMPLE_VI_PARAM_S stViParam;
    VI_CHN_ATTR_S stChnAttr,stChnMinorAttr;
    VI_CHN_BIND_ATTR_S stChnBindAttr;
	
    /*** get parameter from Sample_Vi_Mode ***/
    s32Ret = SAMPLE_COMM_VI_Mode2Param(enViMode, &stViParam);
    if (HI_SUCCESS !=s32Ret)
    {
        SAMPLE_PRT("vi get param failed!\n");
        return HI_FAILURE;
    }
    
    /*** Start AD ***/
    s32Ret = SAMPLE_COMM_VI_ADStart(enViMode, enNorm);
    if (HI_SUCCESS !=s32Ret)
    {
        SAMPLE_PRT("Start AD failed!\n");
        return HI_FAILURE;
    }
    
    /*** Start VI Dev ***/
    for(i=0; i<stViParam.s32ViDevCnt; i++)
    {
        ViDev = i * stViParam.s32ViDevInterval;
        s32Ret = SAMPLE_COMM_VI_StartDev(ViDev, enViMode);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("SAMPLE_COMM_VI_StartDev failed with %#x\n", s32Ret);
            return HI_FAILURE;
        }
    }
    
    /*** Start VI Chn ***/
    for(i=0; i<stViParam.s32ViChnCnt; i++)
    {
        
        ViChn = i * stViParam.s32ViChnInterval;

         if (SAMPLE_VI_MODE_8_1080P == enViMode
         || SAMPLE_VI_MODE_8_720P == enViMode)
        {
            /* When in the 8x1080p mode, bind chn 2,6,10,14 to way1 is needed */
            if (ViChn%4 != 0)
            {
                s32Ret = HI_MPI_VI_GetChnBind(ViChn, &stChnBindAttr);
                if (HI_ERR_VI_FAILED_NOTBIND == s32Ret)
                {
                    stChnBindAttr.ViDev = ViChn/4;
                    stChnBindAttr.ViWay = 1;
                    s32Ret = HI_MPI_VI_BindChn(ViChn, &stChnBindAttr);
                    if (HI_SUCCESS != s32Ret)
                    {
                        SAMPLE_PRT("call HI_MPI_VI_BindChn failed with %#x\n", s32Ret);
                        return HI_FAILURE;
                    } 
                } 
            }
        }

	    stChnAttr.stCapRect.s32X = 0;
	    stChnAttr.stCapRect.s32Y = 0;
	    stChnAttr.enCapSel = VI_CAPSEL_BOTH;                      
	    stChnAttr.enPixFormat = SAMPLE_PIXEL_FORMAT;   /* sp420 or sp422 */
	    stChnAttr.bMirror = HI_FALSE;                             
	    stChnAttr.bFlip   = HI_FALSE;  
		stChnAttr.stCapRect.u32Width= HD_WIDTH;
	    stChnAttr.stCapRect.u32Height= 1080;	
	    stChnAttr.stDestSize.u32Width= HD_WIDTH;
	    stChnAttr.stDestSize.u32Height= 1080;
        stChnAttr.enScanMode = VI_SCAN_PROGRESSIVE;
		
		memcpy(&stChnMinorAttr, &stChnAttr, sizeof(VI_CHN_ATTR_S));
	    stChnMinorAttr.stDestSize.u32Width= HD_WIDTH / 2;
	    stChnMinorAttr.stDestSize.u32Height= 540;

		stChnAttr.s32SrcFrameRate = 30;
		stChnAttr.s32DstFrameRate = 10;
		
		s32Ret = HI_MPI_VI_SetChnAttr(ViChn, &stChnAttr);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("call HI_MPI_VI_SetChnAttr failed with %#x\n", s32Ret);
            return HI_FAILURE;
        } 
		s32Ret = HI_MPI_VI_SetChnMinorAttr(ViChn, &stChnMinorAttr);
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("call HI_MPI_VI_SetChnMinorAttr failed with %#x\n", s32Ret);
			return HI_FAILURE;
		} 


        s32Ret = HI_MPI_VI_SetChnMinorAttr(ViChn, &stChnMinorAttr);
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("call HI_MPI_VI_SetChnMinorAttr failed with %#x\n", s32Ret);
			return HI_FAILURE;
		} 
		s32Ret = HI_MPI_VI_EnableChn(ViChn);
		if (s32Ret != HI_SUCCESS)
		{
			SAMPLE_PRT("HI_MPI_VI_EnableChn failed with %#x!\n", s32Ret);
			return HI_FAILURE;
		}

    }

    return HI_SUCCESS;
}

HI_S32 SAMPLE_COMM_VI_ChangeMixCap(VI_CHN ViChn,HI_BOOL bMixCap,HI_U32 FrameRate)
{
    VI_CHN_ATTR_S stChnAttr,stChnMinorAttr;
	HI_S32 S32Ret = HI_SUCCESS;
	S32Ret = HI_MPI_VI_GetChnAttr(ViChn, &stChnAttr);
	if(HI_SUCCESS!= S32Ret)
	{
	    SAMPLE_PRT( "HI_MPI_VI_GetChnAttr failed");
	}
	
	if(HI_TRUE == bMixCap)
	{
		memcpy(&stChnMinorAttr, &stChnAttr, sizeof(VI_CHN_ATTR_S));
	    stChnMinorAttr.stDestSize.u32Width = D1_WIDTH / 2;
		
		stChnAttr.s32DstFrameRate = FrameRate;
		
		S32Ret = HI_MPI_VI_SetChnAttr(ViChn, &stChnAttr);
        if (HI_SUCCESS != S32Ret)
        {
            SAMPLE_PRT("call HI_MPI_VI_SetChnAttr failed with %#x\n", S32Ret);
            return HI_FAILURE;
        } 
		S32Ret = HI_MPI_VI_SetChnMinorAttr(ViChn, &stChnMinorAttr);
		if (HI_SUCCESS != S32Ret)
		{
			SAMPLE_PRT("call HI_MPI_VI_SetChnMinorAttr failed with %#x\n", S32Ret);
			return HI_FAILURE;
		} 
	}
	else
	{
		stChnAttr.s32DstFrameRate = stChnAttr.s32SrcFrameRate;
		S32Ret = HI_MPI_VI_SetChnAttr(ViChn, &stChnAttr);
        if (HI_SUCCESS != S32Ret)
        {
            SAMPLE_PRT("call HI_MPI_VI_SetChnAttr failed with %#x\n", S32Ret);
            return HI_FAILURE;
        } 
	}
	return HI_SUCCESS;
}
	

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
