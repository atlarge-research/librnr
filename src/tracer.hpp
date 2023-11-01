
#pragma once

#include <string>
#include <openxr/openxr.h>
#include <variant>

using namespace std;

namespace tracer {
	// trace format:
	// time s space w x y z x y z basespace
	// time v space w x y z x y z u r d l type index

	enum Mode {
		RECORD,
		REPLAY
	};

	struct traceLocation {
		XrPosef pose;
		string basespace;
	};

	struct traceView {
		XrPosef pose;
		XrFovf fov;
		uint32_t type, index;
	};

	struct traceActionFloat {
		XrBool32 changed;
		float value;
		XrBool32 isActive;
		XrTime lastChanged;
	};

	using traceBody = variant<traceLocation, traceView, traceActionFloat>;

	struct traceEntry {
		XrTime time;
		char type;
		string path;
		traceBody body;
	};

	void init(Mode);
	void close();
	void writeView(traceEntry);
	bool readNextView(traceEntry*);
	void writeSpace(traceEntry);
	bool readNextSpace(traceEntry*);
	void writeActionFloat(traceEntry);
}
