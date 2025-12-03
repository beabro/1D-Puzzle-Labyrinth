// #include <stdio.h>
//#include <math.h>

extern void print(const char*);
extern void print_dec(unsigned int);
extern void set_vga(int colors[], int res);
extern int get_frame_time();
extern int decode_color(int c);

// constants ?
#define PI 3.14159265
#define RESOLUTION 320  // <= 320 because of lack of pixels
int MAP_WIDTH = 1000;
int MAP_HEIGHT = 1000;
int block_size = 25 ; // half side
int step_length = 5;
double turn_length = PI/30;


// variables
double player_x;
double player_y;
double player_dir; // direction in radians
int colors[RESOLUTION];
int active_map;
int obstacles[120]; // obstacle midpoints & color
int obstacle_range[4]; // {lowest x, highest x, lowest y, highest y}
int frame_time = 0;
int movement_allowed = 0;


int* get_colors() {
    return colors;
}
int get_map() {
    return active_map;
}

int get_btn(void) { // returns 1 while button is being pressed
  volatile int* btn_address = (volatile int*) 0x040000d0;
  return *btn_address &= 1; // just the least significant bit
}

// change levels with switches 0,1,2,3
int get_level_change(void) { // TODO fix or remove
  volatile int* switch_address = (volatile int*) 0x04000010;
  int active_sw = *switch_address &= 15;  // only get the 4 least significant bits
  return active_sw;
}

int get_mv() {  // check if movement switches are active
    volatile int* switch_address = (volatile int*) 0x04000010;
    return *switch_address &= 1008; // check only switches 4,5,6,7,8,9 for now
}

void add_frame_time() { // make the game state tick for every frame
    frame_time++;
    if (get_mv()) movement_allowed = 1;
}

// allows slight screen freeze on win? TODO
void delay(int seconds) {
}

// support print for play testing
void print_angle(double angle) {
    if (angle >= 0) print_dec(angle*100);
    else {
        print("-");
        print_dec(-angle*100);
    }
} // support print for play testing
void print_coords() { // for testing
    print("\nx: ");
    print_dec(player_x);
    print(", y: ");
    print_dec(player_y);
    print(", dir: ");
    print_angle(player_dir);
    print(", frame time: ");
    print_dec(frame_time);
}


void game_init(int map) { // TODO UI menu to manually change map?
    active_map = map; // set active map
    // initialize player start location // TODO make variable for different maps?
    player_x = MAP_WIDTH/2;
    player_y = MAP_HEIGHT/3;
    player_dir = 0;
    
    // reset obstacles
    for (int i = 0; i < sizeof(obstacles)/sizeof(obstacles[0]);i++) {
        obstacles[i]=0;
    }
                    // { x  y  color }
    // Level 0: simple only goal
    int obstacles0[] = {400,800,1};
    // Level 1: a few obstacles in front of the goal
    int obstacles1[] = {550,800,1,500,500,2, // cerise and green blocks
        350,600,3,400,600,3,450,600,3,500,600,3,550,600,3,600,600,3,650,600,3,700,600,3}; // red bar
    // Level 2: scattered obstacles, goal hidden behind one of them
    int obstacles2[] = {875,25,1,
        400,700,2,400,750,2,450,700,2,450,750,2,500,700,2,500,750,2,
        650,500,3,650,550,3,700,500,3,700,550,3,
        200,200,4,200,250,4,200,300,4,200,350,4,250,200,4,250,250,4,250,300,4,250,350,4,
        750,100,5,750,150,5,800,100,5,800,150,5};
    // Level 3: box with goal hidden inside
    int obstacles3[] = {350,400,3,400,400,3,450,400,3,500,400,3,550,400,3,600,400,3,650,400,3,
        350,450,3,350,500,3,350,550,3,350,600,3,350,650,3, // left wall
        650,450,3,650,500,3,650,550,3,650,600,3,650,650,3, // right wall
        450,650,3,500,650,3,550,650,3,  // top wall with holes
        400,450,4,450,450,4,500,450,4,550,450,4,600,450,4, // blue liner in box
        500,600,1};  // goal!
    // labyrinth with goal in the middle  (spawn 500,333)
    int obstacles4[] = {};

    // load the active map into obstacles
    switch (active_map) {
        case (0):
            for (int i = 0; i < sizeof(obstacles0)/sizeof(obstacles0[0]); i++) {
                obstacles[i] = obstacles0[i];
            }
            break;
        case (1):
            for (int i = 0; i < sizeof(obstacles1)/sizeof(obstacles1[0]); i++) {
                obstacles[i] = obstacles1[i];
            }
            break;
        case (2):
            for (int i = 0; i < sizeof(obstacles2)/sizeof(obstacles2[0]); i++) {
                obstacles[i] = obstacles2[i];
            }
            break;
        case (3):
            for (int i = 0; i < sizeof(obstacles3)/sizeof(obstacles3[0]); i++) {
                obstacles[i] = obstacles3[i];
            }
            break;
        case (4):
            for (int i = 0; i < sizeof(obstacles4)/sizeof(obstacles4[0]); i++) {
                obstacles[i] = obstacles4[i];
            }
            break;
        default:
            
    }
    
    

    /*
    static int x_range[] = {350, 400, 200, 900, 0, 0};
    static int y_range[] = {750, 800, 0, 800, 0, 0};
    obstacle_range[0] = x_range[(map*2)];
    obstacle_range[1] = x_range[(map*2)+1];
    obstacle_range[2] = y_range[(map*2)];
    obstacle_range[3] = y_range[(map*2)+1];
    */
}

/* ----------------- MAP CHECKS ------------------ */

int check_obstacle2(double x, double y) {
    for (int i = 0; i < sizeof(obstacles)/sizeof(obstacles[0]); i=i+3) {
        if (obstacles[i] || obstacles[i+1]) { // if obstacle exists
            int obstacle_x[] = {obstacles[i]-block_size, obstacles[i]+block_size};
            int obstacle_y[] = {obstacles[i+1]-block_size, obstacles[i+1]+block_size};
            if (x >= obstacle_x[0] && x <= obstacle_x[1] && y >= obstacle_y[0] && y <= obstacle_y[1]) {
                return obstacles[i+2]; // obstacle color
            }
        }
    }
    return 0;
}

/*
// return color code of obstacles, OLD VERSION
int check_obstacle(double x, double y) {
    switch (active_map) {
        case 0:  // simple map, only goal
            if (y > 750 && y < 800 && x > 350 && x < 400) return 1;
            break;

        case 1:  // test map with 1 obstacle
            if (y > 700 && y < 800 && x > 400 && x < 600) { // green block
                return 2;
            }
            else if (y > 500 && y < 650 && x > 600 && x < 800) { // red block
                return 3;
            }
            else if (y > 200 && y < 400 && x > 200 && x < 300) { // red block
                return 3;
            }
            else if (y > 100 && y < 200 && x > 700 && x < 800) { // red block
                return 3;
            }
            else if (y > 0 && y < 50 && x > 850 && x < 900) { // cerise win!
                return 1;
            }
            break;

        case 2:
            break;
        default:
            break;
    }
    
    return 0; // return 0 if no obstacle
}
*/

void check_for_win(double x, double y) {
    if (check_obstacle2(x, y) == 1) { // if win
        // print victory message
        print("\nLevel ");
        print_dec(active_map+1);
        print(" complete!");
        // start next map after delay
        delay(3);
        game_init(active_map+1);
    }
}

// return 1 if coordinates are within allowed limits
int check_in_bounds(double x, double y) {
    return x >= 0 && x <= MAP_WIDTH && y >= 0 && y <= MAP_HEIGHT;
}
/*
// return 1 if coordinates are out of range of all obstacles, OLD VERSION
int out_of_obstacle_range(double x, double y, double dir) {
    if (x <= obstacle_range[0] && dir < 0) return 1;
    if (x >= obstacle_range[1] && dir > 0) return 1;
    if (y <= obstacle_range[2] && (dir < -PI/2 || dir > PI/2)) return 1;
    if (y >= obstacle_range[3] && !(dir < -PI/2 || dir > PI/2)) return 1;
    return 0;
}
*/

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
double sin(double r) { // works for r = [-pi, pi]
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
double cos(double r) { // works for r = [-pi, pi]
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
double arctan(double r) { // works for r = [-1,1]
    double factor;
    double ans = 0;
    double accuracy = 8;
    for (int i = 0; i < accuracy; i++) {
        factor = pow(-1, i);
        double j = 2 * i + 1;
        ans += factor * pow(r, j) / j;
    }
    return ans;
}
double get_angle(double x, double y) { // return angle to coords
    double dx = x-player_x;
    double dy = y-player_y;
    if (dy > 0) {
        if (dx/dy <= 1 && dx/dy >= -1) { // (x,y) above player
            return arctan(dx/dy); 
        }
    } else if (dy < 0) {
        if (dx/dy <= 1 && dx/dy >= -1) { // (x,y) below player
            return arctan(dx/dy) + PI; 
        }
    }
    if (dx > 0) {
        if (dy/dx <= 1 && dy/dx >= -1) { // (x,y) right of player
            return -arctan(dy/dx) + PI/2;
        }
    } else if (dx < 0) {
        if (dy/dx <= 1 && dy/dx >= -1) { // (x,y) left of player
            return -arctan(dy/dx) - PI/2;
        }
    } 
    return 0; // should only be if dx=dy=0 --> never
}
double sqrt(double S) {
    double d = 1;
    double ans = S/2;
    while (d > 0.1) {
        d = ans - (ans+(S/ans))/2;
        ans -= d;
    }
    return ans;
}
double abs(double S) {
    if (S >= 0) return S;
    else return -S;
}
double translate_rotation(double dir) {
    if (dir > PI) return dir-=2*PI;
    else if (dir < -PI) return dir+=2*PI;
    return dir;
}
double translate_small_rotation(double dir) {
    while (dir >= PI/4 || dir <= -PI/4) {
        if (dir >= PI/4) dir -= PI/2;
        else dir += PI/2;
    }
    return dir;
}
double hypotenuse(double dx, double dy) {
    return sqrt((dx*dx) + (dy*dy));
}


/* ----------------- player movement ------------------------ */

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

    // check movement for win and obstacles
    check_for_win(player_x+dx, player_y+dy);
    if (check_in_bounds(player_x+dx, player_y+dy) && !check_obstacle2(player_x+dx, player_y+dy)) {
        player_x += dx;
        player_y += dy;
    }
    print_coords();
}


/* ------------ output calculations --------------------- */

/*
// probe for the first color encountered in given direction, OLD VERSION
int check_color(double dir) {
    double sin_dir = sin(dir);
    double cos_dir = cos(dir);
    double check_x = player_x;
    double check_y = player_y;
    if (out_of_obstacle_range(check_x,check_y,dir)) {
        return 0;
    }
    while(check_in_bounds(check_x,check_y)) {
        // see if we encounter any obstacles before going out of bounds
        int obstacle = check_obstacle2(check_x,check_y);
        if (obstacle) {
            return obstacle;
        }
        // move along in direction
        check_x += 1*step_length*sin_dir;
        check_y += 1*step_length*cos_dir;
    }
    return 0; // make color white if we reach the edge
} 
// determine colors based on what player sees in RESOLUTION nbr of directions, OLD VERSION
void look() {
    double direction = player_dir+PI; // start behind player
    for (int i = 0; i<RESOLUTION; i++) {
        colors[i] = check_color(translate_rotation(direction));
        //colors[i] = i+192;
        direction+=(2*PI / RESOLUTION);
    }
}    
*/

// return an objects total occupation angle in view
double get_view(double distance, double size) {
    if (distance-size > 0) {
        if (size/(distance-size) <= 1) {
            return arctan(size/(distance-size))*2;
        }
    } // when player is too close to obstacle for the arctan to work
    //return PI - abs(PI*((distance-block_size)/block_size)/2); // make this nice somehow, EDIT: this was not nice :(
    return PI/2; // TODO: fix with dx and dy?
}

// second generation look function, allows better resolution but with some accuracy issues
void look2() {
    // initialize distance with larger than possible values
    int distance[RESOLUTION];
    for (int i = 0; i < RESOLUTION; i++) {
        distance[i] = MAP_HEIGHT+MAP_WIDTH;
        colors[i] = 0; // reset colors between looks
    }
    for (int i = 0; i < sizeof(obstacles)/sizeof(obstacles[0]); i=i+3) {
        if (obstacles[i+2]) { // if obstacle not white --> exists
            // get angle between player and object (not player_dir yet)
            double angle = translate_rotation(get_angle(obstacles[i],obstacles[i+1]));
            // get object distance from player
            double object_distance = hypotenuse(obstacles[i]-player_x, obstacles[i+1]-player_y);
            // account for block diagonals being larger (maybe slightly useless)
            double extra_block_size = abs(block_size*sin(translate_small_rotation(angle)));
            // adjust angle to be between player_dir and player-->coords
            angle -= player_dir; 
            // get total angle of object in player view
            double view = get_view(object_distance,block_size+extra_block_size); // TODO: check this actually works
            // calculate how many indices of RESOLUTION that is
            int occupied_chunks = ((RESOLUTION * view / PI) / 2) +1; // +1 to err upwards
            // find start obstacle start index in player view
            int object_index = ((RESOLUTION*(angle+PI)/PI)/2)-occupied_chunks/2;
            if (object_index<0) object_index+=RESOLUTION;

            // assign colors, check distance to not overwrite closer colors
            while (occupied_chunks>0) {
                if (object_distance < distance[object_index]) {
                    colors[object_index] = obstacles[i+2];
                    distance[object_index] = object_distance;
                }
                object_index++;
                // loop around index if reached end of array
                if (object_index>=RESOLUTION) object_index-=RESOLUTION;
                occupied_chunks--;
            }
        }
    }
}


/* -------- main loop ------------------------- */

void game_loop() {
    static int game_start = 1;
    if (game_start) {
        game_init(0); // start with map 0
        game_start = 0;
    }
    
    if (get_btn()) { // button reset, change map to value of first 4 switches
        game_init(get_level_change());
        look2();
    }
    if (movement_allowed) { // deal with movement if allowed by timer
        move(get_mv());  
        movement_allowed--;
        look2();
    }

    if (frame_time) {
        set_vga(colors, RESOLUTION);
        frame_time--;
    }
}