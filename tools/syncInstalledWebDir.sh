#!/bin/bash

echo $1
while true
do
inotifywait -e create,modify $1 --format '%f' | cp $1/* /usr/local/share/slidekit/
done
