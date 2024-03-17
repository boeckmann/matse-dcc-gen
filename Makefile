# AVR-GCC makefile for Arduino UNO R3 LED example

# call "make" to build the program
# call "make upload" to upload to Arduino UNO
#       or "make upload PORT=... BAUD=..." to define COM port and speed for upload

NAME=matse-dcc-gen
DEVICE=atmega328p
FREQ=16000000
OBJS=main.o stream.o cmd.o isr_dcc.o serial.o ticks.o track.o train.o util.o version.o

# programmer settings
PORT?=/dev/cu.usbmodem1101
BAUD?=115200
PROGRAMMER=arduino

# compiler and linker flags
CFLAGS=-Os -g -std=c99 -Wall -mmcu=$(DEVICE) -DF_CPU=$(FREQ)UL
LFLAGS=-mmcu=$(DEVICE) -Wl,--gc-sections
# the following is a fix for GCC>=12 bug https://gcc.gnu.org/bugzilla/show_bug.cgi?id=105523
# uncomment for GCC>=12, GCC<=13.2 
#CFLAGS+=--param=min-pagesize=0

# set commands
CC=avr-gcc
LD=avr-gcc
AVRDUDE=avrdude


ifdef OS
	RM = del /Q
else
	RM = rm -f
endif

$(NAME).elf: $(OBJS)
	$(LD) $(LFLAGS) -o $@ $(OBJS)

upload: $(NAME).elf
	$(AVRDUDE) -p $(DEVICE) -P $(PORT) -b $(BAUD) -c $(PROGRAMMER) -D -U flash:w:$<

clean:
	-$(RM) $(NAME).elf
	-$(RM) *.o
