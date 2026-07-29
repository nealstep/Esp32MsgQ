#pragma once

#ifndef ARDUINO

#include <arpa/inet.h>

#include "global.hpp"
#include "queueable/error.hpp"
#include "queueable/messages.hpp"

typedef struct in_addr IPAddress;

class Esp32Net {
   public:
    enum class Mesg : uint8_t {
        GotIP,
        TimeSynced,
    };

    QueueT<Mesg> queue;

    // lazy singleton
    static Esp32Net& getInstance(void) {
        static Esp32Net instance;
        return instance;
    }
    Esp32Net(const Esp32Net&) = delete;
    Esp32Net& operator=(const Esp32Net&) = delete;

    Error::Err init(void) {
        if (inet_pton(AF_INET, LOCAL_IP, &_local_ip) != 1) {
            DEBUG("Invalid LOCAL_IP");
            if (inet_pton(AF_INET, "192.168.1.2", &_local_ip) != 1) {
                DEBUG("this shoudld be impossible");
            }
        }
        if (inet_pton(AF_INET, BROAD_ADDR, &_broadcast_addr) != 1) {
            DEBUG("Invalid BROAD_ADDR");
            if (inet_pton(AF_INET, "192.168.1.255", &_local_ip) != 1) {
                DEBUG("this shoudld be impossible");
            }
        }
        char ip_buffer[INET_ADDRSTRLEN];
        if (inet_ntop(AF_INET, &_local_ip, ip_buffer, sizeof(ip_buffer)) !=
            nullptr) {
            LOG_MQ(Messages::Sec::Net, Messages::Sev::Inf, Messages::Not::GotIP,
                   ip_buffer);
        }
        queue.push(Mesg::GotIP);
        return Error::Err::NoErr;
    };
    void set_net_ready() { _net_ready = true; }

    uint32_t get_ip(void) { return ntohl(_local_ip.s_addr); }
    IPAddress get_broadcast(void) { return _broadcast_addr; }
    bool net_ready(void) { return _net_ready; }
    bool have_internet(void) { return _internet_connected; }

    Error::Err check_internet(void) {
        _internet_connected = true;
        LOG_MQ(Messages::Sec::Net, Messages::Sev::Inf, Messages::Not::IntConn,
               "");
        queue.push(Mesg::TimeSynced);
        return Error::Err::NoErr;
    };
    bool is_local(IPAddress x) { return true; }


   protected:
    // flags
    bool _net_ready = false;
    bool _internet_connected = false;
    IPAddress _local_ip;
    IPAddress _broadcast_addr;

    // hiden constructor
    Esp32Net(void) {};
};

static Esp32Net& esp32Net = Esp32Net::getInstance();

#endif  // !ARDUINO
