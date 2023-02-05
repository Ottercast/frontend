#include "main.h"

#define DISP_BUF_SIZE (80 * LV_HOR_RES_MAX)

static pthread_t thread_mpris_poll; 
pthread_mutex_t lock;

int main(void)
{
	int err = 0;
	lv_init();
	lv_png_init();
	fbdev_init();

	mpris_init();
	mpris_poll_all();

	/*A small buffer for LittlevGL to draw the screen's content*/
	static lv_color_t buf[DISP_BUF_SIZE];

	/*Initialize a descriptor for the buffer*/
	static lv_disp_draw_buf_t disp_buf;
	lv_disp_draw_buf_init(&disp_buf, buf, NULL, DISP_BUF_SIZE);

	/*Initialize and register a display driver*/
	lv_disp_drv_t disp_drv;
	lv_disp_drv_init(&disp_drv);
	disp_drv.draw_buf   = &disp_buf;
	disp_drv.flush_cb = fbdev_flush;
	disp_drv.hor_res = 340;
	disp_drv.ver_res = 800;
	disp_drv.sw_rotate = 1;
	disp_drv.rotated = LV_DISP_ROT_90;

	lv_disp_drv_register(&disp_drv);

	gui_draw_display();

	pthread_mutex_init(&lock, NULL);

	err = pthread_create(&thread_mpris_poll, NULL, gui_mpris_poll_task, NULL);
	if (err)
	{
		fprintf(stderr, "pthread create failed, err: %d\n", err);
		exit(1);
	}

	/*Handle tasks (tickless mode)*/
	while(1)
	{
		pthread_mutex_lock(&lock);
		lv_task_handler();
		pthread_mutex_unlock(&lock);
		usleep(5000);
	}

	return 0;
}

static uint64_t start_ms = 0;
uint32_t ottercast_frontend_tick_get(void)
{
	struct timespec now;
	uint64_t now_ms;

	clock_gettime(CLOCK_MONOTONIC, &now);
	now_ms = now.tv_sec * 1000 + now.tv_nsec / 1000000;

	if(start_ms == 0) {
		start_ms = now_ms;
	}

	return now_ms - start_ms;
}
