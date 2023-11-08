# Record and Replay for VR

librnr is an [OpenXR API layer](https://www.khronos.org/registry/OpenXR/specs/1.0/html/xrspec.html#api-layers) that allows recording and replaying user inputs in arbitrary applications built on OpenXR. Its main purpose is to enable the end-to-end detailed study of non-functional properties of virtual reality devices and metaverse applications.
The implementation is built on [this template for OpenXR API layers](https://github.com/Ybalrid/OpenXR-API-Layer-Template).

## Setup

This project is CMake-based.
You can build the project using your IDE or the command line.
CLion, Visual Studio, or Visual Studio Code are all good options.
These IDEs have support CMake, either directly or through plugins.

To build the project using the command line, run the following commands in the project's root folder:

```bash
mkdir build
cd build
cmake ..
```

Depending on your platform, this should create an `rnr.dll` or `librnr.so` file, and a `manifest.json` file.

librnr is an implicit API layer which, once installed, gets loaded automatically by the OpenXR runtime whenever an OpenXR application is started.
To install the library, follow the [API layer discovery steps](https://registry.khronos.org/OpenXR/specs/1.0/loader.html#desktop-api-layer-discovery) for your platform. 

## Record

To record a trace, set the library mode to RECORD in the `ListShims` function in `layer_shims.cpp` and recompile.
A trace will now automatically will created in `trace.txt` when you start an OpenXR application.
**Note:** creating additional traces will overwrite the existing one.

## Replay

To replay a trace, set the library mode to REPLAY in the `ListShims` function in `layer_shims.cpp` and recompile.
A trace will now automatically be played back when you start an OpenXR application.
When the trace ends, control is returned to the physical controllers.

## Trace Format

The library creates a plain-text trace where every line corresponds to a record.
Every record consists of a number of fields separated by spaces.
Every record has the same 3-field header:

```
timestamp type path
```

- The timestamp is in nanoseconds since the start of the trace.
- The type of event, also specifies the format of the record body. The available types are: (s)pace, (v)iew, (b)oolean, and (f)loat. The record body for each type is discussed below.
- The path is a human-readable text field that indicates the source (e.g., [the action space](https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#_action_spaces)) of the record/event 

### Space

Records an objects position and rotation in a space, for example of the controllers and the headset of a VR device.

```
ox oy oz ow px py pz basespace
```

- ox, oy, oz, ow: the coordinates of the orientation [quaternion](https://docs.unity3d.com/Manual/QuaternionAndEulerRotationsInUnity.html).
- px, py, pz: the coordinates of the position vector.
- basespace: the space relative to which the position and orientation is being tracked.

See also https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#xrLocateSpace

### View

Records the orientation, position, and view direction of the user's eyes. Should correspond more or less with the `space` records of the headset.

```
ox oy oz ow px py pz u r d l type index
```

- ox, oy, oz, ow: the coordinates of the orientation [quaternion](https://docs.unity3d.com/Manual/QuaternionAndEulerRotationsInUnity.html).
- px, py, pz: the coordinates of the position vector.
- u r d l: the up, right, down, and left *field of view* angles of the eyes, respectively.
- type: the type of view. See [OpenXR view configurations](https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#view_configuration_type).
- index: when using STEREO, 0 is the left eye, 1 is the right eye

See also https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#xrLocateViews

### Boolean

A record for a boolean event.

```
changed isActive lastChanged value (e.g., pressing a button)
```

- changed: true if value has changed since last poll event.
- isActive: true is the action/button is enabled.
- lastChanged: the timestamp when the value last changed.
- value: true if the action is triggered, false otherwise.

See also https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#XrActionStateBoolean

### Float

A record for a float event (e.g., pulling the trigger button)

- changed: true if value has changed since last poll event.
- isActive: true is the action/button is enabled.
- lastChanged: the timestamp when the value last changed.
- value: a floating point value between 0 and 1.

See also https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#XrActionStateFloat
