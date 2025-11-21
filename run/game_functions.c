// #include <stdio.h>
//#include <math.h>

extern void print(const char*);
extern void print_dec(unsigned int);
extern void set_vga(int colors[], int res);
extern int get_frame_time();

// constants ?
#define PI 3.14159265
#define RESOLUTION 10
int MAP_WIDTH = 1000;
int MAP_HEIGHT = 1000;
int step_length = 1;
double turn_length = PI/6;

// variables
double player_x;
double player_y;
double player_dir; // direction in radians (set big ?)
int colors[RESOLUTION];
int frame_time = 0;
int movement_allowed = 0;


int* get_colors() {
    return colors;
}

int get_btn(void) { // returns 1 while button is being pressed
  volatile int* btn_address = (volatile int*) 0x040000d0;
  return *btn_address &= 1; // just the least significant bit
}

int get_mv() {  // check if movement switches are active
    volatile int* switch_address = (volatile int*) 0x04000010;
    return *switch_address &= 1008; // check only switches 4,5,6,7,8,9 for now
}

void add_frame_time() { // make the game state tick for every frame
    frame_time++;
    if (get_mv()) movement_allowed++;
}

void print_coords() { // for testing
    print("\nx: ");
    print_dec(player_x*100);
    print(", y: ");
    print_dec(player_y*100);
    print(", dir: ");
    print_dec(player_dir*100);
}

/* ----------- some standard math -------------- */
int factorial(int n) {
    if (n <= 1) return 1;
    return factorial(n - 1) * n;
}
int pow(int b, int e) {
    int product = 1;
    for (int i = 0; i<e; i++) {
        product*=b;
    }
    return product;
}
double sin(double r) {
    double factor;
    double ans = 0;
    double accuracy = 4;
    for (int i = 0; i < accuracy; i++) {
        factor = pow(-1, i);
        double j = 2 * i + 1;
        double denominator = factorial((int) j);
        ans = ans + factor * pow(r, j) / denominator;
    }
    return ans;
}
double cos(double r) {
    double factor;
    double ans = 0;
    double accuracy = 4;
    for (int i = 0; i < accuracy; i++) {
        factor = pow(-1, i);
        double j = 2 * i;
        double denominator = factorial((int) j);
        ans = ans + factor * pow(r, j) / denominator;
    }
    return ans;
}
void translate_rotation() {
    if (player_dir > 2*PI) player_dir-=2*PI;
    else if (player_dir < 0) player_dir+=2*PI;
}

void move(int commands) {
    double dx = 0;
    double dy = 0;
    commands = commands >> 4; // shift to lsb
    // forward & backwards movement
    if (commands & 8 && !(commands & 4)) {  // go forward (switch 7)
        //print("forward");
        dx = step_length*sin(player_dir);
        dy = step_length*cos(player_dir);
    } else if (commands & 4 && !(commands & 8)) { // go backwards (switch 6)
        //print("back");
        dx = -step_length*sin(player_dir);
        dy = -step_length*cos(player_dir);
    } 
    // left & right movement
    if (commands & 16 && !(commands & 2)) { // go left (switch 8)
        //print("left");
        dy = step_length*sin(player_dir);
        dx = -step_length*cos(player_dir);
    } else if (commands & 2 && !(commands & 16)) { // go right (switch 5)
        //print("right");
        dy = -step_length*sin(player_dir);
        dx = step_length*cos(player_dir);
    } 
    // turning
    if (commands & 32 && !(commands & 1)) { // turn left (switch 9)
        player_dir-=turn_length;
        translate_rotation();
    } else if (commands & 1 && !(commands & 32)) { // turn right (switch 4)
        player_dir+=turn_length;
        translate_rotation();
    } 
    player_x += dx;
    player_y += dy;
    print_coords();
}

void handle_switches(void) { // TODO fix or remove
  volatile int* switch_address = (volatile int*) 0x04000010;
  int active_sw = *switch_address &= 1023;  // only get the 10 least significant bits
  if (active_sw & 960) {  // check bits 7,8,9,10 for now
    print_dec(frame_time);
  }
}

void game_init() {
    player_x = MAP_WIDTH/2;
    player_y = MAP_HEIGHT/3;
    int colors_test[] = {0,0,1,1,1,0,2,0,0,0};
    for (int i=0; i<(sizeof(colors_test)/(sizeof(colors_test[0]))); i++) {
        colors[i] = colors_test[i];
    }
}

void game_loop() {
    static int game_start = 1;
    if (game_start) {
        game_init(); // TODO fix later
        game_start = 0;
    }
    
    if (get_btn()) { // button reset, TODO fix something good
        player_x = 500;
        player_y = 500;
        player_dir = 0;
    }
    if (movement_allowed) { // deal with movement if allowed by timer
        move(get_mv());  
        movement_allowed--;
    }

    if (frame_time) {
        //print_dec(frame_time);
        //int colorsss[] = {0,0,1,1,1,0,2,0,0,0};
        set_vga(colors, RESOLUTION);
        frame_time--;
    }
}