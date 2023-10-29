
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

	void init()
	{
		trace.open("trace.txt");
	}

	void close()
	{
		trace.close();
	}

	void write(traceEntry entry)
	{
		// Reset the string buffer
		buffer.str(string());
		buffer << entry.time << " " << entry.type << " " << entry.space << " ";
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
		buffer << entry.u << " " << entry.r << " " << entry.d << " " << entry.l << " " << entry.index;
		trace << buffer.str() << endl;
	}

	void read(traceEntry* entry)
	{
		trace >> entry->time >> entry->type >> entry->space >> entry->ow >> entry->ox >> entry->oy >> entry->oz >> entry->px >> entry->py >> entry->pz;
		static XrTime start = entry->time;

		mostRecentEntry = entry->time - start;
		switch (entry->type) {
		case 's':
			trace >> entry->basespace;
			spaceMap[entry->space][entry->basespace] = *entry;
			break;
		case 'v':
			trace >> entry->u >> entry->r >> entry->d >> entry->l >> entry->index;
			viewMap[entry->space][entry->index] = *entry;
			break;
		default:
			stringstream buffer;
			buffer << "RNR ERROR invalid trace entry type: " << entry->type;
			Log::Write(Log::Level::Error, buffer.str());
			break;
		}
	}

	void readUntil(XrTime time, traceEntry* entry)
	{
		// FIXME should make trace time relative
		while (mostRecentEntry < time) {
			read(entry);
		}
	}



	void readNextSpace(XrTime time, traceEntry* entry)
	{
		traceEntry outEntry;

		// Go through the trace until we get to the time we're looking for
		readUntil(time, &outEntry);

		// This is a bit of a hack to bootstrap the process
		// If we read up to the indicated time, but haven't yet seen an entry that matches the space/basespace/index we're looking for, we keep reading until we do
		for (auto findEntry = spaceMap.find(entry->space); findEntry == spaceMap.end(); )
		{
			read(&outEntry);
		}
		auto map = spaceMap[entry->space];
		for (auto findEntry = map.find(entry->basespace); findEntry == map.end(); )
		{
			read(&outEntry);
		}

		// Overwrite the output variable
		*entry = spaceMap[entry->space][entry->basespace];
	}

	void readNextView(XrTime time, traceEntry* entry)
	{
		traceEntry outEntry;

		// Go through the trace until we get to the time we're looking for
		readUntil(time, &outEntry);

		// This is a bit of a hack to bootstrap the process
		// If we read up to the indicated time, but haven't yet seen an entry that matches the space/basespace/index we're looking for, we keep reading until we do
		for (auto findEntry = viewMap.find(entry->space); findEntry == viewMap.end(); )
		{
			read(&outEntry);
		}
		auto map = viewMap[entry->space];
		for (auto findEntry = map.find(entry->index); findEntry == map.end(); )
		{
			read(&outEntry);
		}

		// Overwrite the output variable
		*entry = viewMap[entry->space][entry->index];
	}
}
