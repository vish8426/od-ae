// Demonstration functions for IIC to read inertial sensor values
//
// The program will send read values to the serial port.
// you need to connect the serial port to a terminal to verify operation
// port speed 9600 bauds
//
// Eduardo Nebot  29 January 2016
// Sensors implemented:
// Gryro L2g2400D  (3 axis )
// Accelerometer  ADXL345
// Magnetometer HM5883
// Laser Lidarlight:  the driver is working with version 1 but not with version 2
// Version 2 work in progress: 
// Laser Interface will be done by measuring the pulse PWM pulse width
// Last version: 29/1/16
// 
//  This version installed interrupts with the simple model
//  The user should create a project selecting small memory model
//  and minimal startup code. This form will not intiliase any variables !
//  Your program will need to intialize the variables !
//
// Resources used: This program is using Timer 6 for the sampling time
// 
// the iic drivers are using Timer 7. ( You cannot use this timer in your program)
// Do not change the prescaler. If you do you need to change some code in iic.c
//
//    


#include <hidef.h>      /* common defines and macros */
#include "derivative.h"      /* derivative-specific definitions */
#include "iic.h"  
#include "pll.h"
#include "sci1.h"
#include "io.h"

#include "l3g4200.h"  // register's definitions    ( not used by ed )

 
volatile uint8_t alarmSignaled1 = 0; /* Flag set when alarm 1 signaled */

volatile uint16_t currentTime1 = 0; /* variables private to timeout routines */
uint16_t alarmTime1 = 0;
volatile uint8_t alarmSet1 = 0;

// void INTERRUPT timer6(void);

void setAlarm1(uint16_t msDelay1);
void delay1(uint16_t msDelay1);
void Init_TC6 (void);

#define laser_wr  0xc4
#define laser_rd  0xc5

#define gyro_wr 0xD2
#define gyro_rd 0xD3



#define accel_wr 0xA6    //
#define accel_rd 0xA7    // 
#define ADXL345_TO_READ 6
 
#define ADXL345_POWER_CTL 0x2D
#define ADXL345_DATAX0 0x32
#define ADXL345_DATA_FORMAT 0x31
 
#define ADXL345_OFSX 0x1E
#define ADXL345_OFSY 0x1F
#define ADXL345_OFSZ 0x20
 
#define ALPHA 0.5

#define magnet_wr  0x3C
#define magnet_rd  0x3D

#define HM5883_MODE_REG 0x02
#define HM5883_DATAX0 0x03

#define BUFF_SIZE	100

int k;



        
// Acclerometer Functions and Variables
int axraw[BUFF_SIZE];
int ayraw[BUFF_SIZE],azraw[BUFF_SIZE];

void adxl345_getrawdata(int *axraw, int *ayraw, int *azraw);
void accel_init(void);
void accel_test(void);
void accel_calc(void);
float pitch;


        
        /***********************************/

// Magnetometer Functions and Variables
void hm5883_getrawdata(int *mxraw, int *myraw, int *mzraw);
void magnet_init(void);
void magnet_test(void);

int mxraw[BUFF_SIZE];
int myraw[BUFF_SIZE],mzraw[BUFF_SIZE];

        /***********************************/

// Gyroscope Functions and Variables
void l3g4200d_getrawdata(int *gxraw, int *gyraw, int *gzraw);
void gyro_init(void);
void gyro_test(void);

char buff[BUFF_SIZE];
int gxraw[BUFF_SIZE];
int gyraw[BUFF_SIZE],gzraw[BUFF_SIZE];

        /***********************************/

// LIDAR Functions and Variables
const unsigned char segments[4] = {0xFE,0xFD,0xFB,0xF7};
const unsigned char display_numbers[10] = {             // Lookup table for numbers and their corresponding codes for the 7-seg display
  0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F
  //0,    1,     2,    3,    4,    5,    6,    7,    8,    9
};

void Lidar_Init(void);
void Init_TC1(void);
void Lidar_Display(float average);
void delay3(void);
int initCount;
int secondCount;
float values[10];
int i;
int x;
int j;
int m;
float total;
int numbers[4];
unsigned short diff;
float average;
char* buffer;
        
        /***********************************/

// SERVO Functions and Variables
void Servo_Init(void);
void Servo_Move_Pan(int duty_cycle);
void Servo_Move_Tilt(int cuty_cycle);


/*********************************************************
            END OF DECLARATIONS
**********************************************************/



void main(void) {
  /* put your own code here */
  
 char  myString[20];
 char character;
  
 int  res1, res2,  res3, *p;
 float acc;
 
 
 
 // The next 4 lines just to make sure all is working
 // are not needed for final program

 DDRB= 0xFF;   /* Port B output */
 DDRP = 0xFF;
 DDRJ= 0xFF;   // Port J to Output
 PTJ = 0x00;   // enable LEDs
 
 PLL_Init();  // make sure we are runnign at 24 Mhz
 
 
 EnableInterrupts;

 SCI1_Init(BAUD_9600);   // capped at 9600, if PLL inactive (4 MHz bus)
  
 SCI1_OutString("Program Starting ");      // should display this
 
 
 Init_TC6();
 Init_TC1();

 iicinit();
 
 gyro_init();     // l3g4200 setup
 accel_init();
 magnet_init();
 Servo_Init();
 
 
 //while(1) {
  

   delay1(50);
   
   
   for(i = 80; i >= 60; i--){
   

      PWMDTY5 = i;
      
      delay1(50);
      
      PWMDTY7 = 50;
      
      SCI1_OutString("Start\r\n");

      
      for(j = 50; j <= 100; j++){
      
          Servo_Move_Pan(j);
          
          Lidar_Init();
          Lidar_Display(average);
          
          delay1(20);
          
          if(j == 100){
            PWMDTY7 = 50;
            break;
          }
      }  
      SCI1_OutString("Stop\r\n");
      
      
              
      delay1(50);
   //}
   
 

 /*
// L3G4200d Gyro;
 
 l3g4200d_getrawdata( &gxraw, &gyraw, &gzraw) ;        // read data
 
 SCI1_OutString("Gyro Gx:");
 SCI1_OutUDec((unsigned short) gxraw[0]); 
 SCI1_OutString(" Gy:"); 
 SCI1_OutUDec((unsigned short) gyraw[0]) ;
 SCI1_OutString(" Gz:"); 
 SCI1_OutUDec((unsigned short) gzraw[0]) ;       
 
 SCI1_OutString("\r\n");
 
 
 // ADCL345 Accelerometer
 
  adxl345_getrawdata( &axraw, &ayraw, &azraw) ;        // read data
  SCI1_OutString("Accel Ax:");
  SCI1_OutUDec((unsigned short) axraw[0]); 
  SCI1_OutString(" Ay:"); 
  SCI1_OutUDec((unsigned short) ayraw[0]) ;
  SCI1_OutString(" Az:"); 
  SCI1_OutUDec((unsigned short) azraw[0]) ;       
     
  SCI1_OutString("\r\n");
 
 

 // HM5883_magnetometer
 
 hm5883_getrawdata(&mxraw, &myraw, &mzraw);

 SCI1_OutString("Magn Mx:"); 
 SCI1_OutUDec((unsigned short) mxraw[0]); 
 SCI1_OutString(" My:"); 
 SCI1_OutUDec((unsigned short) myraw[0]) ;
 SCI1_OutString(" Mz:"); 
 SCI1_OutUDec((unsigned short) mzraw[0]) ;       
 
 SCI1_OutString("\r\n");
 */
 
 }
 
 SCI1_OutString("End\r\n");
 

}



  
//   ******************  END Main   *****************

/********************************************************************
* MAGNETOMETER FUNCTIONS
*
*
********************************************************************/

void magnet_init(void){
  
  int  res1; 
  res1=iicstart(magnet_wr);
  res1=iictransmit(HM5883_MODE_REG );  // 
  res1=iictransmit(0x00 );
  iicstop(); 
 
}


void magnet_test(void){
  
}

void hm5883_getrawdata(int *mxraw, int *myraw, int *mzraw){
  
 uint8_t i = 0;
 uint8_t buff[6];
 int res1;
	
 res1=iicstart(magnet_wr);
 res1=iictransmit(HM5883_DATAX0 );
 res1= iicrestart(magnet_rd); 
 iicswrcv();
 
 for(i=0; i<4  ;i++) {
  buff[i]=iicreceive();
 }
 
 buff[i]= iicreceivem1();
 buff[i+1]= iicreceivelast();

	*mxraw = ((buff[0] << 8) | buff[1]);
	*myraw = ((buff[2] << 8) | buff[3]);
	*mzraw = ((buff[4] << 8) | buff[5]);
}  

/****************************************************************
             END OF MAGNETOMETER FUNCTIONS
*****************************************************************/

/********************************************************************
* ACCELEROMETER FUNCTIONS
*
*
********************************************************************/

void accel_test(void){}

void accel_init (void){
  
 int  res1;
 
 res1=iicstart(accel_wr);
 res1=iictransmit(ADXL345_POWER_CTL );  //  
 res1=iictransmit(0x08 );
  
 res1=iictransmit(ADXL345_DATA_FORMAT );  // ; 
 res1=iictransmit(0x08 );
  
 iicstop();  
}


void adxl345_getrawdata(int *axraw, int *ayraw, int *azraw){
  
 uint8_t i = 0;
 uint8_t buff[6];
 int res1;
	
 res1=iicstart(accel_wr);
 res1=iictransmit(ADXL345_DATAX0 );
 res1= iicrestart(accel_rd); 
 iicswrcv();
 
 for(i=0; i<4  ;i++) {
  buff[i]=iicreceive();
 }
 
 buff[i]= iicreceivem1();
 buff[i+1]= iicreceivelast();

	*axraw = ((buff[1] << 8) | buff[0]);
	*ayraw = ((buff[3] << 8) | buff[2]);
	*azraw = ((buff[5] << 8) | buff[4]);
	
}

 

/****************************************************************
             END OF ACCELEROMETER FUNCTIONS
*****************************************************************/ 
  
/********************************************************************
* GYROSCOPE FUNCTIONS
*
*
*********************************************************************/

// test the precense of Gyro , should get 211 on return 
void gyro_test(void) {
 int res1,who; 
 
 res1=iicstart(0xD2);
 res1=iictransmit(L3G4200D_WHO_AM_I);
  
 res1=iicrestart(0xD3);
 who=iicreceiveone();
 //who=who & 0x00ff;     Debugging  info
 //PORTB=  who ;

}


 //  Gyro Initialisation
 
 void gyro_init (void) {
  
 int  res1;
 
 res1=iicstart(gyro_wr);
 res1=iictransmit(L3G4200D_CTRL_REG1 );  // ; 100hz, 12.5Hz, Power up
 res1=iictransmit(0x0f );
 iicstop();  
 }
 
 
// Function to get a set of gyro data
// Eduardo Nebot,30 July 2015 

void l3g4200d_getrawdata(int *gxraw, int *gyraw, int *gzraw) {
 	uint8_t i = 0;
	uint8_t buff[6];
	int res1;
	
   res1=iicstart(gyro_wr);
   res1=iictransmit(L3G4200D_OUT_XYZ_CONT );
   res1= iicrestart(gyro_rd); 
 
 iicswrcv();
 
 for(i=0; i<4  ;i++) {
  buff[i]=iicreceive();
 }
 
buff[i]= iicreceivem1();
buff[i+1]= iicreceivelast();

	*gxraw = ((buff[1] << 8) | buff[0]);
	*gyraw = ((buff[3] << 8) | buff[2]);
	*gzraw = ((buff[5] << 8) | buff[4]);
}

void accel_calc(){

  float ax;
  float ay;
  float az;
  
  ax = (float)axraw[0] / 256;
  ay = (float)ayraw[0] / 256;
  az = (float)azraw[0] / 256;
  /*
  SCI1_OutString("ax = ");
  buffer = flt_to_str(ax);
  SCI1_OutString(buffer);
  SCI1_OutString("\r\n\r\n");
  
  SCI1_OutString("ay = ");
  buffer = flt_to_str(ay);
  SCI1_OutString(buffer);
  SCI1_OutString("\r\n\r\n");
  
  SCI1_OutString("az = ");
  buffer = flt_to_str(az);
  SCI1_OutString(buffer);
  SCI1_OutString("\r\n\r\n");
   */
  
 pitch = atan((az) / (sqrt((ax * ax) + (ay * ay))));
 pitch += 90;
  
  /*                  
  SCI1_OutString("Pitch = ");
  buffer = flt_to_str(pitch);
  SCI1_OutString(buffer);
  SCI1_OutString("\r\n\r\n");
    */
  
  
}

/****************************************************************
             END OF GYROSCOPE FUNCTIONS
*****************************************************************/


/******************************************************************** 
* SERVO FUNCTIONS
* 
* Port 7 Controls the horizontal movement of the PTU
* Port 5 Controls the vertical movement of the PTU
*
* Functions:
*
* Servo_Init() -> Initialises registers for movement and moves both servos to
*                 Neutral Position
*
* Servo_Move_Pan() -> Moves the horizontal motor based on a PWMDTY7 input
*
* Servo_Move_Tilt() -> Moves the veritcal motor based on a PWMDTY5 input
********************************************************************************/

void Servo_Init(){

 // PWM7 -> Sideways
 PWMPOL = 0x80; // Set High polarity
 PWMCAE = 0x00; // Left Aligned
 PWMCTL = 0x00; // 
 PWMPRCLK = 0x20;
 PWMSCLB = 62;
 PWMCLK = 0x80;
 PWMPER7 = 242;
 
 // PWM5 -> UP and DOWN
 
 PWMPOL |= 0x20; // Set High polarity
 PWMCAE = 0x00; // Left Aligned
 PWMCTL = 0x00; // 
 PWMPRCLK |= 0x02;
 PWMSCLA = 62;
 PWMCLK |= 0x20;
 PWMPER5 = 242;
 
 // Send servos to middle positions 
 PWMDTY7 = 75;
 PWMDTY5 = 70;
 PWME = 0x20;
 PWME |= 0x80;
 
 delay1(2000);
 
}


void Servo_Move_Pan(int duty_cycle){
  
 PWMDTY7 = duty_cycle; 
  
}

void Servo_Move_Tilt(int duty_cycle){
  
  PWMDTY5 = duty_cycle;
}

/****************************************************************
             END OF SERVO FUNCTIONS
*****************************************************************/


/******************************************************************** 
* LIDAR FUNCTIONS
* 
* LIDAR input taken as a PWM wave on pin PT1
* PT1 is directly interfaced to Timer 1 of the Timer Module
*
* PWM duty cycle is produced such that a duty cycle of 1ms = 1 metre
*
* Functions:
* 
* Lidar_Init() -> Calculates the distance as a floating point variable
*                 in centimeters
*                 Takes the average of 10 readings
*                  
* Lidar_Display() -> Takes a floating point input as an input
*                    Deciphers input into four parts
*                    Selects and displays respective numbers on 7 segment LEDs
*
* TC1_Init() -> Timer 1 Initalisation Routine
*               Set for input capture,then set to capture high going edges
*
* TC1_ISR() -> Switches between high going and low going edges
*
********************************************************************/

void Lidar_Init(){

  total = 0;

  for(m = 0; m < 10; m++){

     secondCount = 0;
     initCount = 0;
    
    // Set for input capture on high edge
    TCTL4 = 0b00000100; 

    // While we are waiting for a high going transition, do nothing
    while(TCTL4 == 0b00000100){}

    // While we are waiting for a falling edge, do nothing
    while(TCTL4 == 0b00001000){}

    TCTL4 = 0b00000000; // Turn off capture sensing so values dont change

    if(secondCount < initCount){

      diff = secondCount + (65536-initCount); // Account for timer overflow
    } else{

      diff = secondCount - initCount;
    }
    
    // Find time difference between init and second
    values[m] = (diff)*0.000041666666;
    
    total += values[m];
  }
  
  average = (total / 10) - 0.2; // 0.2 used as an average offset for sensing 0.8 to 1,2 metres
  
  buffer = flt_to_str(average);
  SCI1_OutString(buffer);
  SCI1_OutString("\r\n");
 
}

// Lidar Display

void Lidar_Display(float y){

  x = y * 100;
  x = (int)x;

  numbers[0] = (x - x%1000) / 1000;
  numbers[1] = (x - numbers[0]*1000 - x%100)/100;
  numbers[2] = (x - numbers[0]*1000 - numbers[1]*100 - x%10)/10;
  numbers[3] = x%10;
  
  for(m=0;m<30;m++){

    PTP = segments[0];
    PORTB = display_numbers[numbers[0]];
    delay3();

    PTP = segments[1];
    PORTB = display_numbers[numbers[1]];
    delay3();

    PTP = segments[2];
    PORTB = display_numbers[numbers[2]];
    delay3();

    PTP = segments[3];
    PORTB = display_numbers[numbers[3]];
    delay3();
  }
}

// Delay3

void delay3(void) {
// Simply loops through
   int c;
   int d;
   for (c = 1; c <= 100; c++)
       for (d = 1; d <= 100; d++) {}
}

interrupt 9 void TC1_ISR(){

   // Clear interrupt flag by writing one to C1F bit
   TFLG1 |= TFLG1_C1F_MASK;

   // If edge is high going, place timer counter on init count and set next detection for low going
   if(TCTL4 == 0b00000100){

      initCount = TC1;    // Save time
      TCTL4 = 0b00001000; // Set to capture low edge
      
   // Otherise if edge is low going, place timer counter on secondCount and set next detection for high going
   } else if (TCTL4 == 0b00001000){

      secondCount = TC1;
      TCTL4 = 0b00000100;
   
   }
}

void Init_TC1(){


 // Set Timer 1 for input capture by clearing TC1 bit in TIOS
 TIOS |= 0b00000000;

 // Turn off input capture -> Will be turned on in Lidar_Init()
 TCTL4 = 0b00000000;

 // Enale Timer 1 Interrupts
 TIE |= 0b00000010;

 // Enable Timer
 //TSCR1 |= 0b1000000;

}

/********************************************************************
                  END OF LIDAR FUNCTIONS
********************************************************************/

/********************************************************************
* DELAY FUNCTIONS
* 
*
*
*
*/

void setAlarm1(uint16_t msDelay1)
{
    alarmTime1 = currentTime1 + msDelay1;
    alarmSet1 = 1;
    alarmSignaled1 = 0;
}


void delay1(uint16_t msec)
{
    TC6 = TCNT + 24000; // Set initial time
    setAlarm1(msec);
    while(!alarmSignaled1) {};
}



/*  Interrupt   EMN */

// interrupt(((0x10000-Vtimch7)/2)-1) void TC7_ISR(void){
// the line above is to make it portable between differen
// Freescale processors
// The symbols for each interrupt ( in this case Vtimch7 )'
// are defined in the provided variable definition file
// I am usign a much simpler definition ( vector number) 
// that is easier to understand

interrupt 14 void TC6_ISR(void) {
   
  TC6 =TCNT + 24000;   // interrupt every msec
  TFLG1=TFLG1 | TFLG1_C6F_MASK;
  currentTime1++;
    if (alarmSet1 && currentTime1 == alarmTime1)
    {
        alarmSignaled1 = 1;
        alarmSet1 = 0;
    }
   //PORTB=PORTB+1;        // count   (debugging)
}



void Init_TC6 (void) {
  
_asm SEI;

TSCR1=0x80;
TSCR2=0x00;   // prescaler 1
TIOS=TIOS | TIOS_IOS6_MASK;
TCTL1=0x40;
TIE=TIE | 0x40;;

 _asm CLI;
 
}

/********************************************************************
                  END OF DELAY FUNCTIONS
********************************************************************/


// ***********************NEED TO PUT INTO MAIN********************************************
 /*for(i = 60; i <= 80; i++){
 
    Servo_Move_Tilt(i);
    
    delay1(50);
    
    Servo_Move_Pan(50);
    for(j = 50; j <= 100; j++){
    
        Servo_Move_Pan(j);
        Lidar_Init();
        delay1(50);
        
    }
    
    delay1(50);
    
 }
}

*/
