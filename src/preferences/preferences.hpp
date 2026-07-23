#pragma once

#include "global.hpp"

#define PREF_LIST(X)    \
    X(UsSer, "UseSer")  \
    X(SerSpd, "SerSpd") \
    X(ChpNam, "ChpNam")

class Prefs {
   public:
#define GENERATE_ENUM(id, msg) id,
    enum Prf : uint8_t { PREF_LIST(GENERATE_ENUM) Count };
#undef GENERATE_ENUM

    // lazy singleton
    static Prefs& getInstance(void) {
        static Prefs instance;
        return instance;
    }
    Prefs(const Prefs&) = delete;
    Prefs& operator=(const Prefs&) = delete;

    bool get_pref_b(Prf prf);
    uint8_t get_pref_u8(Prf prf);
    uint32_t get_pref_u32(Prf prf);
    const char* get_pref_s(Prf prf);

    bool set_pref_b(Prf prf, bool val);
    bool set_pref_u8(Prf prf, uint8_t val);
    bool set_pref_u32(Prf prf, uint32_t val);
    bool set_pref_s(Prf prf, const char* val);

   protected:
#define GENERATE_STRING(id, msg) msg,
    static constexpr const char* _prefs[] = {PREF_LIST(GENERATE_STRING)};
#undef GENERATE_STRING
#undef PREF_LIST

    // hidden creator
    Prefs(void) {};
};
