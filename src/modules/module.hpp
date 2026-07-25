#pragma once

#include "global.hpp"
#include "modules.hpp"
#include "output/output.hpp"
#include "queueable/error.hpp"

class Module {
   public:
    Module(Modules::Typ typ, const char* kind, int8_t loc, uint8_t id)
        : _loc(loc), _id(id) {
        size_t len = snprintf(_name, sizeof(_name), Modules::name_fmt,
                              Modules::get_tname(typ), kind, _loc, _id);
        if (len >= sizeof(_name)) {
            LOG_ED(Error::Err::Ovr, "Module::Module");
            die();
        }
        _mid = modules.add(this);
    }

    bool is_enabled(void) { return _enabled; }
    void set_enable(bool val) { _enabled = val; }
    const char* get_name(void) { return _name; }

    virtual Error::Err get_control(controls_t cid) = 0;
    virtual const char* get_control_name(controls_t) = 0;

   protected:
    virtual ~Module() = default;

    char _name[Modules::name_size];
    modules_t _mid;
    uint8_t _loc;
    uint8_t _id;
    bool _enabled = true;
};
