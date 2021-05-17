
#ifndef WS2812B_H__
#define WS2812B_H__

#include<xc.h> // processor SFR definitions

// link three 8bit colors together
typedef struct {
    unsigned char r;
    unsigned char g;
    unsigned char b;
} wsColor; 
void readUART1(char * string, int maxLength);
void writeUART1(const char * string);
void ws2812b_setup();
void ws2812b_setColor(int numLEDs);
void HSBtoRGB(float hue, float sat, float brightness);

#endif