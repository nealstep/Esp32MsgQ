#pragma once

#include "global.hpp"

#ifndef ARDUINO

void delay(uint32_t ms);

class Preferences {
   public:
    bool begin(const char* name, bool read_only) { return false; }
    bool isKey(const char* key) { return false; }
    bool getBool(const char* key) { return false; }
    bool getUInt(const char* key) { return 0; }
    size_t getString(const char* key, char* value, size_t maxLen) { return 0; };
    size_t putString(const char* key, const char* value) { return 0; }
    void end() {}
};

#endif  // !ARDUINO
