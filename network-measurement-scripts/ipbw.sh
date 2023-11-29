#!/usr/bin/env bash

# Download
iperf3 -c al01.anac.cs.vu.nl -p 7979 -R --json >>./iperf_download_ams_connectivity.json
# Upload
iperf3 -c al01.anac.cs.vu.nl -p 7979 --json >>./iperf_upload_ams_connectivity.json
# To test for yourself, start server using
iperf3 -s -p 7979
