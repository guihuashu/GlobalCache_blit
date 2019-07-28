
CROSS_COMPILE = arm-linux-
#CROSS_COMPILE = 
AS		= $(CROSS_COMPILE)as
LD		= $(CROSS_COMPILE)ld
CC		= $(CROSS_COMPILE)gcc
CPP		= $(CC) -E
AR		= $(CROSS_COMPILE)ar
NM		= $(CROSS_COMPILE)nm

STRIP		= $(CROSS_COMPILE)strip
OBJCOPY		= $(CROSS_COMPILE)objcopy
OBJDUMP		= $(CROSS_COMPILE)objdump

export AS LD CC CPP AR NM
export STRIP OBJCOPY OBJDUMP

CFLAGS := -Wall -O2 -g
CFLAGS += -I $(shell pwd)/include
CFLAGS += -I /opt/FriendlyARM/toolschain/4.4.3/include
CFLAGS += -I /opt/FriendlyARM/toolschain/4.4.3/arm-none-linux-gnueabi/include
CFLAGS += -I /opt/FriendlyARM/toolschain/4.4.3/arm-none-linux-gnueabi/sys-root/usr/include
	
LDFLAGS := -L /opt/FriendlyARM/toolschain/4.4.3/lib
LDFLAGS += -L /opt/FriendlyARM/toolschain/4.4.3/arm-none-linux-gnueabi/lib

LDFLAGS += -lm -lpthread -lcurl

export CFLAGS LDFLAGS

TOPDIR := $(shell pwd)
export TOPDIR

TARGET := driver


obj-y += main.o
obj-y += itach/

all : 
	make -C ./ -f $(TOPDIR)/Makefile.build
	$(CC) -o $(TARGET) built-in.o  $(LDFLAGS)


clean:
	rm -f $(shell find -name "*.o")
	rm -f $(shell find -name "*.d")
	rm -f $(TARGET)
	
