package main

import (
	"fmt"
	"log"
	"net/http"
	"os"
	"time"
)

var (
	wunderClient = &http.Client{
		Timeout: time.Second * 10,
	}
	baseURL = "https://weatherstation.wunderground.com/weatherstation/updateweatherstation.php"
)

type wundergroundLogger struct {
	apiKey   string
	endpoint string
	_name    string
	id       string
	pass     string
}

func (w wundergroundLogger) buildURL(m Measurement) (string, error) {
	// case m.Name in "humd" return "humidity"
	creds := fmt.Sprintf("ID=%s&PASSWORD=%s", w.id, w.pass)
	// FIXME
	date := "dateutc=2001-01-01+10%3A32%3A35"
	// mstring := "humidity=33"
	mstring, err := w.buildMeasureString(m)
	if err != nil {
		// FIXME: What do we want to return in case we don't log?
		return "", nil
	}
	suffix := "softwaretype=vws%20versionxx&action=updateraw"
	wunderURL := fmt.Sprintf("%s?%s&%s&%s&%s", baseURL, creds, date, mstring, suffix)
	return wunderURL, nil
}

func (w wundergroundLogger) buildMeasureString(m Measurement) (string, error) {
	switch m.Name {
	case "Temp":
		return wunderConvertTemp(m.Value), nil
	case "Humd":
		return wunderConvertHumidity(m.Value), nil
	default:
		return "", fmt.Errorf("Cannot send %s to Wunderground", m.Name)
	}
}

func wunderConvertTemp(celsius float64) string {
	fahr := (celsius * 9 / 5) - 32
	return fmt.Sprintf("tempf=%3.1f", fahr)
}

func wunderConvertHumidity(humd float64) string {
	return fmt.Sprintf("humidity=%3.1f", humd)
}

func wunderConvertPressure(pres float64) string {
	// Convert to inches of mercury!?
	return fmt.Sprintf("FIXME")
}

func (w wundergroundLogger) name() string {
	return w._name
}

// logToWunderground logs mesurement to Wunderground API
func (w wundergroundLogger) log(m Measurement) error {
	if w.apiKey == "" {
		return fmt.Errorf("no API key set")
	}
	if w.endpoint == "" {
		return fmt.Errorf("no API endpoint set")
	}
	now := time.Now().Minute()
	if now == 0 {
		log.Printf("[INFO] Logging to Wunderground")
	}
	url, err := w.buildURL(m)
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
		return fmt.Errorf("Can't log to wunderground without WUNDER_APIKEY environment variable")
	}

	w.endpoint, exists = os.LookupEnv("WUNDER_ENDOINT")
	if exists == false {
		return fmt.Errorf("Can't log to wunderground without WUNDER_APIKEY environment variable")
	}
	w._name = "wunderground"
	return nil
}
