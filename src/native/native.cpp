#include "native.hpp"

#ifndef ARDUINO

// impleement pretend preference
void delay(uint32_t ms) {
    for (uint32_t i=0 ; i < ms ; i++) {}
}

#endif  // !ARDUINO
