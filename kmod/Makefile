KERNDIR := /lib/modules/$(shell uname -r)/build
BUILD_DIR := $(shell pwd)
VERBOSE = 0

obj-m := psock.o
psock-objs := af_psock.o

ccflags-y := -I$(src)/../include

all:
	echo $(ccflags-y)
	make -C $(KERNDIR) M=$(BUILD_DIR) V=$(VERBOSE) modules

clean:
	make -C $(KERNDIR) M=$(BUILD_DIR) clean

insmod: all
	sudo insmod psock.ko
