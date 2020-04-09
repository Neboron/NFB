#!/bin/bash

echo "NGUI 1.0"

tput civis

sudo ./bin/ngui &

hideinput()
{
  if [ -t 0 ]; then
     stty -echo -icanon time 0 min 0
  fi
}

cleanup()
{
  if [ -t 0 ]; then
    stty sane
    tput clear
    tput cnorm
	kill 0
  fi
}

trap cleanup EXIT
trap hideinput CONT
hideinput

n=0
while test $n -lt 1
do
  read line
  sleep 1
done

echo





