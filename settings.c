/*
 * settings.c
 *
 * Created: 4/7/2012 11:32:57 PM
 *  Author: Owner
 */ 
 
#include "settings.h"
#include <avr/eeprom.h>

settings_record_t EEMEM eeprom_settings = {1,3,2};
	
void settings_read(void) {
	eeprom_read_block((void *)&global_settings, (const void *)&eeprom_settings, sizeof(settings_record_t));
}	/*	settings_read	*/

void settings_write(void) {
	eeprom_write_block((const void *)&global_settings, (void *)&eeprom_settings, sizeof(settings_record_t));
}	/*	settings_write	*/