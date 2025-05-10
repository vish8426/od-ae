// filename  ******************* sci1.h **************************
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
// adapted to the Dragon12 board using SCI1  --  fw-07-04
 
// define labels for baudrates
// (necessary 'coz 115200 isn't a 16-bit number anymore  --  fw-08-04)
#define BAUD_300      0
#define BAUD_600      1
#define BAUD_1200     2
#define BAUD_2400     3
#define BAUD_4800     4
#define BAUD_9600     5
#define BAUD_19200    6
#define BAUD_38400    7
#define BAUD_57600    8
#define BAUD_115200   9


// standard ASCII symbols 
#define CR   0x0D
#define LF   0x0A
#define BS   0x08
#define ESC  0x1B
#define SP   0x20       
#define DEL  0x7F
 
//-------------------------SCI1_Init------------------------
// Initialize Serial port SCI1
// Input: baudRate is tha baud rate in bits/sec
// Output: none
extern void SCI1_Init(unsigned short baudRate);
 
//-------------------------SCI1_InStatus--------------------------
// Checks if new input is ready, TRUE if new input is ready
// Input: none
// Output: TRUE if a call to InChar will return right away with data
//         FALSE if a call to InChar will wait for input
extern char SCI1_InStatus(void);  

//-------------------------SCI1_InChar------------------------
// Wait for new serial port input, busy-waiting synchronization
// Input: none
// Output: ASCII code for key typed
extern char SCI1_InChar(void);

extern void SCI1_InString(char *, unsigned short); // Reads in a String of max length

//----------------------SCI1_InUDec-------------------------------
// InUDec accepts ASCII input in unsigned decimal format
//     and converts to a 16 bit unsigned number
//     valid range is 0 to 65535
// Input: none
// Output: 16-bit unsigned number
// If you enter a number above 65535, it will truncate without an error
// Backspace will remove last digit typed
extern unsigned short SCI1_InUDec(void);   

//----------------------SCI1_InULDec-------------------------------
// InUDec accepts ASCII input in unsigned decimal format
//     and converts to a 32 bit unsigned number
//     valid range is 0 to 4294967296
// Input: none
// Output: 32-bit unsigned number
// If you enter a number above 4294967296, it will truncate without an error
// Backspace will remove last digit typed
extern unsigned long SCI1_InULDec(void);   

//----------------------SCI1_InSDec-------------------------------
// InSDec accepts ASCII input in signed decimal format
//     and converts to a 16 bit signed number
//     valid range is -32768 to +32767
// Input: none
// Output: 16-bit signed number
// If you enter a number outside +/-32767, it will truncate without an error
// Backspace will remove last digit typed
extern signed int SCI1_InSDec(void);

//----------------------SCI1_InSLDec-------------------------------
// InSLDec accepts ASCII input in signed decimal format
//     and converts to a 32 bit signed number
//     valid range is -2,147,483,648 to +2,147,483,647
// Input: none
// Output: 32-bit signed number
// If you enter a number outside +/-2147483648, it will truncate without an error
// Backspace will remove last digit typed
extern signed long SCI1_InSLDec(void);

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
extern unsigned short SCI1_InUHex(void);  

//-----------------------SCI1_OutStatus----------------------------
// Checks if output data buffer is empty, TRUE if empty
// Input: none
// Output: TRUE if a call to OutChar will output and return right away
//         FALSE if a call to OutChar will wait for output to be ready
extern char SCI1_OutStatus(void);   

//-------------------------SCI1_OutChar------------------------
// Wait for buffer to be empty, output 8-bit to serial port
// busy-waiting synchronization
// Input: 8-bit data to be transferred
// Output: none
extern void SCI1_OutChar(char);  
 
//-----------------------SCI1_OutUDec-----------------------
// Output a 16-bit number in unsigned decimal format
// Input: 16-bit number to be transferred
// Output: none
// Variable format 1-5 digits with no space before or after
extern void SCI1_OutUDec(unsigned short);    

//-------------------------SCI1_OutString------------------------
// Output String (NULL termination), busy-waiting synchronization
// Input: pointer to a NULL-terminated string to be transferred
// Output: none
extern void SCI1_OutString(char *pt); 
 
//--------------------------SCI1_OutUHex----------------------------
// Output a 16 bit number in unsigned hexadecimal format
// Input: 16-bit number to be transferred
// Output: none
// Variable format 1 to 4 digits with no space before or after
extern void SCI1_OutUHex(unsigned short); 

