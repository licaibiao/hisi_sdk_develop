TARGET := test
 
COMPILE_CROSS:= arm-hisiv300-linux-
CC := $(COMPILE_CROSS)gcc
GG := $(COMPILE_CROSS)gcc
 
INC := -I ./common
INC += -I ./include
INC += -I ./include/font
INC += -I ./include/nvp6124
INC += -I ./include/nvp6134_ex
INC += -I ./include/tlv320aic31
INC += -I ./include/tp2823
INC += -I ./region
INC += -I ./vdec_vo
INC += -I ./venc
INC += -I ./vio

 
SRC_C		 := $(wildcard ./*.c)
COM_C		 := $(wildcard ./common/*.c)
REGION_C     := $(wildcard ./region/*.c)
VDEC_VO_C    := $(wildcard ./vdec_vo/*.c)
VENC_C		 := $(wildcard ./venc/*.c)
VIO_C		 := $(wildcard ./vio/*.c)
 
SRC_OBJ	  	 := $(patsubst %.c,%.o,$(SRC_C))
COM_OBJ	     := $(patsubst %.c,%.o,$(COM_C))
REGION_OBJ	 := $(patsubst %.c,%.o,$(REGION_C))
VDEC_VO_OBJ	 := $(patsubst %.c,%.o,$(VDEC_VO_C))
VENC_OBJ	 := $(patsubst %.c,%.o,$(VENC_C))
VIO_OBJ	     := $(patsubst %.c,%.o,$(VIO_C))


CFLAGS += -Wall  -lpthread

MPPLIBDIR    := ./lib/
FONTLIBDIR   := ./lib/font/
HI_MPI_LIBSO := hive_AGC  hive_GAIN hive_RES dnvqe hive_ANR hive_HDR hive_RNR jpeg6b md tde VoiceEngine hive_common hive_HPF hive_AEC hive_EQ hive_MBC ive  mpi upvqe
FONT_LIBSO   := freetype SDL SDL_ttf
LIB_DIR      := -L$(FONTLIBDIR) -L$(MPPLIBDIR)
LIB_SO       := $(addprefix -l,$(HI_MPI_LIBSO))
LIB_SO       += $(addprefix -l,$(FONT_LIBSO))

ALL_OBJ=$(SRC_OBJ) $(COM_OBJ) $(REGION_OBJ) $(VDEC_VO_OBJ) $(VENC_OBJ) $(VIO_OBJ)

HI_MPI_LIBS := ./lib/libhdmi.a

$(TARGET): $(ALL_OBJ)
	$(GG) $(CFLAGS) -o $(TARGET) $(ALL_OBJ) $(LIB_DIR) $(LIB_SO) $(HI_MPI_LIBS)
	
$(ALL_OBJ):%.o:%.c
	echo $(ALL_OBJ)
	$(GG) $(CFLAGS) -c $< -o $@ $(INC)	

 
clean:
	$(RM) $(ALL_OBJ)
	$(RM) $(TARGET)
