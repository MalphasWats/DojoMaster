#include "SPIKE.h"

#include <avr/io.h>
#include <avr/interrupt.h>

volatile dword _millis = 0;

const __memx Tune *current_tune;
byte current_note;
bool playing = FALSE;

byte rngSEED = 5;
byte rng( void )
{
    rngSEED = (rngSEED*rngA +rngC) % rngM;
    return rngSEED;
}

void delay_ms( word ms )
{
    ms += _millis;
    if (ms<_millis)
    {
        while(_millis > 0);
        while(_millis < ms);
    }
    else
        while(_millis < ms);
}

void initialise( void )
{
    /* Configure Outputs */
    //DDRA
    DDRB = (1<<CS) | (1<<DC) | (1<<RST) | (1<<SCK);
    
    PORTC = 0xFF; // Set pull-up resistors
    
    DDRD = (1 << SND) | (1<<MOSI);
    
    /* Initialise "millis" Timer */
    TCCR0A = 0x02;          // OC0A disconnected, CTC Mode.
    TCCR0B = 0x03;          // 1/64 CLK Prescale.
    
                 //                     F_CPU   Prescale  Timer frequency (1 ms)
    //OCR0A = 125; // Set compare value (8000000Hz / 64) / 1000Hz
    OCR0A = 250; // Set compare value (16000000Hz / 64) / 1000Hz
    
    TIMSK0 |= 0x02;         // Enable OCR0A Compare Interrupt
    
    /* Configure sound timers */
    //TODO: Use Proper bit names
    TCCR1A = 0b01000001;    // phase correct pwm mode
    TCCR1B = 0b00010010;    // 1/8 Prescale
    
    OCR1A = 0;

    
    TCCR3A = 0b00000000;
    TCCR3B = 0b00001101;    // CTC Mode, 1/1024 prescale
    
    TIMSK3 = 0x02;
    
    OCR3A = 0;
    
    
    /* Configure Harware SPI
       USCZ01 = UDORD0 = 0 (MSBFIRST)
       UCSZ00 = UCPHA0 = 0 (SPI MODE 0)
       UCPOL0 =          0 (SPI MODE 0) */
    UBRR0 = 0; // MAXIMUM BAUD RATE
    UCSR0C = (1<<UMSEL01) | (1<<UMSEL00) | (0<<UCSZ01) | (0<<UCSZ00) | (0<<UCPOL0);
    UCSR0B = (0<<RXEN0) | (1<<TXEN0);
    
    UBRR0 = 0; // MAXIMUM BAUD RATE
    
    sei();                  // Enable interrupts
    
    play_tune(&STARTUP_CHIME);
    
    /* Setup Display */
    initialise_oled();
    
    for(byte y=0 ; y<(LOGO.height>>3) ; y++)
        for(byte x=0 ; x<LOGO.width ; x++)
            buffer[(y+2)*SCREEN_WIDTH + (x+16)] = LOGO.data[y*LOGO.width + x];
    
    draw();
    
    delay_ms(SPLASH_DELAY);
}

ISR(TIMER0_COMPA_vect)
{
    _millis += 1;
}

ISR(TIMER3_COMPA_vect)
{
    if (playing)
    {
        if (current_note < current_tune->length)
        {
            byte note_index = current_tune->score[current_note] & 0x0f;
            byte beat_index = current_tune->score[current_note] >> 4;
            OCR1A = NOTES[note_index];
            TCNT3 = 0;
            OCR3A = BEATS[beat_index] * BEAT_ATOM;
            current_note += 1;
            return;
        }
        else
        {
            playing = FALSE;
        }
    }
    
    OCR3A = 0;
    OCR1A = 0;
}

void note(byte note, word duration)
{
    if (!playing)
    {
        OCR1A = NOTES[note];
        TCNT3 = 0;
        OCR3A = duration * NOTE_DURATION_MULTIPLIER;
    }
}

void play_tune(const __memx Tune *t)
{
    if (!playing)
    {
        current_note = 0;
        current_tune = t;
        playing = TRUE;
        TCNT3 = 0;
        OCR3A = 1;
    }
}

void stop_tune()
{
    if (playing)
    {
        OCR3A = 0;
        OCR1A = 0;
        playing = FALSE;
    }
}

dword millis( void )
{
    return _millis;
}

/* OLED Functions */
void shift_out_byte(byte b)
{
    while( !( UCSR0A & (1<<UDRE0) ) );
    UDR0 = b;
}

/* Initiasation for SSD1306 OLED Controller */
void initialise_oled(void)
{
    PORTB &= ~(1 << CS);                // LOW (Enabled)
    PORTB &= ~(1 << DC);                // LOW (Command Mode)
    
    PORTB |= 1 << RST;          // HIGH
    delay_ms(10);
    PORTB &= ~(1 << RST);       // LOW
    delay_ms(10);
    PORTB |= 1 << RST;          // HIGH
    delay_ms(10);
    
    shift_out_byte(0xAE);               // DISPLAYOFF
    
    shift_out_byte(0xD5);               // SETDISPLAYCLOCKDIV
    shift_out_byte(0x80);               // the suggested ratio 0x80

    shift_out_byte(0xA8 );              // SSD1306_SETMULTIPLEX
    shift_out_byte(SCREEN_HEIGHT - 1);

    shift_out_byte(0xD3 );              // SETDISPLAYOFFSET
    shift_out_byte(0x0);                // no offset
    shift_out_byte(0x40  | 0x0);        // SETSTARTLINE line #0
  
    shift_out_byte(0x8D);               // CHARGEPUMP
    shift_out_byte(0x14);               // Not External Vcc
  
    shift_out_byte(0x20 );              // MEMORYMODE
    shift_out_byte(0x00);               // 0x0 act like ks0108
    shift_out_byte(0xA0  | 0x1);        // SEGREMAP
    shift_out_byte(0xC8 );              // COMSCANDEC


    shift_out_byte(0xDA);               // SETCOMPINS
    shift_out_byte(0x12);
  
    shift_out_byte(0x81 );              // SETCONTRAST
    shift_out_byte(0xCF);               // Not External Vcc


    shift_out_byte(0xD9 );              // SETPRECHARGE
    shift_out_byte(0xF1);               // Not External Vcc
  
    shift_out_byte(0xDB);               // SETVCOMDETECT
    shift_out_byte(0x40);
  
    shift_out_byte(0xA4 );              // DISPLAYALLON_RESUME
    shift_out_byte(0xA6 );              // NORMALDISPLAY

    //shift_out_byte(0x2E );            // DEACTIVATE_SCROLL

    shift_out_byte(0xAF);               // DISPLAYON
    
    shift_out_byte(0xB0 + 0);           // PAGEADDR (0 = reset)
    shift_out_byte(0 & 0x0F);           // Column start address (0 = reset)
    shift_out_byte(0x10 | (0 >> 4));    // LOW COL ADDR
    
    PORTB |= 1 << CS;                   // HIGH (Disabled)
    PORTB |= 1 << DC;                   // DATA
    delay_ms(1);
    PORTB &= ~(1 << CS);                // LOW (Enabled)
}

void clear_buffer(void)
{
    for (word i=0 ; i<SCREEN_WIDTH*SCREEN_ROWS ; i++)
        buffer[i] = 0x00;
}

void draw( void )
{
    for (word i=0 ; i<SCREEN_WIDTH*SCREEN_ROWS ; i++)
        shift_out_byte(buffer[i]);
}

void display_off(void)
{
    PORTB &= ~(1 << DC);        // COMMAND
    shift_out_byte(0xAE);       // DISPLAYOFF
    
    PORTB |= 1 << DC;           // DATA
}

void display_on(void)
{
    PORTB &= ~(1 << DC);        // COMMAND
    shift_out_byte(0xAF);       // DISPLAYON
    
    PORTB |= 1 << DC;           // DATA
}

/* Sound Functions */

void click( void )
{
    note(_C5, 15);
}