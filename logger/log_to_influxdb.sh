#!/bin/bash

PATH=/bin:/usr/bin:/sbin:/usr/sbin:/usr/local/bin:/usr/local/sbin

# curl -i -XPOST 'http://localhost:8086/write?db=mydb' --data-binary 'cpu_load_short,host=server01,region=us-west value=0.64 1434055562000000000'
SAMPLE_LINE="Node: 1 , Temp: 20.80 C , Pres: 1013.83 hPA , Humid: 61.50%"
INFLUX_URL=https://saintaardvarkthecarpeted.com:26472
DB=weather

read_line() {
    # echo $SAMPLE_LINE
    head -2 /dev/ttyACM0 | tail -1
}

send_to_influxdb() {
    TMPFILE=$(/bin/mktemp)
    NODE=$1;
    TEMP=$2;
    PRES=$3
    HUMID=$4
    echo "temp,location=${NODE},host=arduino-${NODE} value=${TEMP}" > $TMPFILE
    echo "pressure,location=${NODE},host=arduino-${NODE} value=${PRES}" >> $TMPFILE
    echo "humidity,location=${NODE},host=arduino-${NODE} value=${HUMID}" >> $TMPFILE
    curl --silent \
         --request POST \
         "${INFLUX_URL}/write?db=${DB}" \
         --data-binary @${TMPFILE}
    rm $TMPFILE
}

LINE=$(read_line)

NODE=$(echo $LINE | awk '{print $2}')
TEMP=$(echo $LINE | awk '{print $5}')
PRES=$(echo $LINE | awk '{print $9}')
HUMID=$(echo $LINE | awk '{print $13}' | tr -d '%')

send_to_influxdb $NODE $TEMP $PRES $HUMID
