#include "accelerometer.h"
#include "stm32f4xx.h"                  // Device header
#include "stm32f4xx_conf.h"
#include "lis3dsh.h"
#include "osObjects.h" 
#include <stdio.h>
#include <math.h>
#include "moving_average.h"
#include "seven_segment_display.h"

FilterBuffer angle_avg;

/*!
	Makes system calls for setting up accelerometer
	
	@return 0 on success, else negative
 */
int accelerometer_setup(ACCELEROMETER which) {
	// built in accelerometer
	if (which == ACCELEROMETER_LIS3DSH)
	{
		// setup accelerometer
		LIS3DSH_InitTypeDef lis3dsh_init_s;
		lis3dsh_init_s.Power_Mode_Output_DataRate = LIS3DSH_DATARATE_100;
		lis3dsh_init_s.Axes_Enable = LIS3DSH_X_ENABLE | LIS3DSH_Y_ENABLE | LIS3DSH_Z_ENABLE;
		lis3dsh_init_s.Continous_Update = LIS3DSH_ContinousUpdate_Disabled;
		lis3dsh_init_s.AA_Filter_BW = LIS3DSH_AA_BW_50;
		lis3dsh_init_s.Full_Scale = LIS3DSH_FULLSCALE_2;

		LIS3DSH_Init( &lis3dsh_init_s );
		
		// setup interupts
		LIS3DSH_DRYInterruptConfigTypeDef lis3dsh_dry_interrupt_s;
		lis3dsh_dry_interrupt_s.Dataready_Interrupt = LIS3DSH_DATA_READY_INTERRUPT_ENABLED;
		lis3dsh_dry_interrupt_s.Interrupt_signal = LIS3DSH_ACTIVE_HIGH_INTERRUPT_SIGNAL;
		lis3dsh_dry_interrupt_s.Interrupt_type = LIS3DSH_INTERRUPT_REQUEST_PULSED;
		
		LIS3DSH_DataReadyInterruptConfig(&lis3dsh_dry_interrupt_s);
		
		GPIO_InitTypeDef gpio_init_s;
		EXTI_InitTypeDef exti_init_s;
		NVIC_InitTypeDef nvic_init_s;
		
		//enable GPIOE
		RCC_AHB1PeriphClockCmd(LIS3DSH_SPI_INT1_GPIO_CLK, ENABLE);
		//enable clock for SYSCFG
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
		
    gpio_init_s.GPIO_Mode = GPIO_Mode_IN;
		gpio_init_s.GPIO_Pin = LIS3DSH_SPI_INT1_PIN;
    gpio_init_s.GPIO_PuPd = GPIO_PuPd_DOWN; // Force low from start
    gpio_init_s.GPIO_Speed = GPIO_Speed_100MHz;
		
    GPIO_Init(LIS3DSH_SPI_INT1_GPIO_PORT, &gpio_init_s);
		
		SYSCFG_EXTILineConfig(LIS3DSH_SPI_INT1_EXTI_PORT_SOURCE, LIS3DSH_SPI_INT1_EXTI_PIN_SOURCE); 

		// setup external interupts
		exti_init_s.EXTI_Line = LIS3DSH_SPI_INT1_EXTI_LINE;
		exti_init_s.EXTI_Mode = EXTI_Mode_Interrupt;
		exti_init_s.EXTI_Trigger = EXTI_Trigger_Rising;
		exti_init_s.EXTI_LineCmd = ENABLE;
		
		EXTI_Init(&exti_init_s);
		
		// setup NVIC for accelerometer
		nvic_init_s.NVIC_IRQChannel = LIS3DSH_SPI_INT1_EXTI_IRQn; //

    nvic_init_s.NVIC_IRQChannelPreemptionPriority = 0x02;

		nvic_init_s.NVIC_IRQChannelSubPriority = 0x00;
		nvic_init_s.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&nvic_init_s);
		
	}

	return 0;
}

float xyz[3]; //! contains x, y, and z accelerations

#define OFFSET_X 0
#define OFFSET_Y -13.5f
#define OFFSET_Z -26.0f
void EXTI0_IRQHandler(void)
{
	if (EXTI_GetITStatus(LIS3DSH_SPI_INT1_EXTI_LINE) != RESET){
		LIS3DSH_ReadACC(xyz);
		osSignalSet(accelerometer_thread, SIGNAL_ACCELEROMETER);
		//printf("%f\n", get_value(&angle_avg));
		EXTI_ClearITPendingBit(LIS3DSH_SPI_INT1_EXTI_LINE);
	}
}
/*!
	Updates the angle calculated from the accelerometer
 */
void update() {
	// get offset values
	float x = xyz[0] + OFFSET_X;
	float y = xyz[1] + OFFSET_Y;
	float z = xyz[2] + OFFSET_Z;
	float pitch = atan2f(x, sqrtf(y*y + z*z)) * 180.0f / 3.14f + 89;
	add_value(&angle_avg, pitch);
}

/*!
	Gets the actual angle
	@param *angle the angle measured by the accelerometer
	@return 0 on success, else negative
 */
int get_angle(float* angle) {
	*angle = get_value(&angle_avg);
	return 0;
}

/*!
	Acceleromter Thread
	Polls accelerometer when data is available
 */
void Accelerometer(void const *argument){
	//osDelay(3000);
	float angle;
	while (1){
		//wait untill ready
		osSignalWait(SIGNAL_ACCELEROMETER, osWaitForever);
		update();
		get_angle(&angle);
		printf("angle: %f\n", angle);
		//display if designated
		if (to_display == ACCEL) {
			display(angle);
		}
		osDelay(75);
	}
}

