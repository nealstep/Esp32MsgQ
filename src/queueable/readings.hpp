#pragma once

#include "global.hpp"
#include "queue_t.hpp"

// types for clarity
typedef uint8_t modules_t;
typedef uint8_t controls_t;

#define UNIT_LIST(X) X(None, "None")

class Readings {
   public:
    enum class Payload : uint8_t { U8, FLOAT };
#define GENERATE_ENUM(id, msg) id,
    enum class Unit : uint8_t { UNIT_LIST(GENERATE_ENUM) Count };
#undef GENERATE_ENUM

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
        Unit unit;
    } Entry;

    QueueT<Entry> queue;

    // lazy singleton
    static Readings& getInstance(void) {
        static Readings instance;
        return instance;
    }
    Readings(const Readings&) = delete;
    Readings& operator=(const Readings&) = delete;

    static constexpr const char* get_unit(Unit code) {
        return _units[static_cast<uint16_t>(code)];
    }

   protected:
    Readings() {}

#define GENERATE_STRING(id, msg) msg,
    static constexpr const char* const _units[] = {UNIT_LIST(GENERATE_STRING)};
#undef GENERATE_STRING
#undef UNIT_LIST
};

static Readings& readings = Readings::getInstance();
