/*******************************************************************************
 * Copyright (c) 2015 Thomas Telkamp and Matthijs Kooijman
 * Copyright (c) 2018 Terry Moore, MCCI
 *
 * Permission is hereby granted, free of charge, to anyone
 * obtaining a copy of this document and accompanying files,
 * to do whatever they want with them without any restriction,
 * including, but not limited to, copying, modification and redistribution.
 * NO WARRANTY OF ANY KIND IS PROVIDED.
 *
 * This example sends a valid LoRaWAN packet with payload "Hello,
 * world!", using frequency and encryption settings matching those of
 * the The Things Network.
 *
 * REGIONAL CONFIGURATION:
 *
 * Network Server : Actility ThingPark
 * Region         : AS923-3
 * Country        : Philippines
 * Device Class   : Class A
 * Activation     : OTAA
 *
 * AS923-3 Default Channels:
 *
 *   Channel 0: 916.6 MHz
 *   Channel 1: 916.8 MHz
 *
 * Additional channels may be configured by ThingPark
 * through LoRaWAN MAC commands.
 *
 * IMPORTANT:
 *
 * - Configure LMIC for AS923 with the AS923-3 frequency offset.
 * - Ensure the ThingPark device profile uses AS923-3.
 * - Verify that the gateway supports these frequencies.
 * - Use the same DevEUI, JoinEUI/AppEUI, and AppKey
 *   registered in ThingPark.
 *
 * Pin mapping and advanced HalConfiguration below are for the Heltec
 * HT-VMT190 (Vision Master T190), which uses an SX1262 (RA62 module)
 * wired to GPIO8/9/10/11/12/13/14.
 *
 *******************************************************************************/

#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>

#ifdef COMPILE_REGRESSION_TEST
# define FILLMEIN 0
#else
# warning "You must replace the values marked FILLMEIN with real values from the TTN control panel!"
# define FILLMEIN (#dont edit this, edit the lines that use FILLMEIN)
#endif


/*
 * ThingPark OTAA Configuration
 * Region: AS923-3 (Philippines)
 *
 * APPEUI / JoinEUI : Little-endian (reversed)
 * DEVEUI           : Little-endian (reversed)
 * APPKEY           : Big-endian (normal order)
 */

// JoinEUI / AppEUI
static const u1_t PROGMEM APPEUI[8] = {
    0x42, 0x94, 0x68, 0x03,
    0x52, 0x42, 0x39, 0x58
};

// Device EUI
static const u1_t PROGMEM DEVEUI[8] = {
    0x43, 0x94, 0x68, 0x03,
    0x52, 0x42, 0x39, 0x58
};

// Application Key
static const u1_t PROGMEM APPKEY[16] = {
    0x58, 0x39, 0x42, 0x52,
    0x03, 0x68, 0x94, 0x43,
    0x58, 0x39, 0x42, 0x52,
    0x03, 0x68, 0x94, 0x43
};


void os_getArtEui (u1_t* buf) { memcpy_P(buf, APPEUI, 8);}
void os_getDevEui (u1_t* buf) { memcpy_P(buf, DEVEUI, 8);}
void os_getDevKey (u1_t* buf) {  memcpy_P(buf, APPKEY, 16);}

static uint8_t mydata[] = "Hello, world!";
static osjob_t sendjob;

const unsigned TX_INTERVAL = 60;

// Advanced HalConfiguration - required for all SX126x-series modems.
// HT-VMT190 uses the RA62 (SX1262) module.
class cHalConfiguration_t: public Arduino_LMIC::HalConfiguration_t
{
public:
    virtual u1_t queryBusyPin(void) override { return 13; };
    virtual bool queryUsingDcdc(void) override { return true; };
    virtual bool queryUsingDIO2AsRfSwitch(void) override { return true; };
    virtual bool queryUsingDIO3AsTCXOSwitch(void) override { return true; };
};

cHalConfiguration_t myConfig;

// Pin mapping (HT-VMT190: NSS=8, SCK=9, MOSI=10, MISO=11, RST=12, BUSY=13, DIO1=14)
const lmic_pinmap lmic_pins = {
    .nss = 8,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = 12,
    .dio = {14, LMIC_UNUSED_PIN, LMIC_UNUSED_PIN},
    .rxtx_rx_active = 0,
    .rssi_cal = 10,
    .spi_freq = 8000000,
    .pConfig = &myConfig,
};

void printHex2(unsigned v) {
    v &= 0xff;
    if (v < 16)
        Serial.print('0');
    Serial.print(v, HEX);
}

void onEvent (ev_t ev) {
    Serial.print(os_getTime());
    Serial.print(": ");
    switch(ev) {
        case EV_SCAN_TIMEOUT:
            Serial.println(F("EV_SCAN_TIMEOUT"));
            break;
        case EV_BEACON_FOUND:
            Serial.println(F("EV_BEACON_FOUND"));
            break;
        case EV_BEACON_MISSED:
            Serial.println(F("EV_BEACON_MISSED"));
            break;
        case EV_BEACON_TRACKED:
            Serial.println(F("EV_BEACON_TRACKED"));
            break;
        case EV_JOINING:
            Serial.println(F("EV_JOINING"));
            break;
        case EV_JOINED:
            Serial.println(F("EV_JOINED"));
            {
              u4_t netid = 0;
              devaddr_t devaddr = 0;
              u1_t nwkKey[16];
              u1_t artKey[16];
              LMIC_getSessionKeys(&netid, &devaddr, nwkKey, artKey);
              Serial.print("netid: ");
              Serial.println(netid, DEC);
              Serial.print("devaddr: ");
              Serial.println(devaddr, HEX);
              Serial.print("AppSKey: ");
              for (size_t i=0; i<sizeof(artKey); ++i) {
                if (i != 0)
                  Serial.print("-");
                printHex2(artKey[i]);
              }
              Serial.println("");
              Serial.print("NwkSKey: ");
              for (size_t i=0; i<sizeof(nwkKey); ++i) {
                      if (i != 0)
                              Serial.print("-");
                      printHex2(nwkKey[i]);
              }
              Serial.println();
            }
            LMIC_setLinkCheckMode(0);
            break;
        case EV_JOIN_FAILED:
            Serial.println(F("EV_JOIN_FAILED"));
            break;
        case EV_REJOIN_FAILED:
            Serial.println(F("EV_REJOIN_FAILED"));
            break;
        case EV_TXCOMPLETE:
            Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
            if (LMIC.txrxFlags & TXRX_ACK)
              Serial.println(F("Received ack"));
            if (LMIC.dataLen) {
              Serial.print(F("Received "));
              Serial.print(LMIC.dataLen);
              Serial.println(F(" bytes of payload"));
            }
            os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
            break;
        case EV_LOST_TSYNC:
            Serial.println(F("EV_LOST_TSYNC"));
            break;
        case EV_RESET:
            Serial.println(F("EV_RESET"));
            break;
        case EV_RXCOMPLETE:
            Serial.println(F("EV_RXCOMPLETE"));
            break;
        case EV_LINK_DEAD:
            Serial.println(F("EV_LINK_DEAD"));
            break;
        case EV_LINK_ALIVE:
            Serial.println(F("EV_LINK_ALIVE"));
            break;
        case EV_TXSTART:
            Serial.println(F("EV_TXSTART"));
            break;
        case EV_TXCANCELED:
            Serial.println(F("EV_TXCANCELED"));
            break;
        case EV_RXSTART:
            break;
        case EV_JOIN_TXCOMPLETE:
            Serial.println(F("EV_JOIN_TXCOMPLETE: no JoinAccept"));
            break;
        default:
            Serial.print(F("Unknown event: "));
            Serial.println((unsigned) ev);
            break;
    }
}

void do_send(osjob_t* j){
    if (LMIC.opmode & OP_TXRXPEND) {
        Serial.println(F("OP_TXRXPEND, not sending"));
    } else {
        LMIC_setTxData2(1, mydata, sizeof(mydata)-1, 0);
        Serial.println(F("Packet queued"));
    }
}

void setup() {
    Serial.begin(9600);
    Serial.println(F("Starting"));

    #ifdef VCC_ENABLE
    pinMode(VCC_ENABLE, OUTPUT);
    digitalWrite(VCC_ENABLE, HIGH);
    delay(1000);
    #endif

    // HT-VMT190 wires the LoRa SPI bus to non-default ESP32-S3 pins;
    // set them up before os_init()/hal_init() call the bare SPI.begin().
    SPI.begin(9 /*SCK*/, 11 /*MISO*/, 10 /*MOSI*/, 8 /*NSS*/);

    os_init();
    LMIC_reset();
    do_send(&sendjob);
}

void loop() {
    os_runloop_once();
}
