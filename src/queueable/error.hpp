#pragma once

#include "contexts.hpp"
#include "global.hpp"
#include "queue_t.hpp"

#define ERROR_LIST(X)            \
    X(NoErr, "NoError")          \
    X(NotEn, "Not Enabled")      \
    X(NoCon, "No Such Control")  \
    X(QErr, "Queing Error")      \
    X(Ovr, "Overrun")            \
    X(UnPay, "Unknown Payload")  \
    X(NoMod, "No Module")        \
    X(ModMax, "Maximum Modules") \
    X(Form, "Format Error")      \
    X(Trunc, "Truncated")        \
    X(NotFnd, "Not Found")       \
    X(BdTy, "Bad Type")          \
    X(BdT2, "Bad Type Pref")     \
    X(NoNet, "No Network")       \
    X(NoInt, "No Internet")      \
    X(TimeSyncFail, "Time Sync Failed")

class Error {
   public:
#define GENERATE_ENUM(id, msg) id,
    enum class Err : uint8_t { ERROR_LIST(GENERATE_ENUM) Count };
#undef GENERATE_ENUM

    typedef struct {
        Error::Err err;
        time_t asof;
        cid_t cid;
    } Entry;

    QueueT<Entry> queue;

    // lazy singleton
    static Error& getInstance(void) {
        static Error instance;
        return instance;
    }
    Error(const Error&) = delete;
    Error& operator=(const Error&) = delete;

    static constexpr const char* get_error(Err err) {
        return _errors[static_cast<uint8_t>(err)];
    }

    void add(Err err, const char* name, const char* fname, int line) {
        cid_t cid = contexts.get_next_context();
        if (cid <= Contexts::context_max) {
            Contexts::Context* ctx = contexts.get_context(cid);
            if (ctx) {
                if (ctx->set(name, fname, line)) {
                    cid_t cid2 = contexts.get_next_context();
                    if (cid2 <= Contexts::context_max) {
                        Contexts::Context* ctx2 = contexts.get_context(cid);
                        if (ctx) {
                            ctx2->set("Error::err", __FILE__, __LINE__);
                        }
                    }
                    Entry entry2;
                    entry2.err = Err::Trunc;
                    entry2.asof = time(NULL);
                    entry2.cid = cid2;
                    queue.push(entry2);
                }
                Entry entry;
                entry.err = err;
                entry.asof = time(NULL);
                entry.cid = cid;
                queue.push(entry);
            }
        } else {
            // TODO: #17 handle running out of contexts
#ifdef ARDUINO
#ifdef SER
            DEBUG("Ran out of contexts");
#endif  // SER
#else   // !ARDUINO
            DEBUG("Ran out of contexts");
#endif  // ARDUINO !ARDUINO
        }
    }

   protected:
#define GENERATE_STRING(id, msg) msg,
    static constexpr const char* const _errors[] = {
        ERROR_LIST(GENERATE_STRING)};
#undef GENERATE_STRING
#undef MOD_ERROR_LIST

    Error() {}
};

static Error& error = Error::getInstance();

#define LOG_EQ(E, N) error.add(E, N, __FILE__, __LINE__)
