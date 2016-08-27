#!/bin/bash

PATH=/bin:/usr/bin:/usr/local/bin/

INFLUX_URL=${INFLUX_URL:-https://saintaardvarkthecarpeted.com:26472}
INFLUX_DB=${INFLUX_DB:-weather}

if [[ -z $INFLUX_USER ]] ; then
    echo "INFLUX_USER unset!"
    exit 1
elif [[ -z $INFLUX_PASS ]] ; then
    echo "INFLUX_PASS unset!"
    exit 1
fi

ENV_FILE=$(mktemp)
trap "rm $ENV_FILE; exit" SIGHUP SIGINT SIGTERM

cat << EOF > $ENV_FILE
INFLUX_USER=$INFLUX_USER
INFLUX_PASS=$INFLUX_PASS
INFLUX_URL=$INFLUX_URL
INFLUX_DB=$INFLUX_DB
EOF

docker rm arduino_wx

docker run \
       --env-file=$ENV_FILE \
       --device=/dev/ttyACM0 \
       --detach=true \
       --name=arduino_wx \
       arduino_wx

rm $ENV_FILE
