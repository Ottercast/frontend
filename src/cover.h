#pragma once
#include <MagickWand/MagickWand.h>
#include <curl/curl.h>

#define COVER_IMAGE_X 320
#define COVER_IMAGE_Y 320
#define COVER_IMAGE_SIZE (COVER_IMAGE_X * COVER_IMAGE_Y * LV_COLOR_SIZE / 8)

#define COVER_BACKGROUND_X 800
#define COVER_BACKGROUND_Y 340
#define COVER_BACKGROUND_SIZE (COVER_BACKGROUND_X * COVER_BACKGROUND_Y * LV_COLOR_SIZE / 8)

void cover_decode(uint8_t *filename, uint8_t *buf_cover, uint16_t width, uint16_t height);
void cover_blur_background(uint8_t *filename, uint8_t *buf_background, uint16_t width, uint16_t height);
void cover_download(const char *url, const char *filename);