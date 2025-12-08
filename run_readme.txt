This project uses the dtekv toolchain to run code on the DTEK-V board, it is made to run with the same process we used in the labs in the course.

To run with linux:

1) Compile the code: use the "make" command in the game folder "game_files".

2) Make sure the jtag connection is active with: 
	killall jtagd
	jtagd --user-start
	jtagconfig

3) Reset the board by pressing the upper button.

4) Run the code: from the "dtekv-tools" folder, use the following command:
	./dtekv-run ../game_files/main.bin

Enjoy :)
