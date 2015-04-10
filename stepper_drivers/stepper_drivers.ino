/*
 *
 *
 *
 */
 
 // Include Libraries
#include "Stepper2.h"
#include "Arduino.h"
#include <TimerOne.h>
//#include <SoftwareServo.h>


// Set the time delay. Google Arduino Timer1

/*  Timer Setting
 *
 *  For 16MHz:
 *  Prescale Time_Per_Tick Max_Period
 *  1        0.0625 uS     8.192 mS
 *  8        0.5 uS        65.536 ms
 *  64       4 uS          524.288 ms
 *  256      16 uS         2097.152 ms
 *  1024     64 us         8388.608 ms
 *
 *  The interupt fires when the timer hits the max period
 */ 
#define TIMER_DELAY 64
#define TIMER_STEPS 65536
double seconds_per_interupt = .004096; // ((0.0625/1000000)*65536)

/* Set the pins (make sure these match your setup */
/* X Stepper */
#define X_Dir_Pin 2
#define X_Step_Pin 3
#define X_Enable_Pin 13
#define X_Micro_steps 16

/* Y Stepper */
#define Y_Dir_Pin 4
#define Y_Step_Pin 5
#define Y_Enable_Pin 13
#define Y_Micro_steps 16

/* Overall stepper values */
double speed = 25;		// default speed in steps per second
double Steps_Per_Unit = 10;

/* Serial Connection (USB) variables */
int baud = 115200;		// connection rate
const int MAX_CMD_SIZE = 256;
char buffer[MAX_CMD_SIZE];	// incoming serial data (array)
char inChar;			// single diget 
int index=0;			// serial data index (points to array position)
//
char serial_char; // value for each byte read in from serial comms
int serial_count = 0; // current length of command
char *strchr_pointer; // just a pointer to find chars in the cmd string like X, Y, Z, E, etc


/* Variables */
double xVal;
double yVal;
volatile boolean isRunning=false;
bool comment=0;
double MoveTime=0;
long TimerIncrements=0;
long pathSteps=0;
long  timerPeriod;
double AccRate=3;
long timerOffset=1;

/* debug varible */
int debug=0;

/* Create Instances of stepper class (aka stepper.h, stepper.cpp) */
 Stepper2 x(X_Dir_Pin,X_Step_Pin,X_Enable_Pin,X_Micro_steps,0,0,
	   Steps_Per_Unit);
 Stepper2 y(Y_Dir_Pin,Y_Step_Pin,Y_Enable_Pin,Y_Micro_steps,0,0,
 	   Steps_Per_Unit);

/* Setup function:
 * Start serial (USB) connection
 * Attach interrupt to timer */
void setup()
{
	
	Serial.begin(115200);
	Serial.print("Hello");
	
	/* Attach runInterrupt function to Timer1 */
	timerPeriod = TIMER_DELAY+timerOffset;
	//AccRate = 
	
	Timer1.initialize(timerPeriod); // Timer for updating pwm pins
	Timer1.attachInterrupt(runInterrupt);	
	
}

/* Main loop
 * Check serial connection over and over again */
void loop()  
{
  Read_Serial_Connection(); 
}

/*  */
void runInterrupt()
{
  if(isRunning)
  {
      //if( timerPeriod > TIMER_DELAY){
  	// timerPeriod = timerPeriod - AccRate;
  	// if(timerPeriod < TIMER_DELAY){
  	//   timerPeriod = TIMER_DELAY;
  	// }
  	// Timer1.setPeriod(timerPeriod);
      //}

           x.Step();
           y.Step();
           

         //}
         

	  if(x.StepsToGo==0 && y.StepsToGo==0)
	  {
		isRunning=0;
		timerPeriod = TIMER_DELAY+timerOffset;
		
	  }
  }
}

/* Read the usb serial connection */
void Read_Serial_Connection()
{
	{
	 /* All done with moving get next command */
	  if (!isRunning && Serial.available() > 0)
	  {
	    /* Read a character */
	    inChar = Serial.read();
	    
	    /* Detect end of command. Time to process command and clear 
	    the buffer and reset comment mode flag */
	    if (inChar == '\n' || inChar == '\r') 
	    { 
	    	    if(debug==1)
	    	    	    Serial.print(buffer);
	      
	      buffer[index]=0;
	      parse_GCODE(buffer, index);
	      index = 0;
	      comment = false; 
	    }
	    
	    /* still in middle of reading the command */
	    else 
	    {
	      /* check for start of command */
	      if (inChar == ';' || inChar == '(')
	      {
		comment = true;
	      }
	      
	      if (comment != true) // ignore if a comment has started
	      {
		buffer[index] = inChar; // add byte to buffer string
		index++;
		if (index > MAX_CMD_SIZE) // overflow, dump and restart
		{
		  index = 0;
		  Serial.flush();
		}
	      }
	    }
	  }
	}
}

/* Parse the Gcode and send to path planner */
void parse_GCODE(char command[], int command_length)
{
	  if (command_length>0 && command[0] == 'G') // G code
  {
    /* Return the number starting at the 2 digit (aka 1) and ending at a space*/
    int gcode = (int)strtod(&command[1], NULL);
    
    getValue('X', command, &xVal);
    getValue('Y', command, &yVal);
    
    if(debug==1){
    	    Serial.print("Xval: ");
	    Serial.print(xVal);
    	    Serial.print("\nYval: ");
	    Serial.print(yVal);
    	    Serial.print("\nXval: ");
	    Serial.print(xVal);
    	    Serial.print("\nX positive: ");
	    Serial.print(x.PositionInSteps);
	    Serial.print("\nY positive: ");
	    Serial.print(y.PositionInSteps);
	    Serial.print("\n");
    }
	    
    switch(gcode)
    {
      case 0: // G0, Rapid positioning
        x.setEndPoint(xVal);
        y.setEndPoint(yVal);
        pathPlan(speed);

        break;
        
      case 1: // G1, linear interpolation at specified speed
        x.setEndPoint(xVal);
        y.setEndPoint(yVal);
        pathPlan(speed);
        
        break;    	    
    	    
    }
  }
  /* done processing commands */
  if (Serial.available() <= 0) {
    Serial.print("\nok:");
    Serial.println(command);
  }
  
}

/* Parse the individual gcode commands */
boolean getValue(char key, char command[], double* value)
{  
	strchr_pointer = strchr(buffer, key);
	if (strchr_pointer != NULL) // We found a key value
	{
	  *value = (double)strtod(&command[strchr_pointer - command + 1], NULL);
	  return true;  
	}
	return false;
}

/* Cordinate the stepper and plan the path */
void pathPlan(double speedrate)
{
	/* find the longest axis move */
	if( x.StepsToGo > y.StepsToGo)
	{
		pathSteps = x.StepsToGo;
		x.MaxSteps = pathSteps;
		y.MaxSteps = pathSteps;

		
	}else{
		pathSteps = y.StepsToGo;
		x.MaxSteps = pathSteps;
		y.MaxSteps = pathSteps;

	}
	
	x.Error = pathSteps / 2;
	y.Error = pathSteps / 2;
	
	 if(debug==1){
	    Serial.print("\nError Y: ");
	    Serial.print(y.Error);
	 }
	    
	/* Calculate the time to the end of the move */
	MoveTime = pathSteps / speedrate;    // time in seconds
	TimerIncrements = MoveTime / seconds_per_interupt;
	
	/* Set stepper timer incliments between steps */
	x.TimerPeriod = TimerIncrements / x.StepsToGo;
	y.TimerPeriod = TimerIncrements / y.StepsToGo;	
	
	timerPeriod = x.TimerPeriod * TIMER_DELAY;
	
	Timer1.setPeriod(timerPeriod);
	
	 if(debug==1){
	    Serial.print("\nTimer Increments: ");
	    Serial.print(TimerIncrements);
	    Serial.print("\nTimerPeriod: ");
	    Serial.print(timerPeriod);
	    Serial.print("\n");
	    Serial.print("MoveTime: ");
	    Serial.print(MoveTime);
	    Serial.print("\n");
	    Serial.print("x.TimerPeriod: ");
	    Serial.print(x.TimerPeriod);
	    Serial.print("\n");
	    Serial.print("y.TimerPeriod: ");
	    Serial.print(y.TimerPeriod);
	    Serial.print("\n");
	    Serial.print("x.StepsToGo: ");
	    Serial.print(x.StepsToGo);
	    Serial.print("\n");
	    Serial.print("y.StepsToGo: ");
	    Serial.print(y.StepsToGo);
	    Serial.print("\n");
	 }
	 if(x.StepsToGo>0 && x.StepsToGo>0){
	 	 isRunning = 1;
	 	 x.enable(1);
	 }
	 
}

