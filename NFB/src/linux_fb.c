
// Includes:
#include "linux_fb.h"


#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stddef.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/fb.h>




//============================//
//      STATIC VARIABLES      //
//============================//
linux_fb fb;
static struct fb_var_screeninfo vinfo;
static struct fb_fix_screeninfo finfo;
int fbfd;



//============================//
//      GLOBAL FUNCTIONS      //
//============================//

int fb_init()
{
        // Open the file for reading and writing
        fbfd = open(FBDEV_PATH, O_RDWR);
        if(fbfd == -1)
        {
            perror("Error: cannot open framebuffer device");
            return -1;
        }
        printf("The framebuffer device was opened successfully.\n");

        // Get fixed screen information
        if(ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo) == -1)
        {
            perror("Error reading fixed information");
            return -1;
        }

        // Get variable screen information
        if(ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo) == -1)
        {
            perror("Error reading variable information");
            return -1;
        }

        printf("%dx%d, %dbpp\n", vinfo.xres, vinfo.yres, vinfo.bits_per_pixel);

        if (getenv("WIDTH")) fb.width = atoi(getenv("WIDTH"));
        else fb.width = vinfo.xres;
        if (getenv("HEIGHT")) fb.height = atoi(getenv("HEIGHT"));
        else fb.height = vinfo.yres;
        if (getenv("POSX")) fb.pos_x = atoi(getenv("POSX"));
        else fb.pos_x = 0;
        if (getenv("POSY")) fb.pos_y = atoi(getenv("POSY"));
        else fb.pos_y = 0;

        fb.buffer_len = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;
        // Map the device to memory
        fb.buffer_ptr = (char *)mmap(0, fb.buffer_len, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
        if((intptr_t)fb.buffer_ptr == -1)
        {
            perror("Error: failed to map framebuffer device to memory");
            return -1;
        }
        memset(fb.buffer_ptr, 0, fb.buffer_len);

        printf("The framebuffer device was mapped to memory successfully!!.\n");
        return 1;
}



void fb_exit(void)
{
    close(fbfd);
}



void draw_rectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t collor)
{
    uint8_t *data, r, g, b, a;
    data = fb.buffer_ptr + (fb.pos_y + y) * vinfo.xres * vinfo.bits_per_pixel/8
                         + (fb.pos_x + x) * vinfo.bits_per_pixel/8;
    b = (uint8_t)collor;
    g = (uint8_t)collor << 1;
    r = (uint8_t)collor << 2;
    a = (uint8_t)collor << 3;



    for (uint16_t i = 0; i < height; i++)
    {
        for (uint16_t j = 0; j < width; j++)
        {
            data[vinfo.bits_per_pixel / 8 * j + 3] = a;
            data[vinfo.bits_per_pixel / 8 * j + 2] = r;
            data[vinfo.bits_per_pixel / 8 * j + 1] = g;
            data[vinfo.bits_per_pixel / 8 * j]     = b;
        }
        data += vinfo.xres * vinfo.bits_per_pixel / 8;
    }
}



void draw_rectangle_rgb(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t r, uint8_t g, uint8_t b)
{
    uint8_t *data = fb.buffer_ptr + (fb.pos_y + y) * vinfo.xres * vinfo.bits_per_pixel/8
                                  + (fb.pos_x + x) * vinfo.bits_per_pixel/8;


    for (uint16_t i = 0; i < height; i++)
    {
        for (uint16_t j = 0; j < width; j++)
        {
            data[vinfo.bits_per_pixel / 8 * j + 2] = r;
            data[vinfo.bits_per_pixel / 8 * j + 1] = g;
            data[vinfo.bits_per_pixel / 8 * j    ] = b;
        }
        data += vinfo.xres * vinfo.bits_per_pixel / 8;
    }
}



void fb_flush(int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t *pixel_array)
{
    if(x2 < 0 ||
       y2 < 0 ||
       x1 > (int32_t)vinfo.xres - 1 ||
       y1 > (int32_t)vinfo.yres - 1)
    {
        return;
    }

    /*Truncate the area to the screen*/
    int32_t act_x1 = x1 < 0 ? 0 : x1;
    int32_t act_y1 = y1 < 0 ? 0 : y1;
    int32_t act_x2 = x2 > (int32_t)vinfo.xres - 1 ? (int32_t)vinfo.xres - 1 : x2;
    int32_t act_y2 = y2 > (int32_t)vinfo.yres - 1 ? (int32_t)vinfo.yres - 1 : y2;

    uint32_t w = (act_x2 - act_x1 + 1);
    long int location = 0;

    if(vinfo.bits_per_pixel == 32 || vinfo.bits_per_pixel == 24)
    {

        uint32_t * fbp32 = (uint32_t *)fb.buffer_ptr;
        int32_t y;

        uint32_t *buffer;
        buffer = (uint32_t*) pixel_array;

        for(y = act_y1; y <= act_y2; y++)
        {
            location = (act_x1 + vinfo.xoffset) + (y + vinfo.yoffset) * finfo.line_length / 4;
            memcpy(&fbp32[location], (uint32_t *)pixel_array, (act_x2 - act_x1 + 1) * 4);
            pixel_array += w;
        }
    }
    else
    {
        return;
    }

}





