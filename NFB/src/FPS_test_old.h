#ifndef FPS_TEST_H
#define FPS_TEST_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/time.h>
#include <stdio.h>
#include <linux/input.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdbool.h>


#define	KEYBOARD_MONITOR_INPUT_FIFO_NAME		"KeyboardMonitorInputFifo"
#define BITS_PER_LONG (sizeof(long) * 8)
#define NBITS(x) ((((x)-1)/BITS_PER_LONG)+1)


typedef struct
{
    bool W;
    bool S;
    bool A;
    bool D;
}keys;




int nScreenWidth = 120; // Ширина консольного окна
int nScreenHeight = 40; // Высота консольного окна

float fPlayerX = 1.0f; // Координата игрока по оси X
float fPlayerY = 1.0f; // Координата игрока по оси Y
float fPlayerA = 0.0f; // Направление игрока

int nMapHeight = 16; // Высота игрового поля
int nMapWidth = 16;  // Ширина игрового поля

float fFOV = 3.14159 / 3; // Угол обзора (поле видимости)
float fDepth = 30.0f;     // Максимальная дистанция обзора
struct timeval tv;
double tp1, tp2;
float duration;
float fElapsedTime;
keys key;
int aaa;
int *var1 = &aaa;


char map[16][16] = {"################",
                    "#..............#",
                    "#..............#",
                    "#..............#",
                    "#..............#",
                    "#..............#",
                    "#..............#",
                    "#..............#",
                    "#..............#",
                    "#..............#",
                    "#..............#",
                    "#..............#",
                    "#..............#",
                    "#..............#",
                    "#..............#",
                    "################"};



int input_fifo_filestream = -1;
void KeyboardMonitorInit(void)
{
    int result;
    printf("Making KeyboardMonitor FIFO...\n");
    //(This will fail if the fifo already exists in the system from the app previously running, this is fine)
    result = mkfifo(KEYBOARD_MONITOR_INPUT_FIFO_NAME, 0777);
    if (result == 0)
    {
        //FIFO CREATED
        printf("New FIFO created: %s\n", KEYBOARD_MONITOR_INPUT_FIFO_NAME);
    }
    printf("Process %d opening FIFO %s\n", getpid(), KEYBOARD_MONITOR_INPUT_FIFO_NAME);

    input_fifo_filestream = open(KEYBOARD_MONITOR_INPUT_FIFO_NAME, (O_RDONLY | O_NONBLOCK));
    if (input_fifo_filestream != -1)
            printf("Opened input FIFO: %i\n", input_fifo_filestream);

    //-------------------------------------------------------//
    //----- RUN KEYBOARD CAPTURE ON A BACKGROUND THREAD -----//
    //-------------------------------------------------------//
    int pid2 = fork();
    if(pid2 == 0)
    {
        //----- THIS IS THE CHILD THREAD -----//
        printf("KeyboardMonitor child thread started");

        int FileDevice;
        int ReadDevice;
        int Index;
        struct input_event InputEvent[64];
        int version;
        unsigned short id[4];
        unsigned long bit[EV_MAX][NBITS(KEY_MAX)];
        uint8_t NoError = 1;

        //----- OPEN THE INPUT DEVICE -----
        if ((FileDevice = open("/dev/input/by-path/platform-i8042-serio-0-event-kbd", O_RDONLY)) < 0) //<<<<SET THE INPUT DEVICE PATH HERE
        {
            perror("KeyboardMonitor can't open input device");
            NoError = 0;
        }
        //----- GET DEVICE VERSION -----
        if (ioctl(FileDevice, EVIOCGVERSION, &version))
        {
            perror("KeyboardMonitor can't get version");
            NoError = 0;
        }
        printf("Input driver version is %d.%d.%d\n", version >> 16, (version >> 8) & 0xff, version & 0xff);

        //----- GET DEVICE INFO -----
        if (NoError)
        {
            ioctl(FileDevice, EVIOCGID, id);
            printf("Input device ID: bus 0x%x vendor 0x%x product 0x%x version 0x%x\n", id[ID_BUS], id[ID_VENDOR], id[ID_PRODUCT], id[ID_VERSION]);
            memset(bit, 0, sizeof(bit));
            ioctl(FileDevice, EVIOCGBIT(0, EV_MAX), bit[0]);
        }

        printf("KeyboardMonitor child thread running\n");

        //Create an output filestream for this child thread
        int output_fifo_filestream = -1;
        output_fifo_filestream = open(KEYBOARD_MONITOR_INPUT_FIFO_NAME, (O_WRONLY | O_NONBLOCK));

        //----- READ KEYBOARD EVENTS -----
        while (NoError)
        {
            ReadDevice = read(FileDevice, InputEvent, sizeof(struct input_event) * 64);
            if (ReadDevice < (int) sizeof(struct input_event))
            {
                //This should never happen
                perror("KeyboardMonitor error reading - keyboard lost?");
                NoError = 0;
            }
            else
            {
                for (Index = 0; Index < ReadDevice / sizeof(struct input_event); Index++)
                {
                    if (InputEvent[Index].type == EV_KEY)
                    {
                        if (InputEvent[Index].value == 2)
                        {
                            //This is an auto repeat of a held down key
                            //ss1 << (int)(InputEvent[Index].code) << " Auto Repeat";
                        }
                        else if (InputEvent[Index].value == 1)
                        {
                            printf("%d\n", InputEvent[Index].code);
                            //----- KEY DOWN -----
                            if(InputEvent[Index].code == 17) key.W = true;
                            if(InputEvent[Index].code == 31) key.S = true;
                            if(InputEvent[Index].code == 30) key.A = true;
                            if(InputEvent[Index].code == 32) key.D = true;
                        }
                        else if (InputEvent[Index].value == 0)
                        {
                            if(InputEvent[Index].code == 17) key.W = false;
                            if(InputEvent[Index].code == 31) key.S = false;
                            if(InputEvent[Index].code == 30) key.A = false;
                            if(InputEvent[Index].code == 32) key.D = false;
                        }
                        //printf("%x\n", &var1);
                        *var1 = 1;
                    }
                }
            }
        }
        close(FileDevice);
        printf("KeyboardMonitor child thread ending!\n");
        _exit(0);
    }
}



int FPS_init()
{
    KeyboardMonitorInit();
    return 1;
}



void FPS_handlar()
{
    gettimeofday(&tv, NULL);
    tp2 = tv.tv_sec * 1000.0 + tv.tv_usec / 1000.0;
    fElapsedTime = tp2 - tp1;
    tp1 = tp2;

    if(key.A)
        fPlayerA -= (1.5f) * fElapsedTime; // Клавишей "A" поворачиваем по часовой стрелке

    if(key.D)
        fPlayerA += (1.5f) * fElapsedTime; // Клавишей "D" поворачиваем против часовой стрелки

    if(*var1 == 1)
        printf("Wdwadwaddawdwadaw\n");

}





#endif // FPS_TEST_H
