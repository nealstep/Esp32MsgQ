#pragma once

#include "global.hpp"
#include "queue_t.hpp"

// types for clarity
typedef uint8_t modules_t;
typedef uint8_t controls_t;

class Readings {
   public:
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
    } Entry;

    QueueT<Entry> queue;

    // lazy singleton
    static Readings& getInstance(void) {
        static Readings instance;
        return instance;
    }
    Readings(const Readings&) = delete;
    Readings& operator=(const Readings&) = delete;

   protected:
    Readings() {}
};

static Readings& readings = Readings::getInstance();
