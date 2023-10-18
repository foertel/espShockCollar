#pragma once

class SchockWifi{
public:
    static void init();

    static int8_t getRSSI();
    static const char *getSSID();   // Not Thread Safe!
    static const char *getMac();    // Not Thread Safe!

private:
    static char m_ssid[33]; // station ssid, used as buffer
    static char m_mac[18];  // station mac, used as buffer
};