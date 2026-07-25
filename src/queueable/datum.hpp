#pragma once

#include "global.hpp"
#include "messages.hpp"
#include "queue_t.hpp"

typedef uint8_t sid_t;

class Datum {
   public:
    enum class Payload : uint8_t { U8, FLOAT, SID };
    static constexpr const sid_t str_max = 12;
    static constexpr const sid_t str_size = 12;
    static constexpr const sid_t str_exhausted = 0xFF;

    typedef struct {
        Payload payload;
        Messages::Var vid;
        union {
            uint8_t u8;
            float f;
            sid_t sid;
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

    void add(Messages::Var var, uint8_t val, bool broadcast) {}
    void add(Messages::Var var, const char* val, bool broadcast) {}

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

    void free_context(sid_t ind) { _str_free[ind] = true; }

   protected:
    char _str[str_max][str_size];
    bool _str_free[str_max];
    uint8_t _str_ind = 0;

    Datum() {
        for (uint8_t ind = 0; ind < str_max; ind++) _str_free[ind] = true;
    }
};

static Datum& datum = Datum::getInstance();

#define DATAQ(N, V, B) data.add(N, V, B)
