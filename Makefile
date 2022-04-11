MCU = atmega328p
MCU2 = 328p
F_CPU = 16000000UL
CC = avr-gcc
OBJCOPY = avr-objcopy
CFLAGS = -std=c99 -Wall -g -Os -mmcu=${MCU} -DF_CPU=${F_CPU} -I.
PORT = /dev/ttyUSB0
TARGET = main
SRCS = Main.c
SRCS += PmemC.c
INCLUDE = PmemINC.h

all:
	${CC} ${CFLAGS} -o ${TARGET}.bin ${SRCS} ${INCLUDE}
	${OBJCOPY} -j .text -j .data -O ihex ${TARGET}.bin ${TARGET}.hex

flash:
	avrdude -p ${MCU} -c avr109 -U flash:w:${TARGET}.hex:i -F -P ${PORT}

clean: 
	rm -f *.bin *.hex

