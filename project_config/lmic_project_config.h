// project-specific definitions

// Region: AS923 only -- this device's actual deployment region.
#define CFG_as923 1
// #define LMIC_COUNTRY_CODE LMIC_COUNTRY_CODE_JP      /* only if deploying in Japan specifically */

// Radio: SX1262 -- the RA62 module on the HT-VMT190's onboard radio.
#define CFG_sx1262_radio 1

// This project is Class A/C only, never Class B, so ping-slot and
// beacon-tracking support (code + debug strings for both) is unused.
#define DISABLE_PING 1
#define DISABLE_BEACONS 1

// Strips LMIC's own internal debug printf strings (separate from this
// project's own onEvent() logging, which is already trimmed).
#define LMIC_DEBUG_LEVEL 3
