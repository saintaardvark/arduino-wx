#!/bin/bash

INFLUX_URL=${INFLUX_URL:https://saintaardvarkthecarpeted.com:26472}
INFLUX_DB=${INFLUX_DB:weather}
PATH=/bin:/usr/bin:/usr/local/bin/


if [[ -z $INFLUX_USER ]] ; then
    echo "INFLUX_USER unset!"
    exit 1
elif [[ -z $INFLUX_PASS ]] ; then
    echo "INFLUX_PASS unset!"
    exit 1
fi

docker run \
       -e INFLUX_USER=${INFLUX_USER} \
       -e INFLUX_PASS=${INFLUX_PASS} \
       -e INFLUX_URL=\"${INFLUX_URL}\" \
       -e INFLUX_DB=${INFLUX_DB} \
       --device=/dev/ttyACM0 \
       arduino_wx
