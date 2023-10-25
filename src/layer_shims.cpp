// SPDX-FileCopyrightText: 2021-2023 Arthur Brainville (Ybalrid) <ybalrid@ybalrid.info>
//
// SPDX-License-Identifier: MIT
//
// Initial Author: Arthur Brainville <ybalrid@ybalrid.info>

#include "layer_shims.hpp"
#include "logger.h"

#include <cassert>
#include <iostream>
#include <sstream>
#include <chrono>
#include <map>
#include <string>

using namespace std;

std::map<XrPath*, string> pathToString;
std::map<string, XrPath*> stringToPath;

// IMPORTANT: to allow for multiple instance creation/destruction, the context of the layer must be re-initialized when the instance is being destroyed.
// Hooking xrDestroyInstance is the best way to do that.
XRAPI_ATTR XrResult XRAPI_CALL thisLayer_xrDestroyInstance(
	XrInstance instance)
{
	PFN_xrDestroyInstance nextLayer_xrDestroyInstance = GetNextLayerFunction(xrDestroyInstance);

	OpenXRLayer::DestroyLayerContext();

	assert(nextLayer_xrDestroyInstance != nullptr);
	return nextLayer_xrDestroyInstance(instance);
}

// Define the functions implemented in this layer like this:
XRAPI_ATTR XrResult XRAPI_CALL thisLayer_xrEndFrame(XrSession session,
	const XrFrameEndInfo* frameEndInfo)
{
	// First time this runs, it will fetch the pointer from the loaded OpenXR dispatch table
	static PFN_xrEndFrame nextLayer_xrEndFrame = GetNextLayerFunction(xrEndFrame);

	// Do some additional things;
	std::cout << "Display frame time is " << frameEndInfo->displayTime << "\n";

	// Call down the chain
	const auto result = nextLayer_xrEndFrame(session, frameEndInfo);

	// Maybe do something with the original return value?
	if (result == XR_ERROR_TIME_INVALID)
		std::cout << "frame time is invalid?\n";

	// Return what should be returned as if you were the actual function
	return result;
}

XRAPI_ATTR XrResult XRAPI_CALL thisLayer_xrStringToPath(XrInstance instance, const char* pathString, XrPath* path)
{
	static PFN_xrStringToPath nextLayer_xrStringToPath = GetNextLayerFunction(xrStringToPath);
	const auto result = nextLayer_xrStringToPath(instance, pathString, path);
	std::stringstream buffer;
	buffer << "RNR xrStringToPath " << pathString << " " << path;
	Log::Write(Log::Level::Info, buffer.str());

	// Debugging code. Just testing hypothesis that the same string always results in the same XrPath pointer !!! Pointer need not be the same -> enum value
	// !!! Don't use pointers to XrPaths in the map, they're just numbers
	auto getFromMap = stringToPath[pathString];
	if (auto search = stringToPath.find(pathString); search != stringToPath.end()) {
		if (stringToPath[search->first] != path) {
			buffer.str(std::string());
			buffer << "RNR xrStringToPath STRANGE! Multiple paths for the same string " << pathString << " " << stringToPath[search->first] << " " << pathString;
			Log::Write(Log::Level::Info, buffer.str());
		}
	}

	pathToString[path] = pathString;
	stringToPath[pathString] = path;

	return result;
}

XRAPI_ATTR XrResult XRAPI_CALL thisLayer_xrCreateAction(XrActionSet actionSet, const XrActionCreateInfo* createInfo, XrAction* action)
{
	static PFN_xrCreateAction nextLayer_xrCreateAction = GetNextLayerFunction(xrCreateAction);
	const auto result = nextLayer_xrCreateAction(actionSet, createInfo, action);

	if (createInfo->actionType == XR_ACTION_TYPE_POSE_INPUT) {
		// Creating action for a controller!
		
	}

	return result;
}

XRAPI_ATTR XrResult XRAPI_CALL thisLayer_xrCreateActionSpace(XrSession session, const XrActionSpaceCreateInfo* createInfo, XrSpace* space)
{
	static PFN_xrCreateActionSpace nextLayer_xrCreateActionSpace = GetNextLayerFunction(xrCreateActionSpace);

	std::stringstream buffer;
	//buffer << "RNR xrCreateActionSpace " << createInfo.
	Log::Write(Log::Level::Info, buffer.str());

	const auto result = nextLayer_xrCreateActionSpace(session, createInfo, space);
	return result;
}

/// <summary>
/// This method is called to create "spaces" which are handles used to locate the position of the headset and controllers.
/// We need to figure out which handles correspond to which devices, so that we know which locations we are tracking.
/// </summary>
XRAPI_ATTR XrResult XRAPI_CALL thisLayer_xrCreateReferenceSpace(XrSession session, const XrReferenceSpaceCreateInfo* createInfo, XrSpace* space)
{
	static PFN_xrCreateReferenceSpace nextLayer_xrCreateReferenceSpace = GetNextLayerFunction(xrCreateReferenceSpace);

	if (createInfo->referenceSpaceType == XR_REFERENCE_SPACE_TYPE_VIEW) {
		// We are creating a space for the headset.
	}

	const auto result = nextLayer_xrCreateReferenceSpace(session, createInfo, space);
	return result;
}

XRAPI_ATTR XrResult XRAPI_CALL thisLayer_xrLocateSpace(XrSpace space, XrSpace baseSpace, XrTime time, XrSpaceLocation* location)
{
	// Not sure yet how to get positions of controllers / headsets out of this
	static PFN_xrLocateSpace nextLayer_xrLocateSpace = GetNextLayerFunction(xrLocateSpace);
	const auto result = nextLayer_xrLocateSpace(space, baseSpace, time, location);
	return result;
}

/// <summary>
/// This seems to successfully get the position and the orientation of both eyes.
/// I'm not sure if this also conttrols the head position, or if this is relative to the head position.
/// </summary>
XRAPI_ATTR XrResult XRAPI_CALL thisLayer_xrLocateViews(XrSession session, const XrViewLocateInfo* viewlocateInfo, XrViewState* viewState,
	uint32_t viewCapacityInput, uint32_t* viewCountOutput, XrView* views)
{
	using namespace std::chrono_literals;

	static PFN_xrLocateViews nextLayer_xrLocateViews = GetNextLayerFunction(xrLocateViews);
	static uint32_t times = 0;
	static std::stringstream buffer;
	static auto lastlog = std::chrono::system_clock::now();

	const auto result = nextLayer_xrLocateViews(session, viewlocateInfo, viewState, viewCapacityInput, viewCountOutput, views);

	auto now = std::chrono::system_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - lastlog);

	// input: the type of view (MONO, STEREO, etc.)
	auto configType = viewlocateInfo->viewConfigurationType;
	// input: the time for which to predict view orientation and position
	auto displayTime = viewlocateInfo->displayTime;

	// output: flags indicating if the results are valid
	auto flags = viewState->viewStateFlags;
	auto outputValid = (flags & (XR_VIEW_STATE_ORIENTATION_VALID_BIT | XR_VIEW_STATE_POSITION_VALID_BIT));

	// Test printing the user's head view location at most once per second
	if (result == XR_SUCCESS && *viewCountOutput > 0 && outputValid && duration.count() > 1) {
		std::stringstream buf;
		buf << "logging " << *viewCountOutput << " views";
		Log::Write(Log::Level::Info, buf.str());
		lastlog = now;

		for (auto i = 0; i < *viewCountOutput; i++) {
			const auto view = views[i];
			const auto fov = view.fov;
			const auto pose = view.pose;
			const auto ori = pose.orientation;
			const auto pos = pose.position;
			// Clear the string stream
			buffer.str(std::string());
			// Log head position
			buffer << "view " << i << " fov " << fov.angleUp << " " << fov.angleRight << " " << fov.angleDown << " " << fov.angleLeft << " ";
			buffer << "pose ori " << ori.w << " " << ori.x << " " << ori.y << " " << ori.z << " ";
			buffer << "pos " << pos.x << " " << pos.y << " " << pos.z;
			Log::Write(Log::Level::Info, buffer.str());
		}
	}

	return result;
}

#if XR_THISLAYER_HAS_EXTENSIONS
// The following function doesn't exist in the spec, this is just a test for the extension mecanism
XRAPI_ATTR XrResult XRAPI_CALL thisLayer_xrTestMeTEST(XrSession session)
{
	(void)session;
	std::cout << "xrTestMe()\n";
	return XR_SUCCESS;
}
#endif

// This functions returns the list of function pointers and name we implement, and is called during the initialization of the layer:
std::vector<OpenXRLayer::ShimFunction> ListShims()
{
	std::vector<OpenXRLayer::ShimFunction> functions;
	functions.emplace_back("xrDestroyInstance", PFN_xrVoidFunction(thisLayer_xrDestroyInstance));

	// List every functions that is callable on this API layer
	functions.emplace_back("xrEndFrame", PFN_xrVoidFunction(thisLayer_xrEndFrame));
	functions.emplace_back("xrStringToPath", PFN_xrVoidFunction(thisLayer_xrStringToPath));
	functions.emplace_back("xrLocateViews", PFN_xrVoidFunction(thisLayer_xrLocateViews));
	//functions.emplace_back("xrLocateSpace", PFN_xrVoidFunction(thisLayer_xrLocateSpace));

#if XR_THISLAYER_HAS_EXTENSIONS
	if (OpenXRLayer::IsExtensionEnabled("XR_TEST_test_me"))
		functions.emplace_back("xrTestMeTEST", PFN_xrVoidFunction(thisLayer_xrTestMeTEST));
#endif

	return functions;
}
