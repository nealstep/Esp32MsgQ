#include "util.hpp"

#ifndef ARDUINO
#include <iostream>
#endif  // !ARDUINO

void die(void) {
#ifdef ARDUINO
#ifdef SER
    SER.println("Died");  // OK
#endif                    // SER
    while (true) delay(50);
#else
    std::cout << "Died" << std::endl;  // OK
    exit(1);
#endif  // ARDUINO !ARDUINO
}