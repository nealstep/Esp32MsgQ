#pragma once

#include "global.hpp"

#define UNIT_LIST(X) \
    X(Main, "Main")  \
    X(Mods, "Mods")  \
    X(Mess, "Mess")  \
    X(MNet, "MNet")  \
    X(Pref, "Pref")  \
    X(Quer, "Quer")

#define NOTICE_LIST(X)       \
    X(NoNotice, "No Notice") \
    X(Start, "Starting")     \
    X(Started, "Started")

#define VARIABLE_LIST(X)            \
    X(GitVer, "Git Version")        \
    X(FirmVer, "Firmware Verstion") \
    X(BuildTime, "Build Time")      \
    X(BuildID, "Build ID")          \
    X(CPUF, "CPU Freq (Mhz)")       \
    X(FlshF, "Flash Freq (Mhz)")    \
    X(Heap, "Free Heap (bytes)")    \
    X(MsgId, "Message ID")

#define WORD_LIST(X)      \
    X(Unknown, "Unknown") \
    X(Invalid, "Invalid") \
    X(NoTime, "NoTime")   \
    X(Empty, "Empty")

#define SEVERITY_LIST(X) \
    X(Dbg, "Debug")      \
    X(Inf, "Info")       \
    X(Wrn, "Warning")    \
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
    enum class Var : uint16_t { VARIABLE_LIST(GENERATE_ENUM) Count };
#undef GENERATE_ENUM

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
        Not notice;
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
        if (unit == 0) return _units[0];
        uint8_t bit = __builtin_ctz(unit);
        if (bit >= unit_max) {
            return Word::Invalid;
        }
        return _units[bit + 1];
    }
    static constexpr const char* get_message(Sev code) {
        return _severities[static_cast<uint8_t>(code)];
    }
    static constexpr const char* get_message(Not code) {
        return _notices[static_cast<uint16_t>(code)];
    }
    static constexpr const char* get_message(Var code) {
        return _variables[static_cast<uint16_t>(code)];
    }

   protected:
#define GENERATE_STRING(id, msg) msg,
    static constexpr const char* const _units[] = {"Unamed",
                                                   UNIT_LIST(GENERATE_STRING)};
    static constexpr const char* const _severities[] = {
        SEVERITY_LIST(GENERATE_STRING)};
    static constexpr const char* const _notices[] = {
        NOTICE_LIST(GENERATE_STRING)};
    static constexpr const char* const _variables[] = {
        VARIABLE_LIST(GENERATE_STRING)};
#undef GENERATE_STRING
#undef UNIT_LIST
#undef SEVERITY_LIST
#undef NOTICE_LIST
#undef VARIABLE_LIST

    uint32_t unit_max;

    Messages() { unit_max = __builtin_ctz(static_cast<uint32_t>(Uni::Count)); }
};

static Messages& messages = Messages::getInstance();

// TODO: #15 implement queued messages
// #define LOG_MQ(U, S, N, CI) output.handle(U, S, N, C, __FILE__, __LINE__)
