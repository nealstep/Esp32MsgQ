#pragma once

#ifdef ARDUINO

#include <Arduino.h>
#define LOG_E(x, y) output.handle(x, y, __FILE__, __LINE__)
#define DEBUG(X) do { SER.print(X) ; SER.printf(" %s:%d\n", __FILE__, __LINE__); } while (0)

#else

#include <cstdint>
#include <iostream>

#define LOG_E(x, y) output.handle(x, y, __FILE__, __LINE__)
#define DEBUG(X) std::cout << X << " " << __FILE__ << ":" << __LINE__ << std::endl

#endif   // ARDUINO !ARDUINO

#include "util/util.hpp"