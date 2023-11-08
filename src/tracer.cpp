
#include "tracer.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <cassert>

#include "logger.h"

namespace tracer {
	fstream trace;

	XrTime mostRecentEntry;
	map<string, map<string, traceEntry>> spaceMap;
	map<string, map<uint32_t, traceEntry>> viewMap;
	map<string, traceEntry> floatActionMap;
	map<string, traceEntry> booleanActionMap;

	void init(Mode m)
	{
		ios_base::openmode filemode;
		switch (m)
		{
		case REPLAY:
			filemode = fstream::in;
			break;
		case RECORD:
		default:
			filemode = fstream::out | fstream::trunc;
			break;
		}
		trace.open("trace.txt", filemode);
	}

	void close()
	{
		trace.close();
	}

	void writeHead(traceEntry entry)
	{
		// The first time we are asked to write an entry, set the requested timestamp as zero.
		static XrTime start = entry.time;
		trace << entry.time - start << " " << entry.type << " " << entry.path << " ";
	}

	void writeSpace(traceEntry entry)
	{
		assert(holds_alternative<traceLocation>(entry.body));

		writeHead(entry);
		auto& l = get<traceLocation>(entry.body);
		auto& pose = l.pose;
		auto& o = pose.orientation;
		auto& p = pose.position;
		auto& basespace = l.basespace;
		trace << " " << o.x << " " << o.y << " " << o.z << " " << o.w;
		trace << " " << p.x << " " << p.y << " " << p.z;
		trace << " " << basespace;
		trace << endl;
	}

	void writeView(traceEntry entry)
	{
		assert(holds_alternative<traceView>(entry.body));

		writeHead(entry);
		auto& w = get<traceView>(entry.body);
		auto& pose = w.pose;
		auto& o = pose.orientation;
		auto& p = pose.position;
		auto& fov = w.fov;
		auto& u = fov.angleUp;
		auto& r = fov.angleRight;
		auto& d = fov.angleDown;
		auto& l = fov.angleLeft;
		auto& type = w.type;
		auto& index = w.index;
		trace << o.x << " " << o.y << " " << o.z << " " << o.w;
		trace << p.x << " " << p.y << " " << p.z;
		trace << u << " " << r << " " << d << " " << l << " " << type << " " << index;
		trace << endl;
	}

	bool read(traceEntry* entry)
	{
		// The first time we are asked to read an entry, set the requested timestamp as zero.
		static XrTime start = entry->time;

		Log::Write(Log::Level::Info, "A");

		// Read a line from the trace
		string line;
		if (!getline(trace, line))
		{
			return false;
		}

		stringstream sstream(line);
		if (!(sstream >> entry->time >> entry->type >> entry->path))
		{
			Log::Write(Log::Level::Warning, "RNR read end of file!");
			return false;
		}

		Log::Write(Log::Level::Info, "B");

		// Depending on the trace record type, we need to read different fields
		if (entry->type == 's') {
			traceLocation l;
			auto& o = l.pose.orientation;
			auto& p = l.pose.position;
			sstream >> o.x >> o.y >> o.z >> o.w >> p.x >> p.y >> p.z;
			sstream >> l.basespace;
			entry->body = l;
			spaceMap[entry->path][l.basespace] = *entry;
		}
		else if (entry->type == 'v') {
			traceView v;
			auto& o = v.pose.orientation;
			auto& p = v.pose.position;
			auto& u = v.fov.angleUp;
			auto& r = v.fov.angleRight;
			auto& d = v.fov.angleDown;
			auto& l = v.fov.angleLeft;
			auto& type = v.type;
			auto& index = v.index;
			sstream >> o.x >> o.y >> o.z >> o.w >> p.x >> p.y >> p.z;
			sstream >> u >> r >> d >> l >> type >> index;
			entry->body = v;
			viewMap[entry->path][index] = *entry;
		}
		else if (entry->type == 'b')
		{
			traceActionBoolean b{};
			auto& changed = b.changed;
			auto& isActive = b.isActive;
			auto& lastChanged = b.lastChanged;
			auto& value = b.value;
			sstream >> changed >> isActive >> lastChanged >> value;
			entry->body = b;
			booleanActionMap[entry->path] = *entry;
		}
		else if (entry->type == 'f')
		{
			traceActionFloat f{};
			auto& changed = f.changed;
			auto& isActive = f.isActive;
			auto& lastChanged = f.lastChanged;
			auto& value = f.value;
			sstream >> changed >> isActive >> lastChanged >> value;
			entry->body = f;
			floatActionMap[entry->path] = *entry;
		}
		else {
			stringstream buffer;
			buffer << "RNR ERROR invalid trace entry type: " << entry->type;
			Log::Write(Log::Level::Error, buffer.str());
		}

		// The trace timestamps start at zero. Add the offset back to read value.
		entry->time += start;
		// Keep track of the highest (most recent) timestamp read, used in readUntil
		mostRecentEntry = entry->time;

		Log::Write(Log::Level::Info, "C");

		return true;
	}

	bool readUntil(traceEntry* entry)
	{
		auto until = entry->time;
		stringstream buffer;
		buffer << "RNR " << mostRecentEntry << " " << time;
		Log::Write(Log::Level::Info, buffer.str());
		bool res;
		for (res = true; res && mostRecentEntry <= until; res = read(entry))
		{
			// Nothing to do here
		}
		return res;
	}

	bool readNextSpace(traceEntry* entry)
	{
		assert(holds_alternative<traceLocation>(entry->body));
		auto& l = get<traceLocation>(entry->body);

		traceEntry outEntry;
		outEntry.time = entry->time;

		// Go through the trace until we get to the time we're looking for
		if (!readUntil(&outEntry))
		{
			return false;
		}

		// Overwrite the output variable
		*entry = spaceMap[entry->path][l.basespace];
		return true;
	}

	bool readNextView(traceEntry* entry)
	{
		assert(holds_alternative<traceView>(entry->body));
		auto& v = get<traceView>(entry->body);

		traceEntry outEntry;
		outEntry.time = entry->time;

		// Go through the trace until we get to the time we're looking for
		if (!readUntil(&outEntry))
		{
			return false;
		}

		// Overwrite the output variable
		*entry = viewMap[entry->path][v.index];

		return true;
	}

	void writeActionFloat(traceEntry e)
	{
		assert(holds_alternative<traceActionFloat>(e.body));
		auto& f = get<traceActionFloat>(e.body);

		writeHead(e);
		trace << " " << f.changed << " " << f.isActive << " " << f.lastChanged << " " << f.value;
		trace << endl;
	}

	bool readNextActionFloat(traceEntry* e)
	{
		assert(holds_alternative<traceActionFloat>(e->body));
		auto& f = get<traceActionFloat>(e->body);

		traceEntry outEntry;
		outEntry.time = e->time;
		if (!readUntil(&outEntry) || floatActionMap.find(e->path) == floatActionMap.end())
		{
			return false;
		}

		*e = floatActionMap[e->path];
		return true;
	}

	void writeActionBoolean(traceEntry e)
	{
		assert(holds_alternative<traceActionBoolean>(e.body));
		auto& b = get<traceActionBoolean>(e.body);

		writeHead(e);
		trace << " " << b.changed << " " << b.isActive << " " << b.lastChanged << " " << b.value;
		trace << endl;
	}

	bool readNextActionBoolean(traceEntry* e)
	{
		assert(holds_alternative<traceActionBoolean>(e->body));
		auto& f = get<traceActionBoolean>(e->body);

		traceEntry outEntry;
		outEntry.time = e->time;
		if (!readUntil(&outEntry) || booleanActionMap.find(e->path) == booleanActionMap.end())
		{
			return false;
		}

		*e = booleanActionMap[e->path];
		return true;
	}
}
