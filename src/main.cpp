#include "global.hpp"
#include "modules/m_s_dummy.hpp"
#include "output/output.hpp"
#include "queuer.hpp"

#ifdef ARDUINO
#else
#define CHANCE 1000
#endif  // ARDUINO !ARDUINO

static constexpr const uint16_t startup_delay = 2000;

#ifdef M_S_DUMMY
M_S_Dummy m_s_dummy(1, 1);
#endif  // M_S_DUMMY

void setup() {
#ifdef SER
    SER.begin(SERIAL_SPEED);
    delay(startup_delay);
#endif  // SER
    output.print(m_s_dummy.get_name());
    output.print("Started");
    LOG_E(Output::Err::NoMod, Output::Context("test"));
    LOG_E(Module::Err::NotEn, Output::Context("test"));
}

void loop() {
#ifdef ARDUINO
// TBD
#else
    if (rand() % CHANCE == 0)
        if (rand() % CHANCE == 0) {
            Module::Err err = m_s_dummy.get_control(M_S_Dummy::Con::S1);
            if (err != Module::Err::NoErr)
                LOG_E(err, Output::Context("m_s_dummy.get_control"));
        }
#endif  // ARDUINO !ARDUINO

    Modules::Reading reading;
    while (queuer.pop(Queuer::Queues::Rdgs, reading)) {
        Output::Err err = output.handle(reading);
        if (err != Output::Err::NoErr)
            LOG_E(err, Output::Context("m_s_dummy.get_control"));
    }
}

#ifndef ARDUINO
#ifndef PIO_UNIT_TESTING

int main(void) {
    setup();
    while (true) loop();
    return 0;
}

#endif  // PIO_UNIT_TESTING
#endif  // !ARDUINO
