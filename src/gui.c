#include "gui.h"

struct
{
    lv_obj_t *label_title;
    lv_obj_t *label_artist;
    lv_obj_t *bar_trackprogress;
    lv_obj_t *label_currentposition;
    lv_obj_t *label_totalposition;

    lv_obj_t *image_coverart; 
} gui_state;

void ottercast_frontend_draw_display()
{
    lv_obj_t *screen = lv_scr_act();

    gui_state.label_title = lv_label_create(screen, NULL);
    lv_label_set_long_mode(gui_state.label_title, LV_LABEL_LONG_SROLL_CIRC);
    lv_obj_set_pos(gui_state.label_title, 350, 40);
    lv_obj_set_size(gui_state.label_title, 430, 40);
    lv_label_set_text(gui_state.label_title, "Beds Are Burning - Remastered");

    gui_state.label_artist = lv_label_create(screen, NULL);
    lv_label_set_long_mode(gui_state.label_artist, LV_LABEL_LONG_SROLL_CIRC);
    lv_obj_set_pos(gui_state.label_artist, 350, 80);
    lv_obj_set_size(gui_state.label_artist, 430, 30);
    lv_label_set_text(gui_state.label_artist, "Midnight Oil");

    // Song progress indicator (2 labels, 1 bar)
    gui_state.label_currentposition = lv_label_create(screen, NULL);
    lv_obj_set_pos(gui_state.label_currentposition, 350, 140);
    lv_obj_set_size(gui_state.label_currentposition, 75, 30);
    lv_label_set_text(gui_state.label_currentposition, "3:18");

    gui_state.label_totalposition = lv_label_create(screen, NULL);
    lv_obj_set_pos(gui_state.label_totalposition, 730, 140);
    lv_obj_set_size(gui_state.label_totalposition, 75, 30);
    lv_label_set_text(gui_state.label_totalposition, "4:18");

    gui_state.bar_trackprogress = lv_bar_create(screen, NULL);
    lv_obj_set_pos(gui_state.bar_trackprogress, 350, 115);
    lv_obj_set_size(gui_state.bar_trackprogress, 430, 20);
    lv_bar_set_range(gui_state.bar_trackprogress, 0, 258);
    lv_bar_set_value(gui_state.bar_trackprogress, 198, LV_ANIM_OFF);

    LV_IMG_DECLARE(cover);

    gui_state.image_coverart = lv_img_create(screen, NULL);
    lv_img_set_src(gui_state.image_coverart, &cover);
    lv_obj_set_pos(gui_state.image_coverart, 10, 10);
    lv_obj_set_size(gui_state.image_coverart, 320, 320);
}