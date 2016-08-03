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
    NODE=$1
    shift
    LINE=$*
    OIFS=$IFS
    IFS=","
    for measurement in $LINE ; do
        if [[ $measurement == "Node"* ]] ; then
            continue
        fi
        measurement=$(echo $measurement | tr -d ":" | tr '[:upper:]' '[:lower:]')
        echo $measurement | awk -v"NODE=${NODE}" '{print $1 ",location=" NODE ",host=arduino-" NODE " value=" $2}' >> $TMPFILE
    done
    curl --silent \
         --request POST \
         -u ${INFLUX_USER}:${INFLUX_PASS} \
         "${INFLUX_URL}/write?db=${DB}" \
         --data-binary @${TMPFILE}
    rm $TMPFILE
    IFS=$OIFS
}

get_node() {
    echo $* | awk '{print $2}'
}

# LINE=$SAMPLE_LINE
LINE=$(read_line)

NODE=$(get_node $LINE)
send_to_influxdb $NODE $LINE
