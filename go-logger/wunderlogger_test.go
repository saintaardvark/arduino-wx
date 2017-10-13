package main

import (
	"testing"
)

var wl = wundergroundLogger{
	apiKey:   "key",
	endpoint: "https://wunder.example.com",
	_name:    "test",
}

var wunderURLTestTable = []struct {
	input Measurement
	want  string
	err   error
}{
	{Measurement{"Humd", 33.1, "%"}, "http://slashdot.org", nil},
}

func testBuildURL(t *testing.T) {
func TestBuildURL(t *testing.T) {
	for _, test := range wunderURLTestTable {
		got, err := wl.buildURL(test.input)
		if test.want != got {
			t.Errorf("buildURL(%v) returned (%v, %v), want (%v, %v)",
				test.input, got, err, test.want, test.err)
		}
	}
}
