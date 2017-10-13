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
		Measurement{"Humd", 33.1, "%"},
		"https://weatherstation.wunderground.com/weatherstation/updateweatherstation.php?ID=saintaardvark&PASSWORD=s3cr3t&dateutc=2000-01-01+10%3A32%3A35&humidity=90&softwaretype=vws%20versionxx&action=updateraw",
		nil,
	},
}

// Which brings up the point: I really need to start batching up data points
//

func TestBuildURL(t *testing.T) {
	for _, test := range wunderURLTestTable {
		got, err := wl.buildURL(test.input)
		if test.want != got {
			t.Errorf("buildURL(%v) returned (%v, %v), want (%v, %v)",
				test.input, got, err, test.want, test.err)
		}
	}
}
