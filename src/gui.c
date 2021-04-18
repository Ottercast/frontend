#include "gui.h"

struct
{
    lv_obj_t *label_title;
    lv_obj_t *label_artist;
    lv_obj_t *bar_trackprogress;
    lv_obj_t *label_currentposition;
    lv_obj_t *label_totalposition;

    lv_obj_t *image_coverart;
    char coverart_url[1024];
} gui_state;

char display_time_buffer[64];

void gui_draw_display()
{
    lv_obj_t *screen = lv_scr_act();

    gui_state.label_title = lv_label_create(screen, NULL);
    lv_label_set_long_mode(gui_state.label_title, LV_LABEL_LONG_SROLL_CIRC);
    lv_obj_set_pos(gui_state.label_title, 350, 40);
    lv_obj_set_size(gui_state.label_title, 430, 40);
    lv_label_set_text(gui_state.label_title, "");

    gui_state.label_artist = lv_label_create(screen, NULL);
    lv_label_set_long_mode(gui_state.label_artist, LV_LABEL_LONG_SROLL_CIRC);
    lv_obj_set_pos(gui_state.label_artist, 350, 80);
    lv_obj_set_size(gui_state.label_artist, 430, 30);
    lv_label_set_text(gui_state.label_artist, "");

    // Song progress indicator (2 labels, 1 bar)
    gui_state.label_currentposition = lv_label_create(screen, NULL);
    lv_obj_set_pos(gui_state.label_currentposition, 350, 140);
    lv_obj_set_size(gui_state.label_currentposition, 75, 30);
    lv_label_set_text(gui_state.label_currentposition, "-:--");

    gui_state.label_totalposition = lv_label_create(screen, NULL);
    lv_obj_set_pos(gui_state.label_totalposition, 720, 140);
    lv_obj_set_size(gui_state.label_totalposition, 75, 30);
    lv_label_set_text(gui_state.label_totalposition, "-:--");

    gui_state.bar_trackprogress = lv_bar_create(screen, NULL);
    lv_obj_set_pos(gui_state.bar_trackprogress, 350, 115);
    lv_obj_set_size(gui_state.bar_trackprogress, 430, 20);
    lv_bar_set_range(gui_state.bar_trackprogress, 0, 258);
    lv_bar_set_value(gui_state.bar_trackprogress, 198, LV_ANIM_OFF);

    lv_obj_t *coverborder = lv_img_create(screen, NULL);
    lv_obj_set_pos(coverborder, 0, 0);
    lv_obj_set_size(coverborder, 340, 340);
    lv_img_set_src(coverborder, "./coverborder.png");

    gui_state.image_coverart = lv_img_create(screen, NULL);
    lv_obj_set_pos(gui_state.image_coverart, 10, 10);
    lv_obj_set_size(gui_state.image_coverart, 320, 320);
    lv_img_set_antialias(gui_state.image_coverart, true);
    lv_img_set_auto_size(gui_state.image_coverart, false);
    lv_img_set_src(gui_state.image_coverart, "./cover.png");
}

void gui_mpris_poll_task(lv_task_t *task)
{
    mpris_poll_all();

    mpris_player *mplay = mpris_get_player_by_namespace("org.mpris.MediaPlayer2.spotifyd");

    if (mplay == NULL)
    {
        printf("player not found!\n");
        mpris_close();
        mpris_init();
    }
    else
    {
        lv_label_set_text(gui_state.label_title, mplay->properties.metadata.title);
        lv_label_set_text(gui_state.label_artist, mplay->properties.metadata.album_artist);

        gui_format_seconds_string(mplay->properties.position, display_time_buffer, sizeof(display_time_buffer));
        lv_label_set_text(gui_state.label_currentposition, display_time_buffer);

        gui_format_seconds_string(mplay->properties.metadata.length, display_time_buffer, sizeof(display_time_buffer));
        lv_label_set_text(gui_state.label_totalposition, display_time_buffer);

        lv_bar_set_range(gui_state.bar_trackprogress, 0, mplay->properties.metadata.length / 1000000);
        lv_bar_set_value(gui_state.bar_trackprogress, mplay->properties.position / 1000000, LV_ANIM_ON);

        if (!!strncmp(gui_state.coverart_url, mplay->properties.metadata.art_url, strlen(mplay->properties.metadata.art_url)))
        {
            strncpy(gui_state.coverart_url, mplay->properties.metadata.art_url, 1024);
            gui_fetch_coverart_from_url(gui_state.coverart_url);
        }
    }
}

void gui_format_seconds_string(int input, char *buffer, size_t buffer_length)
{
    input /= 1000000;
    snprintf(buffer, buffer_length, "%2d:%02d", input / 60, input % 60);
}

void gui_fetch_coverart_from_url(const char *url)
{
    printf("Fetching new coverart: %s\n", url);
    lv_img_set_src(gui_state.image_coverart, "C/test.bin");
}