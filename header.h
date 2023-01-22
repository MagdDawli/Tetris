
#include <stdint.h>

// =============  Display functions   =========== //
#define DISPLAY_SIZE 512
#define COLUMNS 128 //number of display columns
#define PAGES 4 //number of display memory pages
uint8_t dispBuffer [DISPLAY_SIZE];

//Functions from display.c
void sleep(int);
void displayPixel(int, int);
void displayInit(void);
void clearDisplay(void);
void displayUpdate(uint8_t*);
void displaySendPage(int, uint8_t*);
void SPIInit(void);
uint8_t sendSPI(uint8_t);
void drawShape(uint8_t*, int, int, int);
void drawNextShape(uint8_t*);
void displayBlock(int, int);
void drawBLOCK(void);
int displayNumber(uint8_t*, int , int , int );

