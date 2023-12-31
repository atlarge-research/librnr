# SPDX-FileCopyrightText: 2021-2023 Arthur Brainville (Ybalrid) <ybalrid@ybalrid.info>
#
# SPDX-License-Identifier: MIT
#
# Initial Author: Arthur Brainville <ybalrid@ybalrid.info>

cmake_minimum_required(VERSION 3.20)

#To configure the layer name, description and output name, change these variables
project(OpenXR-librnr)
set(library_name      "rnr")
set(layer_name        "XR_APILAYER_ATLARGE_RNR") # This name *must* be of the form XR_APILAYER_<VENDOR_TAG>_the_name_of_the_layer to be valid according to the OpenXR 1.0 spec.
set(layer_version     "0.2")
set(layer_description "An OpenXR layer to record and replay user inputs.")

#To add extension declaration to the layer manifest file, fill these two string lists : "extensions" and "extension_versions"
#These two needs to match in lenght
# list(APPEND extensions "XR_TEST_test_me")
# list(APPEND extension_versions "42")

#this need to be done in the exact same pattern for every extensions this OpenXR layer implements.
#the size of the two list *have* to match
#list(APPEND extensions "XR_EXT_b")
#list(APPEND extension_versions "2")
#list(APPEND extensions "XR_EXT_c")
#list(APPEND extension_versions "3")

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/)
include(extension_list_helper)
build_instance_extension_list("${extensions}" "${extension_versions}" instance_extension_list)
build_instance_extension_header("${extensions}" "${extension_versions}" instance_extension_header)

set(layer_directory "${CMAKE_BINARY_DIR}/${layer_name}/")
file(MAKE_DIRECTORY ${layer_directory})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${layer_directory})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${layer_directory})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${layer_directory})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${layer_directory})
 set(current_folder_path_prefix "./") # This works on windows too now with up-to-date OpenXR loaders

set(CMAKE_CXX_STANDARD 17)

#some platform specific twidling of filenames and path conventions
if(WIN32)
	set(library_filename "${library_name}.dll")
	if(msvc)
	add_compile_definitions(/w26812)
	endif()
else()
	set(library_filename "lib${library_name}.so")
endif(WIN32)

configure_file(
	${CMAKE_SOURCE_DIR}/src/layer_config.hpp.template
	${CMAKE_SOURCE_DIR}/src/layer_config.hpp
)

configure_file(
	${CMAKE_SOURCE_DIR}/src/manifest.json.template
	${CMAKE_SOURCE_DIR}/src/manifest.json
)

#TODO make this a bit nicer
file(GLOB layer_src CONFIGURE_DEPENDS src/*)

add_library(${library_name} SHARED ${layer_src} ${CMAKE_SOURCE_DIR}/README.md)

if(WIN32)
	set_target_properties(${library_name} PROPERTIES PREFIX "")
endif()

target_include_directories(${library_name} PRIVATE
	${CMAKE_SOURCE_DIR}/OpenXR-SDK/src/loader
	${CMAKE_SOURCE_DIR}/OpenXR-SDK/src/common
	${CMAKE_SOURCE_DIR}/OpenXR-SDK/include)

add_custom_target(install_manifest
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/src/manifest.json ${layer_directory})
add_dependencies(${library_name} install_manifest)

if(ANDROID)
	# This makes android's log library available to us
	# TODO find out how to do this properly
	# Current solution, see: https://stackoverflow.com/questions/45316915/undefined-reference-to-function-with-cmake-in-android-studio
	find_library(loglib log)
	target_link_libraries(${library_name} ${loglib})
endif()

# Several files use these compile time OS switches
if(WIN32)
    add_definitions(-DXR_OS_WINDOWS)
    add_definitions(-DNOMINMAX)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    add_definitions(-DXR_OS_LINUX)
elseif(ANDROID)
    add_definitions(-DXR_OS_ANDROID)
endif()
