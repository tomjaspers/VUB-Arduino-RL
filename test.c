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

//CONSTANTS
static const int SIZE = 10;
static const int X_PIN = 2;
static const int Y_PIN = 3;
static const int STEP  = 10;

void initializeBoard() {
	USART_Init(MYUBRR);
	initDisplay();
	clearScreen();
}

int  main() {
	initializeBoard();
	//Create ball in the center of the screen
	Ball* b = createBall((SCREEN_WIDTH/2)-SIZE/2,(SCREEN_HEIGHT/2)-SIZE/2, SIZE,SIZE);
	//Set the ball to be white
	b->color = WHITE;
	//Create an accelerometer connected to the X and Y_PIN 
	Accelerometer* acc = newAccelerometer(X_PIN,Y_PIN); 
	printNumber(23);
	while(1) {
		//Ask the accelerometer for the direction
		direction d = sendMessage(acc,getDirection);
		switch(d) {
			case UP:    sendMessage(b,move,b->x_pos,b->y_pos+STEP); break;
			case DOWN:  sendMessage(b,move,b->x_pos,b->y_pos-STEP); break;
			case LEFT:  sendMessage(b,move,b->x_pos+STEP,b->y_pos); break;
			case RIGHT: sendMessage(b,move,b->x_pos-STEP,b->y_pos); break;
		}
	//	printNumber(b->x_pos);
	//	USART_Transmit(':');
///		printNumber(b->y_pos);
//		USART_Transmit('\n');
		_delay_ms(100);	
	}
	//cleanup
	free(b);
	free(acc);
}
