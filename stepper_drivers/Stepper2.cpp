/*
 *  By: Patrick Finucane (www.WelcomeRobotOverlords.com)
 *
 *
 *
 *
 *
 *
 * This software is based on but not a copy of StepperControl by Eberhard Rensch 
 * <http://pleasantsoftware.com/developer/3d>
 */

#include "Stepper2.h"  // header file... google C++ libs
#include "Arduino.h"  // This may be different in different version of Arduino
 
Stepper2::Stepper2(int PinDir, int PinStep, int PinEnable, int inMircoSteps, 
 	 	  int EndStopPlus, int EndStopNeg, double Steps_Per_Unit)
 {
 	 pDir = PinDir;
 	 pStep = PinStep;
 	 pEnable = PinEnable;
 	 mSteps = inMircoSteps;
 	 
 	 nStopP = EndStopPlus;
 	 nStopN = EndStopNeg;
 	 
 	 // Set the Stepper pins
 	 pinMode(pDir, OUTPUT);
 	 pinMode(pStep, OUTPUT);
 	 pinMode(pEnable, OUTPUT);
 	 
 	 // Make sure everything is low
 	 digitalWrite(pDir, HIGH);
 	 digitalWrite(pStep, LOW);
	 digitalWrite(pEnable, HIGH); 	 
 	 
 	 // Option to not use end stops... living on the edge
 	 if(nStopP>=0)
 	   pinMode(nStopP, INPUT);
 	
 	 if(nStopN>=0)
 	   pinMode(nStopN, INPUT); 	 

   	// Steps per unit of input GCODE (GCODE doesn't have units)
 	 StepsPerUnit = Steps_Per_Unit;
 	 
 	 //Set the current location
 	 PositionInSteps = 0;
 	 PositionInUnits = 0;
 	 
 }
 
 /*  */
 void Stepper2::setEndPoint(double cordinate)
{
   EndPointInUnits = cordinate; 
   EndPointInStep = (long)(StepsPerUnit*EndPointInUnits);
   
   StepsToGo = EndPointInStep - PositionInSteps;
   direction = 1;
   digitalWrite(pDir, HIGH);
   
   if (StepsToGo < 0) {
	StepsToGo = -StepsToGo;
	direction = 0;
	digitalWrite(pDir, LOW);
   }
   
   MoveSteps = StepsToGo;
}
 
 void Stepper2::Step()
 {

 	 /* Bresenham's line algorithm */ 	    
 	 Error = Error - MoveSteps;
 	    
 	 if (Error < 0 || MaxSteps==MoveSteps )
 	 {	 
		 if( StepsToGo > 0 ) 
		 {
		 	 /* Step the motor */
			 digitalWrite(pStep, HIGH);
			 
			 /* track the loction */
			 StepsToGo--;
			 Error = Error + MaxSteps;
			 
			 if(direction==1){
			   PositionInSteps++;   // Update the location (+)
			 }else{
			   PositionInSteps--;   // Update the location (-)
			 }
			 digitalWrite(pStep, LOW);
		 }
		 
 	 }
 }
 
void Stepper2::enable(bool enabled)
{
	digitalWrite(pEnable, !enabled);
}
