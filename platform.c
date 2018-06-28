#include "platform.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

word _millis = 0;

word rngSEED = 5;
word rng( void )
{
    rngSEED = (rngSEED*rngA +rngC) % rngM;
    return rngSEED;
}

void delay_ms( word ms )
{
   for (word i=0; i < ms; i++)
   {
      _delay_ms(1);
   }
}

void delay_us( word us )
{
   for (word i=0; i < us; i++)
   {
      _delay_us(1);
   }
}

word read_buttons()
{
    
    ADCSRA = 0xC7;
    
    while(ADCSRA & (1<<ADSC));
    
    byte low = ADCL;
    byte high = ADCH;
    
    return (high << 8) | low;
}

void initialise( void )
{
    DDRB = (1<<SDA) | (1<<DC) | (1<<SCL) | (1<<SND); // Configure Outputs
    
    ADMUX = 0x00 | BTNS;    // Configure analog input
    
    /* Initialise Timer */
    TCCR1 = 0x86;           // CTC Mode, CK/32 prescale (250)
    TIMSK |= 0x40;          // Enable OCIE1A Compare Interrupt
    
    sei();                  // Enable interrupts
    
                 //                     F_CPU   Prescale  Timer frequency (1 ms)
    OCR1A = 250; // Set compare value (8000000Hz / 32) / 1000Hz
}

ISR(TIMER1_COMPA_vect)
{
    _millis += 1;
}

word millis( void )
{
    return _millis;
}