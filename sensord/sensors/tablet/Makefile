#########################################################
##
## Build for accel module
## Create By:	kim Zhu
## Create Data:	2015-03-16
##
######################################################### 

CC = gcc
CFLAGS = -Wall -O2 -fPIC
INCLUDES = -I../../include
LIBS += 
LIBS_DIR = ../../libs

SRC = $(wildcard ./*.c)
OBJ = $(patsubst %.c, %.o, $(SRC))
OBJ_SO = libtablet.so


.PHONY: all debug clean install 

## build all
all: $(OBJ)
$(OBJ): $(SRC)
	@$(CC) $(CFLAGS) -o $@ -c $^ $(INCLUDES)
	@$(CC) -shared -o $(OBJ_SO) $(OBJ)
	@mkdir -p $(LIBS_DIR)
	@cp $(OBJ_SO) $(LIBS_DIR)/

## for debug
debug: all

## for installation
install:
	@echo "not available."

## for clean up
clean:
	@rm -f $(OBJ) $(OBJ_SO)

