#include "util.hpp"

#ifndef ARDUINO
#include <iostream>
#endif  // !ARDUINO

void die(void) {
#ifdef ARDUINO
    SER.println("Died"); // OK
    while (true) delay(50);
#else
    std::cout << "Died" << std::endl; // OK
    exit(1);
#endif  // ARDUINO !ARDUINO
}