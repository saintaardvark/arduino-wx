package main

import (
	"fmt"
	"log"
	"os"
	"time"

	"github.com/influxdata/influxdb/client/v2"
)

type wxLogger interface {
	log(m Measurement) error
	init() error
	name() string
}

type wundergroundLogger struct {
	apiKey   string
	endpoint string
	_name    string
}

func (w wundergroundLogger) name() string {
	return w._name
}

// logToWunderground logs mesurement to Wunderground API
func (w wundergroundLogger) log(m Measurement) error {
	if w.apiKey == "" {
		return fmt.Errorf("Cannot see API key!")
	}
	if w.endpoint == "" {
		return fmt.Errorf("Cannot see API key!")
	}
	now := time.Now().Minute()
	if now == 0 {
		log.Printf("[INFO] Logging to Wunderground")
	}
	return nil
}

func (w wundergroundLogger) init() error {
	var exists bool
	w.apiKey, exists = os.LookupEnv("WUNDER_APIKEY")
	if exists == false {
		return fmt.Errorf("[WARN] Can't log to wunderground without WUNDER_APIKEY environment variable!")
	}

	w.endpoint, exists = os.LookupEnv("WUNDER_ENDOINT")
	if exists == false {
		return fmt.Errorf("[WARN] Can't log to wunderground without WUNDER_APIKEY environment variable!")
	}
	w._name = "wunderground"
	return nil
}

type influxLogger struct {
	ic    client.Client
	pass  string
	_name string
}

func (i influxLogger) init() error {
	var err error
	log.Println("[INFO] Setting up InfluxDB client.")
	pass, exists := os.LookupEnv("INFLUXDB_PASS")
	addr := influxAddr
	user := influxUser
	if exists == false {
		log.Fatal("[FATAL] Can't proceed without environment var INFLUXDB_PASS!")
	}

	i.ic, err = client.NewHTTPClient(client.HTTPConfig{
		Addr:     addr,
		Username: user,
		Password: pass,
	})
	if err != nil {
		log.Println(err)
		return err
	}
	i._name = "InfluxDB"
	return nil
}

// logToInfluxdb sends a measurement to an InfluxDB server
func (i influxLogger) log(m Measurement) error {
	// Create a new point batch
	bp, err := client.NewBatchPoints(client.BatchPointsConfig{
		Database:  influxDB,
		Precision: "s",
	})
	if err != nil {
		return fmt.Errorf("Error creating InfluxDB batch: %s", err)
	}

	// Create a point and add to batch
	tags := map[string]string{
		"location": location,
		"arduino":  arduino,
		"lat":      lat,
		"long":     long,
	}
	fields := map[string]interface{}{
		m.Name: m.Value,
	}

	measureAbbrevs := map[string]string{
		"Humd": "humidity",
		"Prcp": "precipitation",
		"Pres": "pressure",
		"Temp": "temperature",
	}
	pt, err := client.NewPoint(measureAbbrevs[m.Name], tags, fields, time.Now())
	log.Printf("[DEBUG] m.Name is %s\n", m.Name)
	log.Printf("[DEBUG] Trying to log that under %s\n", measureAbbrevs[m.Name])
	if err != nil {
		return fmt.Errorf("Error in client.NewPoint: %s\n", err)
	}
	bp.AddPoint(pt)

	// Write the batch
	if err := i.ic.Write(bp); err != nil {
		return fmt.Errorf("Error writing to Influxdb: %s\n", err)
	}
	return nil
}

func (i influxLogger) name() string {
	return i._name
}
