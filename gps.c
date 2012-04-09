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

#define RMC_RMC_START       0x01    //  GPRMC
#define RMC_FIX_TIME        0x01    //  123519 12:35:19 UTC
#define RMC_VALID_INDEX     0x02    //  A or V
#define RMC_LAT_INDEX       0x03    //
#define RMC_LAT_DIR_INDEX   0x04
#define RMC_LON_INDEX       0x05
#define RMC_LON_DIR_INDEX   0x06
#define RMC_VEL_KTS_INDEX   0x07
#define RMC_TRK_ANGEL_INDEX 0x08
#define RMC_DATE_INDEX      0x09    //  230394 March 23, 1994
#define RMC_MAG_VAR_INDEX   0x10    //  003.1, 3.1 degrees
#define RMC_MVAR_DIR_INDEX  0x11    //  W/E
#define RMC_CHECKSUM_INDEX  0x12    //  *6A, always begins with *

#define GPS_DATA_INVALID	0xFE

char buffer[300];
u08 buffer_index;

BOOL appendCharacter(unsigned char c) {
	BOOL gpr_found = 0;
	buffer[buffer_index] = c;
    buffer_index++;
    if( c == 0x0D )
    {   
        if( strcmp(buffer,"$GPR") )
        {
        	gpr_found = 1;
        	
        	//	temporarily mark as complete.  If there are empty params in parsing,
        	//	then later mark as incomplete.
        	gps_validity_data.isComplete = 1;
        	
            char parts[15][20];
            char *p_start, *p_end;
            uint8_t i = 0;
            
            p_start = buffer;
            while(1) {
                p_end = strchr(p_start, ',' );
                if( p_end ) {
                    strncpy(parts[i], p_start, p_end-p_start);
                    parts[i][p_end-p_start] = 0;
                    i++;
                    p_start = p_end + 1;
                }
                else {
                    strncpy(parts[i], p_start, 20);
                    break;
                }
            }   /*  parsing $GPR string */
            if( strcmp(parts[RMC_VALID_INDEX],"V") ) {
                gps_validity_data.isValid = 0;
				buffer_index = 0;
    			buffer[0] = '\0';
                //return 0;
            }   /*  check for valid data */
			else
				gps_validity_data.isValid = 1;
            
            //  obtain the time in UTC
           
            if( strlen(parts[RMC_FIX_TIME]) == 0 ) {
            	gps_data.time.hour = GPS_DATA_INVALID;
            	gps_data.time.minute = GPS_DATA_INVALID;
            	gps_data.time.second = GPS_DATA_INVALID;
            	
            	gps_validity_data.isComplete = 0;
            }	/* no time is available */	
            else {
             	char *hr = (char *)malloc(2);
            	char *min = (char *)malloc(2);
            	char *sec = (char *)malloc(2);
            	
            	strncpy(hr, parts[RMC_FIX_TIME]+0,2);
				strncpy(min, parts[RMC_FIX_TIME]+2,2);
				strncpy(sec, parts[RMC_FIX_TIME]+4,2);
				//  store time in our registers
				gps_data.time.hour = atoi(hr);
				gps_data.time.minute = atoi(min);
				gps_data.time.second = atoi(sec);
				
				//  free substring memory
            	free(hr);
            	free(min);
            	free(sec);
            }	/* valid time is available */
            

            if( strlen(parts[RMC_LAT_INDEX]) == 0 ) {
            	gps_data.latitude.degrees = GPS_DATA_INVALID;
            	gps_data.latitude.minutes = GPS_DATA_INVALID;
            	gps_data.latitude.seconds = GPS_DATA_INVALID;
            	
            	gps_validity_data.isComplete = 0;
            }	/*	empty latitude */
            else {
            	//  obtain the latitude
				char *min_dec = (char *)malloc(2);
				char *min_int = (char *)malloc(2);
				char *deg = (char *)malloc(3);
				
				uint8_t len = strlen(parts[RMC_LAT_INDEX]);
				strncpy(min_dec,parts[RMC_LAT_INDEX] + len-2,2);
				strncpy(min_int,parts[RMC_LAT_INDEX] + len-5,2);
				if( len == 8 )
					strncpy(deg,parts[RMC_LAT_INDEX],3);
				else
					strncpy(deg,parts[RMC_LAT_INDEX],2);
				
				//  store the latitude
				gps_data.latitude.degrees = atoi(deg);
				gps_data.latitude.minutes = atoi(min_int);
				gps_data.latitude.seconds = atoi(min_dec) * 6/10;
            }	/* valid latitude */
            
            if( strlen(parts[RMC_LAT_DIR_INDEX]) == 0 ) {
				gps_data.latitude.direction = GPS_DATA_INVALID;
				
				gps_validity_data.isComplete = 0;
			}	/*	empty latitude direction */
			else
			{
				//  obtain the latitude direction
				if( strcmp(parts[RMC_LAT_DIR_INDEX],"N") )
					gps_data.latitude.direction = DIR_NORTH;
				else
					gps_data.latitude.direction = DIR_SOUTH;
			}	/* valid latitude direction */
            
            uint8_t len = strlen(parts[RMC_LON_INDEX]);
            if( len == 0 ) {
            	gps_data.longitude.degrees = GPS_DATA_INVALID;
            	gps_data.longitude.minutes = GPS_DATA_INVALID;
            	gps_data.longitude.seconds = GPS_DATA_INVALID;
            	
            	gps_validity_data.isComplete = 0;
            }	/* empty longitude */
            else {
				char *min_dec = (char *)malloc(2);
				char *min_int = (char *)malloc(2);
				char *deg = (char *)malloc(3);
				
            	strncpy(min_dec,parts[RMC_LON_INDEX] + len-2,2);
				strncpy(min_int,parts[RMC_LON_INDEX] + len-5,2);
				strncpy(deg,parts[RMC_LAT_INDEX],(len==8)?3:2);
				
				//  store the longitdue
				gps_data.longitude.degrees = atoi(deg);
				gps_data.longitude.minutes = atoi(min_int);
				gps_data.longitude.seconds = atoi(min_dec) * 6/10;
				
				free(min_dec);
				free(min_int);
				free(deg);
            }	/* valid longitude */
           
            
            //  obtain the longitude direction
            if( strlen(parts[RMC_LON_DIR_INDEX]) == 0 ) {
            	gps_data.longitude.direction = GPS_DATA_INVALID;
            	gps_validity_data.isComplete = 0;
            }	/* longitude direction is empty */
            else {
            	if( strcmp(parts[RMC_LON_DIR_INDEX],"E") )
                	gps_data.longitude.direction = DIR_EAST;
           	 	else
                	gps_data.longitude.direction = DIR_WEST;
            }	/* longitude direction is non-empty */
            
                
            //  parse the velocity
            //  assumes velocity is xxx.x or xx.x
            len = strlen(parts[RMC_VEL_KTS_INDEX]);
            if( len == 0 ) {
            	gps_data.velocity = GPS_DATA_INVALID;
            	gps_validity_data.isComplete = 0;
            }
            else {
            	char *velocity_str = (char *)malloc(3);
            
            	strncpy(velocity_str,parts[RMC_VEL_KTS_INDEX],len-2);
            	gps_data.velocity = atoi(velocity_str);
            
            	free(velocity_str);
            }            
			
			//	parse the date - note that the date is internally maintained
			//	and is reported by the GPS even when it has no satellite connection

			len = strlen(parts[RMC_DATE_INDEX]);
			if( len == 0 ) {
				gps_data.date.year = GPS_DATA_INVALID;
				gps_data.date.month = GPS_DATA_INVALID;
				gps_data.date.day = GPS_DATA_INVALID;
			}	
			else {
				char *yr_str = (char *)malloc(2);
				char *month_str = (char *)malloc(2);
				char *day_str = (char *)malloc(2);
				
				strncpy(yr_str,parts[RMC_DATE_INDEX],2);
				strncpy(month_str,parts[RMC_DATE_INDEX]+2,2);
				strncpy(day_str,parts[RMC_DATE_INDEX]+4,2);
				
				gps_data.date.year = atoi(yr_str);
				gps_data.date.month = atoi(month_str);
				gps_data.date.day = atoi(day_str);
				
				free(yr_str);
				free(month_str);
				free(day_str);
			}			

        }   /*  $GPR line */
        
        //	at the end of line, we can reset our buffer
        buffer_index = 0;
    	buffer[0] = '\0';
    }   /*  EOL */
    return gpr_found;
}
