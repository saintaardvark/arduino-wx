package main

import (
	"fmt"
	"log"
	"os"
	"time"

	"github.com/influxdata/influxdb/client/v2"
)

type influxLogger struct {
	ic    client.Client
	pass  string
	_name string
}

func (i *influxLogger) init() error {
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
		Timeout:  (time.Duration(10) * time.Second),
	})
	if err != nil {
		log.Println("[WARN] Could not set up new InfluxDB client")
		return err
	}
	// log.Printf("[DEBUG] Right after setting, i.ic == |%v|\n", i.ic)
	i._name = "InfluxDB"
	return nil
}

// logToInfluxdb sends a measurement to an InfluxDB server
func (i *influxLogger) log(m Measurement) error {
	// fmt.Println("[DEBUG] FIXME: Made it here")
	// fmt.Printf("[DEBUG] FIXME: i.pass == %v\n", i.pass)
	// fmt.Printf("[DEBUG] FIXME: i._name == %v\n", i._name)
	// fmt.Printf("[DEBUG] FIXME: i.ic == %v\n", i.ic)

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
		return fmt.Errorf("error in client.NewPoint: %s", err)
	}
	bp.AddPoint(pt)
	// Write the batch
	if err := i.ic.Write(bp); err != nil {
		return fmt.Errorf("error writing to Influxdb: %s", err)
	}
	return nil
}

func (i *influxLogger) name() string {
	return i._name
}
