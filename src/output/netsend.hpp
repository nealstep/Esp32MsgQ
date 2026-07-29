#pragma once

#include "global.hpp"
#include "output/output.hpp"
#include "prefs/prefs.hpp"
#include "queueable/error.hpp"

typedef uint8_t eid_t;

class NetSend {
   public:
    static constexpr const eid_t header_size = 24;
    static constexpr const eid_t str_size = Output::output_size + header_size;

    static constexpr const eid_t ent_max = 32;
    static constexpr const eid_t ent_exhausted = 0xFF;

    typedef struct {
        char mesg[str_size];
        bool brdcst;
        bool data;
        bool nmea;
    } Entry;

    // lazy singleton
    static NetSend& getInstance(void) {
        static NetSend instance;
        return instance;
    }
    NetSend(const NetSend&) = delete;
    NetSend& operator=(const NetSend&) = delete;

    Error::Err init(void);
    Error::Err send_str(const char* str, bool brdcst, bool data,
                        bool nmea = false);
    Error::Err chk_queue(void);

    void set_broadcast_addr(IPAddress brd_addr) {
        _brdcst_addr = brd_addr;
        _chk_local();
    }

    void set_broadcst_data(int brdcst_port, bool enc) {
        _brdcst_port = brdcst_port;
        _brdcst_enc = enc;
    }

    void set_data_data(IPAddress data_addr, int data_port, bool enc) {
        _data_addr = data_addr;
        _data_port = data_port;
        _data_enc = enc;
        // check if _addr is local
    }

#ifdef NMEA0183
    void set_nmea_data(IPAddress nmea_addr, int nmea_port) {
        _nmea_addr = nmea_addr;
        _nmea_port = nmea_port;
    }
#endif  // NMEA0183

    IPAddress get_brdcst_data(int& brdcst_port, bool& enc) {
        brdcst_port = _brdcst_port;
        enc = _brdcst_enc;
        return _brdcst_addr;
    }

    IPAddress get_data_data(int& data_port, bool& enc) {
        data_port = _data_port;
        enc = _data_enc;
        return _data_addr;
    }

#ifdef NMEA0183
    IPAddress get_nmea_data(int& nmea_port) {
        nmea_port = _nmea_port;
        return _nmea_addr;
    }
#endif  // NMEA0183

   protected:
    Entry _entries[ent_max];
    bool _ent_free[ent_max];
    eid_t _eid_ind = 0;

    IPAddress _brdcst_addr;
    uint16_t _brdcst_port;
    bool _brdcst_enc;
    IPAddress _data_addr;
    uint16_t _data_port;
    bool _data_enc;
    bool _data_local;
#ifdef NMEA0183
    IPAddress _nmea_addr;
    uint16_t _nmea_port;
    bool _nmea_local;
#endif  // NMEA0183

    // hiden constructor
    NetSend(void) {
        for (uint8_t ind = 0; ind < ent_max; ind++) _ent_free[ind] = true;
    };

    void _chk_local(void);
    Error::Err _send_str(const char* mesg, IPAddress target, uint16_t port,
                         bool enc, bool add_headers);
    Error::Err _queue_str(const char* mesg, bool brdcst, bool data,
                          bool nmea = false);

    eid_t _get_next_eid() {
        eid_t ind = 0;
        while (ind < ent_max) {
            if (_ent_free[ind]) {
                _ent_free[ind] = false;
                return ind;
            }
            ind++;
        }
        return ent_exhausted;
    }

    Entry* _get_entry(eid_t ind) {
        if (_ent_free[ind]) {
            return nullptr;
        } else if (ind >= ent_max) {
            // invalid
        }
        return &_entries[ind];
    }

    void _free_eid(eid_t ind) { _ent_free[ind] = true; }

    template <typename T>
    Error::Err _get_pref_n(Prefs::Prf prf, T& val, T def) {
        Error::Err err = prefs.get_pref(prf, val);
        if (err != Error::Err::NoErr) {
            LOG_EQ(err, "_get_pref_s get");
            val = def;
            LOG_MQ(Messages::Sec::Net, Messages::Sev::Wrn,
                   Messages::Not::PrefNotFnd, Prefs::get_key(prf));
            err = Error::Err::NoErr;
        }
        return err;
    }
};

static NetSend& netSend = NetSend::getInstance();

Error::Err send_nmea(const char *nmea_str) {
    return netSend.send_str(nmea_str, false, false, true);
}