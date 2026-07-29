#include "modules.hpp"

#include "module.hpp"

#ifdef NMEA0183
void Modules::check_nmea(Error::Err (*sendNMEA)(const char*)) {
    for (modules_t i = 0; i < _nmea_ind; i++) {
        _modules[_nmea[i]]->get_nmea(sendNMEA);
    }
}
#endif  // NMEA0183