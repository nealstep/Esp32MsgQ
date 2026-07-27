#include "global.hpp"
#include "modules/m_s_dummy.hpp"
#include "output/output.hpp"
#include "prefs/prefs.hpp"
#include "queueable/datum.hpp"
#include "version.h"

#ifdef ARDUINO
#else
#define CHANCE 1000
#endif  // ARDUINO !ARDUINO

static constexpr const uint16_t startup_delay = 2000;
static constexpr const uint32_t tiny_delay = 5;

static constexpr const uint32_t million = 1000000;
static constexpr const uint32_t thousand = 1000;

#ifdef M_S_DUMMY
M_S_Dummy m_s_dummy(1, 1);
#endif  // M_S_DUMMY

void log_name(void) {
    DATAQ(Messages::Var::ChpNam, output.chip_name, false);
    DATAQ(Messages::Var::ChpID, output.chipid_s, false);
}

void log_version(void) {
    DATAQ(Messages::Var::GitVer, E32M_GIT_VERSION, false);
    DATAQ(Messages::Var::FirmVer, E32M_FIRMWARE_VERSION, false);
    DATAQ(Messages::Var::BuildTime, E32M_DATETIME_VERSION, false);
    DATAQ(Messages::Var::BuildID, BUILD_ID, false);
}

void log_diagnostics(void) {
#ifdef ARDUINO
    DATAQ(Messages::Var::CPUF, ESP.getCpuFreqMHz(), false);
    DATAQ(Messages::Var::FlshF, ESP.getFlashChipSpeed() / million, false);
    DATAQ(Messages::Var::Heap, ESP.getFreeHeap() / thousand, false);
#else
    uint16_t x = 1000;
    DATAQ(Messages::Var::CPUF, x, false);
    uint16_t y = 5;
    DATAQ(Messages::Var::FlshF, y, false);
#endif  // ARDUINO
}

void check_queues(void) {
    Error::Err err;
    Error::Entry erre;
    while (error.queue.pop(erre)) {
        err = output.handle(erre);
        if (err != Error::Err::NoErr) LOG_ED(err, "output erre");
    }

    Datum::Entry data;
    while (datum.queue.pop(data)) {
        err = output.handle(data);
        if (err != Error::Err::NoErr) LOG_ED(err, "output data");
    }

    Readings::Entry reading;
    while (readings.queue.pop(reading)) {
        Error::Err err = output.handle(reading);
        if (err != Error::Err::NoErr) LOG_ED(err, "output reading");
    }

    Messages::Entry mesg;
    while (messages.queue.pop(mesg)) {
        err = output.handle(mesg);
        if (err != Error::Err::NoErr) LOG_ED(err, "output message");
    }
}

void setup() {
    Error::Err err;
    prefs.begin(PREFS_NAME, false);
    err = prefs.get_pref(Prefs::Prf::ChpNam, output.chip_name,
                         sizeof(output.chip_name));
    if (err != Error::Err::NoErr) {
        LOG_EQ(err, "prefs chip_name");
        strlcpy(output.chip_name, Output::no_name, sizeof(output.chip_name));
    }
#ifdef SER
    err = prefs.get_pref(Prefs::Prf::UsSer, output.use_serial);
    if (err != Error::Err::NoErr) {
        LOG_EQ(err, "prefs use_serial");
        output.use_serial = true;
    }
    if (output.use_serial) {
        err = prefs.get_pref(Prefs::Prf::SerSpd, output.serial_speed);
        if (err != Error::Err::NoErr) {
            LOG_EQ(err, "prefs serial_speed");
            output.serial_speed = SERIAL_SPEED;
        }
        SER.begin(SERIAL_SPEED);
        delay(startup_delay);
        output.serial_rdy = true;
    }
#endif  // SER
    // TODO: #24 add a preference to load default severity level and sections
    LOG_MD(Messages::Sec::Main, Messages::Sev::All, Messages::Not::Start,
           "setup");
    DATAQ(Messages::Var::MsgId, output.msgid, false);
    delay(tiny_delay);
    check_queues();

    // display version
    log_name();
    log_version();
    log_diagnostics();
    delay(tiny_delay);
    check_queues();

    // init network
    // TODO: #3 Network init

    // enable scheduler
    // TODO: #6 implement sceduler

    prefs.end();
    delay(tiny_delay);
    LOG_MD(Messages::Sec::Main, Messages::Sev::All, Messages::Not::Started,
           "setup");
    check_queues();
}

void loop() {
#ifdef ARDUINO
// TBD
#else
    // TODO: #7 implement simple timing sceduler native

    if (rand() % CHANCE == 0)
        if (rand() % CHANCE == 0) {
            Error::Err err = m_s_dummy.get_control(M_S_Dummy::Con::S1);
            if (err != Error::Err::NoErr) LOG_ED(err, "m_s_dummy.get_control");
        }
#endif  // ARDUINO !ARDUINO

    check_queues();
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
