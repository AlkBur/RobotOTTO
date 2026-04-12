#pragma once

#include <Arduino.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

// Тип функции-коллбэка для событий WebSocket
// clientID - идентификатор клиента (можно использовать как указатель)
// type - тип события (подключение, отключение, получение данных)
// message - сообщение (для события TEXT)
// len - длина сообщения
typedef std::function<void(void *clientID, uint8_t type, const char *msg, size_t len)> WebSocketEventHandler;

class WebSocketServer
{
public:
	WebSocketServer(uint16_t port = 80);
	~WebSocketServer();

	// Запуск сервера
	void begin();

	AsyncWebServer *getServer() { return &_server; }

	// Установка обработчика событий
	void onEvent(WebSocketEventHandler handler);

	// Отправить текст всем подключённым клиентам
	void broadcast(const char *message);
	void broadcast(String message);

	// Отправить текст конкретному клиенту
	void sendToClient(void *clientID, const char *message);
	void sendToClient(void *clientID, String message);

	// Получить количество клиентов
	size_t clientCount() const;

private:
	AsyncWebServer _server;
	AsyncWebSocket _ws;
	uint16_t _port;
	WebSocketEventHandler _eventHandler;

	void _handleWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
							   AwsEventType type, void *arg, uint8_t *data, size_t len);
};