/*
 * gps.h
 *
 * Created: 4/7/2012 11:18:44 PM
 *  Author: Owner
 */ 


#ifndef GPS_H_
#define GPS_H_

#include <inttypes.h>
#include "global.h"

enum {
	DIR_NORTH,
	DIR_SOUTH,
	DIR_EAST = 0,
	DIR_WEST
};
typedef uint8_t coordinate_direction_t;

typedef struct {
	uint8_t hour;
	uint8_t	minute;
	uint8_t second;
} fix_time_t;

typedef struct {
	uint8_t year;
	uint8_t month;
	uint8_t day;
} fix_date_t;

typedef struct  {
	uint8_t degrees;
	uint8_t minutes;
	uint8_t seconds;
	coordinate_direction_t direction;
} coordinate_component_t;

typedef struct {
	coordinate_component_t latitude;
	coordinate_component_t longitude;
	fix_time_t time;
	fix_date_t date;
	uint8_t velocity;
} gps_data_t;

typedef struct {
	BOOL isValid;
	BOOL isComplete;
} gps_validity_data_t;

gps_data_t gps_data;
gps_validity_data_t gps_validity_data;

BOOL appendCharacter(unsigned char c);



#endif /* GPS_H_ */