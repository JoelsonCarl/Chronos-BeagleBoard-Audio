ARM_TOOLCHAIN_PREFIX  ?= arm-angstrom-linux-gnueabi-
ARM_TOOLCHAIN_PATH    ?= /home/hulettbh/Beagleboard/toolchains/usr/local/angstrom/arm

ARM_INCLUDE			  := $(ARM_TOOLCHAIN_PATH)/arm-angstrom-linux-gnueabi/usr/include
ARM_LIB				  := $(ARM_TOOLCHAIN_PATH)/arm-angstrom-linux-gnueabi/usr/lib

ARM_CC := $(ARM_TOOLCHAIN_PATH)/bin/$(ARM_TOOLCHAIN_PREFIX)gcc

ARM_CFLAGS += 	-Wall -pthread \
				-I$(ARM_INCLUDE)/gstreamer-0.10 \
				-I$(ARM_INCLUDE)/glib-2.0 \
				-I$(ARM_LIB)/glib-2.0/include \
				-I$(ARM_INCLUDE)/libxml2 \
				-pthread -lgstreamer-0.10 -lgobject-2.0 -lgmodule-2.0 \
				-lxml2 -lgthread-2.0 -lrt -lglib-2.0 -lpthread

all: pipe

pipe : main.c pipe.c
	$(ARM_CC) $(ARM_CFLAGS) -o $@ $^

install:
	scp pipe root@beagle:workdir
