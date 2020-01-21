CROSS=arm-hisiv300-linux-

CC  = $(CROSS)gcc
AR  = $(CROSS)ar
ARFLAGS = rcv
OUTPUT_LIBA := libBiao_hi3520_sdk.a
OUTPUT_LIBSO := libBiao_hi3520_sdk.so


HI_MPI_LIBS := ./hi3520dliba/libmpi.a \
			./hi3520dliba/libjpeg.a \
			./hi3520dliba/libVoiceEngine.a \
			./hi3520dliba/libhdmi.a \
			./hi3520dliba/libupvqe.a \
			./hi3520dliba/libdnvqe.a \
			
CFLAGS  += -O2 -lpthread -lm -ldl -DDBUG -DLOCALE_NOT_USED -DWITH_NO_C_LOCALE -DWITH_NONAMESPACES -DWITH_DOM -DWITH_OPENSSL -DDSO_DLFCN 
CFLAGS  += $(INCLUDE)


COMMON_SAMPLE_DIR := ./common
PROJECT_SRC+=$(COMMON_SAMPLE_DIR)/sample_comm_sys.c
PROJECT_SRC+=$(COMMON_SAMPLE_DIR)/sample_comm_vi.c
PROJECT_SRC+=$(COMMON_SAMPLE_DIR)/sample_comm_vpss.c
PROJECT_SRC+=$(COMMON_SAMPLE_DIR)/sample_comm_vo.c
PROJECT_SRC+=$(COMMON_SAMPLE_DIR)/sample_comm_venc.c
PROJECT_SRC+=$(COMMON_SAMPLE_DIR)/sample_comm_vdec.c
#PROJECT_SRC+=$(COMMON_SAMPLE_DIR)/sample_comm_audio.c

HST_SAMPLE_DIR := ./hst3520dSample
PROJECT_SRC+=$(HST_SAMPLE_DIR)/Hst3520dChnAdapt.c
PROJECT_SRC+=$(HST_SAMPLE_DIR)/Hst3520dVi.c
PROJECT_SRC+=$(HST_SAMPLE_DIR)/Hst3520dVo.c
PROJECT_SRC+=$(HST_SAMPLE_DIR)/sample_vio.c
#PROJECT_SRC+=$(HST_SAMPLE_DIR)/Hst3520dVenc.c
#PROJECT_SRC+=$(HST_SAMPLE_DIR)/Hst3520dVdec.c
#PROJECT_SRC+=$(HST_SAMPLE_DIR)/Hst3520dAiAo.c
#PROJECT_SRC+=$(HST_SAMPLE_DIR)/Hst3520dAiAenc.c
#PROJECT_SRC+=$(HST_SAMPLE_DIR)/Hst3520dAdecAo.c
#PROJECT_SRC+=$(HST_SAMPLE_DIR)/Hst3520dSnap.c
#PROJECT_SRC+=./hstAHDSdkAL.c

HI_INCLUDE_DIR := ./include
EXTDRV_INCLUDE := ./extdrv/nvp6134_ex/
TLV320AIC31      := ./extdrv/tlv320aic31/

INCLUDE += -I./
INCLUDE += -I./$(COMMON_SAMPLE_DIR)/
INCLUDE += -I./$(HI_INCLUDE_DIR)/
INCLUDE += -I./$(EXTDRV_INCLUDE)/
INCLUDE += -I./$(TLV320AIC31)/
INCLUDE += -I./$(HST_SAMPLE_DIR)/

LDFLAGS = -shared

OBJS = $(addprefix ./obj/, $(addsuffix .o,$(basename $(notdir $(PROJECT_SRC)))))

all:
	@mkdir -p obj
	@for file in $(PROJECT_SRC); do \
        OBJ=`basename $$file|sed -e 's/\.cpp/\.o/' -e 's/\.c/\.o/'`; \
        echo -e "$(CC) -c -o $$OBJ\n$$file"; \
		$(CC) $(CFLAGS) -c -o obj/$$OBJ $$file; \
    done; \
	echo $(OBJS)
	$(CC) $(LDFLAGS) -o $(OUTPUT_LIBSO) $(OBJS)
	$(AR) $(ARFLAGS) $(OUTPUT_LIBA) $(OBJS)
#	$(CC) $(CFLAGS) -o $(OUTPUT_EXECUT) $(OBJS) $(HI_MPI_LIBS)
	cp -f ./*.a ./liba/
	cp -f ./*.so ./libso/
#	cp -f ./libso/*.so /home/nfs/new_framework_test/
#	cp -f ./libso/*.so /mnt/hgfs/workdir/hddisk_bt20_lianzh_20180427/HstLibSDK/libAHDSdk/libso/
#	cp -f ./libso/$(OUTPUT_LIBSO) /mnt/hgfs/workdir/hddisk_bt20_lianzh_20180427/HstLibSDK/
	rm -rf ./*.o ./*.a ./*.so


.PHONY: clean, copy
clean:
	@echo "DEL OBJ files"
	@rm -fR ./obj/*.o *.a
	rm -rf ./*.o ./*.a ./*.so
	@echo "--------------------------------------------"    
