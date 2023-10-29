
#pragma once

#include <string>
#include <openxr/openxr.h>

using namespace std;

namespace tracer {
	// trace format:
	// time s space w x y z x y z basespace
	// time v space w x y z x y z u r d l type index

	struct traceEntry {
		XrTime time;
		char type;
		string space;
		float ow, ox, oy, oz, px, py, pz;
		string basespace;
		float u, r, d, l;
		uint32_t viewType, index;
	};

	void init();
	void close();
	void writeView(traceEntry);
	bool readNextView(traceEntry*);
	void writeSpace(traceEntry);
	bool readNextSpace(traceEntry*);
}
