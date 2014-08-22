/*
    This software library provides a naive implementation for the various
    components of the ATMEGA168p microcontroller. 

    EEPROM_write, EEPROM_read, USART_Init and USART_Transmit are 
    copy-pasted from the official datasheet.  
  
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
#include <stdio.h>
#include <stdlib.h>

/*
__________________________________________ ________      _____   
\_   _____/\_   _____/\______   \______   \\_____  \    /     \  
 |    __)_  |    __)_  |     ___/|       _/ /   |   \  /  \ /  \ 
 |        \ |        \ |    |    |    |   \/    |    \/    Y    \
/_______  //_______  / |____|    |____|_  /\_______  /\____|__  /
        \/         \/                   \/         \/         \/ 
*/

void EEPROM_write(unsigned int uiAddress, unsigned char ucData) {
	/* Wait for completion of previous write */ 
	while(isSet(EECR,EEPE));
	/* Set up address and Data Registers */ 
	EEAR = uiAddress;
	EEDR = ucData;
	/* Write logical one to EEMPE */
	sbi(EECR,EEMPE);
	/* Start eeprom write by setting EEPE */ 
	sbi(EECR,EEPE);
} 

unsigned char EEPROM_read(unsigned int uiAddress) {
	/* Wait for completion of previous write */
	while(isSet(EECR,EEPE));
	/* Set up address register */
	EEAR = uiAddress;
	/* Start eeprom read by writing EERE */ 
	sbi(EECR,EERE);
	/* Return data from Data Register */ 
	return EEDR;
}

void loop() { while(1) {} };

void blink(int x,int pin) {
	outputPin(DDRD,pin);
	while(x > 0) {
		/* set pin 5 high to turn led on */
		setPin(PORTB,pin);
		_delay_ms(BLINK_DELAY_MS);
		/* set pin 5 low to turn led off */
		clearPin(PORTB,pin);
		_delay_ms(BLINK_DELAY_MS);
		x = x-1;
	}
	return;
}

/*
 ____ ___  _________   _____ _____________________
|    |   \/   _____/  /  _  \\______   \__    ___/
|    |   /\_____  \  /  /_\  \|       _/ |    |   
|    |  / /        \/    |    \    |   \ |    |   
|______/ /_______  /\____|__  /____|_  / |____|   
                 \/         \/       \/           
*/
void USART_Init( unsigned int ubrr) {
	/*Set baud rate */
	UBRR0H = (unsigned char)(ubrr>>8); 
	UBRR0L = (unsigned char)ubrr;
	/*Enable receiver and transmitter */ 
	UCSR0B = (1<<RXEN0)|(1<<TXEN0);
	/* Set frame format: 8data, 2stop bit */ 
	UCSR0C = (1<<USBS0)|(3<<UCSZ00);
}

void USART_Transmit( unsigned char data ) {
	/* Wait for empty transmit buffer */ 
	while ( !( UCSR0A & (1<<UDRE0)) );
	/* Put data into buffer, sends the data */ 
	UDR0 = data;
}


void printNumber(int x) {
	char buffer[8];
	int count = 0;
	while(count<8) {	
		buffer[count++] = '.';
	}	
	sprintf(buffer,"%d",x); 
	count=0;
	while(count<8) {	
		USART_Transmit(buffer[count++]);  
	}
}

/*
   _____        /\ ________    _________                                        .__               
  /  _  \      / / \______ \   \_   ___ \  ____   _______  __ ___________  _____|__| ____   ____  
 /  /_\  \    / /   |    |  \  /    \  \/ /  _ \ /    \  \/ // __ \_  __ \/  ___/  |/  _ \ /    \ 
/    |    \  / /    |    `   \ \     \___(  <_> )   |  \   /\  ___/|  | \/\___ \|  (  <_> )   |  \
\____|__  / / /    /_______  /  \______  /\____/|___|  /\_/  \___  >__|  /____  >__|\____/|___|  /
        \/  \/             \/          \/            \/          \/           \/               \/ 

*/
void initAnalog() {
    	ADMUX |= (1 << REFS0);   // use AVcc as the reference
	ADCSRA |= (1<<ADEN);     // enable analog digital convertor
	ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // set prescale to 128
}

unsigned char readAnalog(unsigned char pin) {
    	ADMUX = (ADMUX & (~(0x7))) | ((pin)& 0x7);
	ADCSRA |= (1<<ADSC);
	while(ADCSRA & (1<<ADSC));
	int ADCval = ADCL;
    	ADCval = (ADCH << 8) + ADCval;
	return ADCval;
}

/*
  ___________________.___  __________                __                      .__   
 /   _____/\______   \   | \______   \_______  _____/  |_  ____   ____  ____ |  |  
 \_____  \  |     ___/   |  |     ___/\_  __ \/  _ \   __\/  _ \_/ ___\/  _ \|  |  
 /        \ |    |   |   |  |    |     |  | \(  <_> )  | (  <_> )  \__(  <_> )  |__
/_______  / |____|   |___|  |____|     |__|   \____/|__|  \____/ \___  >____/|____/
        \/                                                           \/            
*/
void pulsePin(int pin) {
  clearPin(PORTB,pin);
  setPin(PORTB,pin);
}

void sendSPIData(int data) {
	clearPin(PORTB,CS);
	int mask = 1<<8;
	while(mask > 0) {
		if(data & mask) {
			setPin(PORTB,DIO);
		}else {
			clearPin(PORTB,DIO);
		}
		pulsePin(SCK_P);
		mask = mask>> 1;
	}
	setPin(PORTB,CS);
}

void writeLCDCommand(int command) {
	sendSPIData(command & (~(1<<8)));
}

void writeLCDData(int data) {
	sendSPIData(data | (1<<8));
}

/*
________  .__               .__                
\______ \ |__| ____________ |  | _____  ___.__.
 |    |  \|  |/  ___/\____ \|  | \__  \<   |  |
 |    `   \  |\___ \ |  |_> >  |__/ __ \\___  |
/_______  /__/____  >|   __/|____(____  / ____|
        \/        \/ |__|             \/\/     

*/
void initDisplay() {
  //Initalize the display pins as output 
  outputPin(DDRB, RESET);
  outputPin(DDRB, DIO);
  outputPin(DDRB, SCK_P);
  outputPin(DDRB, CS);

  clearPin(PORTB,SCK_P);    // CLK = LOW
  clearPin(PORTB,DIO);    // DIO = LO
  _delay_ms(10);    // 10us delay
  setPin(PORTB,CS);    // CS = HIGH
 _delay_ms(10);    // 10uS Delay

  clearPin(PORTB,RESET);  // RESET = LOW
  _delay_ms(200);	             
  setPin(PORTB,RESET); // RESET = HIGH
  _delay_ms(200);		    // 200ms delay
  setPin(PORTB,SCK_P);   // SCK_P = HIGH
  setPin(PORTB,DIO);   // DIO = HIGH

  writeLCDCommand(DISCTL);	// Display control (0xCA)
  writeLCDData(0x00);		// 12 = 1100 - CL dividing ratio [don't divide] switching period 8H (default)
  writeLCDData(0x20);		// nlines/4 - 1 = 132/4 - 1 = 32 duty
  writeLCDData(0x0a);		// No inversely highlighted lines
  		
  writeLCDCommand(COMSCN);	// common scanning direction (0xBB)
  writeLCDData(0x00);		// 1->68, 132<-69 scan direction
  		
  writeLCDCommand(OSCON);	// internal oscialltor ON (0xD1)
  writeLCDCommand(SLPOUT);	// sleep out (0x94)
  
  writeLCDCommand(VOLCTR);	// electronic volume, this is the contrast/brightness (0x81)
  writeLCDData(40);		// volume (contrast) setting - fine tuning, original (0-63)
  writeLCDData(1);		// internal resistor ratio - coarse adjustment (0-7)
  		
  writeLCDCommand(PWRCTR);	// power ctrl (0x20)
  writeLCDData(0x0F);		// everything on, no external reference resistors
  		
  writeLCDCommand(DISINV);	// invert display mode (0xA7)
  		
  writeLCDCommand(DATCTL);	// data control (0xBC)
  writeLCDData(0x00);		// Inverse page address, reverse rotation column address, column scan-direction	!!! try 0x01
  writeLCDData(0x03);		// normal RGB arrangement
  writeLCDData(0x02);		// 16-bit Grayscale Type A (12-bit color)
  		
  writeLCDCommand(NOP);	// nop (0x25)
  writeLCDCommand(DISON);	// display on (0xAF) */
}

void fillRectangle(int x, int y, int width, int height, int color) {
	int x1 = x;
	int x2=  x+width-1;
	int y2 = y+height+1;
	int i  = (width*height/2);
	int index;
	writeLCDCommand( PASET);
	writeLCDData(y);
	writeLCDData(y2);
	writeLCDCommand(CASET);
	writeLCDData(x1);
	writeLCDData(x2);
	writeLCDCommand(RAMWR);         
	for(index=0; index<i; index++) {
		writeLCDData((color >> 4) & 0xFF); 
		writeLCDData(((color & 0xF) << 4) | ((color >> 8) & 0xF)); 
		writeLCDData(color & 0xFF); 
	}
}

void clearScreen() {
	fillRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, BLACK);
}


int readPulse(int pin) {
	int high = 0;
	int low = 0;
	int count = 0;	
	//wait till low
	while(isSet(PIND,pin)); 
	//loop while it is low 
	while(!isSet(PIND,pin));
	//count the high pulse length
	while(isSet(PIND,pin)) { high++; };
	while(!isSet(PIND,pin)) { low++; };
	count++;
	return high;
};



// ADT Ball
void move(Ball* self,int x, int y) {
	fillRectangle(self->x_pos, self->y_pos, self->width,self->height, BLACK);
	self->x_pos = x;
	self->y_pos = y;
	fillRectangle(self->x_pos, self->y_pos, self->width,self->height, self->color);
};

Ball* createBall(int x,int y,int w,int h) {
	Ball* ball = (Ball*) malloc(sizeof(Ball));
	if(ball == NULL) {
		USART_Transmit('.');
	}else {	
		ball->width = w;
		ball->height = h;
		ball->x_pos = x;
		ball->y_pos = x;
		ball->color = WHITE;
		ball->move = move;
	}
	return ball;
}

//ADT Accelerometer
direction getDirection(Accelerometer* acc) {
	if(readPulse(acc->x_pin) < 7920) return RIGHT;	
	if (readPulse(acc->x_pin)> 9920) return LEFT;		
	if(readPulse(acc->y_pin) < 7920) return UP;	
	if (readPulse(acc->y_pin)> 9920) return DOWN;		
};

int getDetailedDirection() {
	//todo
};

Accelerometer* newAccelerometer(int x_pin,int y_pin) {
	Accelerometer* acc = (Accelerometer*) malloc(sizeof(Accelerometer));
	acc->x_pin = x_pin;
	acc->y_pin = y_pin;
	inputPin(DDRD,x_pin);
	inputPin(DDRD,y_pin);
	acc->getDirection = getDirection;
	acc->getDetailedDirection = getDetailedDirection;
	return acc;	
}
