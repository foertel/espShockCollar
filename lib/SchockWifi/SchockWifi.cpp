#ifdef _ESP8266
    #include <ESP8266WiFiMulti.h>
    
    typedef ESP8266WiFiMulti WiFiMulti;
#endif

#ifdef _ESP32
    #include <WiFi.h>
    #include <WiFiMulti.h>
#endif

#include "SchockWifi.h"

#include "config.h"

WiFiMulti *g_wifi;
char SchockWifi::m_mac[18];
char SchockWifi::m_ssid[33];

void SchockWifi::init()
{
    Serial.print("WiFi: Init Begin\n");
    delay(800); // delay to prevent startup issues due to current spike (bad hw revision/design)

    g_wifi = new WiFiMulti();

    Serial.printf("WiFi: SSID(%s) Secret(%s)\n", WIFI_SSID, WIFI_SECRET);
    g_wifi->addAP(WIFI_SSID, WIFI_SECRET);

    while (1)
    {
        Serial.printf("WiFi: trying to connect..\n");
        uint8_t ret = g_wifi->run(WIFI_CONNECT_TIMEOUT);
        if (ret == WL_CONNECTED)
            break;
    }

#ifdef _ESP8266
    Serial.printf("WiFi: IP...: %s\n", WiFi.localIP().toString().c_str());
#else
    Serial.printf("WiFi: IP...: %s/%u\n", WiFi.localIP().toString().c_str(), WiFi.subnetCIDR());
#endif
    Serial.printf("WiFi: GW...: %s\n", WiFi.gatewayIP().toString().c_str());
    Serial.printf("WiFi: DNS..: %s\n", WiFi.dnsIP().toString().c_str());

    memset(m_mac, 0x00, sizeof(m_mac));
    memset(m_ssid, 0x00, sizeof(m_ssid));

    Serial.printf("WiFi: Init Done\n");
}

int8_t SchockWifi::getRSSI()
{
    return WiFi.RSSI();
}

const char *SchockWifi::getSSID()
{
    strncpy(m_ssid, WiFi.BSSIDstr().c_str(), sizeof(m_ssid) - 1);
    return m_ssid;
}

const char *SchockWifi::getMac()
{
    strcpy(m_mac, WiFi.macAddress().c_str());
    return m_mac;
}