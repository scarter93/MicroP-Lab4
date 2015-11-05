/*----------------------------------------------------------------------------
 * CMSIS-RTOS 'main' function template
 *---------------------------------------------------------------------------*/

#define osObjectsPublic                     // define objects in main module
#include "osObjects.h"                      // RTOS object definitions
#include "stm32f4xx.h"                  		// Device header
#include "stm32f4xx_conf.h"
#include <stdio.h>

#include "accelerometer.h"

#include "keypad.h"
#include "temperature.h"
#include "seven_segment_display.h"


//define thread pointers
osThreadId temperature_thread;
osThreadId accelerometer_thread;
osThreadId display_thread;
osThreadId keypad_thread;
//define variable for display switching
TO_DISPLAY to_display;
//define threads for use with priority
osThreadDef(Temperature, osPriorityNormal, 1, 0);
osThreadDef(Accelerometer, osPriorityNormal, 1, 0);
osThreadDef(Display, osPriorityNormal, 1, 0);
osThreadDef(Keypad, osPriorityNormal, 1, 0);

/*
 * main: initialize and start the system
 */
 int main (void) {
  osKernelInitialize ();                    // initialize CMSIS-RTOS

  // initialize peripherals
	accelerometer_setup(ACCELEROMETER_LIS3DSH);
	keypad_setup();
	temperature_setup();
	seven_segment_setup();
	
	//create the necessary threads
	temperature_thread = osThreadCreate(osThread(Temperature), NULL);
	display_thread = osThreadCreate(osThread(Display), NULL);
	accelerometer_thread = osThreadCreate(osThread(Accelerometer), NULL);
	keypad_thread = osThreadCreate(osThread(Keypad), NULL);

	osKernelStart ();                         // start thread execution 
}


