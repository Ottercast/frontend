#include "main.h"

#define DISP_BUF_SIZE (80 * LV_HOR_RES_MAX)

static pthread_t thread_mpris_poll; 
pthread_mutex_t lock;

int main(void)
{
	int err = 0;
	lv_init();
	lv_fs_if_init();
	lv_png_init();
    lv_split_jpeg_init();

    fbdev_init();

    mpris_init();
    mpris_poll_all();

    /*A small buffer for LittlevGL to draw the screen's content*/
    static lv_color_t buf[DISP_BUF_SIZE];

    /*Initialize a descriptor for the buffer*/
	static lv_disp_buf_t disp_buf;
	lv_disp_buf_init(&disp_buf, buf, NULL, DISP_BUF_SIZE);

	/*Initialize and register a display driver*/
	lv_disp_drv_t disp_drv;
	lv_disp_drv_init(&disp_drv);
	disp_drv.buffer   = &disp_buf;
	disp_drv.flush_cb = fbdev_flush;
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

uint32_t ottercast_frontend_tick_get(void)
{
	static uint64_t start_ms = 0;
	if(start_ms == 0) {
		struct timeval tv_start;
		gettimeofday(&tv_start, NULL);
		start_ms = (tv_start.tv_sec * 1000000 + tv_start.tv_usec) / 1000;
	}

	struct timeval tv_now;
	gettimeofday(&tv_now, NULL);
	uint64_t now_ms;
	now_ms = (tv_now.tv_sec * 1000000 + tv_now.tv_usec) / 1000;

	uint32_t time_ms = now_ms - start_ms;
	return time_ms;
}
