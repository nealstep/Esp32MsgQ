#pragma once

#include "global.hpp"
#include "output/output.hpp"
#include "queueable/error.hpp"
#include "queueable/readings.hpp"

#ifndef ARDUINO
#include <cstring>
#endif  // !ARDUINO

// constant to generate
#define TYPE_LIST(X) \
    X(Sensor, "S")   \
    X(Control, "C")  \
    X(Other, "O")

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

    // lazy singleton
    static Modules& getInstance(void) {
        static Modules instance;
        return instance;
    }
    Modules(const Modules&) = delete;
    Modules& operator=(const Modules&) = delete;

    // functions
    modules_t add(Module* mod_p, bool nmea = false) {
        modules_t mid = _modules_ind;
        if (_modules_ind < modules_max) {
            _modules[_modules_ind++] = mod_p;
        } else {
            LOG_ED(Error::Err::ModMax, "add");
            die();
        }
#ifdef NMEA0183
        if (nmea) _nmea[_nmea_ind++] = mid;
#endif  // NMEA_0183
        return mid;
    }

    Module* get_module(modules_t mid) {
        if (mid < _modules_ind)
            return _modules[mid];
        else
            return nullptr;
    }
    static constexpr const char* get_tname(Typ code) {
        return _types[static_cast<uint8_t>(code)];
    }

#ifdef NMEA0183
    void check_nmea(Error::Err (*sendNMEA)(const char *));
#endif  // NMEA0183

   protected:
    // generated constants
#define GENERATE_STRING(id, msg) msg,
    static constexpr const char* const _types[] = {TYPE_LIST(GENERATE_STRING)};
#undef GENERATE_STRING
#undef TYPE_LIST

    // variables
    Module* _modules[modules_max];
    modules_t _modules_ind = 0;
#ifdef NMEA0183
    modules_t _nmea[modules_max];
    modules_t _nmea_ind = 0;
#endif  // NMEA0183

    // hidden creator
    Modules(void) {}
};

static Modules& modules = Modules::getInstance();
