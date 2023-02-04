#include "gui.h"

extern pthread_mutex_t lock;

struct
{
    lv_obj_t *label_title;
    lv_obj_t *label_title_shadow;

    lv_obj_t *label_artist;
    lv_obj_t *bar_trackprogress;
    lv_obj_t *label_currentposition;
    lv_obj_t *label_totalposition;

    lv_obj_t *image_coverart;
    lv_obj_t *image_background;
    lv_style_t style_title;
    lv_style_t style_title_shadow;
    lv_style_t style_artist;
    lv_style_t style_info;

    char coverart_url[1024];
} gui_state;

char display_time_buffer[64];

uint8_t cover_buffer_1[COVER_IMAGE_SIZE];

const lv_img_dsc_t cover_1 = {
  .header.always_zero = 0,
  .header.w = COVER_IMAGE_X,
  .header.h = COVER_IMAGE_Y,
  .data_size = COVER_IMAGE_SIZE,
  .header.cf = LV_IMG_CF_TRUE_COLOR,
  .data = cover_buffer_1,
};

uint8_t cover_background_buffer[COVER_BACKGROUND_SIZE];

const lv_img_dsc_t cover_background = {
    .header.always_zero = 0,
    .header.w = COVER_BACKGROUND_X,
    .header.h = COVER_BACKGROUND_Y,
    .data_size = COVER_BACKGROUND_SIZE,
    .header.cf = LV_IMG_CF_TRUE_COLOR,
    .data = cover_background_buffer,
};

void gui_draw_display()
{
    lv_obj_t *screen = lv_scr_act();

    lv_style_init(&gui_state.style_title);
    lv_style_set_text_font(&gui_state.style_title, &lv_font_montserrat_40);
    lv_style_set_text_color(&gui_state.style_title, lv_color_white());

    lv_style_init(&gui_state.style_title_shadow);
    lv_style_set_text_font(&gui_state.style_title_shadow, &lv_font_montserrat_40);
    lv_style_set_text_color(&gui_state.style_title_shadow, lv_palette_main(LV_PALETTE_GREY));

    lv_style_init(&gui_state.style_artist);
    lv_style_set_text_font(&gui_state.style_artist, &lv_font_montserrat_28);
    lv_style_set_text_color(&gui_state.style_artist, lv_color_white());

    lv_style_init(&gui_state.style_info);
    lv_style_set_text_font(&gui_state.style_info, &lv_font_montserrat_16);
    lv_style_set_text_color(&gui_state.style_info, lv_color_white());


    gui_state.image_background = lv_img_create(screen);
    lv_obj_set_pos(gui_state.image_background, 0, 0);
    lv_obj_set_size(gui_state.image_background, 800, 340);
    lv_img_set_antialias(gui_state.image_background, false);
    //lv_img_set_auto_size(gui_state.image_background, false);
    lv_img_set_src(gui_state.image_background, &cover_background);

    gui_state.label_title_shadow = lv_label_create(screen);
    lv_label_set_long_mode(gui_state.label_title_shadow, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_pos(gui_state.label_title_shadow, 352, 62);
    lv_obj_set_size(gui_state.label_title_shadow, 430, 50);
    lv_obj_add_style(gui_state.label_title_shadow, &gui_state.style_title_shadow, 0);
    lv_label_set_text(gui_state.label_title_shadow, "");

    gui_state.label_title = lv_label_create(screen);
    lv_label_set_long_mode(gui_state.label_title, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_pos(gui_state.label_title, 350, 60);
    lv_obj_set_size(gui_state.label_title, 430, 50);
    lv_obj_add_style(gui_state.label_title, &gui_state.style_title, 0);
    lv_label_set_text(gui_state.label_title, "");

    gui_state.label_artist = lv_label_create(screen);
    lv_label_set_long_mode(gui_state.label_artist, LV_LABEL_LONG_DOT);
    lv_obj_set_pos(gui_state.label_artist, 350, 110);
    lv_obj_set_size(gui_state.label_artist, 430, 30);
    lv_obj_add_style(gui_state.label_artist, &gui_state.style_artist, 0);
    lv_label_set_text(gui_state.label_artist, "");

    // Song progress indicator (2 labels, 1 bar)
    gui_state.label_currentposition = lv_label_create(screen);
    lv_obj_set_pos(gui_state.label_currentposition, 350, 300);
    lv_obj_set_size(gui_state.label_currentposition, 75, 30);
    lv_obj_add_style(gui_state.label_currentposition, &gui_state.style_info, 0);
    lv_label_set_text(gui_state.label_currentposition, "-:--");

    gui_state.label_totalposition = lv_label_create(screen);
    lv_obj_set_pos(gui_state.label_totalposition, 740, 300);
    lv_obj_set_size(gui_state.label_totalposition, 75, 30);
    lv_obj_add_style(gui_state.label_totalposition, &gui_state.style_info, 0);
    lv_label_set_text(gui_state.label_totalposition, "-:--");

    gui_state.bar_trackprogress = lv_bar_create(screen);
    lv_obj_set_pos(gui_state.bar_trackprogress, 350, 275);
    lv_obj_set_size(gui_state.bar_trackprogress, 430, 20);
    lv_bar_set_range(gui_state.bar_trackprogress, 0, 258);
    lv_bar_set_value(gui_state.bar_trackprogress, 198, LV_ANIM_OFF);

    lv_obj_t *coverborder = lv_img_create(screen);
    lv_obj_set_pos(coverborder, 0, 0);
    lv_obj_set_size(coverborder, 340, 340);
    lv_img_set_src(coverborder, "./coverborder.png");

    gui_state.image_coverart = lv_img_create(screen);
    lv_obj_set_pos(gui_state.image_coverart, 10, 10);
    lv_obj_set_size(gui_state.image_coverart, 320, 320);
    lv_img_set_antialias(gui_state.image_coverart, false);
    //lv_img_set_auto_size(gui_state.image_coverart, false);
    lv_img_set_src(gui_state.image_coverart, "./cover.png");
}

void gui_mpris_poll_task()
{
    while (1)
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
            pthread_mutex_lock(&lock);
            lv_label_set_text(gui_state.label_title_shadow, mplay->properties.metadata.title);
            lv_label_set_text(gui_state.label_title, mplay->properties.metadata.title);
            lv_label_set_text(gui_state.label_artist, mplay->properties.metadata.album_artist);

            gui_format_seconds_string(mplay->properties.position, display_time_buffer, sizeof(display_time_buffer));
            lv_label_set_text(gui_state.label_currentposition, display_time_buffer);

            gui_format_seconds_string(mplay->properties.metadata.length, display_time_buffer, sizeof(display_time_buffer));
            lv_label_set_text(gui_state.label_totalposition, display_time_buffer);

            lv_bar_set_range(gui_state.bar_trackprogress, 0, mplay->properties.metadata.length / 1000000);
            lv_bar_set_value(gui_state.bar_trackprogress, mplay->properties.position / 1000000, LV_ANIM_ON);
            pthread_mutex_unlock(&lock);

            if (!!strncmp(gui_state.coverart_url, mplay->properties.metadata.art_url, strlen(mplay->properties.metadata.art_url)))
            {
                strncpy(gui_state.coverart_url, mplay->properties.metadata.art_url, 1024);
                gui_fetch_coverart_from_url(gui_state.coverart_url);
            }
        }
        usleep(500000);
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

    uint8_t filename[FILENAME_MAX];
    tmpnam(filename);

    cover_download(url, filename);

    cover_decode(filename, cover_buffer_1, COVER_IMAGE_X, COVER_IMAGE_Y);
    cover_blur_background(filename, cover_background_buffer, COVER_BACKGROUND_X, COVER_BACKGROUND_Y);
    
    unlink(filename);

    lv_img_set_src(gui_state.image_coverart, &cover_1);
    lv_img_set_src(gui_state.image_background, &cover_background);
}