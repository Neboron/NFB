#ifndef LINUX_FB_H
#define LINUX_FB_H

#ifdef __cplusplus
extern "C" {
#endif

//============================//
//          INCLUDES          //
//============================//
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>



//============================//
//           DEFINES          //
//============================//
#define USE_FBDEV
#define FBDEV_PATH "/dev/fb0"



//============================//
//          TYPEDEFS          //
//============================//
typedef struct
{
    /*New data for this type */
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;
    int width, height;
    int pos_x, pos_y;
    size_t buffer_len;
    unsigned char *buffer_ptr;
} linux_fb;


int  fb_init();
void draw_point(uint16_t x, uint16_t y, uint32_t collor);
void draw_rectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t collor);
void draw_rectangle_rgb(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t r, uint8_t g, uint8_t b);
void fb_flush(int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t *pixel_array);



#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LINUX_FB */
