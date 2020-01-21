#include "HdmiDisplay.h"
#include "hstChnConfig.h"
#include "hstSdkAL.h"
#include "hstSdkDefine.h"
#include "hstSdkStruct.h"

#include "sample_vio.h"
#include "hstGpioAL.h"

#include "hiir.h"
#include "hiir_codedef.h"

#define REMOTE_KEY0       0xff00ff00
#define REMOTE_KEY1       0xfa05ff00
#define REMOTE_KEY2       0xf30cff00
#define REMOTE_KEY3       0xf10eff00
#define REMOTE_KEY4       0xfd02ff00
#define REMOTE_KEY5       0xfe01ff00
#define REMOTE_KEY6       0xf609ff00
#define REMOTE_KEY7       0xf20dff00
#define REMOTE_KEY8       0xfb04ff00
#define REMOTE_KEY9       0xf906ff00
#define REMOTE_KEY10      0xf50aff00
#define REMOTE_KEY11      0xea15ff00
#define REMOTE_KEY12      0xe916ff00

#define CHANNEL_NUM 	4
#define	BIT_SET(a,b)	((a) |= (1<<(b)))
#define	BIT_CLEAN(a,b)	((a)=((a) & ((a)^(1<<(b)))))
#define	BIT_CHECK(a,b)	((a) & (1<<(b)))


typedef unsigned short int U8;

typedef struct
{
    char *name;
    unsigned int irkey_value;
}IRKEY_ADAPT;


static IRKEY_ADAPT g_irkey_adapt_array[] =
{
    /*irkey_name*/ /*irkey_value*/
    {"REMOTE_KEY0   ", REMOTE_KEY0,/*1*/         },
    {"REMOTE_KEY1   ", REMOTE_KEY1,/*2*/         },
    {"REMOTE_KEY2   ", REMOTE_KEY2,/*3*/         },
    {"REMOTE_KEY3   ", REMOTE_KEY3,/*4*/         },
    {"REMOTE_KEY4   ", REMOTE_KEY4,/*5*/         },
    {"REMOTE_KEY5   ", REMOTE_KEY5,/*6*/         },
    {"REMOTE_KEY6   ", REMOTE_KEY6,/*7*/         },
    {"REMOTE_KEY7   ", REMOTE_KEY7,/*8*/         },
    {"REMOTE_KEY8   ", REMOTE_KEY8,/*9*/         },
    {"REMOTE_KEY9   ", REMOTE_KEY9,/*-/--*/      },
	{"REMOTE_KEY10  ", REMOTE_KEY10,/*8*/        },
	{"REMOTE_KEY11  ", REMOTE_KEY11,/*9*/        },
	{"REMOTE_KEY12  ", REMOTE_KEY12,/*-/--*/     },
};


static int g_irkey_adapt_count = sizeof(g_irkey_adapt_array) / sizeof(IRKEY_ADAPT);
static int key0_down = 0;
static int key1_down = 0;
static int key2_down = 0;
static int key3_down = 0;
static int key4_down = 0;
static int key5_down = 0;
static int key6_down = 0;
static int key7_down = 0;
static int key8_down = 0;
static int key9_down = 0;


static void huawei_report_irkey(irkey_info_s rcv_irkey_info)
{
    int i = 0;
    for(i = 0; i<g_irkey_adapt_count; i++)
    {
        if( (rcv_irkey_info.irkey_datah == 0) &&
            (rcv_irkey_info.irkey_datal == g_irkey_adapt_array[i].irkey_value) )
        {
        	printf("keyvalue=H/L 0x%x/0x%x\n",rcv_irkey_info.irkey_datah,rcv_irkey_info.irkey_datal);
            break;
        }
    }
    if(i>=g_irkey_adapt_count)
    {
        printf("Error. get a invalid code. irkey_datah=0x%.8x,irkey_datal=0x%.8x.\n", 
               (int)rcv_irkey_info.irkey_datah, (int)rcv_irkey_info.irkey_datal);
    }
    else
    {
        printf("RECEIVE ---> %s\t", g_irkey_adapt_array[i].name);
        if(rcv_irkey_info.irkey_state_code == 1)
        {
            printf("KEYUP...");
        }
        printf("\n");
        
        if((rcv_irkey_info.irkey_datah == 0) && 
           (rcv_irkey_info.irkey_state_code == 0) &&
           (rcv_irkey_info.irkey_datal == REMOTE_KEY0))
        {
			g_u8LastShowMode = 0;
			FourWindowsDispalyCha_1_2_3_4();
        }
        else if((rcv_irkey_info.irkey_datah == 0) && 
                (rcv_irkey_info.irkey_state_code == 0) &&
                (rcv_irkey_info.irkey_datal == REMOTE_KEY1)) 
        {
   			g_u8LastShowMode = 1;
			OneWindowsDisplayChannel(0);
        }
        else if((rcv_irkey_info.irkey_datah == 0) && 
                (rcv_irkey_info.irkey_state_code == 0) &&
                (rcv_irkey_info.irkey_datal == REMOTE_KEY2))
        {
			g_u8LastShowMode = 1;
			OneWindowsDisplayChannel(0);
        }
        else if((rcv_irkey_info.irkey_datah == 0) && 
                (rcv_irkey_info.irkey_state_code == 0) &&
                (rcv_irkey_info.irkey_datal == REMOTE_KEY3)) 
        {
			g_u8LastShowMode = 3;
			OneWindowsDisplayChannel(2);
        }
        else if((rcv_irkey_info.irkey_datah == 0) && 
                (rcv_irkey_info.irkey_state_code == 0) &&
                (rcv_irkey_info.irkey_datal == REMOTE_KEY4)) 
        {
            g_u8LastShowMode = 4;
			OneWindowsDisplayChannel(3);
        }
        else if((rcv_irkey_info.irkey_datah == 0) && 
                (rcv_irkey_info.irkey_state_code == 0) &&
                (rcv_irkey_info.irkey_datal == REMOTE_KEY5)) 
        {
            g_u8LastShowMode = 5;
			TwoWindowsDispalyCha_1_2();
        }
				
		else if((rcv_irkey_info.irkey_datah == 0) && 
			(rcv_irkey_info.irkey_state_code == 0) &&
			(rcv_irkey_info.irkey_datal == REMOTE_KEY6)) 
		{
			g_u8LastShowMode = 6;
			TwoWindowsDispalyCha_3_4();
		}

		else if((rcv_irkey_info.irkey_datah == 0) && 
			(rcv_irkey_info.irkey_state_code == 0) &&
			(rcv_irkey_info.irkey_datal == REMOTE_KEY7)) 
		{
			g_u8LastShowMode = 7;
			ThreeWindowsDispalyCha_1_2_3_mode01();
		}


        else if((rcv_irkey_info.irkey_datah == 0) && 
	        (rcv_irkey_info.irkey_state_code == 0) &&
	        (rcv_irkey_info.irkey_datal == REMOTE_KEY8)) 
        {
			g_u8LastShowMode = 8;
			ThreeWindowsDispalyCha_1_2_3_mode02();
        }


		else if((rcv_irkey_info.irkey_datah == 0) && 
			(rcv_irkey_info.irkey_state_code == 0) &&
			(rcv_irkey_info.irkey_datal == REMOTE_KEY9)) 
        {
			g_u8LastShowMode = 9;
			ThreeWindowsDispalyCha_1_2_3_mode03();
        }		
    }
}

int ChangeWindowStyle(void)
{
	HI_CHAR ch;

	while(1)
	{

		if(0==u32KeyValueStatus)
		{
			printf("please choose show mode, press 'q' to exit this sample.\n"); 
			//printf("please choose show mode,enter 0~9\n"); 
			printf("\t1) 1 windows channel 1\n");
			printf("\t2) 1 windows channel 2\n");
			printf("\t3) 1 windows channel 3\n");
			printf("\t4) 1 windows channel 4\n");
			printf("\t5) 2 windows channel 1,2\n");
			printf("\t6) 2 windows channel 3,4\n");
			printf("\t7) 3 windows channel 1~3\n");
			printf("\t8) 3 windows channel 1~3\n");
			printf("\t9) 3 windows channel 1~3\n");
			printf("\t0  4 windows channel 1~4\n");
			printf("\tq) quit\n");
		}else
		{
			for(int i=0;i<CHANNEL_NUM;i++)
			{
				if(BIT_CHECK(u32KeyValueStatus,i))
				{
					printf("Channel %d GPIO is Set \n",i+1);
				}
			}
		}

		
		ch = getchar();
		if(10 == ch)
		{
			continue;
		}
		getchar();

		/**io_key**/
		if(0!=u32KeyValueStatus)
		{
			printf("IO status is 0x%x \n",u32KeyValueStatus);
			continue;
		}
		
		if ('1' == ch)
		{
			g_u8LastShowMode = 1;
			OneWindowsDisplayChannel(0);

		}
		else if ('2' == ch)
		{
			g_u8LastShowMode = 2;
			OneWindowsDisplayChannel(1);


		}
		else if ('3' == ch)
		{
			g_u8LastShowMode = 3;
			OneWindowsDisplayChannel(2);

		}
		else if ('4' == ch)
		{
			g_u8LastShowMode = 4;
			OneWindowsDisplayChannel(3);

		}
		else if ('5' == ch)
		{
			g_u8LastShowMode = 5;
			TwoWindowsDispalyCha_1_2();
		}
		else if ('6' == ch)
		{
			g_u8LastShowMode = 6;
			TwoWindowsDispalyCha_3_4();
		}
		else if ('7' == ch)
		{
			g_u8LastShowMode = 7;
			ThreeWindowsDispalyCha_1_2_3_mode01();
		}
		else if ('8' == ch)
		{
			g_u8LastShowMode = 8;
			ThreeWindowsDispalyCha_1_2_3_mode02();
		}

		else if ('9' == ch)
		{
			g_u8LastShowMode = 9;
			ThreeWindowsDispalyCha_1_2_3_mode03();
		}
		else if ('0' == ch)
		{
			g_u8LastShowMode = 0;
			FourWindowsDispalyCha_1_2_3_4();
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
		
	}
	read_thread_status = 0;	
	show_thread_status = 0;
	IRkey_thread_status = 0;
	sleep(1);
	

}


int HDMIShowMode(unsigned char u8Mode)
{
	printf("resume show mode %d \n",u8Mode);
	switch(u8Mode)
	{
		case 0:
		{
			FourWindowsDispalyCha_1_2_3_4();
			break;
		}
		case 1:
		case 2:
		case 3:
		case 4:
		{
			OneWindowsDisplayChannel(u8Mode - 1);
			break;
		}


		case 5:
		{
			TwoWindowsDispalyCha_1_2();
			break;
		}

		case 6:
		{
			TwoWindowsDispalyCha_3_4();
			break;
		}

		case 7:
		{
			ThreeWindowsDispalyCha_1_2_3_mode01();
			break;
		}

		case 8:
		{
			ThreeWindowsDispalyCha_1_2_3_mode02();
			break;
		}

		case 9:
		{
			ThreeWindowsDispalyCha_1_2_3_mode03();
			break;
		}

		default:
		{
			FourWindowsDispalyCha_1_2_3_4();
			break;
		}



	}

}



void* read_func(void* arg)
{
	char i =0;
	char l_s8bit_val = 0; 
	GPIO_BIT_E	  l_eBit;
	GPIO_GROUP_E  l_eGpioGroup;

	l_eGpioGroup = (GPIO_GROUP_E)12;
	u32KeyValueStatus = 0;
	read_thread_status = 1;

	/**初始化IO方向**/
	for(i=0;i<4;i++)
	{
		l_eBit = (GPIO_BIT_E)(i+1);
		HstGpio_Set_Direction(l_eGpioGroup, l_eBit, GPIO_INPUT);
	}
	
	while(read_thread_status)
	{
		for(i=0;i<4;i++)
		{
			l_eBit = (GPIO_BIT_E)(i+1);
																																																	 																   
			HstGpio_Get_Value(l_eGpioGroup, l_eBit, &l_s8bit_val);	
			if(0==l_s8bit_val)
			{
				BIT_SET(u32KeyValueStatus,i);
			}else
			{
				BIT_CLEAN(u32KeyValueStatus,i);
			}
		}
		usleep(200000);
	}
	sleep(1);
	printf("%s %d exit\n",__FUNCTION__,__LINE__);
}

void* show_func(void* arg)
{
	static unsigned int u32LastKeyValueStatus = 0;
	int i = 0;	
	show_thread_status = 1;
	while(show_thread_status)
	{
		if((u32KeyValueStatus!=0)&&(u32LastKeyValueStatus!=u32KeyValueStatus))
		{
			u32LastKeyValueStatus = u32KeyValueStatus;
			for(i=0;i<CHANNEL_NUM;i++)
			{
				if(BIT_CHECK(u32KeyValueStatus,i))
				{
					OneWindowsDisplayChannel(i);
					break;
				}
			}
		}
		
		/**按键恢复**/
		if((u32LastKeyValueStatus!=u32KeyValueStatus)&&(0==u32KeyValueStatus))
		{
			u32LastKeyValueStatus = u32KeyValueStatus;
			//FourWindowsDispalyCha_1_2_3_4();
			HDMIShowMode(g_u8LastShowMode);
		}
		usleep(200000);
	}
	sleep(1);
	printf("%s %d exit\n",__FUNCTION__,__LINE__);
}

void* IRKey_func(void* arg)
{
	int fp, res, i, count;
	int delay = 0;
	irkey_info_s rcv_irkey_info[4];

    if( -1 == (fp = open("/dev/"HIIR_DEVICE_NAME, O_RDWR) ) )
    {
        printf("ERROR:can not open %s device. read return %d\n", HIIR_DEVICE_NAME, fp);
       goto EXIT;
    }

    ioctl(fp, IR_IOC_SET_ENABLE_KEYUP, 1);

	IRkey_thread_status = 1;
    while(IRkey_thread_status)
    {
        res = read(fp, rcv_irkey_info, sizeof(rcv_irkey_info));
        count = res / sizeof(irkey_info_s);
        if( (res > 0) && (res<=sizeof(rcv_irkey_info)) )
        {
            for(i=0;i<count;i++)
            {
                huawei_report_irkey(rcv_irkey_info[i]);
            }
        }
        else
        {
            printf("Hi_IR_FUNC_TEST_001 Error. read irkey device error. result=%d.\n", res);
        }
		usleep(200);
    }

EXIT:
	if(fp > 0)
	{
		 close(fp);
	}

    printf("%s %d exit\n",__FUNCTION__,__LINE__);

}





int main(void)
{
	int i =0;
	int para = 0;
	Hst3520D_Mpp_init();

	g_u8LastShowMode = 0;
	/**key detect **/
	if(pthread_create(&read_id,NULL,read_func,&para))
	{
		printf("pthread_create read_func err\n");
	}

	/**show task **/
	if(pthread_create(&show_id,NULL,show_func,&para))
	{
		printf("pthread_create show_func err\n");
	}

	/**IR task **/
	if(pthread_create(&irkey_id,NULL,IRKey_func,&para))
	{
		printf("pthread_create show_func err\n");
	}

	sleep(3);
	ChangeWindowStyle();
	//while(1)
	//{
	//	sleep(1);
	//}

	pthread_join(read_id,NULL); 
	pthread_join(show_id,NULL); 
	pthread_join(irkey_id,NULL); 
	
	return 0;
}

