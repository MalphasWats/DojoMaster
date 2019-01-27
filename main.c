#include <avr/io.h>

#include "main.h"

unsigned long t = 0;
unsigned long btn_timer = 0;

const byte LANES[4][3][2] = {
    { {7, 2}, {7, 1}, {7, 0}, },
    { {7, 4}, {7, 5}, {7, 6}, },
    { {8, 3}, {9, 3}, {10, 3},},
    { {6, 3}, {5, 3}, {4, 3}, },
};

int main (void) 
{    
    initialise();
    // display logo
    for(byte y=0 ; y<LOGO_HEIGHT ; y++)
        for(byte x=0 ; x<LOGO_WIDTH ; x++)
            buffer[(y+2)*SCREEN_WIDTH + (x+16)] = LOGO[y*LOGO_WIDTH + x];
            draw();
            
    note(_A4, 90);
    delay_ms(180);
    note(_C5, 60);
    delay_ms(120);
    note(_E5, 60);
    
    delay_ms(SPLASH_DELAY);
    
    byte buttons;
    
    byte player = F_DOWN;
    byte player_timer = 0;
    byte baddies[4];
    
    for (byte i=0 ; i<4 ; i++)
        baddies[i] = 0;
    
    unsigned long game_timer = 0;
    word game_timer_delay = 1500;
    
    word score = 0;
    
    for(ever)
    {
        t = millis();
        
        buttons = ~PINC;
        if (btn_timer <= t)
        {
            if (buttons & _UP)
            {
                click();
                player = F_UP;
                btn_timer = t+BTN_DELAY;
            }
            else if(buttons & _DOWN)
            {
                click();
                player = F_DOWN;
                btn_timer = t+BTN_DELAY;
            }
            else if(buttons & _LEFT)
            {
                click();
                player = F_LEFT;
                btn_timer = t+BTN_DELAY;
            }
            else if(buttons & _RIGHT)
            {
                click();
                player = F_RIGHT;
                btn_timer = t+BTN_DELAY;
            }
            else if(buttons & _A)
            {
                click();
                if (player_timer == 0)
                {
                    player += 4;
                    player_timer = t + FRAME_DURATION;
                    if (baddies[player-P_UP] & 1)
                    {
                        score += 1;
                        baddies[player-P_UP] &= ~1;
                        //TODO: animate baddie dieing
                    }
                }
                btn_timer = t+BTN_DELAY;
            }
        }
        
        if (player_timer && t >= player_timer) //TODO: Edge case where t+dur overflows
        {
            player_timer = 0;
            player -= 4;
        }
        
        clear_buffer();
        
        for (byte row=0 ; row<SCREEN_ROWS ; row++)
        {
            for (byte col=0 ; col<SCREEN_COLUMNS ; col++)
            {
                draw_tile(&GLYPHS[MAP[ SCREEN_COLUMNS * row + col ]*8], col*8, row*8);
            }
        }
        
        word _s = score;
        for (byte d=0 ; d<4 ; d++)
        {
            draw_tile( &GLYPHS[((_s % 10)+1)*8], (4-d)*8, 7*8);
            _s = _s / 10;
        }
            
        draw_tile( &GLYPHS[player*8], 7*8, 3*8);
        for (byte i=0 ; i<4 ; i++)
        {
            for (byte j=0 ; j<3 ; j++)
            {
                if (baddies[i] & (1<<j))
                    draw_tile( &GLYPHS[(B_UP+i)*8], LANES[i][j][0]*8, LANES[i][j][1]*8 );
            }
        }
        
        if (t >= game_timer)
        {
            for (byte i=0 ; i<4 ; i++)
            {
                if (baddies[i] & 1)
                {
                    draw_tile( &GLYPHS[26*8], 7*8, 3*8 );
                    for(ever) {}
                }
                else
                    baddies[i] = baddies[i] >> 1;
            }
            baddies[ rng() % 4 ] |= 1<<2;
            game_timer = t + game_timer_delay;
            if (game_timer_delay > 550)
                game_timer_delay -= SPEED_STEP;
        }
        
        draw();
    }
}

void draw_tile(const byte __memx *glyph, int x, int y)
{
    /* is the tile actually visible
       Last one is y >= SCREEN_HEIGHT because of the HUD */
    if (x < -7 || x >= SCREEN_WIDTH || y < -7 || y >= SCREEN_HEIGHT)
        return;
    
    
    int y_ = y;
    
    if (y < 0)
        y_ = 0-y;
        
    int tile_start = ((y_ >> 3) * SCREEN_WIDTH) + x;
    
    byte y_offset_a = y & 7; // y % 8
    byte y_offset_b = 8-y_offset_a;
    
    byte glyph_index = 0;
    byte tile_width = 8;
    if (x < 0)
    {
        tile_start -= x;
        glyph_index = 0-x;
        tile_width -= glyph_index;
    }
    
    if (x > SCREEN_WIDTH-8)
    {
        tile_width = SCREEN_WIDTH-x;
    }
    
    if (y < 0)
    {
        y_offset_a = 8;
        y_offset_b = 0-y;
        tile_start -= SCREEN_WIDTH;
    }
    
    if (y > SCREEN_HEIGHT-8)
    {
        y_offset_b = 8;
    }
    
    for(byte tile_offset=0 ; tile_offset<tile_width ; tile_offset++, glyph_index++)
    {
        if (y_offset_a < 8)
            buffer[tile_start+tile_offset] |= glyph[glyph_index] << y_offset_a;
        if (y_offset_b < 8)
            buffer[tile_start+SCREEN_WIDTH+tile_offset] |= glyph[glyph_index] >> y_offset_b;
    }
}