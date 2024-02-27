
#include "tracer.hpp"

#include <filesystem>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <cassert>
#include <stdlib.h>

#include "logger.h"

namespace fs = std::filesystem;

namespace tracer {
    fstream trace;

    XrTime mostRecentEntry;
    map<string, map<string, traceEntry>> spaceMap;
    map<string, map<uint32_t, traceEntry>> viewMap;
    map<string, traceEntry> floatActionMap;
    map<string, traceEntry> booleanActionMap;
    map<string, traceEntry> hapticActionMap;
    map<string, traceEntry> vector2fActionMap;
    map<char32_t, traceEntry> refSpaceMap;

    Mode init() {
        auto config_str = std::getenv("LOCALAPPDATA");
        assert(config_str != nullptr);
        auto config = fs::path(config_str);
        config = config / "librnr" / "config.txt";

        auto c = fstream(config);
        string mode_str;
        fs::path trace_file;
        c >> mode_str >> trace_file;

        ios_base::openmode filemode;
        Mode res;
        if (mode_str == "replay") {
            filemode = fstream::in;
            res = REPLAY;
        } else {
            filemode = fstream::out | fstream::trunc;
            res = RECORD;
        }

        if (res == RECORD) {
            auto trace_dir = trace_file;
            trace_dir.remove_filename();
            create_directories(trace_dir);
        }

        trace.open(trace_file, filemode);

        stringstream buffer;
        buffer << "RNR Mode=" << ((res == REPLAY) ? "REPLAY" : "RECORD") << " File=" << trace_file;
        Log::Write(Log::Level::Info, buffer.str());

        return res;
    }

    void close() {
        trace.close();
    }

    void writeHead(traceEntry entry) {
        // The first time we are asked to write an entry, set the requested timestamp as zero.
        static XrTime start = entry.time;
        trace << entry.time - start << " " << entry.type << " " << entry.path << " ";
    }

    void writeSpace(traceEntry entry) {
        assert(holds_alternative<traceLocation>(entry.body));

        writeHead(entry);
        auto &l = get<traceLocation>(entry.body);
        auto &pose = l.pose;
        auto &o = pose.orientation;
        auto &p = pose.position;
        auto &basespace = l.basespace;
        trace << " " << o.x << " " << o.y << " " << o.z << " " << o.w;
        trace << " " << p.x << " " << p.y << " " << p.z;
        trace << " " << basespace;
        trace << endl;
    }

    void writeCreateReferenceSpace(traceEntry entry) {
        assert(holds_alternative<traceCreateReferenceSpace>(entry.body));

        writeHead(entry);
        auto &r = get<traceCreateReferenceSpace>(entry.body);
        auto &pose = r.pose;
        auto &o = pose.orientation;
        auto &p = pose.position;
        trace << " " << o.x << " " << o.y << " " << o.z << " " << o.w;
        trace << " " << p.x << " " << p.y << " " << p.z;
        trace << " " << r.type;
        trace << endl;
    }

    void writeView(traceEntry entry) {
        assert(holds_alternative<traceView>(entry.body));

        writeHead(entry);
        auto &w = get<traceView>(entry.body);
        auto &pose = w.pose;
        auto &o = pose.orientation;
        auto &p = pose.position;
        auto &fov = w.fov;
        auto &u = fov.angleUp;
        auto &r = fov.angleRight;
        auto &d = fov.angleDown;
        auto &l = fov.angleLeft;
        auto &type = w.type;
        auto &index = w.index;
        trace << " " << o.x << " " << o.y << " " << o.z << " " << o.w;
        trace << " " << p.x << " " << p.y << " " << p.z;
        trace << " " << u << " " << r << " " << d << " " << l << " " << type << " " << index;
        trace << endl;
    }

    bool read(string &line, XrTime time) {
        stringstream sstream(line);
        traceEntry entry;
        if (!(sstream >> entry.time >> entry.type >> entry.path)) {
            Log::Write(Log::Level::Warning, "RNR read end of file!");
            return false;
        }

        // Overwrite time to match playback time
        entry.time = time;
        // Keep track of the highest (most recent) timestamp read, used in readUntil
        mostRecentEntry = entry.time;

        // Depending on the trace record type, we need to read different fields
        if (entry.type == 's') {
            traceLocation l;
            auto &o = l.pose.orientation;
            auto &p = l.pose.position;
            sstream >> o.x >> o.y >> o.z >> o.w >> p.x >> p.y >> p.z;
            sstream >> l.basespace;
            entry.body = l;
            spaceMap[entry.path][l.basespace] = entry;
        } else if (entry.type == 'r') {
            traceCreateReferenceSpace r{};
            auto &o = r.pose.orientation;
            auto &p = r.pose.position;
            sstream >> o.x >> o.y >> o.z >> o.w >> p.x >> p.y >> p.z;
            sstream >> r.type;
            entry.body = r;
            refSpaceMap[entry.type] = entry;
        } else if (entry.type == 'v') {
            traceView v;
            auto &o = v.pose.orientation;
            auto &p = v.pose.position;
            auto &u = v.fov.angleUp;
            auto &r = v.fov.angleRight;
            auto &d = v.fov.angleDown;
            auto &l = v.fov.angleLeft;
            auto &type = v.type;
            auto &index = v.index;
            sstream >> o.x >> o.y >> o.z >> o.w >> p.x >> p.y >> p.z;
            sstream >> u >> r >> d >> l >> type >> index;
            entry.body = v;
            viewMap[entry.path][index] = entry;
        } else if (entry.type == 'b') {
            traceActionBoolean b{};
            auto &changed = b.changed;
            auto &isActive = b.isActive;
            auto &lastChanged = b.lastChanged;
            auto &value = b.value;
            sstream >> changed >> isActive >> lastChanged >> value;
            entry.body = b;
            booleanActionMap[entry.path] = entry;
        } else if (entry.type == 'f') {
            traceActionFloat f{};
            auto &changed = f.changed;
            auto &isActive = f.isActive;
            auto &lastChanged = f.lastChanged;
            auto &value = f.value;
            sstream >> changed >> isActive >> lastChanged >> value;
            entry.body = f;
            floatActionMap[entry.path] = entry;
        } else if (entry.type == 'p') {
            traceActionVector2f p{};
            auto &changed = p.changed;
            auto &isActive = p.isActive;
            auto &lastChanged = p.lastChanged;
            auto &value = p.value;

            sstream >> changed >> isActive >> lastChanged >> value.x >> value.y;
            entry.body = p;
            vector2fActionMap[entry.path] = entry;
        } else {
            stringstream buffer;
            buffer << "RNR ERROR invalid trace entry type: " << entry.type;
            Log::Write(Log::Level::Error, buffer.str());
        }

        return true;
    }

    bool readUntil(XrTime until) {
        // First time this function is called,
        // assume that 'until' is the time of the first frame in the playback
        static XrTime startPlayback = until;
        // Static variables to record the first timestamp in the trace
        static XrTime startRecord = 0;
        static bool firstLine = true;

        while (true) {
            // Get current position in trace input stream
            auto pos = trace.tellg();
            // Read a line from the trace
            string line;
            // Return false if there are no more lines
            if (!getline(trace, line)) {
                return false;
            }
            // Create string stream from line read
            stringstream sstream(line);
            // Read timestamp from line read
            XrTime timestamp;
            sstream >> timestamp;
            if (firstLine) {
                startRecord = timestamp;
                firstLine = false;
            }
            auto timestampReplay = timestamp - startRecord + startPlayback;
            // If timestamp smaller than read limit, perform read
            if (timestampReplay <= until) {
                // Parse line into entry
                read(line, timestampReplay);
            } else {
                // Next record is after 'until',
                // set stream position to position before reading line
                trace.seekg(pos);
                // Read until 'until', returning true
                return true;
            }
        }
    }

    bool readNextSpace(XrTime until, traceEntry *entry) {
        assert(holds_alternative<traceLocation>(entry->body));
        auto &l = get<traceLocation>(entry->body);

        // Go through the trace until we get to the time we're looking for
        if (!readUntil(until)) {
            return false;
        }

        // Overwrite the output variable
        *entry = spaceMap[entry->path][l.basespace];
        return true;
    }

    bool readNextCreateReferenceSpace(XrTime until, traceEntry *entry) {
        assert(holds_alternative<traceCreateReferenceSpace>(entry->body));
        auto &r = get<traceCreateReferenceSpace>(entry->body);

        // Go through the trace until we get to the time we're looking for
        if (!readUntil(until)) {
            return false;
        }

        // Overwrite the output variable
        *entry = refSpaceMap[entry->type];
        return true;
    }

    bool readNextView(XrTime until, traceEntry *entry) {
        assert(holds_alternative<traceView>(entry->body));
        auto &v = get<traceView>(entry->body);

        // Go through the trace until we get to the time we're looking for
        if (!readUntil(until)) {
            return false;
        }

        // Overwrite the output variable
        *entry = viewMap[entry->path][v.index];

        return true;
    }

    void writeActionFloat(traceEntry e) {
        assert(holds_alternative<traceActionFloat>(e.body));
        auto &f = get<traceActionFloat>(e.body);

        writeHead(e);
        trace << " " << f.changed << " " << f.isActive << " " << f.lastChanged << " " << f.value;
        trace << endl;
    }

    bool readNextActionFloat(XrTime until, traceEntry *e) {
        assert(holds_alternative<traceActionFloat>(e->body));
        auto &f = get<traceActionFloat>(e->body);

        if (!readUntil(until) || floatActionMap.find(e->path) == floatActionMap.end()) {
            return false;
        }

        *e = floatActionMap[e->path];
        return true;
    }

    void writeActionVector2f(traceEntry e) {
        assert(holds_alternative<traceActionVector2f>(e.body));
        auto &f = get<traceActionVector2f>(e.body);

        writeHead(e);
        trace << " " << f.changed << " " << f.isActive << " " << f.lastChanged << " " << f.value.x << " " << f.value.y;
        trace << endl;
    }

    bool readNextActionVector2f(XrTime until, traceEntry *e) {
        assert(holds_alternative<traceActionVector2f>(e->body));
        auto &f = get<traceActionVector2f>(e->body);

        if (!readUntil(until) || vector2fActionMap.find(e->path) == vector2fActionMap.end()) {
            return false;
        }

        *e = vector2fActionMap[e->path];
        return true;
    }

    void writeActionBoolean(traceEntry e) {
        assert(holds_alternative<traceActionBoolean>(e.body));
        auto &b = get<traceActionBoolean>(e.body);

        writeHead(e);
        trace << " " << b.changed << " " << b.isActive << " " << b.lastChanged << " " << b.value;
        trace << endl;
    }

    bool readNextActionBoolean(XrTime until, traceEntry *e) {
        assert(holds_alternative<traceActionBoolean>(e->body));
        auto &f = get<traceActionBoolean>(e->body);

        if (!readUntil(until) || booleanActionMap.find(e->path) == booleanActionMap.end()) {
            return false;
        }

        *e = booleanActionMap[e->path];
        return true;
    }

    void writeApplyHaptic(traceEntry e) {
        assert(holds_alternative<traceApplyHaptic>(e.body));
        auto &h = get<traceApplyHaptic>(e.body);

        writeHead(e);
        trace << " " << h.value;
        trace << endl;
    }

    bool readNextApplyHaptic(XrTime until, traceEntry *e) {
        assert(holds_alternative<traceApplyHaptic>(e->body));
        auto &h = get<traceApplyHaptic>(e->body);

        if (!readUntil(until) || hapticActionMap.find(e->path) == hapticActionMap.end()) {
            return false;
        }

        *e = hapticActionMap[e->path];
        return true;
    }
}
