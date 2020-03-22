/************************************************************
*Copyright (C),lcb0281at163.com lcb0281atgmail.com
*BlogAddr: caibiao-lee.blog.csdn.net
*FileName: debug_font_osd.c
*Description:测试生成带时间字符的图像
*Date:     2020-02-03
*Author:   Caibiao Lee
*Version:  V1.0
*Others:
*History:
***********************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "SDL/SDL.h"
#include "SDL/SDL_ttf.h"

#define FONT_PATH       "./font/hisi_osd.ttf"

int string_to_bmp(char *pu8Str)
{
    SDL_PixelFormat *fmt;
    TTF_Font *font;  
    SDL_Surface *text, *temp;  

    if (TTF_Init() < 0 ) 
    {  
        fprintf(stderr, "Couldn't initialize TTF: %s\n",SDL_GetError());  
        SDL_Quit();
    }  

    font = TTF_OpenFont(FONT_PATH, 80); 
    if ( font == NULL ) 
    {  
        fprintf(stderr, "Couldn't load %d pt font from %s: %s\n",18,"ptsize", SDL_GetError());  
    }  

    SDL_Color forecol = { 0xff, 0xff, 0xff, 0xff };  
    text = TTF_RenderUTF8_Solid(font, pu8Str, forecol);

    fmt = (SDL_PixelFormat*)malloc(sizeof(SDL_PixelFormat));
    memset(fmt,0,sizeof(SDL_PixelFormat));
    fmt->BitsPerPixel = 16;
    fmt->BytesPerPixel = 2;
    fmt->colorkey = 0xffffffff;
    fmt->alpha = 0xff;

    temp = SDL_ConvertSurface(text,fmt,0);
    SDL_SaveBMP(temp, "save.bmp"); 

    SDL_FreeSurface(text);  
    SDL_FreeSurface(temp);
    TTF_CloseFont(font);  
    TTF_Quit();  

    return 0;
}

int CreateTimeBmpPicture(void)
{
    time_t     l_stTime;
    struct tm  l_stTm;
    struct tm *l_pstTm=&l_stTm;
    char s8Contenx[128]={0};

    time(&l_stTime);
    localtime_r(&l_stTime,l_pstTm); 
    snprintf(s8Contenx,sizeof(s8Contenx), "20%02d-%02d-%02d-%02d:%02d:%02d",\
        (l_pstTm->tm_year-100), (1+l_pstTm->tm_mon), l_pstTm->tm_mday,\
            l_pstTm->tm_hour, l_pstTm->tm_min, l_pstTm->tm_sec);

    printf("string: %s \n",s8Contenx);
    string_to_bmp(s8Contenx);        
}

int main(void)
{
    printf("hello world \n");
    CreateTimeBmpPicture();
    return 0;
}




