/*----------------------------------------------------------------------------
 * CMSIS-RTOS 'main' function template
 *---------------------------------------------------------------------------*/

#define osObjectsPublic                     // define objects in main module
#include "osObjects.h"                      // RTOS object definitions
#include "stm32f4xx.h"                  // Device header
#include "stm32f4xx_conf.h"
#include <stdio.h>
#include "accelerometer.h"

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
		//printf("hello world\n");
		osDelay(250);
	}
}
void Accelerometer(void const *argument){
	float a;
	//osDelay(3000);
	while (1){
		get_angle(&a);
		//printf("angle: %f\n", a);
		osDelay(250);
	}
}

osThreadDef(Blinky, osPriorityNormal, 1, 0);
osThreadDef(Accelerometer, osPriorityNormal, 1, 0);
/*
 * main: initialize and start the system
 */
 int main (void) {
  osKernelInitialize ();                    // initialize CMSIS-RTOS
	
	// ID for thread
	osThreadId	Blinky_thread;
	osThreadId	Accer_thread;
  // initialize peripherals here
	accelerometer_setup(ACCELEROMETER_LIS3DSH);
	Blinky_GPIO_Init();
	
  // create 'thread' functions that start executing,
  // example: tid_name = osThreadCreate (osThread(name), NULL);
	Blinky_thread = osThreadCreate(osThread(Blinky), NULL);
	Accer_thread = osThreadCreate(osThread(Accelerometer), NULL);
	osKernelStart ();                         // start thread execution 
}


