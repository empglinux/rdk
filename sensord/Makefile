#########################################################
##
## Build for sensord main program and all sub-modules
## Create By:	kim Zhu
## Create Data:	2015-03-16
##
######################################################### 


CC = gcc

SRC_FOLDERS = sensors

TARGET = sensord
OBJ_DIR = obj
CONFFILE = conf/sensord_start.sh
LIBS_DIR = libs


SRCS = $(wildcard *.c)
OBJS = $(patsubst %.c, %.o, $(SRCS))

CFLAGS = -Wall -O2
INCLUDES = -I./include -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/lib/x86_64-linux-gnu/dbus-1.0/include
LIBS += -lpthread -laccel -ltablet -lglib-2.0
LDFLAGS += -L./libs
LDFLAGS += $(LIBS)


.PHONY: all debug clean install deb


## build all
all:
	@for subdir in ${SRC_FOLDERS}; do \
		if test -d "$$subdir"; then \
			echo -e "\033[30;32m	Building $$subdir ... \033[0m"; \
			make -C $$subdir all; \
		else \
			echo -e "\033[30;31m	No $$subdir folder ... \033[0m"; \
		fi \
	done
	@$(CC) -o $(OBJS) -c $(SRCS) $(INCLUDES) $(CFLAGS)
	@$(CC) -o $(TARGET) $(OBJS) $(LDFLAGS)

## for debug
debug: CFLAGS += -DDEBUG_MACRO -g
debug:
	@for subdir in ${SRC_FOLDERS}; do \
		if test -d "$$subdir"; then \
			echo -e "\033[30;32m	Building $$subdir ... \033[0m"; \
			make -C $$subdir debug; \
		else \
			echo -e "\033[30;31m	No $$subdir folder ... \033[0m"; \
		fi \
	done
	@$(CC) -o $(OBJS) -c $(SRCS) $(INCLUDES) $(CFLAGS)
	@$(CC) -o $(TARGET) $(OBJS) $(LDFLAGS)


## for installation	(TODO)
install:
	@for subdir in ${SRC_FOLDERS}; do \
		if test -d "$$subdir"; then \
			echo -e "\033[30;32m	Building $$subdir ... \033[0m"; \
			make -C $$subdir install; \
		else \
			echo -e "\033[30;31m	No $$subdir folder ... \033[0m"; \
		fi \
	done
	@cp $(TARGET) /usr/bin/
	@cp $(LIBS_DIR)/*.so /usr/lib/
	@ldconfig
	@cp -af $(CONFFILE) /etc/profile.d/

## for clean up
clean:
	@for subdir in ${SRC_FOLDERS}; do \
		if test -d "$$subdir"; then \
			echo -e "\033[30;32m	Building $$subdir ... \033[0m"; \
			make -C $$subdir clean; \
		else \
			echo -e "\033[30;31m	No $$subdir folder ... \033[0m"; \
		fi \
	done
	@rm -rf $(TARGET) $(OBJS) $(LIBS_DIR)

## for generate debian package
deb:
	@echo "hahahahhahahah......"

