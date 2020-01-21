#ifndef HST_GPIO_AL
#define HST_GPIO_AL


#include "hstGpioStruch.h"



int HstGpio_Set_Direction(GPIO_GROUP_E enGroup, GPIO_BIT_E enBit, GPIO_DIR_E enVal);

int HstGpio_Set_Value(GPIO_GROUP_E enGroup, GPIO_BIT_E enBit, GPIO_DATA_E enVal);

int HstGpio_Get_Value(GPIO_GROUP_E enGroup, GPIO_BIT_E enBit, char *pcVal);


#endif
