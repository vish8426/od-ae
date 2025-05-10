// filename  ***************  sci1.c ****************************** 
// Simple I/O routines to 9S12C32 serial port   
// Jonathan W. Valvano 1/29/04

//  This example accompanies the books
//   "Embedded Microcomputer Systems: Real Time Interfacing",
//         Brooks-Cole, copyright (c) 2000,
//   "Introduction to Embedded Microcomputer Systems: 
//    Motorola 6811 and 6812 Simulation", Brooks-Cole, copyright (c) 2002

// Copyright 2004 by Jonathan W. Valvano, valvano@mail.utexas.edu 
//    You may use, edit, run or distribute this file 
//    as long as the above copyright notice remains 
// Modified by EE345L students Charlie Gough && Matt Hawk
// Modified by EE345M students Agustinus Darmawan + Mingjie Qiu
//
// adapted to the Dragon12 board using SCI1            --  fw-07-04
// allows for 24 MHz bus (PLL) and 4 MHz bus (no PLL)  -- fw-07-04
 
#include <mc9s12dp256.h>        /* derivative information */
#include "sci1.h"
#include "pll.h"                /* macro _SYSCLOCK */

#define RDRF 0x20   // Receive Data Register Full Bit
#define TDRE 0x80   // Transmit Data Register Empty Bit


//-------------------------SCI1_Init------------------------
// Initialize Serial port SCI1
// Input: baudRate is tha baud rate in bits/sec
// Output: none
void SCI1_Init(unsigned short baudRate) {
  
  SCI1BDH = 0;   // br=MCLK/(16*baudRate) 
  
  /* check if bus frequency has been boosted to 24 MHz (fw-07-04) */
  #if _BUSCLOCK == 24

  /* 24 MHz bus frequency (PLL is used, SYNR = 2, REFDV = 0 -> factor 6)
     Baud rate generator: SCI1BDL/H = (24e6/16)/baudrate = 1.5e6/baudrate */
  switch(baudRate){
	case BAUD_300:
	  SCI1BDH=19;
  	  SCI1BDL=136;
	  break;
	case BAUD_600:
	  SCI1BDH=9;
  	  SCI1BDL=196;
	  break;
	case BAUD_1200:
	  SCI1BDH=4;
  	  SCI1BDL=226;
	  break;
	case BAUD_2400:
	  SCI1BDH=2;
  	  SCI1BDL=113;
	  break;
	case BAUD_4800:
      SCI1BDH=1;
      SCI1BDL=56;
	  break;
	case BAUD_9600:
      SCI1BDH=0;
      SCI1BDL=156;
	  break;
	case BAUD_19200:
      SCI1BDH=0;
      SCI1BDL=78;
	  break;
	case BAUD_38400:
      SCI1BDH=0;
      SCI1BDL=39;
	  break;
	case BAUD_57600:
      SCI1BDH=0;
      SCI1BDL=26;
	  break;
	case BAUD_115200:
      SCI1BDH=0;
      SCI1BDL=13;
	  break;
  }
  
  #else
  
  /* 4 MHz bus frequency (PLL not used, SYNR = REFDV = 0 -> factor 2)
     Baud rate generator: SCI1BDL/H = (4e6/16)/baudrate = 250000/baudrate */
  switch(baudRate){
	case BAUD_300:
      SCI1BDH=3;
      SCI1BDL=64;
	  break;
	case BAUD_600:
      SCI1BDH=1;
      SCI1BDL=160;
	  break;
	case BAUD_1200:
      SCI1BDH=0;
      SCI1BDL=208;
	  break;
	case BAUD_2400:
      SCI1BDH=0;
      SCI1BDL=104;
	  break;
	case BAUD_4800:
      SCI1BDH=0;
      SCI1BDL=52;
	  break;
	case BAUD_9600:
      SCI1BDH=0;
      SCI1BDL=26;
	  break;
	case BAUD_19200:
      SCI1BDH=0;
      SCI1BDL=13;
	  break;
  }
  
  #endif /* _BUSCLOCK */

  SCI1CR1 = 0;
/* bit value meaning
    7   0    LOOPS, no looping, normal
    6   0    WOMS, normal high/low outputs
    5   0    RSRC, not appliable with LOOPS=0
    4   0    M, 1 start, 8 data, 1 stop
    3   0    WAKE, wake by idle (not applicable)
    2   0    ILT, short idle time (not applicable)
    1   0    PE, no parity
    0   0    PT, parity type (not applicable with PE=0) */ 
  SCI1CR2 = 0x0C; 
/* bit value meaning
    7   0    TIE, no transmit interrupts on TDRE
    6   0    TCIE, no transmit interrupts on TC
    5   0    RIE, no receive interrupts on RDRF
    4   0    ILIE, no interrupts on idle
    3   1    TE, enable transmitter
    2   1    RE, enable receiver
    1   0    RWU, no receiver wakeup
    0   0    SBK, no send break */ 

}
    
//-------------------------SCI1_InChar------------------------
// Wait for new serial port input, busy-waiting synchronization
// Input: none
// Output: ASCII code for key typed
char SCI1_InChar(void) {

  while((SCI1SR1 & RDRF) == 0){};
  return(SCI1DRL);

}
        
//-------------------------SCI1_OutChar------------------------
// Wait for buffer to be empty, output 8-bit to serial port
// busy-waiting synchronization
// Input: 8-bit data to be transferred
// Output: none
void SCI1_OutChar(char data) {
 
  while((SCI1SR1 & TDRE) == 0){};
  SCI1DRL = data;
  
}

   
//-------------------------SCI1_InStatus--------------------------
// Checks if new input is ready, TRUE if new input is ready
// Input: none
// Output: TRUE if a call to InChar will return right away with data
//         FALSE if a call to InChar will wait for input

char SCI1_InStatus(void) {

  return(SCI1SR1 & RDRF);
  
}

//-----------------------SCI1_OutStatus----------------------------
// Checks if output data buffer is empty, TRUE if empty
// Input: none
// Output: TRUE if a call to OutChar will output and return right away
//         FALSE if a call to OutChar will wait for output to be ready
char SCI1_OutStatus(void) {

  return(SCI1SR1 & TDRE);

}


//-------------------------SCI1_OutString------------------------
// Output String (NULL termination), busy-waiting synchronization
// Input: pointer to a NULL-terminated string to be transferred
// Output: none
void SCI1_OutString(char *pt) {

  while(*pt) {
  
    SCI1_OutChar(*pt);
    pt++;
    
  }
  
}

//----------------------SCI1_InUDec-------------------------------
// InUDec accepts ASCII input in unsigned decimal format
//     and converts to a 16 bit unsigned number
//     valid range is 0 to 65535
// Input: none
// Output: 16-bit unsigned number
// If you enter a number above 65535, it will truncate without an error
// Backspace will remove last digit typed
unsigned short SCI1_InUDec(void) {	

unsigned short number=0, length=0;
char character;

  character = SCI1_InChar();	
  
  while(character!=CR) {
  
    // accepts until carriage return input
    // The next line checks that the input is a digit, 0-9.
    // If the character is not 0-9, it is ignored and not echoed
    if((character>='0') && (character<='9')) {
      number = 10*number+(character-'0');   // this line overflows if above 65535
      length++;
      SCI1_OutChar(character);
    } 
    
// If the input is a backspace, then the return number is
// changed and a backspace is outputted to the screen
    else if((character==BS) && length) {
    
      number /= 10;
      length--;
      SCI1_OutChar(character);
      
    }
    
    character = SCI1_InChar();	
    
  }
  
  return number;

}



//----------------------SCI1_InULDec-------------------------------
// InULDec accepts ASCII input in unsigned decimal format
//     and converts to a 32 bit unsigned number
//     valid range is 0 to 4,294,967,296
// Input: none
// Output: 32-bit unsigned number
// If you enter a number above 4294967296, it will truncate without an error
// Backspace will remove last digit typed
unsigned long SCI1_InULDec(void) {	

unsigned long number=0, length=0;
char character;

  character = SCI1_InChar();	
  
  while(character!=CR) {
  
    // accepts until carriage return input
    // The next line checks that the input is a digit, 0-9.
    // If the character is not 0-9, it is ignored and not echoed
    if((character>='0') && (character<='9')) {
      number = 10*number+(character-'0');   // this line overflows if above 4294967296
      length++;
      SCI1_OutChar(character);
    } 
    
// If the input is a backspace, then the return number is
// changed and a backspace is outputted to the screen
    else if((character==BS) && length) {
    
      number /= 10;
      length--;
      SCI1_OutChar(character);
      
    }
    
    character = SCI1_InChar();	
    
  }
  
  return number;

}


//----------------------SCI1_InSDec-------------------------------
// InSDec accepts ASCII input in signed decimal format
//     and converts to a 16 bit signed number
//     valid range is -32768 to +32767
// Input: none
// Output: 16-bit signed number
// If you enter a number outside +/-32767, it will truncate without an error
// Backspace will remove last digit typed
signed int SCI1_InSDec(void) {	

signed int number=0, length=0;
char sign = 0;  // '0': pos, '1': neg
char character;

  character = SCI1_InChar();	
  
  while(character!=CR) {
  
    // accepts until carriage return input
    // The next lines checks for an optional sign character ('+' or '-')
    // and then that the input is a digit, 0-9.
    // If the character is not 0-9, it is ignored and not echoed
    if(character=='+') {
      SCI1_OutChar(character);
    } else if(character=='-') {
      sign = 1;
      SCI1_OutChar(character);
    } else if((character>='0') && (character<='9')) {
      number = 10*number+(character-'0');   // this line overflows if above 4294967296
      length++;
      SCI1_OutChar(character);
    } 
    
    // If the input is a backspace, then the return number is
    // changed and a backspace is outputted to the screen
    else if((character==BS) && length) {
    
      number /= 10;
      length--;
      SCI1_OutChar(character);
      
    }
    
    character = SCI1_InChar();	
    
  }
  
  if(sign == 1) return -number;
  else          return  number;

}


//----------------------SCI1_InSLDec-------------------------------
// InSLDec accepts ASCII input in signed decimal format
//     and converts to a 32 bit signed number
//     valid range is -2,147,483,648 to +2,147,483,647
// Input: none
// Output: 32-bit signed number
// If you enter a number outside +/-2147483648, it will truncate without an error
// Backspace will remove last digit typed
signed long SCI1_InSLDec(void) {	

signed long number=0, length=0;
char sign = 0;  // '0': pos, '1': neg
char character;

  character = SCI1_InChar();	
  
  while(character!=CR) {
  
    // accepts until carriage return input
    // The next lines checks for an optional sign character ('+' or '-')
    // and then that the input is a digit, 0-9.
    // If the character is not 0-9, it is ignored and not echoed
    if(character=='+') {
      SCI1_OutChar(character);
    } else if(character=='-') {
      sign = 1;
      SCI1_OutChar(character);
    } else if((character>='0') && (character<='9')) {
      number = 10*number+(character-'0');   // this line overflows if above 4294967296
      length++;
      SCI1_OutChar(character);
    } 
    
    // If the input is a backspace, then the return number is
    // changed and a backspace is outputted to the screen
    else if((character==BS) && length) {
    
      number /= 10;
      length--;
      SCI1_OutChar(character);
      
    }
    
    character = SCI1_InChar();	
    
  }
  
  if(sign == 1) return -number;
  else          return  number;

}



//-----------------------SCI1_OutUDec-----------------------
// Output a 16-bit number in unsigned decimal format
// Input: 16-bit number to be transferred
// Output: none
// Variable format 1-5 digits with no space before or after
void SCI1_OutUDec(unsigned short n){
// This function uses recursion to convert decimal number
//   of unspecified length as an ASCII string 
  if(n >= 10){
    SCI1_OutUDec(n/10);
    n = n%10;
  }
  SCI1_OutChar(n+'0'); /* n is between 0 and 9 */
}



//---------------------SCI1_InUHex----------------------------------------
// Accepts ASCII input in unsigned hexadecimal (base 16) format
// Input: none
// Output: 16-bit unsigned number
// No '$' or '0x' need be entered, just the 1 to 4 hex digits
// It will convert lower case a-f to uppercase A-F
//     and converts to a 16 bit unsigned number
//     value range is 0 to FFFF
// If you enter a number above FFFF, it will truncate without an error
// Backspace will remove last digit typed
unsigned short SCI1_InUHex(void){	
unsigned short number=0, digit, length=0;
char character;
  character = SCI1_InChar();
  while(character!=CR){	
    digit = 0x10; // assume bad
    if((character>='0') && (character<='9')){
      digit = character-'0';
    }
    else if((character>='A') && (character<='F')){ 
      digit = (character-'A')+0xA;
    }
    else if((character>='a') && (character<='f')){ 
      digit = (character-'a')+0xA;
    }
// If the character is not 0-9 or A-F, it is ignored and not echoed
    if(digit<=0xF ){	
      number = number*0x10+digit;
      length++;
      SCI1_OutChar(character);
    }
// Backspace outputted and return value changed if a backspace is inputted
    else if(character==BS && length){
      number /=0x10;
      length--;
      SCI1_OutChar(character);
    }
    character = SCI1_InChar();
  }
  return number;
}

//--------------------------SCI1_OutUHex----------------------------
// Output a 16 bit number in unsigned hexadecimal format
// Input: 16-bit number to be transferred
// Output: none
// Variable format 1 to 4 digits with no space before or after
void SCI1_OutUHex(unsigned short number){
// This function uses recursion to convert the number of 
//   unspecified length as an ASCII string
  if(number>=0x10)	{
    SCI1_OutUHex(number/0x10);
    SCI1_OutUHex(number%0x10);
  }
  else if(number<0xA){
    SCI1_OutChar(number+'0');
  }
  else{
    SCI1_OutChar((number-0x0A)+'A');
  }
}

//------------------------SCI1_InString------------------------
// This function accepts ASCII characters from the serial port
//    and adds them to a string until a carriage return is inputted 
//    or until max length of the string is reached.  
// It echoes each character as it is inputted.  
// If a backspace is inputted, the string is modified 
//    and the backspace is echoed
// InString terminates the string with a null character
// -- Modified by Agustinus Darmawan + Mingjie Qiu --
void SCI1_InString(char *string, unsigned short max) {	
int length=0;
char character;
  character = SCI1_InChar();
  while(character!=CR){
    if(character==BS){
      if(length){
        string--;
        length--;
        SCI1_OutChar(BS);
      }
    }
    else if(length<max){
      *string++=character;
      length++; 
      SCI1_OutChar(character);
    }
    character = SCI1_InChar();
  }
  *string = 0;
}



