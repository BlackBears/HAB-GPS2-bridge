/*
 * HAB_GPS2_Bridge.c
 *
 * Created: 4/7/2012 11:18:31 PM
 *  Author: Owner
 */ 

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
#define FIX_TIME	0x50	//	return the time of the most recent fix
#define DEBUG_ON	0x60	//	turn on debugging mode
#define DEBUG_OFF	0x61	//	turn off debugging mode
#define TEST		0x02	//	flash the test LED


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
	
	//serial_init();
	
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
				TWI_Get_Data_From_Transceiver(outbuffer, 1);  
			}
			opcode_process(outbuffer[0]);
			
			TWI_Start_Transceiver_With_Data(outbuffer, 1); 
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
		case 0x02:
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


void serial_init()
{
	/* Set the baud rate */
	UBRR0H = UBRRH_VALUE;
	UBRR0L = UBRRL_VALUE;
	UCSR0C = (3 << UCSZ00);		//	8N1
	UCSR0B = (1 << RXEN0) | (1 << TXEN0);	//	enable RX and TX circuitry
	UCSR0B = (1 << RXCIE0 );	//	enable USART receive interrupt
	
	return;
}

ISR(USART_RX_vect) {
	char serial_data = UDR0;
	appendCharacter(serial_data);
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
