#    This software library provides a naive implementation for the various
#    components of the ATMEGA168p microcontroller. 
#    It is not recomended to use this library for production purposes. 
#
#    Copyright (C) 2014 Christophe Scholliers (Software Languages Lab VUB)
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.

CC = /Applications/Arduino.app/Contents/Resources/Java/hardware/tools/avr/bin/avr-gcc  
OO = /Applications/Arduino.app/Contents/Resources/Java/hardware/tools/avr/bin/avr-objcopy 
DU = /Applications/Arduino.app/Contents/Resources/Java/hardware/tools/avr/bin/avrdude 

all:
	$(CC) -Os -DF_CPU=16000000UL -mmcu=atmega168p -Wall -c lib.c test.c
	$(CC) -mmcu=atmega168p lib.o test.o -o test
	$(OO) -O ihex -R .eeprom test test.hex

upload: 
	$(DU) -F -V -c arduino -p ATMEGA168P -P /dev/tty.usbserial-A5002tJY  -b 19200 -U flash:w:test.hex -v

doc:	
	doxygen config

clean: 
	rm *.o
	rm *.hex
