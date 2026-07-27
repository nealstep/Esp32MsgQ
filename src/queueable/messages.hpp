#pragma once

#include "contexts.hpp"
#include "global.hpp"
#include "queue_t.hpp"

#define SECT_LIST(X) \
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

// ideally units should align with Readings::units if they are the same
#define VARIABLE_LIST(X)                          \
    X(GitVer, "GitVer", "Git Version", "")        \
    X(FirmVer, "FirmVer", "Firmware Version", "") \
    X(BuildTime, "BTime", "Build Time", "")       \
    X(BuildID, "BID", "Build ID", "")             \
    X(CPUF, "CPUF", "CPU Frew", "Mhz")            \
    X(FlshF, "FFreq", "Flash Freq", "Mhz")        \
    X(Heap, "FHeap", "Free Heap", "K")            \
    X(MsgId, "MsgId", "Message ID", "")           \
    X(ChpNam, "ChpNam", "Chip Name", "")          \
    X(ChpID, "ChpID", "Chip Id", "")

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
    enum class Sec : uint32_t {
        Unnamed = 0,
#define AS_ENUM(name, string) name = 1 << __COUNTER__,
        SECT_LIST(AS_ENUM)
#undef AS_ENUM
            Count = 1 << __COUNTER__
    };

#define GENERATE_ENUM(id, msg) id,
    enum class Sev : uint8_t { SEVERITY_LIST(GENERATE_ENUM) Count };
    enum class Not : uint16_t { NOTICE_LIST(GENERATE_ENUM) Count };
#undef GENERATE_ENUM
#define GENERATE_ENUM(id, var, hlp, unit) id,
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
        Sec sect;
        Sev sev;
        Not notice;
        cid_t cid;
    } Entry;

    QueueT<Entry> queue;

    // default mask and severity level
    Sec sect_mask;
    Sev sev_lvl;

    // lazy singleton
    static Messages& getInstance(void) {
        static Messages instance;
        return instance;
    }
    Messages(const Messages&) = delete;
    Messages& operator=(const Messages&) = delete;

    // message functions
    constexpr const char* get_sect(Sec code) {
        uint32_t sect = static_cast<uint32_t>(code);
        if (sect == 0) return _sects[0];
        uint8_t bit = __builtin_ctz(sect);
        if (bit >= sect_max) {
            return Word::Invalid;
        }
        return _sects[bit + 1];
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
    static constexpr const char* get_unit(Var code) {
        return _var_units[static_cast<uint16_t>(code)];
    }

    void add(Sec sect, Sev sev, Not notice, const char* name, const char* fname,
             int line) {
        cid_t cid = contexts.get_next_context();
        if (cid <= Contexts::context_max) {
            Contexts::Context* ctx = contexts.get_context(cid);
            if (ctx) {
                if (ctx->set(name, fname, line)) {
                    // TODO: handle invalid context messages
                }
                Entry entry;
                entry.asof = time(NULL);
                entry.sect = sect;
                entry.sev = sev;
                entry.notice = notice;
                entry.cid = cid;
                queue.push(entry);
            }
        } else {
            // TODO: #21 handle running out of contexts messages
        }
    }

    void clr_sect_mask(void) { sect_mask = Sec::Unnamed; }
    void all_sect_mask(void) {
        sect_mask = static_cast<Sec>(static_cast<uint32_t>(Sec::Count) - 1);
    }
    void add_sect_mask(const char* name) {
        // TODO: #22 search for section anme and add it
    }
    void add_sect_mask(Sec s) {
        sect_mask = static_cast<Sec>(static_cast<uint32_t>(sect_mask) |
                                     static_cast<uint32_t>(s));
    }

    void set_sev_lvl(const char* name) {
        // TODO: #23 search for severity name and set it
    }
    void set_sev_lvl(Sev sev) { sev_lvl = sev; }

    bool inCode(Sec item, Sec code) {
        return (static_cast<uint32_t>(item) & static_cast<uint32_t>(code)) != 0;
    }

   protected:
#define GENERATE_STRING(id, msg) msg,
    static constexpr const char* const _sects[] = {"Unamed",
                                                   SECT_LIST(GENERATE_STRING)};
    static constexpr const char* const _severities[] = {
        SEVERITY_LIST(GENERATE_STRING)};
    static constexpr const char* const _notices[] = {
        NOTICE_LIST(GENERATE_STRING)};
#undef GENERATE_STRING
#define GENERATE_STRING(id, var, help, unit) var,
    static constexpr const char* const _variables[] = {
        VARIABLE_LIST(GENERATE_STRING)};
#undef GENERATE_STRING
#define GENERATE_STRING(id, var, help, unit) help,
    static constexpr const char* const _var_help[] = {
        VARIABLE_LIST(GENERATE_STRING)};
#undef GENERATE_STRING
#define GENERATE_STRING(id, var, help, unit) unit,
    static constexpr const char* const _var_units[] = {
        VARIABLE_LIST(GENERATE_STRING)};
#undef GENERATE_STRING
#undef SECT_LIST
#undef SEVERITY_LIST
#undef NOTICE_LIST
#undef VARIABLE_LIST

    uint32_t sect_max;

    Messages() {
        sect_max = __builtin_ctz(static_cast<uint32_t>(Sec::Count));
        all_sect_mask();
        set_sev_lvl(Sev::Dbg);
    }
};

static Messages& messages = Messages::getInstance();

#define LOG_MQ(Sc, Sv, M, N) messages.add(Sc, Sv, M, N, __FILE__, __LINE__)
