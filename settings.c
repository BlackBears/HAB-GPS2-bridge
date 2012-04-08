/*
 * settings.c
 *
 * Created: 4/7/2012 11:32:57 PM
 *  Author: Owner
 */ 
 
#include "settings.h"
#include <avr/eeprom.h>

void settings_read(void) {
	eeprom_read_block(&global_settings, &eeprom_settings, sizeof(settings_record_t));
}	/*	settings_read	*/

void settings_write(void) {
	eeprom_write_block(&global_settings, &eeprom_settings, sizeof(settings_record_t));
}	/*	settings_write	*/