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
#include <stdio.h>
#include <stdlib.h>	 
	   
// Function prototypes
int randint();

//CONSTANTS
static const int SIZE = 10;
static const int X_PIN = 2;
static const int Y_PIN = 3;
static const int STEP  = 10;
static const int RL_STEP = 10;

// LEARNER PARAMS
static const float ALPHA = 0.1; // Learning rate (rate at which new training samples replace previous knowledge)
static const float GAMMA = 0.9; // Discount factor (defines relative values of the immediate vs delayed reward)
static const int EPSILON = 15; // Exploration rate in epsilon-greedy action select (% of random action instead of optimal)

/*
Drawing of the board divided in its 4 quadrants we use for our states in Q-learning (due to memory restrictions)

E.g.: actions in quadrant B, state (3,1) are the same as in quadrant A for the same state, mirrored over the X-axis

   0  1  2  3  4  5  6  5  4  3  2  1  0 
0 +------------------+-----------------+
1 |                  |                 |
2 |                  |                 |
3 |        A         |        B        |
4 |                  |                 |
5 |                  |                 |
6 +------------------------------------+
5 |                  |                 |
4 |                  |                 |
3 |        C         |        D        |
2 |                  |                 |
1 |                  |                 |
0 +------------------+-----------------+
*
*/ 

void initializeBoard() {
	USART_Init(MYUBRR);
	initDisplay();
	clearScreen();
}

void moveBall(Ball *b, direction d, int step){
	switch(d) {
		case LEFT:  sendMessage(b, move, b->x_pos+step, b->y_pos); break;
		case RIGHT: sendMessage(b, move, b->x_pos-step, b->y_pos); break;
		case DOWN:  sendMessage(b, move, b->x_pos, b->y_pos-step); break;
		case UP:    sendMessage(b, move, b->x_pos, b->y_pos+step); break;
		default: break;
	}
}

int  main() {	
	srand(0.314);
	initializeBoard();
	//Create ball in the center of the screen
	Ball* b = createBall((SCREEN_WIDTH/2)-SIZE/2,(SCREEN_HEIGHT/2)-SIZE/2, SIZE,SIZE);
	// Draw the ball in its initial position 
	fillRectangle(b->x_pos, b->y_pos, b->width, b->height, b->color);
	//Set the ball to be white
	b->color = WHITE;
	//Create an accelerometer connected to the X and Y_PIN 
	Accelerometer* acc = newAccelerometer(X_PIN,Y_PIN); 
		
	// Initialize our Q-values table
	// We do not have enough memory for a simple 13 x, 13y, 5 actions mapping (13x13x5 = 845 floats = 3380 bytes >>> 1024)
	// Note that even with using ints for the float math, it would be impossible (sizeof(int) = 2)
	float qvalues[7][7][3] = {}; // (7x7x4 floats) x 4 bytes = 588 bytes
	int num_actions = 3; // neutral, away from x-axis, away from y-axis
	
	while(1) {
		//Ask the accelerometer for the direction
		direction d = sendMessage(acc,getDirection);
		moveBall(b, d, STEP);
		_delay_ms(150);	
		
		// Get the current state (x,y)
		int x = b->x_pos / 10;
		int y = b->y_pos / 10;
		if (x < 0)	x = 0;
		if (x > 12)	x = 12;
		if (y < 0)	y = 0;
		if (y > 12) y = 12;
		x = (x > 6) ? (12 - x) : x; 		// allow to count states as 0,1,..,5,6,5,..1,0 (see drawing)
		y = (y > 6) ? (12 - y) : y;
		
		// Choose an action following epsilon-greedy
		int action_idx;
		if ((rand() % 101) < EPSILON){ 
			// TODO: achieve better randomness using randint(n) -> it will take memory !
			// Choose a random action with a probability of epsilon
			action_idx = rand() % num_actions;
		} else {
			// Choose the best action (= max Q-value) with probability 1-epsilon
			action_idx = 0;
			int i;
			for (i = 1; i < num_actions; i++) {
				if (qvalues[x][y][i] > qvalues[x][y][action_idx]) {
					action_idx = i;
				}
			}
		}
		// We map the action index on an actual action
		direction action;
		switch(action_idx){
			case 0: action = NEUTRAL; break; // stay in position
			case 1: action = LEFT;	  break; // move away from x-axis
			case 2: action = UP; 	  break; //
		}
		// We have to map the inverse direction depending on the quadrant we are in (default A)
		// (see drawing)
		if (b->x_pos > 61 && action == LEFT){ // Quadrant B / D
			action = RIGHT;
		}
		if (b->y_pos > 61 && action == UP){ // Quadrant C / D
			action = DOWN;
		}
		
		// Perform the action
		moveBall(b, action, RL_STEP);
		
		// Get the new state (x,y)
		int new_x = b->x_pos / 10;
		int new_y = b->y_pos / 10;
		if (new_x < 0)	new_x = 0;
		if (new_x > 12)	new_x = 12;
		if (new_y < 0)	new_y = 0;
		if (new_y > 12) new_y = 12;
		new_x = (new_x > 6) ? (12 - new_x) : new_x; 	// allow to count states as 0,1,..,5,6,5,..1,0 (see drawing)
		new_y = (new_y > 6) ? (12 - new_y) : new_y;
			
		// Get the qvalue for the best possible action in for the new state (call e-greedy with e=0)
		int new_action_idx = 0;
		int i; 
		for (i = 0; i< num_actions; i++) { 
			if (qvalues[new_x][new_y][i] > qvalues[new_x][new_y][new_action_idx]){
				new_action_idx = i;
			}
		}
		
		// Get the reward and reposition if needed
		// TODO : make the reward based on how far the ball is from the wall ?
		int reward = -1;
		if (new_x == 6 && new_y == 6){
			reward = 10;
		} else if (new_x == 0  || new_y == 0 || new_x == 12 || new_y == 12) {
	   		reward = -100;
	   	}
		
		// Update our q-values using the Q-learning update rule
		qvalues[x][y][action_idx] += ALPHA * (reward + GAMMA	 * qvalues[new_x][new_y][new_action_idx] -  qvalues[x][y][action_idx]);
				
		// Throw our ball back to reset position if it crossed the bounds	
		if (b->y_pos > SCREEN_HEIGHT-SIZE || b->y_pos < 0 || b->x_pos > SCREEN_WIDTH-SIZE  || b->x_pos < 0) {
			sendMessage(b, move, (SCREEN_WIDTH/2)-SIZE/2,(SCREEN_HEIGHT/2)-SIZE/2);
		}
		
		_delay_ms(75);
	}
	//cleanup
	free(b);
	free(acc);
}

/* Returns a uniformly random integer in the range [0, n) */
int randint(int n) {	
  if ((n - 1) == RAND_MAX) {
    return rand();
  } else {
    // Chop off all of the values that would cause skew...
    long end = RAND_MAX / n; // truncate skew
    //assert (end > 0L);
    end *= n;

    // ... and ignore results from rand() than fall above that limit.
    // (Worst case the loop condition should succeed 50% of the time, so we can
    // expect to bail out of this loop pretty quickly.)
    int r;
    while ((r = rand()) >= end);
    return r % n;
  }
}
