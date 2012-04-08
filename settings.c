/*
 * settings.c
 *
 * Created: 4/7/2012 11:32:57 PM
 *  Author: Owner
 */ 
#include "settings.h"
#include <avr/eeprom.h>

settings_record_t global_settings_record EEMEM = {1,6,3};

void settings_read(void) {
	eeprom_read_block((void *)&global_settings, (const void *)&global_settings_record, sizeof(global_settings));
}	/*	settings_read	*/

void settings_write(void) {
	eeprom_write_block(&global_settings, &global_settings_record, sizeof(global_settings));
}	/*	settings_write	*/