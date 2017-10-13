package main

type wxLogger interface {
	log(m Measurement) error
	init() error
	name() string
}
