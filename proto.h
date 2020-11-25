/*
   proto.h
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

#ifndef __PROTO_H__
#define __PROTO_H__

#define DEVICE_ID_MIN 1
#define DEVICE_ID_MAX 256

#define READ_POLL_RETRY_COUNT 5
#define READ_POLL_TIEMOUT_MS 50

#define HEADER_1   0xFF
#define HEADER_2   0xFF

#define INSTRUCTION_READ             0x02
#define INSTRUCTION_WRITE            0x03
#define INSTRUCTION_REG_WRITE        0x04
#define INSTRUCTION_ACTION           0x05
#define INSTRUCTION_FACTORY_RESET    0x06
#define INSTRUCTION_REBOOT           0x08
#define INSTRUCTION_SYNC_WRITE       0x83
#define INSTRUCTION_BULK_READ        0x92

#define CMD_SET_ID           0x03
#define CMD_PING             0x01
#define CMD_GOAL_POSITION    0x1E
#define CMD_GOAL_SPEED       0x20

#define AX_PRESENT_POSITION_L  36
#define AX_BYTE_READ_POS       2

#define AX_GOAL_LENGTH      5
#define AX_POS_LENGTH       4
#define AX_ID_LENGTH        4
#define AX_SPEED_LENGTH     5

///

typedef struct instr_packet_hdr {
	uint8_t hdr1;
	uint8_t hdr2;
	uint8_t id;
	uint8_t len;
} instr_packet_hdr_t;

int servo_ping(int fd, uint8_t id);

int servo_set_id(int fd, uint8_t old_id, uint8_t new_id);

int servo_move(int fd, uint8_t id, uint16_t pos);
uint16_t servo_get_position(int fd, uint8_t id);
int servo_set_speed(int fd, uint8_t id, uint16_t speed);

#endif

