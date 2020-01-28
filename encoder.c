#include "rangefinder.h"
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h>

volatile unsigned char new_state, old_state;
volatile unsigned char a, b;
volatile unsigned char read;


int enc(int count);

void encoder()
{
	
	
	
  	DDRC &= ~((1 << 3)|(1 << 4));
    PORTC |= ((1 << 3)|(1 << 4));

    read = (PINC & ((1 << 3)|(1 << 4)));
    a = read & (1 << 4);
    b = read & (1 << 3);
    if (!b && !a)
	old_state = 0;
    else if (!b && a)
	old_state = 1;
    else if (b && !a)
	old_state = 2;
    else
	old_state = 3;

    new_state = old_state;// determining values of states with various inputs

    
    
    	
    
}

int enc(int count)
{
	// Read the input bits and determine A and B
	read = (PINC & ((1 << PC3)|(1 << PC4)));
	a = (read & (1 << 4));
    b = (read & (1 << 3));

	
	// For each state, examine the two input bits to see if state
	// has changed, and if so set "new_state" to the new state,
	// and adjust the count value.
	
	if (old_state == 0)
	{ 

	    	// Handle A and B inputs for state 0
	  	if((b == 0) && (a != 0)) 
	  	{ 
	   		new_state = 1;
    		count++;		//increment count
		}
	    if((b != 0) && (a == 0)) 
	    { 
			new_state = 2;
			count--;  		//decrement count
		}
	}
	else if (old_state == 1) 
	{ 

	    	// Handle A and B inputs for state 1
	    if((b != 0) && (a != 0)) 
	    {
			new_state = 3;
    		count++;		//increment count
	    }
		if((b == 0) && (a == 0))
		 { 
    		new_state = 0;
	    	count--;		//decrement count
	    }
	}
	else if (old_state == 2)
	 { 

	    	// Handle A and B inputs for state 2
		if((b == 0) && (a == 0)) 
		{ 
			new_state = 0;
			count++;		//increment count
		}
		if((b != 0) && (a != 0)) 
		{ 
			new_state = 3;
			count--;			//decrement count
		}
	}
	else 
	{   

	    	// Handle A and B inputs for state 3
	    if((b != 0) && (a == 0))
	     { 
			new_state = 2;
    		count++;		//increment count
	    }
		if((b == 0) && (a != 0))
		 { 
    		new_state = 1;
	    	count--;		//decrement count
	    }
	}

	if(count > 400) // keep count at 400 if encoder is changed further
	{
		count = 400;
	}
	if(count < 1)	// keep count at 1 if encoder is changed further back
	 {
		count = 1;
	}

	
	old_state = new_state;			// set old state as new state
		
	return count;					// returning count
}
	

