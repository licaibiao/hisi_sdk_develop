/************************************************************
*Copyright (C), 2017-2027,lcb0281at163.com lcb0281atgmail.com
*FileName: osd_string.h
*Description:
*Date:     2020-03-08
*Author:   Caibiao Lee
*Version:  V1.0

*Others:
*History:
***********************************************************/
#ifndef _OSD_STRING_H_
#define _OSD_STRING_H_

#include <stdio.h>
#include <stdlib.h>

#define TIMER_BMP_PATH  "/App/Time_OSD_BMP.bmp"

//int TimeOSDCanvasUpdate(unsigned char u8HandleID);
int CreateTimeBmpPicture(void);
int CreateChinesePicture(void);


#endif

