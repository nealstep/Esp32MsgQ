#pragma once

#include "global.hpp"
#include "messages.hpp"
#include "queue_t.hpp"

#ifndef ARDUINO
#include <cstring>
#endif  // !ARDUINO

typedef uint8_t sid_t;

class Datum {
   public:
    enum class Payload : uint8_t {
        DOUBLE,
        FLOAT,
        I8,
        I16,
        I32,
        I64,
        SID,
        U8,
        U16,
        U32,
        U64
    };
    static constexpr const sid_t str_max = 12;
    static constexpr const sid_t str_size = 24;
    static constexpr const sid_t str_exhausted = 0xFF;

    typedef struct {
        Payload payload;
        Messages::Var vid;
        time_t asof;
        union {
            double d;
            float f;
            int8_t i8;
            int16_t i16;
            int32_t i32;
            int64_t i64;
            sid_t sid;
            uint8_t u8;
            uint16_t u16;
            uint32_t u32;
            uint64_t u64;
        } value;
        bool broadcast;
    } Entry;

    QueueT<Entry> queue;

    // lazy singleton
    static Datum& getInstance(void) {
        static Datum instance;
        return instance;
    }
    Datum(const Datum&) = delete;
    Datum& operator=(const Datum&) = delete;

    void add(Messages::Var var, int8_t val, bool broadcast) {
        Entry entry;
        entry.value.i8 = val;
        set_basic(entry, var, Payload::I8, broadcast);
    }

    void add(Messages::Var var, int16_t val, bool broadcast) {
        Entry entry;
        entry.value.i16 = val;
        set_basic(entry, var, Payload::I16, broadcast);
    }

    void add(Messages::Var var, int32_t val, bool broadcast) {
        Entry entry;
        entry.value.i32 = val;
        set_basic(entry, var, Payload::I32, broadcast);
    }

    void add(Messages::Var var, int64_t val, bool broadcast) {
        Entry entry;
        entry.value.i64 = val;
        set_basic(entry, var, Payload::I64, broadcast);
    }

    void add(Messages::Var var, float val, bool broadcast) {
        Entry entry;
        entry.value.f = val;
        set_basic(entry, var, Payload::FLOAT, broadcast);
    }

    void add(Messages::Var var, double val, bool broadcast) {
        Entry entry;
        entry.value.d = val;
        set_basic(entry, var, Payload::DOUBLE, broadcast);
    }

    void add(Messages::Var var, uint8_t val, bool broadcast) {
        Entry entry;
        entry.value.u8 = val;
        set_basic(entry, var, Payload::U8, broadcast);
    }

    void add(Messages::Var var, uint16_t val, bool broadcast) {
        Entry entry;
        entry.value.u16 = val;
        set_basic(entry, var, Payload::U16, broadcast);
    }

    void add(Messages::Var var, uint32_t val, bool broadcast) {
        Entry entry;
        entry.value.u32 = val;
        set_basic(entry, var, Payload::U32, broadcast);
    }

    void add(Messages::Var var, uint64_t val, bool broadcast) {
        Entry entry;
        entry.value.u64 = val;
        set_basic(entry, var, Payload::U64, broadcast);
    }

    void add(Messages::Var var, const char* val, bool broadcast) {
        Entry entry;
        sid_t sid = get_next_str();
        if (sid == str_exhausted) {
            // TODO: #18 fix data add string failing silently somehow
#ifdef ARDUINO
#ifdef SER
            DEBUG("Ran out of data strings");
#endif  // SER
#else   // !ARDUINO
            DEBUG("Ran out of data strings");
#endif  // ARDUINO !ARDUINO

            return;
        }
        entry.value.sid = sid;
        char* sptr = get_str(sid);
        int len = strlcpy(sptr, val, str_size);
        if (len >= str_size) {
// TODO: #19 handle truncated data queue
#ifdef ARDUINO
#ifdef SER
            DEBUG("Truncated data string");
#endif  // SER
#else   // !ARDUINO
            DEBUG("Truncated data string");
#endif  // ARDUINO !ARDUINO
        }
        set_basic(entry, var, Payload::SID, broadcast);
    }

    sid_t get_next_str() {
        sid_t ind = 0;
        while (ind < str_max) {
            if (_str_free[ind]) {
                _str_free[ind] = false;
                return ind;
            }
            ind++;
        }
        return str_exhausted;
    }

    char* get_str(sid_t ind) {
        if (_str_free[ind]) {
            return nullptr;
        } else if (ind >= str_max) {
            // invalid
        }
        return _str[ind];
    }

    void free_str(sid_t ind) { _str_free[ind] = true; }

   protected:
    char _str[str_max][str_size];
    bool _str_free[str_max];
    uint8_t _str_ind = 0;

    Datum() {
        for (uint8_t ind = 0; ind < str_max; ind++) _str_free[ind] = true;
    }

    void set_basic(Entry& entry, Messages::Var var, Payload pay,
                   bool broadcast) {
        entry.payload = pay;
        entry.asof = time(NULL);
        entry.vid = var;
        entry.broadcast = broadcast;
        queue.push(entry);
    }
};

static Datum& datum = Datum::getInstance();

#define DATAQ(N, V, B) datum.add(N, V, B)
