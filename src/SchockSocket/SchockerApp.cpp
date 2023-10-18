#include <Arduino.h>
#include <stdint.h>

#ifdef _ESP8266
const uint8_t tx_pin = D1;
#endif

#ifdef _ESP32
// @todo config PIN for 433 mhz sender
const uint8_t tx_pin = 0;
#endif

#include "SchockSocket.h"
#include "SchockWifi.h"
#include "Collar.h"

Collar *_collar;

SchockSocket *sock = NULL;
SchockWifi *wifi = NULL;

void setup()
{
	Serial.begin(115200);

	Serial.println("\nStarting ...");
	_collar = new Collar();
	_collar->setTxPin(tx_pin);
	_collar->initSender();

	wifi = new SchockWifi();
	wifi->init();

	SchockSocket::getInstance()->init();
	SchockSocket::getInstance()->on("test", [](const char *eventName, DynamicJsonDocument payload)
									{ Serial.println("Got 'test'. hallo."); });

	SchockSocket::getInstance()->on("schock", [&_collar](const char *eventName, DynamicJsonDocument payload) {
		uint8_t strength = payload["strength"];
		uint16_t collarId = payload["id"];
		uint8_t channel = payload["channel"];

		collar_channel ch = collar_channel::CH1;
		switch(channel){
			case 2:
				ch = collar_channel::CH2;
			break;
			
			case 3:
				ch = collar_channel::CH3;
			break;

		}
		_collar->transmit(
			collarId,
			ch,
			collar_mode::VIBE,
			strength
		);

        Serial.printf("Sending schock .. id: %X; strengh: %d\n", collarId, strength);
	});

	SchockSocket::getInstance()->on("vibrate", [&_collar](const char *eventName, DynamicJsonDocument payload) {
		uint8_t strength = payload["strength"];
		uint16_t collarId = payload["id"];
		uint8_t channel = payload["channel"];

		collar_channel ch = collar_channel::CH1;
		switch(channel){
			case 2:
				ch = collar_channel::CH2;
			break;
			
			case 3:
				ch = collar_channel::CH3;
			break;

		}
	
		_collar->transmit(
			collarId,
			ch,
			collar_mode::VIBE,
			strength
		);

        Serial.printf("Sending vibration .. id: %X; strengh: %d\n", collarId, strength);
	});

	SchockSocket::getInstance()->on("beep", [&_collar](const char *eventName, DynamicJsonDocument payload) {
		uint8_t strength = payload["strength"];
		uint16_t collarId = payload["id"];
		uint8_t channel = payload["channel"];

		collar_channel ch = collar_channel::CH1;
		switch(channel){
			case 2:
				ch = collar_channel::CH2;
			break;
			
			case 3:
				ch = collar_channel::CH3;
			break;

		}

		_collar->transmit(
			collarId,
			ch,
			collar_mode::BEEP,
			strength
		);

        Serial.printf("Sending beep .. id: %X; strengh: %d\n", collarId, strength);
	});
}

void loop()
{
	SchockSocket::getInstance()->loop();
}