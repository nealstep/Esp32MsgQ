#pragma once

#include "global.hpp"

#if !defined(ARDUINO_ARCH_ESP32) && defined(ARDUINO)
#error "Not Supported"
#endif  // !ARDUINO_ARCH_ESP32 && ARDUINO

#ifdef ARDUINO_ARCH_ESP32
#include <Preferences.h>
#elifndef ARDUINO
#include "native/native.hpp"
#endif  // ARDUINO_ARCH32_ESP32 !ARDUINO

enum class DType : uint8_t { BOOL, U32, STR };

#define ERROR_LIST(X)      \
    X(NoErr, "NoError")    \
    X(NotFnd, "Not Found") \
    X(BdTy, "Bad Type")    \
    X(BdT2, "Bad Type Pref")

#define PREF_LIST(X)                \
    X(UsSer, "UseSer", DType::BOOL) \
    X(SerSpd, "SerSpd", DType::U32) \
    X(ChpNam, "ChpNam", DType::STR)

class Prefs {
   public:
#define GENERATE_ENUM(id, msg) id,
    enum class Err : uint8_t { ERROR_LIST(GENERATE_ENUM) Count };
#undef GENERATE_ENUM
#define GENERATE_ENUM(id, nam, typ) id,
    enum class Prf : uint8_t { PREF_LIST(GENERATE_ENUM) Count };
#undef GENERATE_ENUM

    // lazy singleton
    static Prefs& getInstance(void) {
        static Prefs instance;
        return instance;
    }
    Prefs(const Prefs&) = delete;
    Prefs& operator=(const Prefs&) = delete;

    static constexpr const char* const get_key(Prf prf) {
        return _keys[static_cast<uint8_t>(prf)];
    }
    static constexpr const DType get_dtype(Prf prf) {
        return _dtypes[static_cast<uint8_t>(prf)];
    }
    static constexpr const char* const get_error(Err err) {
        return _errors[static_cast<uint8_t>(err)];
    }

    Err begin(const char* name, bool read_only) {
        bool succ = preferences.begin(name, read_only);
        if (!succ) return Err::NotFnd;
        return Err::NoErr;
    }

    void end(void) { preferences.end(); }

    bool check_type(Prf prf, DType dty) { return get_dtype(prf) == dty; }

    Err get_pref(Prf prf, bool& val);

    Err get_pref(Prf prf, char* buf, size_t buf_len);

    Err set_pref(Prf prf, bool val);

    Err set_pref(Prf prf, const char* buf);

   protected:
#define GENERATE_STRING(id, msg) msg,
    static constexpr const char* const _errors[] = {
        ERROR_LIST(GENERATE_STRING)};
#undef GENERATE_STRING
#define GENERATE_STRING(id, key, dtyp) key,
    static constexpr const char* const _keys[] = {PREF_LIST(GENERATE_STRING)};
#undef GENERATE_STRING
#define GENERATE_DTYPE(id, key, dtyp) dtyp,
    static constexpr const DType _dtypes[] = {PREF_LIST(GENERATE_DTYPE)};
#undef GENERATE_DTYPE
#undef PREF_LIST
#undef ERROR_LIST

    Preferences preferences;

    // hidden creator
    Prefs(void) {};
};

static Prefs& prefs = Prefs::getInstance();
