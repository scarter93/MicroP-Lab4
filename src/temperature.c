#include "temperature.h"

#include <stdio.h>
#include "stm32f4xx.h"                  					// Device header
#include "stm32f4xx_conf.h"
#include "stm32f4xx.h"
#include <stdint.h>

#include "moving_average.h"

const float to_mV = 1000.0f;
const float mV_25 = 760.0f;
const float slope = 2.5f;
const float max = 4095.0f;

int voltage_to_celcius(uint16_t voltage, float* ouput);

FilterBuffer temperature_filter;

int temperature_setup() 
{
	ADC_InitTypeDef adc_init_s;														//Initializes definitions of SDC
	ADC_CommonInitTypeDef adc_common_init_s;							//Used to def common paramters for ADC peripheral
	
	ADC_DeInit();																					//reset ADC peripheral value to their defaults
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);	//Enable Power for APB2 bus connected to ADC1 (temp sensor)

	adc_common_init_s.ADC_Mode = ADC_Mode_Independent; 		//Configures the ADC for indepedent mode
	adc_common_init_s.ADC_Prescaler = ADC_Prescaler_Div2; //Sets Frequency of the clock to ADC. Count 0 and 1.
	adc_common_init_s.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled; //turn off direct memory access mode
	adc_common_init_s.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles; //sets the delay between two sampling phases as 5
	
	ADC_CommonInit(&adc_common_init_s);										//initializes the struct for values set (above)
	
	adc_init_s.ADC_Resolution = ADC_Resolution_12b;				//Configures the ADC resolution 12 bits
	adc_init_s.ADC_ScanConvMode = DISABLE;								//sets conversion to Single channel mode
	adc_init_s.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None; //Set the external trigger edge to none
	adc_init_s.ADC_DataAlign = ADC_DataAlign_Right;				//MSB in register is 0, data saved starting from LSB
	adc_init_s.ADC_NbrOfConversion = 1; 									//Specifies the number of ADC conversions that will be done
	
	ADC_TempSensorVrefintCmd(ENABLE);											//wake up desired sensor (temp)
	ADC_Cmd(ADC1, ENABLE);																//turn on the ADC1 peripheral
	ADC_RegularChannelConfig(ADC1, ADC_Channel_16, 1, ADC_SampleTime_480Cycles);	//Configures ADC1_channel 16 which is the tempature sensor
	ADC_Init(ADC1, &adc_init_s);													//initialize ADC1
	

	temperature_filter.avg = 0, temperature_filter.replace = 0, temperature_filter.sum = 0;
	
	return 0;
}

float get_temperature()
{
	
	
	ADC_SoftwareStartConv(ADC1);													//start conversion of temp
	while (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);	//Wait for converion to finish

	uint16_t temp_mV = ADC_GetConversionValue(ADC1);	//Returns the last ADC1 converted value
	float out_temp;
	int c = voltage_to_celcius(temp_mV, &out_temp);
	int s = add_value(&temperature_filter, out_temp);
	return temperature_filter.avg;
}

/*!
	Converts a voltage to Degrees Celcius
	@param voltage input voltage, from temperature sensor
	@param output temperature in Celcius
	@return 0 if successful, -1 on failure
 */
int voltage_to_celcius(uint16_t voltage, float* output)
{
	float val2 = ((float)(voltage)/max) *3.0f;
	float val = (val2*to_mV - mV_25)/slope + 25.0f;
	*output = val;

	return 0;
}

