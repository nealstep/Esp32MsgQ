#pragma once

#include "global.hpp"
#include "queueable/datum.hpp"
#include "queueable/error.hpp"
#include "queueable/messages.hpp"
#include "queueable/readings.hpp"

#define OUTPUT_LIST(X) \
    X(Rdg, '@')        \
    X(Log, '*')        \
    X(Erm, '!')        \
    X(Dat, '$')        \
    X(Msg, '#')

void die(void);

class Output {
   public:
#define GENERATE_ENUM(id, msg) id,
    enum Out : uint8_t { OUTPUT_LIST(GENERATE_ENUM) Count };
#undef GENERATE_ENUM

    static constexpr const uint8_t output_size = 128;
    static constexpr const uint8_t timestamp_size = 32;
    static constexpr const uint8_t context_size = 64;
    static constexpr const uint8_t data_size = 64;
    static constexpr const uint8_t chipid_size = 17;
    static constexpr const uint8_t chip_name_size = 32;

    static constexpr const char* const reading_fmt_u = "%c|%s|%s|%u|%s";
    static constexpr const char* const reading_fmt_f = "%c|%s|%s|%f|%s";
    static constexpr const char* const error_fmt = "%c|%s|%s|%s";
    static constexpr const char* const time_fmt = "%Y-%m-%d@%H:%M:%S-%Z";
    static constexpr const char* const data_fmt = "%c|%s|%s";
    static constexpr const char* const data_s_u = "%s|%u|%s";
    static constexpr const char* const data_s_s = "%s|%s|%s";
    static constexpr const char* const msg_fmt = "%c|%s|%s|%s|%s|%s";

    static constexpr const char* const no_time = "NoTime";
    static constexpr const char* const no_name = "NoName";

    // serial parameters
    bool use_serial = false;
    bool serial_rdy = false;
    uint32_t serial_speed;

// chip info
#ifdef ARDUINO_ARCH_ESP32
    uint64_t chipid = ESP.getEfuseMac();
#endif  // ARDUINO_ARCH_ESP32
#ifndef ARDUINO
    uint64_t chipid = CHIP_ID;
#endif  // !ARDUINO
    char chipid_s[chipid_size];
    char chip_name[chip_name_size];

    // message pseudo index
    uint32_t msgid = 0;

    // lazy singleton
    static Output& getInstance(void) {
        static Output instance;
        return instance;
    }
    Output(const Output&) = delete;
    Output& operator=(const Output&) = delete;

    Error::Err handle(Error::Entry erre);
    Error::Err handle(Datum::Entry data);
    Error::Err handle(Readings::Entry reading);
    Error::Err handle(Messages::Entry mesg);
    void handle(Error::Err err, const char* name, const char* fname, int line) {
        _handle(Error::get_error(err), name, fname, line);
    }
    void handle(Messages::Sec sect, Messages::Sev sev, Messages::Not notice,
                const char* name, const char* fname, int line);

    void handle(Messages::Var var, uint32_t val, bool broadcast = false) {
        char dv_s[data_size];
        int len =
            snprintf(dv_s, sizeof(dv_s), data_s_u, Messages::get_message(var),
                     val, Messages::get_unit(var));
        if (len < 0) {
            print("Error in data handler at snprintf format, Aborting");
            return;
        } else if (len >= sizeof(dv_s))
            print("Error in data handler at snprintf truncated");
        _handle(dv_s, time(NULL), broadcast);
    }

    void handle(Messages::Var var, const char* val, bool broadcast = false) {
        char dv_s[data_size];
        int len =
            snprintf(dv_s, sizeof(dv_s), data_s_s, Messages::get_message(var),
                     val, Messages::get_unit(var));
        if (len < 0) {
            print("Error in data handler at snprintf format, Aborting");
            return;
        } else if (len >= sizeof(dv_s))
            print("Error in data handler at snprintf truncated");
        _handle(dv_s, time(NULL), broadcast);
    }

    void print(const char* str) {
#ifdef ARDUINO
#ifdef SER
        if (serial_rdy) SER.println(str);  // OK
#endif                                     // SER
#else                                      // !ARDUINO
        std::cout << str << std::endl;  // OK
#endif                                     // ARDUINO !ARDIUNO
                                           // TODO: #4 network print
    }

    static constexpr const char get_output(Out out) { return _outputs[out]; }

   protected:
#define GENERATE_STRING(id, msg) msg,
    static constexpr const char _outputs[] = {OUTPUT_LIST(GENERATE_STRING)};
#undef GENERATE_STRING
#undef OUTPUT_LIST

    // hidden creator
    Output(void) {
        // by default all sections and all messages
        messages.all_sect_mask();
        messages.set_sev_lvl(Messages::Sev::Dbg);
        // get unique name for chip
        snprintf(chipid_s, sizeof(chipid_s), "%04X%08X",
                 (uint16_t)(chipid >> 32), (uint32_t)chipid);
// get likely unique message id
#ifdef ARDUINO_ARCH_ESP32
        msgid = esp_random();
#endif                              // ARDUINO_ARCH_ESP32
        if (msgid == 0) msgid = 1;  // reserve msgid 0
    };

    Error::Err _get_timestamp(time_t asof, char* buffer, size_t len);
    void _handle(const char* err_m, const char* name, const char* fname,
                 int line);
    void _handle(const char* data_s, time_t tt, bool broadcast);
};

static Output& output = Output::getInstance();

#define LOG_ED(E, N) output.handle(E, N, __FILE__, __LINE__)
#define LOG_MD(Sc, Sv, M, N) output.handle(Sc, Sv, M, N, __FILE__, __LINE__)
#define DATAD(N, V, B) output.handle(N, V, B)
