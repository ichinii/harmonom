//*******************************************************
//
//		Button Pad Controller SPI Definitions
//				SparkFun Electronics
//   		   Written By Ryan Owens
//			
//*******************************************************
//HARDWARE NOTE: High Fuse should be set to 0xDF
//				 Low Fuse should be set to 0xFF

//*******************************************************
//					General Definitions
//*******************************************************
/* SHADE_DEPTH must be a power of 2 -1 */
#define COLOR_DEPTH 7
#define COLOR_BITS	8

//#define NUMBER_OF_BOARDS	2
#define NUMBER_OF_LEDS		16

#define GREEN_POS	4
#define BLUE_POS	8
#define RED_POS		12
#define HOLD_TIME	1

#define NOT_PRESSED	0xFF
#define PRESSED		0x00

//EEPROM ADDRESSES
#define NUMBER_OF_BOARDS_ADDRESS	0

//*******************************************************
//				Command Byte Definitions
//*******************************************************
#define CHANGE_NUM_BOARDS	1

//*******************************************************
//					ATmega168 Pin Definitions
//*******************************************************
//SPI Communication Pins
#define SPI_SCLK	5
#define SPI_IN		4
#define SPI_OUT		3
#define SPI_CS		2

//All Shift Register pins reside on port c of the Atmega
#define SR_SCK    0
#define SR_DATA   1
#define SR_LATCH  2
#define SR_CLEAR  3
#define SR_EN     4

//Button control pins
#define BUTTON0         (1<<4)
#define BUTTON1         (1<<5)
#define BUTTON2         (1<<6)
#define BUTTON3         (1<<7)

//*******************************************************
//					Global Macros
//*******************************************************
#define sbi(var, mask)   ((var) |= (uint8_t)(1 << mask))
#define cbi(var, mask)   ((var) &= (uint8_t)~(1 << mask))


//*******************************************************
//			Global Functions Prototypes
//*******************************************************
void delay_short(uint16_t x); // general purpose delay
void fill16(volatile uint8_t* src, uint8_t fill);
void newcolor(uint8_t * r, uint8_t * g, uint8_t * b);
void clock_byte(volatile uint16_t my_byte, volatile uint8_t rowNum);
void ioinit (void);      // initializes IO
void display(void);
unsigned char read_from_EEPROM(unsigned int Address);
void write_to_EEPROM(unsigned int Address, unsigned char Data);
