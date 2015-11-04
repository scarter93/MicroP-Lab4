#include "seven_segment_display.h"

#include "stm32f4xx.h"                  // Device header
#include "stm32f4xx_conf.h"
		
#define IO_SEVEN_SEGMENT GPIOB

unsigned int wait = 0;

/*!
	Which pin (on IO_SEVEN_SEGMENT) is used for each digit.
 */
typedef enum {
	DIGIT_FIRST = GPIO_Pin_0,  // 1
	DIGIT_SECOND = GPIO_Pin_1, // 2
	DIGIT_THIRD = GPIO_Pin_2,  // 6
	ALL_DIGITS = DIGIT_FIRST | DIGIT_SECOND | DIGIT_THIRD,
} DIGIT;

/*!
	Which pin (on IO_SEVEN_SEGMENT) is used for each segment.
*/
typedef enum {
	SEGMENT_A = GPIO_Pin_4,	 // 14
	SEGMENT_B = GPIO_Pin_5,  // 16
	SEGMENT_C = GPIO_Pin_7,  // 13
	SEGMENT_D = GPIO_Pin_8,  // 3
	SEGMENT_E = GPIO_Pin_11, // 5
	SEGMENT_F = GPIO_Pin_12, // 11
	SEGMENT_G = GPIO_Pin_13, // 15
	SEGMENT_DEC = GPIO_Pin_14,	// 7
	ALL_SEGS = SEGMENT_A | SEGMENT_B | SEGMENT_C | SEGMENT_D | SEGMENT_E | SEGMENT_F | SEGMENT_G
} SEGMENT;

/*!
	Digits which can be dispalyed
	TODO: set equal to bit vector contianing which segments to show
 */
typedef enum {
	NUM_ZERO = ALL_SEGS ^ SEGMENT_G,
	NUM_ONE = SEGMENT_B | SEGMENT_C,
	NUM_TWO = ALL_SEGS ^ SEGMENT_C ^ SEGMENT_F,
	NUM_THREE = ALL_SEGS ^ SEGMENT_E ^ SEGMENT_F,
	NUM_FOUR = SEGMENT_B | SEGMENT_C | SEGMENT_G |SEGMENT_F,
	NUM_FIVE= ALL_SEGS ^ SEGMENT_B ^ SEGMENT_E,
	NUM_SIX = ALL_SEGS ^ SEGMENT_B,
	NUM_SEVEN = SEGMENT_A | SEGMENT_B | SEGMENT_C,
	NUM_EIGHT = ALL_SEGS,
	NUM_NINE = ALL_SEGS ^ SEGMENT_E,
	NUM_OFF = 0,
} NUMBER;

/*!
	Convert from an integer to its representation in segments
 */
NUMBER toSegments[10] = {
	NUM_ZERO,
	NUM_ONE,
	NUM_TWO,
	NUM_THREE,
	NUM_FOUR,
	NUM_FIVE,
	NUM_SIX,
	NUM_SEVEN,
	NUM_EIGHT,
	NUM_NINE
};

/*!
	Allows a modulo 3 counter to choose a digit
 */
DIGIT INDEX[3] = {
	DIGIT_FIRST,
	DIGIT_SECOND,
	DIGIT_THIRD,
};

NUMBER num[3];

/*!
	Makes system calls for setting up GPIO for 7 segment display
	
	@return 0 on success, else negative
 */
int seven_segment_setup() {
	
	/* Enable Timer */
	// Give power to Timer 3
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	
	TIM_TimeBaseInitTypeDef tim_init_s;
	// choose a combination of Prescaler and Period for smooth transitions
	tim_init_s.TIM_Prescaler = 400;
	tim_init_s.TIM_Period = 500;
	tim_init_s.TIM_CounterMode =  TIM_CounterMode_Up;
	tim_init_s.TIM_ClockDivision = TIM_CKD_DIV1;
	tim_init_s.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM3, &tim_init_s);
	// enable the clock
	TIM_Cmd(TIM3, ENABLE);
	// needed for IRQ to be called
	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
	
	/* Enable IO Pins */
	// Give power to IO_SEVEN_SEGMENT
	RCC_AHB1PeriphClockCmd (RCC_AHB1Periph_GPIOB, ENABLE);
		
	GPIO_InitTypeDef gpio_init_s;
	gpio_init_s.GPIO_Mode = GPIO_Mode_OUT; 			// Set as Output
	gpio_init_s.GPIO_Speed = GPIO_Speed_100MHz; // Don't limit slew rate, allow values to change as fast as they are set
	gpio_init_s.GPIO_OType = GPIO_OType_PP;			// Operating output type (push-pull) for selected pins
	gpio_init_s.GPIO_PuPd = GPIO_PuPd_NOPULL; 	// If there is no input, don't pull.
	
	// Turn on Pins for outputs to All Segments and All Digits
	gpio_init_s.GPIO_Pin = ALL_DIGITS | ALL_SEGS | SEGMENT_DEC;
	GPIO_Init(IO_SEVEN_SEGMENT, &gpio_init_s);
	
	// start with all pins off
	GPIO_SetBits(IO_SEVEN_SEGMENT, DIGIT_FIRST | DIGIT_SECOND | DIGIT_THIRD);
	GPIO_ResetBits(IO_SEVEN_SEGMENT, ALL_SEGS | SEGMENT_DEC);
	
	/* Enable Timer 3 Interrupts */
	NVIC_InitTypeDef  nvic_init_s;
	nvic_init_s.NVIC_IRQChannel = TIM3_IRQn;
	nvic_init_s.NVIC_IRQChannelCmd = ENABLE;
	nvic_init_s.NVIC_IRQChannelPreemptionPriority = 0x01;
	nvic_init_s.NVIC_IRQChannelSubPriority = 0x01;
	
	NVIC_Init(&nvic_init_s);
	
	return 0;
}

unsigned int count = 0; //! alternate between digits
void TIM3_IRQHandler() {
	TIM_ClearFlag(TIM3, TIM_IT_Update);
	
	int index_tmp = count++ % 3;
	// Select a Digit
	GPIO_SetBits(IO_SEVEN_SEGMENT, ALL_DIGITS);
	GPIO_ResetBits(IO_SEVEN_SEGMENT, INDEX[index_tmp]);
	
	// Display a Number
	GPIO_ResetBits(IO_SEVEN_SEGMENT, ALL_SEGS | SEGMENT_DEC);
	GPIO_SetBits(IO_SEVEN_SEGMENT, num[index_tmp]);

	wait++;
}

/*!
	displays a number on the seven segment display
	@param to_display number to display
	
	@return 0 on success, else negative
 */
int display(float to_display) {
	// turn all segments off
	num[0] = NUM_OFF;
	num[1] = NUM_OFF;
	num[2] = NUM_OFF;
	
	// error if too small or too large
	if (to_display < 0 || to_display > 180) {
		return -1;
	}
	
	// convert to_display into an integer for easier operations
	int tmp;
	
	// add decimal point if needed and find digits
	if (to_display < 10) {
		num[0] |= SEGMENT_DEC;
		tmp = (int) (100 * to_display);
	} else if (to_display < 100) {
		num[1] |= SEGMENT_DEC;
		tmp = (int) (10 * to_display);
	} else {
		tmp = (int) to_display;
	}
	// set all digits
	// get last digit
	num[2] |= toSegments[tmp % 10];
	// shift over by a decimal place
	tmp = tmp/10;
	
	num[1] |= toSegments[tmp % 10];
	tmp = tmp/10;
	
	num[0] |= toSegments[tmp % 10];
	tmp = tmp/10;
	
	return 0;
}

/*!
	display a guess on the seven segment display. 
	This only displays as many digits as are input

	@param guess integer to display on seven segment display
	@return 0 on success, else -1
 */

int display_guess(int guess) {
		// turn all segments off
	num[0] = NUM_OFF;
	num[1] = NUM_OFF;
	num[2] = NUM_OFF;
	
	// error if too small or too large
	if (guess < 0 || guess > 180) {
		return -1;
	}
	// set LSB
	for (int i = 2; i >=0; i--) {
		num[i] |= toSegments[guess % 10];
		guess = guess / 10;
		if (guess == 0) {
			return 0;
		}
	}
	return -1;
}
