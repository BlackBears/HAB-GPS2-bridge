/*
 * gps.c
 *
 * Created: 4/7/2012 11:18:54 PM
 *  Author: Owner
 */ 

/*
	RMC - NMEA has its own version of essential gps pvt (position, velocity, time) data. It is called RMC, The Recommended Minimum, which will look similar to:

	$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A

	Where:
		 RMC          Recommended Minimum sentence C
		 123519       Fix taken at 12:35:19 UTC
		 A            Status A=active or V=Void.
		 4807.038,N   Latitude 48 deg 07.038' N
		 01131.000,E  Longitude 11 deg 31.000' E
		 022.4        Speed over the ground in knots
		 084.4        Track angle in degrees True
		 230394       Date - 23rd of March 1994
		 003.1,W      Magnetic Variation
		 *6A          The checksum data, always begins with *
	Note that, as of the 2.3 release of NMEA, there is a new field in the RMC sentence at the end just prior to the checksum.
*/

#include "gps.h"

#include <string.h>
#include <stdlib.h>

#define RMC_RMC_START       0x00    //  GPRMC
#define RMC_FIX_TIME        0x01    //  123519 12:35:19 UTC
#define RMC_VALID_INDEX     0x02    //  A or V
#define RMC_LAT_INDEX       0x03    //
#define RMC_LAT_DIR_INDEX   0x04
#define RMC_LON_INDEX       0x05
#define RMC_LON_DIR_INDEX   0x06
#define RMC_VEL_KTS_INDEX   0x07
#define RMC_TRK_ANGEL_INDEX 0x08
#define RMC_DATE_INDEX      0x09    //  230394 March 23, 1994
#define RMC_MAG_VAR_INDEX   0x0A    //  003.1, 3.1 degrees
#define RMC_MVAR_DIR_INDEX  0x0B    //  W/E
#define RMC_CHECKSUM_INDEX  0x0C    //  *6A, always begins with *

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

char buffer[300];
u08 buffer_index;
BOOL _end_of_sentence;
u08 stops[] = {1,2,2,2,2,2,2,1,2,2,3,1,3,2,3,1,3,1,5,1,4,1,4,1,4};

void gps_init() {
    _end_of_sentence = 0;
	buffer_index = 0;
	buffer[0] = '\0';
}

void gps2_append_char(unsigned char c) {
	if( c == '@' ) {
		if( buffer_index != 0 ) {
			strncpy(buffer,"@120607204655N6012249E01107556S015+00130E0021N0018U0000",55);
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
			}
		}
		buffer[0] = c;
		buffer_index = 1;
	}
	else {
		buffer[buffer_index] = c;
		buffer_index++;
	}
}

BOOL gps_has_complete_sentence() {
    if( buffer[buffer_index-1] == 0x0D ) {
        _end_of_sentence = 1;
    }
    return _end_of_sentence;
}