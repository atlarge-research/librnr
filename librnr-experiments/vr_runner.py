import os
import sys
import json
import subprocess
from time import sleep


def start_clumsy(clumsy):
    command = clumsy["clumsy_scripts_path"] + "start-clumsy.ps1 " + \
              "-ClumsyPath \"" + clumsy["clumsy_path"] + "\" " + \
              "-AffectUpload $true -AffectDownload $true "
    if clumsy["delay"] != -1:
        command += "-Delay " + str(clumsy["delay"]) + " "
    if clumsy["delay_chance"] != -1:
        command += "-DelayChance " + str(clumsy["delay_chance"]) + " "
    if clumsy["bandwidth_KBps"] != -1:
        command += "-BandwidthKBps " + str(clumsy["bandwidth_KBps"]) + " "
    if clumsy["drop_chance"] != -1:
        command += "-DropChance " + str(clumsy["drop_chance"]) + " "
              
    p = subprocess.Popen(['powershell.exe', command], stdout=sys.stdout)
    p.communicate()


def stop_clumsy(clumsy):
    command = clumsy["clumsy_scripts_path"] + "stop-clumsy.ps1"
    p = subprocess.Popen(['powershell.exe', command], stdout=sys.stdout)
    p.communicate()


def main():
    # get full path of current folder
    current_dir = os.path.dirname(os.path.realpath(__file__))

    config_file = "./config.json"
    with open(config_file) as f:
        config = json.load(f)

    if "clumsy" in config:
        start_clumsy(config["clumsy"])

    OUTPUT_DIR = os.path.join(current_dir, config["experiment_name"], config["device"])
    RUN_BENCH = os.path.join(current_dir, "runbench.ps1")

    for app in config["apps"]:
        TRACE_PATH = os.path.join(current_dir, config['experiment_name'], config["device"], app["trace_path"])
        out = os.path.join(OUTPUT_DIR, app["name"], app["variation"])
        out = os.path.join(out, "replay" +  TRACE_PATH.split("record")[1][0] + ".")

        for i in range(config["repetitions"]):
            command = RUN_BENCH + ' -Mode "replay" ' + \
                      '-TraceFile "' + TRACE_PATH + '" ' + \
                      '-OutDir "' + out + str(i) + '" ' + \
                      '-SteamAppID "' + app['steam_app_id'] + '" ' + \
                      '-SteamAppExe "' + app['exe_name'] + '" ' + \
                      '-AppStartupTime ' + str(app['startup_time'])
            
            print(command)
            p = subprocess.Popen(['powershell.exe', command], stdout=sys.stdout)
            p.communicate()

            print("Stop app")

            sleep(config["time_between_repetitions"])
        sleep(config["time_between_apps"])
    
    # copy config.json to experiment directory
    with open(os.path.join(current_dir, config['experiment_name'], "config.json"), 'w') as outfile:
        json.dump(config, outfile, indent=4)

    if "clumsy" in config:
        stop_clumsy(config["clumsy"])


if __name__ == "__main__":
    main()


