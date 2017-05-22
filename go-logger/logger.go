package main

import (
	"bufio"
	"fmt"
	"log"

	"github.com/tarm/serial"
	// "github.com/influxdata/influxdb/client/v2"
)

// const (
// 	MyDB       = "weather"
// 	username   = "weather"
// 	influxAddr = "https://home.saintaardvarkthecarpeted.com:26472"
// )

// type Measurement struct {
// 	Name  string
// 	Value float32
// 	Units string
// }

// type Message struct {
// 	Name         string
// 	Measurements []*Measurement
// }

// var readout Message


func main() {
	
	usbdev := "/dev/ttyUSB0"
	c := &serial.Config{Name: usbdev, Baud: 9600}
	s, err := serial.OpenPort(c)
	if err != nil {
		log.Fatal(err)
	}
	fmt.Println("Opened.  Next up: reading.")
	reader := bufio.NewReader(s)
	reply, err := reader.ReadString('}')
	if err != nil {
		panic(err)
	}
	fmt.Println(reply)
}
		
	// Create a new HTTPClient
	// c, err := client.NewHTTPClient(client.HTTPConfig{
	// 	Addr:     influxAddr,
	// 	Username: username,
	// 	Password: os.Getenv("INFLUXDB_PASS"),
	// })
	// if err != nil {
	// 	log.Fatal(err)
	// }
	// if err := json.NewDecoder(os.Stdin).Decode(&readout); err != nil {
	// 	log.Println(err)
	// }
	// fmt.Println("readout:")
	// fmt.Printf("%+v\n", readout)
	// fmt.Println("Measurements:")
	// for i := range readout.Measurements {
	// 	fmt.Printf("%+v\n", readout.Measurements[i])
	// }
	// fmt.Println(readout.Name)
	// // Create a new point batch
	// bp, err := client.NewBatchPoints(client.BatchPointsConfig{
	// 	Database:  MyDB,
	// 	Precision: "s",
	// })
	// if err != nil {
	// 	log.Fatal(err)
	// }

	// // Create a point and add to batch
	// tags := map[string]string{"cpu": "cpu-total"}
	// fields := map[string]interface{}{
	// 	"idle":   10.1,
	// 	"system": 53.3,
	// 	"user":   46.6,
	// }

	// pt, err := client.NewPoint("cpu_usage", tags, fields, time.Now())
	// if err != nil {
	// 	log.Fatal(err)
	// }
	// bp.AddPoint(pt)

	// // Write the batch
	// if err := c.Write(bp); err != nil {
	// 	log.Fatal(err)
	// }

