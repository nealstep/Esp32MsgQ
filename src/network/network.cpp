#include "network.hpp"

#ifdef ARDUINO
#include <ArduinoOTA.h>
#include <HTTPClient.h>
#include <WiFi.h>

#include "queueable/messages.hpp"

void wifi_connected(WiFiEvent_t event, WiFiEventInfo_t info) {
    LOG_MQ(Messages::Sec::Net, Messages::Sev::Inf, Messages::Not::Connectd, "");
}

// got ip event add to queue to handle in main loop
void wifi_got_ip(WiFiEvent_t event, WiFiEventInfo_t info) {
    IPAddress lip = WiFi.localIP();
    LOG_MQ(Messages::Sec::Net, Messages::Sev::Inf, Messages::Not::GotIP,
           lip.toString().c_str());
    esp32Net.got_ip(lip);
}

// disconnected event add to queue to handle in main loop and attempt reconnect
void wifi_disconnected(WiFiEvent_t event, WiFiEventInfo_t info) {
    LOG_MQ(Messages::Sec::Net, Messages::Sev::Inf, Messages::Not::Disconnected,
           "");
    esp32Net.disconnected();
}

Error::Err Esp32Net::init(void) {
    Error::Err err;
    LOG_MQ(Messages::Sec::Net, Messages::Sev::Inf, Messages::Not::NetInit, "");
    // load required prefs
    err = _get_pref_s(Prefs::Prf::SSID, _ssid, sizeof(_ssid), WIFI_SSID);
    if (err != Error::Err::NoErr) return err;
    err =
        _get_pref_s(Prefs::Prf::PASW, _password, sizeof(_ssid), WIFI_PASSWORD);
    if (err != Error::Err::NoErr) return err;
    err = _get_pref_s(Prefs::Prf::OTAPASS, _ota_password, sizeof(_ssid),
                      OTA_PASSWORD);
    if (err != Error::Err::NoErr) return err;
    err = _get_pref_s(Prefs::Prf::TZF, _tz_full, sizeof(_tz_full), TZ_FULL);
    if (err != Error::Err::NoErr) return err;
    err = _get_pref_s(Prefs::Prf::NTP1, _ntp_server_1, sizeof(_ntp_server_1),
                      NTP_1);
    if (err != Error::Err::NoErr) return err;
    err = _get_pref_s(Prefs::Prf::NTP2, _ntp_server_2, sizeof(_ntp_server_2),
                      NTP_2);
    if (err != Error::Err::NoErr) return err;
    err = _get_pref_s(Prefs::Prf::NTP3, _ntp_server_3, sizeof(_ntp_server_3),
                      NTP_3);
    if (err != Error::Err::NoErr) return err;
    // turn it on
    WiFi.onEvent(wifi_connected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_CONNECTED);
    WiFi.onEvent(wifi_got_ip, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
    WiFi.onEvent(wifi_disconnected,
                 WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
    WiFi.begin(_ssid, _password);
    return err;
}

Error::Err Esp32Net::check_internet(void) {
    Error::Err err = Error::Err::NoErr;
    LOG_MQ(Messages::Sec::Net, Messages::Sev::Inf, Messages::Not::ChkInt, "");
    if (net_ready()) {
        HTTPClient http;
        http.setTimeout(long_delay);
        http.begin(internet_check_url);
        int http_code = http.GET();
        http.end();
        char num_s[number_size];
        size_t len = snprintf(num_s, sizeof(num_s), int_fmt, http_code);
        if ((len < 0) && (len > sizeof(num_s))) {
            err = Error::Err::Form;
            LOG_EQ(err, "http code display");
            return err;
        }
        if (http_code == good_http_code) {
            if (!_internet_connected) {
                LOG_MQ(Messages::Sec::Net, Messages::Sev::Inf,
                       Messages::Not::IntConn, "");
                _internet_connected = true;
            }
            err = check_clock();
        } else {
            if (_internet_connected) {
                err = Error::Err::NoInt;
                LOG_EQ(err, "check_internet");
                _internet_connected = false;
            }
        }
    } else {
        err = Error::Err::NoNet;
    }
    return err;
}

Error::Err Esp32Net::check_clock(void) {
    Error::Err err = Error::Err::NoErr;
    LOG_MQ(Messages::Sec::Net, Messages::Sev::Inf, Messages::Not::ChkClk, "");
    if (!_net_ready) return Error::Err::NoNet;
    configTzTime(_tz_full, _ntp_server_1, _ntp_server_2, _ntp_server_3);
    struct tm timeinfo;
    uint8_t attempts = 0;
    while (!getLocalTime(&timeinfo)) {
        delay(medium_delay);
        attempts++;
        if (attempts > time_sync_attempts) {
            err = Error::Err::TimeSyncFail;
            LOG_EQ(err, "check_clock");
            break;
        }
    }
    if (err == Error::Err::NoErr)
        queue.push(Esp32Net::Mesg::TimeSynced);
    return err;
}

void Esp32Net::check_ota(void) {
    if (_ota_ready) {
        ArduinoOTA.handle();
    }
}

void Esp32Net::got_ip(IPAddress lip) {
    _local_ip = lip;
    _set_subnet_mask(WiFi.subnetMask());
    queue.push(Esp32Net::Mesg::GotIP);
    _ota_init();
}

void Esp32Net::disconnected(void) {
    LOG_EQ(Error::Err::NoNet, "disconnected");
    _net_ready = false;
    _internet_connected = false;
    _local_ip = INADDR_NONE;
    _broadcast_addr = 0;
    WiFi.begin(_ssid, _password);
}

void Esp32Net::reconnect(void) { WiFi.disconnect(false, true); }

Error::Err Esp32Net::update_ota_password(const char* ota_password) {
    Error::Err err = Error::Err::NoErr;
    size_t len = strlcpy(_ota_password, ota_password, sizeof(_ota_password));
    if (len >= sizeof(_ota_password)) {
        err = Error::Err::Ovr;
        LOG_EQ(err, "update ota password");
    }
    ArduinoOTA.end();
    _ota_ready = false;
    delay(short_delay);
    _ota_init();
    return err;
}

void Esp32Net::_ota_init() {
    if (!_ota_ready) {
        ArduinoOTA.setPassword(_ota_password);
        ArduinoOTA.begin();
        _ota_ready = true;
    }
}

Error::Err Esp32Net::_get_pref_s(Prefs::Prf prf, char* buf, size_t buf_len,
                                 const char* def) {
    Error::Err err = prefs.get_pref(prf, buf, buf_len);
    if (err != Error::Err::NoErr) {
        LOG_EQ(err, "_get_pref_s get");
        size_t len = strlcpy(buf, def, buf_len);
        if (len > buf_len) {
            err = Error::Err::Trunc;
            LOG_EQ(err, "_get_pref_s def");
        } else {
            LOG_MQ(Messages::Sec::Net, Messages::Sev::Wrn,
                   Messages::Not::PrefNotFnd, Prefs::get_key(prf));
            err = Error::Err::NoErr;
        }
    }
    return err;
}

#endif  // ARDUINO
