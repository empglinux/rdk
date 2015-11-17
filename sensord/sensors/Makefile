#########################################################
##
## Build for sensor modules
## Create By:	kim Zhu
## Create Data:	2015-03-16
##
######################################################### 


SRC_FOLDERS = accel acld

.PHONY: all debug clean install


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

## for debug
debug:
	@for subdir in ${SRC_FOLDERS}; do \
		if test -d "$$subdir"; then \
			echo -e "\033[30;32m	Building $$subdir ... \033[0m"; \
			make -C $$subdir debug; \
		else \
			echo -e "\033[30;31m	No $$subdir folder ... \033[0m"; \
		fi \
	done

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
