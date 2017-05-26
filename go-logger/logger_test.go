package main

import (
	"testing"
)

var testTable = []struct {
	input string
	want  Measurement
	err   error
}{
	{"{Humd: 33.10 %}", Measurement{"Humd", 33.1, "%"}, nil},
	{"{Temp: 22.70 C}", Measurement{"Temp", 22.7, "C"}, nil},
	{"{Pres: 1007.03 hP}", Measurement{"Pres", 1007.03, "hP"}, nil},
	{"{Prcp: 0.00 NA}", Measurement{"Prcp", 0, "NA"}, nil},
	{"Waiting...", Measurement{"", 0, ""}, colonErr},
	{"radio.new", Measurement{"", 0, ""}, colonErr},
}

func TestSplitLineName(t *testing.T) {
	for _, test := range testTable {
		got, err := SplitLine(test.input)
		if test.want.Name != got.Name {
			t.Errorf("SplitLine(%v) returned (%v, %v), want (%v, %v)",
				test.input, got.Name, err, test.want.Name, test.err)
		}
	}
}

func TestSplitLineValue(t *testing.T) {
	for _, test := range testTable {
		got, err := SplitLine(test.input)
		if test.want.Value != got.Value {
			t.Errorf("SplitLine(%v) returned (%v, %v), want (%v, %v)",
				test.input, got.Value, err, test.want.Value, test.err)
		}
	}
}

func TestSplitLineUnits(t *testing.T) {
	for _, test := range testTable {
		got, err := SplitLine(test.input)
		if test.want.Units != got.Units {
			t.Errorf("SplitLine(%v) returned (%v, %v), want (%v, %v)",
				test.input, got.Units, err, test.want.Units, test.err)
		}
	}
}
func TestSplitLineError(t *testing.T) {
	for _, test := range testTable {
		got, err := SplitLine(test.input)
		if test.err != err {
			t.Errorf("SplitLine(%v) returned (%v, %v), want (%v, %v)",
				test.input, got, err, test.want, test.err)
		}
	}
}
