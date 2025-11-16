//#include <stdio.h>

//extern void enable_interrupt(void);
extern void print(const char*);
extern void print_dec(unsigned int);

int timeoutcount = 0;
int active_buffer = 0;

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
  int TO_period = 3000000-1; // 0.1s for 30MHz clock
  *(timer_address+2) = TO_period & 0xffff; // 16 lsb in first TO register
  *(timer_address+3) = TO_period>>16; // 16 msb in second TO register

  // enable VGA control
  //volatile int* vga_address = (volatile int*) 0x04000100; // VGA adress
  //*(vga_address+3) = 4; // enable DMA controller ?

  // start the timer
  *(timer_address+1) = 0x7; // set ITO=1, CONT=1, START=1 in control register
}

void set_vga(int b) {  // two buffers for smooth transitions?
  volatile int* vga_address = (volatile int*) 0x04000100; // VGA adress
  volatile int* buffer0 = (volatile int*) 0x08000000;
  volatile int* buffer1 = (volatile int*) 0x08000000 + (0x257ff / 8); // works maybe
  
  //print("\nControl: ");
  //print_dec((int) *(vga_address+3));
  //*(vga_address+12) = 4; // enable DMA controller ?
  *(buffer0+3) = 0xff; // TEST: set buffer0 to whatever
  for (int i = 0; i<20; i++) {
    *(buffer1+i) = 0xffffffff;
  }
  if (b) {
    print("\n 0");
    *(vga_address+1) = (int) buffer0; // set BackBuffer = buffer0
  } else {
    print("\n 1");
    *(vga_address+1) = (int) buffer1;
  }
  *(vga_address) = 1; // swap buffer
}

void handle_interrupt(unsigned cause) {
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
                
                set_vga(active_buffer);

                active_buffer = !active_buffer; // switch active buffer
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