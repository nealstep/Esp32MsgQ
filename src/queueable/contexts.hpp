#pragma once

#include "global.hpp"

#ifndef ARDUINO
#include <cstring>
#endif  // !ARDUINO

typedef uint8_t cid_t;

class Contexts {
   public:
    static constexpr const cid_t context_max = 12;
    static constexpr const cid_t context_exhausted = 0xFF;

    class Context {
       public:
        static constexpr const uint8_t name_size = 32;
        static constexpr const uint8_t filename_size = 32;

        static constexpr const char* const context_fmt = "%s|%s:%d";
        static constexpr const char* const context_empty = "-|-:-";

        // Error is local to avoid dependency on error.hpp
        enum class Err : uint8_t { NoErr, Format, Trunc };

        Context() {
            _name[0] = '\0';
            _fname[0] = '\0';
            _line = 0;
        }

        bool set(const char* name = nullptr, const char* fname = nullptr,
                 int line = 0) {
            bool err = false;
            _line = line;
            size_t len;
            if (name) {
                len = strlcpy(_name, name, sizeof(_name));
                if (len >= sizeof(_name)) err = true;
            }
            if (fname) {
                len = strlcpy(_fname, fname, sizeof(_fname));
                if (len >= sizeof(_fname)) err = true;
            }
            return err;
        }

        Err get_str(char* buf, size_t buf_len) {
            size_t len =
                snprintf(buf, buf_len, context_fmt, _name, _fname, _line);
            if (len < 0)
                return Err::Format;
            else if (len >= buf_len)
                return Err::Trunc;
            else
                return Err::NoErr;
        }

       protected:
        char _name[name_size];
        char _fname[filename_size];
        int _line;
    };

    // lazy singleton
    static Contexts& getInstance(void) {
        static Contexts instance;
        return instance;
    }
    Contexts(const Contexts&) = delete;
    Contexts& operator=(const Contexts&) = delete;

    cid_t get_next_context() {
        cid_t ind = 0;
        while (ind < context_max) {
            if (_context_free[ind]) {
                _context_free[ind] = false;
                return ind;
            }
            ind++;
        }
        return context_exhausted;
    }

    Context* get_context(cid_t ind) {
        if (_context_free[ind]) {
            return nullptr;
        } else if (ind >= context_max) {
            // invalid
            return nullptr;
        }
        return &_context[ind];
    }

    void free_context(cid_t ind) { _context_free[ind] = true; }

   protected:
    Context _context[context_max];
    bool _context_free[context_max];
    uint8_t _context_ind = 0;

    Contexts() {
        for (uint8_t ind = 0; ind < context_max; ind++)
            _context_free[ind] = true;
    }
};

static Contexts& contexts = Contexts::getInstance();