#
# Makefile
#
CC ?= gcc
LVGL_DIR_NAME ?= lvgl
LVGL_DIR ?= ${shell pwd}

DEPS = dbus-1
DEPFLAGS_CC = `pkg-config --cflags $(DEPS)`
DEPFLAGS_LD = `pkg-config --libs $(DEPS)`

CFLAGS += -I$(LVGL_DIR)/ -Isrc/
LDFLAGS += -lm 
BIN = main


#Collect the files to compile
MAINSRC = src/main.c #src/gui.c src/cover.c src/lv_fs_pc.c src/lv_fs_if.c 

#include $(LVGL_DIR)/lvgl/lvgl.mk
#include $(LVGL_DIR)/lv_drivers/lv_drivers.mk
#include $(LVGL_DIR)/lv_lib_png/lv_lib_png.mk

OBJEXT ?= .o

AOBJS = $(ASRCS:.S=$(OBJEXT))
COBJS = $(CSRCS:.c=$(OBJEXT))

MAINOBJ = $(MAINSRC:.c=$(OBJEXT))

SRCS = $(ASRCS) $(CSRCS) $(MAINSRC)
OBJS = $(AOBJS) $(COBJS)

## MAINOBJ -> OBJFILES

all: default

%.o: %.c
	@$(CC)  $(CFLAGS) $(DEPFLAGS_CC) -c $< -o $@
	@echo "CC $<"

default: $(AOBJS) $(COBJS) $(MAINOBJ)
	$(CC) -o $(BIN) $(MAINOBJ) $(AOBJS) $(COBJS) $(LDFLAGS) $(DEPFLAGS_LD)

clean: 
	rm -f $(BIN) $(AOBJS) $(COBJS) $(MAINOBJ)
	rm -f *.o
