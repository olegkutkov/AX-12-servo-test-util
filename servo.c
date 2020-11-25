/*
   servo.c
	Test & service program for the Dynamixel AX-12A servos (protocol version 1.0)

   Copyright 2020  Oleg Kutkov <contact@olegkutkov.me>

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  
 */


#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include "port_utils.h"
#include "proto.h"

static struct option cmd_long_options[] =
{
	{ "port", required_argument, 0, 'p' },
	{ "baud", required_argument, 0, 'b' },
	{ "scan", no_argument, 0, 's' },
	{ "id", required_argument, 0, 'i' },
	{ "set-id", required_argument, 0, 't' },
	{ "ping", no_argument, 0, 'n' },
	{ "set-position", required_argument, 0, 'e' },
	{ "get-position", no_argument, 0, 'g' },
	{ "set-speed", required_argument, 0,'f' },
    { 0, 0, 0, 0 }
};

typedef enum user_cmd {
	USER_CMD_NO_CMD = 0,
	USER_CMD_SCAN,
	USER_CMD_SET_ID,
	USER_CMD_PING,
	USER_CMD_SET_POS,
	USER_CMD_GET_POS,
	USER_CMD_SET_SPEED
} user_cmd_t;

void do_servo_ping(int fd, uint8_t id)
{
	printf("Ping servo with ID = %d... ", id);
	fflush(stdout);

	if (servo_ping(fd, id) == 1) {
		printf("Success!\n");
	} else {
		printf("Failed!\n");
	}
}

void do_scan_bus(int fd)
{
	int ret;

	for (int id = DEVICE_ID_MIN; id < DEVICE_ID_MAX; id++) {
		ret = servo_ping(fd, id); 

		if (ret == -1) {
			break;
		}

		if (ret > 0) {
			printf("Found device with ID = %d\n", id);
		}

		usleep(100);
	}
}

void do_set_id(int fd, uint8_t id, uint8_t new_id)
{
	printf("Setting servo id=%i new id=%i\n", id, new_id);

	servo_set_id(fd, id, new_id);
}

void do_set_position(int fd, uint8_t id, uint16_t pos)
{
	uint16_t pos_dec = (uint16_t)((float)pos / 0.293);

	printf("Moving servo %d to position %d (DEC: %d)\n", id, pos, pos_dec);

	servo_move(fd, id, pos_dec);
}

void do_get_position(int fd, int id)
{
	uint16_t dec_pos = servo_get_position(fd, id);
	float pos = (float)dec_pos * 0.293;

	printf("Servo ID %d current position %02f (DEC: %d)\n", id, pos, dec_pos);
}

void do_set_speed(int fd, uint8_t id, uint16_t speed)
{
	servo_set_speed(fd, id, speed);
}

int execute_cmd(char *port, uint32_t baud, uint8_t id, user_cmd_t cmd, uint16_t arg)
{
	int fd = open_serial_dev(port, baud);

	if (fd == -1) {
		printf("Failed to open serial device %s, error: %s\n", port, strerror(errno));
		return -1;
	}

	switch (cmd) {
		case USER_CMD_SCAN:	
			do_scan_bus(fd);
			break;

		case USER_CMD_PING:
			do_servo_ping(fd, id);
			break;

		case USER_CMD_SET_ID:
			do_set_id(fd, id, (uint8_t)arg);
			break;

		case USER_CMD_SET_POS:
			do_set_position(fd, id, arg);
			break;

		case USER_CMD_GET_POS:
			do_get_position(fd, id);
			break;

		case USER_CMD_SET_SPEED:
			do_set_speed(fd, id, arg);
			break;

		default:
			break;
	}

	return close_serial_dev(fd);
}

int main(int argc, char *argv[])
{
	int c;
	int option_index;

	char *port = NULL;
	uint32_t baud = 0;
	uint8_t id = 0;
	uint16_t arg = 0;

	user_cmd_t ucmd = USER_CMD_NO_CMD;

	while (1) {
		option_index = 0;

		c = getopt_long(argc, argv, "p:b:si:pe:g:", cmd_long_options, &option_index);

		if (c == -1) {
			break;
		}

		switch (c) {
			case 'p':
				port = optarg;
				break;

			case 'b':
				baud = atoi(optarg);
				break;

			case 's':
				ucmd = USER_CMD_SCAN;
				break;

			case 'i':
				id = atoi(optarg);
				break;

			case 'e':
				arg = (uint16_t) atoi(optarg);
				ucmd = USER_CMD_SET_POS;
				break;

			case 'g':
				ucmd = USER_CMD_GET_POS;
				break;

			case 'n':
				ucmd = USER_CMD_PING;
				break;

			case 't':
				ucmd = USER_CMD_SET_ID;
				arg = (uint16_t) atoi(optarg);
				break;

			case 'f':
				ucmd = USER_CMD_SET_SPEED;
				arg = (uint16_t) atoi(optarg);
				break;
	
		}
	}

	if (!port) {
		fprintf(stderr, "Please set serial port device name\n");
		return -1;
	}

	if (!baud) {
		fprintf(stderr, "Please specify the baud rate\n");
		return -1;
	}

	if (ucmd != USER_CMD_SCAN && !id) {
		fprintf(stderr, "Please set target device id\n");
		return -1;
	}

	return execute_cmd(port, baud, id, ucmd, arg);
}

