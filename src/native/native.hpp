#pragma once

#include "global.hpp"

#ifndef ARDUINO

void delay(uint32_t ms);

class Preferences {
   public:
    bool begin(const char* name, bool read_only) { return false; }
    bool isKey(const char* key) { return false; }
    bool getBool(const char* key) { return false; }
    void end() {}
};

#endif  // !ARDUINO
