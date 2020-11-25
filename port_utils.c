/*
   port_utils.c

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


#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>


speed_t baud_rate_to_speed_t(uint32_t baud)
{
#define B(x) case x: return B##x
	switch (baud) {
		B(50);     B(75);     B(110);    B(134);    B(150);
		B(200);    B(300);    B(600);    B(1200);   B(1800);
		B(2400);   B(4800);   B(9600);   B(19200);  B(38400);
		B(57600);  B(115200); B(230400); B(460800); B(500000); 
		B(576000); B(921600); B(1000000);B(1152000);B(1500000); 
	default:
		return 0;

	}
#undef B
}

/* Set specified baud rate
   Other params are constant:
    - NO PARITY
    - 1 STOP BIT
    - 8 DATA BITS
  */
void set_baud_rate(int fd, speed_t baud)
{
	struct termios settings;
	tcgetattr(fd, &settings);

	cfsetispeed(&settings, baud); /* input baud rate */
	cfsetospeed(&settings, baud); /* output baud rate */

	settings.c_cflag &= ~PARENB; // No parity
	settings.c_cflag &= ~CSTOPB; // Clear stop field, only one stop bit used in communication (most common)
	settings.c_cflag |= CS8; // 8 bits
	settings.c_cflag &= ~CRTSCTS; // Disable RTS/CTS hardware flow control (most common)
	settings.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)

	settings.c_lflag &= ~ICANON;
	settings.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
	settings.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL); // Disable any special handling of received bytes

	settings.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
	settings.c_oflag &= ~ONLCR;
	
	tcsetattr(fd, TCSANOW, &settings); /* apply the settings */
	tcflush(fd, TCOFLUSH);
}

int open_serial_dev(const char* dev, uint32_t baud)
{
	int serial_fd = 0;
	int fd_flags = 0;
	speed_t bs = baud_rate_to_speed_t(baud);

	if (!bs) {
		errno = EINVAL;
		return -1;
	}

	printf("Open %s and set baud rate = %d\n", dev, baud);

	serial_fd = open(dev, O_RDWR | O_NOCTTY);

	if (serial_fd == -1) {
		return -1;
	}

	set_baud_rate(serial_fd, bs);

	fd_flags = fcntl(serial_fd, F_GETFL, 0);

	fcntl(serial_fd, F_SETFL, fd_flags | O_NONBLOCK);

	return serial_fd;
}

int close_serial_dev(int fd)
{
	return close(fd);
}

