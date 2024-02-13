commands = {
    # 'cpu' : "grep -Po "(?<=CPU4/GPU=)[0-9]/[0-9],[0-9]+/[0-9]+(?=MHz,OC)" logcat_VrApi.log | cut -d, -f 2 | cut -d/ -f 1"
}
import subprocess

p = subprocess.Popen("""grep -Po "(?<=CPU4/GPU=)[0-9]/[0-9],[0-9]+/[0-9]+(?=MHz,OC)" logcat_VrApi.log | cut -d, -f 2 | cut -d/ -f 1""", shell=True, stdout=subprocess.PIPE).stdout.read()
print(subprocess.Popen("""grep -Po "(?<=CPU4/GPU=)[0-9]/[0-9],[0-9]+/[0-9]+(?=MHz,OC)" logcat_VrApi.log | cut -d, -f 2 | cut -d/ -f 1""", shell=True, stdout=subprocess.PIPE).stdout.read())
print(p.decode())
# for line in p.decode():
#     print(line)
