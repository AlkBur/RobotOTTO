#pragma once

#include <Arduino.h>
#include <DFRobotDFPlayerMini.h>

class DFPlayer
{
public:
	DFPlayer(uint8_t rxPin, uint8_t txPin);

	bool begin(unsigned long baudRate = 9600);
	void play();
	void stop();
	void next();
	void previous();
	void pause();
	void setVolume(uint8_t volume);
	uint8_t getStatus();

	bool isAvailable() const;
	void update(); // Этот метод нужно вызывать в основном цикле loop()

private:
	uint8_t _rxPin;
	uint8_t _txPin;
	HardwareSerial *_serial;
	DFRobotDFPlayerMini _player;
	bool _initialized;

	bool _isPlaying();
};