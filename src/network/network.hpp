#include "global.hpp"

#ifdef ARDUINO

#include "prefs/prefs.hpp"
#include "queue_t.hpp"
#include "queueable/error.hpp"

class Esp32Net {
   public:
    static constexpr const uint8_t ssid_size = 32 + 1;
    static constexpr const uint8_t passwd_size = 64 + 1;
    static constexpr const uint8_t number_size = 15 + 1;
    static constexpr const uint8_t tz_size = 32 + 1;
    static constexpr const uint8_t fqdn_size = 128 + 1;

    static constexpr const uint32_t long_delay = 1000;
    static constexpr uint32_t medium_delay = 500;
    static constexpr const uint8_t short_delay = 100;

    static constexpr uint8_t time_sync_attempts = 4;

    static constexpr const uint8_t good_http_code = 204;

    static constexpr const char* const int_fmt = "%d";

    static constexpr const char* const internet_check_url =
        "http://clients3.google.com/generate_204";

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

    Error::Err init(void);
    void set_net_ready() { _net_ready = true; }

    IPAddress get_ip(void) { return _local_ip; }
    IPAddress get_broadcast(void) { return _broadcast_addr; }
    bool net_ready(void) { return _net_ready; }
    bool have_internet(void) { return _internet_connected; }

    Error::Err check_internet(void);
    Error::Err check_clock(void);
    void check_ota(void);

    Error::Err update_ssid(const char* ssid) {
        Error::Err err = Error::Err::NoErr;
        size_t len = strlcpy(_ssid, ssid, sizeof(_ssid));
        if (len >= sizeof(_ssid)) {
            err = Error::Err::Ovr;
            LOG_EQ(err, "update ssid");
        }
        // needs a reconnect done via command
        return err;
    }
    Error::Err update_password(const char* password) {
        Error::Err err = Error::Err::NoErr;
        size_t len = strlcpy(_password, password, sizeof(_password));
        if (len >= sizeof(_password)) {
            err = Error::Err::Ovr;
            LOG_EQ(err, "update password");
        }
        // needs a reconnect done via command
        return err;
    }
    Error::Err update_ota_password(const char* ota_password);

    void got_ip(IPAddress lip);
    void disconnected(void);
    void reconnect(void);

    void push_mesg(Mesg mesg) { queue.push(mesg); }

   protected:
    // flags
    bool _net_ready = false;
    bool _internet_connected = false;
    bool _ota_ready = false;

    char _ssid[ssid_size];
    char _password[passwd_size];
    char _ota_password[passwd_size];
    char _tz_full[tz_size];
    char _ntp_server_1[fqdn_size];
    char _ntp_server_2[fqdn_size];
    char _ntp_server_3[fqdn_size];

    // adresses and masks
    IPAddress _local_ip = INADDR_NONE;
    uint32_t _subnet_mask = 0;
    uint32_t _subnet_addr = 0;
    uint32_t _broadcast_addr = 0;

    // hiden constructor
    Esp32Net(void) {};

    void _set_subnet_mask(IPAddress smask) {
        _subnet_mask = smask;
        _subnet_addr = (uint32_t)_local_ip & _subnet_mask;
        _broadcast_addr = (uint32_t)_local_ip | ~_subnet_mask;
    }
    void _ota_init(void);
    Error::Err _get_pref_s(Prefs::Prf prf, char* buf, size_t buf_len,
                           const char* def);
};

static Esp32Net& esp32Net = Esp32Net::getInstance();

#endif // ARDUINO
