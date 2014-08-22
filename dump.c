/*
    This software library provides a naive implementation for the various
    components of the ATMEGA168p microcontroller. 
    It is not recomended to use this library for production purposes. 

    Copyright (C) 2014 Christophe Scholliers (Software Languages Lab VUB)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "lib.h"
#include <util/delay.h>


int  main() {
	USART_Init(MYUBRR);
	initDisplay();
	initAnalog();
	inputPin(DDRD,2);
	clearScreen();
	Ball* b = createBall(100,100, 10,10);
	b->color = WHITE;
	sendMessage(b,move,10,20);
	int p = 80;
	sendMessage(b,move,p,90);
	printNumber(b->x_pos);
	USART_Transmit('\n');
	//printNumber(b->y_pos);
	//USART_Transmit('\n');
	while(1){ 
		sendMessage(b,move,(b->x_pos+1)%131,10); 
		_delay_ms(10);
	};
	//clearScreen();

	int x,y,w,h = 10; 
	int color = 0x8;

	while(1) {
		if(readPulse(2) < 7920) {
			fillRectangle(x, 50, 10, 10, BLACK);
			x = (int)((x+11) % 130);
		} 
		if (readPulse(2) > 9920) {
			fillRectangle(x, 50, 10, 10, BLACK);
			x = (int)((x-11) % 130);
		}

		
		fillRectangle(x, 50, 10, 10, color);
		_delay_ms(100);	
		USART_Transmit('\n');
	}

	unsigned int addr = 5;
	unsigned char val = 2;
	while(1) {
		_delay_ms(BLINK_DELAY_MS);
		USART_Transmit('A');
		USART_Transmit(':');
		USART_Transmit(' ');
		//printNumber(readSensor());
		USART_Transmit(' ');
		printNumber(readAnalog(1));
		USART_Transmit('\n');
	}
	
	DDRD &= ~(1<<2);	
	while((PIND & (1<<2)) == 0) {}; 
	blink(1,5);
	EEPROM_write(addr, val);
	unsigned char c = EEPROM_read(addr);
	blink(c,5);
	while(1){};
}
