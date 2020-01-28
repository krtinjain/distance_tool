#include "rangefinder.h"
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include "lcd.h"
#include <stdio.h>

void serial_init();
void serial_stringout(char *);
void serial_txchar(char);

volatile int start = 0;
volatile int check = 0;
volatile int j = 0;

#define FOSC 16000000           // Clock frequency
#define BAUD 9600               // Baud rate used
#define MYUBRR (FOSC/16/BAUD-1) // Value for UBRR0 register

char volatile a[7];

////////////////////////////////////////////////////////////////////

ISR(USART_RX_vect) // Handle received character
{
	char ch;      
    ch = UDR0;		//storing incoming character
    
    if(start == 0) //checking if the flag is set to 0
    {
	    if(ch == '@') 	// checking if the first digit is @	
	    {
	    	a[j] = ch; 	//storing value of incoming character in an array
	    	j++;  		//increment index Value
	    	start = 1;  //setting flag to 1
	    	
	    }
    }
    else if(start == 1)	//checking if flag is set to 1
    {
    	a[j] = ch;		//storing value of incoming character in an array
    	j++;			//increment index Value
	    if(ch == '$')  //checking is last input character is $
	    {
	    	j = 0; 		// setting index value to 0
	    	check = 1;	//setting flag to 1
	    	start = 0;	// setting flag to 0
	    }
    }
    
}
void serial_init()
{
	UCSR0C = (3 << UCSZ00); // Async., no parity, 1 stop bit , 8 data bits
	UCSR0B |= ((1 << TXEN0)|(1 << RXEN0)); // Enable RX and TX
    DDRB |= (1 << PB4); 	// Set up USART0 registers
    PORTB &= ~(1 << PB4);
    UCSR0B |= (1 << RXCIE0);	// Enable tri-state
    UBRR0 = MYUBRR;
}

void serial_txchar(char ch)       
{
    while ((UCSR0A & (1<<UDRE0)) == 0); //outputting characters
    UDR0 = ch;
}

void serial_stringout(char *s)
{
	char count = 0;
    
	while(count<20) // calling serial_txchar in a loop to send a string
	{             
		serial_txchar(s[count]);
		
		if(s[count] == '$') //stopping loop when array character reaches $
		{
			break; 	
		}
		count++;	//increment index value
	}
}



