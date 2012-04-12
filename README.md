This application is a component of HAB, the high-altitude balloon flight controller.  It provides a bridge between to incoming serial data from the backup GPS (an eTrex Legend) and the main flight computer's I2C bus.  Rather than use a larger MCU to provide more on-board USARTs, we chose a bridge.  Further the secondary control off-loads the NMEA parsing from the primary controller.

---

## Target MCU ##
This code has been tested on an ATmega 328 AVR; but the code is compact enough to work with controllers with less flash memory - such as an ATmega 48 or ATmega 88.  The optimized code is just under 4K; so an ATmega 48 may work acceptably.

To [reduce the USART error](http://electronics.stackexchange.com/questions/5850/how-critical-are-uart-frequencies), the clock frequency should be:

* 1.8432 MHz
* 3.6864 MHz
* 7.3728 MHz
* 14.7456 MHz
* 16.5888 MHz

---

## Project files ##

### settings.h/.c ###
The settings files provide persistent settings functionality for the application.  Additional settings can be added by modifying the `settings_record_t` `struct`.  


### gps.h/.c ###
The gps file mainly takes care of parsing the Garmin text strings from the GPS.  Currently, it provides a limited amount of data.

To access coordinates components (lat/long) use the `gps_data` structure like:

    gps_data.coordinate.latitude.degrees


### TWI_slave.h/.c ###
The interface to the TWI/I2C bus.  This code is provided by Atmel.

---

## Operation ##

This device responds to the following operation codes:

#### E_W_DIR  0x20 ####
Returns the diretion of the East-West velocity vector as single byte character (E/W)

#### E_W_VEL 0x21 ####
Returns the magnitude of the East-West velocity vector as 2 bytes representing m/s * 100

#### N_S_DIR 0x22 ####
Returns the direction of North-South velocity vector as single character (N/S)

#### N_S_VEL 0x23 ####
Returns the magnitude of the North-South velocity vector as 2 bytes representing m/s * 100

#### VERT_DIR 0x24 ####
Returns the direction of the vertical velocity vector as single character (U/D)

#### VERT_VEL 0x25 ####
REturns the magnitude of the vertical velocity vector as 2 bytes representing m/s * 10

#### LAT 0x40 ####
Returns four bytes of data in the following order: direction (N/S), degrees, minutes, seconds.

#### LON 0x41 ####
Returns four bytes of data in the following order: direction (E/W), degrees, minutes, seconds.

#### FIX_TIME 0x50 ####
Returns the time for the current fix as as `fix_time_t` object expressed in six bytes for `year`, `month`, `day`, `hour`, `minute`, `second`.

#### DEBUG_ON 0x60 ####
Turns debugging on.  This enables display of visual data at PB2.  Returns `I2C_DEBUG_CONFIRM_BYTE`

#### DEBUG_OFF 0x61 ####
Turns off debugging.  Returns `I2C_DEBUG_CONFIRM_BYTE`.

#### TEST 0x02	####
Flashes diagnostic LED for testing purposes.