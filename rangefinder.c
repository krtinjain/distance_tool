/********************************************
 *
 *  Name: Krtin Jain	
 *  Email: krtinjai@usc.edu
 *  Section: Friday 11am
 *  Assignment: Final Project
 *
 ********************************************/

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <math.h>
#include "lcd.h"
#include <stdlib.h>
#include <avr/interrupt.h>
#include "rangefinder.h"
#include "serial.h"
#include "encoder.h"
#include <avr/eeprom.h>h

void variable_delay_us(int);

void setloct(int n);
void setremt(int m);
char getmode();
char acquire();
char adjust();
void timer0(char in);

volatile int flag = 0;
volatile int timeout = 0;
volatile char mod = 0;

volatile double pcount = 0;
volatile int cm, mm;
volatile int remt = 1;
volatile int loct = 1;
volatile int lag = 0;
volatile int c, d, r, l, q, y;
volatile unsigned char excess = 0;
volatile unsigned char change = 0; 
volatile unsigned char encchange = 0;
volatile unsigned char send[7];
volatile int remdist;
volatile char o[10];
volatile char data[6];
volatile char out = 0;

volatile int recdist;
volatile char ag = 0;

extern volatile int check;
extern volatile char a[5];

 
int main(void)
{
	
	lcd_init(); //initializing lcd

	serial_init(); //initializing  74LS125

	char cnt[9]; 
	char pnt[9];
	char dis[9];

	lcd_writecommand(1);

	lcd_stringout("Krtin Jain");

	lcd_moveto(1, 0);

	lcd_stringout("Project");

	_delay_ms(1000);
			
	lcd_writecommand(1);
	 // Write a spash screen to the LCD

	encoder(); // initializing encoder
	

    lcd_stringout(cnt); 		
	DDRD |= (1 << PD3); // setting trigger in rangefinder to Output
	DDRD &= ~(1 << PD2); // setting echo in rangefinder to input
	PORTD |= (1 << PD2); // enabling pull-up resistor
	DDRC &= ~(1 << 2); //setting acquire button as input
	DDRC &= ~(1 << 1); // setting adjust button as input
	PORTC |= (1 << 2); //enabling pullup resistors for acqure and adjust buttons
	PORTC |= (1 << 1); 
	DDRC |= (1 << PC5); // setting green led as output
	DDRB |= (1 << PB3); // setting red led as output
	DDRB |= (1 << PB5); // setting buzzer as output
	PORTB &= ~(1 << PB5); // clearing output bit of buzzer

	unsigned int distance; 

	loct = eeprom_read_word((void *) 100); // setting value of location threshold to the stored value in eeprom
	remt = eeprom_read_word((void *) 1000); // setting value of remote threshold to stored value in eeprom
	



	PCICR |= (1 << 1); // setting PCIE1 bit
  	PCMSK1 |= ((1 << 3) | (1 << 4));
	PCMSK2 |= ((1 << PCINT19) | (1 << PCINT18));
	PCICR |= (1 << PCIE2); // enable pin change interrupt
	TCCR1B |= (1 << WGM12);
	// Enable Timer Interrupt
	TIMSK1 |= (1 << OCIE1A);
			// Load the MAX count
			// Assuming prescalar = 8
			// counting to 46400
	OCR1A = 46400;
	TCCR0A |= (1 << WGM01);
			//enabling timer interrupt
	TIMSK0 |= (1 << OCIE0A);
	OCR0A = 50; //counting to 50
	sei();
	while (1)
		{
			int distance;



			if(adjust() != 0) //check if adjust button is pressed
			{
				if(mod == 0) //check mode and change if button is pressed
				{
					mod = 1;
				}
				else if(mod == 1)
				{
					mod = 0;
				}
			}
			if(acquire() != 0) // if acquire button is pressed, send a signal through trigger of rangefinder
			{
				PORTD |= (1 << PD3); //setting output to 1
				_delay_us(10);
				PORTD &= ~(1 << PD3);	// clearing output bit to 0
				
			}
			if (mod == 0) //if mode is 0, stringout local and print value if local threshold
			{
				lcd_moveto(0,0);
				lcd_stringout("Local ");
				lcd_moveto(1,0);
				snprintf(cnt, 10, "Min=%d  ", loct);
				lcd_stringout(cnt);
					
					
				
			}
			else if(mod == 1)  //if mode is 1, stringout remote and print value if remote threshold
			{
				lcd_moveto(0,0);
				lcd_stringout("Remote");
				lcd_moveto(1,0);
			   	snprintf(pnt, 9, "Min=%d  ", remt);// Output count to LCD
			    lcd_stringout(pnt);
			}
			
			
			
				if(mod == 0) // feeding value of location threshold in eeprom and getting the new value of location threshold using enc() function from encoder file
				{	
					if(encchange == 1)
					{
					eeprom_update_word((void *) 100, loct);
					loct = enc(loct);
					encchange = 0;
				
					
					}
				}
				if(mod == 1) 	// feeding value of remote threshold in eeprom and getting the new value of remote threshold using enc() function from encoder file
				{
					if(encchange == 1)
					{
					eeprom_update_word((void *) 1000, remt);
					remt = enc(remt);
			    	encchange = 0;
					}
				}
			
					
				
			
			if(flag == 1)
			{
				flag = 0;  //setting flag to 0
				int d; 
				distance = (unsigned int)(pcount/(5.8*2)); // calculating value of distance using casting
				d = distance;
				snprintf(o, 7, "@%d$", distance); //print value of distance 
				serial_stringout(o); 

				q = distance/10;  
				y = distance;
				mm = distance%10;
				distance = distance/10; // separating first two digits 
				cm = distance%10;
				distance = distance/10;

				if(distance == 0) 		//value of second digit is 0
				{
					snprintf(dis, 10, "%d.%d", cm, mm); //print first digit and decimal
					lcd_moveto(0,11);
					lcd_stringout("     ");
					lcd_moveto(0,13);
					lcd_stringout(dis);
				}
				else if(distance != 0) 
				{
					r = distance%10;             //separate second digit
					distance = distance/10;

					if(distance == 0)		//if value of third digit is 0
					{
						snprintf(dis, 10, "%d%d.%d", r, cm, mm); // print first two digits and decimal place
						lcd_moveto(0,11);
						lcd_stringout("     ");
						lcd_moveto(0,12);
						lcd_stringout(dis);

					}
					else if (distance != 0)
					{
						l = distance%10;   	//separate third digit
						snprintf(dis, 10, "%d%d%d.%d", l, r, cm, mm); 	// print first three digits and decimal place
						lcd_moveto(0,11);
						lcd_stringout("     ");
						lcd_moveto(0,11);
						lcd_stringout(dis);
					}
				}
				
				
				if(excess==1) //if clock reaches count value
					{
						
						lcd_moveto(0,11); 	//print far
						lcd_stringout("  far");
						excess = 0;
				
					}
					
				
				

				
				if(mod == 0) //if local mode
				{
					if((q) > loct) //if distance is more than threshold
					{
						PORTC |= (1 << PC5); // turn green LED on and red LED off
						PORTB &= ~(1 << PB3);
					}
					else if((q) < loct)
					{
						PORTC &= ~(1 << PC5); // turn green LED off and red LED on
						PORTB |= (1 << PB3);
					}
				}
				
				else if(mod == 1) //if remote mode
				{
					PORTB &= ~(1 << PB3); //turn both leds off
					PORTC &= ~(1 << PC5);
				}
			}
		
			if(mod == 1) //if remote mode, printing recieved distance
			{
				if(check) // if flag = 1
				{
					data[0] = a[1]; // putting value of first array element as second element of received character array and consequently the others
					data[1] = a[2];
					data[2] = a[3];
					data[3] = a[4];
					sscanf(data, "%d", &recdist); // converting array values to an integer

					check = 0;           // setting flag to 0
					char rec[7];         
					remdist = recdist;   
					int digit2 = recdist;
					int e, f, g, h;
					e = digit2%10;        //separating first two characters
					digit2 = digit2/10;
					f = digit2%10;
					digit2 = digit2/10;
					if(digit2 == 0)      // if second digit is 0
					{
						snprintf(rec, 7, "  %d.%d", f, e); //printing out
						lcd_moveto(1,11);
						lcd_stringout(rec);
					}
					
					else if(digit2 != 0)
					 {
						g = digit2%10;   //separating third digit
						digit2 = digit2/10;
						
						if(digit2 == 0)  //if third digit is 0
						{
							snprintf(rec, 7, " %d%d.%d", g, f, e); //print out
							lcd_moveto(1,11);
							lcd_stringout(rec);
						}
						
						else if(digit2 != 0)
						{
							h = digit2%10;     // separating fourth digit
							snprintf(rec, 7, "%d%d%d.%d", h, g, f, e); // printing out
							lcd_moveto(1,11);
							lcd_stringout(rec);
						}
					}
					
					if((remt*10) > recdist) // if remote threshold is more than remote distance
					{
						out = 1; //set flag to 1
						timer0(out); // start buzzer
					}
					else if((remt*10) <= recdist) // if remote threshold is less than remote distance
					{
						out = 0; //set flag to 0
						timer0(out); // buzzer stops
					}
					
				
				}
		}
			
				
				
			


		

	}		
}

ISR(PCINT1_vect)  //for encoder
{
  encchange = 1; //check if pin change in encoder
	
} 
ISR(TIMER1_COMPA_vect) //for rangefinder
{
	
	excess = 1;// check if it exceeds value of ocr1a

}
ISR(PCINT2_vect) //for timer
{
	if((PIND & (1 << PD2)) != 0) // reset timer and start timer
	{
		TCNT1 = 0;
		TCCR1B |= (1 << CS11);
	}						
	else					// else set value of pulse count equal to tcnt1 and stop timer and set flag to 1
	{
		pcount = TCNT1;
		TCCR1B &= ~(1 << CS11);
		flag = 1;
	}
}


ISR(TIMER0_COMPA_vect) // using timer ISR for buzzer
{
    
    if(ag == 1) //set flag to 0 if flag is 1
    {
    	ag = 0;
    }
    else if(ag == 0) // set flag to 1 if flag is 0
    {
    	ag = 1;
    }
    if(ag == 0) //if flag is 0, turn on buzzer
    {
    	PORTB |= (1 << PB5);
    }
    else if(ag == 1)
    {
    	PORTB &= ~(1 << PB5); //if flag is 1, turn off buzzer
    }
   
}


   



 char acquire()     // to check if aqcuire button is pressed
{
	
	if((PINC & (1 << 2)) == 0)   // check value between specific interval
	{
		_delay_ms(5);
		while((PINC & (1 << 2)) == 0) //debounce
		{}
		_delay_ms(5);
		return 1;

	}
	else 
	return 0;
}
char adjust() // to check if adjust button is pressed
{
	
	if((PINC & (1 << 1)) == 0)    // check value between specific interval
	{
		_delay_ms(5);
		while((PINC & (1 << 1)) == 0) //debounce
		{}
		_delay_ms(5);
		return 1;

	}
	else 
	return 0;
	
}	
void timer0(char in) //to turn on timer ISR for buzzer if input value is 1 and turn it off if input value is 0
{
	if(in == 1)
	{
		TCCR0B |= (1 << CS02);
		_delay_ms(1500);
		TCCR0B &= ~(1 << CS02);
	}
	else if(in == 0)
	{
		TCCR0B &= ~(1 << CS02);
	}
}

