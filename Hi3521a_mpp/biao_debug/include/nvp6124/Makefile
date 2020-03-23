#ifeq ($(KERNELRELEASE),)
#export MMZ_SRC_BASE=$(PWD)
#endif

ifeq ($(PARAM_FILE), )
	PARAM_FILE:=../../Makefile.param
		include $(PARAM_FILE)
endif
obj-m := nvp6124.o
nvp6124-objs := nvp6124_drv.o coax_protocol.o video_eq.o video.o motion.o audio.o


EXTRA_CFLAGS += -I$(REL_INC)
EXTRA_CFLAGS += $(DRV_CFLAGS)
#EXTRA_CFLAGS += -D$(HI_FPGA)

default:
	@make -C $(LINUX_ROOT) M=$(PWD) modules 
	cp *.ko  $(REL_KO)/extdrv/
clean: 
	@make -C $(LINUX_ROOT) M=$(PWD) clean 


