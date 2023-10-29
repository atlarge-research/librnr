
#pragma once

#include <string>
#include <openxr/openxr.h>

using namespace std;

namespace tracer {
	// trace format:
	// time s space w x y z x y z basespace
	// time v space w x y z x y z u r d l type index

	enum Mode {
		RECORD,
		REPLAY
	};

	struct traceEntry {
		XrTime time;
		char type;
		string space;
		XrQuaternionf o;
		XrVector3f p;
		string basespace;
		float u, r, d, l;
		uint32_t viewType, index;
	};

	void init(Mode);
	void close();
	void writeView(traceEntry);
	bool readNextView(traceEntry*);
	void writeSpace(traceEntry);
	bool readNextSpace(traceEntry*);
}
