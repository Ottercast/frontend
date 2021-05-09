#include "cover.h"

#define ThrowWandException(wand)                                                 \
{                                                                                \
        char *description;                                                       \
                                                                                 \
        ExceptionType severity;                                                  \
                                                                                 \
        description = MagickGetException(wand, &severity);                       \
        (void)fprintf(stderr, "%s %s %lu %s\n", GetMagickModule(), description); \
        description = (char *)MagickRelinquishMemory(description);               \
        exit(-1);                                                                \
 }

void cover_decode(uint8_t *filename, uint8_t *buf_cover, uint16_t width, uint16_t height)
{
    MagickBooleanType status;
    MagickWand *magick_wand;

    MagickWandGenesis();
    magick_wand = NewMagickWand();
    status = MagickReadImage(magick_wand, filename);
    if (status == MagickFalse)
        ThrowWandException(magick_wand);

    MagickResetIterator(magick_wand);
    while (MagickNextImage(magick_wand) != MagickFalse)
    {
        MagickResizeImage(magick_wand, width, height, TriangleFilter);
    }

    status = MagickExportImagePixels(magick_wand, 0, 0, width, height, "BGRP", CharPixel, buf_cover);

    if (status == MagickFalse)
        ThrowWandException(magick_wand);

    magick_wand = DestroyMagickWand(magick_wand);
    MagickWandTerminus();
}

void cover_blur_background(uint8_t *filename, uint8_t *buf_background, uint16_t width, uint16_t height)
{
    MagickBooleanType status;
    MagickWand *magick_wand;

    MagickWandGenesis();
    magick_wand = NewMagickWand();
    status = MagickReadImage(magick_wand, filename);
    if (status == MagickFalse)
        ThrowWandException(magick_wand);

    MagickResetIterator(magick_wand);
    while (MagickNextImage(magick_wand) != MagickFalse)
    {
        MagickCropImage(magick_wand, 640, 640, 0, 150);
        MagickResizeImage(magick_wand, 10, 4, TriangleFilter);
        MagickResizeImage(magick_wand, width, height, TriangleFilter);
    }

    status = MagickExportImagePixels(magick_wand, 0, 0, width, height, "BGRP", CharPixel, buf_background);

    if (status == MagickFalse)
        ThrowWandException(magick_wand);

    magick_wand = DestroyMagickWand(magick_wand);
    MagickWandTerminus();
}

void cover_download(const char *url, const char *filename)
{
    CURL *curl;
    FILE *fp;
    CURLcode res;

    curl = curl_easy_init();
    if (curl)
    {
        fp = fopen(filename, "wb");
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        fclose(fp);
    }
    else
    {
        fprintf(stderr, "curl failed: %d\n", curl);
    }
}

