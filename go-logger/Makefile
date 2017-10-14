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
