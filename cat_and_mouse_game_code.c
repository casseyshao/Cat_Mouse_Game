/* 
Cat and Mouse:
In this game, you are the mouse. You must get to the cheese before
the time on the seven-segment HEX displays run out or the cat catches you.
*/

#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <stdio.h>
#include "visuals.h"

//global variable declarations
volatile int pixel_buffer_start;

//for the maze:
enum {
    W = 10,         // width of maze (# of cells)
    H = 10          // height of maze cell (# of cells)
};
enum {
    North,
    East,
    South,
    West,
    NDir
}; 
char visited[H][W];
char horz[H][W - 1];        // horizontal E-W paths in the maze
char vert[H - 1][W];        // veritcal N-S paths in the maze
int shift = 24;

//timer global variable
int minutes = 0;
int seconds = 0;
int msec = 0;

//function declarations
void clear_screen(); //so far no use
void start_screen();
void game_screen();
void level_one_win_screen();
void time_up_screen();
void cat_caught_mouse_screen();
void game_over_screen();
void wait_for_vsync();
void plot_pixel(int x, int y, short int line_color);
void draw_mouse(int x_box, int y_box);
void draw_cat(int x, int y);
void swap(int *x, int *y); //so far no use
void clear_old_box(int x, int y);
bool maze_limit(int x, int y, int direction_value);

//functions for timer
bool count_down_timer();
void display_sub(int min, int sec);

//functions for maze
int adjacent(int dir[], int x, int y);
void dfs(int x, int y);
void map(void);
void plot_H_dash(int x, int y, short int color);
void plot_V_dash(int x, int y);
void draw_entrance_exit();

int main(void)
{
    volatile int *pixel_ctrl_ptr = (int *)0xFF203020;
    volatile int *SW_ptr = (int *)0xFF200040;
    volatile int *KEY_ptr = (int *)0xFF200050;
    volatile int *LED_ptr = (int *)0xFF200000;
	
    int SW_value, KEY_value;
    bool start_game = false;
    bool level_one = false;
    bool start_level_two =false;
    bool level_two = false;
	
    //initialize mouse position
    int dx_box = 0; //movement in x direction
    int dy_box = 0; //movement in y direction	
    int x_box = 0; //top left corner position
    int y_box = 120; //top left corner position
    int old_x_box = x_box;
    int old_y_box = y_box;
	
    //initialize cat position
    int cat_random_direction = 3;
    bool change_direction = false;
    int cat_dx_box = 0;
    int cat_dy_box = 0;
    int cat_x_box = 42;
    int cat_y_box = 2;
    int cat_old_x_box = cat_x_box;
    int cat_old_y_box = cat_y_box;

    //set front pixel buffer to start of FPGA On-chip memory
    //first store the address is the back buffer
    *(pixel_ctrl_ptr + 1) = 0xC8000000;
	
    //now, swap the front/back buffers, to set the front buffer location
    wait_for_vsync();
	
    //initialize a pointer to the pixel buffer, used by drawing functions
    pixel_buffer_start = *pixel_ctrl_ptr;
    start_screen(); 
    *LED_ptr = 0;
	
    //pixel_buffer_start points to the pixel buffer
    //set back pixel buffer to start of SDRAM memory
    *(pixel_ctrl_ptr + 1) = 0xC0000000;
    pixel_buffer_start = *(pixel_ctrl_ptr + 1); //we draw on the back buffer
	
    KEY_value = *KEY_ptr;
    while (!start_game) {
        KEY_value = *KEY_ptr;
	if (KEY_value == 0x1) {
            start_game = true;
            break;
	}
    }
	
    game_screen();
    //initialize the maze
    srand(time(NULL));
    dfs(0, 0);
    //plot maze
    map();
    draw_entrance_exit();
	
    wait_for_vsync();
    pixel_buffer_start = *(pixel_ctrl_ptr + 1);
    game_screen();
    //plot maze
    map();
    draw_entrance_exit();
	
    //initialize global variables for level 1 timer
    minutes = 2;
    seconds = 30;
    msec = 0;
    bool hex_display_on = count_down_timer();
	
    //start timer
    clock_t before = clock();
    while (!level_one) {
		
        //erase cat and mouse that were drawn in the last iteration
	clear_old_box(old_x_box, old_y_box);
	clear_old_box(cat_old_x_box, cat_old_y_box);
		
	*LED_ptr = 1; //indicates that you are on level 1
		
	SW_value = *SW_ptr;
		
	map();
	draw_entrance_exit();
     	
	draw_mouse(x_box, y_box);
	draw_cat(cat_x_box, cat_y_box);
		
	//update location of the mouse
	if ((SW_value == 0x1) && (x_box > 0) && (maze_limit(x_box,y_box,1) == false)) { //move left
	    dx_box = -1;
	    dy_box = 0;
	} else if ((SW_value == 0x2) && (x_box <= 299) && (maze_limit(x_box,y_box,2) == false)) { //move right
	    dx_box = 1;
	    dy_box = 0;
	} else if ((SW_value == 0x4) && (y_box > 0) && (maze_limit(x_box,y_box,3) == false)) { //move up
	    dx_box = 0;
	    dy_box = -1;
	} else if ((SW_value == 0x8) && (y_box <= 219) && (maze_limit(x_box,y_box,4) == false)) { //move down
	    dx_box = 0;
	    dy_box = 1;
	} else {
	    dx_box = 0;
	    dy_box = 0;
	}
				   
	old_x_box = x_box;
	old_y_box = y_box;
	x_box+=dx_box;
	y_box+=dy_box;
		
	//update location of the cat
	if ((cat_random_direction == 1) && (cat_x_box > 41) && (maze_limit(cat_x_box,cat_y_box,1) == false)) { //move left
	    cat_dx_box = -1;
	    cat_dy_box = 0;
	} else if (cat_random_direction == 1) {
	    cat_dx_box = 0;
	    cat_dy_box = 0;
	    change_direction = true;
	} else if ((cat_random_direction == 2) && (cat_x_box <= 260) && (maze_limit(cat_x_box,cat_y_box,2) == false)) { //move right
	    cat_dx_box = 1;
	    cat_dy_box = 0;
	} else if (cat_random_direction == 2) {
	    cat_dx_box = 0;
	    cat_dy_box = 0;
	    change_direction = true;
	} else if ((cat_random_direction == 3) && (cat_y_box > 2) && (maze_limit(cat_x_box,cat_y_box,3) == false)) { //move up
	    cat_dx_box = 0;
	    cat_dy_box = -1;
	} else if (cat_random_direction == 3) {
	    cat_dx_box = 0;
	    cat_dy_box = 0;
	    change_direction = true;
	} else if ((cat_random_direction == 4) && (cat_y_box <= 219) && (maze_limit(cat_x_box,cat_y_box,4) == false)) { //move down
	    cat_dx_box = 0;
	    cat_dy_box = 1;
	} else if (cat_random_direction == 4){
	    cat_dx_box = 0;
	    cat_dy_box = 0;
	    change_direction = true;
	}
			
	if ((change_direction == true) && (cat_random_direction == 1)) {
	    int numbers[] = {2,3,4};
	    cat_random_direction = numbers[rand()%3];
	    change_direction = false;
	} else if ((change_direction == true) && (cat_random_direction == 2)) {
	    int numbers[] = {1,3,4};
	    cat_random_direction = numbers[rand()%3];
	    change_direction = false;
	} else if ((change_direction == true) && (cat_random_direction == 3)) {
	    int numbers[] = {1,2,4};
	    cat_random_direction = numbers[rand()%3];
	    change_direction = false;
	} else if ((change_direction == true) && (cat_random_direction == 4)) {
	    int numbers[] = {1,2,3};
	    cat_random_direction = numbers[rand()%3];
	    change_direction = false;
	}
		
	cat_old_x_box = cat_x_box;
	cat_old_y_box = cat_y_box;
	cat_x_box+=cat_dx_box;
	cat_y_box+=cat_dy_box;
		
	//check to see if mouse made it to the cheese (win)
	for (int i = 300; i<320; i++) {
	    if ((x_box == i) && (y_box == 113)){
	        level_one = true;
		continue;
	    }
	}
		
	//check to see if cat caught mouse
	bool cat_caught_mouse = false;
	if( ((y_box>=cat_y_box && y_box<=cat_y_box+20)||(y_box+20>=cat_y_box && y_box+20<=cat_y_box+20)) //mouse hits the cat on the left or right
		    &&((x_box == cat_x_box+20)||(x_box+20 == cat_x_box))){
	    cat_caught_mouse = true;	
	}
	if( ((x_box>=cat_x_box && x_box<=cat_x_box+20)||(x_box+20>=cat_x_box && x_box+20<=cat_x_box+20)) //mouse hits the cat on the top or bottom
	    &&((y_box+20 == cat_y_box)||(y_box == cat_y_box+20))){
	    cat_caught_mouse = true;
	}
		
	//check to see if time is out
	bool check_time = false;
	clock_t difference = (int)clock - before; //measure how much time passed in the while loop
	int msec_increase = difference / CLOCKS_PER_SEC;
	msec = msec + msec_increase; //update milliseconds count since game started
	if (msec >= 3000){
	    msec = 0;
	    check_time = count_down_timer();
	}
		
	//restart the game if any of the two conditions is true
	if (check_time || cat_caught_mouse){
	    if(cat_caught_mouse){
	        cat_caught_mouse_screen();
	    }else{
	        time_up_screen();
	    }
	    wait_for_vsync(); //swap front and back buffers on VGA vertical sync
	    pixel_buffer_start = *(pixel_ctrl_ptr + 1); //new back buffer
	    start_game = false;
	    KEY_value = *KEY_ptr;
	    while (!start_game) {
	        KEY_value = *KEY_ptr;
	        if (KEY_value == 0x4) {
	            start_game = true;
	        }
	    }
	    game_screen();
	    srand(time(NULL)); //initialize the maze
	    dfs(0, 0); map(); //plot maze
	    draw_entrance_exit();
	    wait_for_vsync();
	    pixel_buffer_start = *(pixel_ctrl_ptr + 1);
	    game_screen(); map();	//plot maze
	    draw_entrance_exit();
  
	    //initialize mouse position
	    dx_box = 0; dy_box = 0; x_box = 0; y_box = 120; 
	    old_x_box = x_box; old_y_box = y_box;

	    //initialize cat position
	    cat_random_direction = 3;
	    change_direction = false;
	    cat_dx_box = 0; cat_dy_box = 0; cat_x_box = 42; cat_y_box = 2;
	    cat_old_x_box = cat_x_box; cat_old_y_box = cat_y_box;

	    //initialize global variables again for level 1 timer
	    minutes = 2; seconds = 30; msec = 0;
	    //bool hex_display_on = count_down_timer();
	}	
        wait_for_vsync(); //swap front and back buffers on VGA vertical sync
        pixel_buffer_start = *(pixel_ctrl_ptr + 1); //new back buffer
    }
	
    level_one_win_screen();
	
    wait_for_vsync();
    pixel_buffer_start = *(pixel_ctrl_ptr + 1);
    
    //reset mouse position
    dx_box = 0; dy_box = 0; x_box = 0; y_box = 120;
    old_x_box = x_box; old_y_box = y_box;
	
    //reset cat position
    cat_random_direction = 3;
    change_direction = false;
    cat_dx_box = 0; cat_dy_box = 0; cat_x_box = 42; cat_y_box = 2;
    cat_old_x_box = cat_x_box; cat_old_y_box = cat_y_box;
	
    KEY_value = *KEY_ptr;
    while (!start_level_two) {
        KEY_value = *KEY_ptr;
        if (KEY_value == 0x2) {
	    start_level_two = true;
	    break;
	}
    }
	
    game_screen();
    //initialize the maze
    srand(time(NULL));
    dfs(0, 0);
    //plot maze
    map();
    draw_entrance_exit();
	
    wait_for_vsync();
    pixel_buffer_start = *(pixel_ctrl_ptr + 1);
    game_screen();
    //plot maze
    map();
    draw_entrance_exit();
	
    //initialize global variables for level 1 timer
    minutes = 1;
    seconds = 30;
    msec = 0;
    hex_display_on = count_down_timer();
	
    //start timer
    clock_t before_level_two = clock();
    while (!level_two) {
		
        clear_old_box(old_x_box, old_y_box);
	clear_old_box(cat_old_x_box, cat_old_y_box);
		
	*LED_ptr = 2;
		
	SW_value = *SW_ptr;
		
	map();
	draw_entrance_exit();
			
	draw_mouse(x_box, y_box);
	draw_cat(cat_x_box, cat_y_box);
		
	//update location of the mouse
	if ((SW_value == 0x1) && (x_box > 0) && (maze_limit(x_box,y_box,1) == false)) { //move left
	    dx_box = -1;
	    dy_box = 0;
	} else if ((SW_value == 0x2) && (x_box <= 299) && (maze_limit(x_box,y_box,2) == false)) { //move right
	    dx_box = 1;
	    dy_box = 0;
	} else if ((SW_value == 0x4) && (y_box > 0) && (maze_limit(x_box,y_box,3) == false)) { //move up
	    dx_box = 0;
	    dy_box = -1;
	} else if ((SW_value == 0x8) && (y_box <= 219) && (maze_limit(x_box,y_box,4) == false)) { //move down
	    dx_box = 0;
	    dy_box = 1;
	} else {
	    dx_box = 0;
	    dy_box = 0;
	}
				   
	old_x_box = x_box;
	old_y_box = y_box;	
	x_box+=dx_box;
	y_box+=dy_box;
		
	//update location of the cat
	if ((cat_random_direction == 1) && (cat_x_box > 41) && (maze_limit(cat_x_box,cat_y_box,1) == false)) { //move left
	    cat_dx_box = -1;
	    cat_dy_box = 0;
	} else if (cat_random_direction == 1) {
	    cat_dx_box = 0;
  	    cat_dy_box = 0;
	    change_direction = true;
	} else if ((cat_random_direction == 2) && (cat_x_box <= 260) && (maze_limit(cat_x_box,cat_y_box,2) == false)) { //move right
	    cat_dx_box = 1;
	    cat_dy_box = 0;
	} else if (cat_random_direction == 2) {
	    cat_dx_box = 0;
	    cat_dy_box = 0;
	    change_direction = true;
	} else if ((cat_random_direction == 3) && (cat_y_box > 2) && (maze_limit(cat_x_box,cat_y_box,3) == false)) { //move up
	    cat_dx_box = 0;
	    cat_dy_box = -1;
	} else if (cat_random_direction == 3) {
	    cat_dx_box = 0;
	    cat_dy_box = 0;
	    change_direction = true;
	} else if ((cat_random_direction == 4) && (cat_y_box <= 219) && (maze_limit(cat_x_box,cat_y_box,4) == false)) { //move down
	    cat_dx_box = 0;
	    cat_dy_box = 1;
	} else if (cat_random_direction == 4){
	    cat_dx_box = 0;
	    cat_dy_box = 0;
	    change_direction = true;
	}
			
	if ((change_direction == true) && (cat_random_direction == 1)) {
	    int numbers[] = {2,3,4};
	    cat_random_direction = numbers[rand()%3];
	    change_direction = false;
	} else if ((change_direction == true) && (cat_random_direction == 2)) {
	    int numbers[] = {1,3,4};
	    cat_random_direction = numbers[rand()%3];
	    change_direction = false;
	} else if ((change_direction == true) && (cat_random_direction == 3)) {
	    int numbers[] = {1,2,4};
	    cat_random_direction = numbers[rand()%3];
	    change_direction = false;
	} else if ((change_direction == true) && (cat_random_direction == 4)) {
	    int numbers[] = {1,2,3};
	    cat_random_direction = numbers[rand()%3];
	    change_direction = false;
	}

	cat_old_x_box = cat_x_box;
	cat_old_y_box = cat_y_box;	
	cat_x_box+=cat_dx_box;
	cat_y_box+=cat_dy_box;
		
	//check to see if mouse made it to the cheese (win)
	for (int i = 300; i<320; i++) {
	    if ((x_box == i) && (y_box == 113)){
	        level_two = true;
	    }
	}
	
        //check to see if cat caught mouse
	bool cat_caught_mouse = false;
	if( ((y_box>=cat_y_box && y_box<=cat_y_box+20)||(y_box+20>=cat_y_box && y_box+20<=cat_y_box+20))//mouse hits the cat on the left or right
		    &&((x_box == cat_x_box+20)||(x_box+20 == cat_x_box))){
	    cat_caught_mouse = true;	
	}
	if( ((x_box>=cat_x_box && x_box<=cat_x_box+20)||(x_box+20>=cat_x_box && x_box+20<=cat_x_box+20))//mouse hits the cat on the top or bottom
			&&((y_box+20 == cat_y_box)||(y_box == cat_y_box+20))){
	    cat_caught_mouse = true;
	}
		
	//check to see if time is out
	bool check_time = false;
	clock_t difference = (int)clock - before_level_two; //measure how much time passed in the while loop
	int msec_increase = difference / CLOCKS_PER_SEC;
	msec = msec + msec_increase; //update milliseconds count since game started
	if (msec_increase >= 3000){
	    msec = 0;
	    check_time = count_down_timer();
	}
		
	//restart the game if any of the two conditions is true
	if (check_time || cat_caught_mouse){
	    if(cat_caught_mouse){
	        cat_caught_mouse_screen();
	    }else{
	        time_up_screen();
	    }
	    wait_for_vsync(); //swap front and back buffers on VGA vertical sync
	    pixel_buffer_start = *(pixel_ctrl_ptr + 1); //new back buffer
	    start_game = false;
	    KEY_value = *KEY_ptr;

	    while (!start_game) {
	        KEY_value = *KEY_ptr;
		if (KEY_value == 0x4) {
		    start_game = true;
		}
	    }

	    game_screen();
            srand(time(NULL)); //initialize the maze
	    dfs(0, 0); map(); //plot maze
	    draw_entrance_exit();
	    wait_for_vsync();
	    pixel_buffer_start = *(pixel_ctrl_ptr + 1);
	    game_screen(); map(); //plot maze
	    draw_entrance_exit();

	    //initialize mouse position
	    dx_box = 0; dy_box = 0; x_box = 0; y_box = 120; 
	    old_x_box = x_box; old_y_box = y_box;

	    //initialize cat position
	    cat_random_direction = 3;
	    change_direction = false;
	    cat_dx_box = 0; cat_dy_box = 0; cat_x_box = 42; cat_y_box = 2;
	    cat_old_x_box = cat_x_box; cat_old_y_box = cat_y_box;

	    //initialize global variables again for level 1 timer
	    minutes = 2; seconds = 30; msec = 0;
	    //bool hex_display_on = count_down_timer();
	}
	
        wait_for_vsync();
        pixel_buffer_start = *(pixel_ctrl_ptr + 1);
    }
    game_over_screen();
    wait_for_vsync();
    pixel_buffer_start = *(pixel_ctrl_ptr + 1);
    game_over_screen();
}

//function definitions
//for the timer
bool count_down_timer() {
	if (minutes > 0 || seconds >= 0) {
		//display count
		display_sub(minutes, seconds);
		//decrement seconds count
		if(minutes > 0 && seconds <= 0) {
			minutes --;
			seconds = 59;
		} else {
			seconds --;
		}
		return false;
	}else{
		return true; //Time's over when exit while loop
	}
}

void display_sub(int min, int sec) {
	char seg7[] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F};
	volatile int *HEX_ptr = (int *)0xFF200020;
	
	//Display time left
	//HEX3-2 for minutes (MM), HEX1-0 for seconds (SS)
	int hex0_sec1 = sec % 10;
	int hex1_sec2 = (sec - (sec % 10)) / 10;
	int hex2_min1 = min % 10;
	int hex3_min2 =	(min - (min % 10)) / 10;
	
	*HEX_ptr = seg7[hex0_sec1] | seg7[hex1_sec2] << 8 | seg7[hex2_min1] << 16 | seg7[hex3_min2] << 24;
}

//for the maze:
//Fill dir with directions to unvisited cells, return count
int adjacent(int dir[], int x, int y) {
    int ndir = 0;
 
    if (y > 0     && visited[y - 1][x] == 0) dir[ndir++] = North;
    if (x < W - 1 && visited[y][x + 1] == 0) dir[ndir++] = East;
    if (y < H - 1 && visited[y + 1][x] == 0) dir[ndir++] = South;
    if (x > 0     && visited[y][x - 1] == 0) dir[ndir++] = West;
 
    return ndir;
}
 
//Traverse cells depth first and create paths as you go
void dfs(int x, int y) {
    int dir[NDir];
    int ndir;
 
    visited[y][x] = 1;
 
    ndir = adjacent(dir, x, y);
 
    while (ndir) {
        int pick = rand() % ndir;
 
        switch (dir[pick]) {
            case North: vert[y - 1][x] = 1; dfs(x, y - 1); break;
            case East:  horz[y][x] = 1;     dfs(x + 1, y); break;
            case South: vert[y][x] = 1;  	dfs(x, y + 1); break;
            case West:  horz[y][x - 1] = 1; dfs(x - 1, y); break;
        }
 
        ndir = adjacent(dir, x, y);
    }
}
 
//Print a map of the maze
void map(void) {	
	int pos_x = 40;
	int pos_y = 0;
	int i, j;
 
    for (i = 40; i < 280; i = i + shift) {
        plot_H_dash(i, pos_y, 0xFFFF); //putchar('_'); 
        plot_H_dash(i, pos_y, 0xFFFF); //putchar('_');
    }
 
    pos_y = pos_y + shift; //putchar('\n');
 
    for (j = 0; j < H; j++) {
        plot_V_dash(pos_x, pos_y); //putchar('|');
 
        for (i = 0; i < W; i++) {
            //putchar(j < H - 1 && vert[j][i] ? ' ' : '_');
			pos_x = 40 + shift*i;
			if(j < H - 1 && vert[j][i]){
				plot_H_dash(pos_x, pos_y, 0x0000);
			}else{
				plot_H_dash(pos_x, pos_y, 0xFFFF);
			}
			pos_x = pos_x + shift;
            //putchar(i < W - 1 && horz[j][i] ? '_' : '|');
			if(i < W - 1 && horz[j][i]){
				plot_H_dash(pos_x, pos_y, 0xFFFF);
			}else{
				plot_V_dash(pos_x, pos_y);
			}
        }
		pos_x = 40;
        pos_y = pos_y + shift;//putchar('\n');
    }
}

void plot_H_dash(int x, int y, short int color) {
	for(int i = x; i < x + shift; i++){
		plot_pixel(i, y, color); //can be white or black
	}
}

void plot_V_dash(int x, int y) {
	for(int i = y; i > y - shift; i--){
		plot_pixel(x, i, 0xFFFF); //white
	}
}

void draw_entrance_exit() {
	//entrace
	for(int y = 90; y < 150; y++){
		plot_pixel(40, y, 0x0000);
	}
	//exit
	for(int y = 90; y < 150; y++){
		plot_pixel(280, y, 0x0000); //black
	}
}

// Colors every pixel black from top left corner to bottom right corner
void clear_screen() {
	for (int y = 0; y < 240; ++y) {
		for (int x = 0; x < 320; ++x) {
			plot_pixel(x,y,0x0000);
		}
	}
}
//end maze functions

void start_screen() {
	
	int i = 0, j = 0;
	
	for (int k = 0; k<320*2*240-1; k+=2) {
		int red = ((start_screen_color[k+1]&0xF8)>>3)<<11;
		int green = (((start_screen_color[k]&0xE0)>>5)|((start_screen_color[k+1]&0x7)<<3));
	    int blue = (start_screen_color[k]&0x1F);
		
		short int screen_pixel_color = red|((green<<5)|blue);
		
		plot_pixel(i,j,screen_pixel_color);
		//plot_pixel(i,j,0xFFFF);
		i++;
		if (i == 320) {
			i=0;
			j++;
		}
	}
}

void game_screen() {
	
	int i = 0, j = 0;
	
	for (int k = 0; k<320*2*240-1; k+=2) {
		int red = ((game_screen_color[k+1]&0xF8)>>3)<<11;
		int green = (((game_screen_color[k]&0xE0)>>5)|((game_screen_color[k+1]&0x7)<<3));
	    int blue = (game_screen_color[k]&0x1F);
		
		short int screen_pixel_color = red|((green<<5)|blue);
		
		plot_pixel(i,j,screen_pixel_color);
		
		i++;
		if (i == 320) {
			i=0;
			j++;
		}
	}
}

void level_one_win_screen() {
	
    int i = 0, j = 0;
	
	for (int k = 0; k<320*2*240-1; k+=2) {
		int red = ((lvl_one_win_color[k+1]&0xF8)>>3)<<11;
		int green = (((lvl_one_win_color[k]&0xE0)>>5)|((lvl_one_win_color[k+1]&0x7)<<3));
	    int blue = (lvl_one_win_color[k]&0x1F);
		
		short int lvl_one_win_pixel_color = red|((green<<5)|blue);
		
		plot_pixel(i,j,lvl_one_win_pixel_color);
		
		i++;
		if (i == 320) {
			i=0;
			j++;
		}
	}
}

void time_up_screen() {
	
    int i = 0, j = 0;
	
	for (int k = 0; k<320*2*240-1; k+=2) {
		int red = ((time_up_color[k+1]&0xF8)>>3)<<11;
		int green = (((time_up_color[k]&0xE0)>>5)|((time_up_color[k+1]&0x7)<<3));
	    int blue = (time_up_color[k]&0x1F);
		
		short int time_up_pixel_color = red|((green<<5)|blue);
		
		plot_pixel(i,j,time_up_pixel_color);
		
		i++;
		if (i == 320) {
			i=0;
			j++;
		}
	}
}

void cat_caught_mouse_screen() {
	
    int i = 0, j = 0;
	
	for (int k = 0; k<320*2*240-1; k+=2) {
		int red = ((cat_caught_mouse_color[k+1]&0xF8)>>3)<<11;
		int green = (((cat_caught_mouse_color[k]&0xE0)>>5)|((cat_caught_mouse_color[k+1]&0x7)<<3));
	    int blue = (cat_caught_mouse_color[k]&0x1F);
		
		short int cat_caught_mouse_pixel_color = red|((green<<5)|blue);
		
		plot_pixel(i,j,cat_caught_mouse_pixel_color);
		
		i++;
		if (i == 320) {
			i=0;
			j++;
		}
	}
}


void game_over_screen() {
	
    int i = 0, j = 0;
	
	for (int k = 0; k<320*2*240-1; k+=2) {
		int red = ((game_over_color[k+1]&0xF8)>>3)<<11;
		int green = (((game_over_color[k]&0xE0)>>5)|((game_over_color[k+1]&0x7)<<3));
	    int blue = (game_over_color[k]&0x1F);
		
		short int game_over_pixel_color = red|((green<<5)|blue);
		
		plot_pixel(i,j,game_over_pixel_color);
		
		i++;
		if (i == 320) {
			i=0;
			j++;
		}
	}
}

void clear_old_box(int x, int y) {
	if ((x>0)&&(y>0)) {
	    for (int i = x-1; i < x+21; i++) {
		    for (int j = y-1; j < y+21; j++) {
			    plot_pixel(i,j,0x0000);
		    }
	    }
	} else if ((x==0)&&(y>0)){
		for (int i = x; i < x+21; i++) {
		    for (int j = y-1; j < y+21; j++) {
			    plot_pixel(i,j,0x0000);
		    }
	    }
	} else if ((y==0)&&(x>0)){
		for (int i = x-1; i < x+21; i++) {
		    for (int j = y; j < y+21; j++) {
			    plot_pixel(i,j,0x0000);
		    }
	    }
	} else {
		for (int i = x; i < x+21; i++) {
		    for (int j = y; j < y+21; j++) {
			    plot_pixel(i,j,0x0000);
		    }
	    }
	}
}

void wait_for_vsync() {
	volatile int *pixel_ctrl_ptr = (int*)0xFF203020; // pixel controller
	register int status;
	
	*pixel_ctrl_ptr = 1; // start the synchronization process
	
	//wait for s to become 0 in status register
	//of pixel buffer controller registers
	status = *(pixel_ctrl_ptr+3);	
	while ((status&0x01)!=0) {
		status = *(pixel_ctrl_ptr+3);
	}
}
	
void plot_pixel(int x, int y, short int line_color) {
    *(short int *)(pixel_buffer_start + (y << 10) + (x << 1)) = line_color;
}
//bool maze_limit(int x, int y, int direction_value);
//direction value: 1 for left, 2 for right, 3 for up, 4 for down
bool maze_limit(int x, int y, int direction_value) {
	if (direction_value == 1) {
		for (int i = y; i<y+20; i++) {
			if (*(short int *)(0xC8000000 + (i << 10) + ((x-1) << 1)) != (short int)0x0000) {
				return(true);
			}
		}
	} else if (direction_value == 2) {
		for (int i = y; i<y+20; i++) {
			if (*(short int *)(0xC8000000 + (i << 10) + ((x+20) << 1)) != (short int)0x0000) {
				return(true);
			}
		}
	} else if (direction_value == 3) {
		for (int j = x; j<x+20; j++) {
			if (*(short int *)(0xC8000000 + ((y-1) << 10) + ((j) << 1)) != (short int)0x0000) {
				return(true);
			}
		}
	} else if (direction_value == 4) {
		for (int j = x; j<x+20; j++) {
			if (*(short int *)(0xC8000000 + ((y+20) << 10) + ((j) << 1)) != (short int)0x0000) {
				return(true);
			}
		}
	}
	return(false);
}

void draw_mouse(int x, int y) {
	// draw a 20x20 box
	
	//go through this
	int i = x, j = y;
	
	for (int k = 0; k<20*2*20-1; k+=2) {
		int red = ((mouse_color[k+1]&0xF8)>>3)<<11;
		int green = (((mouse_color[k]&0xE0)>>5)|((mouse_color[k+1]&0x7)<<3));
	    int blue = (mouse_color[k]&0x1F);
		
		short int mouse_pixel_color = red|((green<<5)|blue);
		
		plot_pixel(i,j,mouse_pixel_color);
		i++;
		if (i == x+20) {
			i=x;
			j++;
		}
	} 
}

void draw_cat(int x, int y) {
	// draw a 20x20 box
	
	//go through this
	int i = x, j = y;
	
	for (int k = 0; k<20*2*20-1; k+=2) {
		int red = ((cat_color[k+1]&0xF8)>>3)<<11;
		int green = (((cat_color[k]&0xE0)>>5)|((cat_color[k+1]&0x7)<<3));
	    int blue = (cat_color[k]&0x1F);
		
		short int cat_pixel_color = red|((green<<5)|blue);
		
		plot_pixel(i,j,cat_pixel_color);
		i++;
		if (i == x+20) {
			i=x;
			j++;
		}
	} 
}

void swap(int *x, int *y) {
	int *temp = x;
	x = y;
	y = temp;
}
	