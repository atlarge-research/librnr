#!/usr/bin/env bash

# Download
IPERF3_PASSWORD=rockstar iperf3 -c al01.anac.cs.vu.nl -p 7979 -R --json --rsa-public-key-path iperf3_pub.pem --username jesse >>./iperf_download_ams.json

# Upload
IPERF3_PASSWORD=rockstar iperf3 -c al01.anac.cs.vu.nl -p 7979 --json --rsa-public-key-path iperf3_pub.pem --username jesse >>./iperf_upload_ams.json

# Download
IPERF3_PASSWORD=rockstar iperf3 -c 146.190.134.21 -p 7979 -R --json --rsa-public-key-path iperf3_pub.pem --username jesse >>./iperf_download_sfo.json

# Upload
IPERF3_PASSWORD=rockstar iperf3 -c 146.190.134.21 -p 7979 --json --rsa-public-key-path iperf3_pub.pem --username jesse >>./iperf_upload_sfo.json

# # To test for yourself, start server using
# iperf3 -s -p 7979
