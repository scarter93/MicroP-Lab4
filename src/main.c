/*----------------------------------------------------------------------------
 * CMSIS-RTOS 'main' function template
 *---------------------------------------------------------------------------*/

#define osObjectsPublic                     // define objects in main module
#include "osObjects.h"                      // RTOS object definitions
#include "stm32f4xx.h"                  // Device header
#include "stm32f4xx_conf.h"
#include <stdio.h>

#include "accelerometer.h"

#include "keypad.h"
#include "temperature.h"
#include "seven_segment_display.h"

#define OVERHEATING 37f

osThreadId temperature_thread;
osThreadId accelerometer_thread;
osThreadId display_thread;
osThreadId keypad_thread;

TO_DISPLAY to_display;

void Temperature(void const *argument) {
	float temp;
	while(1) {
		temp = get_temperature();
		printf("temp: %f\n", temp);
		if (temp > OVERHEATING) {
			flash_display(1);
		} else {
			flash_display(0);
		}
		if (to_display == TEMP) {
			display(temp);
		}
		//printf("temp: %f\n", temp);
		osDelay(250);
	}
}

void Accelerometer(void const *argument){
	//osDelay(3000);
	float angle;
	while (1){
		// get offset values
		osSignalWait(SIGNAL_ACCELEROMETER, osWaitForever);
		update();
		get_angle(&angle);
		printf("angle: %f\n", angle);
		if (to_display == ACCEL) {
			display(angle);
		}
		osDelay(250);
	}
}

osThreadDef(Temperature, osPriorityNormal, 1, 0);
osThreadDef(Accelerometer, osPriorityNormal, 1, 0);
osThreadDef(Display, osPriorityNormal, 1, 0);
osThreadDef(Keypad, osPriorityNormal, 1, 0);

/*
 * main: initialize and start the system
 */
 int main (void) {
  osKernelInitialize ();                    // initialize CMSIS-RTOS
	
	// ID for thread
  // initialize peripherals here
	accelerometer_setup(ACCELEROMETER_LIS3DSH);
	keypad_setup();
	temperature_setup();
	seven_segment_setup();
	
  // create 'thread' functions that start executing,
  // example: tid_name = osThreadCreate (osThread(name), NULL);
	 
	temperature_thread = osThreadCreate(osThread(Temperature), NULL);
	display_thread = osThreadCreate(osThread(Display), NULL);
	accelerometer_thread = osThreadCreate(osThread(Accelerometer), NULL);
	keypad_thread = osThreadCreate(osThread(Keypad), NULL);

	osKernelStart ();                         // start thread execution 
}


