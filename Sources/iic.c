/*
* IIC Host Driver for Dragon12-plus
* Author: Tom Almy
* Date: January 29, 2008

*  Eduardo Nebot July 2015
   Original code written for debug-12
   this code adapted for using with code warrior
   and small memory model ( interrupt )   
   Simple method
   **********************
   
   Resources Used: Timer 7
   Prescaler is 1; if changed we need to change
   the counter in:
   interrupt 15 void TC7_ISR(void)
   to 1 msec delay. Current value is 24,000
   
*/


#include <hidef.h>      /* common defines and macros */
#include "derivative.h"      /* derivative-specific definitions */



typedef unsigned char uint8_t;
typedef unsigned int uint16_t;

//#define TRUE (1)
//#define FALSE (0)

// *** IIC interface ***
void iicinit(void);
int iicstart(uint8_t control);        // return non-zero if fails
int iicrestart(uint8_t control);      // return non-zero if fails
int iictransmit(uint8_t control);     // return non-zero if fails
void iicstop(void);
void iicswrcv(void);
int iicreceiveone(void);     // return -1 if fails, 0->255 is valid received value
int iicreceive(void);        // return -1 if fails, 0->255 is valid received value    
int iicreceivem1(void);      // return -1 if fails, 0->255 is valid received value
int iicreceivelast(void);    // return -1 if fails, 0->255 is valid received value

uint8_t iic_error_code;
enum IIC_ERRORS {NO_ERROR, NO_RESPONSE, NAK_RESPONSE, IIB_CLEAR_TIMEOUT, IIB_SET_TIMEOUT,
    RECEIVE_TIMEOUT};

// *** timeout control ***

volatile uint8_t alarmSignaled = 0; /* Flag set when alarm 1 signaled */

volatile uint16_t currentTime = 0; /* variables private to timeout routines */
uint16_t alarmTime = 0;
volatile uint8_t alarmSet = 0;

// void INTERRUPT timer6(void);

void setAlarm(uint16_t msDelay);
void delay(uint16_t msDelay);








// *** I2C Functions ***


void iicinit(void) {
    IBFD = 0x23; // 100KHz operation
  //IBFD = 0x35; // 20KHz operation
    IBAD = 0x2;    // Slave address is 1 (never address ourselves, please!)
    IBCR |= 0x80; // Set IBEN
}

int iicresponse(void) { // wait to see what our response is
    setAlarm(1000);
    while ((IBSR & 0x2) == 0 && !alarmSignaled) {}; // Wait for IBIF to set
    if (alarmSignaled) {
        iic_error_code = NO_RESPONSE; 
        iicstop();
        return 1;
    }
    IBSR &= 0x2;        // Clear IBIF
    if (IBSR&1) { // NAK -- stop!
        iic_error_code = NAK_RESPONSE;
        iicstop();
        return 1;
    }
    return 0;
}    

/* Returns 1 on failure and sets iic_error_code, 0 on success */
int iicstart(uint8_t control) {
    setAlarm(1000);
    while ((IBSR & 0x20)!= 0 && !alarmSignaled) {}; // Wait for IBB flag to clear
    if (alarmSignaled) {
        iic_error_code = IIB_CLEAR_TIMEOUT;
        return 1;
    }
    IBCR |= 0x30;
    IBDR = control;
    setAlarm(1000);
    while ((IBSR & 0x20)==0 && !alarmSignaled) {}; // Wait for IBB flag to set
    if (alarmSignaled) {
        iic_error_code = IIB_SET_TIMEOUT;
        return 2;
    }
    return iicresponse();
}
    
void iicstop(void) {
    IBCR &= ~0x20;
}

int iicrestart(uint8_t control) {
    IBCR |= 0x04;
    IBDR = control;
    return iicresponse();
}

int iictransmit(uint8_t val) {
    IBDR = val;
    return iicresponse();
}


void iicswrcv(void) {
    IBCR &= ~0x10; // put in receive mode
    IBDR;    // dummy read (better not be optimized out!)
}

/* Returns -1 and error code set on failure */
int iicreceive(void) {
    setAlarm(1000);
    while ((IBSR & 0x2) == 0 && !alarmSignaled) {};
    if (alarmSignaled) {
        iic_error_code = RECEIVE_TIMEOUT;
        return -1;
    }
    IBSR &= 0x2;
    return IBDR;
}

/* Returns -1 and error code set on failure */
int iicreceivem1(void) {
    setAlarm(1000);
    while ((IBSR & 0x2) == 0 && !alarmSignaled) {};
    if (alarmSignaled) {
        iic_error_code = RECEIVE_TIMEOUT;
        return -1;
    }
    IBSR &= 0x2;
    IBCR |= 0x8; // disable ACK for last read
    return IBDR;
}

/* Returns -1 and error code set on failure */
int iicreceivelast(void) {
    setAlarm(1000);
    while ((IBSR & 0x2) == 0 && !alarmSignaled) {};
    if (alarmSignaled) {
        iic_error_code = RECEIVE_TIMEOUT;
        return -1;
    }
    IBSR &= 0x2;
    IBCR &= ~0x8;    // reenable ACK
    IBCR &= ~0x20;    // generate STOP
    IBCR |= 0x10;    // set transmit
    return IBDR;
}

/* Returns -1 and error code set on failure */
int iicreceiveone(void) {
    IBCR |= 0x8;    // Disable ACK
    iicswrcv();
    return iicreceivelast();
}


// *** Alarms ***

void setAlarm(uint16_t msDelay)
{
    alarmTime = currentTime + msDelay;
    alarmSet = 1;
    alarmSignaled = 0;
}


void delay(uint16_t msec)
{
    TC7 = TCNT + 24000; // Set initial time
    setAlarm(msec);
    while(!alarmSignaled) {};
}



/*  Interrupt   EMN */

// interrupt(((0x10000-Vtimch7)/2)-1) void TC7_ISR(void){
// the line above is to make it portable between differen
// Freescale processors
// The symbols for each interrupt ( in this case Vtimch7 )'
// are defined in the provided variable definition file
// I am usign a much simpler definition ( vector number) 
// that is easier to understand

interrupt 15 void TC7_ISR(void) {
   
  TC7 =TCNT + 24000;   // interrupt every msec assuming prescaler =1
  TFLG1=TFLG1 | TFLG1_C7F_MASK;
  currentTime++;
    if (alarmSet && currentTime == alarmTime)
    {
        alarmSignaled = 1;
        alarmSet = 0;
    }
  // PORTB=PORTB+1;        // count   (debugging)
}



void Init_TC7 (void) {
  
_asm SEI;

TSCR1=0x80;
TSCR2=0x00;   // prescaler 1, before 32 = 0x04
TIOS=TIOS | TIOS_IOS7_MASK;
TCTL1=0x40;
TIE=TIE | 0x80;

 _asm CLI;
 
}

/*

// Millisecond timer
void INTERRUPT timer6(void)
{
    TFLG1 &= 0x40; // reset interrupt flag
    TC6 += 24000;  // interrupt every 1ms
    currentTime++;
    if (alarmSet && currentTime == alarmTime)
    {
        alarmSignaled = 1;
        alarmSet = 0;
    }
}

   */
