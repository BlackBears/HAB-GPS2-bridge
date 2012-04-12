/*
 * gps.c
 *
 * Created: 4/7/2012 11:18:54 PM
 *  Author: Owner
 */ 

/*

*/

#include "gps.h"
#include <string.h>
#include <stdlib.h>


enum {
	k_sentence_start,
	k_year, k_month, k_day, k_hour, k_minute, k_second,
	k_lat_hemisphere,k_lat_position_degree,k_lat_position_minute,k_lat_position_minute_fx,
	k_lon_hemisphere,k_lon_position_degree,k_lon_position_minute,k_long_position_minute_fx,
	k_position_status, k_h_position_error,
	k_alt_sign, k_alt,
	k_e_w_dir, k_e_w_magnitude,
	k_n_s_dir, k_n_s_magnitude,
	k_vert_dir, k_vert_magnitude
};

char buffer[100];   //  local store of GPS text
u08 buffer_index;   //  location in buffer
/*  widths of fields in Garmin GPS text output */
u08 stops[] = {1,2,2,2,2,2,2,1,2,2,3,1,3,2,3,1,3,1,5,1,4,1,4,1,4};

void gps_init() {
	buffer_index = 0;
	buffer[0] = '\0';
}

#define DEBUG_FOR_ABSENT_GPS_FIX 1

void gps2_append_char(unsigned char c) {
	if( c == '@' ) {
		if( buffer_index != 0 ) {
		    #if DEBUG_FOR_ABSENT_GPS_FIX
			strncpy(buffer,"@120607204655N6012249E01107556S015+00130E0021N0018U0000",55);
			#endif
			char *temp_str = (char *)malloc(4);
			u08 index = 0;
			for(u08 i = 0; i < 25; i++) {
				strncpy(temp_str, buffer+index, stops[i]);
				temp_str[stops[i]] = '\0';
				index += stops[i];
				
				switch(i) {
					case k_year:
						gps_data.time.year = (temp_str[0] == '_')?INVALID_DATA:atoi(temp_str);
						break;
					case k_month:
						gps_data.time.month = (temp_str[0] == '_')?INVALID_DATA:atoi(temp_str);
						break;
					case k_day:
						gps_data.time.day = (temp_str[0] == '_')?INVALID_DATA:atoi(temp_str);
						break;
					case k_hour:
						gps_data.time.hour = (temp_str[0] == '_')?INVALID_DATA:atoi(temp_str);
						break;
					case k_minute:
						gps_data.time.minute = (temp_str[0] == '_')?INVALID_DATA:atoi(temp_str);
						break;
					case k_second:
						gps_data.time.second = (temp_str[0] == '_')?INVALID_DATA:atoi(temp_str);
						break;
					case k_lat_hemisphere:
						gps_data.coordinate.latitude.direction = (temp_str[0] == '_')?INVALID_DATA:temp_str[0];
						break;
					case k_lat_position_degree:
						gps_data.coordinate.latitude.degree = (temp_str[0] == '_')?INVALID_DATA:atoi(temp_str);
						break;
					case k_lat_position_minute:
						gps_data.coordinate.latitude.minute = (temp_str[0] == '_')?INVALID_DATA:atoi(temp_str);
						break;
					case k_lat_position_minute_fx:
						if( temp_str[0] == '_' )
							gps_data.coordinate.latitude.second = INVALID_DATA;
						else {
							u16 second = atoi(temp_str) * 6/100;
							gps_data.coordinate.latitude.second = (u08)second;
						}
						break;						
					case k_lon_hemisphere:
						gps_data.coordinate.longitude.direction = (temp_str[0] == '_')?INVALID_DATA:temp_str[0];
						break;
					case k_lon_position_degree:
						gps_data.coordinate.longitude.degree = (temp_str[0] == '_')?INVALID_DATA:atoi(temp_str);
						break;
					case k_lon_position_minute:
						gps_data.coordinate.longitude.minute = (temp_str[0] == '_')?INVALID_DATA:atoi(temp_str);
						break;
					case k_long_position_minute_fx:
					{
						u16 s = atoi(temp_str) * 6/100;
						gps_data.coordinate.longitude.second = (u08)s;
					}					
						break;
					case k_e_w_dir:
						gps_data.e_w_velocity.direction = (temp_str[0] == '_')?INVALID_DATA:temp_str[0];
						break;
					case k_e_w_magnitude:
						gps_data.e_w_velocity.magnitude = (temp_str[0] == '_')?GPS_DATA_INVALID_16:atoi(temp_str);
						break;
					case k_n_s_dir:
						gps_data.n_s_velocity.direction =  (temp_str[0] == '_')?INVALID_DATA:temp_str[0];
						break;
					case k_n_s_magnitude:
						gps_data.n_s_velocity.magnitude = (temp_str[0] == '_')?GPS_DATA_INVALID_16:atoi(temp_str);
						break;
					case k_vert_dir:
						gps_data.vert_velocity.direction = (temp_str[0] == '_')?INVALID_DATA:temp_str[0];
						break;
					case k_vert_magnitude:
						gps_data.vert_velocity.magnitude = (temp_str[0] == '_')?GPS_DATA_INVALID_16:atoi(temp_str);
						break;
				}	/* switch parts */
			}   /* iterate fields */
		}   /* sentence buffer is full */
		buffer[0] = c;
		buffer_index = 1;
	}   /*  @ character */
	else {
		buffer[buffer_index] = c;
		buffer_index++;
	}   /*  not @ character */
}   /* gps2_append_char */
