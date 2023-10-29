// SPDX-FileCopyrightText: 2021-2023 Arthur Brainville (Ybalrid) <ybalrid@ybalrid.info>
//
// SPDX-License-Identifier: MIT
//
// Initial Author: Arthur Brainville <ybalrid@ybalrid.info>

#include "layer_shims.hpp"

#include <cassert>
#include <iostream>
#include <sstream>
#include <chrono>
#include <map>
#include <string>
#include <iostream>
#include <fstream>

#include "logger.h"
#include "tracer.hpp"

using namespace std;

std::map<XrPath, string> pathToString;
std::map<string, XrPath> stringToPath;
map<XrSpace, string> spaceMap;

const string leftHandStr = "/user/hand/left";
const string rightHandStr = "/user/hand/right";
const string headStr = "user/head";

// IMPORTANT: to allow for multiple instance creation/destruction, the context of the layer must be re-initialized when the instance is being destroyed.
// Hooking xrDestroyInstance is the best way to do that.
XRAPI_ATTR XrResult XRAPI_CALL thisLayer_xrDestroyInstance(
	XrInstance instance)
{
	// Close trace filestream
	tracer::close();

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
	buffer << "RNR xrStringToPath " << instance << " " << pathString << " " << *path;
	Log::Write(Log::Level::Info, buffer.str());

	// Debugging code. Just testing hypothesis that the same string always results in the same XrPath
	// TODO: also track the xrinstance name and handle
	if (auto search = stringToPath.find(pathString); search != stringToPath.end()) {
		if (search->second != *path) {
			buffer.str(std::string());
			buffer << "RNR xrStringToPath STRANGE! Multiple paths for the same string " << search->first << " " << pathString << " " << search->second << " " << *path;
			Log::Write(Log::Level::Warning, buffer.str());
		}
	}

	pathToString[*path] = pathString;
	stringToPath[pathString] = *path;

	return result;
}

XRAPI_ATTR XrResult XRAPI_CALL thisLayer_xrCreateActionSet(XrInstance instance, const XrActionSetCreateInfo* createInfo, XrActionSet* actionSet)
{
	static PFN_xrCreateActionSet nextLayer_xrCreateActionSet = GetNextLayerFunction(xrCreateActionSet);
	const auto result = nextLayer_xrCreateActionSet(instance, createInfo, actionSet);

	stringstream buffer;
	buffer << "RNR xrCreateActionSet " << createInfo->actionSetName << " " << createInfo->localizedActionSetName << " " << *actionSet;
	Log::Write(Log::Level::Info, buffer.str());

	return result;
}

XRAPI_ATTR XrResult XRAPI_CALL thisLayer_xrCreateAction(XrActionSet actionSet, const XrActionCreateInfo* createInfo, XrAction* action)
{
	static PFN_xrCreateAction nextLayer_xrCreateAction = GetNextLayerFunction(xrCreateAction);
	const auto result = nextLayer_xrCreateAction(actionSet, createInfo, action);

	if (createInfo->actionType == XR_ACTION_TYPE_POSE_INPUT) {
		// Creating action for a controller!
		stringstream buffer;
		buffer << "RNR xrCreateAction " << createInfo->actionName << " " << createInfo->localizedActionName << " " << *action;
		Log::Write(Log::Level::Info, buffer.str());
	}

	return result;
}

XRAPI_ATTR XrResult XRAPI_CALL thisLayer_xrCreateActionSpace(XrSession session, const XrActionSpaceCreateInfo* createInfo, XrSpace* space)
{
	static PFN_xrCreateActionSpace nextLayer_xrCreateActionSpace = GetNextLayerFunction(xrCreateActionSpace);
	const auto result = nextLayer_xrCreateActionSpace(session, createInfo, space);

	if (auto search = pathToString.find(createInfo->subactionPath); search != pathToString.end()) {
		if (!search->second.compare(leftHandStr)) {
			std::stringstream buffer;
			buffer << "RNR xrCreateActionSpace " << search->second << " " << createInfo->action << " " << *space;
			Log::Write(Log::Level::Info, buffer.str());
			spaceMap[*space] = leftHandStr;
		}
		else if (!search->second.compare(rightHandStr)) {
			std::stringstream buffer;
			buffer << "RNR xrCreateActionSpace " << search->second << " " << createInfo->action << " " << *space;
			Log::Write(Log::Level::Info, buffer.str());
			spaceMap[*space] = rightHandStr;
		}
		else if (!search->second.compare(headStr)) {
			std::stringstream buffer;
			buffer << "RNR xrCreateActionSpace " << search->second << " " << createInfo->action << " " << *space;
			Log::Write(Log::Level::Info, buffer.str());
			spaceMap[*space] = headStr;
		}
	}

	//std::stringstream buffer;
	//buffer << "RNR xrCreateActionSpace " << createInfo.
	//Log::Write(Log::Level::Info, buffer.str());

	return result;
}

/// <summary>
/// This method is called to create "spaces" which are handles used to locate the position of the headset and controllers.
/// We need to figure out which handles correspond to which devices, so that we know which locations we are tracking.
/// </summary>
XRAPI_ATTR XrResult XRAPI_CALL thisLayer_xrCreateReferenceSpace(XrSession session, const XrReferenceSpaceCreateInfo* createInfo, XrSpace* space)
{
	static PFN_xrCreateReferenceSpace nextLayer_xrCreateReferenceSpace = GetNextLayerFunction(xrCreateReferenceSpace);
	const auto result = nextLayer_xrCreateReferenceSpace(session, createInfo, space);

	string type;
	switch (createInfo->referenceSpaceType) {
	case XR_REFERENCE_SPACE_TYPE_VIEW:
		type = "XR_REFERENCE_SPACE_TYPE_VIEW";
		break;
	case XR_REFERENCE_SPACE_TYPE_LOCAL:
		type = "XR_REFERENCE_SPACE_TYPE_LOCAL";
		break;
	case XR_REFERENCE_SPACE_TYPE_STAGE:
		type = "XR_REFERENCE_SPACE_TYPE_STAGE";
		break;
	default:
		type = "OTHER";
		break;
	}

	// Store string to space type, so that we can look it up later
	spaceMap[*space] = type;

	stringstream buffer;
	buffer << "RNR xrCreateReferenceSpace created space type " << type << " " << *space;
	Log::Write(Log::Level::Verbose, buffer.str());

	return result;
}

XRAPI_ATTR XrResult XRAPI_CALL thisLayer_xrLocateSpace(XrSpace space, XrSpace baseSpace, XrTime time, XrSpaceLocation* location)
{
	// Not sure yet how to get positions of controllers / headsets out of this
	static PFN_xrLocateSpace nextLayer_xrLocateSpace = GetNextLayerFunction(xrLocateSpace);

	static auto lastlog = std::chrono::system_clock::now();
	auto now = chrono::system_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastlog);
	string spaceString;
	string baseSpaceString;

	const auto result = nextLayer_xrLocateSpace(space, baseSpace, time, location);

	auto findBaseSpace = spaceMap.find(baseSpace);
	if (findBaseSpace == spaceMap.end()) {
		stringstream buffer;
		buffer << "RNR xrLocateSpace base space not tracked: " << baseSpace;
		Log::Write(Log::Level::Warning, buffer.str());

		baseSpaceString = "???";
	}
	else {
		baseSpaceString = findBaseSpace->second;
	}

	auto findSpace = spaceMap.find(space);
	if (findSpace == spaceMap.end()) {
		stringstream buffer;
		buffer << "RNR xrLocateSpace space not tracked: " << space;
		Log::Write(Log::Level::Warning, buffer.str());

		spaceString = "???";
	}
	else {
		spaceString = findSpace->second;
	}

	if (result == XR_SUCCESS && duration.count() > 8) {
		auto o = location->pose.orientation;
		auto p = location->pose.position;
		tracer::traceEntry entry;
		entry.time = time;
		entry.type = 's';
		entry.space = spaceString;
		entry.ow = o.w;
		entry.ox = o.x;
		entry.oy = o.y;
		entry.oz = o.z;
		entry.px = p.x;
		entry.py = p.y;
		entry.pz = p.z;
		entry.basespace = baseSpaceString;
		tracer::writeSpace(entry);
	}

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
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastlog);

	// input: the type of view (MONO, STEREO, etc.)
	auto configType = viewlocateInfo->viewConfigurationType;
	// input: the time for which to predict view orientation and position
	auto displayTime = viewlocateInfo->displayTime;

	// output: flags indicating if the results are valid
	auto flags = viewState->viewStateFlags;
	auto outputValid = (flags & (XR_VIEW_STATE_ORIENTATION_VALID_BIT | XR_VIEW_STATE_POSITION_VALID_BIT));

	// Test printing the user's head view location at most once per second
	if (result == XR_SUCCESS && *viewCountOutput > 0 && outputValid && duration.count() > 8) {
		std::stringstream buf;
		buf << "logging " << *viewCountOutput << " views";
		//Log::Write(Log::Level::Info, buf.str());
		lastlog = now;

		for (auto i = 0; i < *viewCountOutput; i++) {
			const auto view = views[i];
			const auto o = view.pose.orientation;
			const auto p = view.pose.position;
			tracer::traceEntry entry;
			entry.time = viewlocateInfo->displayTime;
			entry.type = 'v';
			entry.space = spaceMap[viewlocateInfo->space];
			entry.ow = o.w;
			entry.ox = o.x;
			entry.oy = o.y;
			entry.oz = o.z;
			entry.px = p.x;
			entry.py = p.y;
			entry.pz = p.z;
			// TODO check if this entry exists in map
			entry.index = i;
			tracer::writeView(entry);
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
	// TODO move this to another function. Does not belong here.
	tracer::init();

	std::vector<OpenXRLayer::ShimFunction> functions;
	functions.emplace_back("xrDestroyInstance", PFN_xrVoidFunction(thisLayer_xrDestroyInstance));

	// List every functions that is callable on this API layer
	functions.emplace_back("xrEndFrame", PFN_xrVoidFunction(thisLayer_xrEndFrame));
	functions.emplace_back("xrStringToPath", PFN_xrVoidFunction(thisLayer_xrStringToPath));
	functions.emplace_back("xrLocateViews", PFN_xrVoidFunction(thisLayer_xrLocateViews));
	functions.emplace_back("xrCreateActionSet", PFN_xrVoidFunction(thisLayer_xrCreateActionSet));
	functions.emplace_back("xrCreateAction", PFN_xrVoidFunction(thisLayer_xrCreateAction));
	functions.emplace_back("xrCreateActionSpace", PFN_xrVoidFunction(thisLayer_xrCreateActionSpace));
	functions.emplace_back("xrCreateReferenceSpace", PFN_xrVoidFunction(thisLayer_xrCreateReferenceSpace));
	functions.emplace_back("xrLocateSpace", PFN_xrVoidFunction(thisLayer_xrLocateSpace));
	//functions.emplace_back("xrLocateSpace", PFN_xrVoidFunction(thisLayer_xrLocateSpace));

#if XR_THISLAYER_HAS_EXTENSIONS
	if (OpenXRLayer::IsExtensionEnabled("XR_TEST_test_me"))
		functions.emplace_back("xrTestMeTEST", PFN_xrVoidFunction(thisLayer_xrTestMeTEST));
#endif

	return functions;
}
