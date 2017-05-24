package main

import (
	"errors"
	"testing"
)

func TestSplitLine(t *testing.T) {
	var tests = []struct {
		input string
		want  Measurement
		err   error
	}{
		{"{Humd: 33.10 %, }", Measurement{"Humid", 33.10, "%"}, nil},
		{"{Temp: 22.70 C, }", Measurement{"Temp", 22.70, "C,"}, nil},
		{"{Pres: 1007.03 hP, }", Measurement{"Pres", 1007.03, "hP"}, nil},
		{"{Prcp: 0.00 NA, }", Measurement{"Prcp", 0, "NA"}, nil},
		{"Waiting...", Measurement{"", 0, ""}, errors.New("Can't find colon in line, don't know how to split it")},
		{"radio.new", Measurement{"", 0, ""}, errors.New("Can't find colon in line, don't know how to split it")},
	}
	for _, test := range tests {
		got, err := SplitLine(test.input)
		if test.want != got || err != test.err {
			t.Errorf("SplitLine(%v) returned (%v, %v), want (%v, %v)",
				test.input, got, err, test.want, test.err)
		}
	}
}
