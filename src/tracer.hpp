
#pragma once

#include <string>
#include <openxr/openxr.h>
#include <variant>

using namespace std;

namespace tracer
{

	enum Mode
	{
		RECORD,
		REPLAY
	};

	struct traceLocation
	{
		XrPosef pose;
		string basespace;
	};

	struct traceView
	{
		XrPosef pose;
		XrFovf fov;
		uint32_t type, index;
	};

	struct traceActionFloat
	{
		XrBool32 changed;
		float value;
		XrBool32 isActive;
		XrTime lastChanged;
	};

	struct traceActionVector2f
	{
		XrBool32 changed;
		XrVector2f value;
		XrBool32 isActive;
		XrTime lastChanged;
	};

	struct traceActionBoolean
	{
		XrBool32 changed;
		XrBool32 value;
		XrBool32 isActive;
		XrTime lastChanged;
	};

	struct traceApplyHaptic
	{
        XrBool32 value;
        XrTime lastChanged;
	};

	using traceBody = variant<traceLocation, traceView, traceActionBoolean, traceActionFloat, traceActionVector2f, traceApplyHaptic>;

	struct traceEntry
	{
		XrTime time;
		char type;
		string path;
		traceBody body;
	};

	Mode init();
	void close();
	void writeView(traceEntry);
	bool readNextView(traceEntry *);
	void writeSpace(traceEntry);
	bool readNextSpace(traceEntry *);
	void writeActionFloat(traceEntry);
	bool readNextActionFloat(traceEntry *);
	void writeActionBoolean(traceEntry);
	bool readNextActionBoolean(traceEntry *);
	void writeActionVector2f(traceEntry);
	bool readNextActionVector2f(traceEntry *);
	void writeApplyHaptic(traceEntry);
	bool readNextApplyHaptic(traceEntry *);
}
