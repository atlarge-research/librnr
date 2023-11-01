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
#include <vector>

#include "logger.h"
#include "tracer.hpp"

using namespace std;

map<XrPath, string> pathToString;
map<string, XrPath> stringToPath;

map<XrAction, vector<XrPath>> actionBindingMap;
map<XrSpace, string> spaceToFullName;

tracer::Mode mode;
XrTime frameTime = 0;

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

	frameTime = frameEndInfo->displayTime;

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

	pathToString[*path] = pathString;
	stringToPath[pathString] = *path;

	return result;
}

XRAPI_ATTR XrResult XRAPI_CALL thisLayer_xrSuggestInteractionProfileBindings(XrInstance instance, const XrInteractionProfileSuggestedBinding* binding)
{
	static PFN_xrSuggestInteractionProfileBindings nextLayer_xrSuggestInteractionProfileBindings = GetNextLayerFunction(xrSuggestInteractionProfileBindings);
	const auto res = nextLayer_xrSuggestInteractionProfileBindings(instance, binding);
	for (auto i = 0; i < binding->countSuggestedBindings; i++)
	{
		auto b = binding->suggestedBindings[i];
		stringstream buffer;
		auto pathString = pathToString[b.binding];
		buffer << "RNR xrSuggestInteractionProfileBindings action " << b.action << " adding path " << pathString;
		Log::Write(Log::Level::Info, buffer.str());
		actionBindingMap[b.action].push_back(b.binding);
	}
	return res;
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

	stringstream buffer;
	buffer << "RNR xrCreateAction " << createInfo->actionName << " " << createInfo->localizedActionName << " " << *action;
	Log::Write(Log::Level::Info, buffer.str());

	buffer.str(string());
	buffer << "RNR xrCreateAction subactionpaths";
	for (auto i = 0; i < createInfo->countSubactionPaths; i++)
	{
		buffer << " " << pathToString[createInfo->subactionPaths[i]];
	}
	Log::Write(Log::Level::Info, buffer.str());

	return result;
}

XRAPI_ATTR XrResult XRAPI_CALL thisLayer_xrCreateActionSpace(XrSession session, const XrActionSpaceCreateInfo* createInfo, XrSpace* space)
{
	static PFN_xrCreateActionSpace nextLayer_xrCreateActionSpace = GetNextLayerFunction(xrCreateActionSpace);
	const auto result = nextLayer_xrCreateActionSpace(session, createInfo, space);

	// TODO check for createInfo->poseInActionSpace ??

	auto action = createInfo->action;
	auto paths = actionBindingMap[action];

	auto subpath = createInfo->subactionPath;
	auto subPathString = pathToString[subpath];

	for (auto p : paths)
	{
		auto ps = pathToString[p];
		if (ps.rfind(subPathString, 0) == 0)
		{
			stringstream buffer;
			buffer << "RNR xrCreateActionSpace mapping " << ps << " to space " << space;
			Log::Write(Log::Level::Info, buffer.str());
			spaceToFullName[*space] = pathToString[p];
		}
	}

	return result;
}

string refSpaceTypeToString(XrReferenceSpaceType ref)
{
	string type;
	switch (ref) {
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
	return type;
}

/// <summary>
/// This method is called to create "spaces" which are handles used to locate the position of the headset and controllers.
/// We need to figure out which handles correspond to which devices, so that we know which locations we are tracking.
/// </summary>
XRAPI_ATTR XrResult XRAPI_CALL thisLayer_xrCreateReferenceSpace(XrSession session, const XrReferenceSpaceCreateInfo* createInfo, XrSpace* space)
{
	static PFN_xrCreateReferenceSpace nextLayer_xrCreateReferenceSpace = GetNextLayerFunction(xrCreateReferenceSpace);
	const auto result = nextLayer_xrCreateReferenceSpace(session, createInfo, space);

	// Store string to space type, so that we can look it up later
	auto name = refSpaceTypeToString(createInfo->referenceSpaceType);
	spaceToFullName[*space] = name;

	stringstream buffer;
	auto p = createInfo->poseInReferenceSpace;
	buffer << "RNR xrCreateReferenceSpace type " << name << " posef " << p.orientation.w << " " << p.orientation.x << " " << p.orientation.y << " " << p.orientation.z << " " << p.position.x << " " << p.position.y << " " << p.position.z << " space " << *space;
	Log::Write(Log::Level::Info, buffer.str());

	return result;
}

XRAPI_ATTR XrResult XRAPI_CALL recordLocateSpace(XrSpace space, XrSpace baseSpace, XrTime time, XrSpaceLocation* location)
{
	static PFN_xrLocateSpace nextLayer_xrLocateSpace = GetNextLayerFunction(xrLocateSpace);

	static auto lastlog = std::chrono::system_clock::now();
	auto now = chrono::system_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastlog);
	string spaceString;
	string baseSpaceString;

	const auto result = nextLayer_xrLocateSpace(space, baseSpace, time, location);

	auto findBaseSpace = spaceToFullName.find(baseSpace);
	if (findBaseSpace == spaceToFullName.end()) {
		stringstream buffer;
		buffer << "RNR xrLocateSpace base space not tracked: " << baseSpace;
		Log::Write(Log::Level::Warning, buffer.str());

		baseSpaceString = "???";
	}
	else {
		baseSpaceString = findBaseSpace->second;
	}

	auto findSpace = spaceToFullName.find(space);
	if (findSpace == spaceToFullName.end()) {
		stringstream buffer;
		buffer << "RNR xrLocateSpace space not tracked: " << space;
		Log::Write(Log::Level::Warning, buffer.str());

		spaceString = "???";
	}
	else {
		spaceString = findSpace->second;
	}

	if (result == XR_SUCCESS && duration.count() > 8) {
		tracer::traceEntry entry;
		entry.time = time;
		entry.type = 's';
		entry.path = spaceToFullName[space];
		tracer::traceLocation l;
		l.pose = location->pose;
		l.basespace = baseSpaceString;
		entry.body = l;
		tracer::writeSpace(entry);
	}

	return result;
}

bool replayLocateSpace(XrSpace space, XrSpace baseSpace, XrTime time, XrSpaceLocation* location)
{
	tracer::traceEntry entry;
	entry.time = time;
	entry.path = spaceToFullName[space];
	tracer::traceLocation l;
	l.basespace = spaceToFullName[baseSpace];
	entry.body = l;

	if (tracer::readNextSpace(&entry))
	{
		auto& l = get<tracer::traceLocation>(entry.body);
		location->pose = l.pose;
		location->locationFlags = XR_SPACE_LOCATION_ORIENTATION_VALID_BIT | XR_SPACE_LOCATION_POSITION_VALID_BIT | XR_SPACE_LOCATION_ORIENTATION_TRACKED_BIT | XR_SPACE_LOCATION_POSITION_TRACKED_BIT;
		return true;
	}

	//Log::Write(Log::Level::Info, "RNR replayLocateSpace rewrote space loc!");

	return false;
}

XRAPI_ATTR XrResult XRAPI_CALL thisLayer_xrLocateSpace(XrSpace space, XrSpace baseSpace, XrTime time, XrSpaceLocation* location)
{
	static PFN_xrLocateSpace nextLayer_xrLocateSpace = GetNextLayerFunction(xrLocateSpace);

	if (mode == tracer::Mode::REPLAY)
	{
		auto res = nextLayer_xrLocateSpace(space, baseSpace, time, location);
		if (replayLocateSpace(space, baseSpace, time, location)) {
			Log::Write(Log::Level::Info, "RNR location should be overwritten!");
		}
		return res;
	}
	else
	{
		return recordLocateSpace(space, baseSpace, time, location);
	}
}

XRAPI_ATTR XrResult XRAPI_CALL replayLocateViews(XrSession session, const XrViewLocateInfo* viewlocateInfo, XrViewState* viewState,
	uint32_t viewCapacityInput, uint32_t* viewCountOutput, XrView* views)
{
	tracer::traceEntry entry;
	entry.time = viewlocateInfo->displayTime;
	entry.path = spaceToFullName[viewlocateInfo->space];
	tracer::traceView w;
	w.index = 0;
	entry.body = w;
	tracer::readNextView(&entry);

	auto& wl = get<tracer::traceView>(entry.body);
	views[0].fov.angleUp = wl.fov.angleUp;
	views[0].fov.angleRight = wl.fov.angleRight;
	views[0].fov.angleDown = wl.fov.angleDown;
	views[0].fov.angleLeft = wl.fov.angleLeft;
	views[0].pose = wl.pose;

	// TODO support mono view https://registry.khronos.org/OpenXR/specs/1.0/man/html/XrViewConfigurationType.html
	entry.time = viewlocateInfo->displayTime;
	entry.path = spaceToFullName[viewlocateInfo->space];
	wl.index = 1;
	tracer::readNextView(&entry);

	auto& wr = get<tracer::traceView>(entry.body);
	views[0].fov.angleUp = wr.fov.angleUp;
	views[0].fov.angleRight = wr.fov.angleRight;
	views[0].fov.angleDown = wr.fov.angleDown;
	views[0].fov.angleLeft = wr.fov.angleLeft;
	views[0].pose = wr.pose;

	*viewCountOutput = 2;

	return XR_SUCCESS;
}

// FIXME also trace view mode
XRAPI_ATTR XrResult XRAPI_CALL recordLocateViews(XrSession session, const XrViewLocateInfo* viewlocateInfo, XrViewState* viewState,
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
			tracer::traceEntry entry;
			entry.time = viewlocateInfo->displayTime;
			entry.type = 'v';
			// TODO check if this entry exists in map
			entry.path = spaceToFullName[viewlocateInfo->space];
			tracer::traceView w;
			w.pose = view.pose;
			w.index = i;
			entry.body = w;
			tracer::writeView(entry);
		}
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
	static PFN_xrLocateViews nextLayer_xrLocateViews = GetNextLayerFunction(xrLocateViews);
	if (mode == tracer::Mode::REPLAY)
	{
		// FIXME enable
		//return replayLocateViews(session, viewlocateInfo, viewState, viewCapacityInput, viewCountOutput, views);
		return nextLayer_xrLocateViews(session, viewlocateInfo, viewState, viewCapacityInput, viewCountOutput, views);
	}
	else
	{
		return recordLocateViews(session, viewlocateInfo, viewState, viewCapacityInput, viewCountOutput, views);
	}
}

bool replayGetActionStateFloat(const XrActionStateGetInfo* getInfo, XrActionStateFloat* state)
{
	tracer::traceEntry e;
	e.time = frameTime;
	// look up the paths mapped to this action
	if (auto search = actionBindingMap.find(getInfo->action); search != actionBindingMap.end())
	{
		// iterate over all paths for this action
		auto paths = actionBindingMap[getInfo->action];
		for (auto p : paths)
		{
			// check which path got activated
			auto ps = pathToString[p];
			auto pss = pathToString[getInfo->subactionPath];
			if (ps.rfind(pss, 0) == 0)
			{
				e.path = ps;
			}
		}
	}
	e.body = tracer::traceActionFloat{};
	if (!tracer::readNextActionFloat(&e))
	{
		return false;
	}
	assert(holds_alternative<tracer::traceActionFloat>(e.body));
	auto& f = get<tracer::traceActionFloat>(e.body);
	state->changedSinceLastSync = f.changed;
	state->currentState = f.value;
	state->isActive = true;
	state->lastChangeTime = f.lastChanged;
	return true;
}

void recordGetActionStateFloat(const XrActionStateGetInfo* getInfo, XrActionStateFloat* state)
{
	static bool previousWasChanged = true;
	auto changed = state->changedSinceLastSync;
	auto value = state->currentState;
	auto isActive = state->isActive;
	auto lastChanged = state->lastChangeTime;

	// we don't need all values. especially if there are long periods that noting happens. check if we need to log
	if (changed || (previousWasChanged && !changed))
	{
		// look up the paths mapped to this action
		if (auto search = actionBindingMap.find(getInfo->action); search != actionBindingMap.end())
		{
			// iterate over all paths for this action
			auto paths = actionBindingMap[getInfo->action];
			for (auto p : paths)
			{
				// check which path got activated
				auto ps = pathToString[p];
				auto pss = pathToString[getInfo->subactionPath];
				if (ps.rfind(pss, 0) == 0)
				{
					// log the path and the data for the action
					tracer::traceEntry e = { frameTime, 'f', ps };
					tracer::traceActionFloat taf = { changed, value, isActive, lastChanged };
					e.body = taf;
					tracer::writeActionFloat(e);
				}
			}
		}
	}
	previousWasChanged = changed;
}

XRAPI_ATTR XrResult XRAPI_CALL thisLayer_xrGetActionStateFloat(XrSession session, const XrActionStateGetInfo* getInfo, XrActionStateFloat* state)
{
	static PFN_xrGetActionStateFloat nextLayer_xrGetActionStateFloat = GetNextLayerFunction(xrGetActionStateFloat);
	auto res = nextLayer_xrGetActionStateFloat(session, getInfo, state);
	if (mode == tracer::Mode::REPLAY)
	{
		replayGetActionStateFloat(getInfo, state);
	}
	else
	{
		recordGetActionStateFloat(getInfo, state);
	}
	return res;
}

bool replayGetActionStateBoolean(const XrActionStateGetInfo* getInfo, XrActionStateBoolean* state)
{
	tracer::traceEntry e;
	e.time = frameTime;
	// look up the paths mapped to this action
	if (auto search = actionBindingMap.find(getInfo->action); search != actionBindingMap.end())
	{
		// iterate over all paths for this action
		auto paths = actionBindingMap[getInfo->action];
		for (auto p : paths)
		{
			// check which path got activated
			auto ps = pathToString[p];
			auto pss = pathToString[getInfo->subactionPath];
			if (ps.rfind(pss, 0) == 0)
			{
				e.path = ps;
			}
		}
	}
	e.body = tracer::traceActionBoolean{};
	if (!tracer::readNextActionBoolean(&e))
	{
		return false;
	}
	assert(holds_alternative<tracer::traceActionBoolean>(e.body));
	auto& f = get<tracer::traceActionBoolean>(e.body);
	state->changedSinceLastSync = f.changed;
	state->currentState = f.value;
	state->isActive = true;
	state->lastChangeTime = f.lastChanged;
	return true;
}

void recordGetActionStateBoolean(const XrActionStateGetInfo* getInfo, XrActionStateBoolean* state)
{
	static bool previousWasChanged = true;
	auto changed = state->changedSinceLastSync;
	auto value = state->currentState;
	auto isActive = state->isActive;
	auto lastChanged = state->lastChangeTime;

	// we don't need all values. especially if there are long periods that noting happens. check if we need to log
	if (changed || (previousWasChanged && !changed))
	{
		// look up the paths mapped to this action
		if (auto search = actionBindingMap.find(getInfo->action); search != actionBindingMap.end())
		{
			// iterate over all paths for this action
			auto paths = actionBindingMap[getInfo->action];
			for (auto p : paths)
			{
				// check which path got activated
				auto ps = pathToString[p];
				auto pss = pathToString[getInfo->subactionPath];
				if (ps.rfind(pss, 0) == 0)
				{
					// log the path and the data for the action
					tracer::traceEntry e = { frameTime, 'b', ps };
					tracer::traceActionBoolean taf = { changed, value, isActive, lastChanged };
					e.body = taf;
					tracer::writeActionBoolean(e);
				}
			}
		}
	}
	previousWasChanged = changed;
}

XRAPI_ATTR XrResult XRAPI_CALL thisLayer_xrGetActionStateBoolean(XrSession session, const XrActionStateGetInfo* getInfo, XrActionStateBoolean* state)
{
	static PFN_xrGetActionStateBoolean nextLayer_xrGetActionStateBoolean = GetNextLayerFunction(xrGetActionStateBoolean);
	auto res = nextLayer_xrGetActionStateBoolean(session, getInfo, state);
	if (mode == tracer::Mode::REPLAY)
	{
		replayGetActionStateBoolean(getInfo, state);
	}
	else
	{
		recordGetActionStateBoolean(getInfo, state);
	}
	return res;
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
	mode = tracer::Mode::REPLAY;
	tracer::init(mode);

	std::vector<OpenXRLayer::ShimFunction> functions;
	functions.emplace_back("xrDestroyInstance", PFN_xrVoidFunction(thisLayer_xrDestroyInstance));

	// List every functions that is callable on this API layer
	functions.emplace_back("xrEndFrame", PFN_xrVoidFunction(thisLayer_xrEndFrame));
	functions.emplace_back("xrStringToPath", PFN_xrVoidFunction(thisLayer_xrStringToPath));
	functions.emplace_back("xrSuggestInteractionProfileBindings", PFN_xrVoidFunction(thisLayer_xrSuggestInteractionProfileBindings));
	functions.emplace_back("xrLocateViews", PFN_xrVoidFunction(thisLayer_xrLocateViews));
	functions.emplace_back("xrCreateActionSet", PFN_xrVoidFunction(thisLayer_xrCreateActionSet));
	functions.emplace_back("xrCreateAction", PFN_xrVoidFunction(thisLayer_xrCreateAction));
	functions.emplace_back("xrCreateActionSpace", PFN_xrVoidFunction(thisLayer_xrCreateActionSpace));
	functions.emplace_back("xrCreateReferenceSpace", PFN_xrVoidFunction(thisLayer_xrCreateReferenceSpace));
	functions.emplace_back("xrGetActionStateBoolean", PFN_xrVoidFunction(thisLayer_xrGetActionStateBoolean));
	functions.emplace_back("xrGetActionStateFloat", PFN_xrVoidFunction(thisLayer_xrGetActionStateFloat));
	functions.emplace_back("xrLocateSpace", PFN_xrVoidFunction(thisLayer_xrLocateSpace));

#if XR_THISLAYER_HAS_EXTENSIONS
	if (OpenXRLayer::IsExtensionEnabled("XR_TEST_test_me"))
		functions.emplace_back("xrTestMeTEST", PFN_xrVoidFunction(thisLayer_xrTestMeTEST));
#endif

	return functions;
}
