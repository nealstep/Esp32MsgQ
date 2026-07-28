#pragma once

#ifndef ARDUINO

#include "global.hpp"

class Preferences {
   public:
    bool begin(const char* name, bool read_only) { return false; }
    bool isKey(const char* key) { return false; }
    bool getBool(const char* key) { return false; }
    bool getUShort(const char* key) { return 0; }
    bool getUInt(const char* key) { return 0; }
    size_t getString(const char* key, char* value, size_t maxLen) { return 0; };
    size_t putBool(const char* key, bool val) { return sizeof(bool); }
    size_t putUShort(const char* key, uint16_t val) { return sizeof(uint16_t); }
    size_t putUInt(const char* key, uint32_t val) { return sizeof(uint32_t); }
    size_t putString(const char* key, const char* value) { return 0; }
    void end() {}
};

#endif  // !ARDUINO
