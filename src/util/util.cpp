#include "util.hpp"

#ifndef ARDUINO
#include <iostream>
#endif  // !ARDUINO

// TODO: #8 move die to output
void die(void) {
    // TODO: #5 network print die
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