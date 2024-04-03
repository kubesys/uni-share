FROM ubuntu:20.04 AS build

ENV GOLANG_VERSION 1.19.1
ENV GOPATH /go
ENV PATH $GOPATH/bin:/usr/local/go/bin:$PATH

WORKDIR /go/src/uni-share

COPY . .

RUN apt update && \
    apt install -y g++ wget make && \
    wget -nv -O - https://storage.googleapis.com/golang/go${GOLANG_VERSION}.linux-amd64.tar.gz | tar -C /usr/local -xz && \
    go env -w GOPROXY=https://goproxy.cn,direct && \
    make

FROM debian:stretch-slim

COPY --from=build /go/src/uni-share/bin/uni-share /usr/bin/uni-share