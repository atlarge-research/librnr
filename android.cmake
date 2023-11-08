# Line below should not be needed, using CMake's built-in support
# set(CMAKE_TOOLCHAIN_FILE /usr/lib/android-ndk/build/cmake/android.toolchain.cmake)
# We want to compile for android, the OS of our VR device
set(CMAKE_SYSTEM_NAME Android)
# set(ANDROID 1)
# The Meta Quest Pro runs on Android 10, afaik, which corresponds to API version 29
set(CMAKE_SYSTEM_VERSION 29)
set(CMAKE_ANDROID_ARCH_ABI arm64-v8a)
# To make this work, CMake needs to know where to find the android NDK
set(CMAKE_ANDROID_NDK /usr/lib/android-ndk)
set(CMAKE_ANDROID_STL_TYPE c++_static)