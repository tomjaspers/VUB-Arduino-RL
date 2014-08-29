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
	   
//CONSTANTS
static const int SIZE = 10;
static const int X_PIN = 2;
static const int Y_PIN = 3;
static const int STEP  = 10; // The stepsize that the user can move the ball (by physically moving the board)
static const int RL_STEP = 10; // The stepsize that the reinforcement learning system can move the ball

// LEARNER PARAMS
static const float ALPHA = 0.1; // Learning rate (rate at which new training data replace previous knowledge)
static const float GAMMA = 0.9; // Discount factor (defines relative values of the immediate vs delayed reward)
static const int EPSILON = 15; // Exploration rate in epsilon-greedy action select (% of random action instead of optimal)
static const int NUM_ACTIONS = 3;

/**
* Due to severe memory restrictions (only 1024 kb memory), we cannot simple create and state-action table for
* every possible combination (e.g., a 13x13x5 array = 3380). We realise that the problem is a symmetrical one, and divide
* the board in to 4 quadrants: A, B, C, and D (see drawing).
* 
* Given the true x_pos and y_pos of the ball (range [0,121]), we convert this to a x and y ([0,6]) to indicate the state.
* 
* For every state the ball is in, the reinforcement learner has to choose one of 3 actions:
* 	Neutral (stay in position),
* 	Away from X-axis,
* 	Away from Y-axis
* 
* Actions to move away from the axis are dependant on the quadrant that the ball is in. 
* Moving away from X-axis is the same in quadrants A and B, but mirrored in quadrants C and D.
* Moving away from Y-axis is the same in quadrants A and C, but mirrored in quadrants B and D.
* 
* Example:
* The ball is in position (110,30) thus the state is (1,3). We expect the optimal action to be to move away from
* the Y-axis. Because we are in quadrant B, moving away from the Y-axis means reducing the x_pos of the ball.
* Reducing the x_pos of the ball is equal to direction RIGHT (see function moveBall).
* 
*    0  1  2  3  4  5  6  5  4  3  2  1  0 
* 0 +------------------+-----------------+
* 1 |                  |                 |
* 2 |                  |                 |
* 3 |        A         |        B     o  | // the o is the example ball position
* 4 |                  |                 |
* 5 |                  |                 |
* 6 +------------------------------------+
* 5 |                  |                 |
* 4 |                  |                 |
* 3 |        C         |        D        |
* 2 |                  |                 |
* 1 |                  |                 |
* 0 +------------------+-----------------+
*
*/ 

void initializeBoard() {
	USART_Init(MYUBRR);
	initDisplay();
	clearScreen();
}


/* Moves the ball (and redraws) in a given direction for a given stepsize */
void moveBall(Ball *b, direction d, int step){
	switch(d) {
		case LEFT:  sendMessage(b, move, b->x_pos+step, b->y_pos); break;
		case RIGHT: sendMessage(b, move, b->x_pos-step, b->y_pos); break;
		case DOWN:  sendMessage(b, move, b->x_pos, b->y_pos-step); break;
		case UP:    sendMessage(b, move, b->x_pos, b->y_pos+step); break;
		default: break;
	}
}

/* Converts the true position of the ball to a state (x,y), in a manner as described above */
void getState(Ball *b, int *x, int *y){
	// The position of the ball is given by x,y coordinates in [0,131],
	// However the ball is 10 pixels, so it will only move in a range of [0, 121] (because the screen is bounded)
	
	// The first step is to convert this to a [0,12] range		
	*x = b->x_pos / 10;
	*y = b->y_pos / 10;
	
	// Sanity checks (TODO: check if sanity checks can be omitted)		
	if (*x < 0)  *x = 0;
	if (*x > 12) *x = 12;
	if (*y < 0)  *y = 0;
	if (*y > 12) *y = 12;
	
	// Map x,y to conform with the drawing (see in comments up) by counting as 0,1,..,5,6,5,..1,0 
	*x = (*x > 6) ? (12 - *x) : *x; 
	*y = (*y > 6) ? (12 - *y) : *y;
}

/** 
 * Follows the epsilon-greedy action selection method: 
 * With a probability of 1-epsilon, it will choose the action with the highest Q-value (= the optimal action).
 * With a probability of epsilon, it will choose a random action
 * ==> This shows the trade-off that Q-learning has to make between exploitation and exploration
 *
 * NB: we return here the INDEX of the optimal action. The actual ACTION is dependent of the quadrant of the ball.
 *
 */
int selectActionIndex(float qvalues[], int epsilon){
	int action_idx;
	if ((rand() % 101) < EPSILON){ // Choose a random action with a probability of epsilon
		action_idx = rand() % NUM_ACTIONS;
	} else { // Choose the best action (= max Q-value) with probability 1-epsilon
		// We try to select the action index with the highest Q-value.
		// By starting with action_idx = 0, and using > (instead of >=), we bias towards the 0 action (neutral)
		action_idx = 0;
		int i;
		for (i = 1; i < NUM_ACTIONS; i++) {
			if (qvalues[i] > qvalues[action_idx]) {
				action_idx = i;
			}
		}
	}
	return action_idx;
}

/**
 * Returns an action for the given action_idx, based on the position of the ball.
 * Remember that the action_idx stands for: neutral, move away from x-axis, move away from y-axis.
 * And remember that "moving away from an axis" is dependent on which quadrant the ball is positioned. 
 */
direction getAction(Ball *b, int action_idx){
	direction action;
	// Map the action_idx to an action asif we are in the quadrant A:
	switch(action_idx){
		case 0: action = NEUTRAL; break; // stay in position
		case 1: action = LEFT;	  break; // move away from x-axis
		case 2: action = UP; 	  break; // move away from y-axis
	}
	// Check if we are in quadrant B / D
	if (b->x_pos > 61 && action == LEFT){
		action = RIGHT;
	}
	// Check if we are in quadrant C / D
	if (b->y_pos > 61 && action == UP){
		action = DOWN;
	}
	
	return action;
}

/**
 * Returns a reward for the given state of the ball, making the center of the screen the goal state (+10),
 * the bounds of the screen very bad (-100), and every other area -1.
 * The reward structure is open for interpretation and can be toyed with to achieve different goals.
 *
 * Possible ideas: make reward based on how far the ball is away from an edge
 */
int getReward(int x, int y){
	int reward = -1;
	if (x == 6 && y == 6){
		reward = 10;
	} else if (x == 0  || y == 0 || x == 12 || y == 12) {
   		reward = -100;
   	}
	return reward;
}


int  main() {
	initializeBoard();
	// Seed rand: if nothing is connected to the pins, they can pick up environmental noise (= ~ random)
	srand(readAnalog(0)); 
	//Create ball in the center of the screen
	Ball* b = createBall((SCREEN_WIDTH/2)-SIZE/2,(SCREEN_HEIGHT/2)-SIZE/2, SIZE,SIZE);
	// Draw the ball in its initial position 
	fillRectangle(b->x_pos, b->y_pos, b->width, b->height, b->color);
	//Set the ball to be white
	b->color = WHITE;
	//Create an accelerometer connected to the X and Y_PIN 
	Accelerometer* acc = newAccelerometer(X_PIN,Y_PIN); 
		
	// Initialize our Q-values table: (7x7x3 floats) x 4 bytes = 588 bytes
	float qvalues[7][7][3] = {};
	
	while(1) {
		//Ask the accelerometer for the direction
		direction d = sendMessage(acc,getDirection);
		moveBall(b, d, STEP);
		_delay_ms(150);	
		
		// Get the current state of the ball (defined by 2 ints, x and y, in a range of [0, 6])
		int x, y;
		getState(b, &x, &y);
		
		// Select an action for the current state, using epsilon-Greedy action selection
		// We have to keep these 2 separate: the action index will get used to update the Q-value,
		// While the actual action is dependent on the quadrant, and for actually moving the ball
		int action_idx = selectActionIndex(qvalues[x][y], EPSILON);
		direction action = getAction(b, action_idx);
	
		// Perform the action
		moveBall(b, action, RL_STEP);
		
		// Get the resulting state of the ball (defined by 2 ints, x and y, in a range of [0, 6])
		int new_x, new_y;
		getState(b, &new_x, &new_y);
			
		// Get the best action index for our new state. Note that we use epsilon = 0 here, because
		// we want te best possible action without exploration (part of the update rule, see theory)
		int new_action_idx = selectActionIndex(qvalues[new_x][new_y], 0);
		
		// Get the reward and reposition if needed
		int reward = getReward(new_x, new_y);
		
		// Update our q-values using the Q-learning update rule
		qvalues[x][y][action_idx] += ALPHA * (reward + GAMMA * qvalues[new_x][new_y][new_action_idx] - qvalues[x][y][action_idx]);
				
		// If our ball somehow crossed the screen bounds, we will reset it to the center position
		if (b->y_pos > SCREEN_HEIGHT-SIZE || b->y_pos < 0 || b->x_pos > SCREEN_WIDTH-SIZE  || b->x_pos < 0) {
			sendMessage(b, move, (SCREEN_WIDTH/2)-SIZE/2,(SCREEN_HEIGHT/2)-SIZE/2);
		}
		
		_delay_ms(75);
	}
	//cleanup
	free(b);
	free(acc);
}
