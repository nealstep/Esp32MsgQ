#include "netsend.hpp"

#ifdef ARDUINO

#else  // !ARDUINO

#endif  // ARDIONO !ARDUINO

#include "network/network.hpp"
#include "output.hpp"

Error::Err NetSend::init(void) {
    Error::Err err;
    bool def_e;
    uint16_t def_p;
    uint32_t def_a;
    uint32_t addr;

    DEBUG("Not Implemented");
    _brdcst_addr = INADDR_NONE;
    def_p = BRD_PRT;
    err = _get_pref_n(Prefs::Prf::BrdPrt, _brdcst_port, def_p);
    if (err != Error::Err::NoErr) return err;
    def_e = BRD_ENC;
    err = _get_pref_n(Prefs::Prf::BrdEnc, _brdcst_enc, def_e);
    if (err != Error::Err::NoErr) return err;
    IPAddress addr_1(DATA_ADDR);
    def_a = addr_1;
    err = _get_pref_n(Prefs::Prf::DataAddr, addr, def_a);
    if (err != Error::Err::NoErr) return err;
    _data_addr = addr;
    def_p = DATA_PRT;
    err = _get_pref_n(Prefs::Prf::DataPrt, _data_port, def_p);
    if (err != Error::Err::NoErr) return err;
    def_e = DATA_ENC;
    err = _get_pref_n(Prefs::Prf::DataEnc, _data_enc, def_e);
    if (err != Error::Err::NoErr) return err;
    IPAddress addr_2(NMEA_ADDR);
    def_a = addr_2;
    err = _get_pref_n(Prefs::Prf::NMEAAddr, addr, def_a);
    if (err != Error::Err::NoErr) return err;
    _nmea_addr = addr;
    def_p = NMEA_PRT;
    err = _get_pref_n(Prefs::Prf::NMEAPrt, _nmea_port, def_p);
    if (err != Error::Err::NoErr) return err;
    return Error::Err::NoErr;
}

Error::Err NetSend::send_str(const char* str, bool brdcst, bool data,
                             bool nmea) {
    Error::Err err = Error::Err::NoErr;
    bool q_brdcst = false;
    bool q_data = false;
    bool q_nmea = false;
    if (esp32Net.net_ready()) {
        if (brdcst) {
            err = _send_str(str, _brdcst_addr, _brdcst_port, _brdcst_enc, true);
            if (err != Error::Err::NoErr) return err;
        }
        if (data) {
            if ((_data_local) || (esp32Net.have_internet())) {
                err = _send_str(str, _data_addr, _data_port, _data_enc, true);
                if (err != Error::Err::NoErr) return err;
            } else {
                q_data = true;
            }
        }
#ifdef NMEA0183
        if (nmea) {
            if ((_nmea_local) || (esp32Net.have_internet())) {
                err = _send_str(str, _nmea_addr, _nmea_port, false, false);
                if (err != Error::Err::NoErr) return err;
            } else {
                q_nmea = true;
            }
        }
#endif  // NMEA0183
    } else {
        q_brdcst = brdcst;
        q_data = data;
        q_nmea = nmea;
    }
    if (q_brdcst || q_data || q_nmea) {
        err = _queue_str(str, brdcst, data, nmea);
        if (err != Error::Err::NoErr) return err;
    }
    return Error::Err::NoErr;
}

Error::Err NetSend::chk_queue(void) {
    Error::Err err = Error::Err::NoErr;
    if (esp32Net.net_ready()) {
        for (eid_t i = 0; i < _eid_ind; i++) {
            if (!_ent_free[i]) {
                if (_entries[i].brdcst) {
                    err = _send_str(_entries[i].mesg, _brdcst_addr,
                                    _brdcst_port, _brdcst_enc, true);
                    _entries[i].brdcst = false;
                } else if (_entries[i].data) {
                    if ((_data_local) || (esp32Net.have_internet())) {
                        err = _send_str(_entries[i].mesg, _data_addr,
                                        _data_port, _data_enc, true);
                        _entries[i].data = false;
                    }
                } else if (_entries[i].data) {
                    if ((_data_local) || (esp32Net.have_internet())) {
                        err = _send_str(_entries[i].mesg, _nmea_addr,
                                        _nmea_port, false, false);
                        _entries[i].nmea = false;
                    }
                }
            }
            if (!((_entries[i].brdcst) || (_entries[i].nmea) ||
                  (_entries[i].nmea)))
                _free_eid(i);
        }
    }
    return err;
}

void NetSend::_chk_local(void) {
    _data_local = esp32Net.is_local(_data_addr);
#ifdef NMEA0183
    _nmea_local = esp32Net.is_local(_nmea_addr);
#endif  // NMEA0183
}

Error::Err NetSend::_send_str(const char* mesg, IPAddress target, uint16_t port,
                              bool enc, bool add_headers) {
    Error::Err err = Error::Err::NoErr;
    if (add_headers) {
        // add a prefix of chipid and msgid
    }
#ifdef ARDUINO

#else  // !ARDUINO

#endif  // ARDIONO !ARDUINO
    DEBUG("Not Implemented");
    return err;
}

Error::Err NetSend::_queue_str(const char* mesg, bool brdcst, bool data,
                               bool nmea) {
    Error::Err err = Error::Err::NoErr;
    eid_t eid = _get_next_eid();
    Entry* entry = _get_entry(eid);
    entry->brdcst = brdcst;
    entry->data = data;
    entry->nmea = nmea;
    size_t len = strlcpy(entry->mesg, mesg, sizeof(Entry::mesg));
    if (len >= sizeof(Entry::mesg)) {
        err = Error::Err::Trunc;
    }
    return err;
}
