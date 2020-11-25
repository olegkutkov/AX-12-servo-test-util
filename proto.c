/*
   proto.c
	AX12-A Protocol implementation

   Copyright 2020, Oleg Kutkov <contact@olegkutkov.me>

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

#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <poll.h>
#include <errno.h>
#include <string.h>

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "proto.h"

int read_answer(int fd, void *buf, size_t count)
{
	struct pollfd fds[1];
	int ret; 

	fds[0].fd = fd;
	fds[0].events = POLLIN;

	for (int i = 0; i < READ_POLL_RETRY_COUNT; i++) {
		ret = poll(fds, 1, READ_POLL_TIEMOUT_MS);

		if (ret < 0) {
			fprintf(stderr, "poll failed! Error: %s\n", strerror(errno));
			break;
		}

		if (ret == 0) {
#ifdef DEBUG
			fprintf(stderr, "poll timeout\n");
			break;
#endif
		}

		if (fds[0].revents == POLLIN) {
			ret = read(fd, buf, count);

			if (ret != count) {
				fprintf(stderr, "Failed to read %ld bytes from the device! Error: %s\n", count, strerror(errno));
				break;
			}

			return 0;

		} else {
#ifdef DEBUG
			fprintf(stderr, "poll error, revents = %d\n", fds[0].revents);
#endif
			break;
		}
	}

	return -1;
}

int servo_move(int fd, uint8_t id, uint16_t pos)
{
	uint8_t pos_hi, pos_lo;
    uint8_t packet[9];

	pos_hi = pos >> 8;
	pos_lo = pos;

	uint8_t checksum = (~(id + AX_GOAL_LENGTH + INSTRUCTION_WRITE + CMD_GOAL_POSITION + pos_lo + pos_hi)) & 0xFF;

    packet[0] = HEADER_1;
    packet[1] = HEADER_2;
    packet[2] = id;
    packet[3] = AX_GOAL_LENGTH;
    packet[4] = INSTRUCTION_WRITE;
    packet[5] = CMD_GOAL_POSITION;
    packet[6] = pos_lo;
    packet[7] = pos_hi;
    packet[8] = checksum;

	if (write(fd, &packet, sizeof(packet)) != sizeof(packet)) {
		fprintf(stderr, "Failed to write to the device! Error: %s\n", strerror(errno));

		return -1;
	}

	return 0;
}

uint16_t servo_get_position(int fd, uint8_t id)
{
	uint16_t pos;
	unsigned char packet[8];

	uint8_t checksum = (~(id + AX_POS_LENGTH + INSTRUCTION_READ + AX_PRESENT_POSITION_L 
						+ AX_BYTE_READ_POS)) & 0xFF;

	packet[0] = HEADER_1;
	packet[1] = HEADER_2;
	packet[2] = id;
	packet[3] = AX_POS_LENGTH;
	packet[4] = INSTRUCTION_READ;
	packet[5] = AX_PRESENT_POSITION_L;
	packet[6] = AX_BYTE_READ_POS;
	packet[7] = checksum;

	if (write(fd, &packet, sizeof(packet)) != sizeof(packet)) {
		fprintf(stderr, "Failed to write to the device! Error: %s\n", strerror(errno));

		return -1;
	}

	if (read_answer(fd, &packet, sizeof(packet)) != -1) {
		pos = packet[6] << 8;
		pos += packet[5]; 
	}

	return pos;
}

int servo_set_speed(int fd, uint8_t id, uint16_t speed)
{
	uint8_t speed_hi, speed_lo;
    uint8_t packet[9];

	speed_hi = speed >> 8;
	speed_lo = speed;

	uint8_t checksum = (~(id + AX_SPEED_LENGTH + INSTRUCTION_WRITE 
						+ CMD_GOAL_SPEED + speed_lo + speed_hi)) & 0xFF;

    packet[0] = HEADER_1;
    packet[1] = HEADER_2;
    packet[2] = id;
    packet[3] = AX_SPEED_LENGTH;
    packet[4] = INSTRUCTION_WRITE;
    packet[5] = CMD_GOAL_SPEED;
    packet[6] = speed_lo;
    packet[7] = speed_hi;
    packet[8] = checksum;

	if (write(fd, &packet, sizeof(packet)) != sizeof(packet)) {
		fprintf(stderr, "Failed to write to the device! Error: %s\n", strerror(errno));

		return -1;
	}

	return 0;

}

int servo_ping(int fd, uint8_t id)
{
	uint8_t ping_packet[6];
	uint8_t checksum = (~(id + INSTRUCTION_READ + CMD_PING)) & 0xFF;

	ping_packet[0] = HEADER_1;
	ping_packet[1] = HEADER_2;
	ping_packet[2] = id;
	ping_packet[3] = INSTRUCTION_READ;
	ping_packet[4] = CMD_PING;
	ping_packet[5] = checksum;

#ifdef DEBUG
	printf("writing: %02X %02X %02X %02X %02X %02X\n"
		, ping_packet[0], ping_packet[1], ping_packet[2], ping_packet[3], ping_packet[4], ping_packet[5]);
#endif
	if (write(fd, &ping_packet, sizeof(ping_packet)) != sizeof(ping_packet)) {
		fprintf(stderr, "Failed to write to the device! Error: %s\n", strerror(errno));

		return -1;
	}

	if (read_answer(fd, &ping_packet, sizeof(ping_packet)) != -1) {
		if (ping_packet[2] == id) {
			return 1;
		}
	}

	return 0;
}

int servo_set_id(int fd, uint8_t old_id, uint8_t new_id)
{
	uint8_t id_packet[8];

	uint8_t checksum = (~(old_id + AX_ID_LENGTH + INSTRUCTION_WRITE + CMD_SET_ID + new_id)) & 0xFF;

	id_packet[0] = HEADER_1;
	id_packet[1] = HEADER_2;
	id_packet[2] = old_id;
	id_packet[3] = AX_ID_LENGTH;
	id_packet[4] = INSTRUCTION_WRITE;
	id_packet[5] = CMD_SET_ID;
	id_packet[6] = new_id;
	id_packet[7] = checksum;

	if (write(fd, &id_packet, sizeof(id_packet)) != sizeof(id_packet)) {
		fprintf(stderr, "Failed to write to the device! Error: %s\n", strerror(errno));

		return -1;
	}

	return 0;
}

