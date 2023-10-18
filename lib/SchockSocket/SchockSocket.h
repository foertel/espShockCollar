#pragma once

#include "WebSockets.h"
#include "SocketIOclient.h"

#include <map>
#include <vector>

#include <ArduinoJson.h>

class SchockSocket {
public:
	void init();
	void loop();
	void on(const char *eventName, std::function<void(const char *eventName, DynamicJsonDocument payload)>);
	void handle(const char *eventName, DynamicJsonDocument &payload);
	bool emit(const char* event, DynamicJsonDocument &payload);

	static SchockSocket *getInstance();

private:
	SocketIOclient *sock = NULL;
	std::map<String, std::function<void(const char *eventName, DynamicJsonDocument payload)>> _events;
};