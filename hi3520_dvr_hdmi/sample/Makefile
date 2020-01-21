
CROSS=arm-hisiv300-linux-

CC  = $(CROSS)g++
AR  = $(CROSS)ar

GPIO_DIR := HstGpioSdkAPI
TLV320AIC31      := extdrv/tlv320aic31
HI_INCLUDE_DIR :=  include
EXTDRV_INCLUDE := extdrv/nvp6134_ex
COMMON_SAMPLE_DIR := common
HST_SAMPLE_DIR := hst3520dSample


INCLUDE := -I../
INCLUDE += -I../$(COMMON_SAMPLE_DIR)/
INCLUDE += -I../$(HI_INCLUDE_DIR)/
INCLUDE += -I../$(EXTDRV_INCLUDE)/
INCLUDE += -I../$(TLV320AIC31)/
INCLUDE += -I../$(HST_SAMPLE_DIR)/
INCLUDE += -I./$(GPIO_DIR)/

HI_LIB_DIR := -L../libso/
HI_LIB_DIR  += -L./HstGpioSdkAPI/

LINK_LIB    := -lBiao_hi3520_sdk -lHstGpio

MPI_LIB_PATH := ../lib/
HI_MPI_LIBSO := dnvqe hive_AEC hive_AGC hive_ANR hive_common hive_EQ hive_GAIN hive_HDR hive_HPF hive_MBC hive_RES hive_RNR ive jpeg jpeg6b 
HI_MPI_LIBSO   += md mpi tde upvqe VoiceEngine hdmi

TARGET      :=HdmiDisplay.cpp
OUT			:=test

test:
	$(CC) $(TARGET) -lpthread  $(INCLUDE) $(HI_LIB_DIR) $(LINK_LIB)  -L$(MPI_LIB_PATH) $(addprefix -l,$(HI_MPI_LIBSO)) -o $(OUT)

clean:
	@rm $(OUT)

