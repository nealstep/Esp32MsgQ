#include "preferences.hpp"

#ifdef ARDUINO
#include <Preferences.h>
#endif  // ARDUINO

bool Prefs::get_pref_b(Prf prf) { return false; }

uint8_t Prefs::get_pref_u8(Prf prf) { return 0; }

uint32_t Prefs::get_pref_u32(Prf prf) { return 0; }

const char* Prefs::get_pref_s(Prf prf) { return ""; }

bool Prefs::set_pref_b(Prf prf, bool val) { return false; }

bool Prefs::set_pref_u8(Prf prf, uint8_t val) { return false; }

bool Prefs::set_pref_u32(Prf prf, uint32_t val) { return false; }

bool Prefs::set_pref_s(Prf prf, const char* val) { return false; }
