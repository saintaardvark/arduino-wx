package main

import (
	"bufio"
	"errors"
	"fmt"
	"log"
	"strconv"
	"strings"
	"time"

	"github.com/tarm/serial"
)

const (
	influxDB   = "weather"
	influxUser = "weather"
	influxAddr = "https://home.saintaardvarkthecarpeted.com:26472"
	arduino    = "node1"
	location   = "BBY"
	lat        = "49.284"
	long       = "-123.021"
)

var (
	githash    = "deadbeef"
	buildstamp = "June 6, 2017"
)

// Measurement holds information about a single weather reading
type Measurement struct {
	Name  string
	Value float64
	Units string
}

// This lets me compare directly in the test file.  See:
// https://stackoverflow.com/questions/36091610/comparing-errors-in-go
var (
	colonErr          = fmt.Errorf("Can't find colon in line, don't know how to split it")
	incompleteReadErr = fmt.Errorf("Can't find closing '}' -- incomplete read?")
	measure, value    string
)

// SplitLine spits a string and returns a Measurement struct and err
func SplitLine(s string) (measure Measurement, err error) {
	// FIXME: Account for errors in all this
	m := Measurement{"", 0.0, ""}
	if strings.Index(s, "}") < 0 {
		return m, incompleteReadErr
	}
	if strings.Index(s, ":") < 0 {
		return m, colonErr
	}
	// Don't include left bracket itself
	leftBracketPos := strings.Index(s, "{") + 1
	rightBracketPos := strings.Index(s, "}")
	s = s[leftBracketPos:rightBracketPos]
	// log.Printf("[DEBUG] After Trim: %s", s)
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
		err = errors.New("can't figure out value of that measurement")
	}
	return m, err
}

func startupLog() {
	log.Printf("Githash: %s\n", githash)
	log.Printf("Build date: %s\n", buildstamp)
}

func main() {
	startupLog()
	allLoggers := []wxLogger{}

	var influx influxLogger
	if err := influx.init(); err != nil {
		log.Fatalf("[FATAL]: Can't log to InfluxDB: %s", err.Error())
	} else {
		allLoggers = append(allLoggers, influx)
	}

	var wunder wundergroundLogger
	if err := wunder.init(); err != nil {
		log.Printf("[WARN] Can't log to wunderground: %s", err.Error())
	} else {
		allLoggers = append(allLoggers, wunder)
	}

	usbdev := "/dev/ttyUSB0"
	c := &serial.Config{Name: usbdev, Baud: 9600}
	serialPort, err := serial.OpenPort(c)
	for err != nil {
		log.Printf("[WARN] Can't open serial port, trying to sleep it off: %s\n", err)
		time.Sleep(10 * time.Second)
		serialPort, err = serial.OpenPort(c)
	}
	reader := bufio.NewReader(serialPort)
	log.Println("[INFO] Opened. Next up: looping.")
	for {
		log.Println("[DEBUG] About to read...")
		reply, err := reader.ReadString('}')
		if err != nil {
			log.Println("[WARN] Problem reading: ", err)
			// Sleep for a second
			time.Sleep(1 * time.Second)
			continue
		}
		log.Println(reply)
		measure, err := SplitLine(reply)
		if err != nil {
			log.Printf("[WARN] Could not split line: %s", err)
			// Sleep for a second
			time.Sleep(1 * time.Second)
			continue
		}
		log.Printf("[INFO] Read: %s: %f\n", measure.Name, measure.Value)
		for _, l := range allLoggers {
			if err = l.log(measure); err != nil {
				log.Printf("[WARN] Problem logging to %s: %s", l.name(), err)
			}
		}
	}
}
