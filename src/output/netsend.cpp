#pragma once

#include "global.hpp"
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
        bool enc_brdcst;
        IPAddress ip;
        int port;
        bool enc_target;
    } Entry;

    // lazy singleton
    static NetSend& getInstance(void) {
        static NetSend instance;
        return instance;
    }
    NetSend(const NetSend&) = delete;
    NetSend& operator=(const NetSend&) = delete;

    Error::Err send_str(const char* str, bool brdcst, bool enc_brdcst,
                        IPAddress target_ip, int target_port, bool enc_target) {
        // see if we have net
        // if not push the whole thing
        // see if target_ip on local
        // if not see if we have internet
        // if not push tartip into queue

        return Error::Err::NoErr;
    }

    Error::Err chk_queue(void) {
        // check if we have net
        // if we do check if we have internet
        // check queue for items to send
    }

    eid_t get_next_str() {
        sid_t ind = 0;
        while (ind < ent_max) {
            if (_ent_free[ind]) {
                _ent_free[ind] = false;
                return ind;
            }
            ind++;
        }
        return ent_exhausted;
    }

    Entry* get_str(eid_t ind) {
        if (_ent_free[ind]) {
            return nullptr;
        } else if (ind >= ent_max) {
            // invalid
        }
        return &_entries[ind];
    }

    void free_str(eid_t ind) { _ent_free[ind] = true; }

   protected:
    Entry _entries[ent_max];
    bool _ent_free[ent_max];
    uint8_t _str_ind = 0;

    // hiden constructor
    NetSend(void) {
        for (uint8_t ind = 0; ind < ent_max; ind++) _ent_free[ind] = true;
    };
};

static NetSend& netSend = NetSend::getInstance();
