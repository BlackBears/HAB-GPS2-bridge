/*
 * HAB_GPS2_Bridge.c
 *
 * Created: 4/7/2012 11:18:31 PM
 *  Author: Owner
 */ 

#include "global.h"


/*	ETREX LEGEND GPS COMMUNICATES AT 4800 BAUD	*/
#define UART_BAUD_RATE      4800  

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "gps.h"
#include "settings.h"
#include "uart.h"

#undef TRUE
#undef FALSE
#include "TWI_slave.h"

#define I2C_SLAVE_ADDRESS   	0xA0	//	we will listen on this address
#define I2C_DEBUG_CONFIRM_BYTE	0xF0	//	this byte is returned when debug mode is changed
#define I2C_ERROR				0xF2	//	code return when error encountered

/*  OPCODES FOR OUR I2C INTERFACE */

#define VALID_CODE	0x10	//	A if valid, V if invalid
#define VEL_KTS     0x20    //  velocity in knots
#define LAT			0x40	//	return 4 bytes representing the latitude
#define LON			0x41	//	return 4 bytes representing the longitude
#define FIX_TIME	0x50	//	return the time of the most recent fix, returns 3 bytes
#define FIX_DATE	0x51	//	return the date from GPS, returns 3 bytes (year,month,day)
#define DEBUG_ON	0x60	//	turn on debugging mode, returns I2C_DEBUG_CONFIRM_BYTE
#define DEBUG_OFF	0x61	//	turn off debugging mode, returns I2C_DEBUG_CONFIRM_BYTE
#define TEST		0x02	//	flash the test LED, returns I2C_DEBUG_CONFIRM_BYTE


/*
	See: https://brezn.muc.ccc.de/svn/moodlamp-rf/trunk/bussniffer/settings.c
*/


#define IS_DEBUGGING global_settings.debug_mode == 1

/*	FUNCTION PROTOTYPES */
void blink(uint8_t count);

void opcode_process(unsigned char opcode );
void serial_init();

unsigned char messageBuf[TWI_BUFFER_SIZE];
unsigned char TWI_slaveAddress;
unsigned char temp;
unsigned char outbuffer[4];
unsigned char inbuffer[2];

int main(void) {
	settings_read();
	
	if( global_settings.debug_mode == 1 ) {
		DDRD |= (1<<PD2);
		blink(global_settings.pwr_on_dx_count);
		_delay_ms(100);
	}
	uart_init( UART_BAUD_SELECT(UART_BAUD_RATE,F_CPU) ); 
	gps_init();
	
	TWI_slaveAddress = I2C_SLAVE_ADDRESS;
	
	// Initialise TWI module for slave operation. Include address and/or enable General Call.
	TWI_Slave_Initialise( (unsigned char)((TWI_slaveAddress<<TWI_ADR_BITS) | (TRUE<<TWI_GEN_BIT) )); 
		
	sei();
	
	TWI_Start_Transceiver( ); 
	
    while(1) {
		char c = uart_getc();
        if ( c & UART_NO_DATA ) {}
        else {
			if( c > 0x21 ) {
				gps2_append_char(c);
			}	/* not a control character */
		}	/* data available in serial buffer */		
		if( !TWI_Transceiver_Busy() ) {
			if( TWI_statusReg.RxDataInBuf ) {
				TWI_Get_Data_From_Transceiver(&temp, 2);  
			}	//	data in TWI status reg
			opcode_process(temp);
			
			/*
			if( outbuffer[0] == 0x02 ) {
				outbuffer[1] = 0xAA;
			}
			else 
				outbuffer[1] = 0xBB;
				*/

		}	// TWI interface not busy
	} 
}   /*  main */


void opcode_process(unsigned char opcode ) {
	BOOL error = 0;
	switch( opcode ) {
		case VALID_CODE:
			if( gps_validity_data.isValid == FALSE )
				outbuffer[0] = 'V';
			else
				outbuffer[0] = 'A';
			TWI_Start_Transceiver_With_Data(outbuffer,1);
			break;
		case LAT:
			outbuffer[0] = gps_data.coordinate.latitude.degree;
			outbuffer[1] = gps_data.coordinate.latitude.minute;
			outbuffer[2] = gps_data.coordinate.latitude.second;
			outbuffer[3] = gps_data.coordinate.latitude.direction;
			TWI_Start_Transceiver_With_Data(outbuffer, 4); 
			break;
		case LON:
			outbuffer[0] = gps_data.coordinate.longitude.degree;
			outbuffer[1] = gps_data.coordinate.longitude.minute;
			outbuffer[2] = gps_data.coordinate.longitude.second;
			outbuffer[3] = gps_data.coordinate.longitude.direction;
			TWI_Start_Transceiver_With_Data(outbuffer, 4); 
			break;
		case FIX_TIME:
			outbuffer[0] = gps_data.time.hour;
			outbuffer[1] = gps_data.time.minute;
			outbuffer[2] = gps_data.time.second;
			TWI_Start_Transceiver_With_Data(outbuffer, 3); 
			break;
		case FIX_DATE:
			outbuffer[0] = gps_data.time.year;
			outbuffer[1] = gps_data.time.month;
			outbuffer[2] = gps_data.time.day;
			TWI_Start_Transceiver_With_Data(outbuffer,3);
			break;
		case VEL_KTS:
			outbuffer[0] = gps_data.e_w_velocity.magnitude;
			TWI_Start_Transceiver_With_Data(outbuffer, 1); 
			break;
		case DEBUG_ON:
			outbuffer[0] = I2C_DEBUG_CONFIRM_BYTE;
			TWI_Start_Transceiver_With_Data(outbuffer, 1);
			break;
		case DEBUG_OFF:
			outbuffer[0] = I2C_DEBUG_CONFIRM_BYTE;
			TWI_Start_Transceiver_With_Data(outbuffer, 1);
			break;
		case TEST:
			outbuffer[0] = I2C_DEBUG_CONFIRM_BYTE;
			outbuffer[1] = I2C_DEBUG_CONFIRM_BYTE;
			TWI_Start_Transceiver_With_Data(outbuffer,1);
			//blink(2);
			break;
		default: 
			error = 1;
			outbuffer[0] = I2C_ERROR;
			outbuffer[1] = I2C_ERROR;
			TWI_Start_Transceiver_With_Data(outbuffer,2);
			break;
	}	/* opcode switch */
	if( IS_DEBUGGING && error)
		blink(global_settings.error_dx_count);
}	/*	processOpcode()	*/

void blink(uint8_t count)
{
	for(uint8_t i = 0; i < count; i++) {
		PORTD |= (1<<PD2);
		_delay_ms(50);
		PORTD &= ~(1<<PD2);
		_delay_ms(50);
	}
}