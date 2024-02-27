
#pragma once

#include <string>
#include <openxr/openxr.h>
#include <variant>

using namespace std;

namespace tracer {

    enum Mode {
        RECORD,
        REPLAY
    };

    struct traceLocation {
        XrPosef pose;
        string basespace;
    };

    struct traceCreateReferenceSpace {
        XrPosef pose;
        uint32_t type;
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

    struct traceActionVector2f {
        XrBool32 changed;
        XrVector2f value;
        XrBool32 isActive;
        XrTime lastChanged;
    };

    struct traceActionBoolean {
        XrBool32 changed;
        XrBool32 value;
        XrBool32 isActive;
        XrTime lastChanged;
    };

    struct traceApplyHaptic {
        XrBool32 value;
        XrTime lastChanged;
    };
    using traceBody = variant<traceLocation, traceCreateReferenceSpace, traceView, traceActionBoolean, traceActionFloat, traceActionVector2f, traceApplyHaptic>;

    struct traceEntry {
        XrTime time;
        char type;
        string path;
        traceBody body;
    };

    Mode init();

    void close();

    void writeView(traceEntry);

    bool readNextView(XrTime until, traceEntry *);

    void writeSpace(traceEntry);

    bool readNextSpace(XrTime until, traceEntry *);

    void writeCreateReferenceSpace(traceEntry);

    bool readNextCreateReferenceSpace(XrTime until, traceEntry *);

    void writeActionFloat(traceEntry);

    bool readNextActionFloat(XrTime until, traceEntry *);

    void writeActionBoolean(traceEntry);

    bool readNextActionBoolean(XrTime until, traceEntry *);

    void writeActionVector2f(traceEntry);

    bool readNextActionVector2f(XrTime until, traceEntry *);

    void writeApplyHaptic(traceEntry);

    bool readNextApplyHaptic(XrTime until, traceEntry *);
}
