#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hi_common.h"
#include "hi_comm_video.h"
#include "hi_comm_sys.h"
#include "hi_comm_vo.h"
#include "hi_comm_rc.h"
#include "hi_comm_venc.h"

#include "mpi_vb.h"
#include "mpi_sys.h"
#include "mpi_vi.h"
#include "mpi_vo.h"
#include "mpi_venc.h"

#define USAGE_HELP(void)\
{\
    printf("\n\tusage : %s group para value\n", argv[0]);    \
    printf("\n\t para: \n");    \
    printf("\tstattime   [统计时间,单位:秒\n");   \
    printf("\t bitrate  码率，单位:kbps\n");    \
    printf("\tgop  两个I帧的间隔，单位:帧\n");   \
    printf("\tqpdelta I帧与P帧间的QP差异，差异越大整体效果好但呼吸效应严重\n");    \
    printf("\tmqpdelta 在宏块级码率控制时，每一行宏块的起始Qp相对于帧起始Qp的波动幅度值\n");    \
    printf("\tmaxqp     最大QP约束\n");   \
}

#define CHECK_RET(express,name)\
    do{\
        if (HI_SUCCESS != express)\
        {\
            printf("%s failed at %s: LINE: %d ! errno:%d \n",\
                name, __FUNCTION__, __LINE__, express);\
            return HI_FAILURE;\
        }\
    }while(0)


HI_S32 main(int argc, char *argv[])
{
    HI_S32 s32Ret;
	HI_S32 s32EncChn;	
    HI_CHAR para[16];
    HI_U32 value = 0;
	HI_U32 i;
	VENC_RC_MODE_E eMode;
	
	VENC_RC_PARAM_S stVencRcParam;
	VENC_CHN_ATTR_S stVencChnAttr;
	
	if(argc<3)
	{
		USAGE_HELP();
		return -1;
	}
	s32EncChn=0;
    s32EncChn = atoi(argv[1]);
    strcpy(para,argv[2]);  
	
	s32Ret=HI_MPI_VENC_GetRcParam(s32EncChn,&stVencRcParam);	
	CHECK_RET(s32Ret,"get Rc param");
	s32Ret=HI_MPI_VENC_GetChnAttr(s32EncChn,&stVencChnAttr);
	CHECK_RET(s32Ret,"get Rc Attr");

	eMode=stVencChnAttr.stRcAttr.enRcMode;


	if (0 == strcmp(para, "?"))
    {
       	if(eMode==VENC_RC_MODE_H264CBR)
    	{ 
			printf("\tstattime %d\n",stVencChnAttr.stRcAttr.stAttrH264Cbr.u32StatTime);
		  	printf("\tbitrate %d\n",stVencChnAttr.stRcAttr.stAttrH264Cbr.u32BitRate);
			printf("\tgop %d\n",stVencChnAttr.stRcAttr.stAttrH264Cbr.u32Gop);
			printf("\tqpdelta %d\n",stVencRcParam.stParamH264Cbr.s32IPQPDelta);            
			printf("\tmaxqp %d\n",stVencRcParam.stParamH264Cbr.u32MaxQp);		
       	}
		else
		{
			printf("\tstattime %d\n",stVencChnAttr.stRcAttr.stAttrH265Cbr.u32StatTime);
		  	printf("\tbitrate %d\n",stVencChnAttr.stRcAttr.stAttrH265Cbr.u32BitRate);
			printf("\tgop %d\n",stVencChnAttr.stRcAttr.stAttrH265Cbr.u32Gop);
			printf("\tqpdelta %d\n",stVencRcParam.stParamH265Cbr.s32IPQPDelta);
			printf("\tmaxqp %d\n",stVencRcParam.stParamH265Cbr.u32MaxQp);				
		}

        printf("\tmqpdelta %d\n",stVencRcParam.u32RowQpDelta);
        return 0;
    }
    value = atoi(argv[3]);	
	printf("chn %d, para %s, value %d\n",s32EncChn,para,value);	
    if (0 == strcmp(para, "stattime"))
    {
    	if(eMode==VENC_RC_MODE_H264CBR)
    	{
    		stVencChnAttr.stRcAttr.stAttrH264Cbr.u32StatTime=value;
    	}
		else
		{
			stVencChnAttr.stRcAttr.stAttrH265Cbr.u32StatTime=value;
		}
    }
	if (0 ==  strcmp(para,"thresh") && argc==15)		
	{
		for(i=0;i<12;i++)
		{
			stVencRcParam.u32ThrdI[i]= atoi(argv[i+3]);
			stVencRcParam.u32ThrdP[i]= atoi(argv[i+3]);
		}
	}
	if (0 ==  strcmp(para,"pthresh") && argc==15)		
	{
		for(i=0;i<12;i++)
		{
			stVencRcParam.u32ThrdP[i]= atoi(argv[i+3]);
		}
	}	
    else if (0 == strcmp(para, "bitrate"))
    {
       	if(eMode==VENC_RC_MODE_H264CBR)
    	{
    		stVencChnAttr.stRcAttr.stAttrH264Cbr.u32BitRate=value;
    	}
		else
		{ 
      		stVencChnAttr.stRcAttr.stAttrH265Cbr.u32BitRate=value; 
		}
    }
	else if( 0== strcmp(para, "framerate"))
	{
       	if(eMode==VENC_RC_MODE_H264CBR)
    	{
    		stVencChnAttr.stRcAttr.stAttrH264Cbr.fr32DstFrmRate=value;
    	}
		else
		{ 	
			stVencChnAttr.stRcAttr.stAttrH265Cbr.fr32DstFrmRate=value;
		}
	}
    else if (0 == strcmp(para, "gop"))
    {
       	if(eMode==VENC_RC_MODE_H264CBR)
    	{
    		stVencChnAttr.stRcAttr.stAttrH264Cbr.u32Gop=value;
    	}
		else
		{ 

    	  stVencChnAttr.stRcAttr.stAttrH265Cbr.u32Gop=value; 
		}
    }
    else if (0 == strcmp(para, "flut"))
    {
        if(eMode==VENC_RC_MODE_H264CBR)
    	{
    		stVencChnAttr.stRcAttr.stAttrH264Cbr.u32FluctuateLevel=value;
    	}
		else
		{    
      		stVencChnAttr.stRcAttr.stAttrH265Cbr.u32FluctuateLevel=value; 
		}
    }
    else if (0 == strcmp(para, "qpdelta"))
    {
        if(eMode==VENC_RC_MODE_H264CBR)
    	{
    		stVencRcParam.stParamH264Cbr.s32IPQPDelta=value;
    	}
		else
		{       
      		stVencRcParam.stParamH265Cbr.s32IPQPDelta=value;
		}
    }
    else if (0 == strcmp(para, "maxqp"))
    {
        if(eMode==VENC_RC_MODE_H264CBR)
    	{
			stVencRcParam.stParamH264Cbr.u32MaxQp=value;  

    	}
		else
		{       
	      stVencRcParam.stParamH265Cbr.u32MaxQp=value;	
		}
    }
    else if (0 == strcmp(para, "level"))
    {
        if(eMode==VENC_RC_MODE_H264CBR)
    	{
			stVencRcParam.stParamH264Cbr.s32QualityLevel=value;   
    	}
		else
		{      
      		stVencRcParam.stParamH265Cbr.s32QualityLevel=value;	  
		}
    }
    else if (0 == strcmp(para, "mqpdelta"))
    {
        stVencRcParam.u32RowQpDelta = value;
    }
	else if( 0 ==strcmp(para,"deblock"))
	{
		VENC_PARAM_H264_DBLK_S stH264Dblk;
		s32Ret=HI_MPI_VENC_GetH264Dblk(0,&stH264Dblk);
		CHECK_RET(s32Ret,"get deblock");
		stH264Dblk.disable_deblocking_filter_idc=2;
		stH264Dblk.slice_alpha_c0_offset_div2=value;
		stH264Dblk.slice_beta_offset_div2=value;
		s32Ret=HI_MPI_VENC_SetH264Dblk(0,&stH264Dblk);
		CHECK_RET(s32Ret,"get deblock");		
	}
    
	s32Ret=HI_MPI_VENC_SetChnAttr(s32EncChn,&stVencChnAttr);
	CHECK_RET(s32Ret,"set Chn Attr");

	s32Ret=HI_MPI_VENC_SetRcParam(s32EncChn,&stVencRcParam);
	CHECK_RET(s32Ret,"set Rc param");

    return 0;
}

