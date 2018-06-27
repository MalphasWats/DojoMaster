#ifndef OLED_H
# define OLED_H

#include "platform.h"

#define MSBFIRST 0
#define LSBFIRST 1

#define CMD     LOW
#define DATA    HIGH

#define WIDTH   128
#define HEIGHT  64

#define SCREEN_COLUMNS 16
#define SCREEN_ROWS    8

static const __flash byte BLANK[] = {0, 0, 0, 0, 0, 0, 0, 0};

void shift_out(byte val, byte order);

void shift_out_block(const __flash byte *block, byte inverted);

void send_command(byte command);
void initialise_oled(void);
void clear_display(void);
void display_off(void);
void display_on(void);

void display_image(const byte *img, byte col, byte row, byte width, byte height);
void display_block(const byte *block, byte col, byte row);
void display_block_(const byte *block, byte col, byte row, byte inverted);

void set_display_row(byte row);
void set_display_col_row(byte col, byte row);

#endif