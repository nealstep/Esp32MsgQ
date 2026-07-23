#pragma once

#include "modules/module.hpp"

#define OUTPUT_LIST(X) \
    X(Rdg, '@')        \
    X(Log, '*')        \
    X(Erm, '!')

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

    static constexpr const char* const reading_fmt_u = "%c|%s|%s|%u|%s";
    static constexpr const char* const reading_fmt_f = "%c|%s|%s|%f|%s";
    static constexpr const char* const error_fmt = "%c|%s|%s|%s:%d|%s";
    static constexpr const char* const time_fmt = "%Y-%m-%d@%H:%M:%S-%Z";

    static constexpr const char* const no_time = "NoTime";

    class Context {
       public:
        static constexpr const uint8_t name_size = 32;

        Context(const char* name) { strlcpy(_name, name, sizeof(_name)); }
        const char* get_name(void) { return _name; }

       protected:
        char _name[name_size];
    };

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

    void print(const char* str) {
#ifdef ARDUINO
#ifdef SER
        SER.println(str);
#endif  // SER
#else   // !ARDUINO
        std::cout << str << std::endl;
#endif  // ARDUINO !ARDIUNO
    }
    static constexpr const char get_output(Out out) { return _Outputs[out]; }
    static constexpr const char* get_error(Err err) {
        return _Errors[static_cast<uint8_t>(err)];
    }

   protected:
#define GENERATE_STRING(id, msg) msg,
    static constexpr const char _Outputs[] = {OUTPUT_LIST(GENERATE_STRING)};
    static constexpr const char* const _Errors[] = {
        ERROR_LIST(GENERATE_STRING)};
#undef GENERATE_STRING
#undef OUTPUT_LIST
#undef ERROR_LIST

    // hidden creator
    Output(void) {};

    Err _get_timestamp(time_t asof, char* buffer, size_t len);
    void _handle(const char* err_m, Context ctx, const char* fname, int line);
};

static Output& output = Output::getInstance();
