#pragma once

#include "modules/module.hpp"

#ifdef M_S_DUMMY

#define CONTROL_LIST(X) X(S1, "S1")

class M_S_Dummy : public Module {
   public:
#define GENERATE_ENUM(id, msg) id,
    enum Con : controls_t { CONTROL_LIST(GENERATE_ENUM) Count };
#undef GENERATE_ENUM

    static constexpr const Modules::Typ typ = Modules::Typ::Sensor;
    static constexpr const char* const kind = "dummy";

    M_S_Dummy(uint8_t loc, uint8_t id) : Module(typ, kind, loc, id) {}

    Err get_control(controls_t cid) override {
        if (!_enabled) return Module::Err::NotEn;
        Modules::Reading reading;
        if (cid == Con::S1) {
#ifdef ARDUINO
            uint8_t value = random(0, 100);
#else
            uint8_t value = rand() % 101;
#endif  // ARDUINO !ARDUINO
        // create message
            reading.mid = _mid;
            reading.cid = cid;
            reading.asof = time(NULL);
            reading.payload = Modules::Payload::U8;
            reading.value.u8 = value;
        } else {
            return Module::Err::NoCon;
        }
        return _queue_reading(reading);
    }
    const char* get_control_name(controls_t con) override { return _controls[con]; }

   protected:
#define GENERATE_STRING(id, msg) msg,
    static constexpr const char* const _controls[] = {
        CONTROL_LIST(GENERATE_STRING)};
#undef GENERATE_STRING
#undef CONTROL_LIST
};

#endif  // M_S_DUMMY
