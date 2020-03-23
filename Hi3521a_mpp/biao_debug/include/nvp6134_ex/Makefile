#ifeq ($(KERNELRELEASE),)
#export MMZ_SRC_BASE=$(PWD)
#endif

ifeq ($(PARAM_FILE), )
	PARAM_FILE:=../Makefile.param
		include $(PARAM_FILE)
endif
obj-m := nvp6134_ex.o
nvp6134_ex-objs := nvp6134_drv.o coax.o eq.o eq_recovery.o eq_common.o acp.o acp_firmup.o video.o motion.o audio.o

EXTRA_CFLAGS += -Wno-date-time  


EXTRA_CFLAGS += -DI2C_INTERNAL
EXTRA_CFLAGS += -I$(REL_INC)
EXTRA_CFLAGS += $(DRV_CFLAGS)



# add Show/hide NVP6134(C) function (2016-09-10, sat)
EXTRA_CFLAGS += -D__DEC_HIDE_SHOW_FUNCTION

default:
	@make -C $(LINUX_ROOT) M=$(PWD) modules 
	cp *.ko $(REL_KO)/extdrv/
clean: 
	@make -C $(LINUX_ROOT) M=$(PWD) clean 


