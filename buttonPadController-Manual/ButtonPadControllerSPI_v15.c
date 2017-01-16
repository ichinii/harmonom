/********************************************************************************
					Button Pad Controller SPI Code
					  Sparfun Electronics
					 Written by Ryan Owens
*********************************************************************************
*/

//***************************************
//			Include Libraries
//***************************************
#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include "OCTOPLODODON_SLAVE.h"

//***************************************
//			Global Variables
//***************************************
volatile uint8_t row=0;
volatile int frame_num=0, incoming_board_index=0, boards_received=0, outgoing_board_index=0;
volatile int board_complete=0, bit_count_input=0, bit_count_output=0;

volatile int enter_command_mode=0;
char command_id, command_byte;
unsigned char NUMBER_OF_BOARDS=1;

//redf, bluef and greenf are the display buffers.  These arrays contain the color value for the corresponding LED.
unsigned char redf[NUMBER_OF_LEDS];
unsigned char bluef[NUMBER_OF_LEDS];
unsigned char greenf[NUMBER_OF_LEDS];
//The buttons array contains the current button values of the board
unsigned char buttons[NUMBER_OF_LEDS];

//OUtoing Board contains the data to be sent to the next Slave Board
//outgoing_board[NUMBER_OF_LEDS*4] -> redf[NUMBER_OF_LEDS] + greenf[NUMBER_OF_LEDS] + bluef[NUMBER_OF_LEDS] + buttons[NUMBER_OF_LEDS]
//So the current boards button values go with the next boards color values
unsigned char outgoing_board[NUMBER_OF_LEDS*4];

//Data received on the SPI_IN line go into the incoming_board buffer. After receiving values for all 16 leds, the buffer is transfered to 
//the display frames (redf, greenf, bluef) or the outgoing_board buffer depending on how many boards are left to be transmitted.
unsigned char incoming_board[NUMBER_OF_LEDS*4];


ISR (PCINT0_vect)
{
	
	cli();

	if(PINB & (1<<SPI_SCLK)){
		//Make sure that the Chip Select line is high.
		if(PINB & (1<<SPI_CS)){
			enter_command_mode=0;
			//If the clock is high than there is valid data. Put the incoming bit into the color frame
			if(PINB & (1<<SPI_IN))incoming_board[incoming_board_index]|=(1<<bit_count_input++);	//If the recv. bit is high, set the bit in the LED array
			else incoming_board[incoming_board_index]&=~(1<<bit_count_input++);			
			
			if(bit_count_input >= COLOR_BITS){
				bit_count_input=0;
				incoming_board_index+=1;
			}
		}
		//If the chip select line is low and the data line is high, we may be trying to get into "Command Mode."
		else enter_command_mode=1;
	}
	else{
		if(PINB & (1<<SPI_CS)){
			enter_command_mode=0;
			//If the clock is low, than we can send data to the next board.
			if((outgoing_board[outgoing_board_index]>>bit_count_output++)&0x01)sbi(PORTB, SPI_OUT);
			else cbi(PORTB, SPI_OUT);
			
			if(bit_count_output >= COLOR_BITS){
				bit_count_output=0;
				outgoing_board_index+=1;
			}
		}
		//Might be time to enter command mode!
		else{
			if(enter_command_mode==1)enter_command_mode=2;
			else enter_command_mode=0;
		}
	}
	
	//Check to see if we've received a whole board of data
	if(incoming_board_index >= (NUMBER_OF_LEDS*4)){
		incoming_board_index=0;
		outgoing_board_index=0;
		board_complete=1;
	}
	
	sei();
}

int main (void)
{
	int count=0;

	ioinit (); //Boot up defaults
	
	//Startup Sequence
	fill16(redf, 0xff);
	fill16(greenf, 0x00);
	fill16(bluef, 0x00);
	while(count<3000){
		display();
		count++;
	}
	count=0;
	fill16(redf, 0x00);
	fill16(greenf, 0xff);
	fill16(bluef, 0x00);
	while(count<3000){
		display();
		count++;
	}
	count=0;
	fill16(redf, 0x00);
	fill16(greenf, 0x00);
	fill16(bluef, 0xff);
	while(count<3000){
		display();
		count++;
	}
	fill16(redf, 0x00);
	fill16(greenf, 0x00);
	fill16(bluef, 0x00);
	redf[NUMBER_OF_BOARDS-1]=0xff;
	greenf[NUMBER_OF_BOARDS-1]=0xff;
	bluef[NUMBER_OF_BOARDS-1]=0xff;
	
	while(1)
	{
		//Keep displaying until we get a new board image
		while(!board_complete && (enter_command_mode!=2)){
			display();
			if(boards_received==0)for(int i=0; i<NUMBER_OF_LEDS; i++)outgoing_board[i+48]=buttons[i];
			
			//If we've received a CS pulse, then we should reset the frame indices
			if(!(PINB & (1<<SPI_CS))){
				cli();
				bit_count_input=0;
				bit_count_output=0;
				incoming_board_index=0;
				outgoing_board_index=0;
				sei();
			}
		}
		cli();
		if(board_complete){	
			board_complete=0;
			boards_received+=1;
			
			//If we've received a complete data set then retrieve the new board color values
			if(boards_received>=NUMBER_OF_BOARDS){
				for(int i=0; i<NUMBER_OF_LEDS; i++)redf[i]=incoming_board[i];
				for(int i=0; i<NUMBER_OF_LEDS; i++)greenf[i]=incoming_board[i+16];
				for(int i=0; i<NUMBER_OF_LEDS; i++)bluef[i]=incoming_board[i+32];
				boards_received=0;
			}
			//If we haven't received a complete data set, then send the data on to the next board
			else for(int i=0; i<NUMBER_OF_LEDS*4; i++)outgoing_board[i]=incoming_board[i];
		}
		
		if(enter_command_mode==2){			
			//Get the command id from the SPI bus
			for(int id_bit=0; id_bit<8; id_bit++){
				while((PINB & (1<<SPI_SCLK))!=(1<<SPI_SCLK));
				
				if(PINB & (1<<SPI_IN))command_id |= (1<<id_bit);
				else command_id &= ~(1<<id_bit);
				
				while(PINB & (1<<SPI_SCLK));	//Wait for the SPI clock to go low before getting next bit.
			}
			
			//Get the command byte from the SPI bus
			for(int data_bit=0; data_bit<8; data_bit++){
				while((PINB & (1<<SPI_SCLK))!=(1<<SPI_SCLK));
				
				if(PINB & (1<<SPI_IN))command_byte |= (1<<data_bit);
				else command_byte &= ~(1<<data_bit);
				
				while(PINB & (1<<SPI_SCLK));	//Wait for the SPI clock to go low before getting next bit.
			}
			enter_command_mode=0;
		}
		//If we received a command, lets handle it!
		if(command_id==CHANGE_NUM_BOARDS){
			NUMBER_OF_BOARDS=command_byte;
			write_to_EEPROM(NUMBER_OF_BOARDS_ADDRESS, NUMBER_OF_BOARDS);
		}
		sei();
	}

	return (0);
}

void fill16(volatile uint8_t* src, uint8_t fill)
{
    int i;
    for (i = 0; i < 16; i++)
    {
        src[i] = fill;
    }
}

//Initializes the I/O pins of the ATmega168.
//Initializes the Shift Register
//Sets up the Interrupt
//Initalizes the System Parameters for this board
void ioinit (void)
{

    //1 = output, 0 = input
	DDRB |= (1<<SPI_OUT);	//Port B OUtputs
	DDRB &= ~((1<<SPI_IN) | (1<<SPI_SCLK) | (1<<SPI_CS)); //Port B Inputs
	PORTB |= ((1<<SPI_IN) | (1<<SPI_SCLK) | (1<<SPI_CS));	//Use Pull-Ups
	SPCR=0;
			
    DDRC = 0xFF; //Shift Register Outputs
    
	DDRD = 0x0F; //Bits 0-3: LED Cathode for rows; Bits 4-7: Button Inputs
	PORTD = 0xF0;	//Enable pull-ups on button inputs
    
    cbi(PORTC, SR_CLEAR); //Clear current registers
    delay_short(HOLD_TIME);
    sbi(PORTC, SR_CLEAR);
	
	//Setup Interrupts to be generated on SPI_SCK Pin Change
	PCICR = (1<<0);	//Enable Pin Change Interrupts on PCINT0
	PCMSK0 = (1<<SPI_SCLK);	//Enable PC interrupts on the SPI_SCK pin
	
	//Load the number of boards from EEPROM
	NUMBER_OF_BOARDS = read_from_EEPROM(NUMBER_OF_BOARDS_ADDRESS);
	if(NUMBER_OF_BOARDS == 0 || NUMBER_OF_BOARDS > 16){
		NUMBER_OF_BOARDS=1;
		write_to_EEPROM(NUMBER_OF_BOARDS_ADDRESS, NUMBER_OF_BOARDS);
	}	
	sei();
	
    row = 0;
}

//SUMMARY: Sends a row of color values to the board
//Clocks out a 16-bit row_colors to the shift registers and enables them
//Also opens drain for given row
void clock_byte(volatile uint16_t row_colors, volatile uint8_t rowNum)
{  	
	for(int i = 0; i < 16; i++)
    {
        cbi(PORTC, SR_SCK);

        if (row_colors & (1 << i))
            sbi(PORTC, SR_DATA);
        else
            cbi(PORTC, SR_DATA);
    
        sbi(PORTC, SR_SCK);
    }
        
	sbi(PORTC, SR_EN); //Disable outputs

    cbi(PORTC, SR_LATCH); 
	delay_short(HOLD_TIME);
    sbi(PORTC, SR_LATCH); //Latch the current data clocked into registers
 
	//Turn on LED's of the current row
    PORTD &= 0xF0;		//Make sure we keep the high bytes of PortD(Button Values)
    PORTD |= (1 << rowNum);	//Put the rowNum on the Port D pins
    cbi(PORTC, SR_EN); //Enable outputs
}    

//General short delays
void delay_short(uint16_t x)
{
  for ( ; x > 0 ; x--){
    for (char y = 0;  y <  5; y++){
        asm volatile ("nop");
    }
  }
}

//Generates a "random" new color for the input color arrays
void newcolor(uint8_t * r, uint8_t * g, uint8_t * b)
{

  uint16_t k = *r | (*g << 3) | (*b << 6);
  uint8_t i,j;
  
  i = k&0xff;
  j = k >> 8;
  while((i*j)==0)
  {
    i=i+j+1;
    j=j+3;
  }
  k=i*j;
  *r=k&7;
  *g=(k>>3)&7;
  *b=(k>>6)&7;
}

//Combines the three color frames to determine the color for each row. Also 
//determines the button status of the board
void display(void){
    uint16_t lineByte=0; //byte that carries this line's LED info
    uint8_t row_offset,LED, buttonRow;

    row = (row + 1) & 3; //Only 4 rows
    if(row == 0) frame_num = (frame_num + 1) & (COLOR_DEPTH); //Limits frame count to COLOR_DEPTH frames

    row_offset = row << 2;

    for (char column = 0; column < 4; column++) //Only 4 columns
    {
        LED = column + row_offset;
		
		if(column != row)lineByte |= (1<<column);
        if(greenf[LED] > frame_num) lineByte |= (1 << (column + GREEN_POS));
        if(bluef[LED] > frame_num) lineByte |= (1 << (column + BLUE_POS));
        if(redf[LED] > frame_num) lineByte |= (1 << (column + RED_POS));
    }

    //Output the current RGB values to this row
	clock_byte(lineByte, row);
	
	//get the button values
	buttonRow=PIND;	//Get the value of the buttons
	for(int i=0; i<4; i++){
		if(buttonRow & (1<<(i+4)))buttons[(i*4)+row]=NOT_PRESSED;
		else buttons[(i*4)+row]=PRESSED;
	}
}

//Description: Writes an unsigned char(Data)  to the EEPROM at the given Address
//Pre: Unsigned Int Address contains address to be written to
//	 Unsigned Char Data contains data to be written
//Post: EEPROM "Address" contains the "Data"
//Usage: write_to_EEPROM(0, 'A');
void write_to_EEPROM(unsigned int Address, unsigned char Data)
{
    //Interrupts are globally disabled!
	
	while(EECR & (1<<EEPE)); //Wait for last Write to complete
	//May need to wait for Flash to complete also!
	EEAR = Address;			//Assign the Address Register with "Address"
	EEDR=Data;				//Put "Data" in the Data Register
	EECR |= (1<<EEMPE); 	//Write to Master Write Enable
	EECR |= (1<<EEPE);  	//Start Write by setting EE Write Enable
	
}

//Description: Reads the EEPROM data at "Address" and returns the character
//Pre: Unsigned Int Address is the address to be read
//Post: Character at "Address" is returned
//Usage: 	unsigned char Data;
//		Data=read_from_EEPROM(0);
unsigned char read_from_EEPROM(unsigned int Address)
{
	//Interrupts are globally disabled!
	
	while(EECR & (1<<EEPE));	//Wait for last Write to complete
	EEAR = Address;				//Assign the Address Register with "Address"
	EECR |= (1<<EERE); 			//Start Read by writing to EER
	return EEDR;				//EEPROM Data is returned
}
