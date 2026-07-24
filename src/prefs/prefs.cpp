#include "prefs.hpp"

Prefs::Err Prefs::get_pref(Prf prf, bool& val) {
    Err err = Err::NoErr;
    const char* key = get_key(prf);
    if (preferences.isKey(key)) {
        if (check_type(prf, DType::BOOL)) {
#if defined(ARDUINO_ARCH_ESP32)
            if (preferences.getType(key) != PT_U8) {
                return Err::BdT2;
            }
#endif  // ARDUINO_ARCH_ESP32
            val = preferences.getBool(key);
            err = Err::NoErr;
        } else
            err = Err::BdTy;
    } else
        err = Err::NotFnd;
    return err;
}

Prefs::Err Prefs::get_pref(Prf prf, char* buf, size_t buf_len) {
    Err err = Err::NoErr;
    const char* key = get_key(prf);
    // if (check_type(prf, DType::STR)) {
    //     size_t len = preferences.getString(key, buffer, buf_len);

    // } else {
    //     // bad_type
    // }
    return err;
}

Prefs::Err Prefs::set_pref(Prf prf, bool val) {
    Err err = Err::NoErr;
    return err;
}

Prefs::Err Prefs::set_pref(Prf prf, const char* buf) {
    Err err = Err::NoErr;
    return err;
}
