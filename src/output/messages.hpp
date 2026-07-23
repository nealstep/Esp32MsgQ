#pragma once

#include "global.hpp"

#define UNIT_LIST(X) \
    X(Main, "Main")  \
    X(Pref, "Pref")  \
    X(Mods, "Mods")  \
    X(Mess, "Mess")  \
    X(MSer, "MSer")  \
    X(MNet, "MNet")

#define NOTICE_LIST(X)     \
    X(NoError, "No Error") \
    X(UnkError, "Unknown Error")

#define WORD_LIST(X)      \
    X(Unknown, "Unknown") \
    X(Invalid, "Invalid") \
    X(NoTime, "NoTime")

#define SEVERITY_LIST(X) \
    X(Dbg, "Debug")      \
    X(Inf, "Info")       \
    X(Wrn, "Warning")    \
    X(Err, "Error")      \
    X(All, "All")

class Messages {
   public:
    enum class Uni : uint32_t {
        Unnamed = 0,
#define AS_ENUM(name, string) name = 1 << __COUNTER__,
        UNIT_LIST(AS_ENUM)
#undef AS_ENUM
            Count = 1 << __COUNTER__
    };

#define GENERATE_ENUM(id, msg) id,
    enum class Sev : uint8_t { SEVERITY_LIST(GENERATE_ENUM) Count };
    enum class Not : uint16_t { NOTICE_LIST(GENERATE_ENUM) Count };
#undef GENERATE_ENUM

#define GENERATE_STRING(id, msg) msg,
    static constexpr const char* const Units[] = {"Unamed",
                                                  UNIT_LIST(GENERATE_STRING)};
    static constexpr const char* const Severities[] = {
        SEVERITY_LIST(GENERATE_STRING)};
    static constexpr const char* const Notices[] = {
        NOTICE_LIST(GENERATE_STRING)};
#undef GENERATE_STRING

    class Word {
       public:
#define AS_CONSTCHAR(name, string) \
    static constexpr const char* const name = string;
        WORD_LIST(AS_CONSTCHAR)
#undef AS_CONSTCHAR
    };

    static constexpr uint8_t message_size = 80;

    typedef struct {
        time_t asof;
        Uni unit;
        Sev severity;
        char* message[message_size];
    } LogMessage;

    // lazy singleton
    static Messages& getInstance(void) {
        static Messages instance;
        return instance;
    }
    Messages(const Messages&) = delete;
    Messages& operator=(const Messages&) = delete;

    // message functions
    constexpr const char* get_word(Uni code) {
        uint32_t unit = static_cast<uint32_t>(code);
        if (unit == 0) return Units[0];
        uint8_t bit = __builtin_ctz(unit);
        if (bit >= unit_max) {
            return Word::Invalid;
        }
        return Units[bit + 1];
    }
    constexpr const char* get_message(Sev code) {
        return Severities[static_cast<uint8_t>(code)];
    }
    constexpr const char* get_message(Not code) {
        return Notices[static_cast<uint16_t>(code)];
    }

   protected:
    uint32_t unit_max;

    Messages() { unit_max = __builtin_ctz(static_cast<uint32_t>(Uni::Count)); }
};

static Messages& messages = Messages::getInstance();
