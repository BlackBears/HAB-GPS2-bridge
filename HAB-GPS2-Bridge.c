/*
 * HAB_GPS2_Bridge.c
 *
 * Created: 4/7/2012 11:18:31 PM
 *  Author: Owner
 */ 

#define PRODUCTION_VERSION 1

#if PRODUCTION_VERSION

/*	ETREX LEGEND GPS COMMUNICATES AT 4800 BAUD	*/
#define BAUD 4800

#include "global.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <util/setbaud.h>
#include "gps.h"
#include "settings.h"

#undef TRUE
#undef FALSE
#include "TWI_slave.h"

#define I2C_SLAVE_ADDRESS   	0xA0	//	we will listen on this address
#define I2C_DEBUG_CONFIRM_BYTE	0xF0	//	this byte is returned when debug mode is changed
#define I2C_ERROR				0xF2	//	code return when error encountered

/*  OPCODES FOR OUR I2C INTERFACE */

#define VEL_KTS     0x20    //  velocity in knots
#define LAT			0x40	//	return 4 bytes representing the latitude
#define LON			0x41	//	return 4 bytes representing the longitude
#define FIX_TIME	0x50	//	return the time of the most recent fix, returns 3 bytes
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
unsigned char outbuffer[2];

int main(void) {
	settings_read();
	if( global_settings.debug_mode == 1 ) {
		DDRD |= (1<<PD2);
		blink(global_settings.pwr_on_dx_count);
		_delay_ms(100);
	}
	
	serial_init();
	
	TWI_slaveAddress = I2C_SLAVE_ADDRESS;
	
	// Initialise TWI module for slave operation. Include address and/or enable General Call.
	TWI_Slave_Initialise( (unsigned char)((TWI_slaveAddress<<TWI_ADR_BITS) | (TRUE<<TWI_GEN_BIT) )); 
	
	sei();
	
	TWI_Start_Transceiver( ); 
	
    while(1) {
		if( !TWI_Transceiver_Busy() )                              
		{
			if( TWI_statusReg.RxDataInBuf )
			{
				TWI_Get_Data_From_Transceiver(outbuffer, 2);  
			}	/*	data in TWI status reg */
			opcode_process(outbuffer[0]);
		}	/* TWI interfance not busy */
	} 
}   /*  main */


void opcode_process(unsigned char opcode ) {
	BOOL error = 0;
	switch( opcode ) {
		case  LAT:
			outbuffer[0] = gps_data.latitude.degrees;
			outbuffer[1] = gps_data.latitude.minutes;
			outbuffer[2] = gps_data.latitude.seconds;
			outbuffer[3] = gps_data.latitude.direction;
			TWI_Start_Transceiver_With_Data(outbuffer, 4); 
			break;
		case LON:
			outbuffer[0] = gps_data.longitude.degrees;
			outbuffer[1] = gps_data.longitude.minutes;
			outbuffer[2] = gps_data.longitude.seconds;
			outbuffer[3] = gps_data.longitude.direction;
			TWI_Start_Transceiver_With_Data(outbuffer, 4); 
			break;
		case FIX_TIME:
			outbuffer[0] = gps_data.time.hour;
			outbuffer[1] = gps_data.time.minute;
			outbuffer[2] = gps_data.time.second;
			TWI_Start_Transceiver_With_Data(outbuffer, 3); 
		case VEL_KTS:
			outbuffer[0] = gps_data.velocity;
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
		case 0x02:
			outbuffer[0] = I2C_DEBUG_CONFIRM_BYTE;
			outbuffer[1] = I2C_DEBUG_CONFIRM_BYTE;
			TWI_Start_Transceiver_With_Data(outbuffer,2);
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


void serial_init()
{
	/* Set the baud rate */
	UBRR0H = UBRRH_VALUE;
	UBRR0L = UBRRL_VALUE;
	UCSR0C = (3 << UCSZ00);					//	8N1
	UCSR0B = (1 << RXEN0) | (1 << TXEN0);	//	enable RX and TX circuitry
	UCSR0B = (1 << RXCIE0 );				//	enable USART receive interrupt
	
	return;
}

ISR(USART_RX_vect) {
	char serial_data = UDR0;		//	get incoming character from UDR0
	appendCharacter(serial_data);	//	append char to NMEA stream
}

void blink(uint8_t count)
{
	for(uint8_t i = 0; i < count; i++) {
		PORTD |= (1<<PD2);
		_delay_ms(50);
		PORTD &= ~(1<<PD2);
		_delay_ms(50);
	}
}

#else

//	This can be deleted once we've fully debugged
//

#ifndef F_CPU
#define F_CPU 14745600L
#endif

#include "global.h"
#include "gps.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <compat/twi.h>


/*	ETREX LEGEND GPS COMMUNICATES AT 4800 BAUD	*/
#define BAUD 4800

#include <util/setbaud.h>
#include <util/delay.h>
#undef TRUE
#undef FALSE
#include "TWI_Slave.h"

// Sample TWI transmission commands
#define TWI_CMD_MASTER_WRITE 0x10
#define TWI_CMD_MASTER_READ  0x20

enum {
    kSlaveActionRead,
    kSlaveActionWrite
};
typedef unsigned char SlaveAction;

/*  OPCODES FOR OUR I2C INTERFACE */

#define LAT_DEG     0x01    //  latitude degrees
#define LAT_MIN     0x02    //  latitude minutes
#define LAT_SEC     0x03    //  latitude seconds
#define LAT_NS      0x04    //  north/south
#define LON_DEG     0x11    //  longitude degrees
#define LON_MIN     0x12    //  longitude minutes
#define LON_SEC     0x13    //  longitude seconds
#define LON_EW      0x14    //  east/west
#define VEL_KTS     0x20    //  velocity in knots
#define TIME_HR     0x30    //  hour
#define TIME_MIN    0x31    //  minute
#define TIME_SEC    0x32    //  second

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

uint8_t lat[4];
uint8_t lon[4];
uint8_t time[3];
uint8_t velocity;
uint8_t valid_data;

char serial_buffer[300];
uint8_t buffer_index;

#define I2C_SLAVE_ADDRESS   	0xA0	//	we will listen on this address
#define I2C_DEBUG_CONFIRM_BYTE	0xF0	//	this byte is returned when debug mode is changed
#define I2C_ERROR				0xF2	//	code return when error encountered

/*  OPCODES FOR OUR I2C INTERFACE */

#define VEL_KTS     0x20    //  velocity in knots
#define LAT			0x40	//	return 4 bytes representing the latitude
#define LON			0x41	//	return 4 bytes representing the longitude
#define FIX_TIME	0x50	//	return the time of the most recent fix
#define DEBUG_ON	0x60	//	turn on debugging mode
#define DEBUG_OFF	0x61	//	turn off debugging mode
#define TEST		0x02	//	flash the test LED

 
unsigned char messageBuf[TWI_BUFFER_SIZE];
unsigned char TWI_slaveAddress;
unsigned char temp;
unsigned char outbuffer[2];

void blink(uint8_t count);

int main(void) {
	serial_init();
	_delay_ms(100);
	
	TWI_slaveAddress = I2C_SLAVE_ADDRESS;
	
	// Initialise TWI module for slave operation. Include address and/or enable General Call.
	TWI_Slave_Initialise( (unsigned char)((TWI_slaveAddress<<TWI_ADR_BITS) | (TRUE<<TWI_GEN_BIT) )); 
	
    DDRD |= (1<<PD2);
	
	blink(2);
	_delay_ms(1000);
	
	
	
	sei();
	
	TWI_Start_Transceiver( ); 
	
    while(1) {
		if( !TWI_Transceiver_Busy() )                              
		{
			if( TWI_statusReg.RxDataInBuf )
			{
				TWI_Get_Data_From_Transceiver(outbuffer, 2);  
			}
			if( outbuffer[0] == 0x02 ) {
				outbuffer[1] = 0xAA;
			}
			else 
				outbuffer[1] = 0xBB;
			
			TWI_Start_Transceiver_With_Data(outbuffer, 2); 
		}
  } 
}   /*  main */

void opcode_process(unsigned char opcode ) {
	BOOL error = 0;
	switch( opcode ) {
		case  LAT:
			outbuffer[0] = gps_data.latitude.degrees;
			outbuffer[1] = gps_data.latitude.minutes;
			outbuffer[2] = gps_data.latitude.seconds;
			outbuffer[3] = gps_data.latitude.direction;
			TWI_Start_Transceiver_With_Data(outbuffer, 4); 
			break;
		case LON:
			outbuffer[0] = gps_data.longitude.degrees;
			outbuffer[1] = gps_data.longitude.minutes;
			outbuffer[2] = gps_data.longitude.seconds;
			outbuffer[3] = gps_data.longitude.direction;
			TWI_Start_Transceiver_With_Data(outbuffer, 4); 
			break;
		case FIX_TIME:
			outbuffer[0] = gps_data.time.hour;
			outbuffer[1] = gps_data.time.minute;
			outbuffer[2] = gps_data.time.second;
			TWI_Start_Transceiver_With_Data(outbuffer, 3); 
		case VEL_KTS:
			outbuffer[0] = gps_data.velocity;
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
			TWI_Start_Transceiver_With_Data(outbuffer,1);
			break;
		default: 
			error = 1;
			outbuffer[0] = I2C_ERROR;
			TWI_Start_Transceiver_With_Data(outbuffer, 1); 
			break;
	}	/* opcode switch */
	//if( IS_DEBUGGING && error)
		//blink(global_settings.error_dx_count);
}	/*	processOpcode()	*/


ISR(USART_RX_vect) {
	char recByte = UDR0;
	/*
	cli();
	PORTD ^= (1<<PD2);
	sei();
	*/
}

void serial_init()
{
	/* Set the baud rate */
	UBRR0H = UBRRH_VALUE;
	UBRR0L = UBRRL_VALUE;
	/* set the framing to 8N1 */
	UCSR0C = (3 << UCSZ00);
	/* Engage! */
	UCSR0B = (1 << RXEN0) | (1 << TXEN0);
	UCSR0B |= (1<<RXCIE0);
}

unsigned char serial_read(void)
{
	while( !(UCSR0A & (1 << RXC0)) )
		;
	return UDR0;
}

void blink(uint8_t count)
{
	for(uint8_t i = 0; i < count; i++) {
		PORTD |= (1<<PD2);
		_delay_ms(50);
		PORTD &= ~(1<<PD2);
		_delay_ms(50);
	}
}
#endif