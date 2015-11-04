// temperature.
#include <stdint.h>

/*!
	Sets up the Temperature Sensor and ADC using system calls.
	
	It is connected to Pin ADC1_IN16
 */
int temperature_setup(void);

/*!
	Gets the temparture sensor value
	@return Temp Value in  Celcius
*/
float get_temperature(void);
