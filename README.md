This application is a component of HAB, the high-altitude balloon flight controller.  It provides a bridge between to incoming serial data from the backup GPS (an eTrex Legend) and the main flight computer's I2C bus.  Rather than use a larger MCU to provide more on-board USARTs, we chose a bridge.  Further the secondary control off-loads the GPS stream parsing from the primary controller.

### Garmin Text Output Mode Description ###

<table border=1>
<tr>
<th colspan=2>FIELD DESCRIPTION:</th><th>WIDTH:</th><th>NOTES:</th>
</tr><tr>
<td>&nbsp;</td><td>Sentence start</td><td>1</td><td>Always '@'</td>
</tr><tr>
<td rowspan=6 width=6><center>T<br>I<br>M<br>E<br></center></td>
<td>Year</td><td>2</td><td>Last two digits of UTC year</td><tr>
<td>Month</td><td>2</td><td>UTC month, "01".."12"</td><tr>
<td>Day</td><td>2</td><td>UTC day of month, "01".."31"</td><tr>
<td>Hour</td><td>2</td><td>UTC hour, "00".."23"</td><tr>
<td>Minute</td><td>2</td><td>UTC minute, "00".."59"</td><tr>
<td>Second</td><td>2</td><td>UTC second, "00".."59"</td><tr>

<td rowspan=8 width=6><Center>P<br>O<br>S<br>I<br>T<br>I<br>O<br>N</center>
</td>
<td>Latitude hemisphere</td><td>1</td><td>'N' or 'S'</td><tr>
<td>Latitude position</td><td>7</td><td>WGS84 ddmmmmm, with an implied
                                        decimal after the 4th digit</td><tr>
<td>Longitude hemishpere</td><td>1</td><td>'E' or 'W'</td><tr>
<td>Longitude position</td><td>8</td><td>WGS84 dddmmmmm with an implied
                                         decimal after the 5th digit</td><tr>
<td>Position status</td><td>1</td><td>'d' if current 2D differential GPS 
                                    position<br>
                                    'D' if current 3D differential GPS 
                                    position<br>
                                    'g' if current 2D GPS position<br>
                                    'G' if current 3D GPS position<br>
                                    'S' if simulated position<br>
                                    '_' if invalid position</td><tr>
<td>Horizontal posn error</td><td>3</td><td>EPH in meters</td><tr>
<td>Altitude sign</td><td>1</td><td>'+' or '-'</td><tr>
<td>Altitude</td><td>5</td><td>Height above or below mean
                                sea level in meters</td><tr>
<td rowspan=6 width=6><center>V<br>E<br>L<br>O<br>C<br>I<br>T<br>Y</center>
</td>
<td>East/West velocity<br>direction</td><td>1</td><td>'E' or 'W'</td><tr>
<td>East/West velocity<br>magnitude</td><td>4</td><td>
Meters per second in tenths, ("1234" = 123.4 m/s)</td><tr>
<td>North/South velocity<br>direction</td><td>1</td><td>'N' or 'S'</td><tr>
<td>North/South velocity<br>magnitude</td><td>4</td><td>Meters
per second in tenths, ("1234" = 123.4 m/s)</td><tr>
<td>Vertical velocity<br>direction</td><td>1</td><td>'U' (up) or 'D' (down)
</td><tr>
<td>Vertical velocity<br>magnitude</td><td>4</td><td>Meters
per second in hundredths, ("1234" = 12.34 m/s)</td><tr>
<td>&nbsp;</td>
<td>Sentence end</td><td>2</td><td>Carriage return, '0x0D', and
                                    line feed, '0x0A'</td>
</table>

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