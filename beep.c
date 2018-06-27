#include <avr/io.h>

#include "beep.h"

void crap_beep(word note, word dur)
{
    word ts = millis();
    while (millis() < ts + dur)
    {
        PORTB |= 1 << SND;    // HIGH
        delay_us(note);
        PORTB &= ~(1 << SND); // LOW
        delay_us(note);
    }
}

void click( void )
{
    crap_beep(_A9, 15);
}