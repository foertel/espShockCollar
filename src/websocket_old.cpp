#ifdef DONTBUILD
#include <Arduino.h>

#include <Collar.h>

const uint8_t tx_pin = D1;

struct collar_message rx_message;
Collar* _collar;

void setup()
{
    Serial.begin(9600);

    Serial.println("Starting Sender");
    
    _collar = new Collar();
    _collar->setTxPin(tx_pin);
    _collar->initSender();
}

void loop()
{
    delay(2000);
    _collar->transmit(
        0xF737,
        collar_channel::CH1,
        collar_mode::BEEP,
        50
    );
    delay(1000);
    _collar->transmit(
        0xF737,
        collar_channel::CH1,
        collar_mode::VIBE,
        50
    );
}
#endif