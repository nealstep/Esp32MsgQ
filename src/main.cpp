#include "global.hpp"
#include "modules/m_s_dummy.hpp"
#include "network/network.hpp"
#include "output/netsend.hpp"
#include "output/output.hpp"
#include "prefs/prefs.hpp"
#include "queueable/datum.hpp"
#include "version.h"

#ifdef ARDUINO
#include <TaskScheduler.h>
#ifdef IS_M5
#include <M5Unified.h>
#endif  // IS_M5
#endif  // ARDUINO

static constexpr const uint8_t number_size = 16;

static constexpr const uint16_t startup_delay = 2000;
static constexpr const uint8_t tiny_delay = 5;

#ifdef ARDUINO
static constexpr const uint32_t loop_interval = 10000 - 1;
#else   // !ARDUINO
static constexpr const uint32_t loop_interval = 10000000 - 1;
#endif  // ARDUINO !ARDUINO

static constexpr const uint32_t million = 1000000;
static constexpr const uint32_t thousand = 1000;

static constexpr const char* const unsigned_fmt = "%u";

uint32_t loop_counter = 0;

#ifdef M_S_DUMMY
M_S_Dummy m_s_dummy_1_1(1, 1);
#endif  // M_S_DUMMY

// task wrappers
void keep_alive_msg(void) {
    LOG_MD(Messages::Sec::Main, Messages::Sev::Inf, Messages::Not::KpAlive, "");
}

#ifdef M_S_DUMMY
void m_s_dummy_chk(void) {
    Error::Err err = m_s_dummy_1_1.get_control(M_S_Dummy::Con::S1);
    if (err != Error::Err::NoErr) LOG_ED(err, "m_s_dummy.get_control");
}
#endif  // M_S_DUMMY

#ifdef NMEA0183
void nmea_chk(void) { modules.check_nmea(send_nmea); };
#endif  // NMEA0183

#ifdef ARDUINO

void checkInternet(void) { esp32Net.check_internet(); }

// defaults
static constexpr const uint64_t keep_alive_int = 5000UL;
static constexpr const uint64_t check_internet_int = 30000UL;
#ifdef NMEA0183
static constexpr const uint64_t nmea0183_int = 1000UL;
#endif  // NMEA0183

// scheduler
Scheduler runner;

// create tasks
Task taskSendKeepAliveMsg(keep_alive_int, TASK_FOREVER, &keep_alive_msg);
Task taskCheckInternet(check_internet_int, TASK_FOREVER, &checkInternet);

#ifdef M_S_DUMMY
Task taskMSDummy_1_1(m_s_dummy_1_1.interval, TASK_FOREVER, &m_s_dummy_chk);
#endif  // M_S_DUMMY

#ifdef NMEA0183
Task taskNMEA0183(nmea0183_int, TASK_FOREVER, &nmea_chk);
#endif  // NMEA0183

// array of tasks to be added to the scheduler
Task* tasks[] = {
    &taskSendKeepAliveMsg,
#ifdef M_S_DUMMY
    &taskMSDummy_1_1,
#endif  // M_S_DUMMY
#ifdef NMEA0183
    &taskNMEA0183,
#endif  // NMEA0183
        //  &taskCheckInternet
};
#else  // !ARDUINO
static constexpr const time_t keep_alive_int = 5;
time_t keep_alive_last = 0;
static constexpr const time_t m_s_dummy_1_1_int = 15;
time_t m_s_dummy_1_1_last = 0;
#ifdef NMEA0183
static constexpr const time_t nmea0183_int = 1;
time_t nmea0183_last = 0;
#endif  // NMEA0183
#endif  // ARDUINO !ARDUINO

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

    Esp32Net::Mesg msg;
    while (esp32Net.queue.pop(msg)) {
        switch (msg) {
            case Esp32Net::Mesg::GotIP:
                DATAD(Messages::Var::IP, esp32Net.get_ip(), true);
                esp32Net.set_net_ready();
                esp32Net.check_internet();
                netSend.set_broadcast_addr(esp32Net.get_broadcast());
                break;
            case Esp32Net::Mesg::TimeSynced:
                DATAQ(Messages::Var::Time, time(NULL), false);
#ifdef ARDUINO
                if (HAVE_RTC) {
                    struct tm timeinfo;
                    if (getLocalTime(&timeinfo)) {
#ifdef IS_M5
                        M5.Rtc.setDateTime(&timeinfo);
#endif  // IS_M5
                    }
                }
#endif  // ARDUINO
                break;
            default:
                DEBUG("This should not happen");
                break;
        }
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
    DATAQ(Messages::Var::Time, time(NULL), false);
    delay(tiny_delay);
    check_queues();

#ifdef IS_M5
    auto cfg = M5.config();
    M5.begin(cfg);
#endif  // IS_M5

    // display version
    log_name();
    log_version();
    log_diagnostics();
    delay(tiny_delay);
    check_queues();

    err = esp32Net.init();
    check_queues();
    if (err != Error::Err::NoErr) {
        LOG_ED(err, "net init");
        die();
    }

    err = netSend.init();
    check_queues();
    if (err != Error::Err::NoErr) {
        LOG_ED(err, "net send init");
        die();
    }

#ifdef ARDUINO
    delay(tiny_delay);
    for (auto& task : tasks) {
        runner.addTask(*task);
        task->enable();
    }
#endif  // ARDUINO

    prefs.end();
    delay(tiny_delay);
    LOG_MD(Messages::Sec::Main, Messages::Sev::All, Messages::Not::Started,
           "setup");
    check_queues();
}

#ifdef IS_M5
// M5 updates, button handling etc may also go here
void updateM5(void) { M5.update(); }
#endif  // IS_M5

#ifdef SER
void serial_inp(void) {
    // TODO: #25 Serial input
}
#endif  // SER

void check_cmds(void) {
    // TODO: #26 check for commands
}

void loop() {
    if (++loop_counter > loop_interval) {
        char num_s[number_size];
        size_t len = snprintf(num_s, sizeof(num_s), unsigned_fmt, loop_counter);
        if ((len > 0) && (len <= sizeof(num_s)))
            LOG_MD(Messages::Sec::Main, Messages::Sev::Inf,
                   Messages::Not::LoopN, num_s);
        else
            LOG_ED(Error::Err::Form, "loop counter display");
        loop_counter = 0;
    }

#ifdef SER
    serial_inp();
#endif  // SER

#ifdef ARDUINO
    // check if jobs need running
    runner.execute();
#else
    time_t now = time(NULL);
    if (now != keep_alive_last)
        if ((now % keep_alive_int) == 0) {
            keep_alive_last = now;
            keep_alive_msg();
        }
    if (now != m_s_dummy_1_1_last)
        if ((now % m_s_dummy_1_1_int) == 0) {
            m_s_dummy_1_1_last = now;
            m_s_dummy_chk();
        }
#ifdef NMEA0183
    if (now != nmea0183_last)
        if ((now % nmea0183_int) == 0) {
            nmea0183_last = now;
            nmea0183_chk();
        }
#endif  // NMEA0183
#endif  // ARDUINO !ARDUINO

#ifdef IS_M5
    updateM5();
#endif  // IS_M5

    check_cmds();

    check_queues();

    delay(tiny_delay);
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
