
#include "tracer.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <map>

#include "logger.h"

namespace tracer {
	fstream trace;

	XrTime mostRecentEntry;
	map<string, map<string, traceEntry>> spaceMap;
	map<string, map<uint32_t, traceEntry>> viewMap;
	stringstream buffer;

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
		Log::Write(Log::Level::Info, "RNR file is open?");
		Log::Write(Log::Level::Info, trace.is_open() ? "YES" : "NO");
	}

	void close()
	{
		trace.close();
	}

	void write(traceEntry entry)
	{
		// The first time we are asked to write an entry, set the requested timestamp as zero.
		static XrTime start = entry.time;

		// Reset the string buffer
		buffer.str(string());
		buffer << entry.time - start << " " << entry.type << " " << entry.space << " ";
		buffer << entry.ow << " " << entry.ox << " " << entry.oy << " " << entry.oz << " ";
		buffer << entry.px << " " << entry.py << " " << entry.pz << " ";
	}

	void writeSpace(traceEntry entry)
	{
		write(entry);
		buffer << entry.basespace;
		trace << buffer.str() << endl;
	}

	void writeView(traceEntry entry)
	{
		write(entry);
		buffer << entry.u << " " << entry.r << " " << entry.d << " " << entry.l << " " << entry.viewType << " " << entry.index;
		trace << buffer.str() << endl;
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
		if (!(sstream >> entry->time >> entry->type >> entry->space >> entry->ow >> entry->ox >> entry->oy >> entry->oz >> entry->px >> entry->py >> entry->pz))
		{
			Log::Write(Log::Level::Warning, "RNR read end of file!");
			return false;
		}

		Log::Write(Log::Level::Info, "B");

		// Depending on the trace record type, we need to read different fields
		switch (entry->type) {
		case 's':
			sstream >> entry->basespace;
			spaceMap[entry->space][entry->basespace] = *entry;
			break;
		case 'v':
			sstream >> entry->u >> entry->r >> entry->d >> entry->l >> entry->viewType >> entry->index;
			viewMap[entry->space][entry->index] = *entry;
			break;
		default:
			stringstream buffer;
			buffer << "RNR ERROR invalid trace entry type: " << entry->type;
			Log::Write(Log::Level::Error, buffer.str());
			break;
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
		for (res = true; res && mostRecentEntry < until; res = read(entry))
		{
			// Nothing to do here
		}
		return res;
	}



	bool readNextSpace(traceEntry* entry)
	{
		traceEntry outEntry;
		outEntry.time = entry->time;

		// Go through the trace until we get to the time we're looking for
		if (!readUntil(&outEntry))
		{
			return false;
		}

		//// This is a bit of a hack to bootstrap the process
		//// If we read up to the indicated time, but haven't yet seen an entry that matches the space/basespace/index we're looking for, we keep reading until we do
		//for (auto findEntry = spaceMap.find(entry->space); findEntry == spaceMap.end(); )
		//{
		//	read(&outEntry);
		//	Log::Write(Log::Level::Info, "INF LOOP A?");
		//}
		//auto map = spaceMap[entry->space];
		//for (auto findEntry = map.find(entry->basespace); findEntry == map.end(); )
		//{
		//	read(&outEntry);
		//	Log::Write(Log::Level::Info, "INF LOOP B?");
		//}

		// Overwrite the output variable
		*entry = spaceMap[entry->space][entry->basespace];
		return true;
	}

	bool readNextView(traceEntry* entry)
	{
		traceEntry outEntry;
		outEntry.time = entry->time;

		// Go through the trace until we get to the time we're looking for
		if (!readUntil(&outEntry))
		{
			return false;
		}

		// This is a bit of a hack to bootstrap the process
		// If we read up to the indicated time, but haven't yet seen an entry that matches the space/basespace/index we're looking for, we keep reading until we do
		//for (auto findEntry = viewMap.find(entry->space); findEntry == viewMap.end(); )
		//{
		//	read(&outEntry);
		//}
		//auto map = viewMap[entry->space];
		//for (auto findEntry = map.find(entry->index); findEntry == map.end(); )
		//{
		//	read(&outEntry);
		//}

		// Overwrite the output variable
		*entry = viewMap[entry->space][entry->index];

		return true;
	}
}
