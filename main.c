#include <pic32mx.h>
#include <stdint.h>
#include <stdlib.h>  
#include <stdio.h>
#include "header.h"
#include "data.h"

//Defining values and global variables
#define POSX 48
#define POSY 9
#define TMR2PERIOD ((80000000 / 256) / 10)
int timeoutcount = 0;
uint8_t* SHAPES[7] = {J, L, S, Z, O, I, T};
int shape_pos_x = POSX;
int shape_pos_y = POSY;
int score = 0;
int fallSpeed = 10;
int fallCounter = 0;
int ShapeStatus = 1; 
uint8_t* nextShape = 0;
int SCORES[8] = {0, 0, 0, 0, 0, 0, 0, 0};
int GAME_STATE = 0;
int SELECT = 0;
int btnIsPressed = 0;


//Function to draw a block on the screen
void drawBLOCK(void){
	int r, c;
	for(r = 0; r<10; r++){
		for(c=0; c<20; c++){
			if(BLOCKS[r][c] == 1){ //Loop over BLOCKS and draw a block if it is a 1.
				displayBlock((c*3)+1, (r*3)+1);
			}
		}
	}
}

//Function to reset the BLOCKS array for a new game
void clearBlocks(void){
	int r, c;
	for(r=0; r<10; r++){
		for(c=0; c<20; c++){
			BLOCKS[r][c] = 0;  //Sets all BLOCKS to 0
		}
	}
}

// Function to initilize the buttons
void btnsinit(void){
    TRISF |= 0x2;
    TRISD |= 0xe0;
    return;
}

//Function to get the buttons status
int getbtns(void){ 
    int value1 = (PORTF & 0x2) >> 1;  //00000001
	int value2 = (PORTD & 0xe0) >> 4; //00001110
    return value1 | value2;		      //00001111
}

//Fuction to check collision with ground, roof and bottom of the play plane
int checkLimits(uint8_t* shape, int rotation, int choice){
	int r,c;
    shape += rotation * 16;

	//loops over a shape
    for(r = 0; r<4; r++){
        for(c=0; c<4; c++){
            if(*shape == 1){
				int posY = shape_pos_y;
				int posX = shape_pos_x;
				
				//If we want to check bottom wall (landscape) of screen, add 3 to Y and see if it is outside
				if(choice == 0){ 
					posY += 3;
					if((posY/3)+r >= 10){
						return 0;
					}
				//If we want to check roof wall (landscape) of screen, take 3 of Y and see if it is outside
				}else if(choice == 1){  // check roof
					posY -= 3;
					if((posY/3)+r < 0){
						return 0;
					}
				//If we want to check ground (where shapes is falling to), take 3 of Y and see if it is outside
				}else{
					posX -= 3;
					if((posX/3)+c < 0){
						return 0;
					}
				}
            }
            shape++;
        }
    }
	return 1;
}

//Function to check if rotation is available
int checkRotationCollision(uint8_t* shape, int rotation){

    int r, c;
    shape += rotation * 16;
	int counter = 0;
	int posx = shape_pos_x;
	int posy = shape_pos_y;
	//while loop as long as a shape is outside/over another shape
    while(1){
		counter++;
		int valid = 1;	
		//loop over shape
        for(r = 0; r<4; r++){
            for(c=0; c<4; c++){
                if(*shape == 1){
					//If shape is outside then subtract 3 of y
					if((shape_pos_y/3)+r >= 10){
						shape_pos_y-=3;
						valid = 0;
						break;
					}
					//If shape is outside then add 3 to y
					if((shape_pos_y/3)+r < 0){
						shape_pos_y+=3;
						valid = 0;
						break;
					}
					//If shape is outside then add 3 to x
					if((shape_pos_x/3)+c < 0){
						shape_pos_x+=3;
						valid = 0;
						break;
					}
					//If shape is over another shape
					if(BLOCKS[(shape_pos_y/3)+r][(shape_pos_x/3)+c] == 1){
						//add or subtract based on where the shape is over the another
						if(r<2){
							shape_pos_y+=3;
						}
						if(r>1){
							shape_pos_y-=3;
						}
						if(c<2){
							shape_pos_x+=3;
						}
						valid = 0;
						break;
					}
                }
                shape++;
            }
			if(valid == 0){
				break;
			}
        }
		if(valid == 1){
			return rotation;
		}
		//if the shape commute (pendlar) back and forth (the space betwwen two shapes är tight and cant rotate the shape) then dont rotate
		if(counter > 3){
			shape_pos_x = posx;
			shape_pos_y = posy;
			if(rotation == 0){
				return 3;
			}else{
				return rotation -1;
			}
		}
    }   
	
}



//Function to increase the rotation by 1
int rotateShape(int rotation){
    if(rotation == 3){
        return 0;
    }else{
        return rotation + 1;
    }
}

//Initilize the timers
void timeInit(void){
	T2CON = 0x70; // 0111 000 is for 1:256 scale
    PR2 = TMR2PERIOD; // for a timeout value of 100 ms
    TMR2 = 0; // reset the counter
    T2CONSET = 0x8000; // start the timer
}

//Check if shape collides with another shape
int checkBlockCollision(uint8_t* shape, int rotation, int choice){
		int r, c;
		shape += rotation * 16;
		//loops over the shape
		for(r = 0; r<4; r++){
			for(c=0; c<4; c++){
				if(*shape == 1){
					//Choice == 0 we want to check if the shape is over another shape
					if(choice == 0 && BLOCKS[(shape_pos_y/3)+r][(shape_pos_x/3)+c-1] == 1){
						return 1;
					}
					//Choice == 1 we want to check if the shape is to the left of another shape
					if(choice == 1 && BLOCKS[(shape_pos_y/3)+r+1][(shape_pos_x/3)+c] == 1){ 
						return 1;
					}
					//Choice == 2 we want to check if the shape is to the right of another shape
					if(choice == 2 && BLOCKS[(shape_pos_y/3)+r-1][(shape_pos_x/3)+c] == 1){
						return 1;
					}
				}
				shape++;
			}
		}
		return 0;
}

//Function to draw the select ball on main meny screen
void drawBall(int select){
	int r, c;
		for(r = 0; r<5; r++){
			for(c = 0; c<5; c++){
				if(BALL[r*5 + c] == 1){
					if(select == 0){
						displayPixel(75+c, r);
					}else{
						displayPixel(57+c, r);
					}
				}	
			}
		}
}

//Function to copy over the screen into the dispbuffer
void copyDisp(uint8_t* screen){
	int i;
	for(i = 0; i<DISPLAY_SIZE; i++){
		dispBuffer[i] = *screen;
		screen++;
	}
}

//Function to save the fallen shape to the BLOCKS array
void saveShape(uint8_t* shape, int rotation){
	int r,c;
	shape += rotation * 16;
	for(r = 0; r<4; r++){
        for(c=0; c<4; c++){
			if(*shape == 1){
				BLOCKS[(shape_pos_y/3)+r][(shape_pos_x/3)+c] = 1;
			}
			shape++;
		}
	}
}

//Function to display the score
void displayScore (int score, int x, int y){
	int digit;
	int pos = y;  //29 och x = 100
	if(score == 0){
		displayNumber(numbers, 0, x, pos);
	}else{
		while (score > 0){
		digit = score % 10;
		displayNumber(numbers, digit, x, pos);
		score = score / 10;
		pos -= 4;
		}
	}
}

//Function to remove a completed row
void removeRow(int row){
	int r,c;
    for(c=row+1; c<20; c++){
         for(r=0; r<10; r++){
            BLOCKS[r][c-1] = BLOCKS[r][c];
        }
    }
}

//Function to check over completed rows
void checkRowComplete(){
	int r,c;
	int counter = 0;
    for(c=0; c<20; c++){
        for(r=0; r<10; r++){
            if(BLOCKS[r][c]==1){
                counter++;
            }	
        }
		if(counter == 10){
			removeRow(c);
			score += 100;
			c=c-1;
		}
		counter = 0;	
    }

}

//Function to init the game
void gameInit(){
	timeInit();
	SPIInit();
	btnsinit();
	displayInit();
}

//Function to return a random number (0 to 6)
int getRandomNumber(){
	return TMR2%7;
}


//Function to check if it is game over
int checkGameOver (){
	if(BLOCKS[4][19] == 1 && BLOCKS[4][19] == 1){
		return 1;
	}
	return 0;
}

//Function to display all the highscores 
void displayHighScores(){
	int i;
	for (i=0; i<8; i++){
		displayNumber(numbers, i+1, 95 - (i*10), 1);
		displayScore(SCORES[i], 95 - (i*10), 27);
	}
}

//Function to save a score if it is a new highscore
void saveScore(int score){
	int i, j;
	for(i = 0; i<8; i++){
		if(score > SCORES[i]){
			for(j=7; j>i; j--){
				SCORES[j] = SCORES[j-1];
			}
			SCORES[i] = score;
			break;
		}
		
	}
}




int main() {
	
	gameInit();
	int randomInt;
	int rotation = 0;
	uint8_t* shape;
	nextShape = SHAPES[getRandomNumber()];
	while(1){

		int btns = getbtns();
		//resets the timers flag
		if (IFS(0) & 0x100) {
        	timeoutcount++; 
        	IFSCLR(0) = 0x100; 
    	}
		
		//GAME_STATE = 0 is the main menu screen
		if(GAME_STATE == 0){
			copyDisp(main_screen);
			drawBall(SELECT%2);
			//when this btn is pressed you can select between start and highscore
			if(btns & 0x8 && btnIsPressed == 0){
				btnIsPressed = 1;
				SELECT++;
				displayUpdate(dispBuffer);
			}
			//prevent the btn to be pressed many time under short period (debounce)
			if(!(btns & 0x8) && btnIsPressed == 1){
				btnIsPressed = 0;
			}
			//This btn is to select the start or highscore
			if(btns & 0x4){
				//If start is selected, then start the game
				if(SELECT%2 == 0){
					randomInt = getRandomNumber();
					shape = SHAPES[randomInt];
					rotation = 0;
					shape_pos_x = POSX;
					shape_pos_y = POSY;
					clearBlocks();
					score = 0;
					GAME_STATE = 1;
				//else is showing the highscore for about 3 seconds
				}else{
					copyDisp(high_score_screen);
					displayHighScores();
					displayUpdate(dispBuffer);
					sleep(30000000);
				}
				
			}
			//Resents the counter
			if(timeoutcount == 1){
				timeoutcount = 0;
				displayUpdate(dispBuffer);
			}
			
		//Here is the actual game logik
		}else if(GAME_STATE == 1 && timeoutcount == 1){
			clearDisplay();
			copyDisp(game_screen);
			drawNextShape(nextShape);
			drawBLOCK();
			fallCounter++;
			displayScore(score, 100, 29);
			//shapeStatus = 1 means the shape is activ and falling
			if(ShapeStatus == 1){
				//Checking ground collision
				if(checkLimits(shape, rotation, 2)){
					//Checking on top of other shape collision
					if(!checkBlockCollision(shape, rotation, 0)){
						
						//For animating the falling shape
						if(fallSpeed>3){
							if(fallCounter == fallSpeed-2){
							shape_pos_x = shape_pos_x - 1;	
							}
							//For animating the falling shape
							if(fallCounter == fallSpeed-1){
								shape_pos_x = shape_pos_x - 1;
							}	
							//For animating the falling shape
							if(fallCounter == fallSpeed){
								shape_pos_x = shape_pos_x - 1;
								fallCounter = 0;
							}
						}else{
							if(fallCounter == fallSpeed-1){
								shape_pos_x = shape_pos_x - 2;
								fallCounter = 0;
							}
							if(fallCounter == fallSpeed){
								shape_pos_x = shape_pos_x - 1;
								fallCounter = 0;
							}
						}		

					}else{
						ShapeStatus = 0;
						fallCounter = 0;
					}
				}else{
					ShapeStatus = 0;
					fallCounter = 0;
				}
				

				//Move to the left
				if(btns & 0x8){
					//Checks if the move is valid
					if(checkLimits(shape, rotation, 1)){
						if(!checkBlockCollision(shape, rotation, 2)){
							shape_pos_y = shape_pos_y - 3;
						}
					}
				}
				//Move to the right
				if(btns & 0x4){
					//Checks if the move is valid
					if(checkLimits(shape, rotation, 0)){
						if(!checkBlockCollision(shape, rotation, 1)){
							shape_pos_y = shape_pos_y + 3;
						}
					}
				}
				//Move down
				if(btns & 0x2){
					//Checks if the move is valid
					if(checkLimits(shape, rotation, 2)){
						if(!checkBlockCollision(shape, rotation, 0)){
							shape_pos_x = shape_pos_x - 3;
							score += 1;
						}else{
							ShapeStatus = 0;
							fallCounter = 0;
						}
						
					}
				}
				//Rotate the shape
				if(btns & 0x1 && btnIsPressed == 0){
					//Checks if the rotation is valid
					rotation = checkRotationCollision(shape, rotateShape(rotation));
					btnIsPressed = 1;
				}
				//prevent debounce
				if(!(btns & 0x1) && btnIsPressed == 1){
					btnIsPressed = 0;
				}
			
			//If the shape has fallen and inactive
			}else if(ShapeStatus == 0){
				saveShape(shape, rotation);
				checkRowComplete();
				//For speeding up the game every 10000 score
				if(score >= 8000){
					fallSpeed = 2;
				}else{
					fallSpeed = 10;
					fallSpeed -= score/1000;
				}
				//Checks if the game is over and return to the main menu
				if(checkGameOver()){
					sleep(15000000);
					copyDisp(game_over_screen);
					displayScore(score, 42, 25);
					displayUpdate(dispBuffer);
					sleep(35000000);
					fallSpeed = 10;
					GAME_STATE = 0;
					saveScore(score);
				}
				shape = nextShape;
				nextShape = SHAPES[getRandomNumber()];
				rotation = 0;
				
				ShapeStatus = 1;
				shape_pos_x = POSX;
				shape_pos_y = POSY;
				displayUpdate(dispBuffer);
			}
			
			//Draw the shape
			drawShape(shape ,shape_pos_x, shape_pos_y, rotation);
			
			timeoutcount = 0;	
			//updating the screen	
			displayUpdate(dispBuffer);
			
		}
		
	}

}
