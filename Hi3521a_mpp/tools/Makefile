ifeq ($(PARAM_FILE), )
     PARAM_FILE:=../Makefile.param
     include $(PARAM_FILE)
endif

SRC  := $(wildcard *.c)
OBJ  := $(SRC:%.c=%.o)

INC_FLAGS := -I$(REL_INC)
CFLAGS += -Wall -g -lpthread $(INC_FLAGS) -ldl
LIBS += $(REL_LIB)/libmpi.a

LIBS += $(REL_LIB)/libhdmi.a $(JPEGD_LIBA)


CFLAGS += -DHICHIP=$(HICHIP)

TARGET := $(OBJ:%.o=%)

.PHONY : clean all

all: $(TARGET)

$(TARGET):%:%.o 
	$(CC) $(CFLAGS)  -o $@ $^ $(LIBS) $(AUDIO_LIBA)
	@rm *.o
clean:
	@rm -f $(TARGET)
	@rm -f $(OBJ)
