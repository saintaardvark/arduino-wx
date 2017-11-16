package main

import (
	"testing"
)

var wl = wundergroundLogger{
	apiKey:   "key",
	endpoint: "https://wunder.example.com",
	_name:    "test",
	id:       "saintaardvark",
	pass:     "s3cr3t",
}

var wunderURLTestTable = []struct {
	input Measurement
	want  string
	err   error
}{
	{
		// FIXME: Add test for 33.1 to make sure I get decimal point
		Measurement{"Humd", 33.1, "%"},
		"https://weatherstation.wunderground.com/weatherstation/updateweatherstation.php?ID=saintaardvark&PASSWORD=s3cr3t&dateutc=2001-01-01+10%3A32%3A35&humidity=33.1&softwaretype=vws%20versionxx&action=updateraw",
		nil,
	},
}

// Which brings up the point: I really need to start batching up data points
//

func TestBuildURL(t *testing.T) {
	for _, test := range wunderURLTestTable {
		got, _ := wl.buildURL(test.input)
		if test.want != got {
			t.Errorf("buildURL(%v) returned (%v), want (%v)",
				test.input, got, test.want)
		}
	}
}

var wunderMeasureTestTable = []struct {
	input Measurement
	want  string
}{
	{
		Measurement{"Humd", 33.1, "%"},
		"humidity=33.1",
	},
	{
		Measurement{"Temp", 33.1, "%"},
		"tempf=91.58",
	},
	{
		Measurement{"BadMeasurement", 42.0, "X"},
		"",
	},
}

func TestBuildMeasureString(t *testing.T) {
	for _, test := range wunderMeasureTestTable {
		got, _ := wl.buildMeasureString(test.input)
		if test.want != got {
			t.Errorf("buildURL(%v) returned (%v), want (%v)",
				test.input, got, test.want)
		}
	}
}
