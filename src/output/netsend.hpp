#pragma once

#include "global.hpp"
#include "prefs/prefs.hpp"
#include "output.hpp"

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

    Error::Err init(void) {
        uint32_t addr;
        Error::Err err;
        DEBUG("Not Implemented");
        err = prefs.get_pref(Prefs::Prf::BrdAddr, addr);
        if (err != Error::Err::NoErr) return err;
        // need to eplace with what i have in network
        return Error::Err::NoErr; }

    Error::Err send_str(const char* str, bool brdcst, bool data, bool nmea) {
        DEBUG("Not Implemented");
        // see if we have net
        // if not push the whole thing
        // see if target_ip on local
        // if not see if we have internet
        // if not push tartip into queue

        return Error::Err::NoErr;
    }

    Error::Err chk_queue(void) {
        DEBUG("Not Implemented");
        // check if we have net
        // if we do check if we have internet
        // check queue for items to send
        return Error::Err::NoErr;
    }

    void set_broadcst_data(IPAddress brdcst_addr, int brdcst_port, bool enc) {
        _brdcst_addr = brdcst_addr;
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
    uint8_t _str_ind = 0;

    IPAddress _brdcst_addr;
    int _brdcst_port;
    bool _brdcst_enc;
    IPAddress _data_addr;
    int _data_port;
    bool _data_enc;
    bool _data_local;
#ifdef NMEA0183
    IPAddress _nmea_addr;
    int _nmea_port;
#endif  // NMEA0183

    // hiden constructor
    NetSend(void) {
        for (uint8_t ind = 0; ind < ent_max; ind++) _ent_free[ind] = true;
    };

    eid_t _get_next_str() {
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

    Entry* _get_str(eid_t ind) {
        if (_ent_free[ind]) {
            return nullptr;
        } else if (ind >= ent_max) {
            // invalid
        }
        return &_entries[ind];
    }

    void _free_str(eid_t ind) { _ent_free[ind] = true; }
};

static NetSend& netSend = NetSend::getInstance();
