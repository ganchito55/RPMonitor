#!/bin/bash

if [ $1 == "run" ]; then
	gcc server.c -o server
	./server
fi

if [ $1 == "kill" ]; then
	kill -9 $(pidof server)
fi

if [ $1 == "show" ]; then
	lsof -i -p 5555 | grep "server"
fi
