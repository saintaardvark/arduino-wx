#!/bin/bash

docker kill arduino_wx
docker rm arduino_wx
./build.sh
./run.sh
