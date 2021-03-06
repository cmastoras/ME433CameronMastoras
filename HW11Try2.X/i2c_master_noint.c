
// I2C Master utilities, using polling rather than interrupts
// The functions must be called in the correct order as per the I2C protocol
// I2C pins need pull-up resistors, 2k-10k
#include "i2c_master_noint.h"
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

//unsigned char wAdd = 0b01000000;
unsigned char wAdd = 0b11010100;
unsigned char rAdd = 0b11010101;

void readUART1(char * string, int maxLength);
void writeUART1(const char * string);

void readUART1(char * message, int maxLength) {
  char data = 0;
  int complete = 0, num_bytes = 0;
  // loop until you get a '\r' or '\n'
  while (!complete) {
    if (U1STAbits.URXDA) { // if data is available
      data = U1RXREG;      // read the data
      if ((data == '\n') || (data == '\r')) {
        complete = 1;
      } else {
        message[num_bytes] = data;
        ++num_bytes;
        // roll over if the array is too small
        if (num_bytes >= maxLength) {
          num_bytes = 0;
        }
      }
    }
  }
  // end the string
  message[num_bytes] = '\0';
}

// Write a character array  using UART3
void writeUART1(const char * string) {
  while (*string != '\0') {
    while (U1STAbits.UTXBF) {
      ; // wait until tx buffer isn't full
    }
    U1TXREG = *string;
    ++string;
  }
}

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
    I2C1BRG = 1000; // I2CBRG = [1/(2*Fsck) - TPGD]*Pblck - 2 (TPGD is the Pulse Gobbler Delay)
    I2C1CONbits.ON = 1; // turn on the I2C1 module
}

void i2c_master_start(void) {
    //I2C1CONbits.PEN = 0;
    I2C1CONbits.SEN = 1; // send the start bit
    while (I2C1CONbits.SEN) {
        
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
        while(1){} // get stuck here if the chip does not ACK back
    }
} // receive a byte from the slave
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
    
    U1RXRbits.U1RXR = 0b0001; // U1RX is B6
    RPB7Rbits.RPB7R = 0b0001; // U1 TX is B7
    
    U1MODEbits.BRGH = 0;
    U1BRG = ((48000000 / 115200) / 16)-1;
    
    U1MODEbits.PDSEL = 0;
    U1MODEbits.STSEL = 0;
    
    U1STAbits.UTXEN = 1;
    U1STAbits.URXEN = 1;
    
    U1MODEbits.ON = 1;
    
    //JOSE RELEVANT STUFF
    i2c_master_setup();
    //Set bank to 1
    //0x0A is IOCON.bank in bank = 0 default mode
    /*
    setPin(wAdd,0x0A,0b1000000);
    
    //Set A to be output
    setPin(wAdd,0x00,0x00);
    //Set B to be input
    setPin(wAdd,0x10,0xFF);
     */
    int timing = 2;
    __builtin_enable_interrupts();
    LATAbits.LATA4 = 1;
    unsigned char thang;
    while(1){
        /*
        thang = readPin(rAdd,0x19);
        
        if (thang < 1){
            setPin(wAdd,0x0A,0xFF);
        }else{
            setPin(wAdd,0x0A,0x00);
        }
        */
        unsigned char thang;
        //LCD_clearScreen(MAGENTA);
            //i2c_master_stop();
        
        thang = readPin(wAdd,0x0F);
        LATAbits.LATA4 = 0;
        if(thang != 0b01101001){
            timing = 16;
        }
        
        _CP0_SET_COUNT(0);
        while(_CP0_GET_COUNT() < 48000000/timing){
        }
        LATAbits.LATA4 = 1;
        //setPin(wAdd,0x0A,0xFF);
        _CP0_SET_COUNT(0);
        while(_CP0_GET_COUNT() < 48000000/timing){
        }
        //setPin(wAdd,0x0A,0x00);
        LATAbits.LATA4 = 0;
          

    }
    
    
    
}