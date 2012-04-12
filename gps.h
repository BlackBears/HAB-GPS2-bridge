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

typedef char coordination_direction_t;

typedef struct {
	uint8_t year;
	uint8_t month;
	uint8_t day;
	uint8_t hour;
	uint8_t	minute;
	uint8_t second;
} fix_time_t;

typedef struct  {
	uint8_t degree;
	uint8_t minute;
	uint8_t second;
	coordination_direction_t direction;
} coordinate_component_t;

typedef struct {
	coordinate_component_t latitude;
	coordinate_component_t longitude;
} coordinate_t;

typedef struct {
	char direction;
	u16 magnitude;
} velocity_t;

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


void gps_init();
void gps2_append_char(unsigned char c);


#endif /* GPS_H_ */