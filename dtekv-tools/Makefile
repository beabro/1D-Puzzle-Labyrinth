CXX=g++
ifeq ($(shell uname -m),aarch64)
CC=x86_64-linux-gnu-$(CXX)
else
CC=$(CXX)
endif
CXXFLAGS=-O2 -Wall
LDFLAGS=-L. -Wl,-rpath=.
LDLIBS=-ljtag_atlantic -ljtag_client

all:
	$(CC) dtekv-run.c -o dtekv-run $(LDLIBS) $(LDFLAGS)
	$(CC) dtekv-upload.c -o dtekv-upload $(LDLIBS) $(LDFLAGS)
	$(CC) dtekv-download.c -o dtekv-download $(LDLIBS) $(LDFLAGS)

FILE_TO_RUN ?= ../hello_world/main.bin
# DTEKV_ARGS ?= --cable "USB-Blaster [1-7]"
run: clean all
	./dtekv-run $(FILE_TO_RUN) $(DTEKV_ARGS)

clean:
	rm -f dtekv-run dtekv-upload dtekv-download
