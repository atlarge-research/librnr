"""
1. git clone https://github.com/ravinet/mahimahi.git
2. python3 mahimahi.py
"""

import sys


def aggregate(file):
    """Take a MahiMahi trace, which is a text file with on each line a timestamp in ms,
    noting a MTU-sized packet of 1500 bytes being sent.
    We aggregate this data to create a throughput per second number. If a MTU-sized packet
    has been sent at time 1400, 1513, and 1789, we have 3 x 1500 = 4500 bytes per second as
    throuput for this particular second

    Args:
        file (str): MahiMahi trace file to process

    Returns:
        list(int): Throughput for a particular granularity in bytes
    """
    GRANULARITY = 1000  # Aggregate bytes sent per 1000 ms = 1 sec
    MTU = 1500  # MTU size of 1500 bytes
    index = 0
    bytes = 0

    tp = []

    with open(f"mahimahi/traces/{file}", "r") as f:
        for line in f.readlines():
            try:
                timestamp = int(line.rstrip())
                if timestamp < index * GRANULARITY:
                    sys.exit("ERROR: Timestamp goes back in time")
                elif timestamp < (index + 1) * GRANULARITY:
                    bytes += MTU
                elif timestamp >= (index + 1) * GRANULARITY:
                    if bytes == 0:
                        # 0 = unlimited. We don't want any network, so let's do 1 byte/s
                        bytes = 1

                    tp.append(bytes)
                    bytes = 0
                    index += 1
            except:
                print(f"Could not convert timestamp line to int: {line}")

    return tp


def equalize(tp1, tp2):
    """Make 2 arrays the same length by repeating the last value

    Args:
        tp1 (list(int)): Array of timestamps
        tp2 (list(int)): Another array of timestamps

    Returns:
        list(int), list(int): Two arrays of timestamps of the same length
    """
    diff = abs(len(tp1) - len(tp2))
    if len(tp1) < len(tp2):
        add = [tp1[-1]] * diff
        tp1 += add
    elif len(tp2) < len(tp1):
        add = [tp2[-1]] * diff
        tp2 += add

    return tp1, tp2


def write(file, tp):
    """Write an array of timestamps to a file

    Args:
        file (str): MahiMahi trace file to process
        tp (list(int)): Array of timestamps
    """
    with open(file, "w") as f:
        f.writelines([str(t) + "\n" for t in tp])


def main():
    """Main function of script"""
    up = "Verizon-LTE-driving.up"
    down = "Verizon-LTE-driving.down"

    tp_up = aggregate(up)
    tp_down = aggregate(down)

    tp_up, tp_down = equalize(tp_up, tp_down)

    write(up, tp_up)
    write(down, tp_down)


if __name__ == "__main__":
    main()
