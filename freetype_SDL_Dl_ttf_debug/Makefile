PROJECT_TOP_DIR=$(shell cd ./; pwd)
PROJECT_BIN_DIR=$(PROJECT_TOP_DIR)/bin
PROJECT_SRC_DIR=$(PROJECT_TOP_DIR)
PROJECT_INC_DIR=$(PROJECT_TOP_DIR)/inc
PROJECT_LIB_DIR=$(PROJECT_TOP_DIR)/lib
PROJECT_OBJ_DIR=$(PROJECT_BIN_DIR)/objs
MKDIR := mkdir -p
 
CC := arm-hisiv300-linux-gcc
TARGETS = test
 
CFLAGS := -g  -lfreetype -lSDL -lSDL_ttf -I$(PROJECT_INC_DIR)
LDFLAG := -L$(PROJECT_LIB_DIR)
 
src :=$(wildcard $(PROJECT_SRC_DIR)/*.c)
dir := $(notdir $(src))
PROJECT_OBJ := $(patsubst %.c,%.o,$(dir) )
	
PROJECT_ALL_OBJS := $(addprefix $(PROJECT_OBJ_DIR)/, $(PROJECT_OBJ))
	
all:$(PROJECT_SRC_DIR) $(PROJECT_ALL_OBJS)
	$(CC) $(CFLAGS) $(PROJECT_ALL_OBJS) -o $(TARGETS) $(LDFLAG)
	
$(PROJECT_OBJ_DIR)/%.o : $(PROJECT_SRC_DIR)/%.c
	$(MKDIR) $(PROJECT_OBJ_DIR)
	$(CC) -c $(CFLAGS) $< -o $@
 
clean:
	rm -fr $(PROJECT_BIN_DIR)
	#rm -fr $(TARGETS)	
