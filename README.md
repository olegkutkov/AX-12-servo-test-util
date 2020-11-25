# AX-12-servo-test-util

**Get and compile**
  git clone https://github.com/olegkutkov/AX-12-servo-test-util.git
  cd AX-12-servo-test-util
  make

**Usage**

Mandatory params:
- -p, --port - Serial port device, example: -p /dev/ttyUSB0
- -b, --baud - Serial port baud rate, example: -b 115200

Servos lookup: 
  ./servo -b 1000000 -p /dev/ttyUSB0 -s

Ping servo with ID 2:
  ./servo -b 1000000 -p /dev/ttyUSB0 -i 2 -n

Change ID of the servo (old: 2, new: 5)
  ./servo -b 1000000 -p /dev/ttyUSB0 -i 2 -t 5

Get position of the servo with ID 1:
  ./servo -b 1000000 -p /dev/ttyUSB0 -i 1 -g
  
Set position 160 deg of the servo with ID 1:
  ./servo -b 1000000 -p /dev/ttyUSB0 -i 1 -e 160
  
Set rotation speed of the servo with ID 1:
  ./servo -b 1000000 -p /dev/ttyUSB0 -i 1 -f 20
