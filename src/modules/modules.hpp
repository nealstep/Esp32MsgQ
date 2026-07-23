#pragma once

#include "global.hpp"

#ifndef ARDUINO
#include <cstring>
#endif  // !ARDUINO

// constant to generate
#define TYPE_LIST(X) \
    X(Sensor, "S")   \
    X(Control, "C")  \
    X(Other, "O")

// types for clarity
typedef uint8_t modules_t;
typedef uint8_t controls_t;

class Module;

class Modules {
   public:
    // generated constants
#define GENERATE_ENUM(id, msg) id,
    enum class Typ : uint8_t { TYPE_LIST(GENERATE_ENUM) Count };
#undef GENERATE_ENUM

    // constants
    static constexpr const modules_t modules_max = 16;
    static constexpr const controls_t controls_max = 16;
    static constexpr const uint8_t name_size = 32;
    static constexpr const uint8_t reading_name_size = 8;
    static constexpr const char* const name_fmt = "%s.%s.%u.%u";
    enum class Payload : uint8_t { U8, FLOAT };

    // reading sturcture
    typedef struct {
        modules_t mid;
        controls_t cid;
        time_t asof;
        Payload payload;
        union {
            uint8_t u8;
            float f;
        } value;
    } Reading;

    // lazy singleton
    static Modules& getInstance(void) {
        static Modules instance;
        return instance;
    }
    Modules(const Modules&) = delete;
    Modules& operator=(const Modules&) = delete;

    // functions
    modules_t add(Module* mod_p) {
        modules_t mid = _modules_ind;
        if (_modules_ind < modules_max)
            _modules[_modules_ind++] = mod_p;
        else
            die();
        return mid;
    }
    Module* get_module(modules_t mid) {
        if (mid < _modules_ind)
            return _modules[mid];
        else return nullptr;
    }
    static constexpr const char* get_tname(Typ code) {
        return _Types[static_cast<uint8_t>(code)];
    }

   protected:
   // generated constants
#define GENERATE_STRING(id, msg) msg,
    static constexpr const char* const _Types[] = {TYPE_LIST(GENERATE_STRING)};
#undef GENERATE_STRING
#undef TYPE_LIST

    // variables
    Module* _modules[modules_max];
    modules_t _modules_ind;

    // hidden creator
    Modules(void) : _modules_ind(0) {}
};

static Modules& modules = Modules::getInstance();
