// #include <stdio.h>

extern void print(const char*);
extern void print_dec(unsigned int);
extern void set_vga(int colors[], int res);
extern int get_frame_time();

// constants ?
#define RESOLUTION 10
int MAP_WIDTH = 1000;
int MAP_HEIGHT = 1000;

// variables
int player_x;
int player_y;
int player_dir; // direction in radians
int colors[RESOLUTION];
int frame_time = 0;


int* get_colors() {
    return colors;
}

void add_frame_time() {
    frame_time++;
}

void game_init() {
    int colors_test[] = {0,0,1,1,1,0,2,0,0,0};
    for (int i=0; i<(sizeof(colors_test)/(sizeof(colors_test[0]))); i++) {
        colors[i] = colors_test[i];
    }
}

void game_loop() {
    game_init();
    if (frame_time) {
        //print_dec(frame_time);
        //int colorsss[] = {0,0,1,1,1,0,2,0,0,0};
        set_vga(colors, RESOLUTION);
        frame_time--;
    }
}