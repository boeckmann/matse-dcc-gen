#!/bin/sh

avrdude -c arduino -p m328p -P /dev/ttyACM0 -D -v -U flash:w:cmake-build-debug/rail03.hex

