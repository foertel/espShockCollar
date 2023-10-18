#include "SchockSocket.h"

#include "config.h"

SchockSocket *g_socket_inst = NULL;

SchockSocket *SchockSocket::getInstance() {
	if (!g_socket_inst || g_socket_inst == NULL)
		g_socket_inst = new SchockSocket();

	return g_socket_inst;
}

void webSocketEvent(socketIOmessageType_t type, uint8_t *payload, size_t length)
{
	switch (type)
	{
	case socketIOmessageType_t::sIOtype_EVENT:
		DynamicJsonDocument doc(100);

		// Deserialize the JSON document
		DeserializationError error = deserializeJson(doc, payload);
		if (error)
			Serial.println("Failed to decode json from request");

		String eventName = doc[0].as<String>();
		String payload = doc[1].as<String>();
		Serial.printf("SOCKET::IN [%s]: %s\n", eventName.c_str(), payload.c_str());

		DynamicJsonDocument docref(100);
		docref = doc[1];
		SchockSocket::getInstance()->handle(eventName.c_str(), docref);

		docref.clear();
		doc.clear();
		break;
	}
}

void SchockSocket::init()
{
	this->sock = new SocketIOclient();
	this->sock->onEvent(webSocketEvent);
	this->sock->begin(WS_HOST, WS_PORT, "/socket.io/?EIO=3");
	this->sock->setReconnectInterval(5000);
	// this->sock_>s
	Serial.println("Websocket Connected!");

	this->on("helo", [this](const char *eventName, DynamicJsonDocument payload) {
        Serial.printf("Got HELO: sending Join\n");
		DynamicJsonDocument doc(100); 
		SchockSocket::getInstance()->emit("joinSchocker", doc);

		DynamicJsonDocument gameParams(100); 
		gameParams["name"] = "heisserdraht";
		gameParams["humanName"] = "Heißer Draht";
		gameParams["playerCount"] = 1;
		SchockSocket::getInstance()->emit("registerGame", gameParams);


		DynamicJsonDocument gameParams2(100); 
		gameParams2["name"] = "schnitzeljagdt";
		gameParams2["humanName"] = "Schnitel Jagdt";
		gameParams2["playerCount"] = 4;
		SchockSocket::getInstance()->emit("registerGame", gameParams2);

		// DynamicJsonDocument collarData(100); 
		// collarData["channel"] = 1;
		// collarData["receiverId"] = 65342;
		// SchockSocket::getInstance()->emit("newCollar", collarData);
	});
}

void SchockSocket::loop()
{
	if (this->sock != NULL)
		this->sock->loop();
}

void SchockSocket::on(const char *eventName, std::function<void(const char *eventName, DynamicJsonDocument payload)> cb) {
	this->_events[eventName] = cb;
}

void SchockSocket::handle(const char *eventName, DynamicJsonDocument &payload)
{
	if(this->_events.count(eventName) != 1)
		return;

	this->_events[eventName](eventName, payload);
}

bool SchockSocket::emit(const char* event, DynamicJsonDocument &payload) {

	// creat JSON message for Socket.IO (event) 
	DynamicJsonDocument doc(100); 
	JsonArray array = doc.to<JsonArray>(); 
	
	// add event name 
	// Hint: socket.on('event_name', .... 
	array.add(event); 
	
	// add payload (parameters) for the event 
	array.add(payload);
	
	// JSON to String (serializion) 
	String output; 
	serializeJson(doc, output); 
	
	// Send event
	this->sock->sendEVENT(output);
	payload.clear();
	return true;
}