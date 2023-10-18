#include <Arduino.h>
#include <Collar.h>

#ifdef _ESP8266
const uint8_t rx_pin = D2;
#endif

#ifdef _ESP32
const uint8_t rx_pin = 11;
#endif

#include "SchockSocket.h"
#include "SchockWifi.h"
#include "Collar.h"

Collar *_collar;

SchockSocket *sock = NULL;
SchockWifi *wifi = NULL;

volatile bool got_message = false;
struct collar_message rx_message;

/** Called from within an interupt handler when a message is received,
 *  therefore it shouldn't do much.
 * @todo can we move this somewhere nicer?
 */
void message_callback(const struct collar_message *msg)
{
	memcpy(&rx_message, msg, sizeof(collar_message));
	got_message = true;
}

void setup()
{
	Serial.begin(115200);

	Serial.println("\nStarting ...");
	_collar = new Collar();
	_collar->setRxPin(rx_pin);
	_collar->setMessageCallback(message_callback);
	_collar->startRx();

	wifi = new SchockWifi();
	wifi->init();

	SchockSocket::getInstance()->init();	
}

void loop()
{
	SchockSocket::getInstance()->loop();
	if (got_message)
	{
		Serial.println("\nGot message:");
		Collar::print_message(&rx_message);
		DynamicJsonDocument collarData(100); 
		collarData["channel"] = rx_message.channel;
		collarData["receiverId"] = rx_message.id;
		SchockSocket::getInstance()->emit("newCollar", collarData);
		got_message = false;
		Serial.println("");
	}
}