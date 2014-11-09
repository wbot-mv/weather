#!/bin/sh

#set GPIO 25 output
if [ ! -e /sys/class/gpio/gpio25 ]; then
  echo $(date): exporting GPIO... >> restarter.log
  echo "25" > /sys/class/gpio/export
  echo "out" > /sys/class/gpio/gpio25/direction
fi

if [ "$(cat /sys/class/gpio/gpio25/value)" == "0" ]; then
  echo "1" > /sys/class/gpio/gpio25/value
else
  echo "0" > /sys/class/gpio/gpio25/value
fi
