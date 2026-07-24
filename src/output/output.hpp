#pragma once

#include "messages.hpp"
#include "modules/module.hpp"
#include "prefs/prefs.hpp"

#define OUTPUT_LIST(X) \
    X(Rdg, '@')        \
    X(Log, '*')        \
    X(Erm, '!')        \
    X(Dat, '$')

#define ERROR_LIST(X)           \
    X(NoErr, "No Error")        \
    X(Ovr, "Overrun")           \
    X(UnPay, "Unknown Payload") \
    X(NoMod, "No Module")       \
    X(Form, "Format Error")     \
    X(Trunc, "Truncated")       \
    X(EiE, "Error In Error")

class Output {
   public:
#define GENERATE_ENUM(id, msg) id,
    enum Out : uint8_t { OUTPUT_LIST(GENERATE_ENUM) Count };
    enum class Err : uint8_t { ERROR_LIST(GENERATE_ENUM) Count };
#undef GENERATE_ENUM

    static constexpr const uint8_t output_size = 128;
    static constexpr const uint8_t timestamp_size = 32;
    static constexpr const uint8_t data_size = 64;
    static constexpr const uint8_t filename_size = 32;

    static constexpr const uint8_t contexts_max = 12;
    static constexpr const uint8_t context_exhausted = 0xFF;

    static constexpr const char* const reading_fmt_u = "%c|%s|%s|%u|%s";
    static constexpr const char* const reading_fmt_f = "%c|%s|%s|%f|%s";
    static constexpr const char* const error_fmt = "%c|%s|%s|%s:%d|%s";
    static constexpr const char* const time_fmt = "%Y-%m-%d@%H:%M:%S-%Z";
    static constexpr const char* const data_fmt = "%c|%s|%s";
    static constexpr const char* const data_s_u = "%s|%u";
    static constexpr const char* const data_s_s = "%s|%s";

    static constexpr const char* const no_time = "NoTime";

    class Context {
       public:
        static constexpr const uint8_t name_size = 32;

        Context() { _name[0] = '\0'; }
        Context(const char* name) { strlcpy(_name, name, sizeof(_name)); }

        const char* get_name(void) { return _name; }

        // we can add to this over time
        const char* get_str(void) { return get_name(); }

       protected:
        char _name[name_size];
    };

    struct ErrEnt {
        enum EType : uint8_t { Out, Mod, Pref } etype;
        union {
            Output::Err out;
            Module::Err mod;
            Prefs::Err pref;
        } err;
        Context* ctx;
        char file[filename_size];
        int line;
    };

    bool use_serial = false;
    bool serial_rdy = false;
    uint32_t serial_speed;

    // lazy singleton
    static Output& getInstance(void) {
        static Output instance;
        return instance;
    }
    Output(const Output&) = delete;
    Output& operator=(const Output&) = delete;

    Err handle(Modules::Reading reading);
    void handle(Err err, Context ctx, const char* fname, int line) {
        _handle(get_error(err), ctx, fname, line);
    }
    void handle(Module::Err err, Context ctx, const char* fname, int line) {
        _handle(Module::get_error(err), ctx, fname, line);
    }
    void handle(Prefs::Err err, Context ctx, const char* fname, int line) {
        _handle(Prefs::get_error(err), ctx, fname, line);
    }
    void handle(Messages::Uni uni, Messages::Sev sev, Messages::Not notice,
                Context ctx, const char* fname, int line) {}

    void handle(Messages::Var var, uint32_t val, bool broadcast = false) {
        char dv_s[data_size];
        int len = snprintf(dv_s, sizeof(dv_s), data_s_u,
                           Messages::get_message(var), val);
        if (len < 0) {
            print("Error in data handler at snprintf format, Aborting");
            return;
        } else if (len >= sizeof(dv_s))
            print("Error in data handler at snprintf truncated");
        _handle(dv_s, broadcast);
    }

    void handle(Messages::Var var, const char* val, bool broadcast = false) {
        char dv_s[data_size];
        int len = snprintf(dv_s, sizeof(dv_s), data_s_s,
                           Messages::get_message(var), val);
        if (len < 0) {
            print("Error in data handler at snprintf format, Aborting");
            return;
        } else if (len >= sizeof(dv_s))
            print("Error in data handler at snprintf truncated");
        _handle(dv_s, broadcast);
    }

    void print(const char* str) {
#ifdef ARDUINO
#ifdef SER
        if (serial_rdy) SER.println(str);  // OK
#endif                                     // SER
#else                                      // !ARDUINO
        std::cout << str << std::endl;  // OK
#endif                                     // ARDUINO !ARDIUNO
    }
    static constexpr const char get_output(Out out) { return _outputs[out]; }
    static constexpr const char* get_error(Err err) {
        return _errors[static_cast<uint8_t>(err)];
    }

    uint8_t get_next_context() {
        uint8_t ind = 0;
        while (ind < contexts_max) {
            if (context_free[ind]) {
                context_free[ind] = false;
                return ind;
            }
            ind++;
        }
        return context_exhausted;
    }

    Context* get_context(uint8_t ind) {
        if (context_free[ind]) {
            print("Error in error, requested a context that is not in use");
            return nullptr;
        } else if (ind >= contexts_max) {
            // invalid
        }
        return &contexts[ind];
    }

    void free_context(uint8_t ind) { context_free[ind] = true; }

   protected:
#define GENERATE_STRING(id, msg) msg,
    static constexpr const char _outputs[] = {OUTPUT_LIST(GENERATE_STRING)};
    static constexpr const char* const _errors[] = {
        ERROR_LIST(GENERATE_STRING)};
#undef GENERATE_STRING
#undef OUTPUT_LIST
#undef ERROR_LIST

    Context contexts[contexts_max];
    bool context_free[contexts_max];
    uint8_t context_ind = 0;

    // hidden creator
    Output(void) {
        for (uint8_t ind = 0; ind < contexts_max; ind++)
            context_free[ind] = true;
    };

    Err _get_timestamp(time_t asof, char* buffer, size_t len);
    void _handle(const char* err_m, Context ctx, const char* fname, int line);
    void _handle(const char* data_s, bool broadcast);
};

static Output& output = Output::getInstance();
