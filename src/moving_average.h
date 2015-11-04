// moving average filter
#ifndef __INCLUDE_MOVING_AVERAGE_H_
#define __INCLUDE_MOVING_AVERAGE_H_

#define BUFF_SIZE 10

typedef struct {
	float buffer[BUFF_SIZE];
	int replace;
	float avg;
	float sum;
} FilterBuffer;

/*!
	adds a value to the filter
 */
int add_value(FilterBuffer *buf, float f);

/*!
	returns average value
 */
float get_value(FilterBuffer *buf);

#endif //__INCLUDE_MOVING_AVERAGE_H_
