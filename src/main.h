#pragma once

#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>

#include "lvgl/lvgl.h"
#include "lv_drivers/display/fbdev.h"

#include "lv_fs_if.h"
#include "gui.h"

uint32_t ottercast_frontend_tick_get(void);