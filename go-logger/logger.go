package main

import (
	"bufio"
	"errors"
	"fmt"
	"log"
	"os"
	"strconv"
	"strings"
	"time"

	"github.com/influxdata/influxdb/client/v2"
	"github.com/tarm/serial"
)

const (
	MyDB       = "weather"
	username   = "weather"
	influxAddr = "https://home.saintaardvarkthecarpeted.com:26472"
)

type Measurement struct {
	Name  string
	Value float64
	Units string
}

// This lets me compare directly in the test file.  See:
// https://stackoverflow.com/questions/36091610/comparing-errors-in-go
var colonErr = fmt.Errorf("Can't find colon in line, don't know how to split it")

// type Message struct {
// 	Name         string
// 	Measurements []*Measurement
// }

// var readout Message

var measure, value string

func SplitLine(s string) (measure Measurement, err error) {
	// FIXME: Account for errors in all this
	m := Measurement{"", 0.0, ""}
	if strings.Index(s, ":") < 0 {
		return m, colonErr
	}
	s = strings.Trim(s, "{}")
	line := strings.Split(s, ":")
	m.Name = line[0]
	// Before parsing, need to remove units:
	// Humidity -> "%"
	// Pressure -> "hP"
	// Prcp -> "NA"
	// Temp -> "C"
	vals := strings.Fields(line[1])
	m.Units = vals[1]
	if m.Value, err = strconv.ParseFloat(vals[0], 64); err != nil {
		err = errors.New("Can't figure out value of that measurement")
	}
	return m, err
}

func main() {
	influxPass, exists := os.LookupEnv("INFLUXDB_PASS")
	if exists == false {
		log.Fatal("Can't proceed without environment var INFLUXDB_PASS!")
	}
	usbdev := "/dev/ttyUSB0"
	c := &serial.Config{Name: usbdev, Baud: 9600}
	s, err := serial.OpenPort(c)
	if err != nil {
		log.Fatal(err)
	}
	fmt.Println("Opened. Next up: reading.")
	reader := bufio.NewReader(s)
	reply, err := reader.ReadString('}')
	if err != nil {
		panic(err)
	}
	measure, err := SplitLine(reply)
	if err != nil {
		panic(err)
	}
	fmt.Printf("%s: %f\n", measure.Name, measure.Value)
	fmt.Println("Next up: connecting to InfluxDB.")
	// Create a new HTTPClient
	ic, err := client.NewHTTPClient(client.HTTPConfig{
		Addr:     influxAddr,
		Username: username,
		Password: influxPass,
	})
	if err != nil {
		log.Fatal(err)
	}
	// Create a new point batch
	bp, err := client.NewBatchPoints(client.BatchPointsConfig{
		Database:  MyDB,
		Precision: "s",
	})
	if err != nil {
		log.Fatal(err)
	}

	// Create a point and add to batch
	tags := map[string]string{
		"location": "BBY",
		"arduino":  "node1",
		"lat":      "49.284",
		"long":     "-123.021",
	}
	fields := map[string]interface{}{
		measure.Name: measure.Value,
	}

	measureAbbrevs := map[string]string{
		"humd": "humidity",
		"prcp": "precipitation",
		"pres": "pressure",
		"temp": "temperature",
	}
	pt, err := client.NewPoint(measureAbbrevs[measure.Name], tags, fields, time.Now())
	if err != nil {
		log.Fatal(err)
	}
	bp.AddPoint(pt)

	// Write the batch
	if err := ic.Write(bp); err != nil {
		log.Fatal(err)
	}
}
