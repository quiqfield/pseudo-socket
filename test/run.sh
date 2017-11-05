#!/bin/bash

cd $(dirname $0)

if [[ -f server.out && -f client.out ]] ; then
	./server.out &
	PID=$!
	./client.out
	sleep 1
	kill ${PID}
	exit 0
else
	echo "Binaries not found."
	exit 1
fi
