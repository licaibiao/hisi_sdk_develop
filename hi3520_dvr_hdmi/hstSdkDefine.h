#ifndef _HST_SDK_DEFINE_H
#define _HST_SDK_DEFINE_H

#define	HST_SUCCEED         (0)
#define	HST_FAILURE         (-1)

#define	AHD_PHY_CHN_NUM     (8) //当前SDK支持AHD通道数

#define	AHD_STORE_CHN_NUM   (AHD_PHY_CHN_NUM)   //用于本地存储码流视频编码通道数量
#define	AHD_SEND_CHN_NUM	(AHD_PHY_CHN_NUM)   //用于网传码流视频编码通道数量
#define	AHD_SNAP_CHN_NUM	(AHD_PHY_CHN_NUM)   //用于拍照视频编码通道数量
#define	AHD_TAPE_CHN_NUM	(AHD_PHY_CHN_NUM)   //只用于录音通道数量
#define	AHD_STREAM_CHN_NUM  (AHD_STORE_CHN_NUM + AHD_SEND_CHN_NUM)	//用于码流编码通道数量


#define	IPC_PHY_CHN_NUM     (8) //当前SDK支持IPC通道数

#define	IPC_STORE_CHN_NUM   (IPC_PHY_CHN_NUM)   //用于本地存储码流视频编码通道数量
#define	IPC_SEND_CHN_NUM	(IPC_PHY_CHN_NUM)   //用于网传码流视频编码通道数量
#define	IPC_SNAP_CHN_NUM	(IPC_PHY_CHN_NUM)   //用于拍照视频编码通道数量
#define	IPC_TAPE_CHN_NUM	(IPC_PHY_CHN_NUM)   //只用于录音通道数量
#define	IPC_STREAM_CHN_NUM  (IPC_STORE_CHN_NUM + IPC_SEND_CHN_NUM)	//用于码流编码通道数量


/*********************************************************************************
【2017-08-08】慧视通 SDK (AHD+IPC) 编码资源通道定义如下图所示:

--------------------------------- AHD 视频---------------------------------------
| STORE (VeCh[0x00] ~ VeCh[0x07]) | SEND (VeCh[0x10] ~ VeCh[0x17]) |  SNAP (VeCh[0x20] ~ VeCh[0x27]) 

--------------------------------- IPC 视频---------------------------------------
| STORE (VeCh[0x08] ~ VeCh[0x0F]) | SEND (VeCh[0x18] ~ VeCh[0x1F]) |  SNAP (VeCh[0x28] ~ VeCh[0x2F]) 

--------------------------------- AHD 音频---------------------------------------
| STORE (AeCh[0x00] ~ AeCh[0x07]) | SEND (AeCh[0x10] ~ AeCh[0x17]) |  TAPE (AeCh[0x20] ~ AeCh[0x27]) 

--------------------------------- IPC 音频---------------------------------------
| STORE (AeCh[0x08] ~ AeCh[0x0F]) | SEND (AeCh[0x18] ~ AeCh[0x1F]) |  TAPE (AeCh[0x28] ~ AeCh[0x2F]) 

**********************************************************************************/


#define VPSS_MAIN_STREAM_CHN    (0)
#define VPSS_SUB_STREAM_CHN     (1)


//==================== VO DEFINE START ====================
#define  VO_DEV_HD  0	/* high definition device */ /*VGA*/
#define  VO_DEV_AD  1	/* assistant device */ /*CVBS*/
#define  VO_DEV_SD  2	/* spot device */ /*CVBS*/


/* RGB format is 1888. */
#define VO_BKGRD_RED      0xFF0000    /* red back groud color */
#define VO_BKGRD_GREEN    0x00FF00    /* green back groud color */
#define VO_BKGRD_BLUE     0x0000FF    /* blue back groud color */
#define VO_BKGRD_BLACK    0x000000    /* black back groud color */

//==================== VO DEFINE END ====================



//==================== VENC DEFINE START ====================
#define MAX_TIME_CNT    (500)

#define VENC_CREAT_CONTINUE                 (1)
//==================== VENC DEFINE END ====================




//==================== AENC DEFINE START ====================

#define AENC_CREAT_CONTINUE	(1)

#define MAX_TIME_AENC_CNT    (500)


//==================== AENC DEFINE END ====================

#endif

