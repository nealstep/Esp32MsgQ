#include "global.hpp"
#include "modules/m_s_dummy.hpp"
#include "output/output.hpp"
#include "prefs/prefs.hpp"
#include "queuer/queuer.hpp"
#include "version.h"

#ifdef ARDUINO
#else
#define CHANCE 1000
#endif  // ARDUINO !ARDUINO

static constexpr const uint16_t startup_delay = 2000;
static constexpr const uint32_t tiny_delay = 5;

static constexpr const uint32_t million = 1000000;

#ifdef M_S_DUMMY
M_S_Dummy m_s_dummy(1, 1);
#endif  // M_S_DUMMY

void log_name(void) {
    // TODO: #2 log_name
}

void log_version(void) {
    DATAD(Messages::Var::GitVer, E32M_GIT_VERSION, false);
    DATAD(Messages::Var::FirmVer, E32M_FIRMWARE_VERSION, false);
    DATAD(Messages::Var::BuildTime, E32M_DATETIME_VERSION, false);
    DATAD(Messages::Var::BuildID, BUILD_ID, false);
}

void log_diagnostics(void) {
#ifdef ARDUINO
    DATAD(Messages::Var::CPUF, ESP.getCpuFreqMHz(), false);
    DATAD(Messages::Var::FlshF, ESP.getFlashChipSpeed() / million, false);
    DATAD(Messages::Var::Heap, ESP.getFreeHeap(), false);
#else
    uint16_t x = 1000;
    DATAD(Messages::Var::CPUF, x, false);
    uint16_t y = 5;
    DATAD(Messages::Var::FlshF, y, false);
#endif  // ARDUINO
}

void setup() {
    prefs.begin(PREFS_NAME, false);
#ifdef SER
    Prefs::Err err = prefs.get_pref(Prefs::Prf::UsSer, output.use_serial);
    if (err != Prefs::Err::NoErr) {
        LOG_EQ(err, Output::Context("prefs use_serial"));
        output.use_serial = true;
    }
    if (output.use_serial) {
        Prefs::Err err =
            prefs.get_pref(Prefs::Prf::SerSpd, output.serial_speed);
        if (err != Prefs::Err::NoErr) {
            LOG_EQ(err, Output::Context("prefs serial_speed"));
            output.serial_speed = SERIAL_SPEED;
        }
        SER.begin(SERIAL_SPEED);
        delay(startup_delay);
        output.serial_rdy = true;
    }
#endif  // SER
    LOG_ND(Messages::Uni::Main, Messages::Sev::All, Messages::Not::Start,
           Output::Context("setup"));
    DATAD(Messages::Var::MsgId, output.msgid, false);

    // display version
    delay(tiny_delay);
    log_name();
    log_version();
    log_diagnostics();

    // init network
    // TODO: #3 Network init

    // enable scheduler
    // TODO: #6 implement sceduler

    prefs.end();
    delay(tiny_delay);
    LOG_ND(Messages::Uni::Main, Messages::Sev::All, Messages::Not::Started,
           Output::Context("setup"));
    // LOG_ED(Output::Err::NoMod, Output::Context("test"));
    // LOG_ED(Module::Err::NotEn, Output::Context("test"));
}

void loop() {
#ifdef ARDUINO
// TBD
#else
    // TODO: #7 implement simple timing sceduler native

    if (rand() % CHANCE == 0)
        if (rand() % CHANCE == 0) {
            Module::Err err = m_s_dummy.get_control(M_S_Dummy::Con::S1);
            if (err != Module::Err::NoErr)
                LOG_ED(err, Output::Context("m_s_dummy.get_control"));
        }
#endif  // ARDUINO !ARDUINO

    Modules::Reading reading;
    while (queuer.pop(Queuer::Queues::Rdgs, reading)) {
        Output::Err err = output.handle(reading);
        if (err != Output::Err::NoErr)
            LOG_ED(err, Output::Context("m_s_dummy.get_control"));
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
