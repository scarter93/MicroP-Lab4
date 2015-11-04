// seven_segment_display.h - displays numbers

#ifndef __INCLUDE_SEVEN_SEGMENT_H__
#define __INCLUDE_SEVEN_SEGMENT_H__

extern unsigned int wait;

/*!
	Makes system calls for setting up GPIO for 7 segment display
	
	@return 0 on success, else negative
 */
int seven_segment_setup(void);

/*!
	displays a number on the seven segment display
	@param to_display number to display
	
	@return 0 on success, else negative
 */
int display(float to_display);

/*!
	display a guess on the seven segment display. 
	This only displays as many digits as are input

	@param guess integer to display on seven segment display
	@return 0 on success, else -1
 */

int display_guess(int guess);

extern unsigned int wait;
#endif // __INCLUDE_SEVEN_SEGMENT_H__
