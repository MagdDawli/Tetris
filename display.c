#include <pic32mx.h>
#include "header.h"


#define COMMAND_MODE (PORTFCLR = 0x10)
#define DATA_MODE (PORTFSET = 0x10)

#define DISPLAY_ACTIVATE_RESET (PORTGCLR = 0x200)
#define DISPLAY_DO_NOT_RESET (PORTGSET = 0x200)

#define TURN_VDD_ON (PORTFCLR = 0x40)
#define TURN_VDD_OFF (PORTFSET = 0x40)

#define TURN_VBAT_ON (PORTFCLR = 0x20)
#define TURN_VBAT_OFF (PORTFSET = 0x20)


//Delay function
void sleep(int cycles) {
	int i;
	for(i = cycles; i > 0; i--);
}

//clears the displaybuffer
void clearDisplay(void){
	int i;
	for (i=0; i<DISPLAY_SIZE; i++){
		dispBuffer[i] = 0;
	}
}

//Send a pixel to the displaybuffer
void displayPixel(int x, int y) {
	if(x<128 && y<32 && x>=0 && y>=0) {   //Check display limits

        int page = y / 8;             //Calculate display page
		int yInPage = y % 8;          //Calculate which y pixel on the page
		int arrayPos = page*128 + x;  //Calculate the dispbuffer array position

		/* OR the pixel with current value in the column.*/
		dispBuffer[arrayPos] = dispBuffer[arrayPos] | (0x1 << yInPage);
	}
}

//Send CMD/DATA to display via SPI
uint8_t sendSPI(uint8_t byteVal){
	uint8_t byteReturn;
	while ((SPI2STAT & PIC32_SPISTAT_SPITBE) == 0);
	SPI2BUF = byteVal;
	while ((SPI2STAT & PIC32_SPISTAT_SPIRBF) == 0);	
	byteReturn = SPI2BUF;
	return byteReturn;
}

//Send a page to diplay memory
void displaySendPage(int numBytes, uint8_t * byteBuffer){
	int i;
	uint8_t byteTemp;
	
	/* Write/Read the data */
	for (i = 0; i < numBytes; i++) {
		
		/* Wait for transmitter to be ready */
		while ((SPI2STAT & PIC32_SPISTAT_SPITBE) == 0);

		/* Write the next transmit byte. */
		SPI2BUF = *byteBuffer++;
		
		/* Wait for receive byte. */
		while ((SPI2STAT & PIC32_SPISTAT_SPIRBF) == 0);
		byteTemp = SPI2BUF;
	}
}

//Send the displaybuffer to display memory
void displayUpdate(uint8_t * byteBuffer)
{
	int i;
	
	for (i = 0; i < PAGES; i++) {
		COMMAND_MODE;

		/* Set the page address */
		sendSPI(0x22); //Set page command
		sendSPI(i); //page number
		
		/* Start at the left column */
		sendSPI(0x00); //set low nybble of column
		sendSPI(0x10); //set high nybble of column
		DATA_MODE;
		
		/* Copy this memory page of display data. */
		displaySendPage(COLUMNS, byteBuffer);
		byteBuffer += COLUMNS;
	}
}

//Function to turn the display on, (from the reference manual)
void displayInit(void) {
    COMMAND_MODE;
    sleep(10);
	TURN_VDD_ON;
    sleep(1000000);

    sendSPI(0xAE);
	DISPLAY_ACTIVATE_RESET;
    sleep(10);
	DISPLAY_DO_NOT_RESET;
    sleep(10);

    sendSPI(0x8D);
    sendSPI(0x14);

    sendSPI(0xD9);
    sendSPI(0xF1);

	TURN_VBAT_ON;
    sleep(10000000);

    sendSPI(0xA1);
    sendSPI(0xC8);

    sendSPI(0xDA);
    sendSPI(0x20);

	sendSPI(0x20);   // Set addressing mode command
	sendSPI(0x0);    // Change it to Horizontal addressing mode

	
    sendSPI(0xAF);  //Turn on Display
	sleep(100);
	DATA_MODE;
}

//Function to initilize the SPI
//Function from github (hello display project)
void SPIInit(void){
    /* Output pins for display signals */
    SYSKEY = 0xAA996655;  /* Unlock OSCCON, step 1 */
    SYSKEY = 0x556699AA;  /* Unlock OSCCON, step 2 */
    while(OSCCON & (1 << 21)); /* Wait until PBDIV ready */
    OSCCONCLR = 0x180000; /* clear PBDIV bit <0,1> */
    while(OSCCON & (1 << 21));  /* Wait until PBDIV ready */
    SYSKEY = 0x0;  /* Lock OSCCON */

    /* Set up output pins */
    AD1PCFG = 0xFFFF;
    ODCE = 0x0;
    TRISECLR = 0xFF;
    PORTE = 0x0;

	PORTF = 0xFFFF;
	PORTG = (1 << 9);
	ODCF = 0x0;
	ODCG = 0x0;
	TRISFCLR = 0x70;
	TRISGCLR = 0x200;

    /* Set up input pins */
	TRISDSET = (1 << 8);
	TRISFSET = (1 << 1);

	/* Set up SPI as master */
	SPI2CON = 0;
	SPI2BRG = 4;
	/* SPI2STAT bit SPIROV = 0; */
	SPI2STATCLR = 0x40;
	/* SPI2CON bit CKP = 1; */
    SPI2CONSET = 0x40;
	/* SPI2CON bit MSTEN = 1; */
	SPI2CONSET = 0x20;
	/* SPI2CON bit ON = 1; */
	SPI2CONSET = 0x8000;
}

//Function to draw the falling shape on the screen
void drawShape(uint8_t* shape, int x, int y, int rotation){
	int i;
	int r, c;
	shape += rotation * 16;
	for(r = 0; r<4; r++){
		for(c=0; c<4; c++){
			if(*shape == 1){
				displayBlock((c*3)+1 + x, (r*3)+1 + y);
			}
			shape++;
		}
	}
}

//Function to draw the next shape on the screen
void drawNextShape(uint8_t* shape){
	int r, c;
	for(r = 0; r<4; r++){
		for(c=0; c<4; c++){
			if(*shape == 1){
				displayPixel(75+ 2*c,22+2*r);
				displayPixel(75+2*c+1,22+2*r);
				displayPixel(75+2*c,22+2*r+1);
				displayPixel(75+2*c+1,22+2*r+1);
			}
			shape++;
		}
	}
}



//Function to draw a block on the screen
void displayBlock(int x, int y){
	displayPixel(x,y);
	displayPixel(x+1,y);
	displayPixel(x+2,y);

	displayPixel(x,y+1);
	displayPixel(x+2,y+1);

	displayPixel(x,y+2);
	displayPixel(x+1,y+2);
	displayPixel(x+2,y+2);
}

//Function to display a number on screen
int displayNumber(uint8_t* nrs, int number, int x, int y){
	int r, c;
	nrs += number * 15;
	//loop over the number
	for(r = 0; r<5; r++){
        for(c=0; c<3; c++){
			if(*nrs == 1){
				displayPixel(x-r, y+c);
			}
			nrs++;
		}
	}
}


