#include <genesis.h>
#include <resources.h>
#include <string.h>

Sprite* ball;
Sprite* player;
int player_pos_x = 144;
const int player_pos_y = 200;
int player_vel_x = 0;
const int player_width = 32;
const int player_height = 8;

int ball_pos_x = 100;
int ball_pos_y = 100;
int ball_vel_x = 1;
int ball_vel_y = 1;
int ball_width = 8;
int ball_height = 8;

int flashing = FALSE;
int frames = 0;
int ball_color = 0;

game_on = FALSE;
char msg_start[22] = "PRESS START TO BEGIN!\0";
char msg_reset[37] = "GAME OVER! PRESS START TO PLAY AGAIN.";

/*The edges of the play field*/
const int LEFT_EDGE = 0;
const int RIGHT_EDGE = 320;
const int TOP_EDGE = 0;
const int BOTTOM_EDGE = 224;

/*Score variables*/
int score = 0;
char label_score[6] = "SCORE\0";
char str_score[3] = "0";

int sign(int x) {
    return (x > 0) - (x < 0);
}

void updateScoreDisplay(){
	sprintf(str_score,"%d",score);
	VDP_clearText(1,2,3);
	VDP_drawText(str_score,1,2);
}

/*Draws text in the center of the screen*/
void showText(char s[]){
	VDP_drawText(s, 20 - strlen(s)/2 ,15);
}

void startGame(){
	score = 0;
	updateScoreDisplay();

	ball_pos_x = 0;
	ball_pos_y = 0;

	ball_vel_x = 1;
	ball_vel_y = 1;

	/*Clear the text from the screen*/
	VDP_clearTextArea(0,10,40,10);

	game_on = TRUE;
}

void endGame(){
	showText(msg_reset);
	game_on = FALSE;
}

void moveBall() {
    //check screen bounds
    if(ball_pos_x < LEFT_EDGE){
        ball_pos_x = LEFT_EDGE;
        ball_vel_x = -ball_vel_x;
    } else if(ball_pos_x + ball_width > RIGHT_EDGE){
        ball_pos_x = RIGHT_EDGE - ball_width;
        ball_vel_x = -ball_vel_x;
    }

    if(ball_pos_y < TOP_EDGE){
        ball_pos_y = TOP_EDGE;
        ball_vel_y = -ball_vel_y;
    } else if(ball_pos_y + ball_height > BOTTOM_EDGE){
        endGame();
    }

    //check if collided
    if(ball_pos_x < player_pos_x + player_width && ball_pos_x + ball_width > player_pos_x){
        if(ball_pos_y < player_pos_y + player_height && ball_pos_y + ball_height >= player_pos_y){
            score++;
            flashing = TRUE;
            updateScoreDisplay();
            if( score % 10 == 0){
                ball_vel_x += sign(ball_vel_x);
                ball_vel_y += sign(ball_vel_y);
            }	
            ball_pos_y = player_pos_y - ball_height - 1;
		    ball_vel_y = -ball_vel_y;
        }
    }

    //move
    ball_pos_x += ball_vel_x;
    ball_pos_y += ball_vel_y;

    SPR_setPosition(ball,ball_pos_x,ball_pos_y);
}

void myJoyHandler( u16 joy, u16 changed, u16 state)
{
	if (joy == JOY_1)
	{
        if(state & BUTTON_START){
            if(!game_on){
                startGame();
            }
        }
		/*Set player velocity if left or right are pressed;
		 *set velocity to 0 if no direction is pressed */
		if (state & BUTTON_RIGHT)
		{
			player_vel_x = 3;
		}
		else if (state & BUTTON_LEFT)
		{
			player_vel_x = -3;
		} else{
			if( (changed & BUTTON_RIGHT) | (changed & BUTTON_LEFT) ){
				player_vel_x = 0;
			}
		}
	}
}

void positionPlayer(){
	/*Add the player's velocity to its position*/
	player_pos_x += player_vel_x;

	/*Keep the player within the bounds of the screen*/
	if(player_pos_x < LEFT_EDGE) player_pos_x = LEFT_EDGE;
	if(player_pos_x + player_width > RIGHT_EDGE) player_pos_x = RIGHT_EDGE - player_width;

	/*Let the Sprite engine position the sprite*/
	SPR_setPosition(player,player_pos_x,player_pos_y);
}

int main()
{
    JOY_init();
    JOY_setEventHandler( &myJoyHandler );
    //tile background
    VDP_loadTileSet(bgtile.tileset,1,DMA); // load tile into RAM memory slot 1
    VDP_setPalette(PAL1, bgtile.palette->data);
    //VDP_setTileMapXY(BG_B,TILE_ATTR_FULL(PAL1,0,FALSE,FALSE,1),2,2); // Show a tile at 1,2 (8 pixels, 8 pixels, tile in memory slot 1)
    VDP_fillTileMapRect(BG_B,TILE_ATTR_FULL(PAL1,0,FALSE,FALSE,1),0,0,40,30);

    //load sprites
    SPR_init(0,0,0);
    ball = SPR_addSprite(&imgball,100,100,TILE_ATTR(PAL1,0, FALSE, FALSE));
    player = SPR_addSprite(&paddle, player_pos_x, player_pos_y, TILE_ATTR(PAL1, 0, FALSE, FALSE));

    /*Draw the texts*/
    VDP_setTextPlane(BG_A);
    VDP_drawText(label_score,1,1);
    updateScoreDisplay();
    showText(msg_start);
    ball_color = VDP_getPaletteColor(22);

    while(1)
    {
        if(game_on == TRUE){
            moveBall();
            positionPlayer();

            //Handle the flashing of the ball
            if( flashing == TRUE ){
                frames++;
                if( frames % 4 == 0 ){
                    VDP_setPaletteColor(22,ball_color);
                } else if( frames % 2 == 0){
                    VDP_setPaletteColor(22,RGB24_TO_VDPCOLOR(0xffffff));
                }

                //Stop flashing
                if(frames > 30){
                    flashing = FALSE;
                    frames = 0;
                    VDP_setPaletteColor(22,ball_color);
                }
            }
        }

        SPR_update();
        SYS_doVBlankProcess();
    }
    return (0);
}
