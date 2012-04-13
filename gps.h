#ifndef GPS_H_
#define GPS_H_

/*******************************************************************
    Title   :   Header file for GPS2 serial-to-I2C bridge         
    Author  :   Alan K. Duncan <duncan.alan@mac.com>
    File    :   gps.h
    Software    :   AVR-GCC
    Target MCU  :   ATmega328 (ATmega88+)
*******************************************************************/

/**

@brief Bridge between serial output from GPS2 to I2C bus

This code is required to interface the backup GPS for the high-altitude balloon (HAB) to the main flight computer (MFC) which has only 2 UARTS (5 required.)  Rather than transition to a completely different platform (e.g. Xmega) I created this bridge that accepts incoming serial data from an old Garmin eTrex Legend GPS, parses it, stores the data, and provides it to the main flight computer on demand via
the I2C (TWI) interface.  The target MCU for this code acts as a slave I2C device for the MFC.

@note This code assumes that the GPS is an eTrex Legend and is set to Garmin Simple Text output mode at 4800 baud.

@author Alan K. Duncan <duncan.alan@mac.com>

@see <a href="http://www.gpsinformation.org/dale/interface.htm#text">Information</a> on the Garmin text output format.

*/

#include <inttypes.h>
#include "global.h"

typedef char coordination_direction_t;

/**
    @brief  A type that defines the time of a fix
*/
typedef struct {
	uint8_t year;
	uint8_t month;
	uint8_t day;
	uint8_t hour;
	uint8_t	minute;
	uint8_t second;
} fix_time_t;

/**
    @brief  A type that defines a coordinate component
*/
typedef struct  {
	uint8_t degree;
	uint8_t minute;
	uint8_t second;
	coordination_direction_t direction;
} coordinate_component_t;

/**
    @brief  A type that defines a coordinate
*/
typedef struct {
	coordinate_component_t latitude;
	coordinate_component_t longitude;
} coordinate_t;

/**
    @brief  A type that defines a velocity
*/
typedef struct {
	char direction;
	u16 magnitude;
} velocity_t;

/** 
    @brief  A type that defines a complete fix
*/
typedef struct {
	coordinate_t coordinate;
	velocity_t n_s_velocity;
	velocity_t e_w_velocity;
	velocity_t vert_velocity;
	fix_time_t time;
} gps_data_t;

typedef struct {
	BOOL isValid;
	BOOL isComplete;
} gps_validity_data_t;

gps_data_t gps_data;
gps_validity_data_t gps_validity_data;

#define INVALID_DATA 0xFE
#define GPS_DATA_INVALID	INVALID_DATA
#define GPS_DATA_INVALID_16	0xFFFE

/**
    @brief  Initialize the backup GPS interface
*/
void gps_init();

/**
    @brief  Append a character to the output text stream
    
    This function not only appends the character, but on stream completion, it
    parses the data.
    
    @param c an unsigned char to be appended
*/
void gps2_append_char(unsigned char c);

void gps2_generate_diagnostic_data(void);
void gps2_remove_diagnostic_data(void);

#endif /* GPS_H_ */