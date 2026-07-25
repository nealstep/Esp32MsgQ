#include "output.hpp"

#include "modules/module.hpp"
#include "prefs/prefs.hpp"

void die(void) {
    output.print("Died");
#ifdef ARDUINO
    while (true) delay(50);
#else
    exit(1);
#endif  // ARDUINO !ARDUINO
}

Error::Err Output::handle(Readings::Entry reading) {
    char out_s[Output::output_size];
    char ts_s[Output::timestamp_size];
    const char* fmt;

    Error::Err err = _get_timestamp(reading.asof, ts_s, sizeof(ts_s));
    if (err != Error::Err::NoErr) return err;
    Module* mod = modules.get_module(reading.mid);
    if (mod == nullptr) return Error::Err::NoMod;
    int len;
    switch (reading.payload) {
        case Readings::Payload::U8:
            len = snprintf(out_s, sizeof(out_s), reading_fmt_u,
                           get_output(Out::Rdg), mod->get_name(),
                           mod->get_control_name(reading.cid), reading.value.u8,
                           ts_s);
            break;
        case Readings::Payload::FLOAT:
            len = snprintf(out_s, sizeof(out_s), reading_fmt_f,
                           get_output(Out::Rdg), mod->get_name(),
                           mod->get_control_name(reading.cid), reading.value.f,
                           ts_s);
            break;
        default:
            err = Error::Err::UnPay;
            break;
    }
    if (len < 0)
        err = Error::Err::Form;
    else if (len >= sizeof(out_s))
        err = Error::Err::Trunc;
    if (err == Error::Err::NoErr) print(out_s);
    return err;
}

void Output::_handle(const char* err_m, const char* name, const char* fname,
                     int line) {
    char err_s[output_size];
    char ts_s[timestamp_size];
    char ctx_s[context_size];

    Contexts::Context ctx;
    ctx.set(name, fname, line);
    Error::Err err = _get_timestamp(time(NULL), ts_s, sizeof(ts_s));
    if (err != Error::Err::NoErr) {
        // TODO: #12 emit error message for get timestamp
        print("Error in error handler at _get_timestamp, Aborting");
        return;
    }
    Contexts::Context::Err ctx_err = ctx.get_str(ctx_s, sizeof(ctx_s));
    if (ctx_err == Contexts::Context::Err::Format) {
        // TODO: #11 emit error for ctx format error
        strlcpy(ctx_s, Contexts::Context::empty, sizeof(ctx_s));
    } else if (ctx_err == Contexts::Context::Err::Trunc) {
        // TODO: #9 emit error for tuncated context
    }
    int len = snprintf(err_s, sizeof(err_s), error_fmt, get_output(Output::Erm),
                       err_m, ctx_s, fname, line, ts_s);
    if (len < 0) {
        // TODO: #10 emit error message sprintf format
        print("Error in error handler at snprintf type format, Aborting");
        return;
    } else if (len >= sizeof(err_s))
        // TODO: emit error message sprintf truncated
        print("Error in error handler at snprintf type truncated");
    print(err_s);
}

Error::Err Output::_get_timestamp(time_t asof, char* buffer, size_t len) {
    Error::Err err = Error::Err::NoErr;
    if (asof == 0) {
        size_t sz = strlcpy(buffer, no_time, len);
        if (sz >= len) err = Error::Err::Ovr;
    } else {
        const struct tm* timeinfo = localtime(&asof);
        size_t sz = strftime(buffer, len, time_fmt, timeinfo);
        if (sz >= len) err = Error::Err::Ovr;
    }
    return err;
}

void Output::_handle(const char* dv_s, bool broadcast) {
    char data_s[output_size];
    char ts_s[Output::timestamp_size];
    Error::Err err = _get_timestamp(time(NULL), ts_s, sizeof(ts_s));
    if (err != Error::Err::NoErr) {
        print("Error in data handler at _get_timestamp, Aborting");
        return;
    }
    int len = snprintf(data_s, sizeof(data_s), data_fmt,
                       get_output(Output::Dat), dv_s, ts_s);
    if (len < 0) {
        print("Error in data handler at snprintf data format, Aborting");
        return;
    } else if (len >= sizeof(data_s))
        print("Error in data handler at snprintf data truncated");
    print(data_s);
}
