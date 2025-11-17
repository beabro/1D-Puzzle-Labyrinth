//#include <stdio.h>

//extern void enable_interrupt(void);
extern void print(const char*);
extern void print_dec(unsigned int);
extern void game_loop();

// declare global variables
static int SCREEN_WIDTH = 320;
static int SCREEN_HEIGHT = 240;
static int BAR_HEIGHT = 20;
static int FPS = 2;

void enable_interrupt() {
    /*
    enable_interrupt: # step 1: enable timer to send interrupts in control ITO
	csrsi mie,16 # let system accept interrupts from timer (cause 16)
	#csrsi mie,17 # accept interrupts form switch
	csrsi mstatus,3 # enable interrupts in machine status register
	jr ra */
    asm volatile ("csrsi mie,16");
    asm volatile ("csrsi mstatus,3");
}

void run_init(void) { // more init-actions?
  enable_interrupt();
  // prepare timer
  volatile int* timer_address = (volatile int*) 0x04000020; // to 0x0400003F
  int TO_period = (3000000-1)/FPS; // 0.1s for 30MHz clock
  *(timer_address+2) = TO_period & 0xffff; // 16 lsb in first TO register
  *(timer_address+3) = TO_period>>16; // 16 msb in second TO register

  // enable VGA control
  //volatile int* vga_address = (volatile int*) 0x04000100; // VGA adress
  //*(vga_address+3) = 4; // enable DMA controller ?

  // start the timer
  *(timer_address+1) = 0x7; // set ITO=1, CONT=1, START=1 in control register
}

int decode_color(int c) {
  /*
  0 --> white
  1 --> blue
  2 --> green
  3 --> red
  */
  int colors[] = {0xff, 0x2, 0x10, 0x80};
  if (c > sizeof(colors)/4) return 0;
  return colors[c];
}

void make_bar(volatile char* buffer, int colors[], int resolution) {
  int screen_middle = (SCREEN_WIDTH*(SCREEN_HEIGHT/2 -BAR_HEIGHT/2));
  for (int j = 0; j<BAR_HEIGHT; j++) {
    for (int i = 0; i<SCREEN_WIDTH; i++) {
      int color_index = ((i*resolution)/SCREEN_WIDTH);
      *(buffer+screen_middle+(SCREEN_WIDTH*j)+i) = decode_color(colors[color_index]);
    }
  }
}

void set_vga(int colors[], int resolution) {  // two buffers for smooth transitions between frames
  static int active_buffer = 0; // initialize buffer tracker
  volatile int* vga_address = (volatile int*) 0x04000100; // VGA adress
  volatile char* buffer0 = (volatile char*) 0x08000000;
  volatile char* buffer1 = (volatile char*) 0x08000000 + (0x257ff / 2); // works maybe
  
  // TEST, see only one buffer
  //*(buffer0+160+(320*120)) = 0xff;

  // determine which buffer to use
  if (active_buffer) { // edit buffer0
    make_bar(buffer0,colors,resolution);
    *(vga_address+1) = (int) buffer0; // set BackBuffer = buffer0
  } else {             // edit buffer1
    make_bar(buffer1,colors,resolution);
    *(vga_address+1) = (int) buffer1;
  }
  *(vga_address) = 1; // swap active_buffer
  active_buffer = !active_buffer; // track the swap
  //print_dec(active_buffer);
}

void handle_interrupt(unsigned cause) {
    static int timeoutcount = 0; // initialize the timeout counter
    volatile int* timer_address = (volatile int*) 0x04000020;
    //volatile int* switch_address = (volatile int*) 0x04000010;
    //volatile int* button_address = (volatile int*) TODO;

    switch (cause) {
        case 16: //timer
            *(timer_address) = 0; // reset status TO to 0 stops sending IRQ
            timeoutcount++;
            if (timeoutcount==10) { // activate every 0.1*10 s
                timeoutcount=0;
                //print("\nBUFFER: ");
                //print_dec(active_buffer);
                
                //game_loop();
                int test[] = {0,1,2,3,0,0};
                set_vga(test,sizeof(test)/4);
            }
            break;
        default:
            break;
    }
}

int main(void) {
    print("start!");
    run_init();
    //set_vga();
    
    while(1){

    }
}