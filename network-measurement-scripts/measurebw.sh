#!/usr/bin/env bash

chmod +x ./ipbw.sh
chmod +x ./mbw.sh

while true; do
    date +%s >>./time.log
    ./ipbw.sh
    ./mbw.sh
    sleep 60
done
