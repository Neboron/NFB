\


#include "linux_fb.h"
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include "FPS_test.h"


struct timeval tv;
const char keyboard_file[] = "/dev/input/by-path/platform-i8042-serio-0-event-kbd";


int main()
{
    fb_init();

    FPS_init(keyboard_file);

    while(1)
    {
        FPS_handlar();
    }

    return 0;
}

