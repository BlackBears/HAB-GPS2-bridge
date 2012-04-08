/*
 * settings.h
 *
 * Created: 4/7/2012 11:32:43 PM
 *  Author: Owner
 */ 


#ifndef SETTINGS_H_
#define SETTINGS_H_

#include "global.h"
#include <inttypes.h>

typedef struct {
	uint8_t debug_mode;
	uint8_t pwr_on_dx_count;
	uint8_t error_dx_count;
} settings_record_t;

volatile settings_record_t global_settings;

void settings_read(void);
void settings_write(void);

#endif /* SETTINGS_H_ */