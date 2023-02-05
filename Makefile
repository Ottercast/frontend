#
# Makefile
#
CC ?= gcc
LVGL_DIR_NAME ?= lvgl
LVGL_DIR ?= ${shell pwd}

DEPS = dbus-1 ImageMagick libcurl MagickWand
DEPFLAGS_CC = `pkg-config --cflags $(DEPS)`
DEPFLAGS_LD = `pkg-config --libs $(DEPS)` -lpthread

CFLAGS += -I$(LVGL_DIR)/ -Isrc/ -O3 -march=armv7-a+simd+neon
LDFLAGS += -lm
BIN = ottercast-frontend

#Collect the files to compile
MAINSRC = src/main.c src/mpris.c src/dbus/dbus_mpris.c src/gui.c src/cover.c

include $(LVGL_DIR)/lvgl/lvgl.mk
include $(LVGL_DIR)/lv_drivers/lv_drivers.mk

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

install: ottercast-frontend
	install -d $(DESTDIR)/opt/ottercast-frontend
	install -m 755 ottercast-frontend $(DESTDIR)/opt/ottercast-frontend/
	install -m 755 assets/cover.png $(DESTDIR)/opt/ottercast-frontend/
