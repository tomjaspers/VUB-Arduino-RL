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

/**
 * @file lib.h
 * @author Christophe Scholliers
 * @date 21 August 2012
 * @brief File containing library code for the different functionalities of the ATMEGA168 microcontroller.
*/

#ifndef LIB_H 
#define LIB_H 

#include <inttypes.h>

typedef volatile unsigned char  int8;

//EEPROM
#define EECR  (*((volatile unsigned char*)(0x3F)))
#define EEPE  1 
#define EEMPE 2
#define EERE  0
#define EEAR  (*((volatile uint16_t*)(0x41)))
#define EEDR  (*((volatile unsigned char*)(0x40))) 
#define AVR_S (*((volatile unsigned char*)(0x5F)))

/**
* @brief Writes to eeprom memory
* @param param1 The address in memory to write
* @param param2 The data to write (8bit)
* @return void
*/
void EEPROM_write(unsigned int uiAddress, unsigned char ucData);

/**
* @brief Reads a value from eeprom memory
* @param param1 The adress to read from
* @return The 8 bit value read from memory
*/
unsigned char EEPROM_read (unsigned int uiAddress);

//Pin configurations
#define DDRB   *((volatile unsigned char*)(0x24))
#define DDRD   *((volatile unsigned char*)(0x2A))
#define PORTB  *((volatile unsigned char*)(0x25))
#define PORTD  *((volatile unsigned char*)(0x28))
#define PIND   *((volatile unsigned char*)(0x29))
#define PINB   *((volatile unsigned char*)(0x23))
#define BLINK_DELAY_MS 1000


#define OUTPUT 1
/**
* @brief Toggles a pin for a number of times
* @param param1 the amount of times to togle the pin
* @param param2 the pin number
* @return void
*/
void blink(int x,int pin);

#define sbi(addr,bit) ((addr) |=  (1<<bit))
#define cbi(addr,bit) ((addr) &= ~(1<<bit))
#define outputPin(ddr,pin)  ((ddr)  |=  (1<<pin))
#define inputPin(ddr,pin) ((ddr)  &= ~(1<<pin))
#define setPin(addr,bit) ((addr) |=  (1<<bit))
#define clearPin(addr,bit) ((addr) &= ~(1<<bit))
#define isSet(addr,bit) ((addr) &  (1<<bit))

//USART
#define FOSC 16000000 // Clock Speed 
#define BAUD 9600
#define MYUBRR FOSC/16/BAUD-1
#define UBRR0H *((volatile unsigned char*)(0xC5))
#define UBRR0L *((volatile unsigned char*)(0xC4))
#define UCSR0B *((volatile unsigned char*)(0xC1))
#define RXEN0  4
#define TXEN0  3
#define UCSR0C *((volatile unsigned char*)(0xC2))
#define USBS0  3
#define UCSZ00 1
#define UCSR0A *((volatile unsigned char*)(0xC0))
#define UDRE0  5
#define UDR0   *((volatile unsigned char*)(0xC6))

/**
* @brief Initializes the USART 
*/
void USART_Init( unsigned int ubrr);
void USART_Transmit( unsigned char data);
void printNumber(int x);

//ADC
#define ADMUX  *((volatile unsigned char*)(0x7C))
#define ADCSRA *((volatile unsigned char*)(0x7A))
#define ADCSRB *((volatile unsigned char*)(0x7B))
#define ADCH   *((volatile unsigned char*)(0x79))
#define ADCL   *((volatile unsigned char*)(0x78))

#define ADSC  6
#define REFS1 7
#define REFS0 6
#define ADC0  0
#define ADEN  7
#define ADLAR 5

#define ADPS2 2
#define	ADPS1 1
#define ADPS0 0 


//int ADCsingleREAD(uint8_t adctouse);
void initAnalog();
unsigned char readAnalog(unsigned char x);

//Pin configurations for the display
#define SCK_P  1
#define CS     2
#define DIO    3
#define RESET  4

//ESPON message constants
#define DISCTL  0xCA
#define DATCTL  0xBC
#define COMSCN  0xBB
#define OSCON   0xD1
#define SLPOUT  0x94
#define PWRCTR  0x20
#define DISINV  0xA7
#define VOLCTR  0x81
#define NOP     0x25
#define DISON   0xAF
#define PASET   0x75
#define CASET   0x15
#define RAMWR   0x5C

void initDisplay();
void fillRectangle(int x, int y, int width, int height, int color);
void clearScreen();


#define SCREEN_WIDTH 131
#define SCREEN_HEIGHT 131
#define BLACK  0
#define WHITE  0xFFF


int readPulse(int pin);


#define sendMessage(s,methodName,...) s->methodName(s, ##__VA_ARGS__)

typedef struct ball {
	int x_pos;
	int y_pos;
	int width;
	int height;
	int color;
	void(*move)(struct ball*,int x, int y);
} Ball;

Ball* createBall(int x, int y,int w,int h);


typedef enum { LEFT,RIGHT,DOWN,UP } direction; 

typedef struct  acc {
	int x_pin;
	int y_pin;
	direction (*getDirection)(struct acc*);
	int (*getDetailedDirection) (struct acc*);
} Accelerometer;

Accelerometer* newAccelerometer();

#endif
