// #include <stdio.h>
//#include <math.h>

extern void print(const char*);
extern void print_dec(unsigned int);
extern void set_vga(int colors[], int res);
extern int get_frame_time();
extern int decode_color(int c);

// constants ?
#define PI 3.14159265
#define RESOLUTION 80  // < 320 because of lack of pixels
int MAP_WIDTH = 1000;
int MAP_HEIGHT = 1000;
int step_length = 5;
double turn_length = PI/10;

// variables
double player_x;
double player_y;
double player_dir; // direction in radians
int colors[RESOLUTION];
int map;
int obstacle_range[4]; // {lowest x, highest x, lowest y, highest y}
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
    if (get_mv()) movement_allowed = 1;
}

void print_coords() { // for testing
    print("\nx: ");
    print_dec(player_x);
    print(", y: ");
    print_dec(player_y);
    print(", dir: ");
    print_dec(player_dir*100);
    print(", frame time: ");
    print_dec(frame_time);
}

/* ----------------- MAP MAKING ------------------ */

// TODO connect to map making ????
// return color code of obstacles
int check_obstacle(double x, double y) {
    switch (map) {
        case 0:  // simple map, only goal
            if (y < 800 && y > 750 && x < 400 && x > 350) return 1;
            break;
        case 1:  // test map with 1 obstacle
            if (y < 800 && y > 700 && x > 400 && x < 600) { // green block
                return 2;
            }
            else if (y < 700 && y > 600 && x > 450 && x < 550) { // red block
                return 3;
            }
            else if (y < 1000 && y > 950 && x < 400 && x > 350) { // cerise win!
                return 1;
            }
            break;
    }
    
    return 0; // return 0 if no obstacle
}

int win_condition(double x, double y) {
    if (check_obstacle(x, y) == 1) {
        return 1;
    }
    return 0;
}

// return 1 if coordinates are within allowed limits
int check_out_of_bounds(double x, double y) {
    return x >= 0 && x <= MAP_WIDTH && y >= 0 && y <= MAP_HEIGHT;
}
// return 1 if coordinates are out of range of all obstacles
int out_of_obstacle_range(double x, double y, double dir) {
    if (x <= obstacle_range[0] && dir < 0) return 1;
    if (x >= obstacle_range[1] && dir > 0) return 1;
    if (y <= obstacle_range[2] && (dir < -PI/2 || dir > PI/2)) return 1;
    if (y >= obstacle_range[3] && (dir > -PI/2 || dir < PI/2)) return 1;
    return 0;
}

/* ----------- some standard math -------------- */

int factorial(int n) {
    if (n <= 1) return 1;
    return factorial(n - 1) * n;
}
double pow(double b, int e) {
    double product = 1;
    for (int i = 0; i<e; i++) {
        product*=b;
    }
    return product;
}
double sin(double r) {
    char neg = (r > PI/2 || r < -PI/2);
    if (r > PI/2) r -= PI;
    else if (r < -PI/2) r+= PI;
    double factor;
    double ans = 0;
    double accuracy = 4;
    for (int i = 0; i < accuracy; i++) {
        factor = pow(-1, i);
        double j = 2 * i + 1;
        double denominator = factorial((int) j);
        ans += factor * pow(r, j) / denominator;
    }
    if (neg) ans = -ans;
    return ans;
}
double cos(double r) {
    char neg = (r > PI/2 || r < -PI/2);
    if (r > PI/2) r -= PI;
    else if (r < -PI/2) r+= PI;
    double factor;
    double ans = 0;
    double accuracy = 4;
    for (int i = 0; i < accuracy; i++) {
        factor = pow(-1, i);
        double j = 2 * i;
        double denominator = factorial((int) j);
        ans += factor * pow(r, j) / denominator;
    }
    if (neg) ans = -ans;
    return ans;
}
double translate_rotation(double dir) {
    if (dir > PI) return dir-=2*PI;
    else if (dir < -PI) return dir+=2*PI;
    return dir;
}

void move(int commands) {
    double dx = 0;
    double dy = 0;
    commands = commands >> 4; // shift to lsb
    // forward & backwards movement
    if (commands & 8 && !(commands & 4)) {  // go forward (switch 7)
        dx = step_length*sin(player_dir);
        dy = step_length*cos(player_dir);
    } else if (commands & 4 && !(commands & 8)) { // go backwards (switch 6)
        dx = -step_length*sin(player_dir);
        dy = -step_length*cos(player_dir);
    } 
    // left & right movement
    if (commands & 16 && !(commands & 2)) { // go left (switch 8)
        dy = step_length*sin(player_dir);
        dx = -step_length*cos(player_dir);
    } else if (commands & 2 && !(commands & 16)) { // go right (switch 5)
        dy = -step_length*sin(player_dir);
        dx = step_length*cos(player_dir);
    } 
    // turning
    if (commands & 32 && !(commands & 1)) { // turn left (switch 9)
        player_dir=translate_rotation(player_dir-turn_length);
    } else if (commands & 1 && !(commands & 32)) { // turn right (switch 4)
        player_dir=translate_rotation(player_dir+turn_length);
    } 
    if (win_condition(player_x+dx, player_y+dy)) {
        print("Win!");
    }
    if (check_out_of_bounds(player_x+dx, player_y+dy) && !check_obstacle(player_x+dx, player_y+dy)) {
        player_x += dx;
        player_y += dy;
    }
    print_coords();
}



int check_color(double dir) {
    double sin_dir = sin(dir);
    double cos_dir = cos(dir);
    double check_x = player_x;
    double check_y = player_y;
    if (out_of_obstacle_range(check_x,check_y,dir)) {
        return 0;
    }
    while(check_out_of_bounds(check_x,check_y)) {
        // see if we encounter any obstacles before going out of bounds
        int obstacle = check_obstacle(check_x,check_y);
        if (obstacle) {
            return obstacle;
        }
        // move along in direction
        check_x += 1*step_length*sin_dir;
        check_y += 1*step_length*cos_dir;
    }
    return 0; // make color white if we reach the edge
}

// determine colors based on what player sees in RESOLUTION nbr of directions
void look() {
    double direction = player_dir+PI; // start behind player
    for (int i = 0; i<RESOLUTION; i++) {
        colors[i] = check_color(translate_rotation(direction));
        //colors[i] = i+192;
        direction+=(2*PI / RESOLUTION);
    }
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
    
    map = 0; // TODO variable

    static int x_range[] = {350, 400};
    static int y_range[] = {750, 800};
    obstacle_range[0] = x_range[map];
    obstacle_range[1] = x_range[map+1];
    obstacle_range[2] = y_range[map];
    obstacle_range[3] = y_range[map+1];
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
        look();
    }

    if (frame_time) {
        set_vga(colors, RESOLUTION);
        frame_time--;
    }
}