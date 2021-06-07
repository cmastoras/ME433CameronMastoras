
// I2C Master utilities, using polling rather than interrupts
// The functions must be called in the correct order as per the I2C protocol
// I2C pins need pull-up resistors, 2k-10k
#include "IMU_I2C.h"
#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include<math.h>
#include <stdio.h>


//#include <bitset>
// DEVCFG0
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

unsigned char wAdd = 0b11010100;
unsigned char rAdd = 0b11010101;



void initSPI() {
    // Turn of analog functionality of A and B pins
    ANSELA = 0;
    ANSELB = 0;
    
    
   
    
    // Pin B14 has to be SCK1 -> SCL
    
    // set SDO1 - B13 -> SDA 
    //13
    RPB13Rbits.RPB13R = 0b0011;
    
    // set B15 -> RES (reset), initialize to High
    //
    TRISBbits.TRISB15 = 0;
    LATBbits.LATB15 = 1;
    
    // set B12 -> DC (data control), initialize Low
    //
    TRISBbits.TRISB12 = 0;
    LATBbits.LATB12 = 0;
    
    // setup SPI1
    SPI1CON = 0; // turn off the spi module and reset it
    SPI1BUF; // clear the rx buffer by reading from it
    SPI1BRG = 1; // baud rate to 10 MHz [SPI1BRG = (48000000/(2*desired))-1]
    SPI1STATbits.SPIROV = 0; // clear the overflow bit
    SPI1CONbits.CKP = 1; // clock idle high **This is specific to ST77898**
    SPI1CONbits.CKE = 1; // data changes when clock goes from logic hi to lo 
    SPI1CONbits.MSTEN = 1; // master operation
    SPI1CONbits.ON = 1; // turn on spi 
    
    // blink the reset pin to reset the display
    LATBbits.LATB15 = 0;
    _CP0_SET_COUNT(0);
    while(_CP0_GET_COUNT()<24000000/1000){} // 1ms
    LATBbits.LATB15 = 1;
}

// send a byte via spi and return the response
unsigned char spi_io(unsigned char o) {
  SPI1BUF = o;
  while(!SPI1STATbits.SPIRBF) { // wait to receive the byte
    ;
  }
  return SPI1BUF;
}
////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
void setPin(unsigned char address, unsigned char regi, unsigned char value){
    i2c_master_start();
    i2c_master_send(address);
    i2c_master_send(regi);
    i2c_master_send(value);
    i2c_master_stop();
}
unsigned char readPin(unsigned char address, unsigned char regi){
    
    i2c_master_start();
    
    i2c_master_send(wAdd);
    
    i2c_master_send(regi);
    
    i2c_master_restart();
    
    i2c_master_send(rAdd);
    
    unsigned char mf_thang = i2c_master_recv();
    
    i2c_master_ack(1);
    i2c_master_stop();
    return mf_thang;
    
}   

void i2c_master_setup(void) {
    // using a large BRG to see it on the nScope, make it smaller after verifying that code works
    // look up TPGD in the datasheet
    //I2C1BRG = 1000; // I2CBRG = [1/(2*Fsck) - TPGD]*Pblck - 2 (TPGD is the Pulse Gobbler Delay)
    I2C1BRG = 0xE9;
    I2C1CONbits.ON = 1; // turn on the I2C1 module
}

void i2c_master_start(void) {
    
    I2C1CONbits.SEN = 1; // send the start bit
    
    while (I2C1CONbits.SEN == 1) {
        ;
        //LCD_clearScreen(RED);
        //LCD_clearScreen(YELLOW);
    } // wait for the start bit to be sent
    
}

void i2c_master_restart(void) {
    I2C1CONbits.RSEN = 1; // send a restart 
    while (I2C1CONbits.RSEN) {
        ;
    } // wait for the restart to clear
}

void i2c_master_send(unsigned char byte) { // send a byte to slave
    I2C1TRN = byte; // if an address, bit 0 = 0 for write, 1 for read
    while (I2C1STATbits.TRSTAT) {
        ;
    } // wait for the transmission to finish
    
    if (I2C1STATbits.ACKSTAT) { // if this is high, slave has not acknowledged  
        // ("I2C1 Master: failed to receive ACK\r\n");
        while(1){} // get stuck here if the chip does not ACK 
        LCD_clearScreen(GREEN);
    }
}

unsigned char i2c_master_recv(void) { // receive a byte from the slave
    I2C1CONbits.RCEN = 1; // start receiving data
    while (!I2C1STATbits.RBF) {
        ;
    } // wait to receive the data
    return I2C1RCV; // read and return the data
}

void i2c_master_ack(int val) { // sends ACK = 0 (slave should send another byte)
    // or NACK = 1 (no more bytes requested from slave)
    I2C1CONbits.ACKDT = val; // store ACK/NACK in ACKDT
    I2C1CONbits.ACKEN = 1; // send ACKDT
    while (I2C1CONbits.ACKEN) {
        ;
    } // wait for ACK/NACK to be sent
}

void i2c_master_stop(void) { // send a STOP:
    I2C1CONbits.PEN = 1; // comm is complete and master relinquishes bus
    while (I2C1CONbits.PEN) {
        ;
    } // wait for STOP to complete
}
//////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
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
void drawString(char message[50],unsigned short xpos, unsigned short ypos){
    int k = 0;
    char mes[50];
    sprintf(mes,message);
    //sprintf(mes,"NICE ROCK!");
    while(mes[k]){
        drawChar(mes[k],(xpos+6*k),ypos);
        k++;
    }
}

void I2C_read_multiple(unsigned char address, unsigned char regi, unsigned char * data, int length){
    i2c_master_start();
    int i;
    i2c_master_send(wAdd);
    
    i2c_master_send(regi);
    
    i2c_master_restart();
    
    i2c_master_send(rAdd);
    for(i = 0; i < length-1;i++){
        unsigned char mf_thang = i2c_master_recv();
        data[i] = mf_thang;
        i2c_master_ack(0);
    }
    unsigned char mf_thang = i2c_master_recv();
    data[length-1] = mf_thang;
    i2c_master_ack(1);
    i2c_master_stop();
    //return data;
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
/////////////////////////////////////////////////////////////////////////
void inclo(int x, int y,int xpos, int ypos){
    int i,k;
    int height = 4;
    int xmod = 1;
    int ymod = 1;
    if(x < 0){
       xmod = -1;
    }
    if(y < 0){
        ymod = -1;
    }
    //DRAW X
    int realx = abs(x);
    int realy = abs(y);
    for(i = 0;i<realx;i++){
        for(k = 0;k<height;k++){
            LCD_drawPixel((xpos+(i*xmod)),ypos+k,WHITE);
        }
    }
    //DRAW Y
    for(i = 0;i<realy;i++){
        for(k = 0;k<height;k++){
            LCD_drawPixel(xpos+k,(ypos + (i*ymod)),WHITE);
        }
    }
}
////////////////////////////////////////////////////////////////////
int main() {
    
    __builtin_disable_interrupts(); // disable interrupts while initializing things
    // set the CP0 CONFIG register to indicate that kseg0 is cacheable (0x3)
    __builtin_mtc0(_CP0_CONFIG, _CP0_CONFIG_SELECT, 0xa4210583);
      // 0 data RAM access wait states
    BMXCONbits.BMXWSDRM = 0x0;
    // enable multi vector interrupts
    INTCONbits.MVEC = 0x1;

    // disable JTAG to get pins back
    DDPCONbits.JTAGEN = 0;
    
    TRISAbits.TRISA4 = 0;
    TRISBbits.TRISB4 = 1;
    LATAbits.LATA4 = 0;
    
    //TRISBbits.TRISB3 = 0;
    //TRISBbits.TRISB2 = 0;
    

    
    initSPI();
    LCD_init();
    
    i2c_master_setup();
    ANSELB = 0;
    __builtin_enable_interrupts();
    //Set bank to 1
    //0x0A is IOCON.bank in bank = 0 default mode
    unsigned char thang;
    //LCD_clearScreen(MAGENTA);
    thang = readPin(wAdd,0x0F);
    
    if(thang != 0b01101001){
        while(1){
            LCD_clearScreen(YELLOW);
            _CP0_SET_COUNT(0);
            while(_CP0_GET_COUNT() < 48000000/4){
            }
            LATAbits.LATA4 = 1;
            //setPin(wAdd,0x0A,0xFF);
            LCD_clearScreen(RED);
            _CP0_SET_COUNT(0);
            while(_CP0_GET_COUNT() < 48000000/4){
            }
            //setPin(wAdd,0x0A,0x00);
            LATAbits.LATA4 = 0;
            }
    }else{
        LCD_clearScreen(GREEN);
    }
    //setPin(wAdd,0x0A,0b1000000);
    //Set A to be output
    //setPin(wAdd,0x00,0x00);
    //Set B to be input
    //setPin(wAdd,0x10,0xFF);
    
    //Set CRTL1_XL
    setPin(wAdd,0x10,0x82);
    
    //SET CTRL2_G
    setPin(wAdd,0x11,0x88);
    
    //Set CRTL3
    setPin(wAdd,0x12,0x04);
    
    
    
    int spacing = 14;
    int counter = 0;
    while(1){
        counter++;
        if(counter % 5 == 0){
            LCD_clearScreen(RED);
            counter = 0;
        }
        /*
        thang = readPin(rAdd,0x19);
        
        if (thang < 1){
            setPin(wAdd,0x0A,0xFF);
        }else{
            setPin(wAdd,0x0A,0x00);
        }
         * */
        unsigned char *p;
        unsigned char idk[14];
        p = idk;
        //unsigned char this[6];
        I2C_read_multiple(wAdd,0x20,p,14);
        
        unsigned char temp[2] = {*(p+0),*(p+1)};
        unsigned char GyroX[2] = {*(idk+2),*(idk+3)};
        unsigned char GyroY[2] = {*(idk+4),*(idk+5)};
        unsigned char GyroZ[2] = {*(idk+6),*(idk+7)};
        unsigned char AccelX[2] = {*(idk+8),*(idk+9)};
        unsigned char AccelY[2] = {*(idk+10),*(idk+11)};
        unsigned char AccelZ[2] = {*(idk+12),*(idk+13)};
        int16_t my_int;
        int16_t Gyx;
        int16_t Gyy;
        int16_t Gyz;
        int16_t Axx;
        int16_t Axy;
        int16_t Axz;
        unsigned char *ip = (unsigned char *) &my_int;
        ip[0] = temp[0];
        ip[1] = temp[1];
        
        unsigned char *ip2 = (unsigned char *) &Gyx;
        ip2[0] = GyroX[0];
        ip2[1] = GyroX[1];
        
        unsigned char *ip3 = (unsigned char *) &Gyy;
        ip3[0] = GyroY[0];
        ip3[1] = GyroY[1];
        
        unsigned char *ip4 = (unsigned char *) &Gyz;
        ip4[0] = GyroZ[0];
        ip4[1] = GyroZ[1];
        
        unsigned char *ip5 = (unsigned char *) &Axx;
        ip5[0] = AccelX[0];
        ip5[1] = AccelX[1];
        
        unsigned char *ip6 = (unsigned char *) &Axy;
        ip6[0] = AccelY[0];
        ip6[1] = AccelY[1];
        
        unsigned char *ip7 = (unsigned char *) &Axz;
        ip7[0] = AccelZ[0];
        ip7[1] = AccelZ[1];
        
        
        char message[30]; 
        sprintf(message,"Temp %d",my_int);
        drawString(message,20,10);
        
        char message2[30]; 
        sprintf(message2,"Gyro X: %d",Gyx);
        drawString(message2,20,10+spacing*2);
        
        char message3[30]; 
        sprintf(message3,"Gyro Y: %d",Gyy);
        drawString(message3,20,10+spacing*3);
        
        char message4[30]; 
        sprintf(message4,"Gyro Z: %d",Gyz);
        drawString(message4,20,10+spacing*4);
        
        char message5[30]; 
        sprintf(message5,"Accel X: %d",Axx);
        drawString(message5,20,10+spacing*5);
        
        char message6[30]; 
        sprintf(message6,"Accel Y: %d",Axy);
        drawString(message6,20,10+spacing*6);
        
        char message7[30]; 
        sprintf(message7,"Accel Z: %d",Axz);
        drawString(message7,20,10+spacing*7);
        
        int div = 200;
        int xinc = Axx/div;
        int yinc = -1* (Axy/div);
        
        //inclo(-70,-70,150,150);
        inclo(xinc,yinc,150,150);
        
        _CP0_SET_COUNT(0);
        while(_CP0_GET_COUNT() < 48000000/30){
        }
       
       /*
        LCD_clearScreen(BLUE);  
        _CP0_SET_COUNT(0);
        while(_CP0_GET_COUNT() < 48000000/4){
        }
        LATAbits.LATA4 = 1;
        //setPin(wAdd,0x0A,0xFF);
        LCD_clearScreen(MAGENTA);
        _CP0_SET_COUNT(0);
        while(_CP0_GET_COUNT() < 48000000/4){
        }
        //setPin(wAdd,0x0A,0x00);
        LATAbits.LATA4 = 0;
          */

    }
    
    
    
}