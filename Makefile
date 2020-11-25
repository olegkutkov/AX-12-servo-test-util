#
# Copyright 2020, Oleg Kutkov <contact@olegkutkov.me>
#

CC := gcc
PROGRAM = servo
SRC := servo.c port_utils.c proto.c
CFLAGS := -Wall -std=gnu99
LDFLAG := -lpthread

.PHONY: $(PROGRAM)
all: $(PROGRAM)

$(PROGRAM): $(OBJECTS)
	$(CC) $(CFLAGS) $(SRC) $(LDFLAG) -o $(PROGRAM)

.PHONY: clean
clean:
	rm -fr $(PROGRAM) $(PROGRAM).o
