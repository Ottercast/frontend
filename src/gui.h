#pragma once
#include "lvgl/lvgl.h"
#include "mpris.h"

void gui_draw_display();
void gui_mpris_poll_task(lv_task_t *task);
void gui_format_seconds_string(int input, char *buffer, size_t buffer_length);
void gui_fetch_coverart_from_url(const char *url);