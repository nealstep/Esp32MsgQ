#include "util.hpp"

#ifndef ARDUINO
#include <iostream>
#endif  // !ARDUINO

void die(void) {
#ifdef ARDUINO
    SER.println("Died");
    while (true) delay(50);
#else
    std::cout << "Died" << std::endl;
    exit(1);
#endif  // ARDUINO !ARDUINO
}