// functions to operate the ST7789 on the PIC32
// adapted from https://github.com/sumotoy/TFT_ST7789
// and https://github.com/adafruit/Adafruit-ST7789-Library

// pin connections:
// GND - GND
// VCC - 3.3V
// SCL - B14
// SDA - B13
// RES - B15
// DC - B12
// BLK - NC
#include <stdio.h>
#include <xc.h>
#include <math.h> 
#include "ST7789.h"
#include "spi.h"
#include "font.h"

#pragma config DEBUG = OFF // disable debugging
#pragma config JTAGEN = OFF // disable jtag
#pragma config ICESEL = ICS_PGx1 // use PGED1 and PGEC1
#pragma config PWP = OFF // disable flash write protect
#pragma config BWP = OFF // disable boot write protect
#pragma config CP = OFF // disable code protect

// DEVCFG1
#pragma config FNOSC = FRCPLL  // use primary oscillator with pll
#pragma config FSOSCEN = OFF // disable secondary oscillator
#pragma config IESO = OFF // disable switching clocks
#pragma config POSCMOD = HS // high speed crystal mode
#pragma config OSCIOFNC = OFF // disable clock output
#pragma config FPBDIV = DIV_1 // divide sysclk freq by 1 for peripheral bus clock
#pragma config FCKSM = CSDCMD // disable clock switch and FSCM
#pragma config WDTPS = PS1048576 // use largest wdt
#pragma config WINDIS = OFF // use non-window mode wdt
#pragma config FWDTEN = OFF // wdt disabled
#pragma config FWDTWINSZ = WINSZ_25 // wdt window at 25%

// DEVCFG2 - get the sysclk clock to 48MHz from the 8MHz crystal
#pragma config FPLLIDIV = DIV_2 // divide input clock to be in range 4-5MHz
#pragma config FPLLMUL = MUL_24 // multiply clock after FPLLIDIV
#pragma config FPLLODIV = DIV_2 // divide clock after FPLLMUL to get 48MHz

// DEVCFG3
#pragma config USERID = 0 // some 16bit userid, doesn't matter what
#pragma config PMDL1WAY = OFF// allow multiple reconfigurations
#pragma config IOL1WAY = OFF // allow multiple reconfigurations

void LCD_command(unsigned char com) {
    LATBbits.LATB12 = 0; // DC
    spi_io(com);
}

void LCD_data(unsigned char dat) {
    LATBbits.LATB12 = 1; // DC
    spi_io(dat);
}

void LCD_data16(unsigned short dat) {
    LATBbits.LATB12 = 1; // DC
    spi_io(dat>>8);
    spi_io(dat);
}

void LCD_init() {
  unsigned int time = 0;
  LCD_command(ST7789_SWRESET); //software reset
  time = _CP0_GET_COUNT();
  while (_CP0_GET_COUNT() < time + 48000000/2*0.15) {}
  
  LCD_command(ST7789_SLPOUT); //exit sleep
  time = _CP0_GET_COUNT();
  while (_CP0_GET_COUNT() < time + 48000000/2*0.5) {}
  
  LCD_command(ST7789_COLMOD);
  LCD_data(0x55);
  time = _CP0_GET_COUNT();
  while (_CP0_GET_COUNT() < time + 48000000/2*0.01) {}
  
  LCD_command(ST7789_MADCTL);
  LCD_data(0x00);
  time = _CP0_GET_COUNT();
  while (_CP0_GET_COUNT() < time + 48000000/2*0.001) {}
  
  LCD_command(ST7789_CASET);
  LCD_data(0x00);
  LCD_data(ST7789_XSTART);
  LCD_data((240+ST7789_XSTART)>>8);
  LCD_data((240+ST7789_XSTART)&0xFF);
  time = _CP0_GET_COUNT();
  while (_CP0_GET_COUNT() < time + 48000000/2*0.001) {}

  LCD_command(ST7789_RASET);
  LCD_data(0x00);
  LCD_data(ST7789_YSTART);
  LCD_data((240+ST7789_YSTART)>>8);
  LCD_data((240+ST7789_YSTART)&0xFF);
  time = _CP0_GET_COUNT();
  while (_CP0_GET_COUNT() < time + 48000000/2*0.001) {}
  
  LCD_command(ST7789_INVON);
  time = _CP0_GET_COUNT();
  while (_CP0_GET_COUNT() < time + 48000000/2*0.01) {}

  LCD_command(ST7789_NORON);
  time = _CP0_GET_COUNT();
  while (_CP0_GET_COUNT() < time + 48000000/2*0.01) {}
  
  LCD_command(ST7789_DISPON);
  time = _CP0_GET_COUNT();
  while (_CP0_GET_COUNT() < time + 48000000/2*0.5) {}
}

void LCD_drawPixel(unsigned short x, unsigned short y, unsigned short color) {
  // should check boundary first
  LCD_setAddr(x,y,x+1,y+1);
  LCD_data16(color);
}

void LCD_setAddr(unsigned short x0, unsigned short y0, unsigned short x1, unsigned short y1) {
  LCD_command(ST7789_CASET); // Column
  LCD_data16(x0+ST7789_XSTART);
  LCD_data16(x1+ST7789_XSTART);
  
  LCD_command(ST7789_RASET); // Page
  LCD_data16(y0+ST7789_YSTART);
  LCD_data16(y1+ST7789_YSTART);

  LCD_command(ST7789_RAMWR); // Into RAM
}

void LCD_clearScreen(unsigned short color) {
  int i;
  LCD_setAddr(0,0,_GRAMWIDTH,_GRAMHEIGH);
	for (i = 0;i < _GRAMSIZE; i++){
		LCD_data16(color);
	}
}
// drawChar function
void drawChar(unsigned char letter,unsigned short xpos,unsigned short ypos){
    //const char array[5];
    //array = ASCII[letter-32];
    int i;
    int j;
    for(i = 0;i < 5;i++){
        unsigned short currentrow = ASCII[letter-0x20][i];
        for(j = 0;j<8;j++){
            if((xpos+i < 240) & (ypos+j)<240){
                if(((currentrow >> j) & 0b1) == 1){
                    LCD_drawPixel((xpos+i),(ypos+j),WHITE);
                }else{
                    LCD_drawPixel((xpos+i),(ypos+j),BLACK);
            }
            }
        }
    }
}   
void drawString(char message[22],unsigned short xpos, unsigned short ypos){
    int k = 0;
    char mes[22];
    sprintf(mes,message);
    //sprintf(mes,"NICE ROCK!");
    while(mes[k]){
        drawChar(mes[k],(xpos+6*k),ypos);
        k++;
    }
}
void LCDprogress(int xpos, int ypos,int length, int progress){
    //Draw Outline
    int height = 10;
    int j;
    int k;
    int i;
    for(j = 0;j<height;j++){
        LCD_drawPixel(xpos+length-1,ypos+j,WHITE);
    }
    for(j = 0;j<length;j++){
        LCD_drawPixel(xpos+j,ypos,WHITE);
        LCD_drawPixel(xpos+j,ypos+height,WHITE);
    }
    int howmuch = progress;
    for(i = 0;i<howmuch;i++){
        for(k = 0;k<height;k++){
            LCD_drawPixel(xpos+i,ypos+k,WHITE);
        }
    }
}
void main(){
    
    __builtin_disable_interrupts(); // disable interrupts while initializing things
    // set the CP0 CONFIG register to indicate that kseg0 is cacheable (0x3)
    __builtin_mtc0(_CP0_CONFIG, _CP0_CONFIG_SELECT, 0xa4210583);
      // 0 data RAM access wait states
    BMXCONbits.BMXWSDRM = 0x0;
    
    initSPI();

    // enable multi vector interrupts
    INTCONbits.MVEC = 0x1;

    // disable JTAG to get pins back
    DDPCONbits.JTAGEN = 0;
    
    TRISAbits.TRISA4 = 0;
    LATAbits.LATA4 = 1;
    
    LCD_init();
    __builtin_enable_interrupts();
    
    LCD_clearScreen(BLUE);
    LATAbits.LATA4 = 1;
    unsigned char let = 0x55;
    unsigned short x = 100;
    unsigned short y = 100;
    //drawChar(let,x,y);
    char message[10]; 
    sprintf(message,"hello");
    unsigned short x2 = 50;
    unsigned short y2 = 59;
    
    //drawString(message,x2,y2);
    int x3 = 75;
    int y3 = 150;
    int this = 100;
    int that = 60;
    //progress(x3,y3,this,that);
    int x4 = 28;
    int y4 = 32;
    int i; 
    
    for(i = 0;i<100;i++){
        char message[22]; 
        
        
        int length = 100;
        int prog = i;
        
        //LCDprogress(x3,y3,this,that);
        int y5 = y4+20;
        LCDprogress(x4,y5,length,i);
        sprintf(message,"Hello world %d percent done",i);
        drawString(message,x4,y4);
      
    }
    
}
//main();


// drawString function