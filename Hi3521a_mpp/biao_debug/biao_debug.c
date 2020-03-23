/************************************************************
*Copyright (C),lcb0281at163.com lcb0281atgmail.com
*BlogAddr: caibiao-lee.blog.csdn.net
*FileName: biao_debug.c
*Description:所有测试函数的入口
*Date:     2020-02-03
*Author:   Caibiao Lee
*Version:  V1.0
*Others:
*History:
***********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "biao_vio.h"
#include "biao_vdec_vo.h"
#include "biao_region.h"

int BIAO_Debug_Interface(void)
{

    BIAO_VIO_DEBUG();
    BIAO_REGION_DEBUG();
    
    return 0;
}

int main()
{
    printf("%s %d start \n",__FUNCTION__,__LINE__);

    BIAO_Debug_Interface();

    return 0;
}




