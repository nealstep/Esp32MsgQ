#pragma once

#include "global.hpp"
#include "modules.hpp"
#include "queuer/queuer.hpp"

#define ERROR_LIST(X)           \
    X(NoErr, "NoError")         \
    X(NotEn, "Not Enabled")     \
    X(NoCon, "No Such Control") \
    X(QErr, "Queing Error")

class Module {
   public:
#define GENERATE_ENUM(id, msg) id,
    enum class Err : uint8_t { ERROR_LIST(GENERATE_ENUM) Count };
#undef GENERATE_ENUM

    Module(Modules::Typ typ, const char* kind, int8_t loc, uint8_t id)
        : _loc(loc), _id(id) {
        size_t len = snprintf(_name, sizeof(_name), Modules::name_fmt,
                              Modules::get_tname(typ), kind, _loc, _id);
        if (len >= sizeof(_name)) {
            die();
        }
        _mid = modules.add(this);
    }

    bool is_enabled(void) { return _enabled; }
    void set_enable(bool val) { _enabled = val; }
    const char* get_name(void) { return _name; }
    static constexpr const char* get_error(Err err) {
        return _errors[static_cast<uint8_t>(err)];
    }

    virtual Err get_control(controls_t cid) = 0;
    virtual const char* get_control_name(controls_t) = 0;

   protected:
#define GENERATE_STRING(id, msg) msg,
    static constexpr const char* const _errors[] = {
        ERROR_LIST(GENERATE_STRING)};
#undef GENERATE_STRING
#undef ERROR_LIST

    virtual ~Module() = default;

    char _name[Modules::name_size];
    modules_t _mid;
    uint8_t _loc;
    uint8_t _id;
    bool _enabled = true;

    Err _queue_reading(const Modules::Reading reading) { 
        if (queuer.push(Queuer::Queuer::Rdgs, reading)) {
            return Err::NoErr;
        }
        return Err::QErr;
    }
};
