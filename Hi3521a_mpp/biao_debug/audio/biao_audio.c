/************************************************************
*Copyright (C),lcb0281at163.com lcb0281atgmail.com
*BlogAddr: caibiao-lee.blog.csdn.net
*FileName: biao_audio.c
*Description:“Ù∆µ±‡¬Î≤‚ ‘
*Date:     2020-02-03
*Author:   Caibiao Lee
*Version:  V1.0
*Others:
*History:
***********************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>

#include "sample_comm.h"
#include "tlv320aic31.h"
#include "biao_audio.h"

static PAYLOAD_TYPE_E gs_enPayloadType = PT_ADPCMA;

static HI_BOOL gs_bAiVqe= HI_FALSE;
static HI_BOOL gs_bAioReSample = HI_FALSE;
static HI_BOOL gs_bUserGetMode = HI_FALSE;
static HI_BOOL gs_bAoVolumeCtrl = HI_TRUE;
static AUDIO_RESAMPLE_ATTR_S *gs_pstAiReSmpAttr = NULL;
static AUDIO_RESAMPLE_ATTR_S *gs_pstAoReSmpAttr = NULL;
static HI_U32 g_u32AiCnt = 0;
static HI_U32 g_u32AoCnt = 0;
static HI_U32 g_u32AencCnt = 0;
static HI_U32 g_u32Adec = 0;
static HI_U32 g_u32AiDev = 0;
static HI_U32 g_u32AoDev = 0;

#define SAMPLE_DBG(s32Ret)\
do{\
    printf("s32Ret=%#x,fuc:%s,line:%d\n", s32Ret, __FUNCTION__, __LINE__);\
}while(0)

/******************************************************************************
* function : PT Number to String
******************************************************************************/
static char* BIAO_AUDIO_Pt2Str(PAYLOAD_TYPE_E enType)
{
    if (PT_G711A == enType)  
    {
        return "g711a";
    }
    else if (PT_G711U == enType)  
    {
        return "g711u";
    }
    else if (PT_ADPCMA == enType)  
    {
        return "adpcm";
    }
    else if (PT_G726 == enType) 
    {
        return "g726";
    }
    else if (PT_LPCM == enType)  
    {
        return "pcm";
    }
    else 
    {
        return "data";
    }
}

/******************************************************************************
* function : Open Aenc File
******************************************************************************/
static FILE * BIAO_AUDIO_OpenAencFile(AENC_CHN AeChn, PAYLOAD_TYPE_E enType)
{
    FILE *pfd;
    HI_CHAR aszFileName[128];
    
    /* create file for save stream*/
    sprintf(aszFileName, "audio_chn%d.%s", AeChn, BIAO_AUDIO_Pt2Str(enType));
    pfd = fopen(aszFileName, "w+");
    if (NULL == pfd)
    {
        printf("%s: open file %s failed\n", __FUNCTION__, aszFileName);
        return NULL;
    }
    printf("open stream file:\"%s\" for aenc ok\n", aszFileName);
    return pfd;
}



void BIAO_AUDIO_HandleSig(HI_S32 signo)
{
    if (SIGINT == signo || SIGTSTP == signo)
    {
        SAMPLE_COMM_AUDIO_DestoryAllTrd();

		SAMPLE_COMM_AUDIO_StopAi(g_u32AiDev, g_u32AiCnt, gs_bAioReSample, HI_FALSE);
    
    	SAMPLE_COMM_AUDIO_StopAo(g_u32AoDev, g_u32AoCnt, gs_bAioReSample);

		SAMPLE_COMM_AUDIO_StopAdec(g_u32Adec);

		SAMPLE_COMM_AUDIO_StopAenc(g_u32AencCnt);
	
        SAMPLE_COMM_SYS_Exit();
        printf("\033[0;31mprogram exit abnormally!\033[0;39m\n");
    }

    exit(0);
}


/******************************************************************************
* function : Ai -> Aenc -> file
*                       -> Adec -> Ao
******************************************************************************/
HI_S32 BIAO_AUDIO_AiAenc(HI_VOID)
{
    HI_S32 i, s32Ret;
    AUDIO_DEV   AiDev = SAMPLE_AUDIO_AI_DEV;
    AI_CHN      AiChn;
    AUDIO_DEV   AoDev = SAMPLE_AUDIO_AO_DEV;
    AO_CHN      AoChn = 0;
    ADEC_CHN    AdChn = 0;
    HI_S32      s32AiChnCnt;
    HI_S32      s32AencChnCnt;
    AENC_CHN    AeChn;
    HI_BOOL     bSendAdec = HI_TRUE;
    FILE        *pfd = NULL;
    AIO_ATTR_S stAioAttr;
	
    stAioAttr.enSamplerate   = AUDIO_SAMPLE_RATE_8000;
    stAioAttr.enBitwidth     = AUDIO_BIT_WIDTH_16;
    stAioAttr.enWorkmode     = AIO_MODE_I2S_MASTER;
    stAioAttr.enSoundmode    = AUDIO_SOUND_MODE_MONO;
    stAioAttr.u32EXFlag      = 1;
    stAioAttr.u32FrmNum      = 30;
    stAioAttr.u32PtNumPerFrm = SAMPLE_AUDIO_PTNUMPERFRM;
    stAioAttr.u32ChnCnt      = 1;
    stAioAttr.u32ClkChnCnt   = 2;
    stAioAttr.u32ClkSel      = 0;      

    /********************************************
    step 1: config audio codec
    ********************************************/
    s32Ret = SAMPLE_COMM_AUDIO_CfgAcodec(&stAioAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_DBG(s32Ret);
        return HI_FAILURE;
    }

    /********************************************
    step 2: start Ai
    ********************************************/
    s32AiChnCnt = stAioAttr.u32ChnCnt; 
    s32AencChnCnt = 1;//s32AiChnCnt;
    g_u32AiCnt = s32AiChnCnt;
    g_u32AiDev = AiDev;
    s32Ret = SAMPLE_COMM_AUDIO_StartAi(AiDev, s32AiChnCnt, &stAioAttr, AUDIO_SAMPLE_RATE_BUTT, HI_FALSE, NULL);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_DBG(s32Ret);
        return HI_FAILURE;
    }

    /********************************************
    step 3: start Aenc
    ********************************************/
    g_u32AencCnt = s32AencChnCnt;
    s32Ret = SAMPLE_COMM_AUDIO_StartAenc(s32AencChnCnt, SAMPLE_AUDIO_PTNUMPERFRM, gs_enPayloadType);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_DBG(s32Ret);
        return HI_FAILURE;
    }

    /********************************************
    step 4: Aenc bind Ai Chn
    ********************************************/
    for (i=0; i<s32AencChnCnt; i++)
    {
        AeChn = i;
        AiChn = i;

        if (HI_TRUE == gs_bUserGetMode)
        {
            s32Ret = SAMPLE_COMM_AUDIO_CreatTrdAiAenc(AiDev, AiChn, AeChn);
            if (s32Ret != HI_SUCCESS)
            {
                SAMPLE_DBG(s32Ret);
                return HI_FAILURE;
            }
        }
        else
        {        
            s32Ret = SAMPLE_COMM_AUDIO_AencBindAi(AiDev, AiChn, AeChn);
            if (s32Ret != HI_SUCCESS)
            {
                SAMPLE_DBG(s32Ret);
                return s32Ret;
            }
        }
        printf("Ai(%d,%d) bind to AencChn:%d ok!\n",AiDev , AiChn, AeChn);
    }

    /********************************************
    step 5: start Adec & Ao. ( if you want )
    ********************************************/
    if (HI_TRUE == bSendAdec)
    {
        g_u32Adec = AdChn;
        s32Ret = SAMPLE_COMM_AUDIO_StartAdec(AdChn, gs_enPayloadType);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_DBG(s32Ret);
            return HI_FAILURE;
        }

        g_u32AoCnt = s32AiChnCnt;
        g_u32AoDev = AoDev;
        s32Ret = SAMPLE_COMM_AUDIO_StartAo(AoDev, s32AiChnCnt, &stAioAttr, AUDIO_SAMPLE_RATE_BUTT, HI_FALSE);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_DBG(s32Ret);
            return HI_FAILURE;
        }

        pfd = BIAO_AUDIO_OpenAencFile(AdChn, gs_enPayloadType);
        if (!pfd)
        {
            SAMPLE_DBG(HI_FAILURE);
            return HI_FAILURE;
        }        
        s32Ret = SAMPLE_COMM_AUDIO_CreatTrdAencAdec(AeChn, AdChn, pfd);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_DBG(s32Ret);
            return HI_FAILURE;
        }

        s32Ret = SAMPLE_COMM_AUDIO_AoBindAdec(AoDev, AoChn, AdChn);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_DBG(s32Ret);
            return HI_FAILURE;
        }

        printf("bind adec:%d to ao(%d,%d) ok \n", AdChn, AoDev, AoChn);
     }

    printf("\nplease press twice ENTER to exit this sample\n");
    getchar();
    getchar();

    /********************************************
    step 6: exit the process
    ********************************************/
    if (HI_TRUE == bSendAdec)
    {
        s32Ret = SAMPLE_COMM_AUDIO_DestoryTrdAencAdec(AdChn);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_DBG(s32Ret);
            return HI_FAILURE;
        }
        
        s32Ret = SAMPLE_COMM_AUDIO_StopAo(AoDev, s32AiChnCnt, HI_FALSE);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_DBG(s32Ret);
            return HI_FAILURE;
        }
        
        s32Ret = SAMPLE_COMM_AUDIO_StopAdec(AdChn);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_DBG(s32Ret);
            return HI_FAILURE;
        }
        
        s32Ret = SAMPLE_COMM_AUDIO_AoUnbindAdec(AoDev, AoChn, AdChn);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_DBG(s32Ret);
            return HI_FAILURE;
        }
    }
    
    for (i=0; i<s32AencChnCnt; i++)
    {
        AeChn = i;
        AiChn = i;

        if (HI_TRUE == gs_bUserGetMode)
        {
            s32Ret = SAMPLE_COMM_AUDIO_DestoryTrdAi(AiDev, AiChn);
            if (s32Ret != HI_SUCCESS)
            {
                SAMPLE_DBG(s32Ret);
                return HI_FAILURE;
            }
        }
        else
        {        
            s32Ret = SAMPLE_COMM_AUDIO_AencUnbindAi(AiDev, AiChn, AeChn);
            if (s32Ret != HI_SUCCESS)
            {
                SAMPLE_DBG(s32Ret);
                return HI_FAILURE;
            }
        }
    }
    
    s32Ret = SAMPLE_COMM_AUDIO_StopAenc(s32AencChnCnt);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_DBG(s32Ret);
        return HI_FAILURE;
    }
    
    s32Ret = SAMPLE_COMM_AUDIO_StopAi(AiDev, s32AiChnCnt, HI_FALSE, HI_FALSE);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_DBG(s32Ret);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}


int BIAO_AUDIO_DEBUG(void)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR ch;
    HI_BOOL bExit = HI_FALSE;
    VB_CONF_S stVbConf;
    
    signal(SIGINT, BIAO_AUDIO_HandleSig);
    signal(SIGTERM, BIAO_AUDIO_HandleSig);
    
    memset(&stVbConf, 0, sizeof(VB_CONF_S));
    s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);

    BIAO_AUDIO_AiAenc();

    SAMPLE_COMM_SYS_Exit();
    
    return s32Ret;

}





