#include "output.hpp"

Output::Err Output::handle(Modules::Reading reading) {
    char out_s[Output::output_size];
    char ts_s[Output::timestamp_size];
    const char* fmt;

    Err err = _get_timestamp(reading.asof, ts_s, sizeof(ts_s));
    if (err != Err::NoErr) return err;
    Module* mod = modules.get_module(reading.mid);
    if (mod == nullptr) return Err::NoMod;
    int len;
    switch (reading.payload) {
        case Modules::Payload::U8:
            len = snprintf(out_s, sizeof(out_s), reading_fmt_u,
                           get_output(Out::Rdg), mod->get_name(),
                           mod->get_control_name(reading.cid), reading.value.u8,
                           ts_s);
            break;
        case Modules::Payload::FLOAT:
            len = snprintf(out_s, sizeof(out_s), reading_fmt_f,
                           get_output(Out::Rdg), mod->get_name(),
                           mod->get_control_name(reading.cid), reading.value.f,
                           ts_s);
            break;
        default:
            err = Err::UnPay;
            break;
    }
    if (len < 0)
        err = Err::Form;
    else if (len >= sizeof(out_s))
        err = Err::Trunc;
    if (err == Err::NoErr) print(out_s);
    return err;
}

void Output::_handle(const char* err_m, Context ctx, const char* fname,
                     int line) {
    char err_s[output_size];
    char ts_s[Output::timestamp_size];
    Err err = _get_timestamp(time(NULL), ts_s, sizeof(ts_s));
    if (err != Err::NoErr) {
        print("Error in error handler at _get_timestamp, Aborting");
        return;
    }
    int len = snprintf(err_s, sizeof(err_s), error_fmt, get_output(Output::Erm),
                       err_m, ctx.get_name(), fname, line, ts_s);
    if (len < 0) {
        print("Error in error handler at snprintf type format, Aborting");
        return;
    } else if (len >= sizeof(err_s))
        print("Error in error handler at snprintf type truncated");
    print(err_s);
}

Output::Err Output::_get_timestamp(time_t asof, char* buffer, size_t len) {
    Err err = Err::NoErr;
    if (asof == 0) {
        size_t sz = strlcpy(buffer, no_time, len);
        if (sz >= len) err = Err::Ovr;
    } else {
        const struct tm* timeinfo = localtime(&asof);
        size_t sz = strftime(buffer, len, time_fmt, timeinfo);
        if (sz >= len) err = Err::Ovr;
    }
    return err;
}
