#!/usr/bin/env bash

chmod +x ./ipbw.sh
chmod +x ./mbw.sh

ping -i 1 1.1.1.1 >>./ping_1_1_1_1.log &
PID_P1=$!

ping -i 1 8.8.8.8 >>./ping_8_8_8_8.log &
PID_P2=$!

ping -i 1 al01.anac.cs.vu.nl >>./ping_ams.log &
PID_P3=$!

ping -i 1 146.190.134.21 >>./ping_sfo.log &
PID_P4=$!

trap 'kill $PID_P1 $PID_P2 $PID_P3 $PID_P4' EXIT

while true; do
    date +%s >>./time.log
    ./ipbw.sh
    # ./mbw.sh
    sleep 60
done
