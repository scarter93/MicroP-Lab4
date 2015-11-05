/*----------------------------------------------------------------------------
 * CMSIS-RTOS 'main' function template
 *---------------------------------------------------------------------------*/

#define osObjectsPublic                     // define objects in main module
#include "osObjects.h"                      // RTOS object definitions
#include "stm32f4xx.h"                  // Device header
#include "stm32f4xx_conf.h"
#include <stdio.h>

#include "accelerometer.h"
#include "temperature.h"
#include "seven_segment_display.h"

osThreadId Blinky_thread;
osThreadId temperature_thread;
osThreadId accelerometer_thread;
osThreadId display_thread;

void Blinky_GPIO_Init(void){
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13| GPIO_Pin_14| GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOD, &GPIO_InitStructure);
	
}

void Blinky(void const *argument){
	while(1){
		GPIO_ToggleBits(GPIOD, GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);
		osDelay(250);
	}
}

void Temperature(void const *argument) {
	float temp;
	while(1) {
		temp = get_temperature();
		printf("temp: %f\n", temp);
		//display(temp);
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
		display(angle);
		osDelay(250);
	}
}

osThreadDef(Blinky, osPriorityNormal, 1, 0);
osThreadDef(Temperature, osPriorityNormal, 1, 0);
osThreadDef(Accelerometer, osPriorityNormal, 1, 0);
osThreadDef(Display, osPriorityNormal, 1, 0);

/*
 * main: initialize and start the system
 */
 int main (void) {
  osKernelInitialize ();                    // initialize CMSIS-RTOS
	
	// ID for thread
  // initialize peripherals here
	accelerometer_setup(ACCELEROMETER_LIS3DSH);
	Blinky_GPIO_Init();
	temperature_setup();
	seven_segment_setup();
	
  // create 'thread' functions that start executing,
  // example: tid_name = osThreadCreate (osThread(name), NULL);
	Blinky_thread = osThreadCreate(osThread(Blinky), NULL);
	temperature_thread = osThreadCreate(osThread(Temperature), NULL);
	accelerometer_thread = osThreadCreate(osThread(Accelerometer), NULL);
	display_thread = osThreadCreate(osThread(Display), NULL);
	osKernelStart ();                         // start thread execution 
}


