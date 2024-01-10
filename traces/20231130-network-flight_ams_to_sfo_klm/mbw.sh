#!/usr/bin/env bash

# To measure connectivity to a server in amsterdam
speedtest --server-id=30847 --format=csv >>./ams_connectivity.csv
# The server is run by claranet
# Connectivity to San Francisco
speedtest --server-id=5754 --format=csv >>./sf_connectivity.csv
# Server run by fastmetrics

# # Run once to get headers
# speedtest --output-header --format=csv
