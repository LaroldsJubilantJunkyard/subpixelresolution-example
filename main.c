#include <gb/gb.h>
#include <gb/cgb.h>
#include <stdint.h>
#include <gb/metasprites.h>
#include "graphics/mario.h"
#include "graphics/font.h"
#include "graphics/mushroom.h"

#define FLOOR_Y 100

#define DEFAULT 0 
#define FRAME_RATE 1 
#define COUNTER 2
#define SUB_PIXEL_RESOLUTION 3 

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define ABS(x) ((x < 0) ? -x : x)

#define MARIO_RUN_FRAMECOUNT 4

uint8_t runCounter=0;
uint8_t counter=0;
uint16_t marioX;

uint16_t marioDrawX;
uint16_t marioDrawFrame;
uint8_t method=DEFAULT;
uint8_t marioDirection=0;

int16_t mushroomY;
int16_t mushroomX;

uint8_t marioFrame=0;

uint8_t joypadCurrent=0;
uint8_t joypadPrevious=0;

void DrawText(uint8_t x, uint8_t y, unsigned char *text ){

    uint8_t i=0;

    // The VRAM address of the first character
    // After setting a tile, we'll increase the VRAM address each iteration to move to the next tile
    uint8_t *vramAddr= get_bkg_xy_addr(x,y);

    while(text[i]!='\0'){

        // Map our alphabet characters to only use uppercase letters
        // From the SpaceInvadersFont.png/aseprite
        if(text[i]>='A'&&text[i]<='Z')set_vram_byte(vramAddr++,1+(text[i]-'A'));
        else if(text[i]>='a'&&text[i]<='z')set_vram_byte(vramAddr++,1+(text[i]-'a'));
        else if(text[i]>='0'&&text[i]<='9')set_vram_byte(vramAddr++,27+(text[i]-'0'));

        else {

            // Map our special characters manually
            // From the SpaceInvadersFont.png/aseprite
            switch(text[i]){
                case ':': set_vram_byte(vramAddr++,38); break;
                case '.': set_vram_byte(vramAddr++,43);break;
                case '/': set_vram_byte(vramAddr++,40);break;
                default: vramAddr++; break;
            }
        }


        i++;
    }
    VBK_REG=0;

}

void UpdateMethodText(){
    fill_bkg_rect(0,0,20,1,0);
    DrawText(1,1,"Press A to Change");
    DrawText(2,17,"Move Left/Right");
    switch (method)
    {
    case DEFAULT:
        DrawText(6,0,"Default");
        DrawText(1,16,"Frame Rate: 60FPS");
        break;
    case SUB_PIXEL_RESOLUTION:
        DrawText(3,0,"Sub-Pixel Res.");
        DrawText(1,16,"Frame Rate: 60FPS");
        break;
    case FRAME_RATE:
        DrawText(5,0,"Frame Rate");
        DrawText(1,16,"Frame Rate: 20FPS");
        break;
    case COUNTER:
        DrawText(6,0,"Counter");
        DrawText(1,16,"Frame Rate: 60FPS");
        break;
    }
}


void UpdateTheMushroom(){

    // Move the mushroom at a consistent speed
    mushroomY++;

    int16_t d =(mushroomX-marioDrawX);

    // If the horizontal distance is less than 16
    if(ABS(d)<16){

        d =(mushroomY-(FLOOR_Y-12));

        // If the vertical distance is less than 17
        if(ABS(d)<17){
         
            mushroomY=-16;
            mushroomX = 8+(DIV_REG+mushroomX)%144;
        }
    }

    // If the mushroom has fallen offscreen
    if(mushroomY>=152){

        // Reset to the top
        // Pick a random x position
        mushroomY=-16;
        mushroomX = 8+(DIV_REG+mushroomX)%144;
    }

    move_metasprite(mushroom_metasprites[0],mario_TILE_COUNT,4,mushroomX,mushroomY+8);
}

int8_t HandleInput(){
    int8_t movement = 0;
    if(joypadCurrent & J_LEFT){
        movement = -1;
        marioDirection=0;
    }
    else if(joypadCurrent & J_RIGHT){
        movement=1;
        marioDirection=1;
    }

    // When a is pressed
    if((joypadCurrent & J_A) && !(joypadPrevious & J_A)){

        // Unshift the value if we WERE  in SPR
        if(method==SUB_PIXEL_RESOLUTION){
            marioX=marioX>>4;
        }
        method++;

        // SHIFT the value if we are now in SPR
        if(method==SUB_PIXEL_RESOLUTION){
            marioX=marioX<<4;
        }

        // Loop back around
        if(method>=4)method=0;

        UpdateMethodText();
    }
    return movement;
}

void main(void)
{
    SHOW_SPRITES;
    SPRITES_8x16;
    DISPLAY_ON;

    // Put our sprite and backgroud data into VRAM
    set_bkg_data(0,font_TILE_COUNT,font_tiles);
    set_bkg_palette(0,font_PALETTE_COUNT,font_palettes);
    set_sprite_data(0,mario_TILE_COUNT,mario_tiles);
    set_sprite_data(mario_TILE_COUNT,mushroom_TILE_COUNT,mushroom_tiles);
    set_sprite_palette(0,mario_PALETTE_COUNT,mario_palettes);

    // Put mario in the middle
    marioX=80;
    method=DEFAULT;

    // Random mushroom position
    mushroomY=-16;
    mushroomX = 8+(DIV_REG)%144;

    UpdateMethodText();

    // Loop forever
    while(1) {

        joypadPrevious=joypadCurrent;
        joypadCurrent=joypad();

        int16_t movement = HandleInput();

        marioDrawX=marioX;
        marioDrawFrame=marioFrame;

        switch(method){

            // This is okay for certain move speeds( It can't do less than 1px/second though)
            // However, animation is out of control
            default:
                if(movement!=0){
                    marioX+=movement*2;
                    marioDrawX=marioX;
                    
                    // Increase mario's frame
                    // Loop background after 3
                    if(++marioFrame>=MARIO_RUN_FRAMECOUNT)marioFrame=0;

                    // Frame 0 is or standing
                    // Increase by one for running frames
                    marioDrawFrame=1+marioFrame;
                }else{

                    // Draw with our standing frame 0
                    marioFrame=0;
                    marioDrawFrame=0;
                }
                break;

            // This is the best method
            // We don't need an extra variable, and we have more control over slower rates
            case SUB_PIXEL_RESOLUTION:

                // Increase/Decrease the "face" value 
                marioX+=movement*25;

                // Draw mario at the true value
                marioDrawX=marioX>>4;

                // If the player is moving
                if(movement!=0){

                    // Increase mario's frame by 2
                    // This is a sub-pixel resolution value
                    marioFrame+=2;
                    
                    // Increase mario's frame
                    // Loop background after 3
                    // Shift right 4 bits to get it's true value
                    if((marioFrame>>4)>=MARIO_RUN_FRAMECOUNT)marioFrame=0;
                
                    // Frame 0 is or standing
                    // Increase by one for running frames
                    // Shift right 4 bits to get it's true value
                    marioDrawFrame=1+marioFrame>>4;
                }else{

                    // Draw with our standing frame 0
                    marioFrame=0;
                    marioDrawFrame=0;
                }
                break;

            // With the frame rate method , the logic is just like the default method
            // Accept we need to increase some values more since code executes less often
            case FRAME_RATE:
                if(movement!=0){
                    marioX+=movement*5;
                    marioDrawX=marioX;
                    
                    // Increase mario's frame
                    // Loop background after 3
                    if(++marioFrame>=MARIO_RUN_FRAMECOUNT)marioFrame=0;

                    // Frame 0 is or standing
                    // Increase by one for running frames
                    marioDrawFrame=1+marioFrame;
                }else{

                    // Draw with our standing frame 0
                    marioFrame=0;
                    marioDrawFrame=0;
                }
                break;

            // In the counter method, we use a extra counter to increment values at a slower rate
            // This avoids the issue of having to adjust frame rate.
            case COUNTER:
                if(movement!=0){

                    // Change Mario's X position by 5 every 3 frames
                    if(++counter>=3){
                        counter=0;
                        marioX+=movement*5;
                        marioDrawX=marioX;
                    }

                    // Increase our animation counter until it reachces 6, then reset
                    if(++runCounter>=6){
                        runCounter=0;

                        // Increase mario's frame
                        // Loop background after 3
                        if(++marioFrame>=MARIO_RUN_FRAMECOUNT)marioFrame=0;

                        // Frame 0 is or standing
                        // Increase by one for running frames
                        marioDrawFrame=1+marioFrame;
                    }
                }else{

                    // Reset our counters
                    counter=0;
                    runCounter=0;

                    // Draw with our standing frame 0
                    marioFrame=0;
                    marioDrawFrame=0;
                }
                break;
        }




        // Draw the mario metasprite
        // Flip the metasprite horizontally (using vflip) if mario isn' facing right
        if(marioDirection==1)move_metasprite(mario_metasprites[marioDrawFrame],0,0,marioDrawX,FLOOR_Y);
        else move_metasprite_vflip(mario_metasprites[marioDrawFrame],0,0,marioDrawX+4,FLOOR_Y);
        
        UpdateTheMushroom();
       
        uint8_t waitCount= method==FRAME_RATE ? 3 : 1;

        for(uint8_t i=0;i<waitCount;i++){

            // Done processing, yield CPU and wait for start of next frame
            wait_vbl_done();
        }
        
    }
}
