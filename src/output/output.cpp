#include "output.hpp"

#include "modules/module.hpp"
#include "prefs/prefs.hpp"

#ifndef ARDUINO
#include <cstring>
#endif  // !ARDUINO

void die(void) {
    output.print("Died");
#ifdef ARDUINO
    while (true) delay(50);
#else
    exit(1);
#endif  // ARDUINO !ARDUINO
}

Error::Err Output::handle(Error::Entry erre) {
    char err_s[output_size];
    char ts_s[Output::timestamp_size];
    char ctx_s[context_size];

    Error::Err err = _get_timestamp(erre.asof, ts_s, sizeof(ts_s));
    if (err != Error::Err::NoErr) {
        LOG_EQ(err, "Output::_handle erre _get_timestamp");
    }
    Contexts::Context* ctx = contexts.get_context(erre.cid);
    Contexts::Context::Err ctx_err = ctx->get_str(ctx_s, sizeof(ctx_s));
    if (ctx_err == Contexts::Context::Err::Format) {
        LOG_EQ(Error::Err::Form, "Output::_handle erre ct.get_str");
        strlcpy(ctx_s, Contexts::Context::context_empty, sizeof(ctx_s));
    } else if (ctx_err == Contexts::Context::Err::Trunc) {
        LOG_EQ(Error::Err::Trunc, "Output::_handle erre ct.get_str");
    }
    contexts.free_context(erre.cid);
    int len = snprintf(err_s, sizeof(err_s), error_fmt, get_output(Output::Erm),
                       Error::get_error(erre.err), ctx_s, ts_s);
    if (len < 0) {
        err = Error::Err::Form;
        LOG_EQ(err, "Output::_handle erre snprintf format");
        return err;
    } else if (len >= sizeof(err_s)) {
        err = Error::Err::Trunc;
        LOG_EQ(err, "Output::_handle erre snprintf trunc");
    }
    print(err_s);
    return err;
}

Error::Err Output::handle(Datum::Entry data) {
    char dv_s[data_size];
    int len = 0;
    Error::Err err = Error::Err::NoErr;

    char* sptr = nullptr;
    switch (data.payload) {
        case Datum::Payload::U8:
            len = snprintf(dv_s, sizeof(dv_s), data_s_u,
                           Messages::get_message(data.vid), data.value.u8,
                           Messages::get_unit(data.vid));
            break;
        case Datum::Payload::U16:
            len = snprintf(dv_s, sizeof(dv_s), data_s_u,
                           Messages::get_message(data.vid), data.value.u16,
                           Messages::get_unit(data.vid));
            break;
        case Datum::Payload::U32:
            len = snprintf(dv_s, sizeof(dv_s), data_s_u,
                           Messages::get_message(data.vid), data.value.u32,
                           Messages::get_unit(data.vid));
            break;
        case Datum::Payload::SID:
            sptr = datum.get_str(data.value.sid);
            len = snprintf(dv_s, sizeof(dv_s), data_s_s,
                           Messages::get_message(data.vid), sptr,
                           Messages::get_unit(data.vid));
            datum.free_str(data.value.sid);
            break;
        default:
            // TODO: #20 Unknown data type in data output
            break;
    }
    if (len < 0) {
        err = Error::Err::Form;
        LOG_EQ(err, "Output::_handle data snprintf format");
        return err;
    } else if (len >= sizeof(dv_s)) {
        err = Error::Err::Trunc;
        LOG_EQ(err, "Output::_handle data snprintf trunc");
    }
    _handle(dv_s, data.asof, data.broadcast);
    return err;
}

Error::Err Output::handle(Messages::Entry mesg) {
    char msg_s[output_size];
    char ts_s[Output::timestamp_size];
    char ctx_s[context_size];
    Error::Err err = Error::Err::NoErr;

    if (mesg.sev != Messages::Sev::All) {
        if (mesg.sev < messages.sev_lvl) return err;
        if (mesg.sect != Messages::Sec::Unnamed)
            if (!messages.inCode(mesg.sect, messages.sect_mask)) return err;
    }

    err = _get_timestamp(mesg.asof, ts_s, sizeof(ts_s));
    if (err != Error::Err::NoErr) {
        LOG_EQ(err, "Output::_handle mesg _get_timestamp");
    }
    Contexts::Context* ctx = contexts.get_context(mesg.cid);
    Contexts::Context::Err ctx_err = ctx->get_str(ctx_s, sizeof(ctx_s));
    if (ctx_err == Contexts::Context::Err::Format) {
        LOG_EQ(Error::Err::Form, "Output::_handle mesg ct.get_str");
        strlcpy(ctx_s, Contexts::Context::context_empty, sizeof(ctx_s));
    } else if (ctx_err == Contexts::Context::Err::Trunc) {
        LOG_EQ(Error::Err::Trunc, "Output::_handle mesg ct.get_str");
    }
    contexts.free_context(mesg.cid);
    int len =
        snprintf(msg_s, sizeof(msg_s), msg_fmt, get_output(Output::Msg),
                 messages.get_sect(mesg.sect), Messages::get_message(mesg.sev),
                 Messages::get_message(mesg.notice), ctx_s, ts_s);
    if (len < 0) {
        err = Error::Err::Form;
        LOG_EQ(err, "Output::_handle mesg snprintf format");
        return err;
    } else if (len >= sizeof(msg_s)) {
        err = Error::Err::Trunc;
        LOG_EQ(err, "Output::_handle mesg snprintf trunc");
    }
    print(msg_s);
    return err;
}

Error::Err Output::handle(Readings::Entry reading) {
    char out_s[Output::output_size];
    char ts_s[Output::timestamp_size];
    const char* fmt;

    Error::Err err = _get_timestamp(reading.asof, ts_s, sizeof(ts_s));
    if (err != Error::Err::NoErr) {
        LOG_EQ(err, "Output::_handle reading _get_timestamp");
    }
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
        LOG_EQ(err, "Output::_handle err _get_timestamp");
    }
    Contexts::Context::Err ctx_err = ctx.get_str(ctx_s, sizeof(ctx_s));
    if (ctx_err == Contexts::Context::Err::Format) {
        LOG_EQ(Error::Err::Form, "Output::_handle err ct.get_str");
        strlcpy(ctx_s, Contexts::Context::context_empty, sizeof(ctx_s));
    } else if (ctx_err == Contexts::Context::Err::Trunc) {
        LOG_EQ(Error::Err::Trunc, "Output::_handle err ct.get_str");
    }
    int len = snprintf(err_s, sizeof(err_s), error_fmt, get_output(Output::Erm),
                       err_m, ctx_s, ts_s);
    if (len < 0) {
        LOG_EQ(Error::Err::Form, "Output::_handle err snprintf format");
        return;
    } else if (len >= sizeof(err_s))
        LOG_EQ(Error::Err::Trunc, "Output::_handle err snprintf trunc");
    print(err_s);
}

void Output::handle(Messages::Sec sect, Messages::Sev sev, Messages::Not notice,
                    const char* name, const char* fname, int line) {
    char msg_s[output_size];
    char ts_s[timestamp_size];
    char ctx_s[context_size];

    if (sev != Messages::Sev::All) {
        if (sev < messages.sev_lvl) return;
        if (sect != Messages::Sec::Unnamed)
            if (!messages.inCode(sect, messages.sect_mask)) return;
    }

    Contexts::Context ctx;
    ctx.set(name, fname, line);
    Error::Err err = _get_timestamp(time(NULL), ts_s, sizeof(ts_s));
    if (err != Error::Err::NoErr) {
        LOG_EQ(err, "Output::_handle err _get_timestamp");
    }
    Contexts::Context::Err ctx_err = ctx.get_str(ctx_s, sizeof(ctx_s));
    if (ctx_err == Contexts::Context::Err::Format) {
        LOG_EQ(Error::Err::Form, "Output::_handle err ct.get_str");
        strlcpy(ctx_s, Contexts::Context::context_empty, sizeof(ctx_s));
    } else if (ctx_err == Contexts::Context::Err::Trunc) {
        LOG_EQ(Error::Err::Trunc, "Output::_handle err ct.get_str");
    }
    int len = snprintf(msg_s, sizeof(msg_s), msg_fmt, get_output(Output::Msg),
                       messages.get_sect(sect), Messages::get_message(sev),
                       Messages::get_message(notice), ctx_s, ts_s);
    if (len < 0) {
        LOG_EQ(Error::Err::Form, "Output::_handle err snprintf format");
        return;
    } else if (len >= sizeof(msg_s))
        LOG_EQ(Error::Err::Trunc, "Output::_handle err snprintf trunc");
    print(msg_s);
}

Error::Err Output::_get_timestamp(time_t asof, char* buffer, size_t len) {
    Error::Err err = Error::Err::NoErr;
    if (asof == 0) {
        size_t sz = strlcpy(buffer, no_time, len);
        // this should not happen
        if (sz >= len) err = Error::Err::Ovr;
    } else {
        const struct tm* timeinfo = localtime(&asof);
        size_t sz = strftime(buffer, len, time_fmt, timeinfo);
        if (sz >= len) err = Error::Err::Ovr;
    }
    return err;
}

void Output::_handle(const char* dv_s, time_t tt, bool broadcast) {
    char data_s[output_size];
    char ts_s[Output::timestamp_size];
    Error::Err err = _get_timestamp(tt, ts_s, sizeof(ts_s));
    if (err != Error::Err::NoErr) {
        LOG_EQ(err, "Output::_handle data _get_timestamp");
        strlcpy(ts_s, Messages::Word::NoTime, sizeof(ts_s));
    }
    int len = snprintf(data_s, sizeof(data_s), data_fmt,
                       get_output(Output::Dat), dv_s, ts_s);
    if (len < 0) {
        LOG_EQ(Error::Err::Form, "Output::_handle data snprintf format");
        return;
    } else if (len >= sizeof(data_s))
        LOG_EQ(Error::Err::Trunc, "Output::_handle data snprintf trunc");
    print(data_s);
}
