#include "native.hpp"

#ifndef ARDUINO

#include <chrono>
#include <thread>

// impleement pretend preference
void delay(uint32_t ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

#endif  // !ARDUINO
