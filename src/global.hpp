#pragma once

#ifdef ARDUINO

#include <Arduino.h>
#define DEBUG(X)                                    \
    do {                                            \
        SER.print(X);                               \
        SER.printf(" %s:%d\n", __FILE__, __LINE__); \
    } while (0)

#else

#include <cstdint>
#include <iostream>

#include "native/native.hpp"

#define DEBUG(X) \
    std::cout << X << " " << __FILE__ << ":" << __LINE__ << std::endl

#endif  // ARDUINO !ARDUINO

#define LOG_ED(E, C) output.handle(E, C, __FILE__, __LINE__)
#define LOG_ND(U, S, N, C) output.handle(U, S, N, C, __FILE__, __LINE__)
#define DATAD(N, V, B) output.handle(N, V, B)

// #define LOG_EQ(E, CI) output.handle(E, C, __FILE__, __LINE__)
// #define LOG_NQ(U, S, N, CI) output.handle(U, S, N, C, __FILE__, __LINE__)
// #define DATAQ(N, V) output.handle(N, V)

#include "util/util.hpp"