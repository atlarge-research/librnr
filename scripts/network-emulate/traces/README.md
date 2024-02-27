# Network trace processing
MahiMahi-style traces have on each line a timstamp in milliseconds, noting the time (from t=0 on) that one MTU-sized packet (1500 bytes) was sent. We transform this into a MB/s trace, with each line being the bandwidth for one second.

This directory already contains the results for the MahiMahi repository.

## Setup
```
# For MahiMahi traces
git clone https://github.com/ravinet/mahimahi.git
python3 mahimahi.py

# For more traces
git clone https://github.com/Soheil-ab/Cellular-Traces-2018.git
# Update mahimahi.py -> change paths from /mahimahi/traces to /Cellular-Traces-2018
python3 mahimahi.py
```

## Requirements
1. Python3
2. Git
3. Powershell