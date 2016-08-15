FROM alpine:3.3
RUN apk add --no-cache curl \
                       supervisor \
                       bash
COPY assets/log_to_influxdb.sh /opt/log_to_influxdb.sh
CMD ["/opt/log_to_influxdb.sh"]
