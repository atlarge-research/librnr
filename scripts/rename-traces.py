import os
import sys

l = ['m', 'd', 'a', 'l', 'b', 'p', 'n', 'i']


def main():
    traces_dir = sys.argv[1]
    print(traces_dir)
    if not os.path.basename(traces_dir) == "traces":
        print("Given argument should point to 'traces' directory. Exiting...")
    for o in os.listdir(traces_dir):
        d = os.path.join(traces_dir, o)
        if os.path.isdir(d):
            parts = o.split("-")
            try:
                ts = int(parts[0])
            except:
                print(f"skipping directory '{o}'")
                continue
            m = {}
            k = 1
            while k < len(parts):
                m[parts[k]] = parts[k + 1]
                k += 2
            n = [ts]
            for _, v in enumerate(l):
                if v in m:
                    n.append(v)
                    n.append(m[v])
                    del m[v]
            for _, (k, v) in enumerate(m.items()):
                n.append(k)
                n.append(v)
            u = "-".join([str(x) for x in n])
            v = os.path.join(traces_dir, u)
            if len(v) != len(d):
                print("Error:")
                print(d, v)
                break
            os.rename(d, v)


if __name__ == "__main__":
    main()
