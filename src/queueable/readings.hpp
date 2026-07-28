#pragma once

#include "global.hpp"
#include "queue_t.hpp"

// types for clarity
typedef uint8_t modules_t;
typedef uint8_t controls_t;

#define UNIT_LIST(X) \
    X(None, "None")  \
    X(Integer, "i")  \
    X(Hex, "h")      \
    X(Str, "s")      \
    X(Date, "d")     \
    X(Mhz, "Mhz")    \
    X(K, "K")        \
    X(TimeT, "t")

class Readings {
   public:
    enum class Payload : uint8_t { DOUBLE, FLOAT, I8, I16, I32, I64, U8, U16, U32, U64 };
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
            double d;
            float f;
            int8_t i8;
            int16_t i16;
            int32_t i32;
            int64_t i64;
            uint8_t u8;
            uint16_t u16;
            uint32_t u32;
            uint64_t u64;
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
