#include <avr/io.h>

#include "main.h"
#include "oled.h"
#include "beep.h"

word t = 0;
word btn_timer = 0;

const byte LANES[4][3][2] = {
    { {7, 2}, {7, 1}, {7, 0}, },
    { {7, 4}, {7, 5}, {7, 6}, },
    { {8, 3}, {9, 3}, {10, 3},},
    { {6, 3}, {5, 3}, {4, 3}, },
};

int main (void) 
{    
    initialise();
    initialise_oled();
    clear_display();
    
    PORTB |= 1 << DC;           // DATA
    
    display_image(&LOGO[0], 3, 3, 10, 2);
    crap_beep(_A5, 140);
    delay_ms(5);
    crap_beep(_A8, 60);
    
    delay_ms(SPLASH_DELAY);
    
    byte map_dirty = TRUE;
    
    byte player = F_DOWN;
    byte player_timer = 0;
    byte baddies[4];
    
    for (byte i=0 ; i<4 ; i++)
        baddies[i] = 0;
    
    word game_timer = 0;
    word game_timer_delay = 1500;
    
    word score = 0;
    
    for(ever)
    {
        t = millis();
        
        word btn_val = read_buttons();
        if (btn_timer == 0)
        {
            if (btn_val >= _UP-BTN_THRESHOLD && btn_val <= _UP+BTN_THRESHOLD)
            {
                click();
                player = F_UP;
                btn_timer = t;
                
                map_dirty = TRUE;
            }
            else if(btn_val >= _DOWN-BTN_THRESHOLD && btn_val <= _DOWN+BTN_THRESHOLD)
            {
                click();
                player = F_DOWN;
                btn_timer = t;
                
                map_dirty = TRUE;
            }
            else if(btn_val >= _LEFT-BTN_THRESHOLD && btn_val <= _LEFT+BTN_THRESHOLD)
            {
                click();
                player = F_LEFT;
                btn_timer = t;
                
                map_dirty = TRUE;
            }
            else if(btn_val >= _RIGHT-BTN_THRESHOLD && btn_val <= _RIGHT+BTN_THRESHOLD)
            {
                click();
                player = F_RIGHT;
                btn_timer = t;
                
                map_dirty = TRUE;
            }
            else if(btn_val >= _A-BTN_THRESHOLD && btn_val <= _A+BTN_THRESHOLD)
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
                btn_timer = t;
                
                map_dirty = TRUE;
            }
            else if(btn_val >= _B-BTN_THRESHOLD && btn_val <= _B+BTN_THRESHOLD)
            {
                click();
                btn_timer = t;
            }
            else if(btn_val >= _C-BTN_THRESHOLD && btn_val <= _C+BTN_THRESHOLD)
            {
                click();
                btn_timer = t;
            }
        }
        
        if (t - btn_timer >= BTN_DELAY)
            btn_timer = 0;
        
        if (player_timer && t >= player_timer) //TODO: Edge case where t+dur overflows
        {
            player_timer = 0;
            player -= 4;
        }
        
        if (map_dirty)
        {
            set_display_col_row(0, 0);
            for (byte row=0 ; row<SCREEN_ROWS ; row++)
            {
                //set_display_col_row(0, row);
                for (byte col=0 ; col<SCREEN_COLUMNS ; col++)
                {
                    shift_out_block(&GLYPHS[MAP[ SCREEN_COLUMNS * row + col ]*8], FALSE);
                }
            }
            
            word _s = score;
            for (byte d=0 ; d<4 ; d++)
            {
                display_block( &GLYPHS[((_s % 10)+1)*8], 4-d, 7);
                _s = _s / 10;
            }
            
            map_dirty = FALSE;
        }
        
        display_block(&GLYPHS[player*8], 7, 3);
        for (byte i=0 ; i<4 ; i++)
        {
            for (byte j=0 ; j<3 ; j++)
            {
                if (baddies[i] & (1<<j))
                    display_block(&GLYPHS[(B_UP+i)*8], LANES[i][j][0], LANES[i][j][1]);
            }
        }
        
        if (t >= game_timer)
        {
            for (byte i=0 ; i<4 ; i++)
            {
                if (baddies[i] & 1)
                {
                    display_block(&GLYPHS[26*8], 7, 3);
                    for(ever) {}
                }
                else
                    baddies[i] = baddies[i] >> 1;
            }
            baddies[ rng() % 4 ] |= 1<<2;
            game_timer = t + game_timer_delay;
            if (game_timer_delay > 550)
                game_timer_delay -= SPEED_STEP;
            
             map_dirty = TRUE;
        }
    }
}