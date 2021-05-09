#pragma once

#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>

#include "lvgl/lvgl.h"
#include "lv_drivers/display/fbdev.h"
#include "lv_lib_png/lv_png.h"
#include "lv_lib_split_jpg/lv_sjpg.h"

#include "lv_fs_if.h"
#include "gui.h"
#include "mpris.h"

uint32_t ottercast_frontend_tick_get(void);
