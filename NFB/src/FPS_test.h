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
#include <math.h>

#include "linux_fb.h"



#define BITS_PER_LONG (sizeof(long) * 8)
#define NBITS(x) ((((x)-1)/BITS_PER_LONG)+1)

#define clear() printf("\033[H\033[J")
#define gotoxy(x,y) printf("\033[%d;%dH", (y), (x))


typedef struct
{
    bool W;
    bool S;
    bool A;
    bool D;
}keys;




int nScreenWidth = 1280;
int nScreenHeight = 720;
uint32_t nScreenBuffer[1280*720];

float fPlayerX = 1.0f; // Координата игрока по оси X
float fPlayerY = 1.0f; // Координата игрока по оси Y
float fPlayerA = 0.0f; // Направление игрока

int nMapHeight = 32; // Высота игрового поля
int nMapWidth = 32;  // Ширина игрового поля

float fFOV = 3.14159 / 3; // Угол обзора (поле видимости)
float fDepth = 40.0f;     // Максимальная дистанция обзора



struct timeval tv;
long tp1, tp2;
float fElapsedTime;

int FileDevice;
struct input_event InputEvent[64];
int Index;
keys key;

char map[32][32] = {"################################",
                   "#..............................#",
                   "#..............................#",
                   "#..............................#",
                   "#..............................#",
                   "#......##......##......####....#",
                   "#......##......##......##......#",
                   "#......##...########...##......#",
                   "#......##......##......##......#",
                   "#....####......##......####....#",
                   "#..............................#",
                   "#..............................#",
                   "#..............................#",
                   "#..............................#",
                   "#..............................#",
                   "################################"};




void keyboard_init(char* keyboard_file)
{
    int version;
    unsigned short id[4];
    unsigned long bit[EV_MAX][NBITS(KEY_MAX)];

    //----- OPEN THE INPUT DEVICE -----
    if ((FileDevice = open(keyboard_file, O_RDONLY | O_NONBLOCK)) < 0)
    {
        perror("KeyboardMonitor can't open input device");
        close(FileDevice);
        return;
    }

    //----- GET DEVICE VERSION -----
    if (ioctl(FileDevice, EVIOCGVERSION, &version))
    {
        perror("KeyboardMonitor can't get version");
        close(FileDevice);
        return;
    }
    printf("Input driver version is %d.%d.%d\n", version >> 16, (version >> 8) & 0xff, version & 0xff);

    //----- GET DEVICE INFO -----
    ioctl(FileDevice, EVIOCGID, id);
    printf("Input device ID: bus 0x%x vendor 0x%x product 0x%x version 0x%x\n", id[ID_BUS], id[ID_VENDOR], id[ID_PRODUCT], id[ID_VERSION]);

    memset(bit, 0, sizeof(bit));
    ioctl(FileDevice, EVIOCGBIT(0, EV_MAX), bit[0]);
}



void keyboard_monitor(void)
{
    //----- READ KEYBOARD EVENTS -----
    int ReadDevice = read(FileDevice, InputEvent, sizeof(struct input_event) * 64);

    if (ReadDevice < (int) sizeof(struct input_event))
    {
        //This should never happen
        //perror("KeyboardMonitor error reading - keyboard lost?");
        //close(FileDevice);
        //return;
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
                }
                else if (InputEvent[Index].value == 1)
                {
                    //----- KEY DOWN -----
                    if(InputEvent[Index].code == 17) key.W = true;
                    if(InputEvent[Index].code == 31) key.S = true;
                    if(InputEvent[Index].code == 30) key.A = true;
                    if(InputEvent[Index].code == 32) key.D = true;

                }
                else if (InputEvent[Index].value == 0)
                {
                    //----- KEY UP -----
                    if(InputEvent[Index].code == 17) key.W = false;
                    if(InputEvent[Index].code == 31) key.S = false;
                    if(InputEvent[Index].code == 30) key.A = false;
                    if(InputEvent[Index].code == 32) key.D = false;
                }
            }
        }
    }
}



void FPS_init(char* keyboard_file)
{
    keyboard_init(keyboard_file);
}



void FPS_handlar()
{
    gettimeofday(&tv, NULL);
    tp2 = tv.tv_sec * 1000.0 + tv.tv_usec / 1000.0;
    fElapsedTime = (tp2 - tp1)/1000.0;
    tp1 = tp2;

    // FPS calculation
    static uint8_t a_cnt;
    static float time_sum, fps;
    time_sum += fElapsedTime;
    a_cnt++;
    if(time_sum >= 1.0)
    {
        fps = 1/(time_sum/a_cnt);
        time_sum = 0;
        a_cnt = 0;
    }
    gotoxy(1, 1);
    printf("FPS %f\n", fps);



    keyboard_monitor();

    if(key.A)
        fPlayerA -= (1.5f) * fElapsedTime; // Клавишей "A" поворачиваем по часовой стрелке

    if(key.D)
        fPlayerA += (1.5f) * fElapsedTime; // Клавишей "D" поворачиваем против часовой стрелки

    if(key.W)
    {
        fPlayerX += sinf(fPlayerA) * 5.0f * fElapsedTime;
        fPlayerY += cosf(fPlayerA) * 5.0f * fElapsedTime;

        if (map[(int)fPlayerY] [(int)fPlayerX] == '#') // Если столкнулись со стеной, но откатываем шаг
        {
            fPlayerX -= sinf(fPlayerA) * 5.0f * fElapsedTime;
            fPlayerY -= cosf(fPlayerA) * 5.0f * fElapsedTime;
        }
    }

    if(key.S)
    {
        fPlayerX -= sinf(fPlayerA) * 5.0f * fElapsedTime;
        fPlayerY -= cosf(fPlayerA) * 5.0f * fElapsedTime;

        if (map[(int)fPlayerY] [(int)fPlayerX] == '#') // Если столкнулись со стеной, но откатываем шаг
        {
            fPlayerX += sinf(fPlayerA) * 5.0f * fElapsedTime;
            fPlayerY += cosf(fPlayerA) * 5.0f * fElapsedTime;
        }
    }

    for (int x = 0; x < nScreenWidth; x++) // Проходим по всем X
    {
        float fRayAngle = (fPlayerA - fFOV/2.0f) + ((float)x / (float)nScreenWidth) * fFOV; // Направление луча

        // Находим расстояние до стенки в направлении fRayAngle
        float fDistanceToWall = 0.0f; // Расстояние до препятствия в направлении fRayAngle
        bool bHitWall = false; // Достигнул ли луч стенку

        float fEyeX = sinf(fRayAngle); // Координаты единичного вектора fRayAngle
        float fEyeY = cosf(fRayAngle);

        while (!bHitWall && fDistanceToWall < fDepth) // Пока не столкнулись со стеной
        {                                             // Или не вышли за радиус видимости
            fDistanceToWall += 0.02f;

            int nTestX = (int)(fPlayerX + fEyeX*fDistanceToWall); // Точка на игровом поле
            int nTestY = (int)(fPlayerY + fEyeY*fDistanceToWall); // в которую попал луч

            if (nTestX < 0 || nTestX >= nMapWidth || nTestY < 0 || nTestY >= nMapHeight)
            { // Если мы вышли за зону
                bHitWall = true;
                fDistanceToWall = fDepth;
            }
            else if(map[nTestY] [nTestX] == '#')
            {
                bHitWall = true;
            }

        }
        //Вычисляем координаты начала и конца стенки по формулам (1) и (2)
        int nCeiling = (float)(nScreenHeight/2.0) - nScreenHeight / ((float)fDistanceToWall);
        int nFloor = nScreenHeight - nCeiling;

        uint32_t nShade;

        nShade = 255-(fDistanceToWall * 255 / fDepth);
        nShade = nShade << 8;

        for (int y = 0; y < nScreenHeight; y++)
        {
            if (y <= nCeiling)
                nScreenBuffer[y*nScreenWidth + x] = 0;
            else if(y > nCeiling && y <= nFloor)
                nScreenBuffer[y*nScreenWidth + x] = nShade;
            else
            {
                // То же самое с полом - более близкие части рисуем более заметными символами
                float b = 1.0f - ((float)y - nScreenHeight / 2.0) / ((float)nScreenHeight / 2.0);
                nShade = 255 - (b * 255);

                nScreenBuffer[y*nScreenWidth + x] = nShade;
            }
        }
    }

    fb_flush(0, 20, nScreenWidth-1, nScreenHeight, nScreenBuffer);
}





#endif // FPS_TEST_H
