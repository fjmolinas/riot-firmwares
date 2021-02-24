#!/bin/bash

: ${IPV6_PREFIX="2001:db8::/64"}
: ${TAP=riot0}
: ${BAUDRATE=115200}
: ${PORT=/dev/ttyACM0}

./ethos/start-network.sh ${PORT} ${TAP} ${IPV6_PREFIX} ${BAUDRATE}

