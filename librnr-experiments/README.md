# librnr-experiments

## Figures List
| Figure Number | Description | Origin File Location |
|---------------|-------------|---------------|
| 1             |  Meta Quest Pro power use when using different play area configurations. | [./new_bm.ipynb](./new_bm.ipynb) |
| 4             |   Timing accuracy (i.e., error) of librnr over time (top plot) and as a statistical summary (bottom plot). | TODO:Add file location |
| 5             |  librnr energy overhead on Meta Quest Pro (MQP) and Meta Quest 2 (MQ2). | [./analysis_BM.ipynb](./analysis_BM.ipynb) |
| 6             |  librnr GPU-usage overhead on the two PCs. | [./analysis_BM.ipynb](./analysis_BM.ipynb) |
| 7             |   Effect of play area settings on power consumption
for the Quest Pro (MQP) and Quest 2 (MQ2) VR devices. | [./new_bm.ipynb](./new_bm.ipynb) |
| 8             |   Effect of bandwidth limits on Meta Quest 2 frames per second (FPS). |  [./new_systemmetrics.ipynb](./new_systemmetrics.ipynb) |
| 9             |   Cumulative probability density function for the frames per second (FPS) on Meta Quest. | [./fps_bandwithlimit_analysis.ipynb](./fps_bandwithlimit_analysis.ipynb) |
| 10            |   Comparison in workload offloading between Meta Quest 2 (MQ2) and Meta Quest Pro (MQP). | [./analysis_BM.ipynb](./analysis_BM.ipynb) |

## Trace List
| Trace Folder Name                  | Game / App | Duration of trace |  PC  |     VR    |       Network Conditions      | Additional Info       | Location | Paper Section |
|------------------------------------|:----------:|:-----------------:|:----:|:---------:|:-----------------------------:|-----------------------|----------|---------------|
| clumsy-bandwidth-100Mbps/Quest-2   | Beat Saber | 229.161           | PC-B | Quest 2   | 100 Mbps bandwidth limitation | 1 recording, 3 replays | [clumsy-bandwidth-100Mbps/Quest-2](./clumsy-bandwidth-100Mbps/Quest-2/BeatSaber/PopStars-Medium/) | Not used |
| clumsy-bandwidth-100Mbps/Quest-Pro | Beat Saber | 219.424           | PC-A | Quest Pro | 100 Mbps bandwidth limitation | 1 recording, 3 replays | [clumsy-bandwidth-100Mbps/Quest-Pro](./clumsy-bandwidth-100Mbps/Quest-Pro/BeatSaber/PopStars-Medium/) | Not used |
| clumsy-bandwidth-80Mbps/Quest-2    | Beat Saber | 229.161           | PC-B | Quest 2   | 80 Mbps bandwidth limitation  | 1 recording, 3 replays | [clumsy-bandwidth-80Mbps/Quest-2](./clumsy-bandwidth-80Mbps/Quest-2/BeatSaber/PopStars-Medium/) | Not used |
| clumsy-bandwidth-80Mbps/Quest-Pro  | Beat Saber | 219.424           | PC-A | Quest Pro | 80 Mbps bandwidth limitation  | 1 recording, 3 replays | [clumsy-bandwidth-80Mbps/Quest-Pro](./clumsy-bandwidth-80Mbps/Quest-Pro/BeatSaber/PopStars-Medium/) | Not used |
| clumsy-bandwidth-50Mbps/Quest-2    | Beat Saber | 229.161           | PC-B | Quest 2   | 50 Mbps bandwidth limitation  | 1 recording, 3 replays | [clumsy-bandwidth-50Mbps/Quest-2](./clumsy-bandwidth-50Mbps/Quest-2/BeatSaber/PopStars-Medium/) | Not used |
| clumsy-bandwidth-50Mbps/Quest-Pro  | Beat Saber | 219.424           | PC-A | Quest Pro | 50 Mbps bandwith limitation   | 1 recording, 3 replays | [clumsy-bandwidth-50Mbps/Quest-Pro](./clumsy-bandwidth-50Mbps/Quest-Pro/BeatSaber/PopStars-Medium/) | Not used |
| clumsy-bandwidth-30Mbps/Quest-2    | Beat Saber | 229.161           | PC-B | Quest 2   | 30 Mbps bandwith limitation   | 1 recording, 3 replays | [clumsy-bandwidth-30Mbps/Quest-2](./clumsy-bandwidth-30Mbps/Quest-2/BeatSaber/PopStars-Medium/) | Not used |
| clumsy-bandwidth-30Mbps/Quest-Pro  | Beat Saber | 219.424           | PC-A | Quest Pro | 30 Mbps bandwith limitation   | 1 recording, 3 replays | [clumsy-bandwidth-30Mbps/Quest-Pro](./clumsy-bandwidth-30Mbps/Quest-Pro/BeatSaber/PopStars-Medium/) | Not used |
| clumsy-dropchance-0.05/Quest-2     | Beat Saber | 229.161           | PC-B | Quest 2   | 5% drop chance                | 1 recording, 3 replays | [clumsy-dropchance-0.05/Quest-2](./clumsy-dropchance-0.05/Quest-2/BeatSaber/PopStars-Medium/) | Not used |
| clumsy-dropchance-0.05/Quest-Pro   | Beat Saber | 219.424           | PC-A | Quest Pro | 5% drop chance                | 1 recording, 3 replays | [clumsy-dropchance-0.05/Quest-Pro](./clumsy-dropchance-0.05/Quest-Pro/BeatSaber/PopStars-Medium/) |Not used |
| clumsy-dropchance-0.025/Quest-2    | Beat Saber | 229.161           | PC-B | Quest 2   | 2.5% drop chance              | 1 recording, 3 replays | [clumsy-dropchance-0.025/Quest-2](./clumsy-dropchance-0.025/Quest-2/BeatSaber/PopStars-Medium/) | Not used |
| clumsy-dropchance-0.025/Quest-Pro  | Beat Saber | 219.424           | PC-A | Quest Pro | 2.5% drop chance              | 1 recording, 3 replays | [clumsy-dropchance-0.025/Quest-Pro](./clumsy-dropchance-0.025/Quest-Pro/BeatSaber/PopStars-Medium/) | Not used |
| clumsy-dropchance-0.01/Quest-2    | Beat Saber | 229.161                   | PC-B | Quest 2   | 1% drop chance | 1 recording, 3 replays                             | [clumsy-dropchance-0.01/Quest-2](./clumsy-dropchance-0.01/Quest-2/BeatSaber/PopStars-Medium/) | Not used |
| clumsy-dropchance-0.01/Quest-Pro  | Beat Saber | 219.424                   | PC-A | Quest Pro | 1% drop chance | 1 recording, 3 replays                             | [clumsy-dropchance-0.01/Quest-Pro](./clumsy-dropchance-0.01/Quest-Pro/BeatSaber/PopStars-Medium/) | Not used |
| clumsy-dropchance-0.001/Quest-2   | Beat Saber | 229.161                   | PC-B | Quest 2   | 0.1% drop chance | 1 recording, 3 replays                             | [clumsy-dropchance-0.001/Quest-2](./clumsy-dropchance-0.001/Quest-2/BeatSaber/PopStars-Medium/) | Not used |
| clumsy-dropchance-0.001/Quest-Pro | Beat Saber | 219.424                   | PC-A | Quest Pro | 0.1% drop chance | 1 recording, 3 replays                             | [clumsy-dropchance-0.001/Quest-Pro](./clumsy-dropchance-0.001/Quest-Pro/BeatSaber/PopStars-Medium/) | Not used |
| overhead-validation/Quest-2       | Beat Saber | 224.746, 229.161, 228.258 | PC-B | Quest 2   | -                 | 3 baseline, 3 recordings, 10 replays per recording | [overhead-validation/Quest-2](./overhead-validation/Quest-2/BeatSaber/PopStars-Medium/) | Sections 4.2, 5 |
| overhead-validation/Quest-Pro     | Beat Saber | 222.230, 219.424, 217.662 | PC-A | Quest Pro | -                 | 3 baseline, 3 recordings, 10 replays per recording | [overhead-validation/Quest-Pro](./overhead-validation/Quest-Pro/BeatSaber/PopStars-Medium/) | Sections 4.2, 5 |
| swap-vr/Quest-2                   | Beat Saber | 222.146                   | PC-A | Quest 2   | -                 | 1 recording, 10 replays                            | [swap-vr/Quest-2](./swap-vr/Quest-2/BeatSaber/PopStars-Medium/) | Section 5.4 |
| swap-vr/Quest-Pro                 | Beat Saber | 233.927                   | PC-B | Quest Pro | -                 | 1 recording, 10 replays                            | [swap-vr/Quest-Pro](./swap-vr/Quest-Pro/BeatSaber/PopStars-Medium/) | Section 5.4 |
| haptic-traces/Quest-2             | Beat Saber | 188.766, 163.524, 240.221 | PC-B | Quest 2   | -                 | 3 recordings, 1 replay per recording               | [haptic-traces/Quest-2](./haptic-traces/Quest-2/BeatSaber/) | Section 4.1 |
| investigation/Quest-Pro                 | Beat Saber | 233.927                   | PC-B | Quest Pro | -                 | 1 recording, 55 replays                            | [investigation/Quest-Pro](./investigation/Quest-Pro/BeatSaber/PopStars-Medium/) | Section 5.2 |
| investigation/Quest-2             | Beat Saber | 188.766, 163.524, 240.221 | PC-B | Quest 2   | -                 | 1 recording, 25 replays               | [investigation/Quest-2](./investigation/Quest-2/BeatSaber/) | Section 5.2 |

Where:
* `PC-A`: Windows 11 system with a liquid-cooled AMD Ryzen 5 7600X CPU, a GeForce RTX 3080 GPU, and a motherboard that supports Wi-Fi 6E (802.11ax).
* `PC-B`: Windows 10 system with an air-cooled AMD Ryzen 5 7600X CPU, a GeForce RTX 4070 GPU, and a motherboard that supports Wi-Fi 6E (802.11ax).


## Experiment folder structure
Old experiments are stored in the `old-experiments` folder. 


## Before opening Quest Link
1. Connect Quest to PC via cable.
2. In a terminal, run `adb devices` to verify that Quest is connected.
3. Install the BatteryManager app on Quest by running `adb install -g <path-to-apk>`.
4. Start the BatteryManager app on Quest via
```bash
adb shell am start -n "com.example.batterymanager_utility/com.example.batterymanager_utility.MainActivity" -a android.intent.action.MAIN -c android.intent.category.LAUNCHER
```
5. After the app starts, you can now connect the VR headset via adb wifi.
6. Run `adb tcpip 5555` to enable adb wifi.
7. Run `adb connect <ip-address-of-quest>:5555` to connect to Quest via adb wifi. The IP address can be found in the Quest settings > Wifi > `<Your Network>` > Scroll down until you see the IP address.
8. Run `adb devices` to verify that Quest is connected via adb wifi.
9. Disconnect the cable from the Quest.
10. Launch Quest Link.

## Using vr_runner.py
### Before running the script
* Add the steam folder to the environment variable `Path`. (`C:\Program Files (x86)\Steam`)
* Clone Vlad's [traffic shaper repo](https://github.com/Vlad2000Andrei/DistributedSystems-Traffic-Shaper)
* Install clumsy as per [his instructions](https://github.com/Vlad2000Andrei/DistributedSystems-Traffic-Shaper/blob/main/README.md).
* Configure the `config.json` file.

### Configuring the `config.json` file
* `experiment_name`: The name of the experiment. This will be used to create a directory with the same name as the experiment and all results will be stored there under the same file conventions as before.
* `clumsy`
  * `clumsy_scripts_path`: The path to the folder containing the clumsy scripts in Vlad's traffic shaper.
  * `clumsy_path`: Path to the clumsy executable.
  * `delay`, `delay_chance`, `bandwidth_KBps`, `drop_chance`: The values for the clumsy script. `-1` means disabled.
* `apps`
  * `name`: The name of the app. This will be used to create a directory with the same name as the app and all results will be stored there under the same file conventions as before.
  * `exe_name`: The name of the executable for the app. This is used by the `runbench.ps1` script to stop the app.
  * `steam_app_id`: ID of the app on Steam. This is used by the `runbench.ps1` script to start the app. The ID can be found on the app's Steam page >> Settings (Cog wheel) >> Properties >> Updates >> AppID (Below Background downloads).
  * `startup_time`: The time in seconds to wait for the app to start.


### Running the script
1. Open PowerShell with admin privileges.
2. Find the `python.exe` path for your python version Run `<your-python.exe-path> vr_runner.py` to start the script.
3. The script will use the `config.json` file to determine which experiments to run.
4. First the script will launch `clumsy` with the specified parameters, then `runbench.ps1`. The application should start shortly after the `Sleep 5 seconds...BatteryManager`. When the trace is done replaying, the app will automatically close (be patient). After all the iterations are done, the script will close `clumsy` and copy the config file to the experiment directory.