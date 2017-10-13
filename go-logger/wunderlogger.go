package main

import (
	"fmt"
	"log"
	"net/http"
	"os"
	"time"
)

var wunderClient = &http.Client{
	Timeout: time.Second * 10,
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
	url, err := buildWunderURL(m)
	if err != nil {
		return err
	}
	_, err = wunderClient.Get(url)
	return err
}

func (w wundergroundLogger) init() error {
	var exists bool
	w.apiKey, exists = os.LookupEnv("WUNDER_APIKEY")
	if exists == false {
		return fmt.Errorf("Can't log to wunderground without WUNDER_APIKEY environment variable!")
	}

	w.endpoint, exists = os.LookupEnv("WUNDER_ENDOINT")
	if exists == false {
		return fmt.Errorf("Can't log to wunderground without WUNDER_APIKEY environment variable!")
	}
	w._name = "wunderground"
	return nil
}

func buildWunderURL(m Measurement) (string, error) {
	return "http://slashdot.org", nil
}