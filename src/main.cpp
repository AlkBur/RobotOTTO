#include <Arduino.h>
// web
#include <SPI.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include "index_html.h"
#include "WebSocketServer.h"
// servo
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include "ServoController.h"
#include "SchedulerServo.h"
// Player
#include "DFPlayer.h"
#include "Eyes.h"
// Logger
#include "Logger.h"

// Global devices
DFPlayer mp3(20, 21);
WebSocketServer webSocket(80);
Adafruit_PWMServoDriver pwmDriver = Adafruit_PWMServoDriver();

// Создаём объект Eyes (адрес TCA9548A по умолчанию 0x70)
Eyes eyes;

ServoController leftLeg(pwmDriver, 5, SERVO_LEG, 120, 500);
ServoController rightLeg(pwmDriver, 9, SERVO_LEG, 130, 520);
ServoController leftFoot(pwmDriver, 4, SERVO_FOOT, 257, 337, 279, 314);
ServoController rightFoot(pwmDriver, 8, SERVO_FOOT, 257, 335, 279, 312);

SchedulerServo scheduler;

namespace
{
	constexpr uint8_t kDefaultMp3Volume = 10;
	volatile bool g_retryInitAndPlayRequested = false;

	bool initMp3Player()
	{
		if (mp3.begin())
		{
			LOG_INFO("Main", "DFPlayer initialized");
			mp3.setVolume(kDefaultMp3Volume);
			LOG_DEBUG("Main", "DFPlayer status: %d", mp3.getStatus());
			return true;
		}

		LOG_ERROR("Main", "DFPlayer init failed");
		return false;
	}
} // namespace

// WebSocket event handler
void onWebSocketEvent(void *clientID, uint8_t type, const char *msg, size_t len)
{
	if (type == 2 && msg)
	{
		if (msg[0] == 'J' && len >= 9)
		{
			int x = atoi(msg + 1);
			int y = atoi(msg + 5);
			LOG_DEBUG("WEB", "Joystick: X=%d, Y=%d", x, y);

			int leftSpeed = y + x;
			int rightSpeed = y - x;
			int maxAbs = max(abs(leftSpeed), abs(rightSpeed));

			if (maxAbs > 100)
			{
				leftSpeed = leftSpeed * 100 / maxAbs;
				rightSpeed = rightSpeed * 100 / maxAbs;
			}

			leftSpeed = -leftSpeed;

			LOG_DEBUG("WEB", "Left speed: %d, Right speed: %d", leftSpeed, rightSpeed);

			leftFoot.setSpeed(leftSpeed);
			rightFoot.setSpeed(rightSpeed);
		}
		else if (msg[0] == 'D')
		{
			char dir = msg[1];
			bool pressed = (msg[2] == '1');

			if (pressed && dir == 'U')
			{
				LOG_DEBUG("WEB", "D-Pad: %c", dir);
				scheduler.stop();
				scheduler.add(&leftLeg, 180, 30.0f);
				scheduler.add(&rightLeg, 0, 30.0f);
				scheduler.start();
			}
			else if (pressed && dir == 'D')
			{
				LOG_DEBUG("WEB", "D-Pad: %c", dir);
				scheduler.stop();
				scheduler.add(&leftLeg, 90, 30.0f);
				scheduler.add(&rightLeg, 90, 30.0f);
				scheduler.start();
			}
		}
		else if (msg[0] == 'V')
		{
			int vol = atoi(msg + 1);
			LOG_DEBUG("WEB", "Volume: %d", vol);
			mp3.setVolume(vol);
		}
		else if (strncmp(msg, "PP", 2) == 0)
		{
			bool play = (msg[2] == '0');
			LOG_DEBUG("WEB", "Play");
			if (play)
			{
				if (!mp3.isAvailable())
				{
					LOG_WARNING("Main", "DFPlayer is not initialized, scheduling retry before play");
					g_retryInitAndPlayRequested = true;
					return;
				}
				mp3.play();
			}
			else
			{
				mp3.pause();
			}
		}
		else if (strcmp(msg, "PREV") == 0)
		{
			LOG_DEBUG("WEB", "PREV");
			mp3.previous();
		}
		else if (strcmp(msg, "NEXT") == 0)
		{
			LOG_DEBUG("WEB", "NEXT");
			mp3.next();
		}
		else if (msg[0] == 'M')
		{
			int moveId = msg[1] - '0';
			LOG_DEBUG("WEB", "moveId: %d", moveId);
		}
		else if (msg[0] == 'L')
		{
			int leg = msg[1] - '0';
			int angle = atoi(msg + 2);
			LOG_DEBUG("WEB", "leg: %d; angle: %d", leg, angle);
			scheduler.stop();
			if (leg == 0)
			{
				leftLeg.setAngle(angle);
			}
			else
			{
				rightLeg.setAngle(angle);
			}
		}
	}
}

void setup()
{
	Logger::begin(115200);
	Logger::setLevel(LOG_LEVEL_DEBUG);

	Wire.begin(0, 3, 400000);
	// Wire.setClock(400000);
	Serial.println("I2C initialized (SDA=0, SCL=3, 400kHz)");
	delay(10);

	if (initMp3Player())
	{
		mp3.play();
	}
	delay(10);

	// Инициализация глаз
	if (eyes.begin())
	{
		LOG_INFO("Main", "Eyes initialized");
	}
	else
	{
		LOG_ERROR("Main", "Eyes init failed");
	}

	// Настройка (опционально)
	eyes.setAutoMove(true);	 // автоматическое движение
	eyes.setAutoBlink(true); // автоматическое моргание

	pwmDriver.begin();
	pwmDriver.setOscillatorFrequency(27000000);
	pwmDriver.setPWMFreq(50);
	delay(10);
	Serial.println("PCA9685 initialized at 50Hz");

	leftLeg.begin();
	rightLeg.begin();
	leftFoot.begin();
	rightFoot.begin();

	leftLeg.setAngle(90);
	rightLeg.setAngle(90);
	leftFoot.setStop();
	rightFoot.setStop();

	WiFi.mode(WIFI_AP);
	WiFi.softAP(WIFI_SSID, WIFI_PASS);
	LOG_INFO("Main", "AP started");
	LOG_INFO("Main", "AP IP: %s", WiFi.softAPIP().toString().c_str());
	delay(100);

	webSocket.getServer()->on("/", HTTP_GET, [](AsyncWebServerRequest *request)
							  { request->send(200, "text/html", htmlContent); });
	webSocket.onEvent(onWebSocketEvent);
	webSocket.begin();
	LOG_INFO("Main", "WebSocket server started at /ws");
}

void loop()
{
	if (g_retryInitAndPlayRequested)
	{
		g_retryInitAndPlayRequested = false;
		LOG_INFO("Main", "Retrying DFPlayer init from loop");
		if (initMp3Player())
		{
			mp3.play();
		}
		else
		{
			LOG_ERROR("Main", "DFPlayer reinit failed");
		}
	}

	mp3.update();
	scheduler.update();
	// Обновление состояния глаз (автоматическое движение и моргание)
	eyes.update();
}
