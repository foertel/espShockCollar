#include <Arduino.h>
#include <stdint.h>

#ifdef _ESP8266
const uint8_t tx_pin = D1;
const uint8_t start_pin = D5;
const uint8_t wire_pin = D6;
const uint8_t stop_pin = D7;
#endif

#ifdef _ESP32
// @todo config PIN for 433 mhz sender
const uint8_t tx_pin = 0;
#endif

#include "Collar.h"

Collar *_collar;
volatile bool gameStarted = false;
volatile bool wireTouched = false;
volatile bool gameDone = false;
volatile unsigned long timerStart = 0;

IRAM_ATTR void startPinEvent()
{
	gameStarted = true;
	timerStart = millis();
	Serial.println("Game started");
}

IRAM_ATTR void wirePinEvent()
{
	if (gameStarted)
	{
		wireTouched = true;
	}
}

IRAM_ATTR void stopPinEvent()
{
	if (gameStarted)
	{
		gameDone = true;
	}
}

void setup()
{
	Serial.begin(115200);

	Serial.println("\nStarting ...");
	_collar = new Collar();
	_collar->setTxPin(tx_pin);
	_collar->initSender();

	pinMode(start_pin, INPUT_PULLUP);
	pinMode(wire_pin, INPUT_PULLUP);
	pinMode(stop_pin, INPUT_PULLUP);

	attachInterrupt(digitalPinToInterrupt(start_pin), startPinEvent, FALLING);
	attachInterrupt(digitalPinToInterrupt(wire_pin), wirePinEvent, FALLING);
	attachInterrupt(digitalPinToInterrupt(stop_pin), stopPinEvent, FALLING);
}

void loop()
{
	if (wireTouched)
	{
		Serial.println("Wire touched");
		_collar->transmit(
			0xF737,
			collar_channel::CH1,
			collar_mode::VIBE,
			50);

		wireTouched = false;
		gameStarted = false;
	}

	if (gameDone)
	{
		Serial.println("Game done");
		unsigned long timeTaken = millis() - timerStart;
		Serial.printf("Vergangene Zeit: %.2f Sekunden\n", timeTaken / 1000.0);

		gameStarted = false;
		gameDone = false;
	}
}