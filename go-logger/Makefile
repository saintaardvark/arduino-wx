FLAGS=-ldflags "-X main.buildstamp=`date -u '+%Y-%m-%d_%I:%M:%S%p'` -X main.githash=`git rev-parse HEAD`"
build:
	go build $(FLAGS)

send:
	GOARCH=arm GOOS=linux go build $(FLAGS)
	scp go-logger pi@192.168.23.133:~

test:
	go build -v
	go test -v
	go vet
	golint
	errcheck

rebuild: build
	sudo supervisorctl restart piwxlogger && tail -F /var/log/supervisor/piwxlogger.log

docker: build-docker
	docker run -it arduino-wx-go-logger /bin/sh

build-docker:
	docker build --tag arduino-wx-go-logger:latest .
