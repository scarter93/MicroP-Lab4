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
	flash the seven segment display
	@param 1 starts flashing the display and 0
					 stops flashin
 */
void flash_display(int toggle);

/*!
	Thread which updates the Seven Segment Dispaly
 */
void Display(void const *argument);

#endif // __INCLUDE_SEVEN_SEGMENT_H__
