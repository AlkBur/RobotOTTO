#include "WebSocketServer.h"

WebSocketServer::WebSocketServer(uint16_t port)
	: _server(port), _ws("/ws"), _port(port), _eventHandler(nullptr)
{
	// Инициализация WebSocket
	_ws.onEvent([this](AsyncWebSocket *server, AsyncWebSocketClient *client,
					   AwsEventType type, void *arg, uint8_t *data, size_t len)
				{ this->_handleWebSocketEvent(server, client, type, arg, data, len); });
	_server.addHandler(&_ws);
}

WebSocketServer::~WebSocketServer()
{
	// Деструктор – ничего особенного
}

void WebSocketServer::begin()
{
	_server.begin();
}

void WebSocketServer::onEvent(WebSocketEventHandler handler)
{
	_eventHandler = handler;
}

void WebSocketServer::broadcast(const char *message)
{
	_ws.textAll(message);
}

void WebSocketServer::broadcast(String message)
{
	broadcast(message.c_str());
}

void WebSocketServer::sendToClient(void *clientID, const char *message)
{
	AsyncWebSocketClient *client = reinterpret_cast<AsyncWebSocketClient *>(clientID);
	if (client && client->status() == WS_CONNECTED)
	{
		client->text(message);
	}
}

void WebSocketServer::sendToClient(void *clientID, String message)
{
	sendToClient(clientID, message.c_str());
}

size_t WebSocketServer::clientCount() const
{
	return _ws.count();
}

void WebSocketServer::_handleWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
											AwsEventType type, void *arg, uint8_t *data, size_t len)
{
	if (_eventHandler)
	{
		switch (type)
		{
		case WS_EVT_CONNECT:
			_eventHandler((void *)client, 0, nullptr, 0);
			break;
		case WS_EVT_DISCONNECT:
			_eventHandler((void *)client, 1, nullptr, 0);
			break;
		case WS_EVT_DATA:
		{
			AwsFrameInfo *info = (AwsFrameInfo *)arg;
			if (info->final && info->index == 0 && info->len == len)
			{
				// Целое сообщение
				if (info->opcode == WS_TEXT)
				{
					data[len] = 0; // null-terminator для безопасного использования как строки
					_eventHandler((void *)client, 2, (const char *)data, len);
				}
			}
			else
			{
				// Фрагментированное сообщение – можно объединить, но для простоты игнорируем
			}
		}
		break;
		default:
			break;
		}
	}
}