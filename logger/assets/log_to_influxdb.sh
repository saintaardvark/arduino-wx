#!/bin/bash

# Hugh Brown
#
# log_to_influxdb.sh:  A Small but Useful(tm) utility to log lines to InfluxDB.
#
# Copyright (C) 2016 Hugh Brown
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330,
# Boston, MA 02111-1307, USA.

# Much help with parsing args from:
# http://rsalveti.wordpress.com/2007/04/03/bash-parsing-arguments-with-getopts/
# Much help with "Unofficial Bash strict mode" from:
# http://redsymbol.net/articles/unofficial-bash-strict-mode/

# set -eu

PATH=/bin:/usr/bin:/sbin:/usr/sbin:/usr/local/bin:/usr/local/sbin
SLEEPYTIME=60
DEVICE=/dev/ttyACM0
TEST_ONCE=0
VERBOSE=
# Sample curl command to log to InfluxDB:
# curl -i -XPOST 'http://localhost:8086/write?db=mydb' --data-binary 'cpu_load_short,host=server01,region=us-west value=0.64 1434055562000000000'
# Sample line from th emonitor
SAMPLE_LINE='Node: 1 , Temp: 22.70 C , Pres: 1010.20 hPA , Precip: 0 , Humid: 60.00 %'

usage() {
    cat << EOF
$0: A Small but Useful(tm) utility to log lines to InfluxDB.

This script requires the following environment variables to be set:

    INFLUX_DB (example: 'weather')
    INFLUX_USER (example: 'influxdb_user')
    INFLUX_PASS (example: 's00per5ekrit')
    INFLUX_URL (example: 'https://influxdb.example.com:1234' -- port number optional)

Usage: $0 options

OPTIONS:

    -v		Verbose: Show each line as it's being logged.
    -s [arg]     Sleep between sensor readings.  Default: $SLEEPYTIME.
    -d [arg]     Device to read from. Default: $DEVICE
    -t   	Test: run once with a test line.  Implies "Verbose".
    -h		This helpful message.
EOF
    exit 1
}

complain_and_die() {
    echo $*
    usage
}

read_line() {
    # echo $SAMPLE_LINE
    head -2 $DEVICE | tail -1
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
         "${INFLUX_URL}/write?db=${INFLUX_DB}" \
         --data-binary @${TMPFILE}
    if [[ $VERBOSE ]] ; then
        cat $TMPFILE
    fi
    rm $TMPFILE
    IFS=$OIFS
}

get_node() {
    echo $* | awk '{print $2}'
}

read_and_log() {
    if [[ $1 = 1 ]] ; then
        LINE=$SAMPLE_LINE
    else
        LINE=$(read_line)
    fi
    NODE=$(get_node $LINE)
    send_to_influxdb $NODE $LINE
}

while getopts "ts:vh" OPTION ; do
     case $OPTION in
         t) TEST_ONCE=1 ; VERBOSE=1 ;;
         v) VERBOSE=1 ;;
         s) SLEEPYTIME=$OPTARG ;;
         d) DEVICE=$OPTARG ;;
         h) usage ;;
         ?) usage ;;
     esac
done

# Check for mandatory args
if [ -z $INFLUX_USER ] ; then
    complain_and_die '$INFLUX_USER not set!'
elif [ -z $INFLUX_PASS ] ; then
    complain_and_die '$INFLUX_PASS not set!'
elif [ -z $INFLUX_URL ] ; then
    complain_and_die '$INFLUX_URL not set!'
elif [ -z $INFLUX_DB ] ; then
    complain_and_die '$INFLUX_DB not set!'
fi

while true; do
    read_and_log $TEST_ONCE
    if [[ $TEST_ONCE = 1 ]] ; then
        exit
    fi
    sleep $SLEEPYTIME
done
