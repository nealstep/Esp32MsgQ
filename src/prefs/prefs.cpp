#include "prefs.hpp"

Error::Err Prefs::get_pref(Prf prf, bool& val) {
    Error::Err err = Error::Err::NoErr;
    const char* key = get_key(prf);
    if (preferences.isKey(key)) {
        if (check_type(prf, DType::BOOL)) {
#if defined(ARDUINO_ARCH_ESP32)
            if (preferences.getType(key) != PT_U8) {
                return Err::BdT2;
            }
#endif  // ARDUINO_ARCH_ESP32
            val = preferences.getBool(key);
            err = Error::Err::NoErr;
        } else
            err = Error::Err::BdTy;
    } else
        err = Error::Err::NotFnd;
    return err;
}

Error::Err Prefs::get_pref(Prf prf, uint32_t& val) {
    Error::Err err = Error::Err::NoErr;
    const char* key = get_key(prf);
    if (preferences.isKey(key)) {
        if (check_type(prf, DType::U32)) {
#if defined(ARDUINO_ARCH_ESP32)
            if (preferences.getType(key) != PT_U32) {
                return Err::BdT2;
            }
#endif  // ARDUINO_ARCH_ESP32
            val = preferences.getUInt(key);
            err = Error::Err::NoErr;
        } else
            err = Error::Err::BdTy;
    } else
        err = Error::Err::NotFnd;
    return err;
}


Error::Err Prefs::get_pref(Prf prf, char* buf, size_t buf_len) {
    Error::Err err = Error::Err::NoErr;
    const char* key = get_key(prf);
    // if (check_type(prf, DType::STR)) {
    //     size_t len = preferences.getString(key, buffer, buf_len);

    // } else {
    //     // bad_type
    // }
    return err;
}

Error::Err Prefs::set_pref(Prf prf, bool val) {
    Error::Err err = Error::Err::NoErr;
    return err;
}

Error::Err Prefs::set_pref(Prf prf, uint32_t val) {
    Error::Err err = Error::Err::NoErr;
    return err;
}


Error::Err Prefs::set_pref(Prf prf, const char* buf) {
    Error::Err err = Error::Err::NoErr;
    return err;
}
